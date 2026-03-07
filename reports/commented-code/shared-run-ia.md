# ia.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 3 |
| Total commented-out lines | ~7 |
| Safe to remove | 1 block, ~5 lines |
| Needs review | 0 blocks |
| Keep | 2 blocks |

## Blocks

### Block 1: Lines 449-455
**Category:** ORIGINAL_DEAD
**Summary:** Commented-out tile-size halving optimization in `puede_ir()`. Already commented out in original DOS source (lines 445-451 of IA.CPP). The code would halve coordinates for large tiles but was disabled by Daniel.
**Recommendation:** REMOVE

---

### Block 2: Lines 563, 567
**Category:** MIKE_ADDED_DEAD
**Summary:** Two debug printf lines in `path_free()`: `printf("got to 559\n")` and `printf("m is %d %d %d\n",...)`. Not in original.
**Recommendation:** REMOVE

---

### Block 3: Lines 78-83
**Category:** LEGITIMATE
**Summary:** `#ifdef DIV2` / `#else` conditional for bounds checking -- the `#else` branch has different error handling. This is active conditional code, not commented-out code.
**Recommendation:** KEEP (this is a preprocessor conditional, not commented-out code)
