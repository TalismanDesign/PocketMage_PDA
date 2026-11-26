---
title: "Crash logs"
description: "Capture a panic across reboot, retrieve its SD report, and decode firmware addresses."
---

# Crash logs

When PocketMageOS crashes, it keeps a short record of the crash in memory that
survives a reboot. After the device starts again, it saves that record to the SD
card as a text report. Both firmware environments have this turned on.

## Retrieve a report

1. Flash the firmware and keep its exact `firmware.elf` and `firmware.bin`
   together. Rebuilding can change code addresses.
2. Reproduce the failure with a writable SD card installed.
3. Let the device reboot and finish starting up without removing power.
4. Copy `/logs/crash/crash-*.txt` from the card, either through USB file transfer or
   by reading the card on a computer.

The filename is `crash-<boot nonce>-<record CRC>.txt`. The serial log's `CRASH` tag
prints `Saved /logs/crash/...` when the report is written. If saving fails, the report
stays pending and the OS tries again every five seconds. If no card was mounted at
boot, insert one and restart without removing power; inserting a card alone does
not mount it.

You can also read the report over serial. Unplug any USB accessory, connect a
computer and open the serial monitor at 115200 baud. The report is sent each time
a USB serial connection is detected, so reconnect the cable if you opened the
monitor after the device was already connected.

## What is captured

- The panic reason, and the message from `abort()` or a failed assertion.
- The core, the current task name, the fault PC and up to 60 backtrace PCs.
- The Xtensa registers (A0-A15, PS, SAR, EXCCAUSE, EXCVADDR, LBEG, LEND, LCOUNT)
  when an exception frame is available.
- The firmware's ELF SHA256 and the uptime in milliseconds.
- A snapshot of the OS taken by the main loop shortly before the crash: the
  active app, whether an ELF app was running, which feature owned the USB port,
  the free, largest and lowest-ever internal heap, and the number of tasks. The
  report shows how old the snapshot is.
- The last 1 KB of SDK log lines, under `Recent log:`. These come from the
  logger's memory, so they are there even when logging to a file is off. See
  [Logging](https://talismandesign.github.io/PocketMage_SDK/docs/api/log/)
  for the line format.

## Decode firmware addresses

Use the **exact ELF from the flashed build**. Check that its SHA256 matches the
report's `Firmware ELF SHA256` line:

```sh
shasum -a 256 firmware.elf
```

From the directory containing that ELF, run:

```sh
"$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32s3-elf-addr2line" \
  -aipfC -e firmware.elf
```

Paste the addresses from the report's `Backtrace PCs:` line, then press `Ctrl-D`.
The tool prints the function and source line for each address. If the backtrace is
corrupt, the report's `PC` line is still useful. `EXCVADDR` is the address that was
accessed, not a code address.

The firmware ELF only covers the OS, the SDK and the USB driver. Addresses inside
an ELF app need that app's own build files, so keep them with the report.

## How it works

`src/diagnostics/crash_log.cpp` hooks Arduino's panic handler before
`PocketMage_INIT()` runs, using the `-Wl,--wrap=esp_panic_handler` linker flag. At
crash time it only copies data into RTC memory that survives a reboot. It does not
touch the SD card, serial or the heap.

After the next boot, `pm_crash::poll()` writes the report to a temporary file,
reads it back to check it, and renames it to `.txt`. A report is never written
over an existing file. Saving waits while the card is in USB file transfer mode.

Only the latest crash is kept in memory; saved reports pile up on the card. A
power loss, a brownout or a device that hangs without crashing leaves no report.
This is a crash summary, not a full core dump.

## Build and test

From `Code/PocketMageOS`:

```sh
pio run -e PM_BETA -j 2
python3 -m unittest discover -s tests/host -p test_crash_log.py
```

Use `PM_PRODUCTION` for production hardware. The tests need a C++17 compiler with
AddressSanitizer and UndefinedBehaviorSanitizer. They run the real capture and
report code against simulated hardware, so check a real crash report to confirm
behavior on the device.

Upstream references:
[fatal errors](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/fatal-errors.html),
[RTC memory](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/memory-types.html),
[FatFs rename](https://elm-chan.org/fsw/ff/doc/rename.html).
