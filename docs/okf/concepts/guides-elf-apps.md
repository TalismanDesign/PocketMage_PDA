---
type: guide
title: "ELF Apps"
description: "How third-party PocketMage apps are built, installed, and loaded by the OS at runtime."
source: "https://talismandesign.github.io/PocketMage_PDA/docs/guides/elf-apps/"
path: /guides/elf-apps/
updated: 2026-11-01
okf:
  generated_by: "@docmd/plugin-okf"
  generated_at: "2026-11-01T04:07:11.651Z"
---
---
title: "ELF Apps"
description: "How third-party PocketMage apps are built, installed, and loaded by the OS at runtime."
---

# ELF Apps

Third-party apps are compiled outside the OS into a `.app.elf`, packed into a `.tar` with an overlay sidecar and an icon, and installed at runtime through the app loader. No firmware reflash is involved. This page covers the app from the OS side: how the loader runs it, where installed apps live, and what an app can rely on. The binary contract itself, the build tooling, and the full symbol surface are owned by the SDK documentation and are not duplicated here.

For building and shipping an app, start with the SDK docs:

- [App ABI](https://talismandesign.github.io/PocketMage_SDK/docs/app-abi/) - the binary contract
- [Overlay format](https://talismandesign.github.io/PocketMage_SDK/docs/overlay/) - the `.elfo` sidecar
- [Building apps](https://talismandesign.github.io/PocketMage_SDK/docs/build/) - the toolchain and `make` targets
- [Symbols](https://talismandesign.github.io/PocketMage_SDK/docs/symbols/) - the host export surface

## What an ELF app is

An ELF app is a standalone 32-bit `ET_DYN` program linked against the PocketMage SDK with `PM_TARGET_APP`. It links `-nostdlib`, so it ships no runtime of its own; everything it references, from `malloc` to the `pm_*` API, resolves from the host at load time.

The runner (`src/elf_runner.cpp`) loads an installed app in two stages. It reads the `.app.elf` from the SD card and relocates the base image with `esp_elf`, then loads the matching `.elfo` overlay sidecar, which pages the app's code into executable RAM on demand and patches ISA-specific relocations. The entry (`app_main`) then runs in its own 16 KB task on core 1. While the app runs, the OS loop and the E-Ink task stand down and the app owns the UI through the host singletons.

Exiting is a `return` from `app_main`. No reboot is needed: the runner unloads the overlay, deinitializes the base image, and reopens HOME.

## Packaging and installing

1. Build the app into `<name>.app.elf` and its `<name>.elfo` overlay sidecar.
2. Pack them with a 40x40 `<name>_ICON.bin` and an optional `app.properties` manifest into a plain `.tar` (`make pack` does this).
3. Copy the `.tar` onto the SD card, typically over USB.
4. Open the app loader on the device and use the swap action to install it into one of the four slots.

Installed apps live in `/apps/slot1` through `/apps/slot4` on the SD card and are tracked in NVS under `ELFINFO1` through `ELFINFO4`. Launch an installed app from the home screen by typing `A`, `B`, `C`, or `D`, or from the loader.

## The manifest (`app.properties`)

The loader reads an optional `app.properties` from the app's slot directory (up to 1024 bytes). Recognized fields are `name`, `version`, `author`, and `scope`. The first three become the app's display metadata and are passed to `app_main` as `argv`. `scope` (default `all`) restricts which `pm_*` symbols the app may resolve: anything outside its scope fails to resolve, so a mis-scoped app is rejected at load rather than reaching an API it did not declare. The runtime baseline (libc, libstdc++, ESP-IDF) stays available at any scope.

## What an ELF app should assume

- It runs without the native PocketMageOS app set. Do not call functions from `src/OS_APPS/`.
- Keep the app self-contained and link against the SDK only.
- Use the PocketMage library for hardware access, exactly like native apps.
- The host exports a curated symbol set (generated into `src/ELF_SYMS/esp_all_symbol.cpp`), including `pocketmage_sdk_version` for version checks. A symbol outside that set must be added to the SDK surface before an app can resolve it.

## Read next

- [Making Apps](making-apps.md)
- [PocketMage Library](../reference/pocketmage-library.md)
- [App API Reference](../reference/app-api.md)
- [Build Environments](../development/build-environments.md)
