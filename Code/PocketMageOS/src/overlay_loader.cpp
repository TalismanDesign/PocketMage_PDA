#include "overlay_loader.h"
#include "pocketmage_globals.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_cache.h>
#include <esp_log.h>
#include <fs.h>

#include <cstring>
#include <algorithm>

extern "C" {
#include "esp_elf.h"
}

static const char *TAG = "overlay_loader";

#define OVERLAY_MAX_FRAMES 4
#define OVERLAY_FRAME_SIZE 16384  // 16KB default frame
#define OVERLAY_STUB_ALIGN 4

// PMOV v2 header: <4sHH6H6I
struct PmovHeader {
    char magic[4];
    uint16_t version;
    uint16_t flags;
    uint16_t overlays;
    uint16_t stubs;
    uint16_t redirects;
    uint16_t patches;
    uint16_t stub_patches;
    uint16_t host_refs;
    uint32_t stub_block_off;
    uint32_t stub_block_size;
    uint32_t slot_enter;
    uint32_t slot_leave;
    uint32_t strtab_off;
    uint32_t strtab_size;
} __attribute__((packed));

// Table records
struct PmovOverlay {
    uint16_t id;
    uint16_t pad;
    uint32_t offset; // offset in blob from start of header? or from some base
    uint32_t len;
} __attribute__((packed));

struct PmovStub {
    uint32_t canonical;
    uint16_t overlay_id;
    uint16_t body_offset;
    uint32_t stub_off;
} __attribute__((packed));

struct PmovRedirect {
    uint32_t site;
    uint32_t stub;
} __attribute__((packed));

struct PmovPatch {
    uint16_t overlay_id;
    uint16_t blob_offset;
} __attribute__((packed));

struct PmovHostRef {
    uint16_t overlay_id;
    uint16_t blob_offset;
    uint32_t name_off;
} __attribute__((packed));

struct OverlayFrame {
    uint32_t *base;
    int overlay_id;
    uint32_t body_offset;
    uint32_t last_use;
    bool occupied;
};

struct overlay_loader_s {
    File file;
    bool file_open;
    size_t file_size;
    char *bin_path; // path to overlay binary (for fallback logic)
    PmovHeader hdr;
    void *stub_block_base; // exec mem
    size_t stub_block_size;
    void *app_base;
    size_t app_image_size; // resident app segment size, for redirect msync
    
    // Tables (copied for fast access; small)
    PmovOverlay *overlays;
    uint16_t overlay_count;
    PmovStub *stubs;
    uint16_t stub_count;
    PmovRedirect *redirects;
    uint16_t redirect_count;
    PmovPatch *patches;
    uint16_t patch_count;
    PmovPatch *stub_patches;
    uint16_t stub_patch_count;
    PmovHostRef *host_refs;
    uint16_t host_ref_count;
    char *strtab;
    
    // Frame pool
    OverlayFrame frames[OVERLAY_MAX_FRAMES];
    size_t frame_count;
    size_t frame_size;
    uint32_t clock;
};

// esp_cache_msync() rejects addresses and sizes that are not cache-line
// aligned unless ESP_CACHE_MSYNC_FLAG_UNALIGNED is set. heap_caps allocations
// carry no alignment guarantee, and the stub block / frame sizes here are
// arbitrary, so every msync must be UNALIGNED or it fails with
// "invalid addr or null pointer" and the writes never reach the cache.
static void exec_msync(void *addr, size_t size) {
    __sync_synchronize();
    esp_err_t err = esp_cache_msync(addr, size,
                                    ESP_CACHE_MSYNC_FLAG_DIR_C2M |
                                    ESP_CACHE_MSYNC_FLAG_TYPE_INST |
                                    ESP_CACHE_MSYNC_FLAG_UNALIGNED);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "cache msync of %p (%u bytes) failed: %d", addr,
                 (unsigned)size, (int)err);
    }
}

