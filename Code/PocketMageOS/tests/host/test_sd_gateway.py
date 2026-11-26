import unittest

from native import TESTS, build_and_run


class SdGatewayTest(unittest.TestCase):
    def test_completion_ignores_logger_wake_notification(self):
        build_and_run(self, "sd_completion", [TESTS / "sd/sd_completion_test.cpp"])
