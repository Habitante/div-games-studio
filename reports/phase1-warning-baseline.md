# Phase 1 Warning Baseline — DIV Games Studio

Date: 2026-03-06  
Scope: CMake-based Windows-native build in `C:\Src\div` (MSYS2 MinGW32 toolchain)

This report captures the current warning posture, highlights the most important
warning/cleanup targets (with concrete file:line evidence), and recommends a
sequence for the first three small PRs in Phase 1 (Stabilization & Warnings).

---

## 1. Current Warning Posture

### 1.1 Global compiler flags

From `CMakeLists.txt` (lines ~268–284):

```cmake
ADD_DEFINITIONS( ${BUILD_LEVEL}
  -fpermissive
  -fno-omit-frame-pointer
  -funsigned-char
  -w
  -Wno-unused-result
  -Wno-unused
  -D${PLATFORM}=1
  -DTARGET="${TARGET}"
  -DRDEBUG="${DEBUG}"
  -DRUNTIME="${RUNTIME}"
)
...
ELSE()
  ADD_DEFINITIONS( -Wfatal-errors )
ENDIF()
```

And from the generated build files (Windows-native build):

```make
# build/CMakeFiles/div-WINDOWS.dir/flags.make
# compile C with C:/msys64/mingw32/bin/cc.exe
C_DEFINES = -DGIT_SHA1="f80871b" -DMIXER -DRDEBUG="divdbg-WINDOWS" -DRUNTIME="divrun-WINDOWS" -DSDL2=2 -DTARGET="div-WINDOWS" -DUSE_FLI -DWINDOWS=1 -DZLIB -D_GNU_SOURCE
C_INCLUDES = @CMakeFiles/div-WINDOWS.dir/includes_C.rsp
C_FLAGS =   -mwindows   -O1  -fpermissive -fno-omit-frame-pointer -funsigned-char -w -Wno-unused-result -Wno-unused -Wfatal-errors
```

**Implications:**

- `-w` disables **all** diagnostics, including serious ones.
- `-Wno-unused-result` and `-Wno-unused` further suppress useful hygiene.
- `-fpermissive` allows constructs that would otherwise be errors in C++
  code and can hide type mismatches.
- `-funsigned-char` fixes `char` to be unsigned, but the code still mixes
  `char`, `byte`, and `uint8_t` (see below), so signedness issues remain
  non-obvious.
- `-Wfatal-errors` is good (stops on the first error) but currently almost
  nothing surfaces as an error due to `-w`.

There is currently **no way to build with warnings enabled** without editing
`CMakeLists.txt` or the generated flags.

### 1.2 32-bit-only assumptions

The roadmap notes explicitly:

> The codebase MUST be compiled as 32-bit (`sizeof(int) == sizeof(void*)` everywhere)

The code contains many constructs that rely on this, especially in JUDAS and
legacy DOS code, for example:

```c
// src/divtimer.c:82,90 (similar in src/runtime/divtimer.cpp and judas/*.c)
if (!judas_memlock(&timer_code_lock_start,
                   (int)&timer_code_lock_end - (int)&timer_code_lock_start)) return 0;
...
judas_memunlock(&timer_code_lock_start,
                (int)&timer_code_lock_end - (int)&timer_code_lock_start);
```

These casts will warn (`-Wpointer-to-int-cast`) even in 32-bit builds once
warnings are enabled, and are outright UB on 64-bit.

### 1.3 Logging macros and type aliases

From `src/global.h`:

```c
#ifdef DEBUG
#define debugprintf(...) printf ( __VA_ARGS__ )
#else
#define printf debugprintf
#define debugprintf(...)
#endif

#define uchar  uint8_t
#define byte   uint8_t
#define ushort uint16_t
#define word   uint16_t
#ifndef __llvm__
#define ulong  uint32_t
#endif
#define dword  uint32_t

#define swap(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}
```

