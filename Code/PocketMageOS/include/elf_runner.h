#pragma once

#include <Arduino.h>
#include <config.h>

// In-process runner for installed .app.elf files

struct ElfAppInfo {
  char name[32];      // App base name (from the .app.elf file name)
  char elfPath[96];   // Absolute path of the installed .app.elf
  char iconPath[64];  // Absolute path of the installed *_ICON.bin
};

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
