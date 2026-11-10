// In-process runner for installed .app.elf files.

// Load path: the .app.elf is read through the OS filesystem (SD via
// global_fs) into a RAM buffer, relocated by esp_elf, then entered directly.

// the app entry (app_main) runs to completion in its own
// task and owns the UI through the host singletons. The OS loop
// and the e-ink task stand down while CurrentAppState is ELFAPP. Returning
// from the entry unloads the image and goes back to HOME.

#include <elf_overlay/esp_elf_integration.h>
#include <elf_overlay/overlay_loader.h>
#include <elf_runner.h>
#include <errno.h>
#include <esp_elf.h>
#include <esp_heap_caps.h>
#include <esp_idf_version.h>
#include <globals.h>
#include <pm_overlay_api.h>
#include <pocketmage_sd/sd_gateway.h>
#include "../lib/elf_loader/symbol_registry.h"

#if PM_TARGET_HOST

static volatile bool s_running = false;
static TaskHandle_t s_task = NULL;

bool elfAppRunning() {
  return s_running;
}

void elfAppSlotDir(int slot, char* out, size_t outSize) {
  snprintf(out, outSize, "/apps/slot%d", slot);
}

bool loadElfAppManifest(int slot, ElfAppManifest& out) {
  memset(&out, 0, sizeof(out));

  char dir[32];
  elfAppSlotDir(slot, dir, sizeof(dir));
  char path[64];
  snprintf(path, sizeof(path), "%s/app.properties", dir);

  // A missing or non-file manifest is not an error: the ELF falls back to
  // its base name.
  size_t fsz = 0;
  bool isDir = false;
  if (!PM_SD().statPath(path, &fsz, &isDir) || isDir)
    return true;

  if (fsz > ELF_APP_PROPERTIES_MAX_BYTES) {
    Serial.printf("[ELF] %s too large (%u bytes), ignoring\n", path, (unsigned)fsz);
    return false;
  }

  char text[ELF_APP_PROPERTIES_MAX_BYTES];
  size_t got = PM_SD().readBinaryFile(path, (uint8_t*)text, fsz) ? fsz : 0;
  int fields = elfManifestParse(text, got, out);
  Serial.printf("[ELF] %s fields=%d\n", path, fields);
  return true;
}

// Scope state for the resolver below. Only one app runs at a time (guarded by
// s_running) and relocate happens on the app task, so this needs no lock.
static ElfAppScope s_scope;

// Resolver installed for the duration of one app load. It hides pm_* symbols
// outside the app's scope and delegates the rest to the default export table,
// so libc, libstdc++ and ESP-IDF stay reachable for any scope.
static uintptr_t scopedSymResolver(const char* sym_name) {
  if (!sym_name)
    return 0;
  if (!elfScopeAllows(s_scope, sym_name))
    return 0;
  return pm_elf_find_symbol(sym_name);
}

static String infoKey(int slot) {
  return "ELFINFO" + String(slot);
}

bool saveElfAppInfo(int slot, const ElfAppInfo& info) {
  if (slot < 1 || slot > ELF_APP_SLOTS)
    return false;
  prefs.begin("PocketMage", false);
  bool ok = prefs.putBytes(infoKey(slot).c_str(), &info, sizeof(info)) == sizeof(info);
  prefs.end();
  return ok;
}

bool loadElfAppInfo(int slot, ElfAppInfo& info) {
  if (slot < 1 || slot > ELF_APP_SLOTS)
    return false;
  prefs.begin("PocketMage", true);
  size_t n = prefs.getBytes(infoKey(slot).c_str(), &info, sizeof(info));
  prefs.end();
  if (n != sizeof(info))
    return false;
  info.name[sizeof(info.name) - 1] = '\0';
  info.elfPath[sizeof(info.elfPath) - 1] = '\0';
  info.iconPath[sizeof(info.iconPath) - 1] = '\0';
  return info.elfPath[0] != '\0';
}

