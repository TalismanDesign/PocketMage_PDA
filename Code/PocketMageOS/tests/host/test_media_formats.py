
import unittest

from native import FIXTURES, SDK, TESTS, build_and_run


class MediaFormatsTest(unittest.TestCase):
    def test_wav_ogg_and_spec_files_probe_bounded_regions(self):
        build_and_run(self, "media_formats", [
            TESTS / "media/formats_test.cpp",
            SDK / "pocketmage_media/media_wav.cpp",
            SDK / "pocketmage_media/media_ogg.cpp",
            SDK / "pocketmage_media/media_tags.cpp",
        ], includes=[TESTS], args=[str(FIXTURES)])
