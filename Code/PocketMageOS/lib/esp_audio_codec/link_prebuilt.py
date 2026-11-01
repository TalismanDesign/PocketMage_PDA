#
# Link the prebuilt esp_audio_codec archives for the active target.
#
# The esp_audio_codec decoders ship as prebuilt static archives per SoC under
# esp-adf-libs/esp_audio_codec/lib/<target>/. The *_reg.c shims compiled by the
# facet reference the register symbols directly (esp_mp3_dec_register etc.), so
# normal archive resolution pulls in exactly the objects needed. PlatformIO does
# not resolve a library.json -L/-l relative to the library folder, so we add the
# absolute archive paths here.
#
# The archives must appear AFTER the objects that reference them (the linker
# resolves static archives left-to-right). PlatformIO already wraps the whole
# link in a --start-group/--end-group and renders LIBS inside that group via
# $_LIBFLAGS, so adding the archives as LIBS File nodes places them in the group
# alongside the framework archives and the circular reference resolves without
# manual ordering. They are NOT force-loaded with --whole-archive: the codec
# archive has overlapping symbols the linker deduplicates on demand, and
# force-loading every object triggers multiple-definition errors.
#

import os

Import("env")

_FACET_DIR = Dir(".").srcnode().abspath
_LIB_ROOT = os.path.normpath(
    os.path.join(_FACET_DIR, "..", "esp-adf-libs", "esp_audio_codec", "lib")
)

_MCU_TO_TARGET = {
    "esp32s3": "esp32s3",
}

mcu = env.BoardConfig().get("build.mcu", "").lower()
target = _MCU_TO_TARGET.get(mcu)
if target is None:
    raise SystemExit(
        "esp_audio_codec: no prebuilt archive for MCU '%s' "
        "(expected one of %s)" % (mcu, ", ".join(sorted(_MCU_TO_TARGET)))
    )

_archive_dir = os.path.join(_LIB_ROOT, target)
_archives = [
    os.path.join(_archive_dir, "libesp_audio_codec.a"),
    os.path.join(_archive_dir, "libesp_audio_simple_dec.a"),
]
for _a in _archives:
    if not os.path.isfile(_a):
        raise SystemExit("esp_audio_codec: missing prebuilt archive %s" % _a)

env.Append(LIBS=[env.File(_a) for _a in _archives])
