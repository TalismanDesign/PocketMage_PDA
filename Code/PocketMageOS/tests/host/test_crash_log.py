"""Exercise retained panic capture and reboot export with simulated hardware."""

from pathlib import Path
import tempfile
import unittest

from native import ROOT, TESTS, build, run

# Platform headers the crash logger includes; each resolves to the simulated board.
_SHADOWED_HEADERS = (
    "Arduino.h", "pocketmage.h", "pocketmage_globals.h", "elf_runner.h", "esp_app_desc.h",
    "esp_attr.h", "esp_heap_caps.h", "esp_memory_utils.h", "esp_random.h", "esp_timer.h",
    "esp_private/panic_internal.h", "xtensa_context.h", "pocketmage_sys/usb_role.h")


class CrashLogTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        shims = tempfile.TemporaryDirectory()
        cls.addClassCleanup(shims.cleanup)
        for header in _SHADOWED_HEADERS:
            path = Path(shims.name) / header
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('#include "crash_hardware.h"\n')
        cls.binary, temp = build("crash_log", [
            ROOT / "src/diagnostics/crash_record.cpp",
            ROOT / "src/diagnostics/crash_log.cpp",
            TESTS / "diagnostics/crash_log_test.cpp",
        ], includes=[shims.name, ROOT / "src", TESTS / "diagnostics"])
        cls.addClassCleanup(temp.cleanup)

    def test_capture_validates_and_formats_panic_snapshot(self):
        self.run_scenario("record")

    def test_panic_capture_defers_all_io_until_reboot(self):
        self.run_scenario("capture")

    def test_failed_sd_export_remains_pending_and_retries(self):
        for scenario in ("no-card", "mkdir", "listing", "write", "open", "readback", "close", "rename"):
            with self.subTest(scenario=scenario):
                self.run_scenario(scenario)

    def test_existing_report_is_never_overwritten(self):
        self.run_scenario("collision")

    def test_saved_report_replays_on_serial_reconnection_without_duplicate_files(self):
        self.run_scenario("serial")

    def test_serial_replay_rechecks_usb_role_after_runtime_sampling(self):
        self.run_scenario("serial-ownership")

    def test_completed_file_is_reused_after_interrupted_acknowledgment(self):
        self.run_scenario("post-rename")

    def test_cold_boot_does_not_export_and_storage_ownership_defers_retry(self):
        self.run_scenario("storage")

    def test_abort_before_first_sample_handles_missing_frame_and_bounded_metadata(self):
        self.run_scenario("early-abort")

    def run_scenario(self, scenario):
        failure = run(self.binary, scenario)
        self.assertIsNone(failure, failure)
