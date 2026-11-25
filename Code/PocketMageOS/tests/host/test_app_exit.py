
import unittest

from native import TESTS, build_and_run


class AppExitTest(unittest.TestCase):
    def test_callbacks_are_lifo_scoped_and_never_run_twice(self):
        build_and_run(self, "app_exit", [TESTS / "app/exit_test.cpp"])
