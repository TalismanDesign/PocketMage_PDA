#pragma once

#include <Arduino.h>
#include <config.h>
#include <elf_app_manifest.h>

// In-process runner for installed .app.elf files

struct ElfAppInfo {
  char name[32];      // App base name (from the .app.elf file name)
  char elfPath[96];   // Absolute path of the installed .app.elf
  char iconPath[64];  // Absolute path of the installed *_ICON.bin
};

// Refuse a manifest larger than this. The parser works line by line, so the
// cap bounds parse time on a hostile SD card rather than memory.
constexpr size_t ELF_APP_PROPERTIES_MAX_BYTES = 1024;

// Reads <slot dir>/app.properties into out. Returns false only when the file
// exists but cannot be read; a missing manifest leaves out zeroed and true.
bool loadElfAppManifest(int slot, ElfAppManifest &out);

constexpr int ELF_APP_SLOTS = 4;

bool saveElfAppInfo(int slot, const ElfAppInfo &info);
bool loadElfAppInfo(int slot, ElfAppInfo &info);
void clearElfAppInfo(int slot);

// Slot directory "/apps/slot<n>". Writes into out (outSize includes NUL).
void elfAppSlotDir(int slot, char *out, size_t outSize);

#if PM_TARGET_HOST
bool elfAppRunning();
bool runElfApp(int slot);
#else
inline bool elfAppRunning() { return false; }
#endif
