#ifndef SAMCrashMonitor_h
#define SAMCrashMonitor_h

#include <Arduino.h>
#if !defined(SAMD_SERIES)
    #error "This library is currently on compatible with SAM D21 & D51-based MCUs."
#endif


class SAMCrashMonitorClass
{
public:
    SAMCrashMonitorClass();
    int enableWatchdog(int maxPeriodMS);
    void disableWatchdog();
    void iAmAlive();
    uint8_t getResetCause();
    String getResetDescription();

private:
    void initWatchdog();

    bool _initialized;
};

extern SAMCrashMonitorClass SAMCrashMonitor;

#endif