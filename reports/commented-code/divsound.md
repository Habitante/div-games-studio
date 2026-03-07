# divsound.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 5 |
| Total commented-out lines | ~10 |
| Safe to remove | 4 blocks, ~9 lines |
| Needs review | 0 blocks |
| Keep | 1 block |

## Blocks

### Block 1: Line 11
**Category:** ORIGINAL_DEAD
**Summary:** `//int SoundActive=1;` -- alternate initialization value for `SoundActive`. The active line above it sets `SoundActive=0`. Already commented out in the original DOS source (line 11 in original). This was Daniel's toggle to disable sound during development.
**Recommendation:** REMOVE

---

### Block 2: Lines 15-16
**Category:** MIKE_ADDED_DEAD
**Summary:** Two commented-out lines in `InitSound()`: `//SDL_Init(SDL_INIT_AUDIO);` and `//printf("Sound system initialised\n");`. Mike wrote these during initial SDL2 port, then replaced with `SDL_InitSubSystem(SDL_INIT_AUDIO)` on line 17.
**Recommendation:** REMOVE

---

### Block 3: Line 29
**Category:** MIKE_ADDED_DEAD
**Summary:** `//exit(1);` inside the `Mix_OpenAudio` error handler. Mike commented it out so the program continues even if audio init fails (graceful degradation).
**Recommendation:** REMOVE -- the decision to not exit on audio failure is correct and intentional.

---

### Block 4: Lines 44, 47-48
**Category:** ORIGINAL_DEAD
**Summary:** Inside `#ifdef NOTYET` block in `InitSound()`: `//int mixer = FASTMIXER;`, `//int mixmode = SIXTEENBIT | STEREO;`, and `//int mixmode = EIGHTBIT | MONO;` -- alternate mixer configuration values. Already commented out in the original DOS source (lines 24, 27-28). Daniel's configuration alternatives.
**Recommendation:** REMOVE

---

### Block 5: Line 100
**Category:** ORIGINAL_DEAD
**Summary:** `//  timer_init(1193180 / 100, judas_update);` inside `#ifdef NOTYET` block. Timer initialization for Judas sound updates. Already commented out in the original DOS source (line 80). The 8253 PIT timer constant (1193180 Hz) was the standard DOS timer divider.
**Recommendation:** KEEP -- this is inside a `#ifdef NOTYET` block that is already entirely dead code. It will be removed when the NOTYET block is cleaned up. No urgency.

---

## Review Items

None. All commented-out code in this file is either already-dead original code or Mike's discarded SDL2 port experiments. No functional code was disabled.

**Architecture note:** The bulk of `InitSound()` is wrapped in `#ifdef NOTYET` (lines 34-118), containing the entire original Judas sound library initialization (sound card detection from `sound.cfg`, port/IRQ/DMA configuration, `judas_init`). This is replaced by the active `#ifdef MIXER` block (lines 18-32) using SDL2_mixer's `Mix_OpenAudio`. Similarly, `EndSound()` has both active SDL2 cleanup and a `#ifdef NOTYET` Judas cleanup block. The NOTYET blocks themselves are not "commented-out code" per the audit criteria (they are preprocessor-disabled), but they represent ~80 lines of dead Judas-specific code that should eventually be removed.
