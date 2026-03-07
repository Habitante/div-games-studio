# divhelp.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 12 |
| Total commented-out lines | ~30 |
| Safe to remove | 10 blocks, ~26 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 2 blocks |

## Blocks

### Block 1: Line 59
**Category:** ORIGINAL_DEAD
**Summary:** `//free(div_index);` -- commented-out free() in load_index(). Also commented out in the original DOS source (line 50: `//free(index);`). Part of the pattern of deliberately leaked memory throughout this codebase.
**Recommendation:** REMOVE

---

### Block 2: Line 168
**Category:** ORIGINAL_DEAD
**Summary:** `//  wbox(ptr,an,al,c1,2,10,an-12,15);` -- commented-out UI drawing call in help1(). Also commented out at line 148 of the original DOS source. Likely an old layout element that was replaced.
**Recommendation:** REMOVE

---

### Block 3: Line 184
**Category:** ORIGINAL_DEAD
**Summary:** `// tecla();` -- commented-out keyboard polling call in help2(). Also commented out at line 164 of the original DOS source.
**Recommendation:** REMOVE

---

### Block 4: Lines 250-267
**Category:** ORIGINAL_DEAD
**Summary:** `/* ... */` block containing old slider click-to-page logic for the help window scrollbar. Also commented out in the original DOS source (lines 231-248). Superseded by the smooth-drag slider code directly below it.
**Recommendation:** REMOVE

---

### Block 5: Line 380
**Category:** ORIGINAL_DEAD
**Summary:** `// help_xref(backto[a_back],backto[a_back+1]);` -- commented-out back-navigation call. Also commented out in the original DOS source (line 359), with OJO annotation on the line above.
**Recommendation:** REMOVE

---

### Block 6: Line 407
**Category:** ORIGINAL_DEAD
**Summary:** `//free(graf_help[loaded[n_loaded]].ptr);` in help3(). Also commented out in the original (line 386). Deliberate memory leak.
**Recommendation:** REMOVE

---

### Block 7: Line 410
**Category:** ORIGINAL_DEAD
**Summary:** `//free(help_buffer);` in help3(). Also commented out in the original (line 389). Deliberate memory leak.
**Recommendation:** REMOVE

---

### Block 8: Line 625
**Category:** ORIGINAL_DEAD
**Summary:** `//free(h_buffer);` in help(). Also commented out in the original (line 603). Same deliberate memory leak pattern.
**Recommendation:** REMOVE

---

### Block 9: Line 706
**Category:** ORIGINAL_DEAD
**Summary:** `//free(h_buffer);` in help_paint(). Also commented out in the original (line 684). Same pattern.
**Recommendation:** REMOVE

---

### Block 10: Lines 749, 753
**Category:** ORIGINAL_DEAD
**Summary:** `//free(graf_help[...].ptr);` and `//free(help_buffer);` in help_xref(). Also commented out in the original (lines 728, 732). Same pattern.
**Recommendation:** REMOVE

---

### Block 11: Lines 776-777
**Category:** ORIGINAL_DEAD
**Summary:** Two commented-out lines recalculating help_an/help_al in help_xref(). Also commented out in the original (lines 755-756). The recalculation was intentionally disabled so resizing persists across cross-references.
**Recommendation:** REMOVE

---

### Block 12: Line 783
**Category:** ORIGINAL_DEAD
**Summary:** `//free(h_buffer);` in help_xref(). Also commented out in the original (line 762). Same pattern.
**Recommendation:** REMOVE

---

### Block 13: Line 979
**Category:** ORIGINAL_DEAD
**Summary:** `//*di++=0; help_lines++; chars=0;` in tabula_help() under the `{-}` (end-example) handler. Also commented out in the original (line 954). Was replaced by the logic above it.
**Recommendation:** REMOVE

---

### Block 14: Lines 1137-1142
**Category:** ORIGINAL_DEAD
**Summary:** `/* ... */` block containing duplicate variable declarations (`help_an`, `help_al`, `help_buffer`, `help_line`, `help_end`) above vuelca_help(). Also commented out in the original (lines 1112-1117). Appears to have been an old reminder of what globals the function uses.
**Recommendation:** REMOVE

---

### Block 15: Lines 1462-1463
**Category:** ORIGINAL_DEAD
**Summary:** `//free(print_buffer);` and `//free(h_buffer);` in Print_Help(). Also commented out in the original (lines 1433-1434). Same deliberate memory leak pattern.
**Recommendation:** REMOVE

---

## Notes

All 15 blocks of commented-out code in divhelp.c are classified as ORIGINAL_DEAD -- they were already commented out in the original DOS source. The majority (10 of 15) are `//free()` calls, part of a deliberate pattern where Daniel disabled frees to avoid potential double-free or use-after-free issues. No blocks were introduced or modified by Mike during the SDL2 port.

Corrected metrics:

| Metric | Value |
|--------|-------|
| Total blocks | 15 |
| Total commented-out lines | ~30 |
| Safe to remove | 15 blocks, ~30 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 0 blocks |
