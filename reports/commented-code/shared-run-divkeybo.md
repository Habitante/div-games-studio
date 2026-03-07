# divkeybo.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 10 |
| Total commented-out lines | ~35 |
| Safe to remove | 7 blocks, ~22 lines |
| Needs review | 0 blocks |
| Keep | 3 blocks |

## Blocks

### Block 1: Lines 39-40
**Category:** MIKE_REMOVED
**Summary:** `kb_start` and `kb_end` DOS BIOS address pointers (0x41a, 0x41c) commented out. These were active in original (line 31-32) but are DOS-only and correctly guarded by `#ifdef DOS` on line 67 where they are used. Mike commented them out since they are dangerous on non-DOS platforms.
**Recommendation:** REMOVE (the DOS code that uses them is also inside `#ifdef DOS`)

---

### Block 2: Line 63
**Category:** ORIGINAL_DEAD
**Summary:** `if (kbhit()) getch();` -- already commented out in original DOS source (line 54).
**Recommendation:** REMOVE

---

### Block 3: Lines 183-185
**Category:** MIKE_ADDED_DEAD
**Summary:** Debug printf statements in `checkmod()` -- `shift_status = 0;` and `printf("None\n")` commented out. Not in original (function doesn't exist in DOS source).
**Recommendation:** REMOVE

---

### Block 4: Lines 188-189
**Category:** MIKE_ADDED_DEAD
**Summary:** Two commented-out printf lines for NUMLOCK/CAPSLOCK debug output in `checkmod()`.
**Recommendation:** REMOVE

---

### Block 5: Lines 201-215
**Category:** MIKE_ADDED_DEAD
**Summary:** Large block of commented-out printf debug statements for modifier key debugging in `checkmod()`. 14 lines.
**Recommendation:** REMOVE

---

### Block 6: Lines 243-250
**Category:** MIKE_ADDED_DEAD
**Summary:** Commented-out code in SDL2 `PrintEvent` RESIZED handler: `vga_an`/`vga_al` assignments, `EndSound()`, duplicate `vwidth`/`vheight` assignments. Experimental remnants from window resize handling.
**Recommendation:** REMOVE

---

### Block 7: Lines 298-299, 306, 314, 321-324, 335, 360, 371, 375-378, 383, 388-389, 398-399, 405-406, 431, 477
**Category:** MIKE_ADDED_DEAD
**Summary:** Scattered single-line debug printf statements throughout `tecla()` function (event type, keydown/up, scan codes, ascii values, mouse coords). About 15 individual commented-out debug prints.
**Recommendation:** REMOVE

---

### Block 8: Lines 371
**Category:** LEGITIMATE
**Summary:** `//printf("ascii val: %d\n",ascii);` -- single debug comment, trivial.
**Recommendation:** REMOVE (debug leftover)

---

### Block 9: Lines 375, 378
**Category:** MIKE_ADDED_DEAD
**Summary:** `#ifndef GCW` / `#endif` guards commented out around `kbdFLAGS[scan_code]=1;`. The code they guard is now unconditional, which is correct.
**Recommendation:** REMOVE

---

### Block 10: Line 377
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("kbdflags: %d %d\n",...)` debug print.
**Recommendation:** REMOVE

---

## Review Items
None -- all blocks are safe to remove.
