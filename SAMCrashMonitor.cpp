#include "SAMCrashMonitor.h"
#include <samd.h>

void WDT_Handler(void) {
    // ISR for watchdog early warning, DO NOT RENAME!
    #if defined(__SAMD51__)
        WDT->CTRLA.bit.ENABLE = 0;       // Disable watchdog
        while(WDT->SYNCBUSY.reg);
    #else
        WDT->CTRL.bit.ENABLE = 0;        // Disable watchdog
        while(WDT->STATUS.bit.SYNCBUSY); // Sync CTRL write
    #endif

    WDT->INTFLAG.bit.EW  = 1;        // Clear interrupt flag
}

SAMCrashMonitorClass::SAMCrashMonitorClass() {
    this->_initialized = false;
}

void SAMCrashMonitorClass::initWatchdog() {
    if (this->_initialized) { return; }

    #ifdef __SAMD51__
        // SAMD51 WDT uses OSCULP32k as input clock now
        // section: 20.5.3
        OSC32KCTRL->OSCULP32K.bit.EN1K  = 1; // Enable out 1K (for WDT)
        OSC32KCTRL->OSCULP32K.bit.EN32K = 0; // Disable out 32K

        // Enable WDT early-warning interrupt
        NVIC_DisableIRQ(WDT_IRQn);
        NVIC_ClearPendingIRQ(WDT_IRQn);
        NVIC_SetPriority(WDT_IRQn, 0); // Top priority
        NVIC_EnableIRQ(WDT_IRQn);

        while(WDT->SYNCBUSY.reg);
    
        USB->DEVICE.CTRLA.bit.ENABLE = 0;         // Disable the USB peripheral
        while(USB->DEVICE.SYNCBUSY.bit.ENABLE);   // Wait for synchronization
        USB->DEVICE.CTRLA.bit.RUNSTDBY = 0;       // Deactivate run on standby
        USB->DEVICE.CTRLA.bit.ENABLE = 1;         // Enable the USB peripheral
        while(USB->DEVICE.SYNCBUSY.bit.ENABLE);   // Wait for synchronization
    #else
        // Generic clock generator 2, divisor = 32 (2^(DIV+1))
        GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);

        // Enable clock generator 2 using low-power 32KHz oscillator.
        // With /32 divisor above, this yields 1024Hz(ish) clock.
        GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |
                        GCLK_GENCTRL_GENEN |
                        GCLK_GENCTRL_SRC_OSCULP32K |
                        GCLK_GENCTRL_DIVSEL;
        while(GCLK->STATUS.bit.SYNCBUSY);

        // WDT clock = clock gen 2
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
                        GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK2;

        // Enable WDT early-warning interrupt
        NVIC_DisableIRQ(WDT_IRQn);
        NVIC_ClearPendingIRQ(WDT_IRQn);
        NVIC_SetPriority(WDT_IRQn, 0); // Top priority
        NVIC_EnableIRQ(WDT_IRQn);
    #endif

    this->_initialized = true;
}

