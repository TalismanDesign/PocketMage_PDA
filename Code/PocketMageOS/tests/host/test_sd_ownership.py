"""Check task-scoped stream cleanup, including failed flushes and slot reuse."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_usb_app import ROOT, SDK, function_source, without_includes


class SdOwnershipTest(unittest.TestCase):
    def test_foreign_access_preserves_resources_and_snapshot_reads_are_serialized(self):
        gateway = SDK / "pocketmage_sd/sd_gateway.cpp"
        source = gateway.read_text()
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            (build / "gateway_types.inc").write_text(
                without_includes(SDK / "pocketmage_sd/sd_gateway.h"))
            declarations = source[source.index("static constexpr const char *TAG"):
                                  source.index("static const char *resolve(")]
            operations = source[source.index("bool submit("):
                                source.index("bool streamsOpen(")]
            (build / "gateway_operations.inc").write_text("\n".join((
                declarations,
                function_source(gateway, "static const char *resolve("),
                "static void exec(Request *r);", operations,
                function_source(gateway, "static FILE *openResolved("),
                function_source(gateway, "static size_t streamFill("),
                function_source(gateway, "static void dirEnumInner("),
                function_source(gateway, "static void exec(Request *r) {"))))
            binary = build / "gateway_access"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-pthread", "-fsanitize=address,undefined", "-I", str(build),
                str(ROOT / "tests/host/sd/access_test.cpp"), "-o", str(binary),
            ], check=True)
            (build / "track.mp3").write_bytes(b"music")
            (build / "owner.mp3").write_bytes(b"music")
            (build / "empty").mkdir()
            scenarios = ["snapshot-read", "snapshot-count", "snapshot-replace",
                         "snapshot-close", "snapshot-queue", "owner", "empty"]
            scenarios.extend("stream-" + operation for operation in (
                "read", "write", "seek", "tell", "size", "close"))
            for scenario in scenarios:
                with self.subTest(scenario=scenario):
                    result = subprocess.run([str(binary), str(build), scenario],
                                            capture_output=True, text=True, check=False)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_cleanup_drains_only_callers_resources_even_after_close_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            gateway = SDK / "pocketmage_sd/sd_gateway.cpp"
            (build / "cleanup.inc").write_text("\n".join(
                function_source(gateway, signature) for signature in (
                    "static bool closeStream(", "static bool closeOwnedResources(")))
            binary = build / "cleanup"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(build),
                str(ROOT / "tests/host/sd_ownership_test.cpp"), "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True)
