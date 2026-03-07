# runtime/debug/d.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 17 |
| Total commented-out lines | ~63 |
| Safe to remove | 15 blocks, ~54 lines |
| Needs review | 0 blocks |
| Keep | 2 blocks, ~9 lines |

## Blocks

### Block 1: Lines 146-148
**Category:** ORIGINAL_DEAD
**Summary:** Three `#define` macros for `mouse_x`, `mouse_y`, `mouse_b` referencing `cbd.mouse_cx/dx/bx`. Already commented out in the original DOS source (D.CPP lines 58-60). These were superseded by the integer variables declared at line 141.
**Recommendation:** REMOVE

---

### Block 2: Lines 1399-1410
**Category:** ORIGINAL_DEAD
**Summary:** An alternative implementation of `restaura_tapiz()` using `memcpy` per scanline. Already commented out in the original DOS source (D.CPP lines 1300-1311). The active version above (lines 1376-1397) uses a per-pixel copy loop instead.
**Recommendation:** REMOVE

---

### Block 3: Lines 1580-1582
**Category:** ORIGINAL_DEAD
**Summary:** A dead `if (ascii==0x1b)` check inside `_process_items()`. Already commented out in the original DOS source (D.CPP lines 1481-1483). The ESC handling was moved/restructured below at line 1594.
**Recommendation:** REMOVE

---

### Block 4: Line 1589
**Category:** ORIGINAL_DEAD
**Summary:** A single `// tecla();` call inside `_process_items()`. Already commented out in the original (D.CPP line 1490). The keyboard input is handled differently in the enclosing branch.
**Recommendation:** REMOVE

---

### Block 5: Line 1783
**Category:** ORIGINAL_DEAD
**Summary:** `// get[strlen(get)-1]=0;` inside `get_input()`, a backspace handler that truncated the last character. Already commented out in the original (D.CPP line 1684). Was replaced by the `strcpy(&get[get_pos-1],...)` approach that handles cursor-position-aware deletion.
**Recommendation:** REMOVE

---

### Block 6: Line 1832
**Category:** ORIGINAL_DEAD
**Summary:** A commented-out `wwrite_in_box` call in `get_input()`. Already commented out in the original (D.CPP line 1733). Was replaced by the version on the next line that uses pointer arithmetic for scrolling text fields.
**Recommendation:** REMOVE

---

### Block 7: Line 1836
**Category:** ORIGINAL_DEAD
**Summary:** A commented-out cursor-position calculation (`if (strlen(get))...`) in `get_input()`. Already commented out in the original (D.CPP line 1737). Was replaced by the `x=l+1;` calculation on the next line.
**Recommendation:** REMOVE

---

### Block 8: Line 1839
**Category:** ORIGINAL_DEAD
**Summary:** A commented-out `wbox_in_box` call for cursor rendering. Already commented out in the original (D.CPP line 1739). Was replaced by the version on the line above that uses pointer arithmetic for scrolling.
**Recommendation:** REMOVE

---

### Block 9: Line 2094
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("ERROR: %s\n",(char *)text[texto]);` inside an `#ifdef __EMSCRIPTEN__` block in `e()`. Not present in the original DOS source at all. Mike added the Emscripten block and this debug print, then commented the printf out.
**Recommendation:** REMOVE

---

### Block 10: Lines 2116-2118
**Category:** ORIGINAL_DEAD
**Summary:** DAC palette fade-out loop (`while (now_dacout_r!=dacout_r...)`) in the error dialog function `e()`. Already commented out in the original (D.CPP lines 2001-2003). This was a VGA palette fade effect that was disabled, likely because it caused issues or delays in the error path.
**Recommendation:** REMOVE

---

### Block 11: Lines 2497-2500
**Category:** ORIGINAL_DEAD
**Summary:** Another DAC palette fade-out sequence in `debug()`. Already commented out in the original (D.CPP lines 2382-2385). Same pattern as Block 10 -- VGA palette fade disabled in the debug entry path.
**Recommendation:** REMOVE

---

### Block 12: Lines 3768-3770
**Category:** ORIGINAL_DEAD
**Summary:** Three commented-out variable declarations (`int linea0`, `byte * plinea0`, `int linea_sel`) after a `dialogo()` call. Already commented out in the original (D.CPP lines 3653-3655). These are notes/reminders about what variables the dialog may modify, not actual code.
**Recommendation:** KEEP (these serve as documentation comments about side-effects of `dialogo(lista_procesos0)`)

---

### Block 13: Lines 3914-3917
**Category:** ORIGINAL_DEAD
**Summary:** Four case branches for navigating to Father/SmallBro/BigBro/Son processes in the debugger. Already commented out in the original (D.CPP lines 3798-3801). These were part of a planned feature for process tree navigation that was never activated; the UI buttons (items 5-8) were never added.
**Recommendation:** REMOVE

---

### Block 14: Lines 3953-3965
**Category:** ORIGINAL_DEAD
**Summary:** A `/* ... */` block implementing "Go to Father, Brother or Son" logic that corresponds to the case branches in Block 13. Already commented out in the original (D.CPP lines 3837-3849). Same unrealized feature.
**Recommendation:** REMOVE

---

### Block 15: Line 4035
**Category:** ORIGINAL_DEAD
**Summary:** `//if (linea1) l=linea0=linea1-1; else l=linea0=0;` -- an old line-positioning formula in the debugger code viewer. Already commented out in the original (D.CPP line 3919). Was replaced by `l=linea0=linea1-3;` (showing context above the current line).
**Recommendation:** REMOVE

---

### Block 16: Lines 4347-4356
**Category:** ORIGINAL_DEAD
**Summary:** Eight commented-out variable/declaration lines near the profiler section (lp1, lp_num, lp_ini, lp_select, obj_start, obj_size, f_time). Already commented out in the original (D.CPP lines 4231-4240). These are duplicates of declarations earlier in the file (around lines 249-268) that were left as reference comments near where the profiler code begins.
**Recommendation:** KEEP (these serve as a quick-reference guide to the profiler variables, matching the original intent)

---

### Block 17: Lines 4368, 4395, 4402
**Category:** Mixed -- three single-line items in the profiler/timer area:
- **Line 4368:** `//extern int cpu_type;` -- ORIGINAL_DEAD, already commented in the original (D.CPP line 4252). CPU type detection was dropped.
- **Line 4395:** `//printf("ticks\n");` -- MIKE_ADDED_DEAD, not in original. Mike added a debug printf, then commented it out.
- **Line 4402:** `//outp(0x43, 0x34);` -- ORIGINAL_DEAD, already commented in the original (D.CPP line 4282), with OJO note about timer reprogramming.

**Recommendation:** REMOVE all three lines

---
