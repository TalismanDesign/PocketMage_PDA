---
title: "Build Environments & PlatformIO Setup"
description: "How to set up PlatformIO and build PocketMage firmware on Linux, macOS, and Windows."
---

# Build Environments & PlatformIO Setup

PocketMage firmware is built with [PlatformIO](https://platformio.org/). The build configuration lives in `Code/PocketMageOS/platformio.ini` and defines two environments that produce the firmware variants. Third-party apps are not built in this tree; they are compiled against the PocketMage SDK (see [ELF Apps](../guides/elf-apps.md)).

## The two build environments

| Environment | Target | Purpose |
| --- | --- | --- |
| `PM_PRODUCTION` | Production hardware (N16R2, Quad PSRAM enabled) | The stable OS image flashed to production devices |
| `PM_BETA` | Beta hardware (N16R8, PSRAM forced disabled) | A test build with the latest changes |

The two environments share a common `[common]` section and differ in hardware target and version string.

Running `pio run` with no arguments builds both defaults (`PM_PRODUCTION` and `PM_BETA`). Build one explicitly with `pio run -e <name>`.

## Prerequisites

- [VS Code](https://code.visualstudio.com/) installed
- Python 3 installed
- PlatformIO Core (installed automatically by the VS Code extension, or via `pip install platformio`)
- The PocketMage source code, cloned from [GitHub](https://github.com/TalismanDesign/PocketMage_PDA)

The ESP32 platform support downloads automatically on the first build, so the first build is noticeably slower.

## Setup

### Linux

1. Install `python3-venv` if it is missing:

   ```bash
   sudo apt install python3-venv
   ```

2. Install the [PlatformIO IDE](https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode) extension in VS Code.
3. Open the PocketMage source folder in VS Code.
4. When PlatformIO asks for a project folder, choose `Code/PocketMageOS/`.
5. Build from the PlatformIO toolbar or the command palette.

### macOS

Use the same steps as Linux. No extra setup is required.

### Windows

1. Install [Git](https://git-scm.com/) to clone the repository.
2. Follow the same PlatformIO IDE steps as Linux.
3. Build from the PlatformIO toolbar in VS Code.

## Command line

From `Code/PocketMageOS/`:

```bash
pio run -e PM_PRODUCTION   # production firmware
pio run -e PM_BETA         # beta firmware
```

## ELF apps

Third-party apps are not built with this environment set. They are compiled against the PocketMage SDK into `.app.elf` files, packaged with an icon as a `.tar`, and installed through the app loader. See [ELF Apps](../guides/elf-apps.md) and the [app loader section](../command-manual/index.md) of the command manual.

## Testing

There is no automated test suite for the firmware. Verify changes against real hardware, or review the build log for changes with no runtime effect. CI builds `PM_PRODUCTION` and `PM_BETA` on every push, so a clean build locally is the minimum bar before opening a pull request.

## Next steps

::: grids
::: grid
::: button "PocketMageOS overview" ./index.md icon:cpu
:::
::: grid
::: button "Making Apps" ../guides/making-apps.md icon:code
:::
::: grid
::: button "ELF Apps" ../guides/elf-apps.md icon:package
:::
::: grid
::: button "Firmware FAQ" ../faq/index.md icon:help-circle
:::
:::
