# divvideo.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 20 |
| Total commented/dead lines | ~126 (46 commented + ~80 in `#ifdef NOTYET` blocks) |
| Safe to remove | 19 blocks, ~125 lines |
| Needs review | 0 blocks |
| Keep | 1 block, ~1 line |

## Blocks

### Block 1: Lines 7-8
**Category:** MIKE_REMOVED
**Summary:** Two DOS-specific includes commented out: `inc\svga.h` and `inc\vesa.h`. Were active in original (lines 7-8). These provided DOS VESA/SVGA register-level video access. Replaced by SDL2.
**Recommendation:** REMOVE

---

### Block 2: Line 32
**Category:** MIKE_REMOVED
**Summary:** `//byte * vga = (byte *) 0xA0000;` -- DOS VGA framebuffer pointer. Was active in original (line 19). Replaced by `SDL_Surface *vga` (line 34).
**Recommendation:** REMOVE

---

### Block 3: Line 113
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("retrazo (vsync)\n");` -- debug print Mike added then commented out in `retrazo()`.
**Recommendation:** REMOVE

---

### Block 4: Lines 137, 140
**Category:** MIKE_ADDED_DEAD
**Summary:** Commented-out `if(vga->format->BitsPerPixel==8)` guard around `OSDEP_SetPalette` in `set_dac()`. Mike wrote and then removed the bit-depth check, making palette setting unconditional.
**Recommendation:** REMOVE

---

### Block 5: Line 166
**Category:** LEGITIMATE
**Summary:** `//    SDL_SetAlpha(source, 0, 0);` inside `copy_surface()`, preceded by a multi-line `/* */` comment explaining why it was needed. The explanation comment (lines 161-165) is useful documentation about SDL surface behavior. The commented call itself is part of that explanation.
**Recommendation:** KEEP (the whole block lines 161-166 is a documentation comment about SDL alpha behavior)

---

### Block 6: Line 174
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("TODO - Set video mode (%dx%d)\n",...)` -- debug print in `svmode()`.
**Recommendation:** REMOVE

---

### Block 7: Lines 191-202
**Category:** MIKE_ADDED_DEAD
**Summary:** Twelve lines of commented-out SDL surface management in `svmode()`: `SDL_FreeSurface`, `SDL_SetAlpha`, `SDL_DisplayFormat`, `SDL_MapRGB`, `SDL_FillRect`, `SDL_SetColorKey`. Mike wrote these as part of his initial SDL port attempt using surface copy/colorkey, then abandoned the approach.
**Recommendation:** REMOVE

---

### Block 8: Line 302
**Category:** MIKE_ADDED_DEAD
**Summary:** `//SDL_FreeSurface(copia_surface);` in `rvmode()`. Mike's abandoned surface cleanup.
**Recommendation:** REMOVE

---

### Block 9: Lines 327, 341
**Category:** MIKE_ADDED_DEAD
**Summary:** Two `//printf("y:%d n: %d\n",y,n+1/n+3);` debug prints in `volcadosdlp()`.
**Recommendation:** REMOVE

---

### Block 10: Line 353
**Category:** MIKE_ADDED_DEAD
**Summary:** `//SDL_UpdateRect(vga,scan[n+2],y,scan[n+3],1);` -- dead SDL1 call in `volcadosdlp()`, replaced by the `OSDEP_UpdateRect` approach.
**Recommendation:** REMOVE

---

### Block 11: Line 357
**Category:** MIKE_ADDED_DEAD
**Summary:** `//    q+=vga_an;` in `volcadosdlp()` -- Mike replaced this with `q+=vga->pitch;` (line 358).
**Recommendation:** REMOVE

---

### Block 12: Lines 367-368
**Category:** MIKE_ADDED_DEAD
**Summary:** `//SDL_Flip(vga);` and `//volcadosdl(p);` -- dead SDL1 flip calls at the end of `volcadosdlp()`. Replaced by `OSDEP_Flip`.
**Recommendation:** REMOVE

