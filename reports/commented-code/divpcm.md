# divpcm.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 28 |
| Total commented-out lines | ~145 |
| Safe to remove | 19 blocks, ~97 lines |
| Needs review | 5 blocks, ~37 lines |
| Keep | 4 blocks |

## Blocks

### Block 1: Line 7
**Category:** MIKE_REMOVED
**Summary:** `#include <io.h>` was active in original DOS source. Mike commented it out (not available on Linux/SDL2).
**Recommendation:** REVIEW -- `<io.h>` provides `read()` used by `JudasProgressRead()`. On POSIX systems this is `<unistd.h>`. Consider adding a proper cross-platform include.

---

### Block 2: Lines 183
**Category:** MIKE_REMOVED
**Summary:** `judas_freesample(mypcminfo->sample);` in PCM3() was active original Judas sound library call. Mike replaced it with `Mix_FreeChunk`.
**Recommendation:** REMOVE -- the SDL2_mixer replacement is in place above it.

---

### Block 3: Lines 307-313
**Category:** MIKE_REMOVED
**Summary:** `FreeMOD()` body: original used a switch on SongType calling `judas_freexm/frees3m/freemod`. Mike replaced with `Mix_HaltMusic()` + `Mix_SetPostMix(NULL,NULL)`.
**Recommendation:** REMOVE -- SDL2_mixer replacement is active.

---

### Block 4: Lines 322-331
**Category:** MIKE_REMOVED
**Summary:** `GetSongPos()` original switch calling `judas_getxmpos/gets3mpos/getmodpos`. Mike replaced with a simple counter-based `songpos` variable. Has a `// TODO - JUDAS GET SONG POS` note.
**Recommendation:** REMOVE -- SDL2_mixer has no equivalent per-format position API; the counter replacement is adequate.

---

### Block 5: Lines 339-348
**Category:** MIKE_REMOVED
**Summary:** `GetSongLine()` original switch calling `judas_getxmline/gets3mline/getmodline`. Same pattern as GetSongPos.
**Recommendation:** REMOVE -- replaced by `songline` counter.

---

### Block 6: Lines 368-369
**Category:** ORIGINAL_DEAD
**Summary:** Two `wbox()` calls in `mostrar_mod_meters()` were already commented out in the original DOS source (line 334-335 in original).
**Recommendation:** REMOVE

---

### Block 7: Lines 406
**Category:** ORIGINAL_DEAD
**Summary:** Single `wbox()` call in `mostrar_mod_meters()` was already commented out in original DOS source (line 372 in original).
**Recommendation:** REMOVE

---

### Block 8: Line 518
**Category:** MIKE_ADDED_DEAD
**Summary:** `//  SoundInfo *SI;` -- Mike commented out the old Judas type declaration after adding SDL2_mixer `Mix_Chunk *SI` above.
**Recommendation:** REMOVE

---

### Block 9: Lines 579-580
**Category:** MIKE_ADDED_DEAD
**Summary:** `free(pcminfo_aux)` and `if(SI) free(SI)` in `OpenSound()` error path -- Mike commented these out in his SDL2_mixer rewrite. The free of pcminfo_aux is actually a memory leak.
**Recommendation:** REVIEW -- the `free(pcminfo_aux)` should probably be restored to avoid leaking on error.

---

### Block 10: Lines 588-613
**Category:** MIKE_REMOVED
**Summary:** `#ifdef NOTYET` block in OpenSound() containing the original Judas `judas_loadwav` / `judas_loadrawsample` loading path and field assignments. Replaced by SDL2_mixer `Mix_LoadWAV` path above.
**Recommendation:** REMOVE

---

### Block 11: Lines 624-626
**Category:** MIKE_ADDED_DEAD
**Summary:** Three commented-out lines in OpenSound(): alternate `SoundData` assignment and `sample` assignment -- Mike's trial code during SDL2 port.
**Recommendation:** REMOVE

---

### Block 12: Line 634
**Category:** MIKE_ADDED_DEAD
**Summary:** `//#define SoundInfo char` -- Mike's temporary hack definition, then commented out.
**Recommendation:** REMOVE

---

### Block 13: Lines 651, 670-671
**Category:** MIKE_REMOVED / MIKE_ADDED_DEAD
**Summary:** In `OpenSoundFile()`: line 651 `//CloseSound(SoundPathName);` was also commented out in original (ORIGINAL_DEAD). Lines 670-671 are Mike's commented-out `free` calls in error path, same pattern as Block 9.
**Recommendation:** REMOVE (line 651). REVIEW lines 670-671 -- same memory leak concern as Block 9.

---

### Block 14: Lines 679-694
**Category:** MIKE_REMOVED
**Summary:** Original Judas `judas_loadwav` / `judas_loadrawsample` loading and error handling in `OpenSoundFile()`. Replaced by SDL2_mixer `Mix_LoadWAV` above.
**Recommendation:** REMOVE

---

### Block 15: Lines 705, 708
**Category:** MIKE_ADDED_DEAD
**Summary:** Commented-out alternate `SoundData` assignment and `free(SI)` in OpenSoundFile() -- Mike's iterative attempts during port.
**Recommendation:** REMOVE

---

### Block 16: Lines 740-749
**Category:** ORIGINAL_DEAD
**Summary:** `judas_allocsample` block in `OpenDesktopSound()` was already commented out in the original DOS source (lines 552-561 in original). Old approach to allocating sample memory.
**Recommendation:** REMOVE

---

