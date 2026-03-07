# div.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 30 |
| Total commented-out lines | ~90 |
| Safe to remove | 24 blocks, ~70 lines |
| Needs review | 3 blocks, ~16 lines |
| Keep | 3 blocks, ~4 lines |

## Blocks

### Block 1: Lines 99-100
**Category:** MIKE_REMOVED (line 99) / MIKE_ADDED_DEAD (line 100)
**Summary:** Line 99: DOS system clock pointer `int * system_clock = (void*) 0x46c` was active in the original (line 37) -- Mike commented it out and replaced it with the SDL-compatible `system_clock = &mclock` assignment (line 407). Line 100: `int system_clock;` is an alternate declaration Mike tried and abandoned.
**Recommendation:** REMOVE (the replacement on line 407 `system_clock = &mclock` is the active code)

---

### Block 2: Line 230
**Category:** ORIGINAL_DEAD
**Summary:** Commented-out `fwrite(&exe_cola[1], 1, 4, f)` inside the SHARE/demo trial counter logic. Was already non-functional since the whole `#ifdef SHARE` block is disabled.
**Recommendation:** REMOVE (dead code inside dead code)

---

### Block 3: Line 402
**Category:** MIKE_ADDED_DEAD
**Summary:** Partial comment `// SDL_INIT_EVERYTHING);` -- remnant of Mike changing the SDL init call. The active call is `OSDEP_Init()`.
**Recommendation:** REMOVE

---

### Block 4: Line 617
**Category:** LEGITIMATE
**Summary:** Comment documenting the Emscripten API signature `emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop)`. Useful reference for the `#ifdef __EMSCRIPTEN__` block that follows.
**Recommendation:** KEEP

---

### Block 5: Line 634
**Category:** ORIGINAL_DEAD
**Summary:** `//New_DownLoad_Desktop();` -- already commented out in the original DOS source (line 211).
**Recommendation:** REMOVE

---

### Block 6: Line 730
**Category:** ORIGINAL_DEAD
**Summary:** `//  if(!CopiaDesktop) nueva_ventana(menu_principal0); else UpLoad_Desktop();` -- already commented out in the original (line 303). Superseded by the active code on lines 732-738.
**Recommendation:** REMOVE

---

### Block 7: Line 740
**Category:** MIKE_REMOVED
**Summary:** `//  if (test_video) dialogo(test0);` -- was active in original (line 313). The `test0` dialog showed a video test on startup. Mike commented it out during port.
**Recommendation:** REVIEW -- test_video mode may be useful to restore for debugging video setup

---

### Block 8: Lines 762-765
**Category:** ORIGINAL_DEAD
**Summary:** Four commented lines that would open `system/div.txt` on first run. Already commented out in original (lines 335-338). The replacement code `nueva_ventana(menu_principal0)` is active.
**Recommendation:** REMOVE

---

### Block 9: Lines 776-782
**Category:** ORIGINAL_DEAD
**Summary:** Block comment containing sound error dialog code (`SoundError` check). Already commented out in original (lines 350-356). Has OJO annotation on line 774.
**Recommendation:** REMOVE (including the OJO comment on line 774, which refers to this dead block)

---

### Block 10: Lines 1671, 1974
**Category:** LEGITIMATE
**Summary:** `// Alt+S DOS shell disabled (no DOS)` -- placeholder comments noting that the Alt+S shell shortcut was intentionally removed. In the original (line 993), this was `if ((shift_status&8) && scan_code==31) shell();`. The comment documents the deliberate omission.
**Recommendation:** KEEP (both instances, in entorno main loop and dialog loop)

---

### Block 11: Line 1715
**Category:** MIKE_REMOVED
**Summary:** `//  check_free();` -- was active in the original `entorno()` (line 449). The function checks available disk space and warns the user. Mike commented it out.
**Recommendation:** REVIEW -- `check_free()` still exists (line 3800+) and may be worth restoring as a startup warning

---

### Block 12: Lines 1736-1749
**Category:** MIKE_REMOVED
**Summary:** The original DOS `command_path()` function body (finds `command.com` via COMSPEC/PATH) was commented out by Mike and replaced with `return NULL;`. The original had this as active code (lines 1031-1044). The function is DOS-specific (`command.com`, `_searchenv`) and cannot work on SDL platforms.
**Recommendation:** REVIEW -- the function and its caller `shell()` are both dead-end DOS code. Consider removing the whole `command_path()`/`shell()` block rather than just the comment.

---

### Block 13: Line 2011
**Category:** ORIGINAL_DEAD
**Summary:** `//  if (flushall()>10) fcloseall();` -- was active in original (line 1222). These are Watcom C library functions not available on modern compilers.
**Recommendation:** REMOVE

---

### Block 14: Lines 2098, 2102
**Category:** ORIGINAL_DEAD
**Summary:** Two lines in `maximiza_ventana()`: `//  if (v.tipo>=100) activar();` and `//  if (exploding_windows) extrude(...)`. Both already commented out in the original (lines 1305 and 1309 respectively). In the original, line 1309 was active (`if (exploding_windows) extrude(...)`) but in current SDL code Mike rewrote the logic differently so the extrude call became dead. The `activar()` call was already dead in original.
**Recommendation:** REMOVE (line 2098 is ORIGINAL_DEAD; line 2102 is MIKE_REMOVED but replaced by line 2106)

---

### Block 15: Line 2146
**Category:** ORIGINAL_DEAD
**Summary:** `//if (exploding_windows) extrude(x,y,an,al,v.x,v.y,v.an,v.al);` in `minimiza_ventana()`. Was active in original (line 1352) but Mike commented it out as part of the SDL port where window extrusion animation was reworked.
**Recommendation:** REMOVE (the exploding_windows system is handled differently now)

---

