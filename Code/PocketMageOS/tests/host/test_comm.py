"""Exercise COMM startup against failing radio, queue, and allocation boundaries."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, function_source


class CommStartupTest(unittest.TestCase):
    def test_station_startup_and_failed_prerequisites(self):
        source = ROOT / "src/OS_APPS/COMM.cpp"
        text = source.read_text()
        state = text[text.index("// TYPES"):text.index("// MAC HELPERS")]
        config = text[text.index("// CONFIG"):text.index("// Layout constants")]
        startup = text[text.index("// INIT"):text.index("// DRAIN INCOMING QUEUE")]
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / "comm_startup.inc").write_text(
                config + state + function_source(source, "static void macToStr(") + startup)
            binary = build / "comm_test"
            # macToStr's legacy sprintf writes six hex bytes into its 18-byte
            # buffer. Apple deprecates sprintf even for this bounded format.
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-Wno-unused-function", "-Wno-unused-variable", "-Wno-unused-parameter",
                "-Wno-deprecated-declarations",
                "-fsanitize=address,undefined", "-I", str(build),
                str(Path(__file__).with_name("comm") / "startup_test.cpp"), "-o", str(binary),
            ], check=True)
            for scenario in ("station", "reopen", "allocation", "wifi-off", "wifi-start",
                             "power-save", "channel", "mac", "queue", "mesh-stop", "mesh-start"):
                with self.subTest(scenario=scenario):
                    result = subprocess.run([str(binary), scenario], capture_output=True,
                                            text=True, check=False)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
