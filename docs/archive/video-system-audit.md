# Video/Display System Audit Report

**Date:** 2026-03-08

---

## 1. OSDEP_IsFullScreen() -- DESTRUCTIVE, NOT A QUERY

**File:** `C:\Src\div\src\shared\osdep\osd_sdl2.c`, lines 53-79

**Current behavior:** Despite its name suggesting a boolean query ("is the window fullscreen?"), this function:
1. Destroys the SDL window (`SDL_DestroyWindow`)
2. Destroys the SDL renderer (`SDL_DestroyRenderer`)
3. Destroys the SDL texture (`SDL_DestroyTexture`)
4. Frees both surface buffers (`OSDEP_buffer8`, `OSDEP_buffer32`)
5. Calls `SDL_Quit()` -- shuts down the entire SDL subsystem
6. Always returns 0 (i.e., "not fullscreen")

**Callers:**
- `divvideo.c:36` -- IDE's `IsFullScreen()` wrapper, which delegates to `OSDEP_IsFullScreen()`
- `v.c:45` -- Runtime's `IsFullScreen()` wrapper, identical delegation

**Where those wrappers are called:**
- `divvideo.c:41` -- `SDL_ToggleFS()`: calls `IsFullScreen()`, then sets `fsmode` and calls `svmode()`
- `divvideo.c:54` -- `nothing()`: calls `IsFullScreen()` twice (dead code, never called from menus)
- `divvideo.c:193` -- `rvmode()`: calls `IsFullScreen()` and if true, toggles
- `v.c:51` -- Runtime's `SDL_ToggleFS()`: same pattern
- `v.c:384` -- Runtime's `rvmode()`: same pattern
- Runtime `finalizacion()` at `i.c:1521` calls `rvmode()`, which calls `IsFullScreen()`

**Cascade of damage:** When the user presses Alt+Enter (line `divvideo.c:382` / `v.c:462-464`), the chain is:
1. `SDL_ToggleFS()` calls `IsFullScreen()`
2. `IsFullScreen()` calls `OSDEP_IsFullScreen()`
3. Entire window/renderer/texture/buffers destroyed, `SDL_Quit()` called
4. Returns 0 ("not fullscreen")
5. `SDL_ToggleFS()` sets `fsmode=1` and calls `svmode()`
6. `svmode()` calls `OSDEP_SetVideoMode()` which tries to use SDL after `SDL_Quit()` -- crash or undefined behavior

**Same happens at shutdown:** `finalizacion()` calls `rvmode()` -> `IsFullScreen()` -> destroys everything, then `OSDEP_Quit()` via atexit tries `SDL_Quit()` again (double-quit).

**Correct behavior:** Should be a simple query:
```c
int OSDEP_IsFullScreen(void) {
    if (OSDEP_window == NULL) return 0;
    Uint32 flags = SDL_GetWindowFlags(OSDEP_window);
    return (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}
```

---

## 2. OSDEP_ListModes() -- HARDCODED NULL

**File:** `C:\Src\div\src\shared\osdep\osd_sdl2.c`, lines 44-47

**Current behavior:** Unconditionally returns `NULL`. Prints function name to stdout.

**Callers:**
- `src/det_vesa.c:25` -- IDE's `detectar_vesa()`
- The runtime has its own `src/runtime/det_vesa.c` which does NOT call `OSDEP_ListModes()` -- it hardcodes 6 modes.

**Impact on IDE (det_vesa.c, lines 15-49):**
When `modes` is NULL (line 28: `if(modes == 0 || modes == -1)`), the function falls through to the fallback branch that populates `modos[]` with 8 hardcoded resolutions (320x240, 640x480, 800x600, 1024x768, 1280x1024, 1920x1080, 1280x720, 376x282). However, `num_modos` remains 0 (set on line 22, never updated in the fallback branch). This means `qsort` sorts 0 elements.

**Consequence:** `num_modos == 0` means:
- In `divsetup.c:150`, `lvgasizes.maximo=num_modos` = 0 -- the video mode listbox shows ZERO entries
- The modes ARE populated in `modos[]` but `num_modos` says there are none
- The for-loop at `div.c:3350` iterates 0 times, so no validation of the current mode occurs

**Bug in fallback path:** After populating 8 modes, `num_modos` is never set to 8. If the modes list were non-NULL, line 43 sets `num_modos=i-1` (which itself is off-by-one).

**Correct behavior:** Should enumerate display modes using `SDL_GetDisplayMode()`:
```c
OSDEP_VMode ** OSDEP_ListModes(void) {
    int num = SDL_GetNumDisplayModes(0);
    // Allocate array, populate with unique resolutions, NULL-terminate
    // Return the array
}
```

