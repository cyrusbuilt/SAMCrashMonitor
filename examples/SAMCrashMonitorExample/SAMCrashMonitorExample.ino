#include <Arduino.h>
#include "SAMCrashMonitor.h"

void setup() {
    Serial.begin(9600);
    Serial.println();
    Serial.print(F("Previous reset cause: "));
    Serial.println(SAMCrashMonitor.getResetCause());
    Serial.print(F("Previous reset description: "));
    Serial.println(SAMCrashMonitor.getResetDescription());
    Serial.println(F("Enabling watchdog."));
    int timeout = SAMCrashMonitor.enableWatchdog(4000);
    Serial.print(F("Watchdog enabled for "));
    Serial.print(timeout);
    Serial.println(" ms.");
    
    Serial.println();
    Serial.println(F("First test: Looping once per second for 5 seconds while feeding watchdog..."));
    for (int i = 1; i <= 5; i++) {
        Serial.print(F("Loop #"));
        Serial.println(i);
        delay(1000);
        SAMCrashMonitor.iAmAlive();
    }

    Serial.println();
    Serial.println(F("Disabling watchdog..."));
    Serial.println();
    SAMCrashMonitor.disableWatchdog();

    Serial.println(F("Second test: Exceed timeout and reset."));
    timeout = SAMCrashMonitor.enableWatchdog(4000);
    Serial.print(F("Watchdog will reset controller in "));
    Serial.print(timeout);
    Serial.println(" ms!");
    delay(timeout + 1000);

    // We shouldn't get this far since the watchdog should reset the MCU.
}

void loop() {
    // Normally, you would place a call to SAMCrashMonitor.iAmAlive() in here.
}