# ELFLoader

Espressif `elf_loader` vendored for PocketMageOS as a PlatformIO library.

## Source

- Upstream: <https://github.com/espressif/esp-iot-solution>
- Component: `components/elf_loader`
- Version: 1.3.3 (IDF component registry `espressif/elf_loader`)
- Pin: commit `5d75f3f0dc499d9ed4b69284a3741187c2b75a70`
- License: Apache-2.0 (SPDX headers retained in every source file)

PlatformIO cannot consume the upstream repo directly (a subdirectory of a git
monorepo, so no usable `library.json` at the root), so the component sources
are mirrored here. To re-vendor at a newer release, diff against the upstream
commit above and bump the pin + version in `library.json`.

## Kconfig -> build defines

The component's Kconfig options are not available under Arduino, so the loader
is built with these explicit defines (see `library.json`):

| Kconfig                              | Value      | Note                        |
| ------------------------------------ | ---------- | --------------------------- |
| CONFIG_ELF_LOADER                    | 1          | component enable            |
| CONFIG_ELF_LOADER_BUS_ADDRESS_MIRROR | 1          | default for esp32s3         |
| CONFIG_ELF_LOADER_LIBC_SYMBOLS       | 1          | libc export table           |
| CONFIG_ELF_LOADER_ESPIDF_SYMBOLS     | 1          | esp-idf export table        |
| CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS   | 1          | pocketmage API export table |
| CONFIG_ELF_LOADER_NUMBER_SYMBOLS     | 32         | registered tables cap       |
| CONFIG_ELF_FILE_SYSTEM_BASE_PATH     | "/storage" | LittleFS app store          |

Deliberately left undefined: `CONFIG_ELF_LOADER_LOAD_PSRAM`, `CONFIG_ELF_LOADER_SET_MMU`,
`CONFIG_ELF_LOADER_CACHE_OFFSET`, `CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT`.
No-PSRAM keeps app `.text` in internal IRAM (`.data` in internal 8-bit heap),
which both PM_PRODUCTION and PM_BETA support.