---

## 3. OSDEP_SetVideoMode() -- IGNORES FULLSCREEN PARAMETER

**File:** `C:\Src\div\src\shared\osdep\osd_sdl2.c`, lines 92-144

**Current behavior:**
1. Accepts `fs` parameter but never uses it
2. If window already exists: just calls `SDL_SetWindowSize()` (no renderer/texture recreation if only resizing -- BUT it does destroy and recreate texture/buffers below)
3. If window doesn't exist: calls `SDL_CreateWindowAndRenderer()` with `SDL_WINDOW_RESIZABLE` -- always windowed, never fullscreen
4. Destroys and recreates `OSDEP_texture`, `OSDEP_buffer32`, `OSDEP_buffer8`
5. Creates OSDEP_buffer8 (8-bit paletted surface), OSDEP_buffer32 (32-bit surface), and texture from the 32-bit surface
6. Returns `OSDEP_buffer8`

**Problems:**
- `fs` parameter is completely ignored -- window is always SDL_WINDOW_RESIZABLE
- When window already exists (resize path), it calls `SDL_SetWindowSize()` but does NOT recreate the renderer. The old renderer may have cached state from the old size.
- Texture is created via `SDL_CreateTextureFromSurface()` which copies the (empty) surface pixels and picks format from the surface. This creates a STATIC texture. It should create a STREAMING texture for efficient per-frame updates.
- `OSDEP_SetCaption()` stores the title in `windowtitle[]` but never calls `SDL_SetWindowTitle()` -- the window title is whatever SDL defaults to (or the one set if already existing).

**Callers (IDE):**
- `divvideo.c:174,177` -- `svmode()`: passes `fsmode` as 0 or 1
- `divvideo.c:55,58,62,65` -- `nothing()` and `SDL_ToggleFS()` chain

**Callers (Runtime):**
- `v.c:256,260,263` -- `svmode()`: passes fsmode value

**Correct behavior:** Should honor `fs`:
```c
if (fs) {
    SDL_SetWindowFullscreen(OSDEP_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
} else {
    SDL_SetWindowFullscreen(OSDEP_window, 0);
    SDL_SetWindowSize(OSDEP_window, width, height);
}
```

---

## 4. THE RENDERING PIPELINE

There are **two completely separate rendering pipelines** -- one for the IDE and one for the runtime. They share the same OSDEP functions but use them differently.

### 4a. IDE Rendering Pipeline

**Window creation:** `OSDEP_SetVideoMode()` at `osd_sdl2.c:115` via `SDL_CreateWindowAndRenderer()`

**8-bit buffer:** `OSDEP_buffer8` (returned as `vga` pointer) at `osd_sdl2.c:134`

**32-bit conversion buffer:** `OSDEP_buffer32` at `osd_sdl2.c:137`

**Texture:** `OSDEP_texture` at `osd_sdl2.c:141`

**IDE volcado flow (`divvideo.c:256-352`, `volcadosdl()`):**
1. Lock `vga` (which is `OSDEP_buffer8`)
2. Iterate scanlines, convert 8-bit pixels to 32-bit using `colors[]` lookup -- writes directly to `vga->pixels`
3. BUT `vga` IS the 8-bit surface! The `case 8:` branch at line 336 does `memcpy(q,p,vga_an)` -- copies 8-bit data from `copia` to the 8-bit surface. This is correct for the 8-bit case.
4. Calls `OSDEP_UpdateRect()` which just calls `OSDEP_Flip()`
5. `OSDEP_Flip()` at `osd_sdl2.c:147-165`:
   - `SDL_BlitSurface(OSDEP_buffer8, NULL, OSDEP_buffer32, NULL)` -- palette-to-32bit conversion
   - `SDL_UpdateTexture(OSDEP_texture, ...)` -- upload 32-bit pixels to texture
   - `SDL_RenderCopy()` -- render texture to screen
   - `SDL_RenderPresent()` -- present