static inline uint32_t align_up(uint32_t val, uint32_t align) {
    if (align == 0) return val;
    return (val + align - 1) & ~(align - 1);
}

static inline bool word_aligned(void *p) {
    return ((uintptr_t)p & 3) == 0;
}

// Read from file using standard C library (fopen/fread/fseek)
// This avoids Arduino fs::File vtable corruption issues.
static int file_read_at(const char *bin_path, size_t off, void *buf, size_t len) {
    if (!bin_path || !buf || len == 0) return -1;

    char resolved[192];
    const char *mount = global_fs ? global_fs->mountpoint() : nullptr;
    const size_t mountLen = (mount && mount[0]) ? strlen(mount) : 0;
    const bool alreadyPrefixed = mountLen > 0 &&
                                 strncmp(bin_path, mount, mountLen) == 0;
    if (mountLen > 0 && !alreadyPrefixed) {
        snprintf(resolved, sizeof(resolved), "%s%s", mount, bin_path);
    } else {
        snprintf(resolved, sizeof(resolved), "%s", bin_path);
    }

    // Open file using standard C library (layered on ESP-IDF VFS)
    FILE *fp = fopen(resolved, "rb");
    if (!fp) {
        ESP_LOGI(TAG, "Failed to open %s with fopen", resolved);
        return -1;
    }
    
    // Seek to the desired offset
    if (fseek(fp, (long)off, SEEK_SET) != 0) {
        fclose(fp);
        ESP_LOGI(TAG, "Failed to seek to offset %zu in %s", off, resolved);
        return -1;
    }
    
    // Read the requested bytes
    size_t r = fread(buf, 1, len, fp);
    fclose(fp);
    
    if (r != len) {
        ESP_LOGI(TAG, "Short read: got %zu of %zu bytes from %s", r, len, resolved);
        return -1;
    }
    return 0;
}

