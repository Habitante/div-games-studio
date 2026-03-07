# divsound.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 6 |
| Total commented-out lines | ~12 |
| Safe to remove | 6 blocks, ~12 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 62
**Category:** MIKE_ADDED_DEAD
**Summary:** `//exit(1);` in SDL mixer init error path. Mike commented out the exit so the program continues even if audio init fails.
**Recommendation:** REMOVE

---

### Block 2: Lines 94-95, 98
**Category:** ORIGINAL_DEAD
**Summary:** `//int mixer = QUALITYMIXER;` and `//int mixmode = EIGHTBIT | MONO;` in DOS `InitSound()`. Already commented out in original DOS source (lines 31, 34). Alternative config values that Daniel left as reference.
**Recommendation:** REMOVE

---

### Block 3: Lines 241-242, 244-245
**Category:** ORIGINAL_DEAD
**Summary:** Same `QUALITYMIXER`/`EIGHTBIT|MONO` alternatives in `ResetSound()`, also already commented out in original (lines 171, 175).
**Recommendation:** REMOVE

---

### Block 4: Lines 407-409
**Category:** MIKE_ADDED_DEAD
**Summary:** Commented-out `doneEffect()` body: `//if(Mix_Playing(chan)) // StopSound(chan);`. Mike's SDL_mixer effect done callback, written then disabled.
**Recommendation:** REMOVE

---

### Block 5: Lines 487
**Category:** MIKE_ADDED_DEAD
**Summary:** `//Mix_HaltChannel(-1);` in `DivPlaySound()`. A commented-out "halt all channels" that would have been too aggressive.
**Recommendation:** REMOVE

---

### Block 6: Lines 536-538
**Category:** MIKE_ADDED_DEAD
**Summary:** Three commented-out lines in `StopSound()`: `Mix_UnregisterAllEffects()` call with error message. Mike disabled this, possibly because it caused issues.
**Recommendation:** REMOVE