### Block 16: Lines 2294-2299, 2305
**Category:** ORIGINAL_DEAD
**Summary:** Six commented lines in `mueve_ventana_completa()`: `volcado_copia()`, the `fondo_edicion/volcar_barras/actualiza_dialogos` block, and `wrectangulo`. All already commented out in the original (lines 1489-1500).
**Recommendation:** REMOVE

---

### Block 17: Line 2408
**Category:** MIKE_ADDED_DEAD
**Summary:** `#define NIGHTLY_VERSION "DIVDX BUILD 01234abcdef"` -- Mike's dead define for nightly build branding. Never used (the `#ifdef GIT_SHA1` block below handles this).
**Recommendation:** REMOVE

---

### Block 18: Lines 3072-3073
**Category:** MIKE_ADDED_DEAD
**Summary:** Two commented SDL calls (`SDL_FillRect` with color params) in `nueva_ventana()`. Mike wrote these as an alternative to `memset(ptr,c0,an*al)` but commented them out.
**Recommendation:** REMOVE

---

### Block 19: Line 3363
**Category:** MIKE_ADDED_DEAD
**Summary:** Another `SDL_FillRect` call in the dialog creation path. Same as Block 18.
**Recommendation:** REMOVE

---

### Block 20: Lines 3434-3436
**Category:** ORIGINAL_DEAD
**Summary:** Commented-out `SV_cpuType` enum typedef. Already commented out in the original (lines 2553-2555).
**Recommendation:** REMOVE

---

### Block 21: Line 3631
**Category:** MIKE_ADDED_DEAD
**Summary:** `//n=1352;` inside `#ifndef __EMSCRIPTEN__` block. A debug override Mike wrote then commented out.
**Recommendation:** REMOVE

---

### Block 22: Lines 3666, 3673-3680
**Category:** MIKE_ADDED_DEAD
**Summary:** `//mptr=(byte *)malloc(an*al);` and a multi-line `/* ... */` block with Emscripten graph loading code that Mike wrote and abandoned. Includes dead `printf` and incomplete palette adaptation logic.
**Recommendation:** REMOVE

---

### Block 23: Line 3747
**Category:** LEGITIMATE
**Summary:** `//read_mouse();` in `inicializa_entorno()`. In the original (line 2730), `read_mouse()` was part of the line `svmode(); set_dac(dac); read_mouse();`. Mike split the initialization and this comment marks where it used to be. Harmless but could be removed.
**Recommendation:** KEEP (marginal; marks original code location)

---

### Block 24: Line 3795
**Category:** ORIGINAL_DEAD
**Summary:** `//  EndSound();` in `finalizacion()`. Already commented out in original (line 2766).
**Recommendation:** REMOVE

---

### Block 25: Lines 3849, 3874-3875
**Category:** MIKE_ADDED_DEAD
**Summary:** In `determina_unidades()`, two commented-out blocks: `GetDiskFreeSpaceEx` call and a `printf` for drive info. Mike wrote this Win32 drive enumeration code then commented out the space/debug parts.
**Recommendation:** REMOVE

---

### Block 26: Lines 4139-4141, 4148
**Category:** ORIGINAL_DEAD
**Summary:** In `_process_items()`: three lines checking ESC ascii and `// tecla();`. All already commented out in original (lines 3033-3035, 3042).
**Recommendation:** REMOVE

---

### Block 27: Line 4369
**Category:** ORIGINAL_DEAD
**Summary:** `//        get[strlen(get)-1]=0;` in `get_input()`. Already commented out in original (line 3261). Was replaced by the `strcpy(&get[get_pos-1],...)` approach.
**Recommendation:** REMOVE

---

### Block 28: Lines 4424, 4428, 4431
**Category:** ORIGINAL_DEAD
**Summary:** Three commented `wwrite_in_box` / `wbox_in_box` calls in `get_input()`. Already commented out in original (lines 3314, 3318, 3321). Were replaced with scroll-aware versions.
**Recommendation:** REMOVE

---

### Block 29: Lines 4659-4660, 4663-4664, 4695-4696, 4699-4700, 4719-4723
**Category:** MIKE_ADDED_DEAD
**Summary:** In `Load_Cfgbin()`: four pairs of `memcpy` lines with alternative color palettes that Mike tried then commented out, plus a debug `for` loop printing hex values. The original had different single-line `memcpy` calls (lines 3546, 3576). Mike changed the palette values and left the old attempts commented.
**Recommendation:** REMOVE

---

### Block 30: Lines 4843, 4950, 5011
**Category:** ORIGINAL_DEAD
**Summary:** Three single-line comments: `// salir_del_entorno=1;` (original line 3710), `// printf("deleting %s\n",...)` (debug print, not in original -- MIKE_ADDED_DEAD), and `// printf("Check oldpif\n")` (not in original -- MIKE_ADDED_DEAD). All dead debug/test code.
**Recommendation:** REMOVE

---

## Review Items

1. **Block 7 (line 740):** `if (test_video) dialogo(test0)` -- video test dialog that was active in DOS. Decide whether the `test_video` mode serves any purpose in the SDL port; if not, remove both this line and the `test_video` variable.

2. **Block 11 (line 1715):** `check_free()` -- disk space warning on startup. The function body exists and works (Win32 version). Decide whether low-disk-space warnings should be shown in the SDL port.

3. **Block 12 (lines 1736-1749):** `command_path()` body -- the entire `shell()` function chain is DOS-only dead code. Consider removing `command_path()` and `shell()` entirely rather than leaving a stub that always returns NULL.

4. **Block 14/15 (lines 2098, 2102, 2146):** Window animation (`exploding_windows`/`extrude`) -- these were active in DOS but Mike disabled them. The `exploding_windows` config still exists. Decide whether to restore window animation or remove these vestiges.
