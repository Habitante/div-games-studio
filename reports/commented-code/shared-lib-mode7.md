# shared/lib/mode7.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 8 |
| Total commented-out lines | ~27 |
| Safe to remove | 8 blocks, ~27 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 119-128, 132-134, 142-144
**Category:** DEAD_CODE
**Summary:** Commented-out lines in `draw_object()` that are translated from a BASIC-like pseudocode (BlitzMax or similar). They describe the algorithm in English (`' The Object in this Case is at a fixed position...`, `' use a rotation transformation...`, `' calculate the screen coordinates...`). Mixed with these are commented-out `// SDL` labels. These are porting notes from the original BASIC source, now redundant since the C implementation is complete.
**Recommendation:** REMOVE

---

### Block 2: Lines 153-158
**Category:** DEAD_CODE
**Summary:** Three commented-out function calls in `draw_object()`: `zoomSurface(...)`, `SDL_BlitSurface(out, ...)`, and `LoadImage`/`DrawImageRect` (BASIC pseudocode). The `zoomSurface` and `SDL_BlitSurface` were the intended rendering path, but `draw_object` currently computes position/size and does nothing with them (it only `printf`s the rect on line 149). The BASIC lines are from the original source being ported.
**Recommendation:** REMOVE

---

### Block 3: Lines 164, 168, 172-173, 175-177, 194, 199-201, 205, 209, 212-213, 217, 221
**Category:** DEAD_CODE
**Summary:** Approximately 15 lines of BASIC pseudocode comments scattered throughout `mode_7()` (e.g., `' current screen position`, `' the distance And horizontal scale...`, `' calculate the starting position`). These are the original BASIC algorithm description that was ported to C. The C code directly below each comment makes these redundant.
**Recommendation:** REMOVE

---

### Block 4: Lines 214-215
**Category:** DEAD_CODE
**Summary:** Two commented-out lines in the inner loop of `mode_7()`: `putpixel(bmp,screen_x,screen_y, getpixel(tile,...))`. These were the original generic pixel access calls, replaced by the faster `SDL_put_pixel32`/`SDL_get_pixel32` inlined versions on lines 218-219.
**Recommendation:** REMOVE

---

### Block 5: Lines 302-303, 308-309
**Category:** DEAD_CODE
**Summary:** Commented-out angle adjustment expressions in `test_mode_7()` key handling (e.g., `(int)(angle - 3) & 0xFFFFFF`, `angle=(angle-0.03)`). These are earlier float/fixed-point angle approaches that were replaced by the `+= 0.01f` / `-= 0.01f` increments.
**Recommendation:** REMOVE

---

### Block 6: Lines 345-353
**Category:** DEAD_CODE
**Summary:** A `/* ... */` block comment containing BASIC pseudocode for keyboard control instructions (e.g., `If (KeyDown(KEY_X)) params.space_z :- 5`). These are the original BASIC key handlers that were already ported to the C `switch` statement above them.
**Recommendation:** REMOVE

---

### Block 7: Lines 361-365, 368, 372, 377-385, 388
**Category:** DEAD_CODE
**Summary:** Multiple commented-out lines in the main loop of `test_mode_7()`: disabled `x+=dx`/`y+=dy` movement updates, disabled `draw_object` call, a `/* ... */` block with BASIC DrawText calls and a second `draw_object`, and a commented-out `Flip()` BASIC call. The movement disabling means the camera is static (only angle rotates on line 367). The DrawText block is untranslated BASIC.
**Recommendation:** REMOVE

---

### Block 8: Line 244
**Category:** DEAD_CODE
**Summary:** Commented-out BASIC line `'pal:PALETTE` in `test_mode_7()`. A leftover from the original BASIC source indicating a palette variable declaration.
**Recommendation:** REMOVE

---

## Review Items

None -- all blocks are clearly dead code from an incomplete BASIC-to-C port of a standalone Mode 7 test program. This entire file (`mode7.c`) is a standalone SDL1.2 demo with its own `main()` function. It is not compiled as part of the DIV build (it uses `SDL_Flip`, `SDL_SetVideoMode`, `SDL_DisplayFormat` -- all SDL1.2-only APIs). Consider whether this file should be removed entirely from the repository.
