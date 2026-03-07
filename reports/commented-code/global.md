# global.h - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 8 |
| Total commented-out lines | ~13 |
| Safe to remove | 7 blocks, ~12 lines |
| Needs review | 0 blocks |
| Keep | 1 block, ~1 line |

## Blocks

### Block 1: Line 12
**Category:** ORIGINAL_DEAD
**Summary:** `//#define SHARE` -- the SHARE macro enables the demo/trial version build. Already commented out in original (line 3 of GLOBAL.H: `//#define SHARE`).
**Recommendation:** REMOVE

---

### Block 2: Lines 40-43
**Category:** MIKE_REMOVED
**Summary:** Four DOS-specific includes commented out: `i86.h`, `bios.h`, `dos.h`, `conio.h`. These were active in the original (lines 15-18). Mike commented them out because they are Watcom C / DOS-only headers not available on modern compilers.
**Recommendation:** REMOVE (these headers will never be needed again)

---

### Block 3: Lines 58-59
**Category:** MIKE_REMOVED
**Summary:** Two more DOS-specific includes commented out: `graph.h`, `direct.h`. Active in original (lines 24-25). Same reason as Block 2.
**Recommendation:** REMOVE

---

### Block 4: Lines 524-525
**Category:** ORIGINAL_DEAD
**Summary:** Two commented GLOBAL_DATA declarations for desktop wallpaper: `desk_r,desk_g,desk_b,desk_tile` and `desk_file`. Already commented out in original (lines 460-461). These were superseded by the `_gcolor t_gama[9]` system in the SetupFile struct.
**Recommendation:** REMOVE

---

### Block 5: Line 634
**Category:** ORIGINAL_DEAD
**Summary:** `//GLOBAL_DATA struct tipo_undo tundo[max_undos];` -- the old static array declaration. Already commented out in original (line 552). Was replaced by dynamic allocation `struct tipo_undo *tundo` (line 633).
**Recommendation:** REMOVE

---

### Block 6: Line 834
**Category:** ORIGINAL_DEAD
**Summary:** `//GLOBAL_DATA int input_len;` -- already commented out in original (line 746). The variable was never used.
**Recommendation:** REMOVE

---

### Block 7: Line 890
**Category:** ORIGINAL_DEAD
**Summary:** `//#define GRABADORA // TAPE RECORDER` -- the tape recorder (screen capture) subsystem macro. Already commented out in original (line 802). The `#ifdef GRABADORA` block on lines 892-894 is already dead.
**Recommendation:** REMOVE (both the define and the `#ifdef GRABADORA` / `#endif` block on lines 892-894)

---

### Block 8: Line 929
**Category:** LEGITIMATE
**Summary:** `//        SAMPLE *sample;` inside the `pcminfo` struct. In the original (line 830), `SAMPLE *sample` was active (JUDAS sound library type). Mike replaced it with `char *sample` (line 930) because the JUDAS `SAMPLE` type may not be available in all build configs. The comment documents what was replaced.
**Recommendation:** KEEP (documents the original JUDAS type that was replaced)

---

## Review Items

None. All blocks in global.h are straightforward: either already dead in the original DOS source, or DOS-specific headers that will never be needed.
