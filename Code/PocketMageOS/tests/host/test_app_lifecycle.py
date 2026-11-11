"""Verify loader teardown while callback code and SD resources remain resident."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK, function_source


class AppLifecycleTest(unittest.TestCase):
    def test_finalizers_and_retryable_audio_stop_precede_unload(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            for header in ("FreeRTOS.h", "task.h"):
                path = build / "freertos" / header
                path.parent.mkdir(exist_ok=True)
                path.write_text("#ifndef PM_TEST_FREERTOS\n#define PM_TEST_FREERTOS\nusing TaskHandle_t = void *;\n"
                                "extern TaskHandle_t task;\n"
                                "inline TaskHandle_t xTaskGetCurrentTaskHandle() { return task; }\n#endif\n")
            loader = ROOT / "src/elf_runner.cpp"
            (build / "lifecycle.inc").write_text("\n".join(
                function_source(loader, signature) for signature in (
                    "static void finalizeElf(", "static void stopAppAudio(",
                    "static bool executeLoadedElf(")))
            binary = build / "lifecycle"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(build), "-idirafter", str(SDK),
                str(ROOT / "tests/host/app_lifecycle_test.cpp"),
                str(SDK / "pocketmage_app/pocketmage_app.cpp"), "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True)
