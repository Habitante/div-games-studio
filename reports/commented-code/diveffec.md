# diveffec.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 7 |
| Total commented-out lines | ~52 |
| Safe to remove | 7 blocks, ~52 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 234-235
**Category:** ORIGINAL_DEAD
**Summary:** Two commented-out lines setting `v.an=128; v.al=90+5;` in Explode1(). Also commented out in the original DOS source (lines 223-224). These duplicate values already set in Explode0(); the comments are leftover from early layout experimentation.
**Recommendation:** REMOVE

---

### Block 2: Line 239
**Category:** ORIGINAL_DEAD
**Summary:** `//wbox(v.ptr,an,al,c0,90,12,1,44);` -- commented-out vertical divider line in Explode1(). Also commented out in the original (line 228). Old UI layout element removed during development.
**Recommendation:** REMOVE

---

### Block 3: Line 241
**Category:** ORIGINAL_DEAD
**Summary:** `//wbox(v.ptr,an,al,c0,90,10,1,al-30);` -- another commented-out vertical divider in Explode1(). Also commented out in the original (line 230). Same as above.
**Recommendation:** REMOVE

---

### Block 4: Line 244
**Category:** ORIGINAL_DEAD
**Summary:** `//wwrite(v.ptr,an,al,115-21,11,0,texto[308],c3);` -- commented-out label draw in Explode1(). Also commented out in the original (line 233). Old UI label removed when the color picker was replaced by the gradient bar.
**Recommendation:** REMOVE

---

### Block 5: Lines 250-258
**Category:** ORIGINAL_DEAD
**Summary:** `/* ... */` block drawing three color swatches (exp_Color0/1/2) in Explode1(). Also commented out in the original (lines 239-247). This was the old 3-color picker UI, replaced by the gradient bar system.
**Recommendation:** REMOVE

---

### Block 6: Lines 306-347
**Category:** ORIGINAL_DEAD
**Summary:** `/* ... */` block containing mouse-click handlers for the three color swatch regions in Explode2(), opening Selcolor0 dialogs to pick exp_Color0/1/2. Also commented out in the original (lines 295-336). Superseded by the gradient picker (`gama0`) below it.
**Recommendation:** REMOVE

---

### Block 7: Line 430 + Lines 440-473
**Category:** ORIGINAL_DEAD
**Summary:** In GenExplodes(): `//float r,g,b,incr,incg,incb;` (line 430) declares variables used only inside the `/* ... */` block (lines 440-473) that built a color gradient from exp_Color2 to exp_Color1 to exp_Color0 via find_color(). Both the declaration and block are also commented out in the original (lines 419, 429-462). Replaced by the `exp_colores[x/2]` lookup on line 439.
**Recommendation:** REMOVE

---

## Notes

All 7 blocks of commented-out code in diveffec.c are classified as ORIGINAL_DEAD -- they were already commented out in the original DOS source, character-for-character identical (modulo encoding). They represent two generations of the explosion color-picking UI: an old 3-color swatch system that was replaced by a gradient bar during the original DOS development. No blocks were introduced or modified by Mike during the SDL2 port.