In non-DEBUG builds, every `printf` in DIV code expands to a no-op macro.
Combined with the type aliases and XOR-based `swap`, this is likely to produce
large amounts of noisy warnings once `-Wall` is enabled:

- `printf` is not seen as a real function (format checking is disabled).
- `swap` can trigger multiple-evaluation and type-conversion warnings.
- Aliases obscure signedness and width.

### 1.4 String handling at scale

`rg` counts across `src/`:

- `strcpy(`: **733** call sites in 33 files
  - e.g., `div.c`, `divhandl.c`, `divinsta.c`, `runtime/f.c`, `osdep.c`, etc.
- `sprintf(`: **241** call sites in 25 files
  - e.g., `runtime/debug/d.c`, `runtime/debug/decompiler.c`, `divmap3d.c`, etc.

These are the main sources of future `-Wstringop-*` and `-Wformat-*` warnings
and overlap with several of the roadmap’s “known landmines”.

---

## 2. Diagnostics and commands run

Commands were run in `C:\Src\div` unless otherwise noted.

**Build / configuration inspection**

- `dir`, `dir src`, `dir build` — basic layout.
- `type build\CMakeFiles\div-WINDOWS.dir\flags.make` — confirmed `C_FLAGS`.
- `powershell -Command "Get-Content CMakeCache.txt -First 40"` — confirmed
  CMake generator, compiler path, and that `TARGETOS=WINDOWS-NATIVE`.

**Warning experiment (single TU)**

Tried to compile `src/div.c` with warnings enabled outside CMake:

```powershell
& 'C:/msys64/mingw32/bin/cc.exe' `
  -std=gnu11 -Wall -Wextra -Wconversion -Wshadow -Wpointer-arith `
  -D_GNU_SOURCE -DWINDOWS=1 -DSDL2=2 -DZLIB -DUSE_FLI `
  -IC:/msys64/mingw32/bin/../include/SDL2 `
  -IC:/msys64/mingw32/bin/../include/opus `
  -Isrc/shared -Isrc `
  -c src/div.c -o build/tmp-div.o
```

- Result: **exit code 1**, but no diagnostics were emitted (empty
  `build\warn-div.log` / `warn-div-all.log` when stderr/stdout were redirected).
- This suggests either a toolchain quirk (e.g. error before message flush) or
  a fatal error that doesn’t print with this invocation. This deserves a
  follow-up interactive run, but for this pass the baseline is built from
  static inspection + roadmap knowledge.

**Code and text searches (read-only analysis)**

- `rg "sprintf\(" src -c` → per-file counts, summed to 241 call sites.
- `rg "strcpy\(" src -c` → per-file counts, summed to 733 call sites.
- `rg "(int)" src` → located many pointer/int casts (esp. JUDAS, VM, graphics).
- `rg "OJO" -n src` → developer-flagged “OJO!” landmines.
- `rg "0x41" -n src\divkeybo.c` → BIOS keyboard buffer addresses.
- `rg "red_panel" -n src\div.c` → verified dev-only red_panel path is now
  `system/red_panel.png` (the roadmap entry is out of date).
- `rg "PrintEvent" -n src\divmouse.c` plus `Get-Content` slices — SDL2 event
  pattern and window close/focus handling.
- `rg "soundstopped" -n src` — checked use of the resize/audio state machine.

All commands were **non-mutating** except for creating `reports/` and attempting
an object build (`build/tmp-div.o`, which failed to be created due to the
compiler error). No source files were modified.

---

## 3. Top 20 Warning/Cleanup Targets

Each target below includes:

- **Evidence**: concrete file:line or pattern
- **Why it matters**: risk/impact
- **Low-risk fix strategy**: behavioural changes avoided where possible

Priorities are ordered roughly by **risk × leverage**, not by ease alone.

### 1) Global warning suppression & permissive flags

