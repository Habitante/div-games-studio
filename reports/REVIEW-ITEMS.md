# Review Items -- Commented Code Audit

9 items resolved on 2026-03-08 (SoundActive shims, stop_sound, false alarm). 19 items remaining.

---

## 1. divdsktp.c:181-202 -- PCM session save disabled
**File:** divdsktp.c | **Category:** MIKE_ADDED_DEAD | **Priority:** LOW
Case 105 in DownLoad_Desktop wrapped in `#ifdef NOTYET`. PCM windows are silently skipped during session save. Needs SDL2_mixer port or removal.

---

## 2. divdsktp.c:488-507 -- PCM session load disabled
**File:** divdsktp.c | **Category:** MIKE_ADDED_DEAD | **Priority:** LOW
Case 105 in UpLoad_Desktop wrapped in `#ifdef NOTYET`. PCM windows silently skipped during session restore. Must match whatever is done for item 1.

---

## 3. divpcm.c:7 -- Missing `#include <io.h>`
**File:** divpcm.c | **Category:** MIKE_REMOVED | **Priority:** LOW
`#include <io.h>` commented out (provides `read()` for `JudasProgressRead()`). Needs conditional include for `<io.h>` (Win32) vs `<unistd.h>` (POSIX), or verify `read()` is already provided by another header.

---

## 4. divpcm.c:539 -- Memory leak in OpenSound() error path
**File:** divpcm.c | **Category:** MIKE_ADDED_DEAD | **Priority:** HIGH
When `Mix_LoadWAV` and `DIVMIX_LoadPCM` both fail in `OpenSound()`, `pcminfo_aux` is allocated but never freed. The `free(pcminfo_aux)` was commented out by Mike. Restore the free. (Note: the parallel path in `OpenSoundFile()` already has `free(mypcminfo)` and is correct.)

---

## 5. divpcm.c:880-919 -- OpenDesktopSong() entirely stubbed
**File:** divpcm.c | **Category:** MIKE_REMOVED | **Priority:** LOW
Function body inside `#ifdef NOTYET`, replaced with debug printf. Desktop song restore is broken. Needs port to `Mix_LoadMUS()`.

---

## 6. divpcm.c:1632-1701 -- ChangeSoundFreq() entirely stubbed
**File:** divpcm.c | **Category:** MIKE_REMOVED | **Priority:** MEDIUM
Sound editor frequency conversion buttons (11025/22050/44100 Hz) do nothing. Resampling math is portable; only `judas_loadwav_mem` needs replacing with `Mix_LoadWAV_RW(SDL_RWFromMem(...))`.

---

## 7. divpcm.c:1960-2022 -- PasteNewSounds() entirely stubbed
**File:** divpcm.c | **Category:** MIKE_REMOVED | **Priority:** LOW
Paste-from-clipboard to desktop PCM windows broken. Same `judas_loadwav_mem` replacement needed as item 6.

---

## 8. runtime/f.c:2508-2516 -- fade_off() changed from blocking to non-blocking
**File:** runtime/f.c | **Category:** MIKE_REMOVED | **Priority:** MEDIUM
Original `fade_off()` blocked in a while loop until fade completed. Mike converted to non-blocking (`fading=1` + immediate return). This changes semantics for DIV programs that expect `fade_off()` to block. May be intentional for SDL2 event-driven model, but needs verification.

---

## 9. div.c:732 -- test_video dialog disabled
**File:** div.c | **Category:** MIKE_REMOVED | **Priority:** LOW
Startup video test dialog `if (test_video) dialogo(test0)` commented out. Either restore for debugging or remove `test_video` and `TEST` argument handling entirely.

---

## 10. div.c:1693 -- check_free() disk space warning disabled
**File:** div.c | **Category:** MIKE_REMOVED | **Priority:** LOW
Startup disk space check commented out. Function body exists and works (Win32 `GetDiskFreeSpaceEx`). Restore or remove both call and function.

---

## 11. osd_sdl2.c:112-117 -- Video mode resize silently ignored
**File:** osd_sdl2.c | **Category:** INCOMPLETE_PORT | **Priority:** MEDIUM
When `OSDEP_SetVideoMode()` is called and a window already exists, nothing happens. Needs `SDL_SetWindowSize` + texture recreation, or document that resolution switching is unsupported.

