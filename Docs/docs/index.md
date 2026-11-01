---
title: "PocketMage Documentation"
description: "Official documentation for the PocketMage open-source PDA device."
---

# PocketMage Documentation

PocketMageOS docs for the clamshell PDA: the operating system, the device hardware, the built-in apps and commands, and how native and ELF apps run on top. The app-facing build contract lives in the separate [PocketMage SDK docs](https://talismandesign.github.io/PocketMage_SDK/docs).

::: tag "PocketMageOS"
::: tag "Hardware"
::: tag "Apps"
::: tag "Commands"

## What is here

::: grids
::: grid
::: card "Getting Started" icon:rocket
Learn what PocketMage is, how to build it, and where to begin.

::: button "Open" ./getting-started/index.md icon:arrow-right
:::
:::
::: grid
::: card "PocketMageOS" icon:cpu
The operating system, app runtime, and navigation flow.

::: button "Open" ./development/index.md icon:arrow-right
:::
:::
::: grid
::: card "Architecture" icon:layers
Boot, dispatch, the two-display model, storage, and how apps run, end to end.

::: button "Open" ./development/architecture.md icon:arrow-right
:::
:::
::: grid
::: card "Guides" icon:book-open
How to make apps, native apps, ELF apps, and scripts.

::: button "Open" ./guides/index.md icon:arrow-right
:::
:::
::: grid
::: card "Reference" icon:code
The app API, PocketMage Library, and system state.

::: button "Open" ./reference/index.md icon:arrow-right
:::
:::
::: grid
::: card "Commands" icon:keyboard
Keymaps and built-in commands for every app.

::: button "Open" ./command-manual/index.md icon:arrow-right
:::
:::
::: grid
::: card "Features" icon:star
What the device does today and what is still planned.

::: button "Open" ./features/index.md icon:arrow-right
:::
:::
::: grid
::: card "SDK docs" icon:boxes
Build ELF apps against the SDK: the toolchain, the full component API, and the binary contract.

::: button "Open" https://talismandesign.github.io/PocketMage_SDK/docs icon:arrow-right
:::
:::
:::

## Quick path

::: steps

1. Read [What is PocketMage](getting-started/what-is-pocketmage.md).
2. Understand the system in [Architecture](development/architecture.md).
3. Set up the firmware build in [Build Environments](development/build-environments.md).
4. Learn the app model in [Making Apps](guides/making-apps.md); for standalone ELF apps, follow the [SDK docs](https://talismandesign.github.io/PocketMage_SDK/docs).
5. Use the [PocketMage Library](reference/pocketmage-library.md) when you need the hardware wrapper.
6. Keep the [Command Manual](command-manual/index.md) open while testing apps.

:::

## Quick links

- [FAQ](faq/index.md)
- [Scripting](scripting/index.md)
- [GitHub Repository](https://github.com/TalismanDesign/PocketMage_PDA)
- [Discord Community](https://discord.gg/KSCapSf4XH)
