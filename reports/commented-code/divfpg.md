# divfpg.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 6 |
| Total commented-out lines | ~43 |
| Safe to remove | 6 blocks, ~43 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

Note: Three inline comments on lines 182, 232, and 942 (`//Error: ...`) were examined but are documentation, not commented-out code. They are excluded from the count.

## Blocks

### Block 1: Lines 104-132
**Category:** ORIGINAL_DEAD
**Summary:** A large `/* */` block containing palette-change handling logic for FPG windows (comparing old/new DAC, calling RemapAllFiles/RefPalAndDlg). Already commented out in the DOS original (lines 97-124 of DIVFPG.CPP). The code was replaced by the `NewDacLoaded` check at the top of FPG2(). Contains the comment `//CACA` (Daniel's debug marker).
**Recommendation:** REMOVE

---

### Block 2: Line 313
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("v.aux %x\n",v.aux);` -- debug printf added by Mike. Not in DOS original (original line 304 has no comment).
**Recommendation:** REMOVE

---

### Block 3: Lines 1401-1402
**Category:** ORIGINAL_DEAD
**Summary:** Two lines of old scroll-up logic for the FPG listbox: `if (l->zona==2 && (mouse_b&1)) { if (old_mouse_b&1) { retrazo(); retrazo(); retrazo(); retrazo(); }`. Already commented out in the DOS original (lines 1236-1237 of DIVFPG.CPP). Daniel replaced the simple scroll logic with a pause-aware version on the lines below. Mike later added mouse wheel support but these commented lines were already dead.
**Recommendation:** REMOVE

---

### Block 4: Line 1404
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("zona: %d\n",l->zona);` -- debug printf by Mike. Not in DOS original (DOS original has `//---` markers but no printf).
**Recommendation:** REMOVE

---

### Block 5: Lines 1428-1429
**Category:** ORIGINAL_DEAD
**Summary:** Two lines of old scroll-down logic, same pattern as Block 3. Already commented out in DOS original (lines 1255-1256 of DIVFPG.CPP). Superseded by the pause-aware scroll logic below.
**Recommendation:** REMOVE

---

### Block 6: Lines 1969-1970, 1987, 2010
**Category:** ORIGINAL_DEAD
**Summary:** In `emplazar_map()`: commented-out `if` guard around the scan-height loop body (lines 1969-1970, 1987) and a debug printf `//if(lgraf[lnum].y+lgraf[lnum].al>lmapal) printf(...)` (line 2010). All present identically in DOS original (lines 1757-1758, 1771, 1796 of DIVFPG.CPP).
**Recommendation:** REMOVE

---

## Review Items

None. All commented-out code in this file is either already dead in the DOS original or debug printfs added by Mike.
