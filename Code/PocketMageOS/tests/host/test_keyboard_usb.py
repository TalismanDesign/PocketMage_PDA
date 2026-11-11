"""Exercise HID worker acknowledgements and failure recovery without hardware."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK, without_includes


class KeyboardUsbTest(unittest.TestCase):
    def test_reconnect_clears_queued_and_held_keys(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            source = without_includes(SDK / "pocketmage_kb/pocketmage_kb.cpp")
            source = source.split("#pragma region keymaps", 1)[0]
            source = "\n".join(line for line in source.splitlines()
                               if not line.startswith("#pragma"))
            (build / "keyboard_input.inc").write_text(source)
            binary = build / "input_test"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra",
                "-Werror", "-fsanitize=address,undefined", "-I", str(build),
                "-I", str(ROOT / ".pio/libdeps/PM_BETA/ESP32_USB_Host_HID"),
                "-idirafter", str(SDK),
                str(Path(__file__).with_name("keyboard") / "input_test.cpp"),
                "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True)

    def test_shutdown_retains_resources_until_workers_and_devices_are_done(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / "usb_keyboard.inc").write_text(
                without_includes(SDK / "pocketmage_kb/usb_keyboard.cpp"))
            binary = build / "keyboard_test"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra",
                "-Werror", "-pthread", "-fsanitize=address,undefined",
                "-I", str(build), "-idirafter", str(SDK),
                str(Path(__file__).with_name("keyboard") / "lifecycle_test.cpp"),
                "-o", str(binary),
            ], check=True)
            scenarios = ["normal", "connections-join", "power-off", "devices", "info",
                         "driver-uninstall", "library-join", "events", "free-devices",
                         "host-uninstall", "phy"]
            scenarios.extend("start:" + operation for operation in (
                "input", "allocation1", "allocation2", "allocation3", "host-install",
                "library-task", "connections-task", "driver-install", "power-on"))
            for scenario in scenarios:
                with self.subTest(scenario=scenario):
                    result = subprocess.run([str(binary), scenario], capture_output=True,
                                            text=True, timeout=20, check=False)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
