# v.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 10 |
| Total commented-out lines | ~30 |
| Safe to remove | 9 blocks, ~28 lines |
| Needs review | 0 blocks |
| Keep | 1 block |

## Blocks

### Block 1: Line 109
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("set paleta\n");` debug print in `set_paleta()`.
**Recommendation:** REMOVE

---

### Block 2: Lines 232-233
**Category:** MIKE_ADDED_DEAD
**Summary:** `printf("setting new video mode...")` preceded by commented-out `#ifdef STDOUTLOG` guard. The printf itself is NOT commented out (active code), but the `#ifdef` guard is commented out, leaving the printf always active.
**Recommendation:** KEEP (the guard is what's commented out -- the printf itself is a separate issue of always-on logging, not commented code)

---

### Block 3: Lines 238-239
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines: `//if(vga) //SDL_FreeSurface(vga);` in `svmode()`. Mike wrote and disabled an SDL surface free before reassigning vga.
**Recommendation:** REMOVE

---

### Block 4: Line 251
**Category:** MIKE_ADDED_DEAD
**Summary:** Trailing comment `// SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);` -- remnant of old SDL1 flags.
**Recommendation:** REMOVE (just a trailing comment of old API flags)

---

### Block 5: Line 411
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("draw screen\n");` debug print in `volcadosdl()`.
**Recommendation:** REMOVE

---

### Block 6: Line 415
**Category:** MIKE_ADDED_DEAD
**Summary:** `//SDL_UpdateRect(vga,0,0,vga_an,vga_al);` -- old SDL1 update call replaced by `OSDEP_Flip()`.
**Recommendation:** REMOVE

---

### Block 7: Line 431
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("%d %d %d\n",game_fps,freloj,ireloj);` debug print in `volcado()`.
**Recommendation:** REMOVE

---

### Block 8: Lines 442-445
**Category:** MIKE_ADDED_DEAD
**Summary:** Commented-out `programRunning` check and extra braces around framecount increment in recording code.
**Recommendation:** REMOVE

---

### Block 9: Lines 652-655
**Category:** ORIGINAL_DEAD
**Summary:** Three commented-out scan array declarations (`scan0[]`, `scan1[]`, `scan[]*4`). Already commented out in original DOS source (lines 428-431 of V.CPP). These were superseded by the active `scan[]` declaration at line 66.
**Recommendation:** REMOVE

---

### Block 10: Lines 1009, 1016
**Category:** ORIGINAL_DEAD
**Summary:** Two commented-out `cprintf()` progress display lines in `crear_ghost()`. Already commented out in original DOS source (lines 770, 777).
**Recommendation:** REMOVE
