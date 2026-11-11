# elf_loader

Espressif `elf_loader` consumed by PocketMageOS as a PlatformIO library.

## Upstream

- Fork: <https://github.com/TalismanDesign/esp-iot-solution>
- Upstream: <https://github.com/espressif/esp-iot-solution>
- Component: `components/elf_loader`
- Library facet version: 1.3.3
- Current upstream pin: `90fcf2170c3ea1c7f8a19b49d58a8557781e5457`, in the sibling
  `lib/esp-iot-solution` submodule.

`build.py` compiles the upstream component in this PlatformIO facet.
`symbol_registry.c` includes the upstream symbol source unchanged and exposes
its enabled static tables as `g_pm_elf_static_tables`. `pm_elf_find_symbol()`
searches them in libc, ESP-IDF, customer order before upstream fallback.
The SDK's exact firmware checker reads that same retained registry.

## Pending upstream APIs

- [#841](https://github.com/espressif/esp-iot-solution/pull/841): expose
  `g_esp_elf_static_symbol_registry` in resolver order. Once the pinned upstream
  includes it, remove the private-source inclusion, update the offline checker
  for the upstream registry layout, and verify disabled tables and first-match
  precedence against the final linked firmware.
- [#842](https://github.com/espressif/esp-iot-solution/pull/842): add
  `esp_elf_finalize()` before deallocation. Replace the OS runner's local
  `finalizeElf()` only when this API is available. Preserve resident finalizer,
  app callback, audio-stop, SD cleanup, overlay-unload, and ELF-deinit ordering.

## Updating

1. Select the reviewed upstream revision in `Code/PocketMageOS/lib/esp-iot-solution`.
2. Update this pin record with the submodule change.
3. Run OS host tests, build `PM_BETA`, verify generated bindings, and run the SDK
   exact export gate against the newly linked firmware and a rebuilt app.
