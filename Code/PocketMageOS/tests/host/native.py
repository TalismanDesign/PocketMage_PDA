
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SDK = ROOT / "lib/PocketMage_SDK"
TESTS = Path(__file__).resolve().parent
FIXTURES = ROOT.parents[1] / "Resources/Subsystem Tests/Audio"

_FLAGS = ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined"]


def build(name, sources, includes=()):
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
    result = subprocess.run(
        [str(binary), *args], capture_output=True, text=True, check=False
    )
    return None if result.returncode == 0 else result.stdout + result.stderr


def build_and_run(test, name, sources, includes=(), args=()):
    binary, temp = build(name, sources, includes)
    test.addCleanup(temp.cleanup)
    failure = run(binary, *args)
    test.assertIsNone(failure, failure)
