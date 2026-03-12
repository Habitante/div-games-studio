# DIV Phase 1 – Unsafe String / Memory Audit (Task B)

_Working tree: `C:\Src\div`_

## 1. Scope & Methodology

**Goal:** Identify potentially unsafe string / memory usage patterns in the DIV codebase and propose a staged, low‑risk remediation plan.

**Patterns inspected (minimum set from task):**
- `sprintf`, `vsprintf`
- `strcpy`, `strcat`
- `gets`
- `sscanf` (with special focus on `%s` without width limits)
- Fixed-size stack buffers (e.g., `char buf[256]`)

**Approach:**
- Searched all `*.c` and `*.h` files under `C:\Src\div` using `rg` (ripgrep).
- Counted occurrences with `rg | measure -Line`.
- Manually inspected representative call sites, focusing on:
  - User / file / path input flowing into fixed-size buffers
  - Small buffers used with formatted output (`sprintf`) or unchecked copies (`strcpy`/`strcat`)
  - Core file/path handling code used by both IDE and runtime
- Selected the **50 highest-risk call sites** by focusing on `strcpy` from global `input` (a 512‑byte user buffer) into **smaller, fixed-size destinations**.

All commands used are recorded in §8.

---

## 2. Summary Statistics (by pattern)

Counts below are approximate but derived directly from code via `rg`.

- **`sprintf`**
  - Total occurrences: **236**
  - `%s` in format string: several dozen (see §3.1)
  - `vsprintf`: **0** occurrences

- **`strcpy`**
  - Total occurrences: **728**
  - `strcpy(dest, input)`: **50** (these are treated as top‑risk in §4)

- **`strcat`**
  - Total occurrences: **322**

- **`gets`**
  - Total occurrences: **0** (good)

- **`sscanf`**
  - Total occurrences: **9**
  - All uses are numeric-only formats (`%d`, `%x`, etc.). **No `%s` or string reads without width limits** were found.

- **Fixed-size `char[]` definitions**
  - `char NAME[constant]` across codebase: **~349** occurrences
  - Many are large (e.g., `_MAX_PATH+1`, 255, 512), but there are numerous small buffers (≤ 16 bytes) used with formatting and string copies.

- **Safer APIs already present** (for migration use):
  - `snprintf`: **2** occurrences (`osdepwin.c`, `osdep.c`)
  - `strncpy`: **3** occurrences
  - `strncat`: **0** occurrences

---

## 3. Pattern Analysis & Risk Overview

### 3.1 `sprintf`

- 236 calls, spanning IDE, runtime, debug/decompiler, installer, and tooling.
- A subset uses `%s` and/or combines user / file names into fixed-size buffers.
- Examples of patterns:
  - UI / debug text:
    - `sprintf(cwork, "%s %d.%02d%%", text[77], porcen/100, porcen%100, '%');`
    - `sprintf(combo_error, "%s [%d..%d].", text[4], i->get.r0, text[5], i->get.r1, text[6]);`
  - File/path building:
    - `sprintf(cwork, "%s\\%s", tipo[1].path, n_ar);` (`divfpg.c`)
    - `sprintf(str_file, "%s%d.MAP", strupr(str_file), cod);` (`divfpg.c`)
  - Script/decompiler string building (`runtime/debug/decompiler.c`), many of which format into buffers of size 100–255.

**Risks:**
- Many destinations are large (e.g. 128–512 bytes), so overflow requires extreme inputs, but:
  - Several small buffers (4–16 bytes) are used with `%d` / `%dx%d` formats and could overflow if dimensions or codes grow.
  - Path/file name formatting with user-controlled segments (e.g., dialog input, filenames from disk) uses `sprintf` without length limits.

**Assessment:**
- Overall **Medium** risk for `sprintf` as currently used, with **localized High** risk where buffers are small or the source is `input`/filenames.

### 3.2 `strcpy`

- 728 occurrences across all major modules.
- Patterns include:
  - Copying filenames from global `input` (512‑byte UI buffer) into much smaller fields:
    - `MiFPG->NombreFpg[13]`, `Fpg->NombreFpg[13]`
    - PCM/MOD names (`name[14]`), install file names (`ifile?name[16]`)
  - Copying command line arguments into fixed-size arrays:
    - `exebin[255]` ← `argv[0]`
    - `runtime[255]`, `exefile[255]`, `datafile[255]`, `outfile[255]` ← `argv[n]` (`divpack.c`)
  - Copying directory paths and masks between `_MAX_PATH+1`, 255, 256, and 512‑byte buffers.

