# Code Review — 2026-03-12

Five-axis automated audit of the entire `src/` tree (~87K lines of C).
Each axis was reviewed by a dedicated agent scanning all subdirectories.
Findings are prioritized by severity and actionability.

---

## Audit Axes

1. **Memory Safety** — buffer overflows, raw pointer arithmetic, unchecked
   mallocs, use-after-free, integer overflow
2. **Global State & Coupling** — global variable census, cross-module
   dependencies, god structs, shared mutable buffers
3. **Magic Numbers & Limits** — hardcoded array sizes, raw memory offsets,
   file format magic, compiler-runtime coupling
4. **Error Handling** — unchecked file/SDL ops, setjmp leaks, division by
   zero, null derefs, exit paths
5. **Performance & Rendering** — O(N^2) loops, redundant blits, cache
   patterns, compiler scalability

---

## TIER 1: Real Bugs (fix immediately)

### Division by zero in release builds
- **Where:** `runtime/debug/kernel.inc` lines 78, 97, 345, 362
- **Issue:** `ldiv`, `lmod`, `ldia`, `lmoa` opcodes check for zero divisor
  in debug builds but not release. Any DIV program dividing by zero crashes
  `divrun` (the shipped runtime).
- **Fix:** 4 `if` statements

### Double-free in BMP loader
- **Where:** `formats/image.c` ~lines 1362-1380
- **Issue:** `CopiaBuffer` is freed up to 3 times on certain BMP bit depths.
  Heap corruption or crash.
- **Fix:** Restructure cleanup to single free-on-exit

### FPG index overflow
- **Where:** `formats/fpg.c` ~line 138
- **Issue:** `Fpg->nIndex++` never checked against array size (1000). An FPG
  file with >1000 graphics causes heap corruption via `code_desc[1000]`.
- **Fix:** 1 bounds check

### FPG code field unchecked
- **Where:** `formats/fpg.c` ~lines 136, 253, 277
- **Issue:** `kkhead.code` from file used as array index into
  `grf_offsets[1000]` without validation. Crafted FPG causes heap overflow.
- **Fix:** Reject code < 0 or >= 1000

---

## TIER 2: File Format Parsing (fix before Steam)

Every file parser trusts the file completely — no validation of dimensions,
offsets, or array indices read from file headers. This was fine when files
came from your own tools on DOS. On Steam, users load random files.

### Integer overflow in width*height malloc
- **Where:** `formats/image.c`, `formats/fpg.c`, `runtime/functions.c`
- **Pattern:** `malloc(width * height)` where both come from file headers.
  Large values overflow 32-bit int to small allocation, then pixel data
  overflows the buffer.

### Unchecked fread() calls
- **Where:** `formats/image.c` — 12+ sites
- **Pattern:** `fread()` return value never checked. Partial reads from
  truncated files leave buffers with uninitialized data.

### PCX RLE decompression overflow
- **Where:** `formats/image.c` ~lines 291-332
- **Pattern:** Malformed RLE run length can cause output pointer to exceed
  allocated buffer.

### FPG palette read without truncation check
- **Where:** `formats/fpg.c` ~line 357
- **Pattern:** Reads 768-byte palette from file without checking file size.
  Truncated file leaves `tmp[]` partially uninitialized.

### Ghost palette array bounds
- **Where:** `editor/brush.c`, `ide/browser.c`, `editor/charset.c`
- **Pattern:** `ghost[n * 256 + m]` where n and m are derived from pixel
  data without validation. Ghost table is 65536 bytes; out-of-range values
  read/write out of bounds.

---

## TIER 3: Error Handling & Robustness

### SDL initialization not checked
- **Where:** `shared/osdep/osd_sdl2.c` line 89
- **Issue:** `SDL_CreateWindowAndRenderer()` result not checked. Also
  `SDL_CreateTexture()` (lines 118-119) and `SDL_CreateRGBSurface()`
  (lines 114-116). NULL returns cause immediate crash on first render.

