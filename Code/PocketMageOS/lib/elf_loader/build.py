"""Compile the PocketMage registry with the loader facet's existing flags.

See https://docs.platformio.org/en/latest/scripting/examples/external_sources.html
"""

Import("env")

env.BuildSources(
    "$BUILD_DIR/pocketmage-elf",
    "$PROJECT_DIR/lib/elf_loader",
    src_filter="+<symbol_registry.c>",
)
