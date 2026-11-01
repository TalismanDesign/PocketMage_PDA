# elf_loader

Espressif `elf_loader` consumed by PocketMageOS as a PlatformIO library.

## Upstream

- Fork: <https://github.com/TalismanDesign/esp-iot-solution>
- Upstream: <https://github.com/espressif/esp-iot-solution>
- Component: `components/elf_loader`
- Version: 1.3.3
- Pin: commit `5d75f3f0dc499d9ed4b69284a3741187c2b75a70`, tracked by branch
  `pocketmage-pin-1.3.3` in the fork so the submodule can be a shallow clone.

The submodule is pinned to the exact commit; the branch only exists so
`git submodule update` can fetch that commit shallowly.

## Updating

1. Move the pin branch in the fork to the desired upstream commit.
2. `git -C Code/PocketMageOS/lib/elf_loader/esp-iot-solution checkout <sha>`
3. Record the new SHA here and in the commit that bumps the submodule.
