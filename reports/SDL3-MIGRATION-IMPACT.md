# SDL2 to SDL3 Migration Impact Report

**Date:** 2026-03-08
**SDL3 version evaluated:** 3.4.2 (stable since January 2025)
**Scope:** All SDL subsystems used by DIV Games Studio

## Rating Key

- **TRIVIAL** -- Simple rename or drop-in replacement, mechanical change
- **MODERATE** -- Signature changed, but same concept; requires updating call sites
- **SIGNIFICANT** -- Conceptual change requiring rework of logic or architecture

---

## 1. Window/Renderer Creation

### SDL_CreateWindowAndRenderer()
**Rating: MODERATE**
- SDL2: `SDL_CreateWindowAndRenderer(width, height, window_flags, &window, &renderer)`
- SDL3: `bool SDL_CreateWindowAndRenderer(const char *title, int width, int height, SDL_WindowFlags window_flags, SDL_Window **window, SDL_Renderer **renderer)`
- Changes: Added `title` parameter as first arg. Returns `bool` instead of `int`.
- **Impact on DIV:** Used in `osd_sdl2.c:115`. Must add title string and update return check.

### SDL_CreateWindow()
**Rating: MODERATE**
- SDL2: `SDL_CreateWindow(title, x, y, w, h, flags)`
- SDL3: `SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags)`
- Changes: `x` and `y` position parameters removed. Use `SDL_SetWindowPosition()` after creation if needed, or `SDL_CreateWindowWithProperties()` for full control.
- **Impact on DIV:** Not called directly (uses CreateWindowAndRenderer), but good to know.

### SDL_CreateRenderer()
**Rating: MODERATE**
- SDL2: `SDL_CreateRenderer(window, index, flags)`
- SDL3: `SDL_CreateRenderer(SDL_Window *window, const char *name)`
- Changes: Integer `index` replaced with `const char *name` (pass `NULL` for auto-select). `flags` parameter removed entirely.
- **Impact on DIV:** Not called directly (uses CreateWindowAndRenderer).

### SDL_CreateTexture()
**Rating: TRIVIAL**
- Signature unchanged. Textures now default to `SDL_SCALEMODE_LINEAR` and `SDL_BLENDMODE_BLEND`.
- **Impact on DIV:** May need to explicitly set `SDL_SCALEMODE_NEAREST` for pixel-art rendering (8-bit paletted games).

### SDL_CreateTextureFromSurface()
**Rating: TRIVIAL**
- Signature unchanged.
- **Impact on DIV:** Used in `osd_sdl2.c:141`. No changes needed.

### SDL_SetWindowSize()
**Rating: TRIVIAL**
- Signature unchanged. Now always sends `SDL_EVENT_WINDOW_RESIZED`.
- **Impact on DIV:** No changes needed.

### SDL_DestroyWindow()
**Rating: TRIVIAL**
- Signature unchanged.

### SDL_SetWindowTitle()
**Rating: TRIVIAL**
- Signature unchanged.

### SDL_ShowCursor()
**Rating: MODERATE**
- SDL2: `SDL_ShowCursor(SDL_DISABLE)` / `SDL_ShowCursor(SDL_ENABLE)` / `SDL_ShowCursor(SDL_QUERY)` -- one function for show/hide/query
- SDL3: Split into three separate functions:
  - `bool SDL_ShowCursor(void)` -- shows cursor
  - `bool SDL_HideCursor(void)` -- hides cursor
  - `bool SDL_CursorVisible(void)` -- queries state
- **Impact on DIV:** 3 call sites (`div.c:555`, `v.c:253`, `osd_sdl2.c:117`) all use `SDL_ShowCursor(SDL_DISABLE)` or `SDL_ShowCursor(SDL_FALSE)` -- replace with `SDL_HideCursor()`.

### SDL_WarpMouseInWindow()
**Rating: TRIVIAL**
- Now uses `float` coordinates instead of `int`.
- **Impact on DIV:** Used in `osd_sdl2.c:50`. Implicit int-to-float conversion will work, but should cast explicitly.

---

## 2. Rendering