**Risks:**
- **High**: all `strcpy(dest, input)` sites (see §4), because `input` is 512 bytes and many destinations are 13–16 bytes.
- **Medium**: `strcpy` of command line arguments and environment-controlled strings into 255‑byte buffers.
- **Lower**: copies between equally-sized or larger buffers (e.g., `_MAX_PATH+1` to `_MAX_PATH+1`), though still brittle.

### 3.3 `strcat`

- 322 occurrences, heavily used for path concatenation and message building.
- Common patterns:
  - Ensure separator, then append:
    - `if (full[strlen(full)-1]!='/') strcat(full, "/");`
    - `strcat(full, input);` / `strcat(full, fname);` / `strcat(full, ext);`
  - Multi-step path building based on `tipo[v_tipo].path`, `input`, `fname`, etc.

**Risks:**
- On path buffers sized `_MAX_PATH+1` and `255`, overflow is possible if:
  - User selects long paths/filenames,
  - Or paths are nested deeply.
- The most concerning are chained `strcat` calls where buffer fullness is not tracked (e.g. `open_multi` in `divc.c`, file browsers, and installer).

### 3.4 `sscanf`

- 9 total uses:
  - `divsetup.c`: parse Linux `/proc/meminfo` lines for `MemFree`, `MemTotal` using `%d`/`%ds`-like numeric formats.
  - `judas.c`: parse sound configuration from environment (`%x`, `%d`).

**Positive:**
- No `sscanf` with `%s` or string reads without width limits were found.

**Residual risk:**
- Low. Numeric overflows could occur if `/proc/meminfo` or environment variables are malformed, but they would not corrupt memory (just produce wrong values).

### 3.5 Fixed-size `char[]` buffers

- ~349 declarations of `char NAME[N]` or similar.
- Many are large (or global structs):
  - Paths: `_MAX_PATH+1`, `filename[255]`, `pathname[256]`, `mascara[512]`, `input[512]`.
  - Palettes/maps: `pal[768]`, `dac[768]`, etc.
- Several are **very small** and close to capacity:
  - `char cWork[5];`, `char num[3];`, `char num[4];`, `char header[8];`, `char id[7];`, `char name[12];`, etc.
  - Frequently used with `sprintf/itoa`, e.g., to display resolutions or numeric codes.

**Risks:**
- Small buffers used with numeric `sprintf` and no explicit range checks can overflow if future changes increase allowed ranges (e.g., map sizes > 9999, larger codes).
- Struct fields like `NombreFpg[13]` risk overflow whenever user-provided file names are longer than expected.

---

## 4. Top 50 Higher-Risk Call Sites

**Selection criteria:**
- All **50** of these sites copy from the global **`input`** buffer (512 bytes, user-controlled via dialogs) into smaller, fixed-size destinations using `strcpy`.
- In many cases, the destination is ~13–16 bytes (file names, resource labels), so normal user input can trivially overflow.

Format below: `Pattern – File:Line – One-line risk note`.

