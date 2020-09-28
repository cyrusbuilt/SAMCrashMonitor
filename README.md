# SAMCrashMonitor
SAM-series compatible Crash Monitor library for Arduino compatible with SAM D21/D51 variants.

[![Build Status](https://travis-ci.com/cyrusbuilt/SAMCrashMonitor.svg?branch=master)](https://travis-ci.com/cyrusbuilt/SAMCrashMonitor)

## Description

Similar to [ArduinoCrashMonitor](https://github.com/cyrusbuilt/ArduinoCrashMonitor),
and [ESPCrashMonitor](https://github.com/cyrusbuilt/ESPCrashMonitor), This library takes advantage of the SAM D21/D51's watchdog capability to get the
cause of a reset in the event that the Arduino sketch
(firmware) becomes unresponsive due to a crash (divide by zero, WiFi fails to
initialize, main loop hangs, etc). The general idea behind this is: during each iteration of loop(), CrashMonitor's iAmAlive() method should be called to signal to
CrashMonitor that the sketch is still running. If CrashMonitor does not
receive the iAmAlive() signal before the end of the timeout, then the built-in
watchdog will reset store information about the crash and reset the device.
You can then use the dump() method to load the crash reports and dump them to
a Serial port. Most of the code in this version of the library was taken from [Adafruit's SleepyDog Library](https://github.com/adafruit/Adafruit_SleepyDog). Key differences between this library and Adafruit's is:
- 1) This library *only* supports the SAM D21/D51.
- 2) This library does not depend on [Adafruit ASFCore](https://github.com/adafruit/Adafruit_ASFcore), except when using the SAM D51 variant.
- 3) This library provides descriptions for reset cause codes and a means to dump both the code and description to the Serial port.
- 4) This library does not make use of a sleep() function.
- 5) This library also includes a hard fault handler for when the MCU crashes which will dump crash data to the Serial monitor just before reset.

Specifics regarding the SAM D-series Watchdog registers and reset codes can be found in the [datasheet](https://cdn.sparkfun.com/datasheets/Dev/Arduino/Boards/Atmel-42181-SAM-D21_Datasheet.pdf).

## How to install

For PlatformIO:
```bash
$ pio lib install SAMCrashMonitor
```

For Arduino IDE:

See <https://www.arduino.cc/en/Guide/Libraries>

## How to use

After installing this library, include SAMCrashMonitor.h in your sketch and then
see the example below:

```cpp
#include <Arduino.h>
#include "SAMCrashMonitor.h"

bool firstLoopDone = false;

void setup() {
    SerialUSB.begin(9600);
    while (!SerialUSB);

    SAMCrashMonitor::disableWatchdog(); // Make sure it is turned off during init.
    SAMCrashMonitor::dump();            // Dump watchdog reset data to the console.

    // Turn the watchdog on and get actual timeout value based on the provided one.
    int timeout = SAMCrashMonitor::enableWatchdog(2000);
    SerialUSB.print(F("Watchdog enabled for: "));
    SerialUSB.print(timeout);
    SerialUSB.println(F(" ms"));

    // Also if a hard fault occurs in your sketch (such as divide-by-zero),
    // then crash data (including the address of the instruction that caused
    // the crash) will be dumped to the serial monitor automatically *PRIOR*
    // to reset.
}

void loop() {
    if (!firstLoopDone) {
        // On the first loop, tell the crash monitor we are alive so we get the full timeout.
        SAMCrashMonitor::iAmAlive();
        firstLoopDone = true;
    }
    else {
        // We completed the first loop. Now trigger a crash.
        // Following the crash, the device will reset and dump the crash data to console.
        while(true) {
            ;
        }
    }
}
```

## Generating API documentation
To generate and view the API documentation run the following command from the root project folder:

```bash
make docs
```

The above command assumes you have [doxygen](http://www.doxygen.nl/) installed. The documentation will be compiled to the docs/ directory within the project structure in HTML format.