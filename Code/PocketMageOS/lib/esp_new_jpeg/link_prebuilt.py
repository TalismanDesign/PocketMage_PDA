import os

Import("env")

_FACET_DIR = Dir(".").srcnode().abspath
_LIB_ROOT = os.path.normpath(
    os.path.join(_FACET_DIR, "..", "esp-adf-libs", "esp_new_jpeg", "lib")
)

_MCU_TO_TARGET = {
    "esp32s3": "esp32s3",
}

mcu = env.BoardConfig().get("build.mcu", "").lower()
target = _MCU_TO_TARGET.get(mcu)
if target is None:
    raise SystemExit(
        "esp_new_jpeg: no prebuilt archive for MCU '%s' "
        "(expected one of %s)" % (mcu, ", ".join(sorted(_MCU_TO_TARGET)))
    )

_archive_dir = os.path.join(_LIB_ROOT, target)
_archive = os.path.join(_archive_dir, "libesp_new_jpeg.a")
if not os.path.isfile(_archive):
    raise SystemExit("esp_new_jpeg: missing prebuilt archive %s" % _archive)

env.Append(LIBS=[env.File(_archive)])