### SDL_RenderCopy()
**Rating: MODERATE**
- SDL2: `SDL_RenderCopy(renderer, texture, srcrect, dstrect)`
- SDL3: `bool SDL_RenderTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, const SDL_FRect *dstrect)`
- Changes: Renamed to `SDL_RenderTexture()`. Uses `SDL_FRect` (float) instead of `SDL_Rect`. Returns `bool`.
- **Impact on DIV:** Used in `osd_sdl2.c:162` and `v.c:410`. Both pass NULL for src/dst so the float rect change is moot, but the rename is required.

### SDL_RenderPresent()
**Rating: TRIVIAL**
- Signature unchanged. Returns `bool` now.
- **Impact on DIV:** Used in `osd_sdl2.c:83,164` and `v.c:411`. No changes needed.

### SDL_RenderClear()
**Rating: TRIVIAL**
- Signature unchanged. Returns `bool` now.
- **Impact on DIV:** Used in `v.c:409`. No changes needed.

### SDL_UpdateTexture()
**Rating: TRIVIAL**
- Signature unchanged.
- **Impact on DIV:** Used in `osd_sdl2.c:159` and `v.c:408`. No changes needed.

### SDL_SetRenderDrawColor()
**Rating: TRIVIAL**
- Signature unchanged. Returns `bool` now.

### SDL_CreateTextureFromSurface()
**Rating: TRIVIAL**
- Signature unchanged.

---

## 3. Surface Operations

### SDL_CreateRGBSurface()
**Rating: SIGNIFICANT**
- SDL2: `SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask)`
- SDL3: `SDL_CreateSurface(int width, int height, SDL_PixelFormat format)`
- Changes: Renamed. 8 parameters collapsed to 3. No more manual masks -- use `SDL_PixelFormat` enum values (e.g., `SDL_PIXELFORMAT_INDEX8`, `SDL_PIXELFORMAT_ARGB8888`).
- **Impact on DIV:** Used in `osd_sdl2.c:134,137` (8-bit and 32-bit surfaces). Must determine correct `SDL_PixelFormat` enum values:
  - 8-bit paletted: `SDL_PIXELFORMAT_INDEX8`
  - 32-bit ARGB: `SDL_PIXELFORMAT_ARGB8888` (or whichever mask combination matches current code)

### SDL_CreateRGBSurfaceFrom()
**Rating: SIGNIFICANT**
- SDL2: `SDL_CreateRGBSurfaceFrom(pixels, w, h, depth, pitch, Rmask, Gmask, Bmask, Amask)`
- SDL3: `SDL_CreateSurfaceFrom(int width, int height, SDL_PixelFormat format, void *pixels, int pitch)`
- Changes: Renamed. Parameter order changed (pixels moved to end). Same mask-to-format simplification.
- **Impact on DIV:** Used in `divvideo.c:148`. Must reorder parameters and use format enum.

### SDL_FreeSurface()
**Rating: TRIVIAL**
- SDL2: `SDL_FreeSurface(surface)`
- SDL3: `SDL_DestroySurface(surface)`
- Changes: Simple rename.
- **Impact on DIV:** ~15 call sites across `osd_sdl2.c`, `osd_sdl12.c`, `v.c`. Mechanical rename.

### SDL_LockSurface() / SDL_UnlockSurface()
**Rating: TRIVIAL**
- Signatures unchanged.

### SDL_SetPaletteColors()
**Rating: TRIVIAL**
- Signature unchanged.
- **Impact on DIV:** Used in `osd_sdl2.c:172`. No changes needed.

### SDL_SetSurfacePalette()
**Rating: TRIVIAL**
- SDL3: `bool SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette)`
- Returns `bool` instead of `int`. Otherwise same.

### SDL_AllocPalette()
**Rating: TRIVIAL**
- SDL2: `SDL_AllocPalette(ncolors)`
- SDL3: `SDL_CreatePalette(int ncolors)` -- renamed.

