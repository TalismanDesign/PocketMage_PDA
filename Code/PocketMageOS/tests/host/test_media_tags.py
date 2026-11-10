"""Run streaming metadata regressions with an in-memory media source."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK


class MediaTagsTest(unittest.TestCase):
    def test_streamed_tags_and_frame_duration(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "media_tags_test"
            result = subprocess.run(
                [os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 "-fsanitize=address,undefined", "-idirafter", str(SDK),
                 str(ROOT / "tests/host/media_tags_test.cpp"),
                 str(SDK / "pocketmage_media/media_tags.cpp"),
                 str(SDK / "pocketmage_media/media_duration.cpp"), "-o", str(binary)],
                capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(binary)], check=True)
