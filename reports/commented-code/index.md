# Commented-Out Code Audit — Master Index

**Date:** 2026-03-07
**Scope:** 52 source files audited across src/, runtime/, shared/, judas/, win/, runner/
**Method:** Each file compared against original DOS source (Back01) where available

## Aggregate Summary

| Metric | Value |
|--------|-------|
| Files audited | 52 |
| Total blocks found | ~452 |
| Total commented-out lines | ~1,500 |
| **Safe to remove** | **~388 blocks, ~1,350 lines** |
| Needs human review | ~25 blocks, ~120 lines |
| Keep (legitimate comments) | ~39 blocks |

**~92% of all commented-out code is safe to delete.**

## Review Items (Bugs / Incomplete Porting)

These are the ~25 blocks where Mike commented out active DOS code during the SDL2 port,
indicating incomplete porting work or behavioral changes that need human decision.

### Recurring Pattern: `judascfg_device == DEV_NOSOUND` shims
Mike replaced Judas sound library device checks with hardcoded `false`/`true` across multiple files.
This means audio-dependent code runs unconditionally even when audio init fails.
**Recommendation:** Create `OSDEP_AudioAvailable()` and replace all shims.
- divdsktp.c (lines 712-714)
- divbrow.c (1 block)
- divhandl.c (3 blocks)
- divmixer.c (2 blocks)

### Sound System
- **divdsktp.c**: `#ifdef NOTYET` around case 105 in both `DownLoad_Desktop` and `UpLoad_Desktop` — PCM sound session save/load completely disabled
- **divpcm.c**: `free(pcminfo_aux)` commented out in error paths — memory leaks in `OpenSound()`/`OpenSoundFile()`
- **divpcm.c**: Three `#ifdef NOTYET` stubs: `OpenDesktopSong`, `ChangeSoundFreq`, `PasteNewSounds` — broken sound editor features

### Runtime Semantics
- **runtime/f.c** (lines 2153-2158): `stop_sound()` lost its "stop all channels" branch when `pila[sp]==-1`
- **runtime/f.c** (lines 2567-2572): `fade_off()` changed from blocking to non-blocking — changes DIV program behavior
- **runtime/i.c** (line 689): `//free()` is NOT a memory leak (would be double-free) — MEMORY.md landmine entry is wrong, needs correction

### Video / Display
- **div.c**: `test_video` dialog (Block 7), `check_free()` disk space check (Block 11), `command_path()`/`shell()` DOS function (Block 12) — all need decision
- **divpalet.c** (line 1367): `case 4: descomprime_JPG()` commented out — JPEG desktop wallpaper support broken

### Input
- **divmouse.c**: Spacebar-as-mouse-button logic was active in DOS, Mike commented it out — needed for keyboard-driven paint editing

### Installer (non-critical)
- **divinsta.c**: Entire thumbnail rendering body of `crear_imagen_install()` commented out (48 lines) — installer is non-functional on SDL2 anyway

### Platform / Path Resolution
- **osdepwin.c + osdep.c**: `realpath()` stubbed with `strcpy()` — no actual path resolution
- **osd_sdl2.c**: Video mode changes silently ignored when window already exists

## Bonus Findings (Active Bugs, Not Commented-Out)

Found during the audit but not part of the commented-out code removal:
- **divc.c** (line 1872): Active `fprintf(stdout,"FOUND FILE: ...")` debug print left by Mike
- **divwindo.c**: Dangling-if bug — commenting out printf inside for/if made a NULL check a no-op
- **divtimer.c**: Entire file is effectively dead (`#ifdef NOTYET` around everything). Candidate for deletion.
- **mode7.c**: Standalone SDL1.2 demo with its own `main()`. Not part of build. Delete candidate.
- **runner/runner.c**: DOS-era Watcom launcher. Not compiled. Delete candidate.
- **divhelp.c**: 10 of 15 blocks are `//free()` — deliberate pattern to avoid use-after-free

## Per-File Reports