### SDL_MapRGB()
**Rating: MODERATE**
- SDL2: `SDL_MapRGB(format, r, g, b)` where `format` is `SDL_PixelFormat *`
- SDL3: `SDL_MapRGB(const SDL_PixelFormatDetails *format, const SDL_Palette *palette, Uint8 r, Uint8 g, Uint8 b)`
- Changes: First parameter is now `SDL_PixelFormatDetails *` (obtained via `SDL_GetPixelFormatDetails()`). Added explicit `palette` parameter (can be NULL for non-paletted formats).
- **Impact on DIV:** Must replace `surface->format` with `SDL_GetPixelFormatDetails(surface->format->format)` and pass palette separately.

### SDL_PixelFormat struct access
**Rating: SIGNIFICANT**
- SDL2: `surface->format` is an `SDL_PixelFormat *` with embedded palette
- SDL3: `SDL_PixelFormat` is now an enum (like `SDL_PIXELFORMAT_INDEX8`). For details, use `SDL_GetPixelFormatDetails()`. Palette is separate.
- **Impact on DIV:** Code accessing `surface->format->palette` must change to `surface->palette` or use `SDL_GetSurfacePalette()`.

---

## 4. Events

### SDL_PollEvent()
**Rating: TRIVIAL**
- Signature unchanged.

### Event type constants
**Rating: MODERATE** (but mechanical -- can be done with search-and-replace)

| SDL2 | SDL3 |
|------|------|
| `SDL_KEYDOWN` | `SDL_EVENT_KEY_DOWN` |
| `SDL_KEYUP` | `SDL_EVENT_KEY_UP` |
| `SDL_MOUSEMOTION` | `SDL_EVENT_MOUSE_MOTION` |
| `SDL_MOUSEBUTTONDOWN` | `SDL_EVENT_MOUSE_BUTTON_DOWN` |
| `SDL_QUIT` | `SDL_EVENT_QUIT` |

- **Impact on DIV:** ~15 event type checks across `divmouse.c`, `divkeybo.c`, `flxplay.c`. All mechanical renames.

### SDL_WINDOWEVENT (sub-events)
**Rating: SIGNIFICANT**
- SDL2: `SDL_WINDOWEVENT` with `event.window.event == SDL_WINDOWEVENT_RESIZED` etc.
- SDL3: Each window sub-event is now a top-level event type: `SDL_EVENT_WINDOW_RESIZED`, `SDL_EVENT_WINDOW_SHOWN`, `SDL_EVENT_WINDOW_CLOSE_REQUESTED`, etc. No more `event.window.event` sub-dispatch.
- **Impact on DIV:** Large switch statements in `divmouse.c:293-349` and `divkeybo.c:200-252` must be restructured. The outer `if (event->type == SDL_WINDOWEVENT)` + inner `switch (event->window.event)` becomes a flat `switch (event->type)` with `case SDL_EVENT_WINDOW_RESIZED:` etc.
- Renamed sub-events:
  - `SDL_WINDOWEVENT_CLOSE` -> `SDL_EVENT_WINDOW_CLOSE_REQUESTED`
  - `SDL_WINDOWEVENT_SHOWN` -> `SDL_EVENT_WINDOW_SHOWN`
  - `SDL_WINDOWEVENT_HIDDEN` -> `SDL_EVENT_WINDOW_HIDDEN`
  - `SDL_WINDOWEVENT_EXPOSED` -> `SDL_EVENT_WINDOW_EXPOSED`
  - `SDL_WINDOWEVENT_MOVED` -> `SDL_EVENT_WINDOW_MOVED`
  - `SDL_WINDOWEVENT_RESIZED` -> `SDL_EVENT_WINDOW_RESIZED`
  - `SDL_WINDOWEVENT_SIZE_CHANGED` -> removed (use `SDL_EVENT_WINDOW_RESIZED`)
  - `SDL_WINDOWEVENT_MINIMIZED` -> `SDL_EVENT_WINDOW_MINIMIZED`
  - `SDL_WINDOWEVENT_MAXIMIZED` -> `SDL_EVENT_WINDOW_MAXIMIZED`
  - `SDL_WINDOWEVENT_RESTORED` -> `SDL_EVENT_WINDOW_RESTORED`
  - `SDL_WINDOWEVENT_ENTER` -> `SDL_EVENT_WINDOW_MOUSE_ENTER`
  - `SDL_WINDOWEVENT_LEAVE` -> `SDL_EVENT_WINDOW_MOUSE_LEAVE`
  - `SDL_WINDOWEVENT_FOCUS_GAINED` -> `SDL_EVENT_WINDOW_FOCUS_GAINED`
  - `SDL_WINDOWEVENT_FOCUS_LOST` -> `SDL_EVENT_WINDOW_FOCUS_LOST`