- **Evidence**:
  - `CMakeLists.txt:268–284` — `ADD_DEFINITIONS(... -fpermissive ... -w -Wno-unused-result -Wno-unused ...)`
  - `build/CMakeFiles/div-WINDOWS.dir/flags.make` — `C_FLAGS` includes the same.
- **Why it matters (Risk: High, Effort: Medium/High)**:
  - Completely suppresses all warnings, including serious UB hints.
  - `-fpermissive` allows questionable constructs in C++ code.
  - Makes it impossible to get a clean warning picture without editing CMake.
- **Low-risk fix strategy**:
  - Add a CMake option `DIV_ENABLE_WARNINGS` (default OFF).
  - For one target at a time (start with `divrun-WINDOWS`):
    - When enabled, replace `-w` with `-Wall -Wextra` and drop
      `-Wno-unused-result`, `-Wno-unused`.
    - Keep `-Wfatal-errors` and optionally add a temporary
      `-Wno-unused-parameter` to keep noise manageable.
  - Use `target_compile_options` instead of global `ADD_DEFINITIONS` so the
    rollout is incremental.

### 2) 32-bit-only pointer/int casts in timer & JUDAS code

- **Evidence**:
  - `src/divtimer.c:82,90`
  - `src/runtime/divtimer.cpp:70,76`
  - `src/judas/judas.c:1051,1089`
  - `src/judas/kbd.c:167,175`
  - `src/judas/timer.c:73,79`

  Representative pattern:

  ```c
  if (!judas_memlock(&timer_code_lock_start,
                     (int)&timer_code_lock_end - (int)&timer_code_lock_start)) return 0;
  ```

- **Why it matters (Risk: High, Effort: Low/Medium)**:
  - Relies on `(int)` being large enough to hold a pointer.
  - Will emit `-Wpointer-to-int-cast` and `-Wint-to-pointer-cast` with warnings
    enabled.
  - Hard-blocks any future 64-bit port and is brittle even on 32-bit.
- **Low-risk fix strategy**:
  - Introduce a typedef in a shared header, e.g. `typedef intptr_t div_ptrdiff;`.
  - Replace `(int)` casts with `div_ptrdiff` (or `ptrdiff_t`/`size_t` as
    appropriate) in these locations.
  - No behavioural change on 32-bit; purely type-correctness.

### 3) Debug macros redefining `printf`

- **Evidence**: `src/global.h:47–55`:

  ```c
  #ifdef DEBUG
  #define debugprintf(...) printf ( __VA_ARGS__ )
  #else
  #define printf debugprintf
  #define debugprintf(...)
  #endif
  ```

- **Why it matters (Risk: Medium, Effort: Medium)**:
  - In non-DEBUG builds, all `printf` calls in DIV code vanish at preprocess
    time; the compiler never sees the function prototype.
  - This suppresses format-string checking and can confuse warnings involving
    varargs.
  - It also risks interfering with third-party code or future standard-library
    use if headers see the macro.
- **Low-risk fix strategy**:
  - Introduce a `DIV_LOG(...)` macro in a small logging header and migrate
    internal logging call sites (starting with `div.c`, `runtime/i.c`).
  - Then remove `#define printf debugprintf` and let `printf` be the standard
    function again.
  - Behavioural impact is limited: in release builds you can still turn
    `DIV_LOG` into a no-op if desired.

### 4) Unsized integer type aliases & XOR `swap` macro

- **Evidence**:
  - Type aliases: `src/global.h:68–82`.
  - `swap` macro: `src/global.h:108`, `src/runtime/inter.h:173`,
    `src/div1run/inter.h:138`, used in many files (e.g. `rg "swap\(" src`).
- **Why it matters (Risk: Medium, Effort: Medium)**:
  - Aliases like `byte`, `word`, `dword`, `ulong` make it harder to see
    signedness and width at a glance.
  - XOR `swap` is fragile and undefined if `a` and `b` alias; compilers may
    warn about this once optimisations and warnings are turned up.
