
import unittest

from native import FIXTURES, ROOT, TESTS, build_and_run


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

    def test_supported_formats_map_to_their_decoder_stacks(self):
        build_and_run(self, "audio_format", [TESTS / "audio/format_test.cpp"])

    def test_extensible_wav_header_is_rewritten_for_the_decoder(self):
        build_and_run(self, "wav_stream", [TESTS / "audio/wav_stream_test.cpp"],
                      includes=[TESTS], args=[str(FIXTURES)])

    def test_ogg_codec_is_identified_for_stall_diagnostics(self):
        build_and_run(self, "ogg_stream", [TESTS / "audio/ogg_stream_test.cpp"],
                      includes=[TESTS], args=[str(FIXTURES)])

    def test_decoder_feed_tracks_eof_and_real_parser_stalls(self):
        build_and_run(self, "audio_feed", [TESTS / "audio/feed_test.cpp"])
