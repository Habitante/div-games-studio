# divdsktp.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 31 |
| Total commented-out lines | ~220 |
| Safe to remove | 19 blocks, ~200 lines |
| Needs review | 3 blocks, ~25 lines |
| Keep | 9 blocks |

## Blocks

### Block 1: Line 23
**Category:** LEGITIMATE
**Summary:** Removal note for M3D_info.
**Recommendation:** KEEP

---

### Block 2: Lines 25-26
**Category:** ORIGINAL_DEAD
**Summary:** Two extern declarations (FontPathName, FontName).
**Recommendation:** REMOVE

---

### Block 3: Line 31
**Category:** ORIGINAL_DEAD
**Summary:** Extern declaration for Nombres[256][15].
**Recommendation:** REMOVE

---

### Block 4: Line 33
**Category:** MIKE_REMOVED
**Summary:** //extern on struct _calc — intentional C compilation fix (not dead code).
**Recommendation:** KEEP

---

### Block 5: Lines 72-181
**Category:** ORIGINAL_DEAD
**Summary:** Entire New_DownLoad_Desktop() function (~110 lines), older alternative implementation.
**Recommendation:** REMOVE

---

### Block 6: Line 183
**Category:** MIKE_ADDED_DEAD
**Summary:** //#define SLST — debug toggle for session-list logging.
**Recommendation:** REMOVE

---

### Block 7: Lines 192-239 (scattered #ifdef SLST blocks)
**Category:** MIKE_ADDED_DEAD
**Summary:** ~13 #ifdef SLST blocks throughout DownLoad_Desktop() with debug fprintf calls (~50 lines total).
**Recommendation:** REMOVE

---

### Block 8: Lines 305-369 (scattered // fprintf lines)
**Category:** ORIGINAL_DEAD
**Summary:** ~20 scattered // fprintf(lst,...) debug lines throughout DownLoad_Desktop().
**Recommendation:** REMOVE

---

### Block 9: Lines 375-384
**Category:** ORIGINAL_DEAD
**Summary:** Old DOS pcm save code inside #ifdef NOTYET.
**Recommendation:** REMOVE

---

### Block 10: Lines 385-388
**Category:** ORIGINAL_DEAD
**Summary:** Alternative pcm save approach inside #ifdef NOTYET.
**Recommendation:** REMOVE

---

### Block 11: Lines 371-392
**Category:** MIKE_ADDED_DEAD
**Summary:** #ifdef NOTYET wrapper around case 105 pcm save. Contains Blocks 9-10.
**Recommendation:** REVIEW (PCM session save disabled)

---

### Block 12: Line 393
**Category:** LEGITIMATE
**Summary:** Removal note for case 106 (map3d).
**Recommendation:** KEEP

---

### Block 13: Line 462
**Category:** MIKE_ADDED_DEAD
**Summary:** Debug printf for loading saved session.
**Recommendation:** REMOVE

---

### Block 14: Line 504
**Category:** LEGITIMATE
**Summary:** Removal note for case 9 (menu_mapas3D0).
**Recommendation:** KEEP

---

### Block 15: Line 570
**Category:** ORIGINAL_DEAD
**Summary:** Redundant fread for map data.
**Recommendation:** REMOVE

---

### Block 16: Lines 620, 622
**Category:** MIKE_ADDED_DEAD
**Summary:** //(char *)texto[75] and texto[76] reminder comments.
**Recommendation:** REMOVE

---

### Block 17: Lines 629, 633
**Category:** LEGITIMATE
**Summary:** Explanatory comments in file-reload feature.
**Recommendation:** KEEP

---

### Block 18: Lines 686-705
**Category:** MIKE_ADDED_DEAD
**Summary:** #ifdef NOTYET around case 105 pcm load.
**Recommendation:** REVIEW (PCM session load disabled)

---

### Block 19: Lines 688-695
**Category:** ORIGINAL_DEAD
**Summary:** Old DOS pcm load code inside NOTYET.
**Recommendation:** REMOVE

---

### Block 20: Lines 696-702
**Category:** ORIGINAL_DEAD
**Summary:** Alternative pcm load approach inside NOTYET.
**Recommendation:** REMOVE

---

### Block 21: Lines 712-714
**Category:** MIKE_REMOVED
**Summary:** judascfg_device sound check on OpenDesktopSong.
**Recommendation:** REVIEW (needs SDL2_mixer equivalent)

---

### Block 22: Line 706
**Category:** LEGITIMATE
**Summary:** Removal note for case 106 (map3d).
**Recommendation:** KEEP

---

### Block 23: Lines 740-747
**Category:** ORIGINAL_DEAD
**Summary:** Window deactivation before loading new one.
**Recommendation:** REMOVE

---

### Block 24: Lines 784-791
**Category:** ORIGINAL_DEAD
**Summary:** Dialog centering code.
**Recommendation:** REMOVE

---

### Block 25: Lines 823-831
**Category:** ORIGINAL_DEAD
**Summary:** Duplicate window type check.
**Recommendation:** REMOVE

---

### Block 26: Lines 837-846
**Category:** ORIGINAL_DEAD
**Summary:** Hide foreground windows for dialogs.
**Recommendation:** REMOVE

---

### Block 27: Lines 871-873
**Category:** ORIGINAL_DEAD
**Summary:** Closing brace of Block 26's else-branch.
**Recommendation:** REMOVE

---

### Block 28: Line 884
**Category:** MIKE_ADDED_DEAD
**Summary:** SDL_FillRect call, superseded by memset.
**Recommendation:** REMOVE

---

### Block 29: Lines 903-905
**Category:** ORIGINAL_DEAD
**Summary:** Busy-wait loop draining mouse/keyboard input.
**Recommendation:** REMOVE

---

### Block 30: Line 998
**Category:** LEGITIMATE
**Summary:** Removal note for nuevo_mapa3d_carga.
**Recommendation:** KEEP

---

### Block 31: Various section headers
**Category:** LEGITIMATE
**Summary:** Box-drawing separator comments.
**Recommendation:** KEEP

## Review Items

1. **Block 11 (lines 371-392)**: #ifdef NOTYET around PCM save — entire case 105 disabled
2. **Block 18 (lines 686-705)**: #ifdef NOTYET around PCM load — entire case 105 disabled
3. **Block 21 (lines 712-714)**: judascfg_device check removed — needs SDL2_mixer equivalent