void clearElfAppInfo(int slot) {
  if (slot < 1 || slot > ELF_APP_SLOTS)
    return;
  prefs.begin("PocketMage", false);
  prefs.remove(infoKey(slot).c_str());
  // Drop legacy bin-era keys so stale installs vanish on upgrade.
  prefs.remove(("OTAINFO" + String(slot)).c_str());
  prefs.remove(("OTA" + String(slot)).c_str());
  prefs.end();
}

struct ElfRunParams {
  char elfPath[96];
  char appName[32];
  char version[16];
  char author[32];
  char scope[128];
};

// Random-access reader over a gateway stream slot, so the loader can stream
// the image by offset instead of the caller holding the whole file in RAM.
struct ElfFileSource {
  int slot;
};

static size_t elfFileSourceSize(void* ctx) {
  return PM_SD().streamSize(((ElfFileSource*)ctx)->slot);
}

static int elfFileSourceRead(void* ctx, size_t offset, void* buf, size_t len) {
  ElfFileSource* s = (ElfFileSource*)ctx;
  // streamSeek takes a long position, so app images never approach that.
  if (offset > LONG_MAX)
    return -EINVAL;
  if (!PM_SD().streamSeek(s->slot, (long)offset, SEEK_SET))
    return -EIO;
  return PM_SD().streamRead(s->slot, buf, len) == len ? 0 : -EIO;
}

static void finalizeElf(esp_elf_t* elf) {
  if (!elf->init_done)
    return;
  // Separate finalization from deallocation so fini hooks can register exit work.
  // Match esp_elf_deinit's array order and legacy sentinel handling without
  // changing upstream: components/elf_loader/src/esp_elf.c in esp-iot-solution.
  elf->init_done = false;
  for (uint32_t i = 0; i < elf->fini_count; ++i) {
    void (*callback)() = elf->fini_array[elf->fini_reverse ? elf->fini_count - i - 1 : i];
    if (callback && reinterpret_cast<uintptr_t>(callback) != UINTPTR_MAX)
      callback();
  }
}

static void stopAppAudio() {
  // Retain the image while a decoder or USB worker can still reference app state.
  while (!P_AUDIO.stopChecked()) {
    Serial.println("[ELF] audio shutdown incomplete; retaining app and retrying");
    delay(100);
  }
}

static bool executeLoadedElf(esp_elf_t* elf, int argc, char** argv) {
  if (!P_APP.beginSession())
    return false;
  const int result = esp_elf_request(elf, 0, argc, argv);
  stopAppAudio();
  finalizeElf(elf);
  P_APP.finishSession();
  // Exit hooks may have started audio or opened another stream.
  stopAppAudio();
  const bool closed = pm_sd_gateway::closeCallerResources();
  if (!closed)
    Serial.println("[ELF] app stream flush failed during cleanup");
  return result == 0 && closed;
}