### Keyboard event structure
**Rating: SIGNIFICANT**
- SDL2: `event.key.keysym.sym`, `event.key.keysym.mod`, `event.key.keysym.scancode`, `event.key.keysym.unicode`
- SDL3: `event.key.key` (was `keysym.sym`), `event.key.mod` (was `keysym.mod`), `event.key.scancode` (was `keysym.scancode`). The `.unicode` field was already removed in SDL2 (replaced by `SDL_TEXTINPUT` events).
- **Impact on DIV:** ~15 references to `event.key.keysym.*` across `divmouse.c` and `divkeybo.c`. All must drop the `.keysym` level.

### Keycode constants
**Rating: MODERATE**
- `SDLK_*` lowercase letter constants renamed to uppercase: `SDLK_a` -> `SDLK_A`, etc.
- `KMOD_*` renamed to `SDL_KMOD_*`: `KMOD_CTRL` -> `SDL_KMOD_CTRL`, etc.
- **Impact on DIV:** Need to audit all `SDLK_*` and `KMOD_*` references.

### SDL_GetKeyboardState()
**Rating: TRIVIAL**
- Returns `const bool *` instead of `const Uint8 *`. Semantically the same.

### SDL_GetScancodeFromKey()
**Rating: MODERATE**
- SDL2: `SDL_GetScancodeFromKey(key)`
- SDL3: `SDL_GetScancodeFromKey(SDL_Keycode key, SDL_Keymod *modstate)` -- added modstate parameter.

### Mouse coordinates
**Rating: TRIVIAL**
- Mouse event coordinates (`event.motion.x`, `event.button.x` etc.) are now `float` instead of `int`. May need explicit casts in DIV's integer-math world.

### Event state field
**Rating: TRIVIAL**
- `event.key.state` (`SDL_PRESSED`/`SDL_RELEASED`) replaced by `event.key.down` (`bool`).

### Event timestamps
**Rating: TRIVIAL**
- Timestamps are now in nanoseconds (via `SDL_GetTicksNS()`). Unlikely to affect DIV since timestamps aren't used directly.

---

## 5. Init/Quit

### SDL_Init()
**Rating: MODERATE**
- `SDL_INIT_EVERYTHING` removed. Must list specific subsystems.
- `SDL_INIT_TIMER` removed (timers work without explicit init).
- Remaining valid flags: `SDL_INIT_AUDIO`, `SDL_INIT_VIDEO`, `SDL_INIT_JOYSTICK`, `SDL_INIT_HAPTIC`, `SDL_INIT_GAMEPAD`, `SDL_INIT_EVENTS`, `SDL_INIT_SENSOR`, `SDL_INIT_CAMERA`.
- **Impact on DIV:** `osd_sdl2.c:20` uses `SDL_INIT_EVERYTHING`. Must replace with `SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS` (or whatever we actually need).

### SDL_Quit()
**Rating: TRIVIAL**
- Signature unchanged.

### SDL_bool / SDL_TRUE / SDL_FALSE
**Rating: MODERATE**
- `SDL_bool` replaced by C standard `bool`. `SDL_TRUE` -> `true`, `SDL_FALSE` -> `false`.
- `SDL_DISABLE` / `SDL_ENABLE` removed.
- **Impact on DIV:** `SDL_FALSE` used in `SDL_ShowCursor(SDL_FALSE)` at `div.c:555` -- but that whole call changes anyway.

---

## 6. Timer

### SDL_Delay()
**Rating: TRIVIAL**
- Signature unchanged. Parameter is `Uint32` milliseconds.