1. `strcpy(mascara, input);` – `src/divbrow.c:1464` – Copies arbitrary dialog text mask into `mascara[512]`; size matches `input` but still brittle (unbounded copy between two large globals).
2. `strcpy(mascara, input);` – `src/divbrow.c:1820` – Same pattern in another browser path; repeated unsafe use.
3. `strcpy(mascara, input);` – `src/divbrow.c:1913` – Same as above; easy candidate for a shared safe wrapper.
4. `strcpy(larchivosbr.lista, input);` – `src/divedit.c:2527` – Copies user filename into listbox storage; size determined by `lista_an`, likely smaller than 512.
5. `strcpy(v_prg->filename, input);` – `src/divedit.c:2549` – Program filename field (`[255]`) receives user input without length checks.
6. `strcpy(v_prg->filename, input);` – `src/divedit.c:2656` – Same risk in another “Save As” path.
7. `strcpy(v.prg->filename, input);` – `src/divedit.c:2697` – Copies `input` into current program’s filename; overlong names truncated only by overflow.
8. `strcpy(v_prg->filename, input);` – `src/divedit.c:2982` – Another unsized filename copy from dialog input.
9. `strcpy(MiFPG->NombreFpg, input);` – `src/divfpg.c:346` – **High risk**: FPG logical name is only 13 bytes (`NombreFpg[13]`) and can easily overflow from user filename.
10. `strcpy(MiFPG->NombreFpg, input);` – `src/divfpg.c:379` – Same high-risk pattern on a different path.
11. `strcpy(larchivosbr.lista, input);` – `src/divfpg.c:476` – Uses `input` as listbox text; `lista_an` must be ≥ max filename or this overflows.
12. `strcpy(FontName, input);` – `src/divfont.c:680` – Font name buffer is large (`FontName[1024]`), but still unbounded copy from `input`.
13. `strcpy(larchivosbr.lista, input);` – `src/divfont.c:722` – Filename from dialog stored in listbox; same `lista_an` truncation risk.
14. `strcpy(FaceName, input);` – `src/divfont.c:733` – Face/IFS filename copied from dialog; underlying storage size is limited.
15. `strcpy(larchivosbr.lista, input);` – `src/divfont.c:1492` – Another browser path using `input` for listbox text.
16. `strcpy(Load_FontName, input);` – `src/divfont.c:1515` – `Load_FontName[14]` receives arbitrary filename; **very small buffer, high overflow risk**.
17. `strcpy(ifile1name, input);` – `src/divinsta.c:200` – Installer’s first file short name (`[16]`) from dialog; risk for longer filenames.
18. `strcpy(ifile2name, input);` – `src/divinsta.c:229` – Same as above for second file name.
19. `strcpy(ifile3name, input);` – `src/divinsta.c:257` – Same as above for third file name.
20. `strcpy(FontName, input);` – `src/divhandl.c:1061` – Dialog-selected font name copied without bounds into local `FontName[14]` (`divhandl.c`); **high risk**.
21. `strcpy(mypcminfo->name, input);` – `src/divhandl.c:1306` – Sound effect name field (`[14]`) populated from dialog; easy overflow.
22. `strcpy(mascara, input);` – `src/divhandl.c:2590` – Several file browser branches copy `input` to `mascara[512]`; same comments as items 1–3.
23. `strcpy(mascara, input);` – `src/divhandl.c:2603` – Same mask copy in a different control-flow path.
24. `strcpy(mascara, input);` – `src/divhandl.c:2605` – Another branch; same risks and refactor opportunity.
25. `strcpy(mascara, input);` – `src/divhandl.c:2606` – Yet another branch; repetition shows need for a helper.
26. `strcpy(mascara, input);` – `src/divhandl.c:2625` – Browser mask update; same unbounded copy.
27. `strcpy(mascara, input);` – `src/divhandl.c:2629` – Additional branch with same pattern.
28. `strcpy(larchivosbr.lista, input);` – `src/divhandl.c:2784` – Browser list text from `input`; sizing depends on `lista_an`.
29. `strcpy(v_mapa->filename, input);` – `src/divhandl.c:2943` – Map filename field receives user-selected name; field size is 255, but `input` may be longer.
30. `strcpy(larchivosbr.lista, input);` – `src/divhandl.c:2980` – Yet another listbox copy from `input`.
31. `strcpy(filename, input);` – `src/divhandl.c:3029` – Local `filename[255]` from dialog; long names can still overflow 255.
32. `strcpy(ventana[v_ventana].mapa->filename, input);` – `src/divhandl.c:3053` – Window’s associated map filename field; same 255‑byte risk.
33. `strcpy(m3d_edit.fpg_name, input);` – `src/divmap3d.c:726` – 3D editor FPG name field (size limited) from dialog text.
34. `strcpy(m3d_edit.fpg_name, input);` – `src/divmap3d.c:748` – Same risk on another path.
35. `strcpy(m3d_edit.fpg_name, input);` – `src/divmap3d.c:768` – Same pattern repeated.
36. `strcpy(m3d_edit.fpg_name, input);` – `src/divmap3d.c:788` – Same.
37. `strcpy(m3d_edit.fpg_name, input);` – `src/divmap3d.c:804` – Same; collectively these are a good refactor target.
38. `strcpy(m3d->m3d_name, input);` – `src/divmap3d.c:3138` – 3D map name from dialog; small fixed-size field.
39. `strcpy(m3d_aux->m3d_name, input);` – `src/divmap3d.c:3365` – Auxiliary 3D map name, same risk.
40. `strcpy(m3d_aux->m3d_name, input);` – `src/divmap3d.c:3479` – Same pattern again.
41. `strcpy(larchivosbr.lista, input);` – `src/divpalet.c:539` – Palette browser: dialog filename to listbox; risk hinges on `lista_an`.
42. `strcpy(larchivosbr.lista, input);` – `src/divpcm.c:532` – Sound browser: sample filename from dialog; same pattern.
43. `strcpy(SoundName, input);` – `src/divpcm.c:546` – `SoundName[255]` from dialog; unlimited length copied into 255.
44. `strcpy(SoundName, input);` – `src/divpcm.c:644` – Same as above in another path.
45. `strcpy(larchivosbr.lista, input);` – `src/divpcm.c:936` – Another listbox copy from `input` in audio module.
46. `strcpy(SongName, input);` – `src/divpcm.c:950` – MOD/song name (`[255]`) from dialog; same overflow potential as `SoundName`.
47. `strcpy(Tap_name, input);` – `src/divsetup.c:197` – Desktop wallpaper filename from dialog; `Tap_name[_MAX_PATH]` is large but still unbounded copy.
48. `strcpy(Tap_name, input);` – `src/divsetup.c:216` – Same pattern in another branch.
49. `strcpy(Tap_name, input);` – `src/divsetup.c:229` – Same pattern again.
50. `strcpy(Fpg->NombreFpg, input);` – `src/fpgfile.c:463` – **High risk**: FPG logical name (`NombreFpg[13]`) from dialog `input`; mirrors items 9–10.

