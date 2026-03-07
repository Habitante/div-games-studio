# s.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 2 |
| Total commented-out lines | ~11 |
| Safe to remove | 0 blocks |
| Needs review | 0 blocks |
| Keep | 2 blocks, ~11 lines |

## Blocks

### Block 1: Lines 321-328
**Category:** ORIGINAL_DEAD
**Summary:** A `/* ... */` block containing an optimization shortcut for `set_scroll()` that Daniel commented out. The comment header reads "Se ha quitado esta optimizacion" ("This optimization has been removed"). The block bypassed the tiled-copy loop with a simple `memcpy` when the scroll fits entirely within the map. Identical to the original S.CPP (lines 291-298).
**Recommendation:** KEEP -- This is a deliberate, labeled optimization removal by the original author. The comment explains *why* it was removed and provides context for anyone revisiting scroll performance. It's only 8 lines.

---

### Block 2: Lines 2010-2013
**Category:** ORIGINAL_DEAD
**Summary:** Three lines of debug visualization code that draws a white pixel at the base of each Mode 7 object. The comment reads "Pixel blanco en la base del objeto". Was already commented out identically in the original S.CPP (lines 1980-1983). This was a visual debugging aid Daniel used during Mode 7 development.
**Recommendation:** KEEP -- This is a labeled debug snippet (only 3 lines of code) that could be useful for anyone debugging Mode 7 sprite positioning. Harmless to leave.