### SDL_GetTicks()
**Rating: TRIVIAL**
- Returns `Uint64` instead of `Uint32` in SDL3 (but the change happened in SDL 2.0.18 already).
- **Impact on DIV:** Used in `divkeybo.c:124-125`, `osd_sdl2.c:35`, `flxplay.c:418`, `SDL_framerate.c`. Should be fine since values are assigned to Uint32/int anyway.

### SDL_AddTimer() / SDL_RemoveTimer()
**Rating: TRIVIAL**
- Signature unchanged. No longer requires `SDL_INIT_TIMER`.
- **Impact on DIV:** Not currently used in the codebase.

---

## 7. Video Modes

### SDL_GetDesktopDisplayMode()
**Rating: SIGNIFICANT**
- SDL2: `int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode)` -- fills struct via output pointer
- SDL3: `const SDL_DisplayMode * SDL_GetDesktopDisplayMode(SDL_DisplayID displayID)` -- returns pointer directly
- Changes: Uses `SDL_DisplayID` instead of int index. Returns pointer instead of filling output param.
- **Impact on DIV:** Used indirectly through `OSDEP_ListModes()` which is already broken. Must be rewritten in Phase 1 anyway.

### SDL_GetNumDisplayModes() / SDL_GetDisplayMode()
**Rating: SIGNIFICANT**
- SDL2: iterate with `SDL_GetNumDisplayModes(display)` + `SDL_GetDisplayMode(display, modeIndex, &mode)`
- SDL3: `SDL_DisplayMode ** SDL_GetFullscreenDisplayModes(SDL_DisplayID displayID, int *count)` -- returns NULL-terminated array. Must be freed with `SDL_free()`.
- Changes: Completely different enumeration pattern. Display indices replaced with `SDL_DisplayID` (obtained via `SDL_GetDisplays()`).
- **Impact on DIV:** `OSDEP_ListModes()` in `osd_sdl2.c:44` currently returns NULL. Full rewrite needed regardless.

### SDL_SetWindowFullscreen()
**Rating: MODERATE**
- SDL2: `SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)`
- SDL3: `bool SDL_SetWindowFullscreen(SDL_Window *window, bool fullscreen)` -- simple bool toggle.
- For exclusive fullscreen mode, use `SDL_SetWindowFullscreenMode()` first to set the mode, then `SDL_SetWindowFullscreen(window, true)`.
- **Impact on DIV:** Cleaner API. Part of Phase 1 video rework.

---

## 8. SDL_mixer (SDL2_mixer -> SDL3_mixer)

**Overall Rating: SIGNIFICANT -- this is a complete API redesign.**

SDL3_mixer is a fundamentally different library. The chunk/music distinction is gone. Channels are replaced by "tracks". Volume uses floats instead of 0-128 integers.

### Mix_OpenAudio()
**Rating: SIGNIFICANT**
- SDL2: `Mix_OpenAudio(frequency, format, channels, chunksize)`
- SDL3: `MIX_Mixer * MIX_CreateMixerDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec)`
- Changes: Returns a mixer object. Takes device ID and audio spec. Multiple mixers supported.
- **Impact on DIV:** `divsound.c:24` and `shared/run/divsound.c:58`. Must create audio device first, then mixer.

### Mix_LoadWAV() / Mix_LoadWAV_RW()
**Rating: SIGNIFICANT**
- SDL2: `Mix_LoadWAV(file)` returns `Mix_Chunk *`; `Mix_LoadWAV_RW(rwops, freesrc)` returns `Mix_Chunk *`
- SDL3: `MIX_Audio * MIX_LoadAudio(const char *file)`; `MIX_Audio * MIX_LoadAudio_IO(SDL_IOStream *src, bool closeio)`
- Changes: `Mix_Chunk` and `Mix_Music` unified into `MIX_Audio`. RWops becomes IOStream.
- **Impact on DIV:** ~10 call sites. `Mix_LoadWAV_RW(SDL_RWFromMem(...), 1)` becomes `MIX_LoadAudio_IO(SDL_IOFromMem(...), true)`.

