"""Exercise compiled registry selection and resolver order for every Kconfig combination."""

import ctypes
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class StaticResolver(unittest.TestCase):
    def test_enabled_tables_and_lookup_share_the_same_registry(self):
        source = (ROOT / "lib/elf_loader/symbol_registry.c").read_text()
        start = source.index("#ifdef CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS\nextern")
        preface = '''
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define ESP_LOGE(...) ((void)0)
struct esp_elfsym { const char *name; const void *sym; };
const struct esp_elfsym g_esp_libc_elfsyms[] = {
  {"shared", (void *)1}, {"libc", (void *)11}, {NULL, NULL}};
const struct esp_elfsym g_esp_espidf_elfsyms[] = {
  {"shared", (void *)2}, {"idf", (void *)22}, {NULL, NULL}};
const struct esp_elfsym g_customer_elfsyms[] = {
  {"shared", (void *)3}, {"customer", (void *)33}, {NULL, NULL}};
static uintptr_t elf_find_sym_default(const char *name) {
  return strcmp(name, "dynamic") == 0 ? 99 : 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "resolver.c"
            path.write_text(preface + source[start:])
            for flags in range(8):
                with self.subTest(flags=flags):
                    defines = [f"-DCONFIG_ELF_LOADER_{name}_SYMBOLS=1"
                               for bit, name in enumerate(("LIBC", "ESPIDF", "CUSTOMER"))
                               if flags & (1 << bit)]
                    library = Path(tmp) / f"resolver-{flags}.so"
                    kind = "-dynamiclib" if sys.platform == "darwin" else "-shared"
                    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fPIC",
                                    kind, *defines, str(path), "-o", str(library)], check=True)
                    loaded = ctypes.CDLL(str(library))
                    resolve = loaded.pm_elf_find_symbol
                    resolve.argtypes = [ctypes.c_char_p]
                    resolve.restype = ctypes.c_size_t
                    self.assertEqual(resolve(None), 0)
                    self.assertEqual(resolve(b"missing"), 0)
                    self.assertEqual(resolve(b"dynamic"), 99)
                    enabled = [bit + 1 for bit in range(3) if flags & (1 << bit)]
                    self.assertEqual(resolve(b"shared"), enabled[0] if enabled else 0)
                    for bit, (name, address) in enumerate(((b"libc", 11), (b"idf", 22),
                                                         (b"customer", 33))):
                        self.assertEqual(resolve(name), address if flags & (1 << bit) else 0)
