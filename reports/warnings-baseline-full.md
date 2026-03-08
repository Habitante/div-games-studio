# Compiler Warnings Baseline Report

**Date**: 2026-03-08

**Build flags**: `-Wall -Wextra -Wformat=2 -Wshadow -Wpointer-arith -Wcast-align` (via `DIV_WARNING_BASELINE=ON`)
**Result**: All 4 targets (div, divrun, divdbg, d) **build successfully** with 1,822 warning instances (1,745 unique after deduplication across targets).
**Third-party noise**: 38 warnings from miniz.h/zip.c (can be suppressed with per-file flags).

---

## Warning Counts by Category (unique instances)

| # | Warning Flag | Count | Danger Level |
|---|---|---|---|
| 1 | `-Wshadow` | 653 | Low-Medium |
| 2 | `-Wmisleading-indentation` | 209 | Medium |
| 3 | `-Wparentheses` | 120 | Medium |
| 4 | `-Wsign-compare` | 109 | Low |
| 5 | `-Wunused-variable` | 88 | Low |
| 6 | `-Wunused-parameter` | 85 | Low |
| 7 | `-Wmaybe-uninitialized` | 81 | **HIGH** |
| 8 | `-Wchar-subscripts` | 73 | Low |
| 9 | `-Wdangling-else` | 55 | Medium |
| 10 | `-Wpointer-to-int-cast` | 35 | **HIGH** |
| 11 | `-Wunused-but-set-variable` | 31 | Low |
| 12 | `-Wpointer-sign` | 29 | Low-Medium |
| 13 | `-Wmissing-field-initializers` | 19 | Low |
| 14 | `-Wempty-body` | 19 | Medium |
| 15 | `-Wformat-nonliteral` | 17 | Medium |
| 16 | `-Wrestrict` | 16 | **HIGH** |
| 17 | `-Wabsolute-value` | 14 | **HIGH** |
| 18 | `-Wreturn-type` | 7 | **HIGH** |
| 19 | `-Wformat-contains-nul` | 7 | Medium |
| 20 | `-Wint-conversion` | 6 | **HIGH** |
| 21 | `-Wunknown-pragmas` | 5 | Low |
| 22 | `-Wuse-after-free` | 4 | **CRITICAL** |
| 23 | `-Wint-to-pointer-cast` | 4 | **HIGH** |
| 24 | `-Wunused-function` | 4 | Low |
| 25 | `-Wunused-value` | 3 | Low |
| 26 | `-Wint-in-bool-context` | 3 | Medium |
| 27 | `-Wreturn-mismatch` | 2 | **HIGH** |
| 28 | `-Wlogical-not-parentheses` + `-Wbool-compare` | 1+1 | **CRITICAL (BUG)** |
| 29 | `-Wunterminated-string-initialization` | 1 | Medium |

**Total: 1,745 unique warnings**

---

## Most Dangerous Warnings (actual bugs or UB)

### CRITICAL: Confirmed Bugs

1. **`divc.c:1854` -- Logical operator precedence bug (`-Wlogical-not-parentheses` + `-Wbool-compare`)**
   - Code: `!ivnom.b[0]!='.'` is parsed as `(!ivnom.b[0]) != '.'`, i.e., `(0_or_1) != 46` -- **always true**.
   - The intended check `ivnom.b[0] != '.'` is completely bypassed. This is in the compiler's `#include` path resolution, meaning the "skip dot-files" guard never fires.

2. **`divforma.c:1410-1417` -- Use-after-free (`-Wuse-after-free`, 4 instances)**
   - `free(CopiaBuffer)` at line 1403, then `buffer` (which may alias `CopiaBuffer`) is used in `memcpy` at lines 1410-1417. The BMP loader reads palette data from a freed buffer.

3. **`i.c:627,640,656` -- Pointer/integer type confusion in stack management (`-Wint-conversion`)**
   - `stack[]` is `uint64_t[]` but pointers (`int32_t*`) are stored/loaded from it with implicit integer/pointer conversions. This works only because 32-bit pointers fit in `uint64_t`, but the signedness and implicit conversions are dangerous -- a refactor could easily introduce a truncation bug.

