# shared/osdep/osd_sdl2.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 8 |
| Total commented-out lines | ~27 |
| Safe to remove | 6 blocks, ~20 lines |
| Needs review | 1 block, ~3 lines |
| Keep | 1 block |

## Blocks

### Block 1: Line 27
**Category:** DEAD_CODE
**Summary:** Commented-out `atexit(SDL_Quit)` in `OSDEP_Init()`. Unnecessary because `OSDEP_Quit()` calls `SDL_Quit()` explicitly, and `OSDEP_IsFullScreen()` also calls `SDL_Quit()`.
**Recommendation:** REMOVE

---

### Block 2: Line 43
**Category:** DEAD_CODE
**Summary:** Commented-out `SDL_WM_SetCaption(...)` in `OSDEP_SetCaption()`. This is an SDL1.2 API call that does not exist in SDL2. The function currently stores the title but never applies it to the window.
**Recommendation:** REMOVE

---

### Block 3: Line 48
**Category:** DEAD_CODE
**Summary:** Commented-out `SDL_ListModes(...)` in `OSDEP_ListModes()`. This is an SDL1.2 API that does not exist in SDL2. The function currently returns NULL (no mode enumeration).
**Recommendation:** REMOVE

---

### Block 4: Lines 57-58
**Category:** DEAD_CODE
**Summary:** Two commented-out lines in `OSDEP_IsFullScreen()` that check `OSDEP_surface->flags & SDL_FULLSCREEN`. This is SDL1.2 logic; the function was repurposed into a teardown/quit function for SDL2.
**Recommendation:** REMOVE

---

### Block 5: Lines 118-122
**Category:** INCOMPLETE_PORT
**Summary:** In `OSDEP_SetVideoMode()`, the `if(OSDEP_window != NULL)` branch has commented-out `SDL_DestroyRenderer`/`SDL_DestroyWindow` and `SDL_SetWindowSize` calls. Currently, when the window already exists, nothing happens (the `else` branch creates the window). This means video mode changes after initial creation are silently ignored.
**Recommendation:** REVIEW -- This is incomplete SDL2 porting. When changing resolution after initial window creation, the existing window/renderer should be resized or recreated. The commented-out code shows the intended approach.

---

### Block 6: Lines 149-152
**Category:** DEAD_CODE
**Summary:** Four commented-out lines that were explicit RGBA mask arguments to `SDL_CreateRGBSurface()`. The call now passes all zeros (which lets SDL choose the default format). The explicit masks are no longer needed.
**Recommendation:** REMOVE

---

### Block 7: Lines 162, 174-180, 182-190, 202, 204, 208-209
**Category:** DEBUG_LEFTOVER
**Summary:** Multiple commented-out `fprintf(stdout, "%s\n", __FUNCTION__)` traces and disabled code in the rendering path: an unused `SDL_LockTexture`/`SDL_ConvertPixels`/`SDL_UnlockTexture` sequence (lines 174-180) that was an alternative pixel-copy approach, a double-nested width loop skeleton (lines 182-190) that was never completed, and redundant `SDL_RenderPresent` / `return 0` lines. All are development-time artifacts from iterating on the SDL2 rendering pipeline.
**Recommendation:** REMOVE

---

### Block 8: Lines 379, 381-384
**Category:** LEGITIMATE
**Summary:** Line 379 is a commented-out key mapping (`OSDEP_key[SDLK_LSHIFT-0x3FFFFD1A]=43`). Lines 381-384 are `#ifdef NOTYET` with two constant definitions (`_wave=41`, `_c_center=76`). These are placeholders for incomplete key mapping work. The `NOTYET` guard and the disabled LSHIFT mapping serve as notes for future completion.
**Recommendation:** KEEP (they document planned but unfinished key mappings)

---

## Review Items

### Block 5 (Lines 118-122): Incomplete video mode resize handling
When `OSDEP_SetVideoMode()` is called and a window already exists, the function does nothing. The commented-out code shows two approaches: (a) destroy and recreate the window/renderer, or (b) call `SDL_SetWindowSize`. Either would be needed for proper resolution switching. This should be addressed when stabilizing the SDL2 port -- currently, any call to change video mode after initial setup is silently ignored.
