# runtime/divmixer.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 6 |
| Total commented-out lines | ~60 |
| Safe to remove | 6 blocks, ~60 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 32-41
**Category:** ORIGINAL_DEAD
**Summary:** GUSMAX case branch inside SetMasterVolume switch, commented out with "OJO !!!" by Daniel in the original DOS source. GUSMAX hardware support was never finished.
**Recommendation:** REMOVE

---

### Block 2: Lines 72-82
**Category:** ORIGINAL_DEAD
**Summary:** GUSMAX case branch inside SetVocVolume switch, identical pattern to Block 1. Commented out with "OJO !!!" in the original.
**Recommendation:** REMOVE

---

### Block 3: Lines 112-122
**Category:** ORIGINAL_DEAD
**Summary:** GUSMAX case branch inside SetCDVolume switch, identical pattern. Commented out with "OJO !!!" in the original.
**Recommendation:** REMOVE

---

### Block 4: Lines 155-163
**Category:** ORIGINAL_DEAD
**Summary:** Old inline assembly (`asm mov dx,...`) for MIX_SetInput, replaced by C `outp()` calls in the original DOS source. The assembly was the previous implementation, kept as reference.
**Recommendation:** REMOVE

---

### Block 5: Lines 176-192
**Category:** ORIGINAL_DEAD
**Summary:** Old inline assembly for MIX_GetVolume, replaced by C `outp()`/`inp()` calls in the original DOS source. Same pattern as Block 4.
**Recommendation:** REMOVE

---

### Block 6: Lines 206-217
**Category:** ORIGINAL_DEAD
**Summary:** Old inline assembly for MIX_SetVolume, replaced by C `outp()` calls in the original DOS source. Same pattern as Blocks 4-5.
**Recommendation:** REMOVE

---

## Notes

All 6 blocks are identical between the current file and the original DOS source (DIVMIXER.CPP). Mike's SDL2 port added `#ifdef DOS` guards around the function bodies (lines 9, 48, 89, 144, 152, 169, 199, 222, 229) but did not touch the commented-out blocks themselves. The entire file is DOS-only hardware mixer code that is dead on SDL2 platforms.