### Mix_PlayChannel()
**Rating: SIGNIFICANT**
- SDL2: `Mix_PlayChannel(channel, chunk, loops)` -- one call
- SDL3: Two-step: `MIX_SetTrackAudio(track, audio)` then `MIX_PlayTrack(track)`
- Tracks are created individually with `MIX_CreateTrack(mixer)` instead of bulk-allocated.
- **Impact on DIV:** `divpcm.c:171,1556` and `divsound.c:482`. Must pre-create tracks and manage them.

### Mix_HaltChannel()
**Rating: MODERATE**
- SDL2: `Mix_HaltChannel(channel)` (pass -1 for all)
- SDL3: `MIX_StopTrack(track)` for individual, `MIX_StopTag(mixer, tag)` for groups.
- **Impact on DIV:** `divpcm.c:1214,1373`, `divbrow.c:1714`, `divsound.c:525,528`. Must convert -1 halt-all to stopping all tracks.

### Mix_Volume()
**Rating: MODERATE**
- SDL2: `Mix_Volume(channel, volume)` where volume is 0-128
- SDL3: `MIX_SetTrackGain(track, gain)` where gain is `float` (0.0 = silence, 1.0 = full)
- **Impact on DIV:** `divsound.c:505,541,563`. Must convert `Volumen/2` (0-128 range) to float (divide by 128.0).

### Mix_SetPanning()
**Rating: MODERATE**
- SDL2: `Mix_SetPanning(channel, left, right)` with 0-255 values
- SDL3: `MIX_SetTrackStereo(track, left_gain, right_gain)` or `MIX_SetTrack3DPosition()`
- **Impact on DIV:** `divsound.c:566`. Must convert 0-255 to float gains.

### Mix_FreeChunk()
**Rating: TRIVIAL**
- SDL2: `Mix_FreeChunk(chunk)`
- SDL3: `MIX_DestroyAudio(audio)` -- rename only.
- **Impact on DIV:** `divpcm.c:184,1686`, `divbrow.c:1716`, `divsound.c:380`.

### Mix_CloseAudio()
**Rating: TRIVIAL**
- SDL2: `Mix_CloseAudio()`
- SDL3: `MIX_DestroyMixer(mixer)` -- rename, plus need to track mixer pointer.
- **Impact on DIV:** `divsound.c:119`, `runtime/i.c:1573`.

### Mix_AllocateChannels()
**Rating: SIGNIFICANT**
- SDL2: `Mix_AllocateChannels(n)` -- bulk allocate
- SDL3: Create tracks individually with `MIX_CreateTrack(mixer)`. Store in array.
- **Impact on DIV:** Must create and manage track array manually.

### Mix_Playing()
**Rating: TRIVIAL**
- SDL2: `Mix_Playing(channel)`
- SDL3: `MIX_TrackPlaying(track)` -- renamed, takes track pointer.
- **Impact on DIV:** `divsound.c:408,524,527,585`.

### Mix_PlayingMusic()
**Rating: TRIVIAL**
- SDL2: `Mix_PlayingMusic()`
- SDL3: `MIX_TrackPlaying(music_track)` -- unified with track API.

---

## 9. SDL_net (SDL2_net -> SDL3_net)

**Overall Rating: SIGNIFICANT -- complete API redesign with async/non-blocking architecture.**

### SDLNet_Init()
**Rating: TRIVIAL**
- SDL2: `SDLNet_Init()`
- SDL3: `NET_Init()` -- renamed.

### SDLNet_ResolveHost()
**Rating: SIGNIFICANT**
- SDL2: `SDLNet_ResolveHost(&ip, host, port)` -- synchronous, fills `IPaddress` struct
- SDL3: `NET_Address * NET_ResolveHostname(const char *host)` -- asynchronous, returns address object. Must poll with `NET_WaitUntilResolved()` or `NET_GetAddressStatus()`.
- Port is specified at connection time, not resolution time.

### SDLNet_TCP_Open()
**Rating: SIGNIFICANT**
- SDL2: `SDLNet_TCP_Open(&ip)` -- synchronous connect, returns `TCPsocket`
- SDL3: `NET_StreamSocket * NET_CreateClient(NET_Address *address, Uint16 port)` -- asynchronous, non-blocking. Must use `NET_WaitUntilConnected()` or `NET_GetConnectionStatus()` to check.
- For servers: `NET_Server * NET_CreateServer(NET_Address *addr, Uint16 port)` + `NET_AcceptClient(server)`.

