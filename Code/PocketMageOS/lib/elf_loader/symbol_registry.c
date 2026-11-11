/*
 * Compile the upstream source unchanged in this translation unit so its private
 * tables can be referenced without copying them or patching the vendor tree.
 * library.json excludes the standalone compilation of that source.
 */
#include "../esp-iot-solution/components/elf_loader/src/esp_elf_symbol.c"
#include "symbol_registry.h"

#ifdef CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS
extern const struct esp_elfsym g_customer_elfsyms[];
#endif

/** Enabled tables shared by the PocketMage resolver and the SDK firmware gate. */
const struct esp_elfsym *const g_pm_elf_static_tables[] = {
#ifdef CONFIG_ELF_LOADER_LIBC_SYMBOLS
    g_esp_libc_elfsyms,
#endif
#ifdef CONFIG_ELF_LOADER_ESPIDF_SYMBOLS
    g_esp_espidf_elfsyms,
#endif
#ifdef CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS
    g_customer_elfsyms,
#endif
    NULL
};

uintptr_t pm_elf_find_symbol(const char *name)
{
    if (!name) {
        return 0;
    }
    for (size_t index = 0; g_pm_elf_static_tables[index]; ++index) {
        const struct esp_elfsym *symbol = g_pm_elf_static_tables[index];
        for (; symbol->name; ++symbol) {
            if (!strcmp(symbol->name, name)) {
                return (uintptr_t)symbol->sym;
            }
        }
    }
    return elf_find_sym_default(name);
}
