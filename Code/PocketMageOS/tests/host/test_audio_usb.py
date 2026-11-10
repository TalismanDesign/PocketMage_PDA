"""Run the native audio lifecycle against a simulated USB controller."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK, function_source, without_includes


class AudioUsbTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        build = Path(cls.temp.name)
        (build / "usb_role.inc").write_text(without_includes(SDK / "pocketmage_sys/usb_role.cpp"))
        for header in ("Arduino.h", "freertos/FreeRTOS.h", "freertos/queue.h",
                       "freertos/semphr.h", "freertos/task.h", "usb_stream.h"):
            path = build / header
            path.parent.mkdir(exist_ok=True)
            path.write_text('#include "audio_usb_hardware.h"\n')
        audio = SDK / "pocketmage_audio/pocketmage_audio.cpp"
        functions = ("PocketMageAudio &PocketMageAudio::getInstance()",
                     "PocketMageAudio::PocketMageAudio()", "PocketMageAudio::~PocketMageAudio()",
                     "void PocketMageAudio::begin()", "void PocketMageAudio::stop()",
                     "bool PocketMageAudio::stopChecked()", "bool PocketMageAudio::beginUsb()",
                     "bool PocketMageAudio::pollUsb()", "bool PocketMageAudio::stopUsb()",
                     "esp_err_t PocketMageAudio::writeUsb(", "bool PocketMageAudio::acquireUsb(",
                     "void PocketMageAudio::releaseUsb()",
                     "void PocketMageAudio::usbStateCb(", "void PocketMageAudio::setError(",
                      "void PocketMageAudio::publishEvent()", "String PocketMageAudio::getLastError()",
                      "void PocketMageAudio::play(", "uint32_t PocketMageAudio::getPlaybackId()",
                      "int PocketMageAudio::getPlaybackResult(", "uint64_t PocketMageAudio::getPositionMs()",
                     "void PocketMageAudio::setOutput(", "AudioOutput PocketMageAudio::getOutput()",
                     "void PocketMageAudio::setVolume(", "int8_t PocketMageAudio::getVolume()")
        (build / "audio_usb.inc").write_text("\n".join(
            function_source(audio, signature) for signature in functions) + "\n" +
            without_includes(SDK / "pocketmage_audio/audio_settings.cpp"))
        (build / "audio_play.inc").write_text("\n".join(
            function_source(audio, signature) for signature in (
                "void PocketMageAudio::doPlay()", "void PocketMageAudio::doStop()")))
        cls.binary = build / "audio_usb_test"
        cls.play_binary = build / "audio_play_test"
        for binary in (cls.binary, cls.play_binary):
            subprocess.run(
                [os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 "-Wno-unused-parameter", "-Wno-unused-const-variable",
                 "-fsanitize=address,undefined", "-I", str(build),
                 "-idirafter", str(SDK), "-I", str(ROOT / "tests/host"),
                 str(ROOT / "tests/host" / (binary.name + ".cpp")), "-o", str(binary)], check=True)

    def test_rejected_play_has_failed_id_and_cannot_inherit_decoder_completion(self):
        for scenario in ("null", "empty", "long", "task", "init", "shutdown", "transfer",
                         "queue", "stale", "queued", "accepted"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([str(self.play_binary), scenario], capture_output=True,
                                        text=True, check=False)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_audio_releases_serial_only_after_host_stops(self):
        for scenario in ("stop", "stop-failure", "start-failure", "restart", "power-failure",
                         "keyboard-busy", "storage-busy", "live-phy", "auto-output",
                         "settings", "settings-failure", "serial-default", "piezo",
                         "auto-unplug", "piezo-switch", "auto-reconnect", "probe-timeout",
                         "piezo-stop-retry", "piezo-power-failure", "connected",
                         "keyboard-handoff", "keyboard-handoff-retry", "piezo-keyboard-usb"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([str(self.binary), scenario], capture_output=True,
                                        text=True, check=False)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