### SDLNet_TCP_Send()
**Rating: MODERATE**
- SDL2: `SDLNet_TCP_Send(socket, data, len)` -- blocking, returns bytes sent
- SDL3: `bool NET_WriteToStreamSocket(NET_StreamSocket *sock, const void *buf, int buflen)` -- non-blocking, queues data. Returns bool success.

### SDLNet_TCP_Recv()
**Rating: MODERATE**
- SDL2: `SDLNet_TCP_Recv(socket, data, maxlen)` -- blocking, returns bytes received
- SDL3: `int NET_ReadFromStreamSocket(NET_StreamSocket *sock, void *buf, int buflen)` -- non-blocking.

### SDLNet_TCP_Close()
**Rating: TRIVIAL**
- SDL2: `SDLNet_TCP_Close(socket)`
- SDL3: `NET_DestroyStreamSocket(socket)` -- renamed.

### Overall SDL_net impact on DIV
- Network code was already removed in Phase 0, so this has **zero immediate impact**.
- If networking is re-added later, the async architecture will require a different design pattern.

---

## 10. SDL_RWops -> SDL_IOStream

### SDL_RWFromMem()
**Rating: TRIVIAL**
- SDL2: `SDL_RWops * SDL_RWFromMem(void *mem, int size)`
- SDL3: `SDL_IOStream * SDL_IOFromMem(void *mem, size_t size)` -- renamed, size is now `size_t`.
- **Impact on DIV:** 9 call sites across `divpcm.c`, `divsound.c`, `v.c`. Mechanical rename.

### SDL_RWFromFile()
**Rating: TRIVIAL**
- SDL2: `SDL_RWops * SDL_RWFromFile(const char *file, const char *mode)`
- SDL3: `SDL_IOStream * SDL_IOFromFile(const char *file, const char *mode)` -- renamed.
- **Impact on DIV:** Not currently used directly.

### SDL_RWops type
**Rating: MODERATE**
- All `SDL_RWops *` declarations must become `SDL_IOStream *`.
- `SDL_RWclose()` -> `SDL_CloseIO()`
- `SDL_RWread()` -> `SDL_ReadIO()` (signature changed to POSIX-style)
- `SDL_RWwrite()` -> `SDL_WriteIO()` (signature changed to POSIX-style)
- **Impact on DIV:** Variable declarations in `divpcm.c` and `divsound.c` (type rename).

---

## 11. Clipboard

### SDL_SetClipboardText() / SDL_GetClipboardText() / SDL_HasClipboardText()
**Rating: TRIVIAL**
- All three unchanged.
- **Impact on DIV:** Not currently used. No action needed.

---

## 12. Miscellaneous

### SDL_GetError()
**Rating: TRIVIAL**
- Signature unchanged.

### SDL_SetHint()
**Rating: TRIVIAL**
- Signature unchanged.
- Some hint string constants may have been renamed.

### SDL_GetVersion()
**Rating: MODERATE**
- SDL2: `void SDL_GetVersion(SDL_version *ver)` -- fills struct with major/minor/patch
- SDL3: `int SDL_GetVersion(void)` -- returns packed integer, decode with `SDL_VERSION_MAJOR()`, `SDL_VERSIONNUM_MINOR()`, `SDL_VERSIONNUM_PATCH()` macros.
- **Impact on DIV:** Not currently used.

### SDL_GetBasePath()
**Rating: TRIVIAL**
- Signature unchanged.
- **Impact on DIV:** Not currently used.

---

## 13. Include Path Changes

- SDL2: `#include <SDL2/SDL.h>`, `#include <SDL2/SDL_mixer.h>`
- SDL3: `#include <SDL3/SDL.h>`, `#include <SDL3_mixer/SDL_mixer.h>`, `#include <SDL3_net/SDL_net.h>`

---

## 14. Build System Changes