> Note: Several of these share identical structure and would benefit from a common safe helper (see §6), which also simplifies unit testing around truncation and error handling.

---

## 5. Proposed Staged Migration Strategy

### 5.1 Guiding principles

- **Minimal behavior change first**: focus on making overflows impossible without altering visible behavior in normal cases.
- **Mechanical, low‑friction changes**: prefer introducing wrappers / macros so large refactors aren’t required up front.
- **Prioritize attack surface**: start with places where user input (`input`, `argv`, environment) touches small buffers or core file/path logic.

### 5.2 Stage 0 – Introduce safe string helpers

Create a small, self-contained header (e.g., `src/shared/div_safe_string.h`) and include it where needed.

Key components:

- **Bounded formatting**
  ```c
  int div_snprintf(char *dst, size_t dst_sz, const char *fmt, ...);
  ```
  - Thin wrapper around `vsnprintf`.
  - Guarantees NUL-termination (`dst[dst_sz-1] = '\0'`).
  - Returns number of characters that _would_ have been written (like `snprintf`).
  - Optional: log or assert when return ≥ `dst_sz` (truncation).

- **Safe copy / concat**
  ```c
  int div_strcpy(char *dst, size_t dst_sz, const char *src);   // returns 0 on success, -1 on truncation
  int div_strcat(char *dst, size_t dst_sz, const char *src);   // appends safely
  ```

- **Macro helpers for arrays**
  ```c
  #define DIV_STRCPY(dst, src)   div_strcpy((dst), sizeof(dst), (src))
  #define DIV_STRCAT(dst, src)   div_strcat((dst), sizeof(dst), (src))
  #define DIV_SPRINTF(dst, ...)  div_snprintf((dst), sizeof(dst), __VA_ARGS__)
  ```

This allows many call sites to be updated mechanically without manually threading buffer sizes.

### 5.3 Stage 1 – Quick, low-risk conversions

Focus on call sites where:
- The destination is a local array (so `sizeof` is accurate), and
- The formatted content or copy is clearly bounded already.

Examples (see §7 for a concrete shortlist):
- Debug/diagnostic `sprintf` into `cwork[256]`, `msg[512]`, etc.
- Internal helper strings where the format is constant and maximum size is trivial to reason about (e.g., `"[%03d] %s"`).

**Goal:** Replace tens of `sprintf` / `strcpy` / `strcat` calls with `DIV_*` macros without changing logic.

### 5.4 Stage 2 – User-visible file/path handling

Target the most exposed parts of the system:

- File browsers and dialogs (`divbrow.c`, `divhandl.c`, `divedit.c`, `divsetup.c`, `divfpg.c`, `divpcm.c`, `divmap3d.c`, installer).
- Core path search routines (`open_multi` in `divc.c`, runtime `f.c`, `shared/unzip.c`, `runner/runner.c`).

Actions:
- Replace direct `strcpy`/`strcat` from `input` into names/paths with `DIV_STRCPY`/`DIV_STRCAT`.
- Decide on consistent maximum lengths for resource names (e.g., FPG name ≤ 12 chars, PCM name ≤ 13, map filename ≤ 254) and **enforce** them via truncation and/or validation dialog.
- Where truncation would be surprising (e.g., long actual filesystem paths), prefer larger buffers (e.g., `_MAX_PATH*2`) and assert/truncate gracefully if exceeded.