- **Low-risk fix strategy**:
  - Replace `swap` with a simple temporary-based macro:

    ```c
    #define SWAP_INT(a,b) do { int tmp = (a); (a) = (b); (b) = tmp; } while (0)
    ```

    after auditing usage to confirm it’s only ever used with integers.
  - Prefer `uint8_t/16_t/32_t` directly in new code and gradually phase out
    the legacy aliases.

### 5) Generic globals `r,g,b,c,d,a` and `f`

- **Evidence**: `src/global.h:501–509`:

  ```c
  GLOBAL_DATA int r,g,b,c,d,a; // Contadores genéricos OJO! Quitarlos
  GLOBAL_DATA FILE * f;        // Handle genérico de fichero
  ```

- **Why it matters (Risk: Medium, Effort: Medium/High)**:
  - These are visible in every translation unit; they hide data flow and make
    it extremely easy to accidentally reuse them in unrelated contexts.
  - The comment explicitly says “OJO! Quitarlos” — even the original code
    considers them a smell.
  - They will generate `-Wunused-variable` warnings in units that don’t touch
    them and make real usages harder to audit.
- **Low-risk fix strategy**:
  - Use a search like `rg "\br\b|\bg\b|\bc\b|\bd\b|\ba\b"` to find
    non-local uses and convert them to local variables with descriptive names.
  - Do the same for `f`, introducing local `FILE *` handles instead of reusing
    the global.
  - Once all references are removed, delete the globals.

### 6) BIOS keyboard buffer pointers and low-memory assumptions

- **Evidence**: `src/divkeybo.c:19–33`:

  ```c
  byte * shift = (byte *) 0x417; // Shift status

  #ifdef NOTYET
  word * kb_start = (void*) 0x41a;
  word * kb_end   = (void*) 0x41c;
  #endif
  ```

- **Why it matters (Risk: Low/Medium, Effort: Low)**:
  - On modern OSes, dereferencing these addresses would be UB; fortunately
    `shift` is currently **unused**, and `kb_start/kb_end` are under `#ifdef NOTYET`.
  - Nonetheless, these will show up under `-Wint-to-pointer-cast` and are dead
    baggage from the DOS era.
- **Low-risk fix strategy**:
  - Either wrap all of this in `#if defined(DOS)` (if you intend to keep DOS
    builds) or remove it completely from SDL builds.
  - This is pure cleanup with no behavioural impact on the current targets.

### 7) Missing error handling for `lenguaje.div`

- **Evidence**: `src/div.c:497–507`:

  ```c
  inicializa_textos((uint8_t *)"system/lenguaje.div"); // OJO emitir un error si lenguaje.div no existe
  ```

- **Why it matters (Risk: Medium, Effort: Low)**:
  - If `system/lenguaje.div` is missing or corrupted, the IDE’s text system
    will misbehave or crash, but there is no explicit error path.
  - This is labelled as an “OJO” in the source.
- **Low-risk fix strategy**:
  - Make `inicializa_textos` return a `bool` or error code.
  - Check the result here and call `error(<new code>)` or display a clear
    dialog that the language file is missing, then exit cleanly.

### 8) Hard-coded TTF font path to a dev machine

- **Evidence**: `src/div.c:3645–3646` (inside `#ifdef TTF`):

  ```c
  sysfont    = loadfont("/home/mike/cool-retro-term/app/qml/fonts/modern-fixedsys-excelsior/FSEX301-L2.ttf", (big==1)?18:9);
  editorfont = loadfont("/home/mike/cool-retro-term/app/qml/fonts/modern-fixedsys-excelsior/FSEX301-L2.ttf", (big==1)?18:9);
  ```

- **Why it matters (Risk: Medium, Effort: Low)**:
  - This path only exists on Mike’s machine. Any build with `TTF` enabled on a
    different system will silently fail to load fonts.
  - There are already commented alternatives pointing at `system/` paths.
