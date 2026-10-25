#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Result codes for overlay operations
#define OVERLAY_OK 0
#define OVERLAY_ERR_IO -1
#define OVERLAY_ERR_FORMAT -2
#define OVERLAY_ERR_MEM -3
#define OVERLAY_ERR_RANGE -4

// Opaque handle for loaded overlays
typedef struct overlay_loader_s overlay_loader_t;

// Load overlay manifest/blob from SD. The loader reads from the same
// filesystem as the app (global_fs). `bin_path` is the full path to
// <app>.overlay.bin (e.g. "/apps/slot0/app.overlay.bin").
int overlay_loader_load(overlay_loader_t **out, const char *bin_path);

// Unload and free all overlay resources (stub block, frames, file handle).
void overlay_loader_unload(overlay_loader_t *loader);

// Overlay enter/leave called by position-independent stubs.
// Key is packed as (overlay_id << 16) | body_offset (as emitted by the tool).
// overlay_enter returns the resident body pointer in exec RAM (frame_base + body_offset).
// Never returns NULL in normal operation; paging failures are fatal.
void *overlay_enter(uint32_t key);
void overlay_leave(uint32_t key);

// Apply redirects to the resident app image at load time.
// `app_base` is the resident app base address (elf.psegment equivalent).
// Call after the app is relocated but before execution; must be called after
// overlay_loader_load() so the stub block is allocated and patched.
int overlay_loader_apply_redirects(overlay_loader_t *loader, void *app_base);

// Tell the loader how large the resident app segment is, so the final
// cache flush after redirects covers exactly the app image.
void overlay_loader_set_app_image_size(overlay_loader_t *loader, size_t size);

// Get stub block base (for diagnostics). Returns NULL if not loaded.
void *overlay_loader_get_stub_block_base(const overlay_loader_t *loader);

// Find the stub block offset of the stub fronting the function whose
// canonical (pre-relocation) virtual address is `canonical` -- e.g. the
// ELF entry vaddr. Returns OVERLAY_OK and sets *stub_off on success,
// OVERLAY_ERR_RANGE if no stub matches.
int overlay_loader_find_stub_off(const overlay_loader_t *loader,
                                 uint32_t canonical, uint32_t *stub_off);

// Get frame pool stats (optional, for debugging)
size_t overlay_loader_get_frame_size(const overlay_loader_t *loader);
size_t overlay_loader_get_frame_count(const overlay_loader_t *loader);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Set/get active loader for enter/leave functions
void overlay_loader_set_active(overlay_loader_t *loader);
overlay_loader_t *overlay_loader_get_active(void);
#ifdef __cplusplus
}
#endif