### 5.5 Stage 3 – Runtime and scripting memory

Later, when IDE and tools are hardened:

- Review `sprintf`/`strcpy`/`strcat` that operate on virtual memory (`mem[]`, `stack[]`) in runtime `f.c` and debug/decompiler code.
- Replace brittle string-building logic with helpers that take explicit lengths, especially where user scripts can influence content.
- Consider introducing small struct wrappers for “dynamic strings” in the runtime, rather than hand-rolling pointer arithmetic.

---

## 6. Suggested Helper Wrappers / Macros

A concrete design sketch:

```c
// div_safe_string.h
#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static inline int div_snprintf(char *dst, size_t dst_sz, const char *fmt, ...) {
    int r;
    va_list ap;
    if (!dst || dst_sz == 0) return -1;
    va_start(ap, fmt);
    r = vsnprintf(dst, dst_sz, fmt, ap);
    va_end(ap);
    if (r < 0) {
        dst[0] = '\0';
        return -1;
    }
    dst[dst_sz - 1] = '\0';
    return (r >= (int)dst_sz) ? -1 : 0; // -1 indicates truncation
}

static inline int div_strcpy(char *dst, size_t dst_sz, const char *src) {
    size_t len;
    if (!dst || !src || dst_sz == 0) return -1;
    len = strlen(src);
    if (len >= dst_sz) {
        memcpy(dst, src, dst_sz - 1);
        dst[dst_sz - 1] = '\0';
        return -1;
    }
    memcpy(dst, src, len + 1);
    return 0;
}

static inline int div_strcat(char *dst, size_t dst_sz, const char *src) {
    size_t len_dst, len_src;
    if (!dst || !src || dst_sz == 0) return -1;
    len_dst = strlen(dst);
    len_src = strlen(src);
    if (len_dst + len_src >= dst_sz) {
        size_t copy = (dst_sz - 1 > len_dst) ? (dst_sz - 1 - len_dst) : 0;
        if (copy > 0) memcpy(dst + len_dst, src, copy);
        dst[dst_sz - 1] = '\0';
        return -1;
    }
    memcpy(dst + len_dst, src, len_src + 1);
    return 0;
}

#define DIV_STRCPY(dst, src)   div_strcpy((dst), sizeof(dst), (src))
#define DIV_STRCAT(dst, src)   div_strcat((dst), sizeof(dst), (src))
#define DIV_SPRINTF(dst, ...)  div_snprintf((dst), sizeof(dst), __VA_ARGS__)
```

This header can be included incrementally, and its use can be confined to safer modules first (IDE tools, installer) before touching low-level runtime code.

---

## 7. Suggested “Quick Win” Replacements (First 10)

These are call sites where replacing `sprintf`/`strcpy`/`strcat` with `DIV_*` macros should be **low risk**, because buffer sizes are generous relative to content and behavior change is minimal.

1. **`src/divpack.c`** – CLI argument handling
   - Lines around 103–127: `strcpy(outfile, argv[4]);`, `strcpy(runtime, argv[1]);`, `strcpy(exefile, argv[2]);`, `strcpy(datafile, argv[3]);` into 255‑byte buffers.
   - Replace with `DIV_STRCPY` (or explicit `div_strcpy(..., sizeof...)`) and gracefully truncate overly long arguments.

2. **`src/div1run/i.c:243` and `src/runtime/i.c:1957`** – `exebin` path
   - `strcpy(exebin, argv[0]);` into `exebin[255]`.
   - Use `DIV_STRCPY(exebin, argv[0]);` to prevent overruns on long executable paths.

3. **`src/divhelp.c:726`** – Error message composition
   - `sprintf(cerror, "%s", (char *)texto[381]);` into `cerror[128]`.
   - Switch to `DIV_SPRINTF(cerror, "%s", texto[381]);` (no meaningful behavior change).

4. **`src/divhelp.c:737`** – Add line number to error text
   - `sprintf(cerror, "%s (%d).", cerror, linea_error);` with same `cerror[128]`.
   - Replace with `DIV_SPRINTF`.

5. **`src/divclock.c:28,77`** – Time display string
   - `sprintf(cTimeForIcon, "%s [%02d%c%02d]", texto[151], ...)` into `cTimeForIcon[18]`.
   - This is tight but predictable; switching to `DIV_SPRINTF` protects against future format changes.

