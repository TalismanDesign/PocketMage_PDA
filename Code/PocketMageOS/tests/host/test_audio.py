"""Audio logic that decides playback results, gain, timing and USB memory checks."""

import unittest

from native import ROOT, TESTS, build_and_run


class AudioTest(unittest.TestCase):
    def test_gain_curve_and_pcm_timing(self):
        build_and_run(self, "pcm", [TESTS / "audio/pcm_test.cpp"])

    def test_playback_ids_resist_stale_and_rejected_requests(self):
        build_and_run(self, "playback_session", [TESTS / "audio/playback_session_test.cpp"])

    def test_settings_record_roundtrips_and_migrates_shared_level(self):
        build_and_run(self, "settings_format", [TESTS / "audio/settings_format_test.cpp"])

    def test_usb_admission_matches_per_urb_allocations(self):
        build_and_run(self, "usb_admission", [TESTS / "audio/usb_admission_test.cpp"],
                      includes=[ROOT / "lib/usb_stream_wrapper"])
