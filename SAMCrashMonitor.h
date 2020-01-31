#ifndef SAMCrashMonitor_h
#define SAMCrashMonitor_h

#include <Arduino.h>
#if !defined(SAMD_SERIES)
    #error "This library is currently on compatible with SAM D21 & D51-based MCUs."
#endif

/**
 * Crash report structure. Contains register values captured at the time
 * of a hard fault.
 */
struct SAMCrashReport
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;

    /**
     * 
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

    uint32_t cfsr;

    uint32_t hfsr;

    uint32_t dfsr;

    uint32_t afsr;

    uint32_t mmar;

    uint32_t bfar;
} __attribute__((__packed__));

/**
 * Provides firmware Watchdog functionality for SAM D21/D51 variant
 * Arduino-compatible boards.
 */
class SAMCrashMonitor
{
public:
    /**
     * 
     * 
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
     * 
     */
    static void dumpCrash(SAMCrashReport &report);

private:
    static void initWatchdog();
    static void printValue(const __FlashStringHelper *pLabel, uint32_t uValue, uint8_t uRadix, bool newLine);

    static bool _initialized;
};

#endif