# SDL2 → SDL3 Migration Report

**Date:** 2026-03-08
**SDL3 version evaluated:** 3.4.2 (stable, released January 2025)
**Current SDL version:** SDL2 + SDL2_mixer (no SDL2_net usage)
**Recommendation:** Do not migrate yet. See [Assessment](#assessment) at the end.

---

## Table of Contents

1. [Current SDL Usage Inventory](#1-current-sdl-usage-inventory)
2. [SDL_mixer → SDL3_mixer (SIGNIFICANT)](#2-sdl_mixer--sdl3_mixer)
3. [Window Events Flattening (SIGNIFICANT)](#3-window-events-flattening)
4. [Surface Creation & Palette Pipeline (MODERATE)](#4-surface-creation--palette-pipeline)
5. [Other API Changes (TRIVIAL to MODERATE)](#5-other-api-changes)
6. [Build System Changes](#6-build-system-changes)
7. [Migration Order](#7-migration-order)
8. [Assessment](#8-assessment)

---

## 1. Current SDL Usage Inventory

~456 SDL API calls across ~10 key files:

| Subsystem | Unique APIs | Call Sites | Key Files |
|-----------|------------|------------|-----------|
| SDL core (window/render) | ~40 | ~80 | osd_sdl2.c, shared/run/video.c, ide/video.c |
| SDL events/input | ~20 | ~50 | ide/mouse.c, ide/keyboard.c (IDE + runtime) |
| SDL_mixer | ~25 | ~50 | editor/pcm.c, ide/sound.c, ide/browser.c, ide/mixer.c |
| SDL joystick | ~10 | ~20 | osd_sdl2.c |
| SDL_RWops | 3 | ~12 | editor/pcm.c, ide/sound.c, shared/run/video.c |
| SDLK_* key constants | 85+ | ~170 | osd_sdl2.c, osd_sdl12.c |
| SDL_net | 0 | 0 | (removed in Phase 0) |

### Key source files by SDL usage

| File | Role | SDL subsystems |
|------|------|---------------|
| `src/shared/osdep/osd_sdl2.c` | SDL2 OS abstraction | Window, renderer, surface, texture, palette, joystick |
| `src/shared/osdep/osd_sdl12.c` | SDL 1.2 legacy layer | (dead code, would be deleted) |
| `src/shared/run/video.c` | Runtime rendering | Blit, flip, texture update, splash screen |
| `src/ide/video.c` | IDE video setup | Surface creation, palette, fullscreen toggle |
| `src/ide/mouse.c` | IDE event handler | PollEvent, mouse, keyboard, window events |
| `src/ide/keyboard.c` | IDE keyboard (no SDL) | Only SDL_GetTicks |
| `src/shared/run/keyboard.c` | Runtime event handler | PollEvent, mouse, keyboard, window events |
| `src/editor/pcm.c` | IDE PCM sound editor | Mix_LoadWAV, Mix_PlayChannel, Mix_SetPostMix, RWops |
| `src/ide/sound.c` | IDE audio init | Mix_Init, Mix_OpenAudio |
| `src/shared/run/sound.c` | Runtime audio | Full mixer: load, play, volume, panning, effects |
| `src/ide/browser.c` | File browser | Sound preview playback |
| `src/ide/mixer.c` | IDE mixer UI | Mix_Volume, Mix_VolumeMusic (2 lines) |
| `runtime/fli/flxplay.c` | FLI animation player | SDL 1.2 APIs — needs full rewrite |
| `src/shared/lib/sdlgfx/SDL_framerate.c` | FPS limiter | SDL_GetTicks, SDL_Delay |

---

## 2. SDL_mixer → SDL3_mixer

**Overall: SIGNIFICANT — complete API redesign. Largest single migration task.**

### 2.1 Conceptual changes

SDL3_mixer replaces the channel-based model with an explicit track/audio model:

| SDL2_mixer | SDL3_mixer |
|-----------|-----------|
| `Mix_Chunk` (sound effect) | `MIX_Audio` (unified) |
| `Mix_Music` (streaming music) | `MIX_Audio` (unified) |
| Implicit channels (0-63, auto-allocated) | Explicit `MIX_Track` objects (create/destroy) |
| `Mix_PlayChannel(-1, chunk, loops)` | `MIX_SetTrackAudio(track, audio)` + `MIX_PlayTrack(track, loops)` |
| Volume: int 0-128 | Volume: float 0.0-1.0 |
| `Mix_OpenAudio(freq, fmt, ch, buf)` | `MIX_CreateMixerDevice(devid, &spec)` → returns `MIX_Mixer*` |
| `Mix_Init(flags)` | `MIX_Init()` (no flags) |

### 2.2 Data structure changes

**IDE structs (global.h):**
```c
// pcminfo: Mix_Chunk *SI  →  MIX_Audio *SI
// modinfo: Mix_Music *music  →  MIX_Audio *music
```

**Runtime structs (sound.h):**
```c
// sound_t: Mix_Chunk *sound  →  MIX_Audio *sound
// song_t: Mix_Music *music  →  MIX_Audio *music
//           SDL_RWops *rw  →  SDL_IOStream *io (or removed)
```

**New globals needed:**
- IDE: `MIX_Mixer *g_mixer`, `MIX_Track *preview_track`, `MIX_Track *music_track`
- Runtime: `MIX_Mixer *g_mixer`, `MIX_Track *g_tracks[64]`, `MIX_Track *g_music_track`

### 2.3 IDE audio init (ide/sound.c) — TRIVIAL

```c
// SDL2:  Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
// SDL3:  SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 44100 };
//        g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
```

Also `Mix_Init(flags)` in ide/main.c becomes `MIX_Init()` (no flags).

### 2.4 IDE PCM sound editor (editor/pcm.c) — MODERATE, ~40 call sites

This is 2,150 lines and the most audio-intensive file in the IDE.

**Sound loading** (`open_sound`, `open_sound_file`, `open_desktop_sound`):
- Current: `Mix_LoadWAV(path)` or wrap raw PCM in WAV header via `DIVMIX_LoadPCM` + `SDL_RWFromMem` + `Mix_LoadWAV_RW`
- SDL3: `MIX_LoadAudio(mixer, path, true)` or `MIX_LoadRawAudio(mixer, data, len, &spec)` — **the WAV-wrapping hack is eliminated entirely**

**Opaque `MIX_Audio` — no `abuf`/`alen` access:**
The editor currently reads/writes `SI->abuf` directly for waveform display and editing (volume, fade, silence, copy/paste). In SDL3_mixer, `MIX_Audio` is opaque. Solution: the code already maintains a separate `sound_data` buffer — make that the authority, rebuild `MIX_Audio` via `MIX_LoadRawAudio()` after each edit. This is cleaner architecture.

**Sound playback** (`pcm2`, `modify_sound`):
- Current: `Mix_PlayChannel(0, mypcminfo->SI, 0)`
- SDL3: `MIX_SetTrackAudio(preview_track, mypcminfo->SI); MIX_PlayTrack(preview_track, 0);`
- IDE only plays one sound at a time for preview → single pre-allocated track suffices.

**Cut/paste** (`modify_sound` lines 1447, 1526):
- Current: `Mix_QuickLoad_RAW(FileBuffer, FilePos)` — actually incorrect (passes WAV buffer with offset hack)
- SDL3: `MIX_LoadRawAudio(mixer, sound_data, sound_size*2, &spec)` — skip WAV wrapping entirely

**`change_sound_freq()`**: Resamples PCM, wraps in WAV, loads via `Mix_LoadWAV_RW`. Simplifies to `MIX_LoadRawAudio()`.

**`Mix_SetPostMix` callback** (`noEffect` function, line 918): Used as a song-position counter — increments counters, doesn't look at audio data. Becomes `MIX_SetPostMixCallback(mixer, noEffectSDL3, NULL)`. Trivial.

**Music playback** (`sound_play_song`): `Mix_LoadMUS` + `Mix_PlayMusic` → `MIX_LoadAudio` + `MIX_SetTrackAudio` + `MIX_PlayTrack`. Needs dedicated music track.

### 2.5 IDE file browser (ide/browser.c) — TRIVIAL

4 lines: `Mix_HaltChannel(-1)` + `Mix_FreeChunk(smp)` + `Mix_LoadWAV(path)` + `Mix_PlayChannel(0, smp, 0)` → stop/destroy/load/set+play with track.

### 2.6 IDE mixer (ide/mixer.c) — TRIVIAL

2 lines: `Mix_VolumeMusic(cd*8)` and `Mix_Volume(-1, voc*8)` → float-based gain calls.

### 2.7 Runtime audio (shared/run/sound.c) — SIGNIFICANT, ~35 call sites

**Init** (`sound_init`): Same pattern as IDE — `MIX_Init()` + `MIX_CreateMixerDevice()`. Must pre-create track pool (64 tracks for sounds + 1 for music).

**Sound loading** (`sound_load`): Takes raw memory, wraps in WAV if needed, calls `Mix_LoadWAV_RW`. Simplifies with `MIX_LoadRawAudio` for raw PCM, `MIX_LoadAudio_IO` for WAV/OGG.

**Sound playback** (`sound_play`) — **THE HARDEST PART:**
```c
// Current SDL2 code:
con = Mix_PlayChannel(-1, sounds[NumSonido].sound, loop);
Mix_UnregisterAllEffects(con);
Mix_RegisterEffect(con, freqEffect, doneEffect, NULL);
Mix_Volume(con, Volumen/2);
```

The `freqEffect` callback (lines 402-465) is a **60-line hand-written sample-rate converter** registered as a channel effect. It reads from `s->sound->abuf` at a variable rate controlled by `channels[chan].freq` (ratio 0-1024, where 256 = normal speed). This implements variable-speed playback with looping.

**SDL3_mixer replaces this entirely with a single call:**
```c
MIX_SetTrackFrequencyRatio(track, freq / 256.0f);
```

This is the biggest win of the migration — 60+ lines of buggy callback code replaced with one API call. SDL3_mixer handles frequency shifting natively.

Channel allocation changes from auto-allocating (`Mix_PlayChannel(-1, ...)` returns index) to explicit track pool:
```c
int con = find_free_track();  // scan g_tracks[] for idle track
MIX_SetTrackAudio(g_tracks[con], sounds[NumSonido].sound);
MIX_SetTrackFrequencyRatio(g_tracks[con], Frec / 256.0f);
MIX_SetTrackGain(g_tracks[con], Volumen / 256.0f);
MIX_PlayTrack(g_tracks[con], 0);
```

**Volume/panning** (`sound_change`, `sound_change_channel`):
- `Mix_Volume(ch, vol/2)` → `MIX_SetTrackGain(track, vol/256.0f)`
- `Mix_SetPanning(ch, 255-pan, pan)` → float stereo gains
- Frequency: currently sets `channels[ch].freq` for the callback → `MIX_SetTrackFrequencyRatio(track, freq/256.0f)`

**Music** (`sound_load_song`, `sound_play_song`, `sound_stop_song`, `sound_unload_song`): `Mix_LoadMUS_RW` → `MIX_LoadAudio_IO`, `Mix_PlayMusic` → `MIX_SetTrackAudio` + `MIX_PlayTrack` on dedicated music track. Double-load-to-validate pattern can be simplified.

### 2.8 Runtime cleanup (i.c) — TRIVIAL

`Mix_CloseAudio()` + `Mix_Quit()` → destroy all tracks, destroy mixer, `MIX_Quit()`.

### 2.9 SDL_mixer migration summary

| File | Call Sites | Difficulty | Key Change |
|------|-----------|------------|-----------|
| editor/pcm.c | ~40 | Moderate | sound_data as authority, MIX_LoadRawAudio |
| shared/run/sound.c | ~35 | Significant | freqEffect deletion, track pool |
| ide/browser.c | 4 | Trivial | Preview playback |
| ide/mixer.c | 2 | Trivial | Volume int→float |
| ide/sound.c (IDE) | 4 | Trivial | Init/shutdown |
| ide/main.c | 3 | Trivial | Mix_Init |
| runtime/interpreter.c | 2 | Trivial | Shutdown |
| global.h, sound.h | — | Trivial | Struct field types |

**Estimated effort: 3-5 days** (2-3 coding, 1-2 testing both IDE and runtime).

---

## 3. Window Events Flattening

**Overall: SIGNIFICANT structurally, but mostly mechanical. Hidden gotchas.**

### 3.1 Current structure

Two files have `PrintEvent()` functions with identical structure (ide/mouse.c lines 291-358, shared/run/keyboard.c lines 198-261):

```c
if (event->type == SDL_WINDOWEVENT) {
    switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN: ...      // log only
        case SDL_WINDOWEVENT_HIDDEN: ...     // log only
        case SDL_WINDOWEVENT_EXPOSED: ...    // log only
        case SDL_WINDOWEVENT_MOVED: ...      // log only
        case SDL_WINDOWEVENT_RESIZED: ...    // FUNCTIONAL: updates vga_an/vga_al/vwidth/vheight
        case SDL_WINDOWEVENT_SIZE_CHANGED: ...  // log only
        case SDL_WINDOWEVENT_MINIMIZED: ...  // log only
        case SDL_WINDOWEVENT_MAXIMIZED: ...  // log only
        case SDL_WINDOWEVENT_RESTORED: ...   // log only
        case SDL_WINDOWEVENT_ENTER: ...      // log only
        case SDL_WINDOWEVENT_LEAVE: ...      // log only
        case SDL_WINDOWEVENT_FOCUS_GAINED: ... // log only
        case SDL_WINDOWEVENT_FOCUS_LOST: ...   // log only
        case SDL_WINDOWEVENT_CLOSE: ...      // log only
    }
}
```

In SDL3, each becomes a top-level event type. The nested if+switch becomes a flat switch.

Only `SDL_WINDOWEVENT_RESIZED` has functional (non-logging) behavior. The other 13 are pure `SDL_Log()` calls — could be collapsed to a single default case during migration.

### 3.2 Window event name mapping

| SDL2 Sub-Event | SDL3 Top-Level Event | Notes |
|---|---|---|
| `SDL_WINDOWEVENT_SHOWN` | `SDL_EVENT_WINDOW_SHOWN` | Direct |
| `SDL_WINDOWEVENT_HIDDEN` | `SDL_EVENT_WINDOW_HIDDEN` | Direct |
| `SDL_WINDOWEVENT_EXPOSED` | `SDL_EVENT_WINDOW_EXPOSED` | Direct |
| `SDL_WINDOWEVENT_MOVED` | `SDL_EVENT_WINDOW_MOVED` | Direct |
| `SDL_WINDOWEVENT_RESIZED` | `SDL_EVENT_WINDOW_RESIZED` | **Behavioral: now also fires from SDL_SetWindowSize()** |
| `SDL_WINDOWEVENT_SIZE_CHANGED` | `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` | Or just use RESIZED |
| `SDL_WINDOWEVENT_MINIMIZED` | `SDL_EVENT_WINDOW_MINIMIZED` | Direct |
| `SDL_WINDOWEVENT_MAXIMIZED` | `SDL_EVENT_WINDOW_MAXIMIZED` | Direct |
| `SDL_WINDOWEVENT_RESTORED` | `SDL_EVENT_WINDOW_RESTORED` | Direct |
| `SDL_WINDOWEVENT_ENTER` | `SDL_EVENT_WINDOW_MOUSE_ENTER` | **Name differs** |
| `SDL_WINDOWEVENT_LEAVE` | `SDL_EVENT_WINDOW_MOUSE_LEAVE` | **Name differs** |
| `SDL_WINDOWEVENT_FOCUS_GAINED` | `SDL_EVENT_WINDOW_FOCUS_GAINED` | Direct |
| `SDL_WINDOWEVENT_FOCUS_LOST` | `SDL_EVENT_WINDOW_FOCUS_LOST` | Direct |
| `SDL_WINDOWEVENT_CLOSE` | `SDL_EVENT_WINDOW_CLOSE_REQUESTED` | **Name differs** |

### 3.3 Other event renames

| SDL2 | SDL3 |
|------|------|
| `SDL_QUIT` | `SDL_EVENT_QUIT` |
| `SDL_KEYDOWN` | `SDL_EVENT_KEY_DOWN` |
| `SDL_KEYUP` | `SDL_EVENT_KEY_UP` |
| `SDL_TEXTINPUT` | `SDL_EVENT_TEXT_INPUT` |
| `SDL_MOUSEMOTION` | `SDL_EVENT_MOUSE_MOTION` |
| `SDL_MOUSEBUTTONDOWN` | `SDL_EVENT_MOUSE_BUTTON_DOWN` |
| `SDL_MOUSEBUTTONUP` | `SDL_EVENT_MOUSE_BUTTON_UP` |
| `SDL_MOUSEWHEEL` | `SDL_EVENT_MOUSE_WHEEL` |
| `SDL_JOYAXISMOTION` | `SDL_EVENT_JOYSTICK_AXIS_MOTION` |

### 3.4 Keyboard event structure changes

| SDL2 | SDL3 |
|------|------|
| `event.key.keysym.sym` | `event.key.key` |
| `event.key.keysym.mod` | `event.key.mod` |
| `event.key.keysym.scancode` | `event.key.scancode` |
| `event.key.state` (SDL_PRESSED/RELEASED) | `event.key.down` (bool) |

~15 references across ide/mouse.c and shared/run/keyboard.c.

### 3.5 Mouse event type changes

Mouse coordinates change from `int` to `float` in SDL3:
- `event.motion.x/y/xrel/yrel` — Sint32 → float
- `event.wheel.y` — Sint32 → float
- `event.text.text` — char[32] → const char* (pointer indexing still works)

### 3.6 GOTCHA: OSDEP_key[] table must be rebuilt from scratch

The keyboard mapping in osd_sdl2.c uses `SDLK_*` values as array indices with a magic offset `0x3FFFFD1A` (derived from SDL2's `SDLK_SCANCODE_MASK`). SDL3 keycode values are completely different — the entire `OSDEP_keyInit()` function (~115 lines) must be rewritten.

Additionally: `SDLK_a` → `SDLK_A` (lowercase to uppercase naming), `KMOD_*` → `SDL_KMOD_*`.

### 3.7 GOTCHA: Re-entrant resize danger

SDL3's `SDL_SetWindowSize()` now fires `SDL_EVENT_WINDOW_RESIZED`, which didn't happen in SDL2. Our resize handler calls `EndSound()` → sets `soundstopped` → triggers `svmode()` → calls `OSDEP_SetVideoMode()` → calls `SDL_SetWindowSize()` → fires RESIZED → **infinite loop**. Must add a re-entrancy guard.

### 3.8 GOTCHA: Window operations are now asynchronous

`SDL_SetWindowSize()` is a request in SDL3, not immediate. Use `SDL_SyncWindow()` for synchronous behavior.

### 3.9 Window events migration summary

| Category | Sites | Difficulty |
|----------|-------|-----------|
| PrintEvent() restructuring | ~140 lines (2 files) | Mechanical |
| Event type renames | ~20 lines | Mechanical |
| keysym → key/mod/scancode | ~15 lines | Mechanical |
| OSDEP_key[] table rebuild | ~115 lines (osd_sdl2.c) | **Significant** — full rewrite |
| Resize re-entrancy guard | ~5-10 lines | Small but critical |
| flxplay.c (SDL 1.2 code) | ~50+ lines | Needs full rewrite (separate concern) |
| **Total** | **~350 lines** | |

**Estimated effort: 1-2 days** (the OSDEP_key table is the hard part).

---

## 4. Surface Creation & Palette Pipeline

**Overall: MODERATE. The 8-bit paletted pipeline is fully supported in SDL3.**

### 4.1 SDL_CreateRGBSurface → SDL_CreateSurface

**Site A — 8-bit paletted back-buffer** (`OSDEP_SetVideoMode()` in `osd_sdl2.c`, creates `OSDEP_buffer8`):
```c
// SDL2:  SDL_CreateRGBSurface(0, width, height, 8, 0,0,0,0);
// SDL3:  SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
```
**CRITICAL**: Must also call `SDL_CreateSurfacePalette(OSDEP_buffer8)` immediately after — SDL3 does NOT auto-create palettes on indexed surfaces. Missing this = black screen.

**Site B — 32-bit blit target** (`OSDEP_SetVideoMode()` in `osd_sdl2.c`, creates `OSDEP_buffer32`):
```c
// SDL2:  SDL_CreateRGBSurface(0, width, height, 32, 0,0,0,0);
// SDL3:  SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
```
`SDL_PIXELFORMAT_RGBA32` auto-selects correct byte order for the platform.

**Site C — Surface clone** (`copy_surface()` in `ide/video.c`):
```c
// SDL2:  SDL_CreateRGBSurface(0, src->w, src->h, src->format->BitsPerPixel, masks...);
// SDL3:  SDL_CreateSurface(src->w, src->h, src->format);  // format is now an enum directly
```
Simpler in SDL3. If source is paletted, must copy palette to new surface afterward.

### 4.2 `surface->format` type change — THE STRUCTURAL CHANGE

In SDL2: `surface->format` is `SDL_PixelFormat*` (struct pointer with `BitsPerPixel`, `Rmask`, `palette`, etc.)
In SDL3: `surface->format` is `SDL_PixelFormat` (an enum value like `SDL_PIXELFORMAT_INDEX8`)

| SDL2 Expression | SDL3 Replacement | File |
|----------------|-----------------|------|
| `surface->format->palette` | `SDL_GetSurfacePalette(surface)` | `OSDEP_SetPalette()` in `osd_sdl2.c` |
| `source->format->BitsPerPixel` | `SDL_GetPixelFormatDetails(source->format)->bits_per_pixel` | `copy_surface()` in `ide/video.c` |
| `source->format->Rmask` etc. | Not needed — use `source->format` enum directly | `copy_surface()` in `ide/video.c` |
| `vga->format->BitsPerPixel` | `SDL_GetPixelFormatDetails(vga->format)->bits_per_pixel` | (search for `vga->format` usage) |

`surface->pixels`, `->pitch`, `->w`, `->h` — **unchanged**, still directly accessible.

### 4.3 Palette management

Current (`OSDEP_SetPalette()` in `osd_sdl2.c`):
```c
SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
```

SDL3:
```c
SDL_SetPaletteColors(SDL_GetSurfacePalette(surface), colors, 0, 256);
```

`SDL_SetPaletteColors` signature unchanged, just returns `bool` instead of `int`.

### 4.4 Paletted → 32-bit blit (the heart of the pipeline)

`SDL_BlitSurface(OSDEP_buffer8, NULL, OSDEP_buffer32, NULL)` — this is where 8-bit paletted is converted to 32-bit for texture upload. **Still fully supported in SDL3.** Same function name, same automatic format conversion including palette lookup.

### 4.5 Other surface-related renames

| SDL2 | SDL3 | Sites |
|------|------|-------|
| `SDL_FreeSurface()` | `SDL_DestroySurface()` | ~6 |
| `SDL_AllocPalette()` | `SDL_CreatePalette()` | (if used) |
| `SDL_LockSurface()` | `SDL_LockSurface()` (unchanged, returns bool) | 4 pairs |
| `SDL_MUSTLOCK()` | `SDL_MUSTLOCK()` (unchanged) | 4 |
| `SDL_BlitSurface()` | `SDL_BlitSurface()` (unchanged, returns bool) | 3 |
| `SDL_SaveBMP()` | `SDL_SaveBMP()` (unchanged, returns bool) | 1 |

### 4.6 Surface migration summary

| Category | Sites | Difficulty |
|----------|-------|-----------|
| SDL_CreateRGBSurface → SDL_CreateSurface | 3 | Low (but palette gotcha!) |
| surface->format->* field access | 6 | Medium |
| SDL_FreeSurface → SDL_DestroySurface | 6 | Trivial |
| SDL_SetPaletteColors palette access | 1 | Low |
| Return type int → bool | ~15 | Trivial |

**Estimated effort: 0.5-1 day.**

**Risk of visual artifacts:** None, assuming palette is correctly created. The 256-color rendering pipeline maps cleanly to SDL3.

---

## 5. Other API Changes

### 5.1 Rendering

| SDL2 | SDL3 | Sites | Notes |
|------|------|-------|-------|
| `SDL_RenderCopy(r, t, src, dst)` | `SDL_RenderTexture(r, t, src, dst)` | 2 | Uses SDL_FRect* but both pass NULL |
| `SDL_RenderPresent()` | unchanged (returns bool) | 3 | |
| `SDL_RenderClear()` | unchanged (returns bool) | 1 | |
| `SDL_UpdateTexture()` | unchanged (returns bool) | 2 | |
| `SDL_CreateTextureFromSurface()` | unchanged | 1 | |
| `SDL_GetRendererOutputSize()` | `SDL_GetCurrentRenderOutputSize()` | 1 | |
| `SDL_CreateTexture()` | unchanged | — | Note: defaults to LINEAR scaling, may need NEAREST for pixel art |

### 5.2 Window/Init

| SDL2 | SDL3 | Sites | Notes |
|------|------|-------|-------|
| `SDL_CreateWindowAndRenderer(w, h, flags, &win, &rend)` | `SDL_CreateWindowAndRenderer(title, w, h, flags, &win, &rend)` | 1 | Added title param |
| `SDL_Init(SDL_INIT_EVERYTHING)` | `SDL_Init(SDL_INIT_VIDEO \| SDL_INIT_AUDIO \| ...)` | 2 | EVERYTHING removed |
| `SDL_ShowCursor(SDL_DISABLE)` | `SDL_HideCursor()` | 3 | Split into separate functions |
| `SDL_WarpMouseInWindow(win, x, y)` | same, but float coords | 1 | |
| `SDL_SetWindowFullscreen(win, flags)` | `SDL_SetWindowFullscreen(win, bool)` | — | Simpler |
| `SDL_bool` / `SDL_TRUE` / `SDL_FALSE` | `bool` / `true` / `false` | few | |

### 5.3 SDL_RWops → SDL_IOStream

| SDL2 | SDL3 | Sites |
|------|------|-------|
| `SDL_RWops *` | `SDL_IOStream *` | ~9 declarations |
| `SDL_RWFromMem(mem, size)` | `SDL_IOFromMem(mem, size)` | 9 calls |
| `SDL_LoadBMP_RW(rw, free)` | `SDL_LoadBMP_IO(io, close)` | 1 |
| `SDL_FreeRW(rw)` | eliminated (closeio parameter handles it) | 2 |

### 5.4 Joystick

| SDL2 | SDL3 | Sites | Notes |
|------|------|-------|-------|
| `SDL_NumJoysticks()` | `SDL_GetJoysticks(&count)` | 3 | Returns ID array |
| `SDL_JoystickOpen(index)` | `SDL_OpenJoystick(id)` | 6 | Takes ID not index |
| `SDL_JoystickClose()` | `SDL_CloseJoystick()` | 5 | |
| `SDL_JoystickNumButtons()` | `SDL_GetNumJoystickButtons()` | 2 | |
| `SDL_JoystickGetButton()` | `SDL_GetJoystickButton()` | 2 | |
| `SDL_JoystickGetAxis()` | `SDL_GetJoystickAxis()` | 2 | |
| `SDL_JoystickNameForIndex()` | `SDL_GetJoystickNameForID()` | 1 | |

Joystick overhaul is moderate — index-based → ID-based, but all in osd_sdl2.c.

### 5.5 Video modes (already broken, Phase 1 rewrite)

| SDL2 | SDL3 |
|------|------|
| `SDL_GetDesktopDisplayMode(index, &mode)` | `SDL_GetDesktopDisplayMode(displayID)` → returns pointer |
| `SDL_GetNumDisplayModes()` + `SDL_GetDisplayMode()` | `SDL_GetFullscreenDisplayModes(displayID, &count)` → returns array |

`OSDEP_ListModes()` already returns NULL and needs a full rewrite in Phase 1 regardless.

---

## 6. Build System Changes

- Library names: `SDL2` → `SDL3`, `SDL2_mixer` → `SDL3_mixer`
- CMake: `find_package(SDL3 REQUIRED)`, target `SDL3::SDL3`
- Include paths: `SDL2/` → `SDL3/`, `SDL2_mixer/` → `SDL3_mixer/`
- `SDLmain` library removed — `SDL_main.h` handles it via header include
- Coccinelle semantic patches available for automated renames: https://github.com/libsdl-org/SDL/tree/main/build-scripts/cocci

**Pre-requisite check**: Verify availability of 32-bit MSYS2 packages:
- `mingw-w64-i686-SDL3` — status unknown
- `mingw-w64-i686-SDL3_mixer` — status unknown (may not exist yet)
- Also check Linux and macOS package availability if cross-platform is a goal

---

## 7. Migration Order

If/when we decide to proceed:

1. **Verify package availability** — 32-bit MSYS2 packages for SDL3 + SDL3_mixer
2. **Delete osd_sdl12.c/osd_sdl12.h** — remove dead SDL 1.2 layer first
3. **Remove SDL1 preprocessor branches** — all `#ifdef SDL2` / `#ifndef SDL2` (Phase 1 task)
4. **Headers + build system** — SDL3 packages, include paths, cmake targets
5. **Trivial renames** — use Coccinelle patches where possible (~30 min)
6. **Surface creation + palette** — SDL_CreateSurface + SDL_CreateSurfacePalette (~0.5 day)
7. **Event system** — types, window events, keysym structure, OSDEP_key[] rebuild (~1-2 days)
8. **SDL_RWops → SDL_IOStream** — mechanical rename (~30 min)
9. **Joystick** — index→ID overhaul (~0.5 day)
10. **SDL_mixer → SDL3_mixer** — do last, it's isolated and biggest (~3-5 days)

**Total estimated effort: 5-8 days of focused work.**

---

## 8. Assessment

### Recommendation: Do not migrate now.

**Reasons to wait:**
- SDL2 is actively maintained and will receive security/bug fixes for years
- `mingw-w64-i686-SDL3_mixer` may not be packaged yet for 32-bit MSYS2
- SDL3 ecosystem is still maturing; SDL2 is proven on Steam
- Phase 1 (video system fix, warnings, landmines) and Phase 2 (cleanup) should come first
- The audio system works — `freqEffect` is buggy but stable
- Our audio needs are simple (8-bit WAV at 11-44kHz + MOD music)

**Reasons to migrate (eventually):**
- `MIX_SetTrackFrequencyRatio()` deletes the buggiest audio code we have (60-line freqEffect callback)
- `MIX_LoadRawAudio()` eliminates WAV-header-wrapping hacks (DIVMIX_LoadPCM, SaveSoundMem)
- Unified `MIX_Audio` type eliminates awkward Chunk/Music distinction
- SDL3_mixer is the future — eventually SDL2_mixer stops getting updates
- Several broken systems (video modes, fullscreen) need rewriting anyway — doing it once for SDL3 is better than doing it twice

**Recommended timing:** Phase 2.5, after Phase 2 (cleanup) is done. By then:
- SDL3 packages will be mature across platforms
- SDL1 preprocessor branches will be gone (Phase 1)
- OSDEP abstraction will be documented (Phase 2)
- Video mode system will already be rewritten (Phase 1)

### Key risks

1. **32-bit SDL3 packages** — may not exist yet. Must verify before starting.
2. **SDL3_mixer `MIX_Audio` opacity** — IDE sound editor accesses decoded PCM buffers directly. Requires data flow change (sound_data as authority).
3. **OSDEP_key[] table** — must be rebuilt from scratch for new SDL3 keycode values.
4. **Resize re-entrancy** — SDL_SetWindowSize now fires RESIZED event, can cause infinite loops.
5. **SDL_CreateSurfacePalette** — forgetting this = black screen. Easy to miss, hard to debug.

---

## Sources

- [SDL3 Migration Guide (Wiki)](https://wiki.libsdl.org/SDL3/README-migration)
- [SDL3 Migration Guide (GitHub)](https://github.com/libsdl-org/SDL/blob/main/docs/README-migration.md)
- [SDL3_mixer Migration Guide](https://wiki.libsdl.org/SDL3_mixer/README-migration)
- [SDL3 Coccinelle Patches](https://github.com/libsdl-org/SDL/tree/main/build-scripts/cocci)
- [SDL3 API Reference](https://wiki.libsdl.org/SDL3/)
- [SDL3_mixer API Reference](https://wiki.libsdl.org/SDL3_mixer/)
