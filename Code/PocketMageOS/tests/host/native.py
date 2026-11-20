"""Compile and run sanitized C++17 test programs against pure SDK and OS modules.

Only hardware-free logic is tested here. Driver, USB, radio and SD behavior is
validated on the device.
"""

import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SDK = ROOT / "lib/PocketMage_SDK"
TESTS = Path(__file__).resolve().parent

_FLAGS = ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined"]


def build(name, sources, includes=()):
    """Compile sources into a temporary binary; return (binary, cleanup handle).

    The SDK root is searched last so test-local headers can stand in for hardware ones.
    """
    temp = tempfile.TemporaryDirectory()
    binary = Path(temp.name) / name
    command = [os.environ.get("CXX", "c++"), *_FLAGS]
    for include in includes:
        command += ["-I", str(include)]
    command += ["-idirafter", str(SDK), *map(str, sources), "-o", str(binary)]
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    if result.returncode:
        temp.cleanup()
        raise AssertionError(f"compile {name} failed:\n{result.stderr}")
    return binary, temp


def run(binary, *args):
    """Run a test binary and return a failure message, or None when it passes."""
    result = subprocess.run([str(binary), *args], capture_output=True, text=True, check=False)
    return None if result.returncode == 0 else result.stdout + result.stderr


def build_and_run(test, name, sources, includes=()):
    """Compile, run once, and fail the unittest case on any error."""
    binary, temp = build(name, sources, includes)
    test.addCleanup(temp.cleanup)
    failure = run(binary)
    test.assertIsNone(failure, failure)
