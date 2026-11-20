"""Streamed ID3 tags and bounded MPEG duration probing over an in-memory reader."""

import unittest

from native import SDK, TESTS, build_and_run


class MediaTagsTest(unittest.TestCase):
    def test_tags_and_duration_without_full_file_scan(self):
        build_and_run(self, "media_tags", [
            TESTS / "media/tags_test.cpp",
            SDK / "pocketmage_media/media_tags.cpp",
            SDK / "pocketmage_media/media_duration.cpp",
        ])