### Compiler longjmp leaks
- **Where:** `compiler/compiler.c` ~lines 404-450
- **Issue:** Compiler allocates `vnom`, `mem`, `loc`, `frm`. If parser
  encounters an error and calls `comp_exit()` (longjmp), later allocations
  are leaked. Happens on every compile error.

### Unchecked fopen in Windows fmemopen shim
- **Where:** `win/osdepwin.c` lines 22, 25
- **Issue:** `fopen()` calls not NULL-checked. Crashes if temp directory
  is full or has permission issues.

### Runtime exit() without cleanup
- **Where:** `runtime/functions.c` line 3413
- **Issue:** `exit(stack[sp])` called directly. No SDL cleanup, no temp
  file deletion, no session save.

### JPEG buffer leak on error
- **Where:** `formats/image.c` ~lines 1464-1470
- **Issue:** Buffer allocated before `setjmp`. If JPEG decode fails,
  `longjmp` skips the free.

---

## TIER 4: Architectural Observations (don't fix standalone)

### Global state census
- **154 globals** in `global.h`: ~20 video/display, ~25 paint state, ~35
  UI/window, ~20 colors/theme, ~15 compiler, ~15 text/fonts, ~10 runtime
- **`screen_buffer`** written by IDE, paint editor, compiler (progress),
  and runtime. No ownership model. Works because they never run simultaneously.
- **IDE and runtime allocate the same globals differently** — IDE does
  `malloc(w*h+6)+6`, runtime does `malloc(w*h)`. Same global, different
  sizes. Works because they're separate executables.

### Cross-module coupling (BLOCKING level)
- `screen_buffer` — 4+ writers, 5+ readers, no synchronization
- `window[]` — 96-entry god struct array, 200+ type checks scattered across
  9 IDE files and 4 editor files
- `dac` (palette) — 3 writers, 5+ readers
- `mouse_x`/`mouse_y` — 5+ writers, 6+ readers
- `full_redraw` flag — 8+ writers including runtime triggering IDE refresh

### Compiler-runtime coupling
- `end_struct` in `inter.h` is a hand-calculated sum of struct sizes
- Process field offsets (`_X=26`, `_Y=27`) baked into compiled bytecode
- `ltobj.def` function codes must match `functions.c` switch cases
- No versioning in compiled `.prg` files — any layout change breaks all
  existing programs
- **Stable contract** — hasn't changed in decades, doesn't need to for
  Phase 4 features

### Magic numbers and limits
- 768 (palette size) appears 50+ times with no named constant
- `MAX_FILES=512` redefined in 7+ places
- UI layout uses hardcoded pixel offsets (+7, +23, x9) everywhere
- FPG/MAP/FNT file formats have no version fields

### Hardcoded limits (appropriate for DIV's scope)
- 1000 map codes, 32 fonts, 64 FPGs, 8 runtime windows, 256 texts
- 32 file handles, 8192 compiler symbols, 512 expression elements
- MAX_WINDOWS=96, MAX_ITEMS=24, max_procesos=2048

---

## TIER 5: Performance (not a concern)

### O(N^2) process scheduling
- `exec_process()` does linear search for highest-priority process, called
  once per active process per frame. O(N^2) with process count.
- Fine for typical DIV games (dozens of processes, not thousands).

### O(N^2) Z-order rendering
- `frame_end()` finds highest-Z unrendered sprite by linear scan, renders
  it, repeats. O(N^2) with sprite count.
- Same: fine at DIV scale.

### Dirty rectangle system bypassed
- Full-screen `blit_sdl()` called every frame regardless of dirty state.
- The partial blit infrastructure exists but is inconsistently used.
- Irrelevant with SDL2 on modern hardware.

### 8-bit to 32-bit palette conversion every frame
- `SDL_BlitSurface(buffer8 → buffer32)` unavoidable with current pipeline.
- Would need renderer rewrite to eliminate. Not worth it.

### Busy-wait frame limiter on non-Windows
- `SDL_framerate.c` uses `sched_yield()` spin loop instead of sleep on
  Linux/macOS. Wastes CPU. Windows path correctly uses `SDL_Delay()`.
