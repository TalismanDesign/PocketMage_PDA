"""Exercise gain and playback timing independently of hardware and encoded tags."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK


class AudioPcmTest(unittest.TestCase):
    def test_shared_gain_and_decoded_frame_timing(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "audio_pcm_test"
            subprocess.run(
                [os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 "-fsanitize=address,undefined", "-idirafter", str(SDK),
                 str(ROOT / "tests/host/audio_pcm_test.cpp"), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
