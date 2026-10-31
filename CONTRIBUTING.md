# Contributing to PocketMage

Thanks for your interest in contributing! This guide covers everything you need to get started.

---

## Table of contents

1. [Getting the code](#getting-the-code)
2. [Building the firmware](#building-the-firmware)
3. [Testing without hardware](#testing-without-hardware)
4. [Coding conventions](#coding-conventions)
5. [Submitting changes](#submitting-changes)
6. [Third-party apps (Bazaar)](#third-party-apps-bazaar)
7. [AI-assisted contributions](#ai-assisted-contributions)
8. [Community](#community)

---

## Getting the code

```bash
git clone https://github.com/TalismanDesign/PocketMage_PDA.git
```

Main firmware source lives under `Code/PocketMageOS/`. Hardware design files (KiCad schematics/PCBs) are under `Resources/PCB/`.

---

## Building the firmware

PocketMage uses [PlatformIO](https://platformio.org/). From `Code/PocketMageOS/`:

```bash
pio run -e PM_PRODUCTION   # Production hardware (N16R2, Quad PSRAM)
pio run -e PM_BETA         # Beta hardware (N16R8, PSRAM disabled)
```

Third-party apps are built against the PocketMage SDK into a `.app.elf` (see `Docs/docs/guides/elf-apps.md`).

CI (`build-firmware.yml`) builds `PM_PRODUCTION` and `PM_BETA` on every push. Run the same commands locally before opening a PR.

**Minimum requirements:** PlatformIO Core (via `pip install platformio`), ESP32 platform support (installed automatically on first build).

---

## Testing without hardware

There is currently **no automated test suite**. `src/readme.md` references a `pio test -e native` environment, but no `[env:native]` exists in `platformio.ini`. Treat that reference as aspirational, not a real capability, until someone adds it.

Manual verification against real hardware (or a build log review, for changes with no runtime effect) is the only current path. Say what you tested in your PR description.

---

## Coding conventions

- **Language:** C++ (Arduino framework, ESP32-S3 target).
- **Pin/peripheral config:** centralised in `include/config.h`. Add new pin definitions there, not scattered across source files.
- **App dispatch:** built-in apps are wired into a `switch (CurrentAppState)` in `PocketMageV3.cpp` (`applicationEinkHandler()`/`processKB()`), plus the `AppState` enum in `globals.h`. Follow the existing pattern (see `// ADD APP CASES HERE`) when adding a new built-in app.
- **Shared state:** `CurrentAppState` and similar globals are read from one FreeRTOS task (E-Ink handler, core 0) and written from another (`processKB()`, core 1) with no explicit synchronisation today. Be aware of this if your change touches app-state transitions.

---

## Submitting changes

1. **Open or comment on an issue first.** This avoids duplicated effort and lets the maintainer give early feedback on the approach.
2. **Check for open PRs** on the same topic before starting. Search by title as well as issue cross-references.
3. **Fork the repository** and create a branch from `main`.
4. **Keep PRs small and focused.** One concern per PR makes review much faster.
5. **Describe what and why** in the PR body. Include how you tested it (which hardware revision, if applicable).

---

## Third-party apps (Bazaar)

PocketMage loads third-party apps from the SD card at runtime through `APPLOADER.cpp`. Two app forms share the device:

- **Native apps** compile into the firmware and use the three-function stubs `APP_INIT`, `processKB_APP`, and `einkHandler_APP`. Start from `APP_TEMPLATE.cpp`.
- **ELF apps** build standalone against the [PocketMage SDK](https://talismandesign.github.io/PocketMage_SDK/docs/) into a `.app.elf`, pack with an overlay sidecar, and load from the SD card without a firmware reflash. See `Docs/docs/guides/elf-apps.md` and the [SDK docs](https://talismandesign.github.io/PocketMage_SDK/docs/).

Note: the ELF loader (`src/elf_runner.cpp`) resolves an app's symbols against the host at load time, but does not perform a runtime version handshake, so an app built against a different OS version loads without warning. The SDK's build-time gate (`pm check`, `PM_APP_API_ABI`, `pocketmage_sdk_version`) is where compatibility is enforced; an app that reads the SDK-version macros can compare against the host at startup. Bear this in mind if your app relies on specifics of `globals.h` or pin layout that may change between OS releases.

---

## AI-assisted contributions

AI tools are welcome for drafting code or documentation, but **every submission must be reviewed and understood by the person submitting it**. You should be able to explain how your own code works, not just that an assistant produced it. If a contribution was substantially generated or guided by an AI tool, please say so briefly in the PR description, for example: *"Drafted with an AI assistant, reviewed and tested manually."* This helps the maintainer understand the provenance of the code and focus review effort appropriately.

---

## Community

See the [docs site](https://talismandesign.github.io/PocketMage_PDA/docs/) for the FAQ, tutorials, and command manual.
