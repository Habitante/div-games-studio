# divfli.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 8 |
| Total commented-out lines | ~35 |
| Safe to remove | 8 blocks, ~35 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 3-5
**Category:** MIKE_ADDED_DEAD
**Summary:** `#include <conio.h>` and `#include <mem.h>` commented out. DOS-only headers removed during SDL port.
**Recommendation:** REMOVE

---

### Block 2: Lines 106-114
**Category:** MIKE_ADDED_DEAD
**Summary:** Block of debug printf statements inside `#ifdef DEBUG` / `/* */` for FLC header fields. Mike wrote these then commented them out. ~8 lines.
**Recommendation:** REMOVE

---

### Block 3: Lines 155, 234, 320
**Category:** MIKE_ADDED_DEAD
**Summary:** Three individual commented-out `Palette_Update()` and `SDLInit()` calls scattered through the FLC decoder. These are remnants of Mike's initial SDL port approach before settling on the current design.
**Recommendation:** REMOVE

---

### Block 4: Lines 162-187
**Category:** MIKE_ADDED_DEAD
**Summary:** `#ifdef NOTYET` block in `StartFLI()` containing the original TopFLC-based implementation. Mike rewrote FLI loading to use raw FLC parsing instead. This is dead code behind a never-defined macro. ~25 lines.
**Recommendation:** REMOVE

---

### Block 5: Lines 191-210
**Category:** MIKE_ADDED_DEAD
**Summary:** `#ifdef NOTYET` block containing old `Palette_Update()` implementation using the TopFLC approach.
**Recommendation:** REMOVE

---

### Block 6: Lines 430-434, 460-461, 467-469
**Category:** MIKE_ADDED_DEAD
**Summary:** Debug printf blocks for frame/chunk info inside `#ifdef DEBUG` / `/* */`, plus commented-out `SDL_LockSurface`/`SDL_UnlockSurface` calls and `SDLWaitFrame()`.
**Recommendation:** REMOVE

---

### Block 7: Lines 532-539
**Category:** MIKE_ADDED_DEAD
**Summary:** `#ifdef NOTYET` block in `Nextframe()` with old TopFLC-based frame decode path.
**Recommendation:** REMOVE

---

### Block 8: Lines 555-561, 566-573, 580-583, 586-603
**Category:** MIKE_ADDED_DEAD
**Summary:** Multiple `#ifdef NOTYET` blocks in `EndFli()`, `ResetFli()`, `Error_Reporter()`, and `Palette_Update()` containing old TopFLC-based implementations. All dead code behind never-defined macro.
**Recommendation:** REMOVE
