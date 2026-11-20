import ctypes
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
COMPONENT = ROOT / "lib/esp-iot-solution/components/elf_loader"
FACET = ROOT / "lib/elf_loader/library.json"

# Distinct sentinel targets per table so precedence is observable.
LIBC_TARGET, IDF_TARGET, CUSTOMER_TARGET, RUNTIME_TARGET = 11, 22, 33, 99

TABLES = f'''
#include "esp_elf.h"
const struct esp_elfsym g_esp_libc_elfsyms[] = {{
  {{"shared", (void *){LIBC_TARGET}}}, {{"libc_only", (void *){LIBC_TARGET}}}, ESP_ELFSYM_END}};
const struct esp_elfsym g_esp_espidf_elfsyms[] = {{
  {{"shared", (void *){IDF_TARGET}}}, {{"idf_only", (void *){IDF_TARGET}}}, ESP_ELFSYM_END}};
const struct esp_elfsym g_customer_elfsyms[] = {{
  {{"shared", (void *){CUSTOMER_TARGET}}}, {{"customer_only", (void *){CUSTOMER_TARGET}}},
  ESP_ELFSYM_END}};
uintptr_t esp_elf_find_symbol(const char *name) {{
  return strcmp(name, "runtime_only") == 0 ? {RUNTIME_TARGET} : 0;
}}
unsigned registry_version(void) {{ return g_esp_elf_static_symbol_registry.version; }}
unsigned registry_count(void) {{ return g_esp_elf_static_symbol_registry.table_count; }}
'''


def facet_defines():
    """Return the CONFIG_ELF_* -D flags the elf_loader library compiles with."""
    flags = json.loads(FACET.read_text())["build"]["flags"]
    return [flag for flag in flags if flag.startswith("-DCONFIG_ELF_")]


class StaticResolver(unittest.TestCase):
    def test_facet_enables_every_compiled_table(self):
        defines = facet_defines()
        for table in ("LIBC", "ESPIDF", "CUSTOMER"):
            self.assertIn(f"-DCONFIG_ELF_LOADER_{table}_SYMBOLS=1", defines)

    def test_facet_compiles_the_upstream_registry_and_tables(self):
        source_filter = json.loads(FACET.read_text())["build"]["srcFilter"].split()
        for source in ("esp_elf_symbol.c", "esp_elf_symbol_registry.c"):
            self.assertNotIn(f"-<{source}>", source_filter)
            self.assertTrue((COMPONENT / "src" / source).is_file())

    def test_resolver_order_is_libc_idf_customer_then_runtime(self):
        with tempfile.TemporaryDirectory() as tmp:
            tables = Path(tmp) / "tables.c"
            tables.write_text("#include <string.h>\n" + TABLES)
            library = Path(tmp) / "registry.so"
            kind = "-dynamiclib" if sys.platform == "darwin" else "-shared"
            subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fPIC", kind,
                            "-DCONFIG_IDF_TARGET_ARCH_XTENSA=1", *facet_defines(),
                            "-I", str(COMPONENT / "tests/host/include"),
                            "-I", str(COMPONENT / "include"),
                            str(COMPONENT / "src/esp_elf_symbol_registry.c"), str(tables),
                            "-o", str(library)], check=True)
            loaded = ctypes.CDLL(str(library))
            loaded.registry_version.restype = ctypes.c_uint
            loaded.registry_count.restype = ctypes.c_uint
            self.assertEqual(loaded.registry_version(), 1)
            self.assertEqual(loaded.registry_count(), 3)
            resolve = loaded.elf_find_sym_default
            resolve.argtypes = [ctypes.c_char_p]
            resolve.restype = ctypes.c_size_t
            self.assertEqual(resolve(None), 0)
            self.assertEqual(resolve(b"missing"), 0)
            self.assertEqual(resolve(b"shared"), LIBC_TARGET)
            self.assertEqual(resolve(b"libc_only"), LIBC_TARGET)
            self.assertEqual(resolve(b"idf_only"), IDF_TARGET)
            self.assertEqual(resolve(b"customer_only"), CUSTOMER_TARGET)
            self.assertEqual(resolve(b"runtime_only"), RUNTIME_TARGET)

