import os
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SDK = ROOT / "lib/PocketMage_SDK"


def function_source(path, signature):
    """Return a complete function body for a hardware-bound translation unit."""
    text = path.read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def without_includes(path):
    """Keep production code intact while substituting hardware-only includes."""
    return "\n".join(line for line in path.read_text().splitlines()
                     if not line.startswith("#include"))


class UsbAppTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        build = Path(cls.temp.name)
        usb = ROOT / "src/OS_APPS/USB.cpp"
        source = without_includes(usb)
        (build / "usb_role.inc").write_text(without_includes(SDK / "pocketmage_sys/usb_role.cpp"))
        poll = function_source(SDK / "pocketmage_kb/pocketmage_kb.cpp",
                               "void PocketmageKB::checkUSBKB()")
        boot = function_source(ROOT / "src/UTILS.cpp", "AppState bootShortcutApp(")
        load = function_source(ROOT / "src/UTILS.cpp", "void loadState(")
        unmount = function_source(SDK / "pocketmage_sd/sd_gateway.cpp",
                                  "static bool unmountForUsb()")
        clock = function_source(SDK / "pocketmage_sys/pocketmage_sys.cpp", "void setCpuSpeed(")
        (build / "usb_app.inc").write_text("\n".join((
            source, poll, boot, load,
            "namespace pm_sd_gateway {", unmount, "}",
            "namespace pocketmage {", clock, "}")))
        cls.binary = build / "usb_app_test"
        # The fake boundary intentionally implements only the peripherals used
        # by these entry points; other globals and callback parameters are inert.
        subprocess.run(
            [os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra",
             "-Werror", "-Wno-unused-parameter", "-Wno-unused-function",
             "-Wno-unused-variable",
             "-fsanitize=address,undefined",
             "-I", str(build), "-idirafter", str(SDK),
             str(Path(__file__).with_name("usb_app_test.cpp")),
             "-o", str(cls.binary)], check=True)

    def test_usb_ownership_startup_failures_and_io(self):
        for scenario in ("home", "poll", "stop-failure", "card-busy", "msc-failure",
                         "device-failure", "status", "bounds", "partial", "shutdown",
                         "boot", "boot-home-preference", "boot-locked", "cpu-floor",
                         "open-stream", "unmount-failure", "serial-poll", "audio-poll",
                         "keyboard-busy", "keyboard-unplug", "keyboard-transfer"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([str(self.binary), scenario], capture_output=True,
                                        text=True, check=False)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