### Block 17: Lines 982-1016
**Category:** MIKE_REMOVED
**Summary:** Entire body of `OpenDesktopSong()` wrapped in `#ifdef NOTYET`. Original loaded songs via `judas_loadxm/loads3m/loadmod`. Mike stubbed the function with a `debugprintf("TODO")`.
**Recommendation:** REVIEW -- this function is called from desktop restore. Song loading via `Mix_LoadMUS` was implemented in `PlaySong()` but never ported here. Desktop song restore is broken.

---

### Block 18: Lines 1037-1039
**Category:** MIKE_ADDED_DEAD
**Summary:** Three commented-out debug lines in `noEffect()` callback: `fprintf` and `memset` stream debug code Mike wrote and abandoned.
**Recommendation:** REMOVE

---

### Block 19: Lines 1079-1114
**Category:** MIKE_REMOVED
**Summary:** `#ifdef NOTYET` in `PlaySong()` containing original Judas `judas_loadxm/loads3m/loadmod` + `judas_playxm/plays3m/playmod` + channel count queries. Replaced by SDL2_mixer `Mix_LoadMUS`/`Mix_PlayMusic` above.
**Recommendation:** REMOVE

---

### Block 20: Lines 1145
**Category:** LEGITIMATE
**Summary:** `//SAMPLE sample;` -- documents that the SAMPLE type declaration was intentionally removed (Judas type no longer exists).
**Recommendation:** KEEP (or REMOVE, harmless either way)

---

### Block 21: Lines 1167
**Category:** ORIGINAL_DEAD
**Summary:** `//sel_2 = ancho_ventana-1;` was already commented out in original (line 902).
**Recommendation:** REMOVE

---

### Block 22: Lines 1240-1244
**Category:** ORIGINAL_DEAD
**Summary:** Three `wbox/wrectangulo/wwrite` calls for conversion options UI in `EditSound1()` -- already commented out in original (lines 975-979).
**Recommendation:** REMOVE

---

### Block 23: Lines 1254-1255 and 1264-1265
**Category:** ORIGINAL_DEAD
**Summary:** "No pone cajas de 1 de ancho" comments with `wbox()` calls -- already commented out in original (lines 989-990, 999-1000).
**Recommendation:** KEEP -- these are explanatory comments ("does not draw 1-pixel-wide boxes") alongside the replaced code, explaining why the manual pixel loop is used instead.

---

### Block 24: Lines 1366, 1441, 1529, 1573-1574, 1605-1607, 1609, 1611-1612, 1615-1616, 1625-1627, 1639-1640, 1644, 1700-1701, 1708-1709, 1720-1721, 1724
**Category:** MIKE_ADDED_DEAD
**Summary:** Scattered commented-out lines throughout `ModifySound()` case 6 (Cut) and case 7 (Paste): `Mix_FreeChunk`, `judas_freesample`, `SDL_RWFromMem` attempts, `free(SI)`, alternate `SoundData`/`sample` assignments. Mike's iterative debugging during the SDL2 port of the sound editor.
**Recommendation:** REMOVE -- approximately 25 lines of trial-and-error commented-out code.

---

### Block 25: Lines 1728-1753
**Category:** MIKE_REMOVED
**Summary:** `#ifdef NOTYET` blocks in ModifySound() case 8 (Play): original Judas `judas_playsample` with SAMPLE setup for selection playback, and DEV_NOSOUND error check. Mike replaced with `Mix_PlayChannel`.
**Recommendation:** REMOVE

---

### Block 26: Lines 1829-1895
**Category:** MIKE_REMOVED
**Summary:** Entire `ChangeSoundFreq()` body wrapped in `#ifdef NOTYET`. Original used Judas `judas_loadwav_mem` to reload the resampled sound. Mike stubbed with `printf("TODO")`.
**Recommendation:** REVIEW -- frequency conversion in the sound editor is completely broken. The function does nothing. Would need SDL2_mixer port.

---

### Block 27: Lines 2003, 2008-2011, 2080
**Category:** ORIGINAL_DEAD
**Summary:** `//MIX_SetVolume(MIX_INPUT, 15, 15);` and `//dspwrite` lines in `RecordSound()` and `PollRecord()` were already commented out in the original DOS source (lines 1684, 1687-1690 in original). Line 2080 `printf` is Mike debug.
**Recommendation:** REMOVE

---

### Block 28: Lines 2166-2222
**Category:** MIKE_REMOVED
**Summary:** Entire `PasteNewSounds()` body wrapped in `#ifdef NOTYET`. Original used `judas_loadwav_mem` to create sound windows from desktop clipboard. Mike stubbed with `printf("TODO")`.
**Recommendation:** REVIEW -- paste-from-editor-to-desktop is broken. Would need SDL2_mixer port.

---

## Review Items

### Block 1 (Line 7): `#include <io.h>`
The `read()` function used in `JudasProgressRead()` (line 2106) needs a header. On POSIX/MinGW, `<unistd.h>` provides it. Consider a conditional include.

### Block 9 (Lines 579-580) and Block 13 (Lines 670-671): Missing `free(pcminfo_aux)` in error paths
When `Mix_LoadWAV` and `DIVMIX_LoadPCM` both fail, `pcminfo_aux` is leaked. The `free(pcminfo_aux)` was commented out but should be restored.

### Block 17 (Lines 982-1016): `OpenDesktopSong()` stubbed
Desktop song restoration is non-functional. Low priority since desktop save/restore is rarely used, but it will silently fail.

### Block 26 (Lines 1829-1895): `ChangeSoundFreq()` stubbed
Sound editor frequency conversion buttons (11025/22050/44100 Hz) do nothing. Medium priority for sound editor functionality.

### Block 28 (Lines 2166-2222): `PasteNewSounds()` stubbed
Paste-from-editor-to-desktop is broken. Low priority.