4. **`i.c:537`, `divsound.c:379` -- Assigning NULL to `char` field (`-Wint-conversion`)**
   - `sonido[n].smp = NULL;` where `.smp` is `char`, not a pointer. This assigns 0 but is a type error -- the field type is wrong (should be a pointer type to hold Mix_Chunk* or NULL).

5. **`i.c:2041` -- Wrong argument type to `OSDEP_JoystickNumHats` (`-Wint-conversion`)**
   - Passes `SDL_Joystick*` where an `int` (joystick index) is expected, or vice versa. Signature mismatch.

### HIGH: Likely bugs or undefined behavior

6. **Missing return values (`-Wreturn-type`, 7 instances)**
   - `div.c:3841,3858`, `divc.c:7370`, `divpack.c:88`, `divkeybo.c:93`, `divsound.c:763,781`
   - Non-void functions that fall off the end without returning. The caller gets garbage. UB in C.

7. **Return mismatch (`-Wreturn-mismatch`, 2 instances)**
   - `div.c:3719,3764` -- `return` with a value in a `void` function. Less dangerous but still UB.

8. **Pointer-to-int casts (`-Wpointer-to-int-cast`, 35 instances)**
   - Spread across `divc.c` (9), `divcolor.c` (4), `divdsktp.c` (4), `divedit.c` (4), `divbasic.c` (4), `divpaint.c` (2), `i.c` (4), `f.c` (3), `divinsta.c` (1). Many store pointers in `int` variables -- works on 32-bit only, would silently corrupt on 64-bit.

9. **`-Wrestrict` overlapping strcpy/sprintf (16 instances)**
   - `divc.c` (4): `strcpy` with potentially overlapping src/dst.
   - `divhelp.c:705`: `sprintf(cerror, ..., cerror)` -- formatting into itself.
   - `kernel.cpp` (8): `strcpy`/`strcat`/`sprintf` with overlapping `mem` buffer.
   - `f.c` (3): same overlapping buffer pattern.
   - These are UB per C11 7.24.2.3 and can corrupt data in optimized builds.

10. **`-Wabsolute-value` -- wrong abs() function (14 instances)**
    - `divfpg.c` and `divhandl.c`: calling `abs()` (parameter type `int`) on `unsigned long long` arguments. The `abs()` truncates the value to `int` before taking the absolute value, and taking absolute value of an unsigned type has no effect. The calculations are wrong.

11. **`-Wmaybe-uninitialized` (81 instances)**
    - Many are in sprite rotation code (`divsprit.c`, `c.c`) where variables like `ix0`, `ix1`, `ig0x`, etc. are set inside conditional branches but used unconditionally. Most are probably false positives (the conditions always hold at runtime), but some are genuinely risky:
      - `divforma.c:240` -- `pSrc` in PCX decompressor
      - `divc.c:4592` -- `ob` in compiler
      - `i.c:1411-1412` -- in mainloop
      - `divsetup.c:816` -- `f` (FILE handle)
      - `divfpg.c:1133` -- `f` (FILE handle)

12. **`ifs.c:831,884,932` -- Multiplication in boolean context (`-Wint-in-bool-context`)**
    - `if (tablaFNT[x].ancho * tablaFNT[x].alto)` -- intended to check if both width and height are nonzero, but multiplication can overflow or be zero if only one is zero. Should use `&&`.

### MEDIUM: Code quality / potential bugs

13. **`-Wshadow` (653 instances)** -- The massive count is almost entirely due to the global variables `r,g,b,c,d,a,f,x,y` in `global.h` being shadowed by local variables in nearly every function. This is the single largest source of warnings. While not directly buggy, it makes the code impossible to reason about (is `a` the global or the local?).

14. **`-Wmisleading-indentation` (209 instances)** -- Code that looks like it's guarded by an if/else but isn't. The `divbrow.c:159-160` case (`strcat(full,"/"); strcat(full,mascara);` both execute regardless of the `if`) is a representative example. Each of these needs manual review to determine if the indentation or the logic is wrong.

15. **`-Wdangling-else` (55 instances)** -- Ambiguous `else` binding in macros or nested if-else chains.

