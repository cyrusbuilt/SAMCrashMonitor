#ifndef SAMCrashMonitor_h
#define SAMCrashMonitor_h

#include <Arduino.h>
#if !defined(SAMD_SERIES)
    #error "This library is currently on compatible with SAM D21 & D51-based MCUs."
#endif


/**
 * Provides firmware Watchdog functionality for SAM D21/D51 variant
 * Arduino-compatible boards.
 */
class SAMCrashMonitorClass
{
public:
    /**
     * Default ctor.
     */ 
    SAMCrashMonitorClass();

    /**
     * Enables the Watchdog. Once enabled, you must call iAmAlive() within
     * the specified timeout period or the system will automatically reset.
     * @param maxPeriodMS The maximum period in milliseconds. This is the
     * maximum amount of time to wait before considered the system "hung".
     * This value is merely a suggestion though, the actual timeout value
     * *may* differ but will not exceed this value.
     * @returns The actual value set, depending on clock divisor.
     */
    int enableWatchdog(int maxPeriodMS);

    /**
     * Disables the Watchdog completely, until enabled again.
     */
    void disableWatchdog();

    /**
     * Resets ("feeds") the Watchdog. This method must be called within
     * the set timeout to prevent system reset. This should especially
     * be called at the beginning of your loop() method.
     */
    void iAmAlive();

    /**
     * Gets a flag value representing the cause for the previous
     * system reset.
     * @returns The code for the reset cause.
     */
    uint8_t getResetCause();

    /**
     * Gets a description of the previous reset cause.
     * @returns A description string.
     */
    String getResetDescription();

    /**
     * Dumps the reason code and description for the previous reset to
     * the specified printable stream interface (ie. Serial).
     * @param destination The print stream to dump the message to.
     */
    void dump(Print &destination);

private:
    void initWatchdog();

    bool _initialized;
};

extern SAMCrashMonitorClass SAMCrashMonitor;

#endif