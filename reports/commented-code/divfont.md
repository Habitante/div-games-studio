# divfont.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 7 |
| Total commented-out lines | ~30 |
| Safe to remove | 7 blocks, ~30 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 149-151
**Category:** ORIGINAL_DEAD
**Summary:** Early-return check `if((wmouse_y==-1)&&(wmouse_x==-1)) return;` and a `//ShowText();` call. Already commented out in DOS original (lines 146-148 of DIVFONT.CPP). Was an abandoned mouse-bounds check.
**Recommendation:** REMOVE

---

### Block 2: Line 247
**Category:** ORIGINAL_DEAD
**Summary:** `// fin_dialogo=1;` after `cierra_ventana()`. Already commented out in DOS original (line 244). The window-close path uses `cierra_ventana()` + return instead of setting the dialog-finished flag.
**Recommendation:** REMOVE

---

### Block 3: Lines 409-410
**Category:** ORIGINAL_DEAD
**Summary:** `v.item[2].estado&=-3; v.item[3].estado&=-3;` -- clearing state bits to avoid "out of range" on font size items. Already commented out in DOS original (lines 406-407). The fix was apparently unnecessary.
**Recommendation:** REMOVE

---

### Block 4: Lines 925-935
**Category:** ORIGINAL_DEAD
**Summary:** A `/* */` block containing an older version of the text size calculation loop for Preview0(). Already commented out in DOS original (lines 922-934). Was replaced by the improved loop above it that handles spacing via `spacelen`.
**Recommendation:** REMOVE

---

### Block 5: Lines 1071-1072
**Category:** ORIGINAL_DEAD
**Summary:** `if(!CreateFont(GenCode)) font_generated=1; else font_generated=0;` -- old inverted-logic version of the CreateFont call. Already commented out in DOS original (lines 1068-1069). Replaced by `font_generated=CreateFont(GenCode);` on the line above.
**Recommendation:** REMOVE

---

### Block 6: Lines 1177-1190
**Category:** ORIGINAL_DEAD
**Summary:** A `/* */` block containing an older version of the Preview20() text size calculation. Already commented out in DOS original (lines 1174-1189). Same pattern as Block 4 -- replaced by the improved loop above.
**Recommendation:** REMOVE

---

### Block 7: Lines 1428-1429
**Category:** ORIGINAL_DEAD
**Summary:** `_button(113,7,v.al-14,0); _button(122,v.an-8,v.al-14,2);` -- two button definitions in ShowFont0(). Already commented out in DOS original (the DOS original at this location does not have these lines at all; checking line 1424 of DIVFONT.CPP shows `v.nombre=v.aux;` with no buttons after). These were commented out by Daniel in the original source before the SDL port.
**Recommendation:** REMOVE

---

## Review Items

None. All commented-out code in this file was already dead in the DOS original.
