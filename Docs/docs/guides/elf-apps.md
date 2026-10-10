---
title: "ELF Apps"
description: "How third-party PocketMage apps are built and loaded."
---

# ELF Apps

Third-party apps are compiled outside the OS into a `.app.elf`, packed in a `.tar`, and installed at runtime through the app loader. No firmware reflash is involved. This page explains how an app is built, installed, and what it can rely on at runtime.

## What an ELF app is

An ELF app is a standalone program linked against the PocketMage SDK with `PM_TARGET_APP`. The loader reads the `.app.elf` from the SD card into RAM, relocates it with `esp_elf`, and runs its `main` entry in its own task (`src/elf_runner.cpp`). While the app runs, the OS loop and the E-Ink task stand down.

Exiting is just `return` from `main`. There is no reboot needed: the loader unloads the image and the OS goes back to HOME itself.

## Building an app

Build against the SDK, not against the OS tree. The SDK ships a working example in `Code/PocketMageOS/lib/PocketMage_SDK/examples/hello_app`, built through `tools/app.mk`. `pm check` (from the SDK) validates the result before you package it. The SDK docs under `lib/PocketMage_SDK/docs` cover the app ABI and the version-stamp contract. `Code/PocketMageOS/src/APP_TEMPLATE.cpp` shows the `PM_TARGET_APP` entry hooks for the compiled-into-OS scaffolding.

## Packaging and installing

1. Build the app into a `.app.elf`.
2. Package it with a `<name>_ICON.bin` icon in a `.tar` (gzip is fine).
3. Copy the `.tar` onto the SD card, typically over USB.
4. Open the app loader on the device and use the swap action to install it into one of the four slots.

Installed apps live in `/apps/slot1` through `/apps/slot4` on the SD card and are tracked in NVS under `ELFINFO1` through `ELFINFO4`. Launch an installed app from the home screen by typing `A`, `B`, `C`, or `D`, or from the loader.

## What an ELF app should assume

- It runs without the native PocketMageOS app set. Do not call functions from `src/OS_APPS/`.
- Keep the app self-contained and link against the SDK only.
- Use the PocketMage library for hardware access, exactly like native apps.
- The host exports the curated symbol set (see `Code/PocketMageOS/src/ELF_SYMS/esp_all_symbol.cpp`), including `pocketmage_sdk_version` for version checks. Any symbol outside that set must be imported into the SDK before an app can link against it.

## Read next

- [Making Apps](making-apps.md)
- [PocketMage Library](../reference/pocketmage-library.md)
- [App API Reference](../reference/app-api.md)
- [Build Environments](../development/build-environments.md)
