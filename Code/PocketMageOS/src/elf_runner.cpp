// In-process runner for installed .app.elf files.

// Load path: the .app.elf is read through the OS filesystem (SD via
// global_fs) into a RAM buffer, relocated by esp_elf, then entered directly.

// the app entry (app_main) runs to completion in its own
// task and owns the UI through the host singletons. The OS loop
// and the e-ink task stand down while CurrentAppState is ELFAPP. Returning
// from the entry unloads the image and goes back to HOME.

#include <globals.h>
#include <elf_runner.h>
#include <overlay_loader.h>
#include <esp_elf.h>
#include <esp_heap_caps.h>
#include <esp_idf_version.h>
#include <errno.h>

#if PM_TARGET_HOST

static void ensureCpuForSd() {
  if (getCpuFrequencyMhz() != 240) pocketmage::setCpuSpeed(240);
}

static void restoreCpuAfterSd() {
  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
}

static volatile bool s_running = false;
static TaskHandle_t s_task = NULL;

bool elfAppRunning() { return s_running; }

void elfAppSlotDir(int slot, char *out, size_t outSize) {
  snprintf(out, outSize, "/apps/slot%d", slot);
}

bool loadElfAppManifest(int slot, ElfAppManifest &out) {
  memset(&out, 0, sizeof(out));

  char dir[32];
  elfAppSlotDir(slot, dir, sizeof(dir));
  char path[64];
  snprintf(path, sizeof(path), "%s/app.properties", dir);

  File f = global_fs->open(path, "r");
  if (!f) return true;

  if (f.size() > ELF_APP_PROPERTIES_MAX_BYTES) {
    Serial.printf("[ELF] %s too large (%u bytes), ignoring\n",
                  path, (unsigned)f.size());
    f.close();
    return false;
  }

  char text[ELF_APP_PROPERTIES_MAX_BYTES];
  size_t got = f.readBytes(text, sizeof(text));
  f.close();
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
static uintptr_t scopedSymResolver(const char *sym_name) {
  if (!sym_name) return 0;
  if (!elfScopeAllows(s_scope, sym_name)) return 0;
  return elf_find_sym_default(sym_name);
}

static String infoKey(int slot) { return "ELFINFO" + String(slot); }

bool saveElfAppInfo(int slot, const ElfAppInfo &info) {
  if (slot < 1 || slot > ELF_APP_SLOTS) return false;
  prefs.begin("PocketMage", false);
  bool ok = prefs.putBytes(infoKey(slot).c_str(), &info, sizeof(info)) == sizeof(info);
  prefs.end();
  return ok;
}

bool loadElfAppInfo(int slot, ElfAppInfo &info) {
  if (slot < 1 || slot > ELF_APP_SLOTS) return false;
  prefs.begin("PocketMage", true);
  size_t n = prefs.getBytes(infoKey(slot).c_str(), &info, sizeof(info));
  prefs.end();
  if (n != sizeof(info)) return false;
  info.name[sizeof(info.name) - 1] = '\0';
  info.elfPath[sizeof(info.elfPath) - 1] = '\0';
  info.iconPath[sizeof(info.iconPath) - 1] = '\0';
  return info.elfPath[0] != '\0';
}

void clearElfAppInfo(int slot) {
  if (slot < 1 || slot > ELF_APP_SLOTS) return;
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

// Random-access reader over an open File, so the loader can stream the image
// by offset instead of the caller holding the whole file in RAM.
struct ElfFileSource {
  File *file;
};

static size_t elfFileSourceSize(void *ctx) {
  return ((ElfFileSource *)ctx)->file->size();
}

static int elfFileSourceRead(void *ctx, size_t offset, void *buf, size_t len) {
  File *file = ((ElfFileSource *)ctx)->file;
  // File::seek takes a 32-bit position, so app images never approach that.
  if (offset > UINT32_MAX) return -EINVAL;
  if (!file->seek((uint32_t)offset)) return -EIO;
  return file->read((uint8_t *)buf, len) == len ? 0 : -EIO;
}

// Segment span for the resident app image: esp_elf_t does not record the
// PT_LOAD extent, so recompute it from the program headers with the same
// rules esp_elf_load_segment() uses (contiguous PT_LOADs from vaddr_s).
static int elfSegmentSpan(File *f, uint32_t *vaddr_s, uint32_t *span) {
  elf32_hdr_t ehdr;
  if (!f->seek(0) || f->read((uint8_t *)&ehdr, sizeof(ehdr)) != (int)sizeof(ehdr))
    return -EIO;
  if (ehdr.ident[0] != 0x7f || ehdr.ident[1] != 'E' ||
      ehdr.ident[2] != 'L' || ehdr.ident[3] != 'F')
    return -EINVAL;
  uint32_t count = ehdr.phnum;
  if (count == 0 || count > 16) return -EINVAL; // app images have a handful
  elf32_phdr_t phdr[16];
  if (!f->seek(ehdr.phoff) ||
      f->read((uint8_t *)phdr, count * sizeof(elf32_phdr_t)) != (int)(count * sizeof(elf32_phdr_t)))
    return -EIO;
  uint32_t lo = 0, hi = 0;
  bool any = false;
  for (uint32_t i = 0; i < count; i++) {
    if (phdr[i].type != PT_LOAD) continue;
    uint32_t s = phdr[i].vaddr;
    uint32_t e = s + phdr[i].memsz;
    if (e < s) return -EINVAL;
    if (!any) { lo = s; hi = e; any = true; }
    else if (s < hi) return -EINVAL; // overlapping, matches loader's check
    else hi = e;
  }
  if (!any || hi <= lo) return -EINVAL;
  *vaddr_s = lo;
  *span = hi - lo;
  return 0;
}

static void elfAppTask(void *param) {
  ElfRunParams *p = (ElfRunParams *)param;
  bool ok = false;

  // Scope is set before relocate, because relocate is what asks the resolver
  // for each host symbol. Reset happens on every exit path below.
  elfScopeFromManifest(p->scope, s_scope);
  if (!s_scope.all) {
    Serial.printf("[ELF] scope:");
    for (int i = 0; i < s_scope.count; i++) Serial.printf(" %s", s_scope.prefix[i]);
    Serial.println();
  }
  elf_set_symbol_resolver(scopedSymResolver);

  ensureCpuForSd();
  uint32_t t0 = millis();
  File f = global_fs->open(p->elfPath, "r");
  if (f) {
    size_t sz = f.size();
    Serial.printf("[ELF] open %s size=%u t=%u\n",
                  p->elfPath, (unsigned)sz, (unsigned)(millis() - t0));

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
      ElfFileSource fileSrc = { &f };
      const elf_source_t src = {
          .size  = elfFileSourceSize,
          .read  = elfFileSourceRead,
          .close = NULL,
          .ctx   = &fileSrc,
      };

      esp_elf_t elf;
      uint32_t t1 = millis();
      int ire = esp_elf_init(&elf);
      int rre = (ire == 0) ? esp_elf_relocate_source(&elf, &src) : ire;
      Serial.printf("[ELF] init=%d relocate=%d entry=%p t=%u\n",
                    ire, rre, (void *)elf.entry, (unsigned)(millis() - t1));
      if (ire == 0 && rre == 0) {
        char *argv[4] = { p->appName, p->elfPath, p->version, p->author };
        Serial.printf("[ELF] %s RUN v%s by %s\n", p->appName, p->version,
                      p->author);
        const bool prevFastRefresh = FAST_REFRESH;
        FAST_REFRESH = true;
        overlay_loader_t *ov_loader = NULL;
        {
          char ov_path[160];
          bool found = false;
          snprintf(ov_path, sizeof(ov_path), "%s.overlay.bin", p->elfPath);
          if (global_fs->exists(ov_path)) found = true;
          if (!found) {
            size_t len = strlen(p->elfPath);
            if (len > 8 && strcmp(p->elfPath + len - 8, ".app.elf") == 0) {
              strcpy(ov_path, p->elfPath);
              strcpy(ov_path + len - 8, ".overlay.bin");
              if (global_fs->exists(ov_path)) found = true;
            }
          }
          if (!found) {
            const char *base = strrchr(p->elfPath, '/');
            base = base ? base + 1 : p->elfPath;
            char dir[128];
            strncpy(dir, p->elfPath, sizeof(dir)-1);
            dir[sizeof(dir)-1]=0;
            char *slash = strrchr(dir, '/');
            if (slash) *slash = 0; else strcpy(dir, "/apps");
            snprintf(ov_path, sizeof(ov_path), "%s/%.*s.overlay.bin", dir, (int)(strchr(base,'.') ? (strchr(base,'.')-base) : (int)strlen(base)), base);
            if (global_fs->exists(ov_path)) found = true;
          }
          if (!found) {
            const char *base = strrchr(p->elfPath, '/');
            base = base ? base + 1 : p->elfPath;
            char dir[128];
            strncpy(dir, p->elfPath, sizeof(dir)-1);
            dir[sizeof(dir)-1]=0;
            char *slash = strrchr(dir, '/');
            if (slash) *slash = 0; else strcpy(dir, "/apps");
            snprintf(ov_path, sizeof(ov_path), "%s/callgraph.overlay.bin", dir);
            if (global_fs->exists(ov_path)) found = true;
          }
          if (found) {
            Serial.printf("[ELF] loading overlay from %s\n", ov_path);
            if (overlay_loader_load(&ov_loader, ov_path) == OVERLAY_OK) {
              overlay_loader_set_active(ov_loader);
              if (elf.psegment) {
                // The entry vaddr recorded at relocate time; canonical is the
                // pre-relocation address the emitter stamped into the stub
                // table, so it survives across emissions instead of a magic
                // stub-block offset.
                uint32_t entry_vaddr = (uint32_t)elf.entry - (uint32_t)elf.psegment + elf.svaddr;
                uint32_t seg_svaddr = 0, seg_size = 0;
                if (elfSegmentSpan(&f, &seg_svaddr, &seg_size) == 0 &&
                    seg_svaddr == elf.svaddr) {
                  overlay_loader_set_app_image_size(ov_loader, seg_size);
                } else {
                  Serial.println("[ELF] segment span unavailable; redirect msync skipped");
                }
                Serial.printf("[ELF] overlay loaded, app_base=%p entry_vaddr=0x%08x seg_size=0x%x\n",
                              elf.psegment, entry_vaddr, (unsigned)seg_size);
                overlay_loader_apply_redirects(ov_loader, elf.psegment);
                void *stub_base = overlay_loader_get_stub_block_base(ov_loader);
                if (stub_base) {
                  // Stubs call overlay_enter themselves with the key baked in
                  // at emit time, so no manual pre-load of a body is needed.
                  Serial.printf("[ELF] stub_base=%p\n", stub_base);
                  uint32_t stub_off = 0;
                  if (overlay_loader_find_stub_off(ov_loader, entry_vaddr, &stub_off) == OVERLAY_OK) {
                    elf.entry = (int (*)(int, char**))((uintptr_t)stub_base + stub_off);
                    Serial.printf("[ELF] entry redirected to %p (stub_off=0x%x)\n",
                                  (void*)elf.entry, stub_off);
                  } else {
                    Serial.printf("[ELF] no stub for entry 0x%08x; running unredirected\n",
                                  entry_vaddr);
                  }
                }
              }
            }
          }
        }
        esp_elf_request(&elf, 0, 4, argv);
        FAST_REFRESH = prevFastRefresh;
        overlay_loader_set_active(NULL);
        if (ov_loader) {
          overlay_loader_unload(ov_loader);
          ov_loader = NULL;
        }
        esp_elf_deinit(&elf);
        Serial.printf("[ELF] %s EXITED\n", p->appName);
        Serial.printf("[ELF] heap8 free=%u largest=%u after exit\n",
                      (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
                      (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        ok = true;
      } else {
        // ENOSYS is what the loader returns for a symbol it cannot resolve,
        // which for a scoped app means the scope is too narrow.
        Serial.println(rre == -ENOSYS
                           ? "ELF needs symbols outside its scope"
                           : "ELF relocate failed");
      }
    }
    f.close();
  } else {
    Serial.printf("ELF open failed: %s\n", p->elfPath);
  }

  elf_reset_symbol_resolver();

  if (!ok) {
    OLED().sysMessage("App failed to start", 2000);
  }
  restoreCpuAfterSd();
  HOME_INIT();
  s_running = false;
  s_task = NULL;
  delete p;
  vTaskDelete(NULL);
}

bool runElfApp(int slot) {
  if (slot < 1 || slot > ELF_APP_SLOTS) return false;
  if (s_running) return false;

  ElfAppInfo info;
  if (!loadElfAppInfo(slot, info)) {
    OLED().sysMessage("No app in this slot", 2000);
    return false;
  }
  ensureCpuForSd();
  if (!global_fs->exists(info.elfPath)) {
    restoreCpuAfterSd();
    OLED().sysMessage("App file missing", 2000);
    return false;
  }

  ElfRunParams *p = new ElfRunParams;
  strncpy(p->elfPath, info.elfPath, sizeof(p->elfPath) - 1);
  p->elfPath[sizeof(p->elfPath) - 1] = '\0';

  // The manifest is optional and lives on the SD card, so an unreadable one
  // falls back to the ELF base name rather than blocking the app.
  ElfAppManifest manifest;
  bool haveManifest = loadElfAppManifest(slot, manifest);
  const char *display = (haveManifest && manifest.name[0]) ? manifest.name
                                                            : info.name;
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

#endif // PM_TARGET_HOST