### IDE / Main (originals in Back01/DIV/)
| Report | Blocks | Remove | Review | Keep |
|--------|--------|--------|--------|------|
| [div.md](div.md) | 30 | 24 | 3 | 3 |
| [divc.md](divc.md) | 17 | 14 | 0 | 3 |
| [divdsktp.md](divdsktp.md)* | 31 | 19 | 3 | 9 |
| [divedit.md](divedit.md) | 7 | 5 | 0 | 2 |
| [diveffec.md](diveffec.md) | 7 | 7 | 0 | 0 |
| [divfont.md](divfont.md) | 7 | 7 | 0 | 0 |
| [divforma.md](divforma.md) | 3 | 3 | 0 | 0 |
| [divfpg.md](divfpg.md) | 6 | 6 | 0 | 0 |
| [divhandl.md](divhandl.md) | 19 | 15 | 4 | 0 |
| [divhelp.md](divhelp.md) | 12 | 10 | 0 | 2 |
| [divinsta.md](divinsta.md) | 10 | 7 | 3 | 0 |
| [divmixer.md](divmixer.md) | 8 | 5 | 2 | 1 |
| [divmouse.md](divmouse.md) | 7 | 5 | 2 | 0 |
| [divpaint.md](divpaint.md)* | 12 | 12 | 0 | 0 |
| [divpalet.md](divpalet.md) | 6 | 5 | 1 | 0 |
| [divpcm.md](divpcm.md) | 28 | 19 | 5 | 4 |
| [divsound.md](divsound.md) | 5 | 4 | 0 | 1 |
| [divvideo.md](divvideo.md) | 20 | 19 | 0 | 1 |
| [divwindo.md](divwindo.md) | 15 | 15 | 0 | 0 |
| [divsetup.md](divsetup.md) | 13 | 13 | 0 | 0 |
| [global.md](global.md) | 8 | 7 | 0 | 1 |
| [ifs.md](ifs.md) | 10 | 10 | 0 | 0 |
| [divbrow.md](divbrow.md) | 10 | 8 | 1 | 1 |
| [divkeybo.md](divkeybo.md) | 4 | 4 | 0 | 0 |
| [divtimer.md](divtimer.md) | 7 | 7 | 0 | 0 |
| [det_vesa.md](det_vesa.md) | 1 | 1 | 0 | 0 |
| [divbrush.md](divbrush.md) | 0 | 0 | 0 | 0 |
| [divbasic.md](divbasic.md) | 0 | 0 | 0 | 0 |

*divdsktp.md and divpaint.md from pilot run (not in parsed totals above)

### Runtime (originals in Back01/DIV/SRC/SOURCE/)
| Report | Blocks | Remove | Review | Keep |
|--------|--------|--------|--------|------|
| [runtime-f.md](runtime-f.md) | 41 | 31 | 3 | 7 |
| [runtime-i.md](runtime-i.md) | 54 | 51 | 1 | 2 |
| [runtime-s.md](runtime-s.md) | 2 | 0 | 0 | 2 |
| [runtime-inter.md](runtime-inter.md) | 7 | 5 | 0 | 2 |
| [runtime-debug-d.md](runtime-debug-d.md) | 17 | 15 | 0 | 2 |
| [runtime-debug-decompiler.md](runtime-debug-decompiler.md) | 34 | 34 | 0 | 0 |
| [runtime-divmixer.md](runtime-divmixer.md) | 6 | 6 | 0 | 0 |
| [runtime-divlengu.md](runtime-divlengu.md) | 3 | 3 | 0 | 0 |

### Shared / Platform (mixed origins)
| Report | Blocks | Remove | Review | Keep |
|--------|--------|--------|--------|------|
| [shared-run-divkeybo.md](shared-run-divkeybo.md) | 10 | 7 | 0 | 3 |
| [shared-run-v.md](shared-run-v.md) | 10 | 9 | 0 | 1 |
| [shared-run-divfli.md](shared-run-divfli.md) | 8 | 8 | 0 | 0 |
| [shared-run-ia.md](shared-run-ia.md) | 3 | 1 | 0 | 2 |
| [shared-run-divsound.md](shared-run-divsound.md) | 6 | 6 | 0 | 0 |
| [shared-run-c.md](shared-run-c.md) | 0 | 0 | 0 | 0 |
| [shared-run-topflc.md](shared-run-topflc.md) | 0 | 0 | 0 | 0 |
| [shared-osdep.md](shared-osdep.md) | 13 | 12 | 1 | 0 |
| [shared-osdep-osd_sdl2.md](shared-osdep-osd_sdl2.md) | 8 | 6 | 1 | 1 |
| [shared-lib-mode7.md](shared-lib-mode7.md) | 8 | 8 | 0 | 0 |
| [win-osdepwin.md](win-osdepwin.md) | 17 | 15 | 1 | 1 |
| [runner-runner.md](runner-runner.md) | 4 | 4 | 0 | 0 |

### Judas Sound Library (originals in Back01/DIV/SRC/JUDAS/)
| Report | Blocks | Remove | Review | Keep |
|--------|--------|--------|--------|------|
| [judas-judas.md](judas-judas.md) | 5 | 5 | 0 | 0 |
| [judas-judaswav.md](judas-judaswav.md) | 4 | 4 | 0 | 0 |
| [judas-judasraw.md](judas-judasraw.md) | 3 | 3 | 0 | 0 |
| [judas-judass3m.md](judas-judass3m.md) | 1 | 1 | 0 | 0 |
| [judas-judasxm.md](judas-judasxm.md) | 1 | 1 | 0 | 0 |
| [judas-judasmod.md](judas-judasmod.md) | 1 | 1 | 0 | 0 |
