# elf_loader

Espressif `elf_loader` consumed by PocketMageOS as a PlatformIO library.

## Upstream

- Fork: <https://github.com/TalismanDesign/esp-iot-solution>
- Upstream: <https://github.com/espressif/esp-iot-solution>
- Component: `components/elf_loader`
- Library facet version: 1.3.3
- Current upstream pin: `9e3ce01ba256603617b5746df2b5ddb63e600377`, in the sibling
  `lib/esp-iot-solution` submodule. It includes
  [#841](https://github.com/espressif/esp-iot-solution/pull/841) (compiled symbol
  registry) and [#842](https://github.com/espressif/esp-iot-solution/pull/842)
  (`esp_elf_finalize()`).

This library compiles the upstream sources directly; PocketMage adds no loader code.

- Symbol lookup: the OS scoped resolver delegates to upstream
  `elf_find_sym_default()`, which searches the libc, ESP-IDF and customer tables
  listed in `g_esp_elf_static_symbol_registry`, then runtime registrations.
  The SDK firmware gate (`tools/pm/compiled_exports.py`) reads that same
  registry from the linked ELF.
- Registry retention: upstream's CMakeLists adds `-u g_esp_elf_static_symbol_registry`,
  which PlatformIO does not read. `platformio.ini` passes the same option so
  `--gc-sections` keeps the registry for the gate.
- Unload: `src/elf_runner.cpp` calls `esp_elf_finalize()` after audio stops, so
  fini hooks run against a resident image and can still register exit work. The
  order is entry, audio stop, finalize, app exit callbacks, audio stop, SD
  cleanup, `esp_elf_deinit()`, overlay unload.

## Updating

1. Select the reviewed upstream revision in `Code/PocketMageOS/lib/esp-iot-solution`.
2. Update this pin record with the submodule change.
3. Run OS host tests, build `PM_BETA`, verify generated bindings, and run the SDK
   exact export gate against the newly linked firmware and a rebuilt app.