---

## 12. shared/osdep.c:231 -- _fullpath() uses strcpy instead of realpath
**File:** shared/osdep.c | **Category:** INCOMPLETE_PORT | **Priority:** LOW
`realpath()` commented out, replaced with `strcpy()`. No actual path resolution on Linux/macOS. Restore `realpath()` on POSIX.

---

## 13. win/osdepwin.c:233 -- _fullpath() uses strcpy instead of realpath
**File:** win/osdepwin.c | **Category:** INCOMPLETE_PORT | **Priority:** LOW
Same as item 12 but Windows platform layer. On MinGW32, CRT `_fullpath()` is available -- this wrapper may be unnecessary.

---

## 14. divmouse.c:76-83,137-144 -- Spacebar-as-mouse-button for paint editor
**File:** divmouse.c | **Category:** MIKE_REMOVED | **Priority:** LOW
Spacebar mapped to pseudo mouse button `0xfffd` for keyboard-driven paint editing. Commented out in both physical-mouse and keyboard-cursor paths. Likely interfered with SDL2 text input; needs guard to only activate in paint editor.

---

## 15. divpalet.c:1355 -- JPEG desktop wallpaper disabled
**File:** divpalet.c | **Category:** MIKE_REMOVED | **Priority:** LOW
`case 4: descomprime_JPG()` commented out. `es_JPG()` detection still runs, so selecting a JPG wallpaper silently fails. Either restore JPEG support or remove the detection too.

---

## 16. div.c:1713-1729 -- command_path()/shell() dead DOS code
**File:** div.c | **Category:** MIKE_REMOVED | **Priority:** LOW
`command_path()` body commented out, returns NULL. `shell()` is dead-end DOS code. DOS Shell already hidden from menu. Remove both functions entirely.

---

## 17. divc.c:1872 -- Active debug fprintf left by Mike
**File:** divc.c:1872 | **Category:** DEBUG_LEFT_IN | **Priority:** HIGH
Active `fprintf(stdout,"FOUND FILE: [%s] [%s] [%s]\n",...)` left in production code. Not commented out -- just forgotten. Prints to stdout every time a file is found during compilation. Should be removed.

---

## 18. divwindo.c wvolcado/wvolcado_oscuro -- Dangling-if bug
**File:** divwindo.c | **Category:** BUG | **Priority:** HIGH
Mike commented out a printf that was the body of an `if(ventana[vn].ptr!=NULL)` statement, making the NULL check a no-op. The subsequent `if(ventana[vn].ptr==p)` runs unconditionally, potentially dereferencing NULL. Need to either restore a proper if-body or restructure the loop.

---

## 19. divpaint.c put_barra_inv():~5118 -- Daniel's "OJO esto no vale" annotation
**File:** divpaint.c | **Category:** KNOWN_BAD | **Priority:** MEDIUM
Daniel's own annotation `//*** OJO *** esto no vale` ("this doesn't work") on the `wput_in_box()` replacement call. Suggests the current replacement code for the inverted toolbar rendering may not be fully correct.

---

## Summary by priority

| Priority | Items | Description |
|----------|-------|-------------|
| **HIGH** | 4 | Memory leak in `OpenSound()` error path |
| **HIGH** | 17 | Active debug fprintf left in divc.c |
| **HIGH** | 18 | Dangling-if bug in divwindo.c |
| **MEDIUM** | 6 | Sound editor frequency conversion broken |
| **MEDIUM** | 8 | `fade_off()` blocking semantics change |
| **MEDIUM** | 11 | Video mode resize silently ignored |
| **MEDIUM** | 19 | divpaint.c "OJO esto no vale" known-bad code |
| **LOW** | 1, 2, 5, 7 | Desktop session PCM/song persistence broken |
| **LOW** | 3 | Missing `<io.h>` / `<unistd.h>` include |
| **LOW** | 9, 10 | test_video dialog and check_free() |
| **LOW** | 12, 13 | `_fullpath` path resolution stubs |
| **LOW** | 14 | Spacebar-as-mouse-button for paint editor |
| **LOW** | 15 | JPEG wallpaper support |
| **LOW** | 16 | Dead `command_path()`/`shell()` functions |
