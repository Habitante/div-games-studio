# judasraw.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 3 |
| Total commented-out lines | ~11 |
| Safe to remove | 3 blocks, ~11 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 7
**Category:** MIKE_ADDED_DEAD
**Summary:** `//#include <mem.h>` -- DOS-only header commented out during SDL port.
**Recommendation:** REMOVE

---

### Block 2: Lines 35-40
**Category:** MIKE_REMOVED
**Summary:** `/* if (judas_device == DEV_NOSOUND) { return(SI); } */` in `judas_loadrawsample()`. Was active in original (lines 31-34). Mike commented it out because the SDL port doesn't use `judas_device`.
**Recommendation:** REMOVE

---

### Block 3: Lines 126-131
**Category:** MIKE_REMOVED
**Summary:** Same DEV_NOSOUND check in `judas_loadrawsample_mem()`. Was active in original (lines 106-109). Same rationale as Block 2.
**Recommendation:** REMOVE