---

### Block 13: Line 391
**Category:** MIKE_ADDED_DEAD
**Summary:** `//SDL_FillRect(vga, NULL, 0);` in `volcadosdl()`. Dead SDL surface clear.
**Recommendation:** REMOVE

---

### Block 14: Lines 471-472
**Category:** MIKE_ADDED_DEAD
**Summary:** Two commented lines at end of `volcadosdl()`: `SDL_MapRGB` and `SDL_FillRect` on `copia_surface`. Dead SDL1 surface operations.
**Recommendation:** REMOVE

---

### Block 15: Lines 536, 552, 556
**Category:** MIKE_ADDED_DEAD
**Summary:** Three debug prints: `//printf("partial dump\n")`, `//SDL_Flip(vga)`, `//printf("complete dump\n")` in the 320x200 dump functions.
**Recommendation:** REMOVE

---

### Block 16: Lines 570, 619
**Category:** MIKE_ADDED_DEAD
**Summary:** Two `//printf("divvideo.cpp - volcadopsvga/volcadocsvga\n")` debug prints at the top of the SVGA dump functions.
**Recommendation:** REMOVE

---

### Block 17: Lines 571, 615, 620, 632
**Category:** MIKE_REMOVED
**Summary:** Four `#ifdef NOTYET` bracket comments (`//#ifdef NOTYET`, `//#endif`) in `volcadopsvga()` and `volcadocsvga()`. Mike initially wrapped the DOS bank-switching code in `#ifdef NOTYET` then partially unwrapped it, leaving these commented preprocessor directives as dead markers. The code between them is active (linear mode memcpy path works on SDL).
**Recommendation:** REMOVE (these are stale `#ifdef` wrappers that no longer gate anything)

---

### Block 18: Lines 590, 592, 595, 605, 607, 610, 626
**Category:** MIKE_REMOVED
**Summary:** Seven commented-out `SV_setBank()` calls in `volcadopsvga()` and `volcadocsvga()`. These were active in the original DOS source (the SVGA bank-switching logic). Mike commented them out because SDL does not use SVGA bank switching. The surrounding memcpy code still runs for the linear mode path.
**Recommendation:** REMOVE (SVGA bank switching is permanently dead under SDL)

---

### Block 19: Lines 209-268 (svmode #ifdef NOTYET block)
**Category:** MIKE_REMOVED
**Summary:** Large `#ifdef NOTYET` block containing the entire original DOS VESA/mode-X video mode setup (60 lines). This was the active `svmode()` body in the original (lines 87-145). Mike wrapped it in `#ifdef NOTYET` and wrote the SDL replacement above it (lines 173-207). The `NOTYET` macro is never defined.
**Recommendation:** REMOVE (entire block, plus the `#ifdef NOTYET` wrappers -- the SDL replacement is the permanent code)

---

### Block 20: Lines 273-292 (svmodex #ifdef NOTYET block)
**Category:** MIKE_REMOVED
**Summary:** The original `svmodex()` body wrapped in `#ifdef NOTYET`. Was active in original (lines 147-167). Mode-X is a DOS VGA register-programming technique that has no SDL equivalent.
**Recommendation:** REMOVE (the entire function `svmodex()` could be removed since it just prints "TODO" and returns)

---

## Review Items

1. **Blocks 17-18 (SVGA bank-switching in volcadopsvga/volcadocsvga):** The SVGA dump functions `volcadopsvga()` and `volcadocsvga()` still contain active code paths for linear-mode memcpy. However, these functions are only called when `modovesa` is true, which is always set to 1 in the SDL port's `svmode()`. The bank-switching paths within them are dead. Consider whether these entire functions should be simplified to just `volcadosdl()`/`volcadosdlp()` since all rendering goes through SDL anyway. The whole `volcado()` switch-case dispatching DOS video modes (320x200, mode-X, SVGA) is obsolete under SDL.
