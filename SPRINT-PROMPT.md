# DIV Games Studio — Agent Working Guide

How to work on this codebase safely and effectively. Read this before starting
any sprint or task.

## Context loading

Read these files first (use Agent/Explore for parallel reads):

1. `ROADMAP.md` — project vision, completed work, current priorities
2. `docs/architecture-overview.md` — IDE startup, main loop, OSDEP layer, file formats
3. `docs/compiler-pipeline.md` — DIV compiler: lexer, parser, 127-opcode bytecode
4. `docs/vm-and-runtime.md` — stack VM, process scheduling, rendering pipeline
5. `docs/archive/glossary-spanish-english.md` — Spanish→English identifier translations
6. `src/global.h` — all globals, type aliases, macros
7. `src/div_string.h` — safe string helpers

## Your role

You lead, I provide support when necessary. However, be mindful of preserving your precious context, use agent teams where it could help you. Your responsability is to lead the agents team, and leave the codebase and documentation always in a better state at the end. Any changes made to the repo, must be reflected on the documentation before context ends. And roadmap must be updated.

Feel free to discuss with me high-level strategy, and re-focus next roadmap efforts to any areas you think are most critical / important to tackle first.

## Build & verify

```bash
export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"
cd /c/Src/div/build
cmake -G "MinGW Makefiles" -DTARGETOS=WINDOWS-NATIVE \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev ..
mingw32-make -j$(nproc)
```

All 4 targets must build: `d`, `div`, `divrun`, `divdbg`.
After ANY change, rebuild and confirm **zero warnings**.

## Hard constraints

1. **Must compile.** All 4 targets. Zero warnings.
2. **No behavioral changes** during cleanup/rename sprints. If you find a bug,
   note it separately — don't "fix" it during a rename.
3. **Project-wide consistency.** No half-renamed identifiers. If you rename
   `crear_menu` to `create_menu`, every reference in the entire codebase changes.
4. **32-bit only.** `sizeof(int) == sizeof(void*)`. No 64-bit assumptions.
5. **UTF-8 source files.** Use `\xNN` for high-byte string/char literals.
6. **No unnecessary new files.** Prefer editing existing files unless splitting
   a large file or reorganizing source structure.
7. **Commit after each sprint** with a descriptive message.

## Safe string helpers (from Sprint E)

In `src/div_string.h`:
- `div_strcpy(dst, sizeof(dst), src)` — bounded copy
- `div_strcat(dst, sizeof(dst), src)` — bounded concatenation
- `div_snprintf(dst, sizeof(dst), fmt, ...)` — bounded format
- `DIV_STRCPY`/`DIV_STRCAT`/`DIV_SPRINTF` — macros using sizeof automatically

Rules: use `sizeof(dst)` for stack arrays. Find allocation size for heap buffers.
Use `memmove()` for overlapping buffers.

## Coding standards

- `.clang-format` and `.editorconfig` define the project style
- Apply formatting incrementally via `git clang-format` on changed lines,
  not whole-codebase reformats
- Watch for name collisions when renaming — two different Spanish names might
  map to the same English name
- When in doubt about the intent of code, consult the original MSDOS source
  in `original_msdos_source/` for comparison

## Agent team methodology

For any sprint, use parallel agents where possible:

```
Agent 1: File A (read → modify → build-verify)
Agent 2: File B (read → modify → build-verify)
Agent 3: File C (read → modify → build-verify)
```

**Independence rule:** Two agents can work in parallel ONLY if they modify
different files. Cross-file renames must be done sequentially or by one agent.

**Context loading per agent:** Each agent should read the target file, the
glossary, `global.h`, and any headers the target file includes.

## Sprint cleanup protocol

**Every sprint must update project docs as part of its deliverables:**

1. **MEMORY.md** — Update the "Current state" section with what changed
2. **ROADMAP.md** — Check off completed items, update line counts if significant,
   add any new items discovered during the sprint
3. **README.md** — Update if the sprint changed source layout, file names,
   build process, or anything user-facing
4. **docs/*.md** — Update architecture/compiler/runtime docs if affected
5. **SPRINT-PROMPT.md** — Update if constraints, methodology, or tooling changed

This ensures future sessions start with accurate, up-to-date documentation
instead of having to re-discover the project state.
