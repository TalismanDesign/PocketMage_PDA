
import tempfile
import unittest
from pathlib import Path

from native import SDK, TESTS, build_and_run


_SHIMS = ("pocketmage.h", "pocketmage_sd/pocketmage_sd.h")


class ImageStreamTest(unittest.TestCase):
    def test_failed_decoder_open_closes_region_stream(self):
        shims = tempfile.TemporaryDirectory()
        self.addCleanup(shims.cleanup)
        for header in _SHIMS:
            path = Path(shims.name) / header
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('#include "region_hardware.h"\n')
        build_and_run(self, "image_streams", [
            TESTS / "image/image_stream_test.cpp",
            SDK / "pocketmage_image/jpeg.cpp",
            SDK / "pocketmage_image/png.cpp",
            SDK / "pocketmage_image/gray_downscaler.cpp",
            SDK / "pocketmage_image/region_stream.cpp",
        ], includes=[shims.name, TESTS / "image"])