**Oddity:** `volcadosdl()` in `divvideo.c` has BOTH a manual pixel conversion loop (lines 308-338 for 32/24/16/8 bit cases) AND then calls `OSDEP_Flip()` which does its own 8->32 conversion via `SDL_BlitSurface`. The manual conversion is redundant/dead for the 8-bit path (the `case 8` branch just memcpy's raw 8-bit data). The manual 32-bit conversion writes to `vga->pixels` which is the 8-bit surface, corrupting it. This code path only works because `vga->format->BitsPerPixel` is 8, so the `case 8` branch executes.

### 4b. Runtime Rendering Pipeline

**Window creation:** Same `OSDEP_SetVideoMode()` -- shares `OSDEP_window`/`OSDEP_renderer`

**Runtime has SEPARATE local globals** at `v.c:23-25`:
```c
SDL_Window *divWindow=NULL;
SDL_Renderer *divRender=NULL;
SDL_Texture *divTexture=NULL;
```
These are declared but **never initialized** -- they remain NULL.

**Runtime volcado flow (`v.c:397-418`, `volcadosdl()`):**
1. `SDL_UpdateTexture(divTexture, NULL, copia, ...)` -- **divTexture is NULL! This is a no-op or crash.**
2. `SDL_RenderClear(divRender)` -- **divRender is NULL! Same problem.**
3. `SDL_RenderCopy(divRender, divTexture, ...)` -- NULL renderer
4. `SDL_RenderPresent(divRender)` -- NULL renderer
5. Then copies 8-bit data from `copia` to `vga->pixels` (OSDEP_buffer8)
6. Calls `OSDEP_Flip(vga)` which does the real work via OSDEP_renderer/OSDEP_texture

**So the first 4 lines of runtime volcadosdl() are dead code** -- they operate on NULL pointers. The actual rendering happens through `OSDEP_Flip()` using the OSDEP globals, which is the same path as the IDE.

**Additional runtime call:** `OSDEP_SetWindowSize(vga_an, vga_al)` at `f.c:2004` -- called when the runtime sets a new video mode (the `set_mode()` DIV language function). This resizes the existing window.

---

## 5. WINDOW CREATION FLOW

### 5a. IDE Startup

1. `main()` at `div.c:389` -- entry point
2. `OSDEP_Init()` at `div.c:395` -- calls `SDL_Init(SDL_INIT_EVERYTHING)`
3. `load_config()` at `div.c:560` -- reads `system/setup.bin`, sets `vga_an`, `vga_al`, `fsmode` from saved settings (lines 4591-4594)
4. `inicializacion()` at `div.c:584`:
   - `detectar_vesa()` at `div.c:3346` -- populates `modos[]` array but `num_modos` stays 0
   - Allocates `copia` buffer (vga_an * vga_al) at line 3387
   - Calls `svmode()` at `div.c:3630` -- the actual window creation:
     - `divvideo.c:163-182`: calls `OSDEP_SetVideoMode(vga_an, vga_al, 8, fsmode)`
     - fsmode is passed but ignored -- always creates windowed
5. `inicializa_entorno()` at `div.c:586` -- sets up desktop, loads session

**Default IDE resolution:** 640x480 or whatever was saved in `setup.bin`. Minimum enforced: 640x480 (see divmouse.c:518-523 resize handler).

### 5b. Runtime Startup

1. `main()` at `i.c` -- calls `OSDEP_Init()` at line 1733
2. Reads compiled program header to get `vga_an`/`vga_al` from the DIV program
3. `inicializacion()` -> `detectar_vesa()` at `i.c:445` (runtime version hardcodes 6 modes)
4. `svmode()` at `i.c:457` -> `OSDEP_SetVideoMode(vga_an, vga_al, 8, fsmode)`
5. `madewith()` splash screen rendered at `v.c:229-241` (if not DEBUG)
6. In `f.c:1999-2004`, when changing modes: `svmode()` then `OSDEP_SetWindowSize()`

**Runtime resolution:** Determined by the DIV program's `set_mode()` call. Common modes: 320x200, 320x240, 640x480, etc.

---

## 6. ALL OSDEP VIDEO FUNCTION CALLERS

| Function | Callers |
|----------|---------|
| **OSDEP_Init()** | `div.c:395` (IDE main), `i.c:1733` (runtime main) |
| **OSDEP_Quit()** | `osd_sdl2.c:76` (inside IsFullScreen!), `i.c:1590` (runtime finalizacion), `i.c:1730` (atexit) |
| **OSDEP_IsFullScreen()** | `divvideo.c:36` (IDE IsFullScreen wrapper), `v.c:45` (runtime IsFullScreen wrapper) |
| **OSDEP_ListModes()** | `det_vesa.c:25` (IDE only) |
| **OSDEP_SetVideoMode()** | `divvideo.c:55,58,62,65,174,177` (IDE), `v.c:256,260,263` (runtime) |
| **OSDEP_SetWindowSize()** | `f.c:2004` (runtime set_mode) |
| **OSDEP_Flip()** | `divvideo.c:350` (IDE volcadosdl), `v.c:211` (runtime fade_wait), `v.c:238` (runtime madewith), `v.c:418` (runtime volcadosdl), `osd_sdl2.c:168` (OSDEP_UpdateRect delegates) |
| **OSDEP_SetPalette()** | `divvideo.c:129` (IDE set_dac), `v.c:161` (runtime set_dac), `divfli.c:170,255` (FLI animation) |
| **OSDEP_UpdateRect()** | `divvideo.c:251,345` (IDE volcadosdl/volcadosdlp), `divfli.c:444` (FLI animation) |
| **OSDEP_SetCaption()** | `div.c:552,555` (IDE), `v.c:272` (runtime) |
| **OSDEP_WarpMouse()** | Never directly called from app code (only through wrappers) |

---

## 7. ADDITIONAL BUGS FOUND

### 7a. OSDEP_SetPalette ignores firstcolor/ncolors parameters (`osd_sdl2.c:172`)
The function signature accepts `firstcolor` and `ncolors` but always passes `0, 256` to `SDL_SetPaletteColors`. The FLI animation code at `divfli.c:170,255` passes partial palette updates (e.g., only N colors starting at an offset). These are silently ignored -- the full 256-color palette is set from whatever data happens to be in the colors array.

### 7b. OSDEP_SetCaption never sets the window title (`osd_sdl2.c:39-42`)
It copies the title into `windowtitle[]` but never calls `SDL_SetWindowTitle(OSDEP_window, title)`. The window title will be whatever SDL defaults to (usually blank or the executable name).

### 7c. Runtime's volcadosdl uses NULL divTexture/divRender (`v.c:399-402`)
Lines 399-402 call `SDL_UpdateTexture`/`SDL_RenderClear`/`SDL_RenderCopy`/`SDL_RenderPresent` on `divTexture` and `divRender` which are always NULL. These calls are dead code at best, segfaults at worst (SDL2 typically returns errors for NULL pointers rather than crashing, but it's undefined).

### 7d. Texture created as STATIC, updated every frame (`osd_sdl2.c:141`)
`SDL_CreateTextureFromSurface()` creates a `SDL_TEXTUREACCESS_STATIC` texture. Then `OSDEP_Flip()` calls `SDL_UpdateTexture()` on it every frame. This works but is suboptimal -- `SDL_TEXTUREACCESS_STREAMING` is designed for this use case and is faster.

### 7e. IDE resize handler is destructive (`divmouse.c:318-327`)
On `SDL_WINDOWEVENT_RESIZED`, the handler sets `vga_an`/`vga_al` to the new window size, then calls `EndSound()` + sets `soundstopped=1`. When `soundstopped` is processed (lines 513-585), it reallocates `copia`, calls `svmode()` (which calls `OSDEP_SetVideoMode` which calls `SDL_SetWindowSize` -- potentially triggering ANOTHER resize event), then `preparar_tapiz()`, etc. This is fragile and could loop.

### 7f. Double rendering in IDE volcadosdl (`divvideo.c:256-351`)
The function has a manual pixel-format conversion loop (lines 308-338) that handles 32/24/16/8 bit cases. For the 8-bit case, it copies raw bytes. Then it calls `OSDEP_UpdateRect` (which calls `OSDEP_Flip`) which does its OWN 8->32 conversion via `SDL_BlitSurface`. The manual loop is redundant dead code for all cases except 8-bit, and even for 8-bit it duplicates work that `SDL_BlitSurface` already does.

---

## 8. DEPENDENCIES BETWEEN FUNCTIONS

```
OSDEP_Init()
  +-- Must be called first (initializes SDL)

OSDEP_SetVideoMode()
  |-- Creates OSDEP_window (if NULL)
  |-- Creates OSDEP_renderer (if NULL)
  |-- Creates OSDEP_buffer8 (8-bit surface, returned as "vga")
  |-- Creates OSDEP_buffer32 (32-bit conversion surface)
  +-- Creates OSDEP_texture (from buffer32)

OSDEP_SetPalette()
  +-- Requires OSDEP_buffer8 to exist (sets palette on it)

OSDEP_Flip()
  |-- Requires OSDEP_buffer8 (source for blit)
  |-- Requires OSDEP_buffer32 (destination for blit)
  |-- Requires OSDEP_texture (upload target)
  +-- Requires OSDEP_renderer (render target)

OSDEP_IsFullScreen()
  +-- DESTROYS all of the above + calls SDL_Quit()

OSDEP_SetWindowSize()
  |-- Requires OSDEP_window
  +-- Requires OSDEP_renderer
```

---

## 9. SUGGESTED FIX ORDER

### Phase 1A: Non-destructive fixes (safe, no behavior change for user)

1. **Fix OSDEP_IsFullScreen()** -- Replace with a proper query using `SDL_GetWindowFlags()`. This is the most critical fix as it blocks all other fullscreen work and causes crashes on Alt+Enter and shutdown.

2. **Fix OSDEP_SetCaption()** -- Add `SDL_SetWindowTitle()` call.

3. **Fix OSDEP_SetPalette()** -- Use the actual `firstcolor`/`ncolors` parameters instead of hardcoded 0/256.

4. **Remove dead code in runtime v.c:399-402** -- The NULL `divTexture`/`divRender` calls.

5. **Remove redundant manual conversion in IDE divvideo.c volcadosdl()** -- The format-switching loop is dead code; let `OSDEP_Flip()` handle conversion.

### Phase 1B: Fullscreen support

6. **Fix OSDEP_SetVideoMode()** -- Honor the `fs` parameter. Use `SDL_WINDOW_FULLSCREEN_DESKTOP` for fullscreen (scales to desktop resolution while keeping the logical resolution), or `SDL_SetWindowFullscreen()` when toggling on an existing window.

7. **Fix OSDEP_ListModes()** -- Enumerate actual display modes via `SDL_GetNumDisplayModes()` / `SDL_GetDisplayMode()`. Or, since `SDL_WINDOW_FULLSCREEN_DESKTOP` makes mode enumeration less important, return a curated list of common retro resolutions.

8. **Fix detectar_vesa() fallback** -- Set `num_modos` when populating the fallback mode list (currently stays 0 even though 8 modes are written).

### Phase 1C: Quality improvements

9. **Use SDL_TEXTUREACCESS_STREAMING** -- Create the texture with `SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h)` instead of `SDL_CreateTextureFromSurface()`.

10. **Add integer scaling** -- Use `SDL_RenderSetLogicalSize(renderer, width, height)` to get proper scaling with letterboxing. This single call handles: aspect ratio preservation, integer-multiple scaling (with `SDL_RenderSetIntegerScale(renderer, SDL_TRUE)`), and mouse coordinate mapping.

11. **Add re-entrancy guard for resize** -- The IDE resize handler -> `svmode()` -> `SDL_SetWindowSize()` -> another resize event chain needs a guard flag.

12. **Unify divWindow/divRender/divTexture with OSDEP globals** -- The runtime declares its own window/renderer/texture globals that are never used. Remove them to avoid confusion.

---

## 10. SDL2 BEST PRACTICES TO FOLLOW

**Integer scaling:** `SDL_RenderSetLogicalSize(renderer, game_width, game_height)` automatically handles scaling the logical resolution to the window/fullscreen size. Combined with `SDL_RenderSetIntegerScale(renderer, SDL_TRUE)`, this provides crisp pixel-art scaling with letterboxing. The `SDL_RenderCopy(renderer, texture, NULL, NULL)` call already in place would then "just work" at any window size.

**Fullscreen:** Use `SDL_WINDOW_FULLSCREEN_DESKTOP` rather than `SDL_WINDOW_FULLSCREEN`. Desktop fullscreen doesn't change the display resolution, is faster to enter/exit, and works better with modern multi-monitor setups. Combined with `SDL_RenderSetLogicalSize`, the game renders at its native resolution and SDL handles the scaling.

**High-DPI:** For future-proofing, create the window with `SDL_WINDOW_ALLOW_HIGHDPI`. Then use `SDL_GetRendererOutputSize()` instead of `SDL_GetWindowSize()` for the actual pixel count. The logical size abstraction via `SDL_RenderSetLogicalSize` handles this transparently.

**Streaming textures:** For content that changes every frame (which is the case here), `SDL_TEXTUREACCESS_STREAMING` with `SDL_LockTexture()`/`SDL_UnlockTexture()` is more efficient than `SDL_UpdateTexture()` on a static texture, as it avoids a memory copy.

**Vsync:** Create the renderer with `SDL_RENDERER_PRESENTVSYNC` to get hardware vsync. The current `retrazo()` function is a no-op on SDL2 (the DOS VGA retrace wait is ifdef'd out). Using renderer vsync replaces this cleanly.

**Mouse coordinate mapping:** When using `SDL_RenderSetLogicalSize`, SDL automatically maps physical mouse coordinates to logical coordinates. The manual mouse coordinate scaling the codebase does would become unnecessary.
