
import tempfile
import unittest
from pathlib import Path

from native import SDK, TESTS, build_and_run


_SHIMS = ("pocketmage.h", "pocketmage_sd/pocketmage_sd.h")


class RegionStreamTest(unittest.TestCase):
    def test_base64_regions_decode_and_seek(self):
        shims = tempfile.TemporaryDirectory()
        self.addCleanup(shims.cleanup)
        for header in _SHIMS:
            path = Path(shims.name) / header
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('#include "region_hardware.h"\n')
        build_and_run(self, "region_stream", [
            TESTS / "image/region_stream_test.cpp",
            SDK / "pocketmage_image/region_stream.cpp",
        ], includes=[shims.name, TESTS / "image"])
