#include <Arduino.h>
#include "SAMCrashMonitor.h"

void setup() {
    // The following 2 lines are *REQUIRED* to properly init Serial monitor.
    SerialUSB.begin(9600);
    while(!SerialUSB);
    
    SerialUSB.println();
    SAMCrashMonitor::begin();
    SAMCrashMonitor::dump();
    SerialUSB.println();
    
    SerialUSB.println(F("Enabling watchdog."));
    int timeout = SAMCrashMonitor::enableWatchdog(4000);
    SerialUSB.print(F("Watchdog enabled for "));
    SerialUSB.print(timeout);
    SerialUSB.println(" ms.");
    
    SerialUSB.println();
    SerialUSB.println(F("First test: Looping once per second for 5 seconds while feeding watchdog..."));
    for (int i = 1; i <= 5; i++) {
        SerialUSB.print(F("Loop #"));
        SerialUSB.println(i);
        delay(1000);
        SAMCrashMonitor::iAmAlive();
    }

    SerialUSB.println();
    SerialUSB.println(F("Disabling watchdog..."));
    SerialUSB.println();
    SAMCrashMonitor::disableWatchdog();

    SerialUSB.println(F("Second test: Exceed timeout and reset."));
    timeout = SAMCrashMonitor::enableWatchdog(4000);
    SerialUSB.print(F("Watchdog will reset controller in "));
    SerialUSB.print(timeout);
    SerialUSB.println(" ms!");
    delay(timeout + 1000);

    // We shouldn't get this far since the watchdog should reset the MCU.
}

void loop() {
    // Normally, you would place a call to SAMCrashMonitor::iAmAlive() in here.
}