- **Low-risk fix strategy**:
  - Replace these with a `system/`-relative font (`system/KenPixel.ttf` or
    similar) and/or make the font path configurable via `Setupfile`.
  - Keep the dev-specific path under `#ifdef DIV_INTERNAL` or remove it.

### 9) Hard-coded frame-dump path in the runtime renderer

- **Evidence**: `src/shared/run/v.c:449–457`:

  ```c
  sprintf(filename,"/home/mike/Desktop/out/out%05d.bmp",framecount);
  SDL_SaveBMP(vga, filename);
  ```

- **Why it matters (Risk: Medium, Effort: Low)**:
  - Writes to `/home/mike/Desktop/out` which won’t exist on typical user
    machines.
  - Uses `sprintf` into a fixed 255-byte buffer. It’s unlikely to overflow
    here, but still not ideal.
- **Low-risk fix strategy**:
  - Gate this block behind a debug macro, e.g. `#ifdef DIV_INTERNAL_CAPTURE`.
  - Or redirect the output to a project-relative folder (e.g. `captures/`).
  - Replace `sprintf` with `snprintf` while touching the code.

### 10) Unbounded string ops in IDE path handling

- **Evidence** (representative, all in `src/div.c`):

  - `:4640–4895` — repeated `strcpy` calls into `Setupfile.Dir_*` and
    `tipo[?].path` without length checks.

    ```c
    strcpy(Setupfile.Dir_cwd,tipo[0].path);
    strcpy(Setupfile.Dir_map,tipo[2].path);
    ...
    strcpy(tipo[13].path,tipo[1].path);
    ```

  - `:5043–5053` — `_fullpath` fallback path logic using `strcpy`:

    ```c
    strcpy(cwork2, name);
    ...
    if (_fullpath(cwork1, cwork3, _MAX_PATH)==NULL) strcpy(cwork1,ft.name);
    ```

- **Why it matters (Risk: Medium, Effort: Medium)**:
  - The buffers involved are fixed-size arrays; in practice they’re probably
    large enough, but the compiler can’t reason about that and will warn.
  - These are central to the IDE’s file browser and setup dialogs.
- **Low-risk fix strategy**:
  - Introduce helpers like `div_strcpy(char *dst, size_t dst_size, const char *src)`
    and `div_snprintf` in a small header.
  - Convert the most central path-handling sites (above) to use these helpers.
  - Leave behaviour unchanged (truncate or error in the helper only if length
    truly doesn’t fit, and surface an error rather than truncating silently).

### 11) Unbounded string ops in runtime file APIs

- **Evidence**: `src/runtime/f.c:3520–3533` and neighbouring code:

  ```c
  strcpy(fileinfo->fullpath,full);
  strcpy(fileinfo->dir,dir);
  strcpy(fileinfo->name,fname);
  strcpy(fileinfo->ext,ext);
  ```

- plus many other `strcpy`/`sprintf` calls in `runtime/f.c` used to assemble
  paths and manipulate filenames.

- **Why it matters (Risk: High, Effort: Medium)**:
  - These functions underpin the DIV runtime’s file API. Malicious or malformed
    filenames could overflow these buffers.
  - They are a high-value target for future warnings and potential security
    issues.
- **Low-risk fix strategy**:
  - Reuse the same helpers from target 10.
  - Start with `fileinfo` population and any code that constructs full paths
    from user-controlled strings.
  - Behaviour is preserved, but errors become explicit instead of silent
    memory corruption.

### 12) Volume of `strcpy`/`sprintf` usage overall

- **Evidence**:
  - `rg "strcpy\(" src -c` → 733 call sites.
  - `rg "sprintf\(" src -c` → 241 call sites.

- **Why it matters (Risk: Medium, Effort: High)**:
  - Even if each individual site is “probably safe”, this volume guarantees a
    lot of warning noise.
  - It also makes it harder to see which calls are genuinely dangerous.