16. **`-Wempty-body` (19 instances)** -- Empty `if` bodies, often from `if (condition);` typos. Some in `div.c:374-379` and `divsound.c:24-27` could be intentional no-ops but deserve verification.

### LOW: Noise (fix opportunistically)

17. **`-Wunused-parameter` (85)** -- Mostly stub functions from the mixer/sound layer that have DOS-era parameters.
18. **`-Wunused-variable` (88)** -- Dead variables, easy cleanup.
19. **`-Wunused-but-set-variable` (31)** -- Set but never read.
20. **`-Wsign-compare` (109)** -- Signed/unsigned comparison, almost all benign in practice.
21. **`-Wchar-subscripts` (73)** -- Using `char` as array index. Mostly harmless since `-funsigned-char` is enabled.
22. **`-Wpointer-sign` (29)** -- `char*` vs `unsigned char*` mismatches.
23. **`-Wmissing-field-initializers` (19)** -- Partial struct initialization, C standard says remaining fields are zero-initialized. Not a bug.

---

## Top Warning-Dense Files

| File | Warnings |
|---|---|
| `src/divc.c` | 172 |
| `src/div.c` | 163 |
| `src/divpaint.c` | 153 |
| `src/runtime/f.c` | 152 |
| `src/runtime/s.c` | 120 |
| `src/divhandl.c` | 78 |
| `src/divbasic.c` | 75 |
| `src/shared/run/c.c` | 72 |
| `src/divedit.c` | 64 |
| `src/divpalet.c` | 57 |

---

## Recommended Fix Priority

**Tier 1 -- Fix immediately (actual bugs / UB):**
1. `divc.c:1854` -- logical-not precedence bug (1 line fix)
2. `divforma.c:1403-1417` -- use-after-free in BMP loader (reorder free)
3. `i.c:537`, `divsound.c:379` -- `sonido[].smp` type is wrong (`char` should be a pointer)
4. `i.c:2041` -- wrong argument type to `OSDEP_JoystickNumHats`
5. `div.c:3841,3858`, `divc.c:7370`, `divpack.c:88`, `divkeybo.c:93`, `divsound.c:763,781` -- add missing return statements (7 sites)
6. `div.c:3719,3764` -- remove return values from void functions
7. `divfpg.c`, `divhandl.c` -- fix `abs()` on unsigned types (use correct cast or `llabs()`)

**Tier 2 -- Fix soon (UB / dangerous patterns):**
8. `-Wrestrict` overlapping buffer operations (16 sites) -- use intermediate buffers or `memmove`
9. `i.c:627,640,656` -- stack[] pointer/integer confusion -- use proper `intptr_t` or typed array
10. Pointer-to-int casts (35 sites) -- replace `(int)ptr` with `(intptr_t)ptr` or `ptrdiff_t`
11. `ifs.c` -- multiplication-as-boolean (3 sites) -- use `&&`
12. Review all 19 `-Wempty-body` sites for accidental semicolons

**Tier 3 -- Systematic cleanup (high volume, low individual risk):**
13. Remove globals `r,g,b,c,d,a,f,x,y` from `global.h` (eliminates ~650 shadow warnings)
14. Fix misleading indentation (209 sites) -- many could be latent logic bugs
15. Add parentheses around assignments in conditionals (120 `-Wparentheses` sites)
16. Initialize variables that trigger `-Wmaybe-uninitialized` (81 sites, start with non-rotation code)
17. Fix dangling-else ambiguity (55 sites)

**Tier 4 -- Long-term hygiene (do alongside other work):**
18. Mark unused parameters with `(void)param;` or remove them (85 sites)
19. Remove unused variables (88 sites)
20. Fix sign-compare mismatches (109 sites)
21. Suppress third-party warnings (38 from miniz.h) with per-file `-w` flags
22. Remove `#pragma check_stack(off)` from `inter.h` (MSVC-only, 5 instances)

**Build system recommendation:** The `DIV_WARNING_BASELINE` cmake option already exists and works. To make incremental progress, consider adding a `-Wno-shadow` override initially (removes 653 warnings, ~37% of total) so the remaining ~1,100 warnings are actionable. Once the globals are eliminated, remove `-Wno-shadow`.
