"""Exercise app-scoped exit callbacks without loading a hardware ELF image."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK


class AppExitTest(unittest.TestCase):
    def test_callbacks_are_lifo_scoped_and_never_run_twice(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "app_exit"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-idirafter", str(SDK),
                str(ROOT / "tests/host/app_exit_test.cpp"), "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True)
