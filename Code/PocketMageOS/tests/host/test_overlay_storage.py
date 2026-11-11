"""Keep overlay handles alive without holding SD streams between page reads."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, without_includes


class OverlayStorageTest(unittest.TestCase):
    def test_each_page_read_closes_its_stream_on_success_and_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / "storage.inc").write_text(without_includes(ROOT / "src/elf_overlay_storage.cpp"))
            binary = build / "storage"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(build),
                str(ROOT / "tests/host/overlay_storage_test.cpp"), "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True)
