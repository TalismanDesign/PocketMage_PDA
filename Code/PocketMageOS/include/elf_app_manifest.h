#pragma once

#include <stddef.h>

// Parsing for an installed app's app.properties.
struct ElfAppManifest {
  char name[32];     // Display name; empty falls back to the ELF base name
  char version[16];  // Free form, e.g. "1.2.0"
  char author[32];   // Free form
  char scope[128];   // Comma separated SDK module list, or "all"
};

// Scope token limit and the length of one token buffer.
constexpr int ELF_APP_SCOPE_MAX_TOKENS = 16;
constexpr size_t ELF_APP_SCOPE_TOKEN_LEN = 24;

// The set of pm_* prefixes one app is allowed to resolve.
struct ElfAppScope {
  char prefix[ELF_APP_SCOPE_MAX_TOKENS][ELF_APP_SCOPE_TOKEN_LEN];
  int count;
  bool all;
};

// Parses app.properties text into out. Blank lines, lines starting with '#' or
// ';', and unknown keys are ignored.
int elfManifestParse(const char *text, size_t len, ElfAppManifest &out);

// Resolves a scope string into the prefixes it gates.
void elfScopeFromManifest(const char *scope, ElfAppScope &out);

// Reports whether sym_name is visible to an app with this scope. Only pm_*
// symbols are gated, so libc, libstdc++ and ESP-IDF stay reachable regardless.
bool elfScopeAllows(const ElfAppScope &scope, const char *sym_name);