- **Low-risk fix strategy**:
  - Treat this as a **long-term cleanup track**, not a single PR.
  - Use the per-file counts from `rg -c` to choose one or two high-density
    files per PR (e.g., `runtime/debug/d.c`, `divhandl.c`).
  - Convert a handful of calls per PR to the safe helpers; keep behaviour stable.

### 13) SDL window close event is ignored (no `salir_del_entorno`)

- **Evidence**: `src/divmouse.c:346–371` (`PrintEvent`):

  ```c
  case SDL_WINDOWEVENT_CLOSE:
      SDL_Log("Window %d closed", event->window.windowID);
      break;
  ```

- **Why it matters (Risk: Medium, Effort: Low)**:
  - Clicking the window’s close button merely logs and does nothing.
  - The roadmap explicitly calls out “Fix window close button (currently logged
    but ignored — no `salir_del_entorno`).”
- **Low-risk fix strategy**:
  - In this case, set `salir_del_entorno = 1;` and confirm this follows the same
    shutdown path as the menu-driven exit.
  - Optionally integrate with any “Are you sure?” confirmation later.

### 14) SDL focus loss handling is logging-only

- **Evidence**: `src/divmouse.c:360–368`:

  ```c
  case SDL_WINDOWEVENT_FOCUS_GAINED:
      SDL_Log("Window %d gained keyboard focus", ...);
      break;
  case SDL_WINDOWEVENT_FOCUS_LOST:
      SDL_Log("Window %d lost keyboard focus", ...);
      break;
  ```

- **Why it matters (Risk: Medium, Effort: Low/Medium)**:
  - Focus loss doesn’t pause or otherwise control the IDE/runtime.
  - Combined with active audio and VM processes, this can be surprising for users.
- **Low-risk fix strategy**:
  - Introduce a simple global “paused” flag.
  - On `FOCUS_LOST`, set `paused=1` and pause audio; on `FOCUS_GAINED`, resume.
  - Initially, only pause the runtime (game preview), not the entire IDE, to
    keep behaviour predictable.

### 15) Resolution change and `soundstopped` state machine

- **Evidence**:
  - `src/divmouse.c:294` — `int soundstopped=0;`
  - `src/divmouse.c:323–332` — set `soundstopped=1;` after resize + `EndSound()`.
  - `src/divmouse.c:584–620` — if `soundstopped==1`, adjust `vga_an/vga_al`,
    reallocate `barra` & `copia`, etc.
  - `src/divsetup.c:106` — `extern int soundstopped;` (historic linkage).

- **Why it matters (Risk: Medium, Effort: Medium)**:
  - This is an ad-hoc state machine scattered between event and input code.
  - It’s easy to break or mis-synchronise, and warnings will appear as soon as
    unused or mismatched state is detected.
- **Low-risk fix strategy**:
  - Extract a dedicated “handle window resize” helper called only from
    `PrintEvent`.
  - Have that helper manage `vga_an/vga_al`, `VS_ANCHO/VS_ALTO`, `barra`/
    `copia` reallocation, and audio stop/start in one place.
  - Reduce or remove the need for `soundstopped` as a global flag.

### 16) Commented-out `free()` in runtime stack management

- **Evidence**: `src/runtime/i.c:689–703` (`carga_pila`):

  ```c
  if (mem[id+_SP]) {
    p=stack[mem[id+_SP]];
    ...
//    free(stack[mem[id+_SP]]);
    stack[mem[id+_SP]]=0;
    mem[id+_SP]=0;
    sp=p[1];
    free(p);
  } else sp=0;
  ```

- **Why it matters (Risk: Medium/High, Effort: Medium)**:
  - The roadmap explicitly calls out “Fix or remove the commented-out `free()`
    in runtime stack management (`i.c:778`).”
  - As written, `p` is freed, but `stack[mem[id+_SP]]` is only zeroed, which
    suggests a partially refactored ownership model.
  - This can lead to leaks or, if uncommented improperly, double frees.
