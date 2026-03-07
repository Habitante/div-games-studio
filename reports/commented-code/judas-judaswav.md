# judaswav.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 4 |
| Total commented-out lines | ~12 |
| Safe to remove | 4 blocks, ~12 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 7-8
**Category:** MIKE_ADDED_DEAD
**Summary:** `#include <mem.h>` and `#include <io.h>` commented out. DOS-only headers removed during SDL port.
**Recommendation:** REMOVE

---

### Block 2: Lines 20-24
**Category:** MIKE_ADDED_DEAD
**Summary:** Block comment containing `#define max_textos`, `Progress()` declaration, and `extern char *texto[]`. Mike added and then commented out a progress bar integration. Not in original.
**Recommendation:** REMOVE

---

### Block 3: Lines 168, 181-186
**Category:** ORIGINAL_DEAD
**Summary:** `// int con, pasos, resto;` (line 168) and `/* if (judas_device == DEV_NOSOUND) { return(SI); } */` (lines 181-186). The variable declaration was already commented out in original (line 159). The DEV_NOSOUND check was active in original (lines 172-175) but Mike commented it out since SDL_mixer handles no-sound differently.
**Recommendation:** REMOVE (line 168 is ORIGINAL_DEAD; lines 181-186 are MIKE_REMOVED but correct for SDL port -- the SDL_mixer path doesn't use judas_device)

---

### Block 4: Lines 487, 501-506
**Category:** ORIGINAL_DEAD / MIKE_REMOVED
**Summary:** Same pattern in `judas_loadwav_mem()`: `// int con, pasos, resto;` (line 487, ORIGINAL_DEAD at line 426 of original) and `/* if (judas_device == DEV_NOSOUND)... */` (lines 501-506, was active in original lines 440-443, MIKE_REMOVED).
**Recommendation:** REMOVE
