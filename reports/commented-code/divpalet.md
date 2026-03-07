# divpalet.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 6 |
| Total commented-out lines | ~12 |
| Safe to remove | 5 blocks, ~11 lines |
| Needs review | 1 block, ~1 line |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 880-883
**Category:** ORIGINAL_DEAD
**Summary:** A four-line `do/while` loop that iterated through all windows to call `RemapAllFiles()` one at a time. Already commented out in DOS original (lines 865-868 of DIVPALET.CPP). Was replaced by the `for` loop on lines 874-878 which does the same thing more correctly (decrementing `n` when a remap succeeds, to handle index shifts).
**Recommendation:** REMOVE

---

### Block 2: Line 903
**Category:** ORIGINAL_DEAD
**Summary:** `wrectangulo(v.ptr,an,al,c2,2,10,65*2+3,65*2+3);` in `ordena1()` -- drawing a rectangle around the palette sort preview. Already commented out in DOS original (line 888). Was replaced by the `wbox` call on line 904 that draws a slightly different region.
**Recommendation:** REMOVE

---

### Block 3: Line 909
**Category:** ORIGINAL_DEAD
**Summary:** `wbox(v.ptr,an,al,c2,2,10+132,65*2+3,15);` in `ordena1()` -- drawing a box below the palette grid. Already commented out in DOS original (line 894). Unused UI element that was removed during development.
**Recommendation:** REMOVE

---

### Block 4: Lines 1175-1177
**Category:** ORIGINAL_DEAD
**Summary:** Three commented-out global variable declarations: `byte * pal;`, `byte * paleta;`, `byte * dist;` with size comments. Already commented out in DOS original (lines 1159-1161). These were replaced by local variables inside `crear_paleta()` to avoid permanent memory allocation.
**Recommendation:** REMOVE

---

### Block 5: Line 1324
**Category:** ORIGINAL_DEAD
**Summary:** `//byte col_tapiz[65];` -- unused array for desktop wallpaper colors. Already commented out in DOS original (line 1307). Never used anywhere in the codebase.
**Recommendation:** REMOVE

---

### Block 6: Line 1367
**Category:** MIKE_REMOVED
**Summary:** `//    case 4: descomprime_JPG(temp2,temp,0,lon); break;` in `preparar_tapiz()`. This was active in the DOS original (line 1349 of DIVPALET.CPP). Mike commented it out, likely because JPEG decompression support was disabled or unavailable in the SDL2 port (the `#ifdef JPGLIB` guard in divforma.c and the `jmp_buf` usage suggest JPEG support is partially stubbed).
**Recommendation:** REVIEW -- JPG wallpaper loading is disabled. If JPEG support is restored in divforma.c, this line should be uncommented.

---

## Review Items

1. **Block 6 (line 1367):** The desktop wallpaper loader skips JPEG files. The `es_JPG()` check on line 1349 still detects JPEGs (setting `x=4`), but the decompression case is commented out, so a JPEG wallpaper would be detected but then fall through to "no format matched" (since `x=4` and only cases 1-3 execute). This means selecting a JPG as desktop wallpaper silently fails. Low priority since JPEG support appears broadly disabled.
