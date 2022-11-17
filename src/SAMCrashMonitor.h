#ifndef SAMCrashMonitor_h
#define SAMCrashMonitor_h

#include <Arduino.h>
#ifndef ARDUINO_ARCH_SAMD
    #error "This library is currently only compatible with SAM D21 & D51-based MCUs."
#endif

/**
 * Crash report structure. Contains register values captured at the time
 * of a hard fault.
 */
struct SAMCrashReport
{
    /**
     * Register 0 from the stack.
     */
    uint32_t r0;

    /**
     * Register 1 from the stack.
     */
    uint32_t r1;

    /**
     * Register 2 from the stack.
     */
    uint32_t r2;

    /**
     * Register 3 from the stack.
     */
    uint32_t r3;

    /**
     * Register 12 from the stack.
     */
    uint32_t r12;

    /**
     * Link register.
     */
    uint32_t lr;

    /**
     * Program counter. This register value will contain the address of the
     * executing instruction when the crash occurred.
     */
    uint32_t pc;

    /**
     * Program status register.
     */
    uint32_t psr;

    /**
     * Configurable fault status register.
     */
    uint32_t cfsr;

    /**
     * Hard fault status register.
     */
    uint32_t hfsr;

    /**
     * Debug fault status register.
     */
    uint32_t dfsr;

    /**
     * Auxillary fault status register.
     */
    uint32_t afsr;

    /**
     * MemManage fault address register.
     */
    uint32_t mmar;

    /**
     * Bus fault address register.
     */
    uint32_t bfar;
} __attribute__((__packed__));

/**
 * Handler method signature for a user-defined crash handler.
 */
typedef void (*UserCrashHandler)(SAMCrashReport &report);

/**
 * Provides firmware Watchdog functionality for SAM D21/D51 variant
 * Arduino-compatible boards.
 */
class SAMCrashMonitor
{
public:
    /**
     * Initializes the monitor.
     */
    static void begin();

    /**
     * Enables the Watchdog. Once enabled, you must call iAmAlive() within
     * the specified timeout period or the system will automatically reset.
     * @param maxPeriodMS The maximum period in milliseconds. This is the
     * maximum amount of time to wait before considered the system "hung".
     * This value is merely a suggestion though, the actual timeout value
     * *may* differ but will not exceed this value.
     * @returns The actual value set, depending on clock divisor.
     */
    static int enableWatchdog(int maxPeriodMS);

    /**
     * Disables the Watchdog completely, until enabled again.
     */
    static void disableWatchdog();

    /**
     * Resets ("feeds") the Watchdog. This method must be called within
     * the set timeout to prevent system reset. This should especially
     * be called at the beginning of your loop() method.
     */
    static void iAmAlive();

    /**
     * Gets a flag value representing the cause for the previous
     * system reset.
     * @returns The code for the reset cause.
     */
    static uint8_t getResetCause();

    /**
     * Gets a description of the previous reset cause.
     * @returns A description string.
     */
    static String getResetDescription();

    /**
     * Dumps the reason code and description for the previous reset to
     * the specified printable stream interface (ie. Serial).
     */
    static void dump();

    /**
     * Dumps the specified crash report to the serial console.
     * @param report The crash report to dump.
     */
    static void dumpCrash(SAMCrashReport &report);

    /**
     * Sets the user crash handler. The specified handler will get called
     * any time the hard fault interrupt is triggered.
     * @param handler The callback method to execute.
     * @note It is important to keep the implmentation logic in the specified
     * handler as short as possible and execute as fast as possible. If your
     * callback takes too long to execute, it may not finish before the MCU
     * resets.
     */
    static void setUserCrashHandler(UserCrashHandler handler);

    /**
     * The user-defined crash handler if set; Otherwise, NULL.
     */
    static UserCrashHandler userCrashHandler;

private:
    static void initWatchdog();
    static void printValue(const __FlashStringHelper *pLabel, uint32_t uValue, uint8_t uRadix, bool newLine);

    static bool _initialized;
};

#endif