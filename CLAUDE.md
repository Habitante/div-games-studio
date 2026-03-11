# DIV Games Studio — Agent Instructions

DIV Games Studio: 1990s DOS game-creation IDE ported to SDL2. ~87K lines of C.
Created by Daniel Navarro Medrano. 32-bit only (sizeof(int)==sizeof(void*)).

## Essential reading

Read these before doing any work. Use parallel Agent/Explore reads.

| File | What it tells you |
|------|-------------------|
| `ROADMAP.md` | Project vision, completed work, **current priorities** |
| `SPRINT-PROMPT.md` | How to work safely: build, constraints, methodology, cleanup protocol |
| `docs/architecture-overview.md` | IDE startup, main loop, OSDEP layer, file formats |
| `docs/compiler-pipeline.md` | Compiler: lexer, parser, 127-opcode bytecode |
| `docs/vm-and-runtime.md` | Stack VM, process scheduling, rendering pipeline |
| `docs/archive/glossary-spanish-english.md` | Spanish→English identifier translations |
| `src/global.h` | All globals, type aliases, macros |
| `src/div_string.h` | Safe string helpers (use these, not raw strcpy/strcat/sprintf) |

## Build

```bash
export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"
cd /c/Src/div/build
cmake -G "MinGW Makefiles" -DTARGETOS=WINDOWS-NATIVE \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev ..
mingw32-make -j$(nproc)
```

All 4 targets must build with **zero warnings**: d, div, divrun, divdbg.

## Hard rules

1. Must compile. All 4 targets. Zero warnings.
2. No behavioral changes during cleanup sprints. Note bugs separately.
3. Project-wide consistency. No half-renamed identifiers.
4. 32-bit only. No 64-bit assumptions.
5. UTF-8 source. Use `\xNN` for high-byte literals.
6. After every sprint: update MEMORY.md, ROADMAP.md, README.md, and docs as needed.
