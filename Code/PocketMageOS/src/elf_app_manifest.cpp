#include <elf_app_manifest.h>

#include <string.h>
#include <strings.h>

static const char *fieldValue(const char *line, const char *key) {
  size_t klen = strlen(key);
  if (strncmp(line, key, klen) != 0) return nullptr;
  const char *p = line + klen;
  while (*p == ' ' || *p == '\t') p++;
  if (*p != '=') return nullptr;
  return p + 1;
}

// Copies a value into dst, stopping at the first byte in stops. Short fields
// stop at any space; scope keeps internal spaces so "eink, text" survives.
static bool setField(char *dst, size_t dstSize, const char *value,
                     const char *stops) {
  if (!value) return false;
  while (*value == ' ' || *value == '\t') value++;
  size_t n = strcspn(value, stops);
  if (n >= dstSize) n = dstSize - 1;
  memcpy(dst, value, n);
  dst[n] = '\0';
  return true;
}

int elfManifestParse(const char *text, size_t len, ElfAppManifest &out) {
  memset(&out, 0, sizeof(out));
  if (!text) return 0;

  int fields = 0;
  char line[160];
  size_t pos = 0;
  while (pos < len) {
    size_t start = pos;
    while (pos < len && text[pos] != '\n' && text[pos] != '\r') pos++;
    size_t n = pos - start;
    if (n >= sizeof(line)) n = sizeof(line) - 1;
    memcpy(line, text + start, n);
    line[n] = '\0';
    while (pos < len && (text[pos] == '\n' || text[pos] == '\r')) pos++;

    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (!*p || *p == '#' || *p == ';') continue;

    if (setField(out.name, sizeof(out.name), fieldValue(p, "name"),
                 " \t\r\n")) {
      fields++;
    }
    if (setField(out.version, sizeof(out.version), fieldValue(p, "version"),
                 " \t\r\n")) {
      fields++;
    }
    if (setField(out.author, sizeof(out.author), fieldValue(p, "author"),
                 " \t\r\n")) {
      fields++;
    }
    if (setField(out.scope, sizeof(out.scope), fieldValue(p, "scope"),
                 "\r\n")) {
      fields++;
    }
  }
  return fields;
}

static void scopeReset(ElfAppScope &out) {
  memset(out.prefix, 0, sizeof(out.prefix));
  out.count = 0;
  out.all = true;
}

// Turns one scope token into the symbol prefix it gates.
static bool tokenToPrefix(const char *token, char *out, size_t outSize) {
  while (*token == ' ' || *token == '\t') token++;
  size_t n = strcspn(token, " \t\r\n,");
  if (n == 0 || n >= outSize - 3) return false;

  const char *body = token;
  if (strncmp(body, "pm_", 3) == 0) body += 3;
  size_t blen = n - (size_t)(body - token);
  while (blen > 0 && body[blen - 1] == '_') blen--;
  if (blen == 0) return false;

  memcpy(out, "pm_", 3);
  memcpy(out + 3, body, blen);
  out[3 + blen] = '_';
  out[4 + blen] = '\0';
  return true;
}

void elfScopeFromManifest(const char *scope, ElfAppScope &out) {
  scopeReset(out);
  if (!scope || !*scope) return;

  char buf[sizeof(out.prefix[0]) * ELF_APP_SCOPE_MAX_TOKENS];
  size_t n = strlen(scope);
  if (n >= sizeof(buf)) n = sizeof(buf) - 1;
  memcpy(buf, scope, n);
  buf[n] = '\0';

  for (char *tok = strtok(buf, ","); tok; tok = strtok(nullptr, ",")) {
    if (strcasecmp(tok, "all") == 0) {
      scopeReset(out);
      return;
    }
    if (out.count >= ELF_APP_SCOPE_MAX_TOKENS) break;
    if (tokenToPrefix(tok, out.prefix[out.count], ELF_APP_SCOPE_TOKEN_LEN)) {
      out.count++;
    }
  }
  // A scope with no usable token would otherwise hide the whole SDK, which is
  // the opposite of what the author asked for.
  if (out.count == 0) scopeReset(out);
  else out.all = false;
}

bool elfScopeAllows(const ElfAppScope &scope, const char *sym_name) {
  if (scope.all || !sym_name) return true;
  if (strncmp(sym_name, "pm_", 3) != 0) return true;
  for (int i = 0; i < scope.count; i++) {
    size_t plen = strlen(scope.prefix[i]);
    if (strncmp(sym_name, scope.prefix[i], plen) == 0) return true;
  }
  return false;
}