static void elfAppTask(void* param) {
  ElfRunParams* p = (ElfRunParams*)param;
  bool ok = false;

  // Scope is set before relocate, because relocate is what asks the resolver
  // for each host symbol. Reset happens on every exit path below.
  elfScopeFromManifest(p->scope, s_scope);
  if (!s_scope.all) {
    Serial.printf("[ELF] scope:");
    for (int i = 0; i < s_scope.count; i++)
      Serial.printf(" %s", s_scope.prefix[i]);
    Serial.println();
  }
  elf_set_symbol_resolver(scopedSymResolver);

  uint32_t t0 = millis();
  int slot = PM_SD().streamOpen(p->elfPath, 'r');
  if (slot >= 0) {
    size_t sz = PM_SD().streamSize(slot);
    Serial.printf("[ELF] open %s size=%u t=%u\n", p->elfPath, (unsigned)sz,
                  (unsigned)(millis() - t0));

    // The image is streamed by the loader, so the whole file is never held in
    // RAM. Only the transient section buffers (symtab/strtab/relocs) add to
    // the peak. Log the free heaps so a device run confirms the budget for the
    // no-PSRAM case (PM_BETA).
    Serial.printf("[ELF] heap8 free=%u largest=%u exec free=%u largest=%u\n",
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
                  (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_EXEC),
                  (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_EXEC));

    if (sz == 0) {
      Serial.println("ELF empty");
    } else {
      ElfFileSource fileSrc = {slot};
      const elf_source_t src = {
          .size = elfFileSourceSize,
          .read = elfFileSourceRead,
          .close = NULL,
          .ctx = &fileSrc,
      };

      esp_elf_t elf{};
      uint32_t t1 = millis();
      int ire = esp_elf_init(&elf);

      // Load overlay before relocation
      elf_overlay_loader_t* ov_loader = NULL;
      {
        char ov_path[160];
        bool found = false;
        snprintf(ov_path, sizeof(ov_path), "%s.elfo", p->elfPath);
        if (PM_SD().exists(ov_path))
          found = true;
        if (!found) {
          size_t len = strlen(p->elfPath);
          if (len > 8 && strcmp(p->elfPath + len - 8, ".app.elf") == 0) {
            strcpy(ov_path, p->elfPath);
            strcpy(ov_path + len - 8, ".elfo");
            if (PM_SD().exists(ov_path))
              found = true;
          }
        }
        if (!found) {
          const char* base = strrchr(p->elfPath, '/');
          base = base ? base + 1 : p->elfPath;
          char dir[128];
          strncpy(dir, p->elfPath, sizeof(dir) - 1);
          dir[sizeof(dir) - 1] = 0;
          char* slash = strrchr(dir, '/');
          if (slash)
            *slash = 0;
          else
            strcpy(dir, "/apps");
          snprintf(ov_path, sizeof(ov_path), "%s/%.*s.elfo", dir,
                   (int)(strchr(base, '.') ? (strchr(base, '.') - base) : (int)strlen(base)), base);
          if (PM_SD().exists(ov_path))
            found = true;
        }
        if (!found) {
          const char* base = strrchr(p->elfPath, '/');
          base = base ? base + 1 : p->elfPath;
          char dir[128];
          strncpy(dir, p->elfPath, sizeof(dir) - 1);
          dir[sizeof(dir) - 1] = 0;
          char* slash = strrchr(dir, '/');
          if (slash)
            *slash = 0;
          else
            strcpy(dir, "/apps");
          snprintf(ov_path, sizeof(ov_path), "%s/%s.elfo", dir, base);
          if (PM_SD().exists(ov_path))
            found = true;
        }
        if (found) {
          Serial.printf("[ELF] loading overlay from %s\n", ov_path);
          int ov_rc = elf_overlay_load(&ov_loader, ov_path);
          if (ov_rc != ELF_OVERLAY_OK) {
            Serial.printf("[ELF] overlay load skipped rc=%d path=%s\n", ov_rc, ov_path);
            ov_loader = NULL;
          }
        }
      }

      int rre = (ire == 0) ? esp_elf_relocate_source(&elf, &src) : ire;
      const bool sourceClosed = PM_SD().streamClose(slot);
      slot = -1;
      if (!sourceClosed && rre == 0)
        rre = -EIO;
      Serial.printf("[ELF] init=%d relocate=%d entry=%p t=%u\n", ire, rre, (void*)elf.entry,
                    (unsigned)(millis() - t1));
      if (ire == 0 && rre == 0) {
        Serial.printf("[ELF] runtime_base=%p\n", (void*)esp_elf_runtime_base(&elf));
        char* argv[4] = {p->appName, p->elfPath, p->version, p->author};
        Serial.printf("[ELF] %s RUN v%s by %s\n", p->appName, p->version, p->author);
        const bool prevFastRefresh = FAST_REFRESH;
        FAST_REFRESH = true;

        if (ov_loader) {
          elf_overlay_set_active(ov_loader);
          elf_overlay_esp_elf_set_section_map(ov_loader, &elf);
          elf_overlay_apply_redirects(ov_loader);
          elf_overlay_set_symbol_resolver(ov_loader, scopedSymResolver);
          elf_overlay_probe_host_refs(ov_loader);
          void* stub_base = elf_overlay_get_stub_block(ov_loader);
          if (stub_base) {
            Serial.printf("[ELF] overlay loaded, stub_base=%p\n", stub_base);
            uint32_t stub_off = 0;
            uint32_t entry_vaddr = (uint32_t)elf.entry - (uint32_t)elf.sec[ELF_SEC_TEXT].addr +
                                   elf.sec[ELF_SEC_TEXT].v_addr;
            if (elf_overlay_find_stub(ov_loader, entry_vaddr, &stub_off) == ELF_OVERLAY_OK) {
              Serial.printf("[ELF] entry vaddr=0x%08x stub_off=0x%x\n", entry_vaddr, stub_off);
            } else {
              Serial.printf("[ELF] no stub for entry 0x%08x; running unredirected\n", entry_vaddr);
            }
          }
        }
        ok = executeLoadedElf(&elf, 4, argv);
        FAST_REFRESH = prevFastRefresh;
        Serial.printf("[ELF] %s EXITED\n", p->appName);
        Serial.printf("[ELF] heap8 free=%u largest=%u after exit\n",
                      (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
                      (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      } else {
        // ENOSYS is what the loader returns for a symbol it cannot resolve,
        // which for a scoped app means the scope is too narrow.
        Serial.println(rre == -ENOSYS ? "ELF needs symbols outside its scope"
                                      : "ELF relocate failed");
      }
      if (ire == 0)
        esp_elf_deinit(&elf);
      elf_overlay_set_active(NULL);
      if (ov_loader)
        elf_overlay_unload(ov_loader);
    }
    if (slot >= 0 && !PM_SD().streamClose(slot))
      Serial.println("[ELF] source stream close failed");
  } else {
    Serial.printf("ELF open failed: %s\n", p->elfPath);
  }

  elf_reset_symbol_resolver();

  if (!ok) {
    OLED().sysMessage("App failed to start", 2000);
  }
  HOME_INIT();
  s_running = false;
  s_task = NULL;
  delete p;
  vTaskDelete(NULL);
}

bool runElfApp(int slot) {
  if (slot < 1 || slot > ELF_APP_SLOTS)
    return false;
  if (s_running)
    return false;

  ElfAppInfo info;
  if (!loadElfAppInfo(slot, info)) {
    OLED().sysMessage("No app in this slot", 2000);
    return false;
  }
  if (!PM_SD().exists(info.elfPath)) {
    OLED().sysMessage("App file missing", 2000);
    return false;
  }

  ElfRunParams* p = new ElfRunParams{};
  strncpy(p->elfPath, info.elfPath, sizeof(p->elfPath) - 1);
  p->elfPath[sizeof(p->elfPath) - 1] = '\0';

  // The manifest is optional and lives on the SD card, so an unreadable one
  // falls back to the ELF base name rather than blocking the app.
  ElfAppManifest manifest;
  bool haveManifest = loadElfAppManifest(slot, manifest);
  const char* display = (haveManifest && manifest.name[0]) ? manifest.name : info.name;
  strncpy(p->appName, display, sizeof(p->appName) - 1);
  p->appName[sizeof(p->appName) - 1] = '\0';
  if (haveManifest) {
    strncpy(p->version, manifest.version, sizeof(p->version) - 1);
    strncpy(p->author, manifest.author, sizeof(p->author) - 1);
    strncpy(p->scope, manifest.scope, sizeof(p->scope) - 1);
  }
  p->version[sizeof(p->version) - 1] = '\0';
  p->author[sizeof(p->author) - 1] = '\0';
  p->scope[sizeof(p->scope) - 1] = '\0';

  OLED().sysMessage(String("Loading ") + p->appName, 1500);
  CurrentAppState = ELFAPP;
  s_running = true;
  if (xTaskCreatePinnedToCore(elfAppTask, "elfApp", 16384, p, 1, &s_task, 1) != pdPASS) {
    Serial.println("ELF task create failed");
    delete p;
    s_running = false;
    HOME_INIT();
    return false;
  }
  return true;
}

#endif  // PM_TARGET_HOST
