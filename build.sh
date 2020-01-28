#!/bin/sh

# Get script dir.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

# Build targets.
export PATH=~/.platformio/penv/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:~/n/bin:$PATH

platformio ci --lib="." --verbose --board=mkrnb1500 examples/SAMCrashMonitorExample/SAMCrashMonitorExample.ino

exit $?