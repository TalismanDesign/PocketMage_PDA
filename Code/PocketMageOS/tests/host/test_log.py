import unittest

from native import TESTS, build_and_run


class LogCoreTest(unittest.TestCase):
    def test_format_filter_queue_rotation_and_tail(self):
        build_and_run(self, "log_core", [TESTS / "log/log_core_test.cpp"])