int SAMCrashMonitorClass::enableWatchdog(int maxPeriodMS) {
    // Enable the watchdog with a period up to the specified max period in
    // milliseconds.

    // Review the watchdog section from the SAMD21 datasheet section 17:
    // http://www.atmel.com/images/atmel-42181-sam-d21_datasheet.pdf

    int     cycles;
    uint8_t bits;

    if (!_initialized) {
        this->initWatchdog();
    }

    #ifdef __SAMD51__
        WDT->CTRLA.reg = 0; // Disable watchdog for config
        while(WDT->SYNCBUSY.reg);
    #else
        WDT->CTRL.reg = 0; // Disable watchdog for config
        while(WDT->STATUS.bit.SYNCBUSY);
    #endif

    // You'll see some occasional conversion here compensating between
    // milliseconds (1000 Hz) and WDT clock cycles (~1024 Hz).  The low-
    // power oscillator used by the WDT ostensibly runs at 32,768 Hz with
    // a 1:32 prescale, thus 1024 Hz, though probably not super precise.

    if((maxPeriodMS >= 16000) || !maxPeriodMS) {
        cycles = 16384;
        bits   = 0xB;
    } else {
        cycles = (maxPeriodMS * 1024L + 500) / 1000; // ms -> WDT cycles
        if(cycles >= 8192) {
            cycles = 8192;
            bits   = 0xA;
        } else if(cycles >= 4096) {
            cycles = 4096;
            bits   = 0x9;
        } else if(cycles >= 2048) {
            cycles = 2048;
            bits   = 0x8;
        } else if(cycles >= 1024) {
            cycles = 1024;
            bits   = 0x7;
        } else if(cycles >= 512) {
            cycles = 512;
            bits   = 0x6;
        } else if(cycles >= 256) {
            cycles = 256;
            bits   = 0x5;
        } else if(cycles >= 128) {
            cycles = 128;
            bits   = 0x4;
        } else if(cycles >= 64) {
            cycles = 64;
            bits   = 0x3;
        } else if(cycles >= 32) {
            cycles = 32;
            bits   = 0x2;
        } else if(cycles >= 16) {
            cycles = 16;
            bits   = 0x1;
        } else {
            cycles = 8;
            bits   = 0x0;
        }
    }

    // Watchdog timer on SAMD is a slightly different animal than on AVR.
    // On AVR, the WTD timeout is configured in one register and then an
    // interrupt can optionally be enabled to handle the timeout in code
    // (as in waking from sleep) vs resetting the chip.  Easy.
    // On SAMD, when the WDT fires, that's it, the chip's getting reset.
    // Instead, it has an "early warning interrupt" with a different set
    // interval prior to the reset.  For equivalent behavior to the AVR
    // library, this requires a slightly different configuration depending
    // whether we're coming from the sleep() function (which needs the
    // interrupt), or just enable() (no interrupt, we want the chip reset
    // unless the WDT is cleared first).  In the sleep case, 'windowed'
    // mode is used in order to allow access to the longest available
    // sleep interval (about 16 sec); the WDT 'period' (when a reset
    // occurs) follows this and is always just set to the max, since the
    // interrupt will trigger first.  In the enable case, windowed mode
    // is not used, the WDT period is set and that's that.
    // The 'isForSleep' argument determines which behavior is used;
    // this isn't present in the AVR code, just here.  It defaults to
    // 'false' so existing Arduino code works as normal, while the sleep()
    // function (later in this file) explicitly passes 'true' to get the
    // alternate behavior.

    #if defined(__SAMD51__)
        WDT->INTENCLR.bit.EW     = 1;    // Disable early warning interrupt
        WDT->CONFIG.bit.PER      = bits; // Set period for chip reset
        WDT->CTRLA.bit.WEN       = 0;    // Disable window mode
        while(WDT->SYNCBUSY.reg);        // Sync CTRL write

        this->iAmAlive();                    // Clear watchdog interval
        WDT->CTRLA.bit.ENABLE = 1;           // Start watchdog now!
        while(WDT->SYNCBUSY.reg);
    #else
        WDT->INTENCLR.bit.EW   = 1;         // Disable early warning interrupt
        WDT->CONFIG.bit.PER    = bits;      // Set period for chip reset
        WDT->CTRL.bit.WEN      = 0;         // Disable window mode
        while(WDT->STATUS.bit.SYNCBUSY);    // Sync CTRL write

        this->iAmAlive();                    // Clear watchdog interval
        WDT->CTRL.bit.ENABLE = 1;            // Start watchdog now!
        while(WDT->STATUS.bit.SYNCBUSY);
    #endif

    return (cycles * 1000L + 512) / 1024; // WDT cycles -> ms
}

void SAMCrashMonitorClass::disableWatchdog() {
    #ifdef __SAMD51__
        WDT->CTRLA.bit.ENABLE = 0;
        while(WDT->SYNCBUSY.reg);
    #else
        WDT->CTRL.bit.ENABLE = 0;
        while(WDT->STATUS.bit.SYNCBUSY);
    #endif
}

void SAMCrashMonitorClass::iAmAlive() {
    // Write the watchdog clear key value (0xA5) to the watchdog
    // clear register to clear the watchdog timer and reset it.
    #ifdef __SAMD51__
        while(WDT->SYNCBUSY.reg);
    #else
        while(WDT->STATUS.bit.SYNCBUSY);
    #endif

    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}

uint8_t SAMCrashMonitorClass::getResetCause() {
    #ifdef __SAMD51__
        return RSTC->RCAUSE.reg;
    #else
        return PM->RCAUSE.reg;
    #endif
}

String SAMCrashMonitorClass::getResetDescription() {
    uint8_t cause = this->getResetCause();
    String result;
    switch (cause) {
        case PM_RCAUSE_SYST:
            result = "Reset requested by system";
            break;
        case PM_RCAUSE_WDT:
            result = "Reset requested by Watchdog";
            break;
        case PM_RCAUSE_EXT:
            result = "External reset requested";
            break;
        case PM_RCAUSE_BOD33:
            result = "Reset brown-out 3.3V";
            break;
        case PM_RCAUSE_BOD12:
            result = "Reset brown-out 1.2V";
            break;
        case PM_RCAUSE_POR:
            result = "Normal power-on reset";
            break;
        default:
            result = "Unknown reset code";
            break;
    }

    return result;
}

SAMCrashMonitorClass SAMCrashMonitor;