- **Low-risk fix strategy**:
  - Document intended ownership: is `stack[]` supposed to own the allocation or
    just store a pointer to `p`?
  - Adjust the code so *exactly one* free happens per allocation and `stack[]`
    is treated consistently.

### 17) 32-bit-only DOS memory probing (`GetFree4kBlocks`)

- **Evidence**: `src/runtime/i.c:2315–2343` under `#ifdef DOS`:

  ```c
  remove("C:\\DIV\\FREESRC.TXT");
  if ((f=fopen("C:\\DIV\\FREESRC.TXT","a")) != NULL) { ... }
  _dos_setdrive((int)toupper(*divpath)-'A'+1,&divnum);
  ...
  exit(0);
  ```

- **Why it matters (Risk: Low, Effort: Low)**:
  - Completely tied to legacy DOS memory probing, with hard-coded `C:\DIV`
    paths and `_dos_*` APIs.
  - Currently under `#ifdef DOS`, but keeping it around invites misconfig and
    extra warnings if `DOS` ever leaks into a modern build.
- **Low-risk fix strategy**:
  - If DOS support is not a goal, remove this entirely.
  - If kept for historical reference, move it into a clearly marked
    `#if 0 /* legacy DOS tool */` block.

### 18) Path handling in installer uses POSIX-only separators

- **Evidence**: `src/divinsta.c:760–775`:

  ```c
  if (_drive<=2) { strcpy(dir,"/"); is_disk=_drive; } // En un disquete no crear directorios

  for(x=1;x<strlen(dir);x++) if(dir[x]=='/') {
      strcpy(cWork,full);
#ifdef NOTYET
      mkdir(cWork);
#endif
  }
  ```

