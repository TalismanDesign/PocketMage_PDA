#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Resolve name through PocketMage's compiled table registry, then upstream.
 * Return zero for NULL or an unavailable name. Runtime registrations remain
 * available through upstream fallback but cannot be confirmed by the SDK gate.
 */
uintptr_t pm_elf_find_symbol(const char *name);

#ifdef __cplusplus
}
#endif
