# divpaint.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 12 |
| Total commented-out lines | ~185 |
| Safe to remove | 12 blocks, ~185 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 239
**Category:** ORIGINAL_DEAD
**Summary:** Old code to open toolbar on click, replaced by undo logic on lines 241-243.
**Recommendation:** REMOVE

---

### Block 2: Lines 572-574
**Category:** ORIGINAL_DEAD
**Summary:** Three alternative Bezier tension formulas, replaced by tension variable. Daniel's "OJO" comment confirms experimental.
**Recommendation:** REMOVE

---

### Block 3: Lines 997-999
**Category:** ORIGINAL_DEAD
**Summary:** Three lines of block delete code (box2mab/mab_delete), replaced by move/copy.
**Recommendation:** REMOVE

---

### Block 4: Line 1139
**Category:** MIKE_ADDED_DEAD
**Summary:** Debug printf for cclock/system_clock.
**Recommendation:** REMOVE

---

### Block 5: Line 1505
**Category:** ORIGINAL_DEAD
**Summary:** Commented-out set_mab call, handled inside if/else branches below.
**Recommendation:** REMOVE

---

### Block 6: Line 3306
**Category:** ORIGINAL_DEAD
**Summary:** Old write() call for toolbar number display, replaced by wwrite().
**Recommendation:** REMOVE

---

### Block 7: Lines 4771-4772
**Category:** ORIGINAL_DEAD
**Summary:** Two for loops drawing rectangle border manually, replaced by wrectangulo().
**Recommendation:** REMOVE

---

### Block 8: Line 4834
**Category:** ORIGINAL_DEAD
**Summary:** Hardcoded 320-pixel width for loop, replaced by wbox().
**Recommendation:** REMOVE

---

### Block 9: Lines 4875-4967
**Category:** ORIGINAL_DEAD
**Summary:** Entire select_icon() function (93 lines), replaced by different icon selection.
**Recommendation:** REMOVE

---

### Block 10: Lines 5053-5079
**Category:** ORIGINAL_DEAD
**Summary:** Entire vuelca_barra2() function (27 lines) with ghost transparency. Preceded by "// OJO !!!" on line 5051 (remove that too).
**Recommendation:** REMOVE

---

### Block 11: Lines 5089-5113
**Category:** ORIGINAL_DEAD
**Summary:** Old put_barra() body (25 lines), manual pixel rendering replaced by wput_in_box().
**Recommendation:** REMOVE

---

### Block 12: Lines 5121-5147
**Category:** ORIGINAL_DEAD
**Summary:** Old put_barra_inv() body (27 lines), manual inverted rendering replaced by wput_in_box()+wresalta_box().
**Recommendation:** REMOVE

---

### Additional stray comments to remove
- Line 3971: `//Find: SCO` — editor search bookmark
- Line 4775: `//**** OJO ****` — cryptic warning with no context
- Line 5051: `// OJO !!!` — goes with Block 10