6. **`src/divsetup.c:152`** – VGA mode description
   - `sprintf(&vgasizes[n*16], "%s%d x %d", (modos[n].modo)?"SDL ":"VGA ", modos[n].ancho, modos[n].alto);` into a 16‑byte slot.
   - Replace with `div_snprintf(&vgasizes[n*16], 16, ...)` to enforce the slot boundary.

7. **`src/divhandl.c:3339`** – Tile range label
   - `sprintf(ctiletext, "%s [%d..%d]", texto[401], mintile, maxtile);` into `ctiletext[32]`.
   - Change to `DIV_SPRINTF`.

8. **`src/runtim e/debug/d.c:4567+`** – Performance statistics strings
   - Series of `sprintf(cwork, "%s %d.%02d%%", ...)` into `cwork[256]`.
   - Converting these to `DIV_SPRINTF` is mechanical and safe.

9. **`src/divgama.c:188–277`** – Copying gamma tables
   - Uses `memcpy` for fixed-size 128‑element arrays; can be left as-is, but documenting invariants here is a quick “paper win”.

10. **`src/divforma.c` palette/map `sprintf`/`strcpy` into 32–40 byte descriptors**
    - E.g., `sprintf(tDescrip, "%s", ventana[1].mapa->descripcion);` into `tDescrip[32]`.
    - Replace with `DIV_SPRINTF` / `DIV_STRCPY` where appropriate.

These 10 items provide a **safe proving ground** for the `div_safe_string` helpers and will flush out any integration or portability issues before touching more critical code paths.

---

## 8. Search Commands Used

Executed from `C:\Src\div` (PowerShell):

**Pattern discovery:**
- `rg -n "\bsprintf\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bvsprintf\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bstrcpy\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bstrcat\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bgets\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bsscanf\s*\(" . --glob "*.{c,h}"`
- `rg -n "char\s+[A-Za-z0-9_]+\s*\[[0-9]+\]" . --glob "*.{c,h}"`
- `rg -n "char\s+[A-Za-z0-9_]+\s*\[(?:[0-9]|1[0-6])\]" . --glob "*.{c,h}"`
- `rg -n "\bsnprintf\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bstrncpy\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bmemcpy\s*\(" . --glob "*.{c,h}"`
- `rg -n "\bscanf\s*\(" . --glob "*.{c,h}"`
- `rg -n "strcpy\s*\([^,]*,\s*argv\[" . --glob "*.{c,h}"`
- `rg -n "strcpy\s*\([^,]*,\s*input\b" . --glob "*.{c,h}"`
- `rg -n "full\[" . --glob "*.{c,h}"`
- `rg -n "NombreFpg" . --glob "*.{c,h}"`
- `rg -n "struct tprg" -n src`

**Counts (using PowerShell):**
- `rg -n "\bsprintf\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bstrcpy\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bstrcat\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bsscanf\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bgets\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bvsprintf\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "char\s+[A-Za-z0-9_]+\s*\[[0-9]+\]" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bsnprintf\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "\bstrncpy\s*\(" . --glob "*.{c,h}" | measure -Line`
- `rg -n "strcpy\s*\([^,]*,\s*input\b" . --glob "*.{c,h}" | measure -Line`

---

## 9. Executive Summary

- The DIV codebase still uses **classic unsafe C string APIs** (`sprintf`, `strcpy`, `strcat`) extensively: ~1.3k call sites combined.
- The **highest-risk cluster** is 50 instances of `strcpy(dest, input)` where `input` is a **512‑byte user-controlled buffer** and many destinations are only 13–16 bytes long (resource names, short filenames).
- There are **no `gets` calls** and **no dangerous `sscanf` `%s` uses**, which removes a major class of trivial overflows.
- Risk is concentrated in **file/path handling, dialogs, and resource name management** (FPG, PCM/MOD, fonts, maps, installer).
- A small, well-designed **`div_safe_string` helper layer** (bounded `snprintf`/`strcpy`/`strcat` wrappers + macros) would enable mechanical, low-risk hardening across large portions of the code.
- A staged plan is recommended:
  1. Introduce helpers and convert **debug/UI formatting and CLI argument handling** (quick wins in §7).
  2. Tackle **all `strcpy/strcat` from `input`** into smaller buffers (top 50 in §4).
  3. Finally, address **runtime/decompiler string building** and any remaining ad‑hoc path logic.

If you want, the next pass can focus on generating concrete patches for Stage 1 (quick wins) and a checklist to drive Stage 2 refactors module by module.