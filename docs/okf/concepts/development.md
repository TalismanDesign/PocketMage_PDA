---
type: concept
title: PocketMageOS
description: "The PocketMage operating system and app runtime."
source: "https://talismandesign.github.io/PocketMage_PDA/docs/development/"
path: /development/
updated: 2026-11-01
okf:
  generated_by: "@docmd/plugin-okf"
  generated_at: "2026-11-01T04:07:11.650Z"
---
---
title: "PocketMageOS"
description: "The PocketMage operating system and app runtime."
---

# PocketMageOS

PocketMageOS is the operating system layer of the firmware. It boots the device, starts and switches apps, routes keyboard input, dispatches screen updates, and tracks the system state that every app depends on. This page explains how that runtime works and where the code lives.

::: grids
::: grid
::: card "Boot and route" icon:play
Starts the right app, routes keyboard input, and keeps screen updates on the right handler.
:::
:::
::: grid
::: card "Manage state" icon:settings
Tracks timeout, sleep, lock, and the active app.
:::
:::
::: grid
::: card "Bridge hardware" icon:cpu
Coordinates the shared hardware layer, storage, and device helpers.
:::
:::
::: grid
::: card "Support apps" icon:box
Native apps and ELF apps plug into the same runtime model.
:::
:::
:::

## The app runtime model

Every PocketMage app is a set of three functions. The OS calls them at the right time; the app never drives the loop itself.

1. **`*_INIT()`** - runs once when the app is entered. Sets app state and prepares the screen.
2. **`processKB_*()`** - runs on every keypress. The app reads the key, updates its state, and queues a redraw.
3. **`einkHandler_*()`** - runs when the screen needs updating. The app draws the current state to the E-Ink buffer.

The dispatch loop in `PocketMageV3.cpp` owns the switch between these handlers. It is a `switch (CurrentAppState)` that routes each keystroke and each E-Ink refresh to the active app. Adding an app means adding a case to that switch and an entry to the `AppState` enum in `globals.h`. The spots are marked `// ADD APP CASES HERE`.

## How boot works

On power-up, PocketMageOS mounts the SD card, initializes the displays and keyboard, and loads persistent settings from NVS. It then checks which app to enter. This depends on how the device was woken:

- **Normal boot** - enters the home app (unless the `HOME_ON_BOOT` setting is off).
- **Woken from sleep with a shortcut key** - enters the app mapped to that key. For example, pressing `N` while sleeping opens TXT. The mapping is `bootShortcutApp()` in `UTILS.cpp`.
- **Woken from now-later** - returns to the app that was active before sleep, restoring its saved state.

## State the OS tracks

The OS keeps shared runtime state in `globals.h`. The pieces you will meet most often:

- `CurrentAppState` - which app is active. This drives the dispatch switch.
- `CurrentHOMEState` - the home screen mode (`HOME_HOME` or `NOWLATER`).
- `newState` - a flag that tells the loop to re-run `*_INIT()` for the target app.
- `disableTimeout` - a global lockout so an app can prevent idle sleep while it is busy.
- `prefs` - persistent preferences stored in NVS.
- `global_fs` - the active filesystem handle.

The timeout system is also part of the OS. After `IDLE_TIME` without input, the OS shows the mage idle state; after `TIMEOUT` seconds it sleeps. Apps can reset the idle clock with `resetIdle()` or block it entirely with `disableTimeout`.

## How apps reach the hardware

Apps do not talk to hardware directly. They call the PocketMage Library, which owns the physical devices. `OLED().sysMessage(...)` flashes a message, `EINK().refresh()` pushes the E-Ink buffer, `KB()` handles key state, and `SD()` handles storage. The library is the only layer that knows the pins. This is what lets the same app code build for native, ELF, and future hardware revisions.

See [PocketMage Library](../reference/pocketmage-library.md) for the full module map.

## Native vs ELF apps

- **Native apps** live in `src/OS_APPS/`, are compiled into the firmware, and are wired into the dispatch switch.
- **ELF apps** are compiled separately against the PocketMage SDK into a `.app.elf`, packed with an overlay sidecar, and loaded from the SD card at runtime through the app loader. No firmware reflash is involved. The runner (`src/elf_runner.cpp`) relocates the base image with `esp_elf`, then pages the app's code into executable RAM through the `elf_overlay` runtime, and runs the app's entry in its own task while the OS loop stands down.

Both follow the same three-function shape. The difference is where the code ships and how it is loaded. See [ELF Apps](../guides/elf-apps.md) for the device side, [Making Apps](../guides/making-apps.md) for the workflow, and the [SDK docs](https://talismandesign.github.io/PocketMage_SDK/docs/) for the binary contract.

## Source layout

- `Code/PocketMageOS/src/PocketMageV3.cpp` - main loop and app dispatch
- `Code/PocketMageOS/src/OS_APPS/` - native app code, one file per app
- `Code/PocketMageOS/src/UTILS.cpp` - shared prompts, boot shortcuts, and runtime helpers
- `Code/PocketMageOS/include/globals.h` - app prototypes, app state, shared runtime state
- `Code/PocketMageOS/include/config.h` - pins and system settings
- `Code/PocketMageOS/lib/PocketMage_SDK/` - the reusable hardware layer (the PocketMage SDK, vendored as a submodule)
- `Code/PocketMageOS/lib/elf_overlay/` - the overlay runtime that pages ELF-app code into executable RAM
- `Code/PocketMageOS/src/elf_runner.cpp` - loads and runs installed ELF apps

## Next steps

::: grids
::: grid
::: button "Architecture" ./architecture.md icon:layers
:::
::: grid
::: button "Build Environments" ./build-environments.md icon:settings
:::
::: grid
::: button "Making Apps" ../guides/making-apps.md icon:code
:::
::: grid
::: button "Native Apps" ../guides/native-apps.md icon:terminal
:::
::: grid
::: button "App API Reference" ../reference/app-api.md icon:book
:::
:::
