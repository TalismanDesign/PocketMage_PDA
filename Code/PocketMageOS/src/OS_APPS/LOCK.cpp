// LOCK.cpp
//
// The e-ink never repaints while the device is locked: the sleep screensaver
// is already on the panel, and unlocking should be OLED-fast

#include <globals.h>
#if PM_TARGET_HOST // POCKETMAGE_OS

#include <mbedtls/sha256.h>

// Transient lock requirement: set by loadState() on every boot/wake and by
// lockSetPin()/the "lock on" command; cleared only by a successful PIN entry.
// While set, loop() blocks on lockEnsureUnlocked() and the e-ink keeps the
// current panel content instead of repainting.
volatile bool deviceLocked = false;

// NVS keys (namespace "PocketMage", shared with the rest of the OS)
static const char* LOCK_PREF_ENABLED = "LOCK_ENABLED";
static const char* LOCK_PREF_HASH    = "LOCK_HASH";
static const char* LOCK_PREF_SALT    = "LOCK_SALT";

// PIN policy: 4..8 decimal digits
constexpr int LOCK_PIN_MIN_LEN = 4;
constexpr int LOCK_PIN_MAX_LEN = 8;
constexpr int LOCK_SALT_BYTES  = 16;

static String lockSalt;
static String lockHash;

// Salted SHA-256, hex-encoded.  The stored hash is sha256(salt + pin), so the
// PIN is never written to flash in any recoverable form.
static String lockSha256Hex(const String& data) {
  unsigned char digest[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
  mbedtls_sha256_finish(&ctx, digest);
  mbedtls_sha256_free(&ctx);

  String hex;
  for (int i = 0; i < 32; i++) {
    if (digest[i] < 0x10) hex += "0";
    hex += String(digest[i], HEX);
  }
  return hex;
}

static String lockNewSalt() {
  // Seeded by randomSeed(analogRead(BAT_SENS)) at boot; fresh per PIN change.
  String salt;
  for (int i = 0; i < LOCK_SALT_BYTES; i++) {
    uint8_t b = static_cast<uint8_t>(random(0, 256));
    if (b < 0x10) salt += "0";
    salt += String(b, HEX);
  }
  return salt;
}

static void lockLoadPrefs() {
  prefs.begin("PocketMage", true);
  lockHash = prefs.getString(LOCK_PREF_HASH, "");
  lockSalt = prefs.getString(LOCK_PREF_SALT, "");
  prefs.end();
}

bool lockIsEnabled() {
  prefs.begin("PocketMage", true);
  bool enabled = prefs.getBool(LOCK_PREF_ENABLED, false);
  prefs.end();
  return enabled;
}

bool lockHasPin() {
  prefs.begin("PocketMage", true);
  bool hasPin = prefs.getString(LOCK_PREF_HASH, "").length() > 0;
  prefs.end();
  return hasPin;
}

bool lockPinValid(const String& pin) {
  if (pin.length() < LOCK_PIN_MIN_LEN || pin.length() > LOCK_PIN_MAX_LEN) return false;
  for (unsigned int i = 0; i < pin.length(); i++) {
    if (pin[i] < '0' || pin[i] > '9') return false;
  }
  return true;
}

// Stores a salted hash of the PIN and enables the lock immediately
void lockSetPin(const String& pin) {
  String salt = lockNewSalt();
  String hash = lockSha256Hex(salt + pin);

  prefs.begin("PocketMage", false);
  prefs.putBool(LOCK_PREF_ENABLED, true);
  prefs.putString(LOCK_PREF_SALT, salt);
  prefs.putString(LOCK_PREF_HASH, hash);
  prefs.end();

  lockSalt = salt;
  lockHash = hash;
  deviceLocked = true;
}

void lockDisable() {
  prefs.begin("PocketMage", false);
  prefs.putBool(LOCK_PREF_ENABLED, false);
  prefs.putString(LOCK_PREF_HASH, "");
  prefs.putString(LOCK_PREF_SALT, "");
  prefs.end();
  deviceLocked = false;
}

// Constant-time-ish comparison of hashes: both sides are already hashed, so a
// timing leak would only disclose the hash, not the PIN.
bool lockVerifyPin(const String& pin) {
  lockLoadPrefs();
  if (lockHash.length() == 0) return true;  // No PIN configured -> nothing to check
  return lockSha256Hex(lockSalt + pin) == lockHash;
}

// Called from applicationEinkHandler() while locked.  Deliberately a no-op:
// the sleep screensaver is already on the e-ink and a repaint would make every
// wake slow.  The PIN gate lives entirely on the OLED.
void einkHandler_LOCK() {
  // keep the screensaver on screen; do not repaint while locked
}

// Called from the OS loop() while a lock is required (boot/wake, NOWLATER exit,
// or a mid-session "lock on").  Blocks on the masked OLED prompt until the
// stored PIN is entered; cancel/escape keys just re-prompt so a locked device
// cannot fall through to the home command bar.  Yields when NOWLATER takes
// over (power-off while charging), so the shutdown screen can run.
void lockEnsureUnlocked() {
  while (deviceLocked && CurrentHOMEState != NOWLATER) {
    String entered = textPrompt(TR(STR_LOCK_PROMPT), "", true, true);

    if (entered == "_EXIT_" || entered == "_RETURN_" || entered == "_CENTER_") continue;

    if (lockVerifyPin(entered)) {
      deviceLocked = false;
      OLED().sysMessage(TR(STR_LOCK_UNLOCKED), 800);
    } else {
      OLED().sysMessage(TR(STR_LOCK_WRONG), 1200);
    }
  }
}

#endif // POCKETMAGE_OS
