---
title: "SDK consolidation validation"
description: "Build, export, lifecycle, and device-validation for the SDK and PocketMuse consolidation."
---

# SDK consolidation validation

## Automated checks

The OS native suite exercises serial-default and audio probe policy, exclusive
MSC ownership, transfer startup failures and sector bounds, boot/PIN paths,
PCM gain/timing, media tags, task-owned SD cleanup, app exit/destructor order,
resident ELF finalization, and overlay storage. HID tests cover resource
allocation and task-start failures, each retryable shutdown phase, outstanding
device drainage, worker acknowledgements, queued/held key reset, and reconnect.
SD access regressions cover foreign stream operations, snapshot read/count/
replacement/close, empty-snapshot ownership, serialized entry copies, owner
success, and directory opens with already-prefixed paths. The ownership and
snapshot-serialization cases reproduced failures before the fixes.
Playback regressions reproduced stale request IDs and results on rejection.
Invalid paths, initialization failure, pending shutdown, transfer mode, task
creation failure, and queue failure now receive fresh IDs with `FAILED` results.
Late decoder completion and queued commands cannot revive a rejected request;
accepted replacement playback still completes normally.
They use simulated hardware and do not establish electrical behavior.

Run the OS checks from `Code/PocketMageOS`:

```sh
pio run -e PM_BETA -j 2
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

Run the SDK checks from `Code/PocketMageOS/lib/PocketMage_SDK`:

```sh
python3 -m unittest discover -s tools/pm/tests -t .
python3 tools/gen_app_api.py --root . --host-root ../.. --verify
```

From the adjacent PocketMuse checkout, build with the matching SDK:

```sh
export SDK_ROOT="$(cd ../PocketMage_PDA/Code/PocketMageOS/lib/PocketMage_SDK && pwd)"
python3 tests/run.py
make SDK_ROOT="$SDK_ROOT" clean
make SDK_ROOT="$SDK_ROOT" elf
PYTHONPATH="$SDK_ROOT/tools" python3 -m pm --ci check . \
  --host-elf "$SDK_ROOT/../../.pio/build/PM_BETA/firmware.elf"
```

After changing the export surface, regenerate the customer table using the
[SDK symbol-generation procedure](https://talismandesign.github.io/PocketMage_SDK/docs/symbols/),
rebuild the firmware, and rerun the exact gate. A global in the ELF symbol table
is not sufficient: only the enabled compiled loader tables count, in runtime
first-match order. Runtime registrations cannot be confirmed offline.
The PocketMuse ELF strips `app_main` from its shipped symbol table; the gate
reports the expected notice and trusts the entry pinned by `-e app_main`.

The full firmware build reports dependency warnings in LibSSH's deprecated SCP
APIs, upstream usb_stream's packed descriptors, and the existing deprecated
SDMMC unmount call in `pocketmage_sd.cpp`. The documentation build
also warns that repository inference sees the PDA parent remote when building
the SDK site as a submodule; its configured SDK publication URL is intentional.
