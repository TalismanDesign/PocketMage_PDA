# arduino_tinyusb

The Arduino core's TinyUSB component, used only when pioarduino rebuilds the
ESP-IDF libraries for `custom_sdkconfig` in `platformio.ini`. That rebuild uses
the plain `framework-espidf` package, which has no TinyUSB, so without this
folder USB mass storage disappears from the firmware.

- `CMakeLists.txt`, `Kconfig.projbuild` and `include/tusb_config.h` come from
  [esp32-arduino-lib-builder](https://github.com/espressif/esp32-arduino-lib-builder/tree/6671d0b/components/arduino_tinyusb)
  at `6671d0b`, the commit listed in the libs package's `versions.txt`.
- `tinyusb/` is a submodule of [hathach/tinyusb](https://github.com/hathach/tinyusb)
  pinned to `88c7f0b8`, the commit the stock libs were built from.

Local changes:

- `TINYUSB_HOST_ENABLED` switches off the TinyUSB host stack. Upstream always
  builds it in.
- The `arduino` and `main` private requirements are dropped, since neither
  component exists in the hybrid build.

When the platform moves to a new Arduino release, update both pins to the
commits in the new `framework-arduinoespressif32-libs/esp32s3/versions.txt`.