int overlay_loader_load(overlay_loader_t **out, const char *bin_path) {
    ESP_LOGI(TAG, "load start %s", bin_path);
    if (!out || !bin_path) return OVERLAY_ERR_FORMAT;
    *out = nullptr;
    
    overlay_loader_t *loader = (overlay_loader_t *)calloc(1, sizeof(overlay_loader_t));
    if (!loader) { ESP_LOGE(TAG, "calloc failed"); return OVERLAY_ERR_MEM; }
    
    // Open file
    if (!global_fs) {
        free(loader);
        return OVERLAY_ERR_IO;
    }
    loader->file = global_fs->open(bin_path, FILE_READ);
    if (!loader->file) {
        free(loader);
        return OVERLAY_ERR_IO;
    }
    loader->file_open = true;
    loader->file_size = loader->file.size();
    // Store the binary path for use by overlay_read_at
    loader->bin_path = strdup(bin_path);
    if (!loader->bin_path) {
        free(loader);
        return OVERLAY_ERR_MEM;
    }
    
    // Read header
    if (file_read_at(loader->bin_path, 0, &loader->hdr, sizeof(PmovHeader)) != 0) {
        overlay_loader_unload(loader);
        return OVERLAY_ERR_IO;
    }
    // Validate
    if (memcmp(loader->hdr.magic, "PMOV", 4) != 0) {
        overlay_loader_unload(loader);
        return OVERLAY_ERR_FORMAT;
    }
    if (loader->hdr.version != 2) {
        overlay_loader_unload(loader);
        return OVERLAY_ERR_FORMAT;
    }
    
    // Allocate tables
    size_t fixed = sizeof(PmovHeader);
    loader->overlay_count = loader->hdr.overlays;
    loader->stub_count = loader->hdr.stubs;
    loader->redirect_count = loader->hdr.redirects;
    loader->patch_count = loader->hdr.patches;
    loader->stub_patch_count = loader->hdr.stub_patches;
    loader->host_ref_count = loader->hdr.host_refs;
    
    loader->overlays = (PmovOverlay *)calloc(loader->overlay_count, sizeof(PmovOverlay));
    loader->stubs = (PmovStub *)calloc(loader->stub_count, sizeof(PmovStub));
    loader->redirects = (PmovRedirect *)calloc(loader->redirect_count, sizeof(PmovRedirect));
    loader->patches = (PmovPatch *)calloc(loader->patch_count, sizeof(PmovPatch));
    loader->stub_patches = (PmovPatch *)calloc(loader->stub_patch_count, sizeof(PmovPatch));
    loader->host_refs = (PmovHostRef *)calloc(loader->host_ref_count, sizeof(PmovHostRef));
    if ((!loader->overlays && loader->overlay_count) || (!loader->stubs && loader->stub_count) ||
        (!loader->redirects && loader->redirect_count) || (!loader->patches && loader->patch_count) ||
        (!loader->stub_patches && loader->stub_patch_count) || (!loader->host_refs && loader->host_ref_count)) {
        overlay_loader_unload(loader);
        return OVERLAY_ERR_MEM;
    }
    
    // Read tables
    size_t off = fixed;
    if (loader->overlay_count) {
        if (file_read_at(loader->bin_path, off, loader->overlays, loader->overlay_count * sizeof(PmovOverlay)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->overlay_count * sizeof(PmovOverlay);
    }
    if (loader->stub_count) {
        if (file_read_at(loader->bin_path, off, loader->stubs, loader->stub_count * sizeof(PmovStub)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->stub_count * sizeof(PmovStub);
    }
    if (loader->redirect_count) {
        if (file_read_at(loader->bin_path, off, loader->redirects, loader->redirect_count * sizeof(PmovRedirect)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->redirect_count * sizeof(PmovRedirect);
    }
    if (loader->patch_count) {
        if (file_read_at(loader->bin_path, off, loader->patches, loader->patch_count * sizeof(PmovPatch)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->patch_count * sizeof(PmovPatch);
    }
    if (loader->stub_patch_count) {
        if (file_read_at(loader->bin_path, off, loader->stub_patches, loader->stub_patch_count * sizeof(PmovPatch)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->stub_patch_count * sizeof(PmovPatch);
    }
    if (loader->host_ref_count) {
        if (file_read_at(loader->bin_path, off, loader->host_refs, loader->host_ref_count * sizeof(PmovHostRef)) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->host_ref_count * sizeof(PmovHostRef);
    }
    // strtab
    if (loader->hdr.strtab_size > 0 && loader->hdr.strtab_off == off) {
        loader->strtab = (char *)malloc(loader->hdr.strtab_size);
        if (!loader->strtab) { overlay_loader_unload(loader); return OVERLAY_ERR_MEM; }
        if (file_read_at(loader->bin_path, off, loader->strtab, loader->hdr.strtab_size) != 0) {
            overlay_loader_unload(loader); return OVERLAY_ERR_IO;
        }
        off += loader->hdr.strtab_size;
    }
    
    // Allocate stub block (exec, writable)
    loader->stub_block_size = loader->hdr.stub_block_size;
    if (loader->stub_block_size > 0) {
        loader->stub_block_base = heap_caps_malloc(loader->stub_block_size, MALLOC_CAP_EXEC);
        if (!loader->stub_block_base) {
            overlay_loader_unload(loader);
            return OVERLAY_ERR_MEM;
        }
        // Copy stub block from blob
        if (file_read_at(loader->bin_path, loader->hdr.stub_block_off, loader->stub_block_base, loader->stub_block_size) != 0) {
            overlay_loader_unload(loader);
            return OVERLAY_ERR_IO;
        }
        // Patch enter/leave slots: slots at hdr.slot_enter/leave contain addresses of overlay_enter/overlay_leave
        if (loader->hdr.slot_enter < loader->stub_block_size) {
            *(uint32_t *)((uint8_t *)loader->stub_block_base + loader->hdr.slot_enter) = (uint32_t)&overlay_enter;
        }
        if (loader->hdr.slot_leave < loader->stub_block_size) {
            *(uint32_t *)((uint8_t *)loader->stub_block_base + loader->hdr.slot_leave) = (uint32_t)&overlay_leave;
        }
        exec_msync(loader->stub_block_base, loader->stub_block_size);
    }
    
    // Init frame pool
    loader->frame_count = OVERLAY_MAX_FRAMES;
    loader->frame_size = OVERLAY_FRAME_SIZE;
    for (size_t i = 0; i < loader->frame_count; i++) {
        loader->frames[i].base = (uint32_t *)heap_caps_malloc(loader->frame_size, MALLOC_CAP_EXEC);
        if (!loader->frames[i].base) {
            // try smaller count
            loader->frame_count = i;
        }
        loader->frames[i].occupied = false;
        loader->frames[i].overlay_id = -1;
        loader->frames[i].body_offset = 0;
        loader->frames[i].last_use = 0;
    }
    if (loader->frame_count == 0 && loader->overlay_count > 0) {
        overlay_loader_unload(loader);
        return OVERLAY_ERR_MEM;
    }
    loader->clock = 0;
    
    *out = loader;
    return OVERLAY_OK;
}

void overlay_loader_unload(overlay_loader_t *loader) {
    if (!loader) return;
    if (loader->file_open) {
        loader->file.close();
        loader->file_open = false;
    }
    free(loader->bin_path);
    free(loader->overlays);
    free(loader->stubs);
    free(loader->redirects);
    free(loader->patches);
    free(loader->stub_patches);
    free(loader->host_refs);
    free(loader->strtab);
    if (loader->stub_block_base) {
        heap_caps_free(loader->stub_block_base);
        loader->stub_block_base = nullptr;
    }
    for (size_t i = 0; i < OVERLAY_MAX_FRAMES; i++) {
        if (loader->frames[i].base) {
            heap_caps_free(loader->frames[i].base);
            loader->frames[i].base = nullptr;
        }
    }
    free(loader);
}

static overlay_loader_t *g_active_loader = nullptr;

static int exec_copy_words(uint32_t *dst, const uint32_t *src, size_t byte_len) {
    if (!dst || !src || (byte_len % 4) != 0) return -1;
    size_t count = byte_len / 4;
    for (size_t i = 0; i < count; i++) {
        dst[i] = src[i];
    }
    __sync_synchronize();
    return 0;
}

static int pick_victim(overlay_loader_t *loader) {
    int victim = -1;
    for (size_t i = 0; i < loader->frame_count; i++) {
        if (!loader->frames[i].occupied) return (int)i;
        if (victim < 0 || loader->frames[i].last_use < loader->frames[victim].last_use) {
            victim = (int)i;
        }
    }
    return victim;
}

static int find_resident(overlay_loader_t *loader, int overlay_id) {
    for (size_t i = 0; i < loader->frame_count; i++) {
        if (loader->frames[i].occupied && loader->frames[i].overlay_id == overlay_id) {
            return (int)i;
        }
    }
    return -1;
}

static int find_overlay(overlay_loader_t *loader, uint16_t id) {
    for (uint16_t i = 0; i < loader->overlay_count; i++) {
        if (loader->overlays[i].id == id) return i;
    }
    return -1;
}

void *overlay_enter(uint32_t key) {
    overlay_loader_t *loader = g_active_loader;
    if (!loader) {
        ESP_LOGE(TAG, "overlay_enter called with no active loader");
        return nullptr;
    }
    uint16_t overlay_id = (key >> 16) & 0xFFFF;
    ESP_LOGI(TAG, "enter ov=%d key=%08x", overlay_id, key);
    uint16_t body_offset = key & 0xFFFF; // body offset within overlay blob (relative to body start in frame context)
    
    // Find resident frame
    int slot = find_resident(loader, overlay_id);
    if (slot < 0) {
        // Need to page in
        int oidx = find_overlay(loader, overlay_id);
        if (oidx < 0) {
            ESP_LOGE(TAG, "unknown overlay %d", overlay_id);
            return nullptr;
        }
        PmovOverlay *ov = &loader->overlays[oidx];
        slot = pick_victim(loader);
        if (slot < 0) {
            ESP_LOGE(TAG, "no frame available");
            return nullptr;
        }
        // Bodies follow after header+stub_block aligned (first_overlay_offset)
        // ov->offset is stored as an absolute offset from the start of the overlay blob
        // by pm emit (first_overlay_offset + cursor). Read from ov->offset directly.
        size_t read_off = ov->offset;
        if (read_off + ov->len > loader->file_size) {
            ESP_LOGE(TAG, "overlay read out of range");
            return nullptr;
        }
        // Read into staging buffer
        uint32_t *staging = (uint32_t *)heap_caps_malloc(ov->len, MALLOC_CAP_8BIT);
        if (!staging) {
            ESP_LOGE(TAG, "alloc staging failed");
            return nullptr;
        }
        if (file_read_at(loader->bin_path, read_off, staging, ov->len) != 0) {
            heap_caps_free(staging);
            ESP_LOGE(TAG, "read overlay failed");
            return nullptr;
        }
        // Apply patches for this overlay at page-in time
        // patches: app-relative canonical -> word + app_base (applied to frame)
        for (uint16_t i = 0; i < loader->patch_count; i++) {
            if (loader->patches[i].overlay_id == overlay_id) {
                uint16_t boff = loader->patches[i].blob_offset;
                if (boff + 4 <= ov->len) {
                    uint32_t *w = (uint32_t *)((uint8_t *)staging + boff);
                    if (loader->app_base) {
                        *w = (uint32_t)loader->app_base + *w;
                    }
                }
            }
        }
        // Apply stub_patches
        for (uint16_t i = 0; i < loader->stub_patch_count; i++) {
            if (loader->stub_patches[i].overlay_id == overlay_id) {
                uint16_t boff = loader->stub_patches[i].blob_offset;
                if (boff + 4 <= ov->len && loader->stub_block_base) {
                    uint32_t *w = (uint32_t *)((uint8_t *)staging + boff);
                    *w = (uint32_t)loader->stub_block_base + *w;
                }
            }
        }
        // Apply host_refs
        for (uint16_t i = 0; i < loader->host_ref_count; i++) {
            if (loader->host_refs[i].overlay_id == overlay_id) {
                uint16_t boff = loader->host_refs[i].blob_offset;
                if (boff + 4 <= ov->len) {
                    uint32_t *w = (uint32_t *)((uint8_t *)staging + boff);
                    const char *name = loader->strtab + loader->host_refs[i].name_off;
                    // elf_find_sym_default searches the built-in libc/espidf/
                    // customer tables (esp_elf_symbol.c). esp_elf_find_symbol()
                    // only covers tables registered at runtime via
                    // esp_elf_register_symbol(), which the firmware never does,
                    // so pm_* host refs would always resolve to 0 through it.
                    uintptr_t sym = elf_find_sym_default(name);
                    if (sym == 0) {
                        ESP_LOGW(TAG, "unresolved host symbol: %s, using 0", name);
                        *w = 0;
                    } else {
                        *w = (uint32_t)sym;
                    }
                }
            }
        }
        // Copy to frame (32-bit aligned)
        if (exec_copy_words(loader->frames[slot].base, (const uint32_t *)staging, ov->len) != 0) {
            heap_caps_free(staging);
            ESP_LOGE(TAG, "copy to frame failed");
            return nullptr;
        }
        heap_caps_free(staging);
        exec_msync(loader->frames[slot].base, loader->frame_size);
        loader->frames[slot].overlay_id = overlay_id;
        loader->frames[slot].occupied = true;
    }
    loader->frames[slot].last_use = ++loader->clock;
    // Return frame base + body_offset? body_offset is offset within overlay body
    // The key gives body_offset as (overlay_id << 16) | body_offset - we need to extract body_offset properly
    uint32_t body_off = key & 0xFFFF;
    return (void *)((uint8_t *)loader->frames[slot].base + body_off);
}

void overlay_leave(uint32_t key) {
    (void)key;
    // MVP behavior: no-op for now; pinning/unpinning can be added later
}

void overlay_loader_set_app_image_size(overlay_loader_t *loader, size_t size) {
    if (loader) loader->app_image_size = size;
}

int overlay_loader_apply_redirects(overlay_loader_t *loader, void *app_base) {
    if (!loader || !app_base) { ESP_LOGE(TAG, "apply_redirects: bad args"); return OVERLAY_ERR_FORMAT; }
    if (!loader->stub_block_base) { ESP_LOGE(TAG, "apply_redirects: no stub block"); return OVERLAY_ERR_FORMAT; }
    loader->app_base = app_base;
    ESP_LOGI(TAG, "applying %d redirects, app_base=%p stub=%p", loader->redirect_count, app_base, loader->stub_block_base);
    for (uint16_t i = 0; i < loader->redirect_count; i++) {
        uint32_t site = loader->redirects[i].site;
        uint32_t stub_off = loader->redirects[i].stub;
        uint32_t val = (uint32_t)loader->stub_block_base + stub_off;
        if (i == 0) {
            ESP_LOGI(TAG, "redir0 site=%08x", site);
        }

        uintptr_t candidates[4];
        candidates[0] = (uintptr_t)app_base + site;
        candidates[1] = site;
        candidates[2] = (uintptr_t)app_base + (site & 0xFFFFFFF);
        candidates[3] = (uintptr_t)app_base + (site - 0x3C000000);
        for (int c=0; c<4; c++) {
            uintptr_t d = candidates[c];
            if (d >= 0x3c000000 && d < 0x50000000 && (d & 3) == 0) {
                *(uint32_t *)d = val;
            }
        }
    }
    __sync_synchronize();
    // The app image spans several MB of segment buffer; msync the exact region
    // rather than a fixed 16 MB so the unaligned flush stays inside the heap.
    exec_msync(app_base, loader->app_image_size);
    ESP_LOGI(TAG, "redirects applied");
    return OVERLAY_OK;
}

// Global accessor for enter/leave
extern "C" void overlay_loader_set_active(overlay_loader_t *loader) {
    g_active_loader = loader;
}

void *overlay_loader_get_stub_block_base(const overlay_loader_t *loader) {
    if (!loader) return nullptr;
    return loader->stub_block_base;
}

// The ELF entry point (ehdr->entry) is a virtual address inside the app's
// text segment; the emitter recorded that same canonical address on the
// stub that fronts app_main. Lookup by canonical is what ties the
// redirect target to the right stub without hardcoding emission offsets.
int overlay_loader_find_stub_off(const overlay_loader_t *loader,
                                 uint32_t canonical, uint32_t *stub_off) {
    if (!loader || !stub_off) return OVERLAY_ERR_RANGE;
    for (uint16_t i = 0; i < loader->stub_count; i++) {
        if (loader->stubs[i].canonical == canonical) {
            *stub_off = loader->stubs[i].stub_off;
            return OVERLAY_OK;
        }
    }
    return OVERLAY_ERR_RANGE;
}

size_t overlay_loader_get_frame_size(const overlay_loader_t *loader) {
    if (!loader) return 0;
    return loader->frame_size;
}

size_t overlay_loader_get_frame_count(const overlay_loader_t *loader) {
    if (!loader) return 0;
    return loader->frame_count;
}

extern "C" overlay_loader_t *overlay_loader_get_active(void) {
    return g_active_loader;
}
