# runner/runner.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 4 |
| Total commented-out lines | ~9 |
| Safe to remove | 4 blocks, ~9 lines |
| Needs review | 0 blocks, ~0 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Lines 2, 5, 8
**Category:** DEAD_CODE
**Summary:** Three commented-out `#include` directives: `<dos.h>`, `<process.h>`, and `<graph.h>`. These are DOS/Watcom-specific headers that do not exist on modern platforms. The file is a DOS-era launcher (`runner.c`) that spawns `DOS4GW.EXE` -- none of these headers are available or needed in the SDL2 port.
**Recommendation:** REMOVE

---

### Block 2: Lines 74-75
**Category:** DEAD_CODE
**Summary:** Two commented-out lines: a Spanish comment (`// La siguiente linea daba problemas de arranque en algunos equipos ...` = "The following line caused startup problems on some machines") and a `putenv("DOS4GVM=...")` call that set DOS4GW virtual memory parameters. This was intentionally disabled in the original DOS era due to compatibility issues.
**Recommendation:** REMOVE

---

### Block 3: Lines 22-27
**Category:** DEAD_CODE
**Summary:** A `#pragma aux set_mode` block defining an inline assembly function that calls `INT 10h` (BIOS video interrupt) to set text mode 3. This is Watcom C-specific syntax and 16-bit DOS BIOS code. It is not compiled by GCC/MinGW (the `#pragma aux` directive is silently ignored or errors).
**Recommendation:** REMOVE (note: this is active code, not commented out -- but it is a dead Watcom pragma that modern compilers ignore. Including for completeness since it is functionally dead.)

---

### Block 4: Line 5
**Category:** DEAD_CODE
**Summary:** `///#include <process.h>` -- uses triple slash, likely an accidental extra `/` when commenting out. The `<process.h>` header provided `spawnvp()` on Watcom/MSVC. On MinGW it may or may not be available but is unused in the SDL2 port context.
**Recommendation:** REMOVE

---

## Review Items

None. This entire file (`runner.c`) is a DOS-era launcher that spawns `DOS4GW.EXE` as an extender, checks CPU type via `get_cpuid()`, and uses Watcom-specific APIs (`getcmd`, `_heapmin`, `_heapshrink`, `spawnvp`). It is almost certainly not compiled in the current SDL2 build. Consider whether this file should be removed entirely from the repository.