- **Why it matters (Risk: Low/Medium, Effort: Low)**:
  - Uses `'/'` as the only path separator; this works on Windows but is
    inconsistent and brittle.
  - Any future path-normalisation will need to treat `\` as well.
- **Low-risk fix strategy**:
  - Introduce a helper `is_path_sep(char c)` and use it here and in similar
    code instead of direct `'/'` checks.
  - This is a small, mechanical change that makes future cleanups easier.

### 19) Mouse scaling and integer rounding in SDL2 paths

- **Evidence**:
  - `src/divmouse.c:89–90`:

    ```c
    mouse_x = (int)(m_x*(float)((float)vga_an / (float)vwidth));
    mouse_y = (int)(m_y*(float)((float)vga_al / (float)vheight));
    ```

  - `src/shared/run/divkeybo.c:398–399` mirrors this for the runtime mouse.

- **Why it matters (Risk: Low/Medium, Effort: Low/Medium)**:
  - These will trigger `-Wconversion`/`-Wdouble-to-int` warnings.
  - The formulations are duplicated between IDE and runtime, risking drift.
- **Low-risk fix strategy**:
  - Introduce a helper like `int div_scale_coord(float pos, int src, int dst)`
    that performs the cast + clamp once.
  - Use it in both IDE and runtime code.

### 20) Large clusters of float→int math in sprite and VM code

- **Evidence** (representative):

  - `src/divsprit.c:196–199, 502–505`
  - `src/shared/run/c.c:367–379`
  - `src/div1run/s.c:1123–1143`
  - `src/runtime/s.c:1204–1224`

  Typical pattern:

  ```c
  p[0]=x+(int)((float)cos(a0)*d0);
  p[1]=y+(int)(-(float)sin(a0)*d0);
  ```

- **Why it matters (Risk: Low/Medium, Effort: Medium)**:
  - Dozens of similar casts will generate a lot of `-Wconversion` noise.
  - A few of the negation patterns are easy to get wrong (`-(int)(-(float)sin...)`).
- **Low-risk fix strategy**:
  - Add a tiny helper, e.g. `static inline int div_round_to_int(double v)`
    that performs consistent rounding.
  - Replace direct casts in these hotspots with calls to it.
  - Behaviour can be kept identical by using the same rounding semantics.

---

## 4. Recommended Execution Order for the Next 3 Small PRs

A pragmatic sequence that de-risks the build while keeping PRs small and
reviewable:

### PR 1 — Build flags and type-safe pointer maths

**Goals:** Start getting real warnings (opt-in) and fix the worst pointer/int
landmines.

**Scope:**

1. Add `DIV_ENABLE_WARNINGS` option to CMake.
2. When enabled and **only for `divrun-WINDOWS` initially**:
   - Replace `-w` with `-Wall -Wextra`.
   - Drop `-Wno-unused-result` and `-Wno-unused`.
3. Fix type warnings in a narrowly-scoped set of files:
   - `src/divtimer.c`, `src/runtime/divtimer.cpp`, `src/judas/judas.c`,
     `src/judas/kbd.c`, `src/judas/timer.c` — replace `(int)` pointer casts
     with an `intptr_t`-based helper. (Target 2)

**Why this first:**

- Unlocks warning signal for at least one target without touching the whole
  project.
- Fixes the most blatant 32-bit-only constructs early, which also clarifies
  future 64-bit discussions.

### PR 2 — String & path safety (runtime-first nucleus)

**Goals:** Reduce the risk and warning noise from string handling in the
runtime’s file APIs.

**Scope:**

1. Introduce safe string helpers, e.g. `div_strcpy`, `div_snprintf`, in a
   shared header.
2. Update the following to use the helpers:
   - `src/runtime/f.c:3520–3533` (`fileinfo` population) and nearby path
     construction code. (Target 11)
3. Clean up the hard-coded frame dump in `src/shared/run/v.c:449–457`:
   - Either gate behind a debug macro or redirect to a project-relative
     folder.
   - Replace `sprintf` with `snprintf`. (Target 9)

**Why this second:**

- Directly improves runtime robustness.
- The helper functions introduced here will be reused in future IDE-focused
  cleanups (Targets 10 & 12).

### PR 3 — SDL window semantics (close/focus/resize)

**Goals:** Fix user-visible window behaviour with minimal code changes and set
up a clean structure for future warnings around event handling.

**Scope:**

1. In `src/divmouse.c::PrintEvent`:
   - On `SDL_WINDOWEVENT_CLOSE`, set `salir_del_entorno = 1;`. (Target 13)
   - On `SDL_WINDOWEVENT_FOCUS_LOST/FOCUS_GAINED`, toggle a simple `paused`
     flag (and optionally pause/resume audio). (Target 14)
2. Refactor the `soundstopped` + resize handling in `read_mouse2`:
   - Move the reallocation and `vga_an/vga_al` normalisation into a dedicated
     function called from the resize event handler. (Target 15)

**Why this third:**

- Delivers clear user-facing improvements with small, testable changes.
- Simplifies the event-handling code, making it easier to reason about and less
  noisy when warnings are eventually turned on for the IDE target.

---

## 5. Summary

- The **current warning posture** is essentially “warnings off”: global `-w`
  and `-fpermissive` hide nearly all diagnostics, and type/signing decisions are
  encoded via macros and aliases.
- Static inspection plus roadmap cross-checks identified **20 concrete
  warning/cleanup targets**, prioritised by a mix of risk and effort:
  - Root cause: global warning suppression and 32-bit-only pointer casts.
  - High-leverage cleanup: global debug macros, string handling, and generic
    globals.
  - Behavioural landmines: SDL window close/focus handling, resize/audio
    state machine, hard-coded dev paths.
- The **first three PRs** recommended are deliberately small and local:
  1. Add an opt-in strict-warning mode and fix pointer-int casts in timer/JUDAS
     code.
  2. Introduce safe string helpers and apply them in the core runtime file API
     and frame-dump path.
  3. Fix SDL window close/focus behaviour and tame the resize/audio state
     machine.

Together, these steps give you a practical on-ramp into Phase 1: you can start
turning warnings back on in a controlled way while knocking out some of the
most obvious landmines without invasive refactors.