- Library names: `SDL2` -> `SDL3`, `SDL2_mixer` -> `SDL3_mixer`, `SDL2_net` -> `SDL3_net`
- `SDLmain` library removed -- `SDL_main.h` handles it via header include
- CMake: `find_package(SDL3 REQUIRED)`, target is `SDL3::SDL3`
- **Impact on DIV:** CMakeLists.txt must be updated for new package names and targets.

---

## Summary: Work Breakdown by Effort

### Trivial (search-and-replace, <1 hour total)
- `SDL_FreeSurface` -> `SDL_DestroySurface` (~15 sites)
- `SDL_RWFromMem` -> `SDL_IOFromMem` (9 sites)
- `SDL_RWops` -> `SDL_IOStream` (variable type declarations)
- `SDL_AllocPalette` -> `SDL_CreatePalette`
- `SDL_RenderPresent`, `SDL_RenderClear`, `SDL_UpdateTexture` -- unchanged
- `SDL_Delay`, `SDL_GetTicks` -- unchanged
- `SDL_SetPaletteColors` -- unchanged
- Include paths `SDL2/` -> `SDL3/`

### Moderate (requires understanding but straightforward, ~1 day)
- Event type renames: `SDL_KEYDOWN` -> `SDL_EVENT_KEY_DOWN` etc. (~15 sites)
- `SDL_RenderCopy` -> `SDL_RenderTexture` (2 sites)
- `SDL_ShowCursor(SDL_DISABLE)` -> `SDL_HideCursor()` (3 sites)
- `SDL_CreateWindowAndRenderer` signature update (1 site)
- `SDL_INIT_EVERYTHING` -> explicit flags (2 sites)
- `SDL_MapRGB` -- add palette parameter
- `KMOD_*` -> `SDL_KMOD_*`
- `SDL_bool`/`SDL_TRUE`/`SDL_FALSE` -> `bool`/`true`/`false`
- `SDL_SetWindowFullscreen` bool parameter

### Significant (requires architectural rework, ~2-4 days each)
- **SDL_CreateRGBSurface -> SDL_CreateSurface**: Must convert all mask-based surface creation to format enums. 3 sites but need to verify exact format mappings.
- **SDL_WINDOWEVENT flattening**: Two large switch blocks (~30 cases each in divmouse.c and divkeybo.c) must be restructured from nested if/switch to flat switch.
- **Keyboard event structure** (`event.key.keysym.sym` -> `event.key.key`): 15 references, plus the `keysym.unicode` field needs `SDL_TEXTINPUT` event handling.
- **SDL_mixer total rewrite**: Channel-based API replaced with track-based API. Volume scaling changes. ~30 call sites across 4 files. This is the largest single migration task.
- **Video mode enumeration**: New API with `SDL_GetFullscreenDisplayModes()`. Already needs full rewrite (Phase 1).

### Not needed (removed or unused in DIV)
- SDL_net: networking was removed in Phase 0
- SDL_GetVersion, SDL_GetBasePath, clipboard: not used
- SDL_AddTimer/SDL_RemoveTimer: not used
- SDL_GetKeyboardState, SDL_GetScancodeFromKey: not used directly

---

## Recommended Migration Order

1. **Headers + build system** -- get it compiling with SDL3 headers
2. **Trivial renames** -- mechanical search-and-replace pass
3. **Event system** -- fix event types, window events, keyboard structure
4. **Surface creation** -- SDL_CreateSurface with format enums
5. **SDL_RWops -> SDL_IOStream** -- rename + type changes
6. **SDL_mixer** -- this is the big one, do it last since sound is isolated

## External Dependencies

- SDL3 3.4.2: available via MSYS2 (`mingw-w64-i686-sdl3`)
- SDL3_mixer: check MSYS2 availability (`mingw-w64-i686-sdl3-mixer`)
- SDL3_net: not needed (networking removed)
- libjpeg-turbo: unchanged, not SDL-dependent

## Coccinelle Migration Tool

SDL provides Coccinelle semantic patches to automate many of the renames:
https://github.com/libsdl-org/SDL/tree/main/build-scripts/cocci

This can handle many of the trivial and moderate renames automatically, but will not help with the significant architectural changes (SDL_mixer, window events, surface creation).
