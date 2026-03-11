# Snake_case Collision Report

Generated: 2026-03-11 | Phase 2C-1 | clang-format 20.1.8
**Updated: 2026-03-11** | Phase 2C-4a: All 18 collisions resolved, 76 functions renamed.

## Summary

- **1,596** unique public identifiers analyzed (functions, globals, structs)
- **1,204** (75%) already snake_case — no change needed
- **392** (25%) needed conversion → **76 resolved** in Phase 2C-4a, ~316 remaining
- **18 collisions** found — **all resolved** via module-prefixed implementation names

## Collisions: Runtime wrapper vs. implementation

All 18 collisions are between:
- **Runtime API wrappers** in `runtime/f.c` (already snake_case: `load_song`, `stop_sound`, etc.)
- **Implementation functions** in `shared/run/` (PascalCase: `LoadSong`, `StopSound`, etc.)

| snake_case target | f.c wrapper | Implementation |
|---|---|---|
| `change_channel` | `change_channel()` | `ChangeChannel()` in divsound.c |
| `change_sound` | `change_sound()` | `ChangeSound()` in divsound.c |
| `end_fli` | `end_fli()` | `EndFli()` in divfli.c |
| `get_song_line` | `get_song_line()` | `GetSongLine()` in divsound.c, divpcm.c |
| `get_song_pos` | `get_song_pos()` | `GetSongPos()` in divsound.c, divpcm.c |
| `is_playing_song` | `is_playing_song()` | `IsPlayingSong()` in divsound.c |
| `is_playing_sound` | `is_playing_sound()` | `IsPlayingSound()` in divsound.c |
| `load_pal` | `load_pal()` | `LoadPal()` in divpalet.c |
| `load_song` | `load_song()` | `LoadSong()` in divsound.c |
| `reset_fli` | `reset_fli()` | `ResetFli()` in divfli.c |
| `reset_sound` | `reset_sound()` | `ResetSound()` in divsound.c |
| `save_map` | `save_map()` in divhandl.c | `save_MAP()` in divforma.c, f.c, v.c |
| `set_song_pos` | `set_song_pos()` | `SetSongPos()` in divsound.c |
| `sort` | `sort()` in f.c | `Sort()` in fpgfile.c |
| `start_fli` | `start_fli()` | `StartFLI()` in divfli.c |
| `stop_song` | `stop_song()` | `StopSong()` in divsound.c |
| `stop_sound` | `stop_sound()` | `StopSound()` in divsound.c |
| `unload_song` | `unload_song()` | `UnloadSong()` in divsound.c |

### Recommended resolution

Add a module prefix to implementation functions:
- `LoadSong()` → `sound_load_song()` (in shared/run/divsound.c)
- `StartFLI()` → `fli_start()` (in shared/run/divfli.c)
- `LoadPal()` → `pal_load()` (in divpalet.c)
- `Sort()` → `fpg_sort()` (in fpgfile.c)
- `save_MAP()` → `format_save_map()` (in divforma.c)

This makes the distinction clear: `load_song()` = runtime API, `sound_load_song()` = implementation.

## Problematic mechanical conversions

The naive camelCase-to-snake_case regex fails on abbreviations and compound words:

| Current | Naive result | Correct target |
|---|---|---|
| `DOSalloc4k` | `do_salloc4k` | `dos_alloc_4k` |
| `DPMIalloc4k` | `dpm_ialloc4k` | `dpmi_alloc_4k` |
| `CDinit` | `c_dinit` | `cd_init` |
| `Can_UpLoad_Desktop` | `can_up_load_desktop` | `can_upload_desktop` |
| `DownLoad_Desktop` | `down_load_desktop` | `download_desktop` |
| `AplieX` | `aplie_x` | needs translation |
| `ComprimeFrameySalva` | `comprime_framey_salva` | needs translation |

**Conclusion:** Automated renaming tools cannot be trusted blindly. Each identifier needs human review, especially abbreviations (DOS, DPMI, FPG, BMP, PCX, RLE) and Spanish names.

## Spanish identifiers still remaining

~90 functions and a few structs/globals still have Spanish names. Key ones:

| Category | Examples |
|---|---|
| FPG operations | `Abrir_FPG`, `Anadir_FPG`, `Borrar_FPG`, `Crear_FPG` |
| Palette | `paleta0-1`, `fusionar_paletas`, `crear_paleta`, `ord_paleta0-3`, `TratarPaleta0-3` |
| Map/image | `mapa0-3`, `nuevo_mapa0-2`, `hay_mapas`, `continua_imagen` |
| Compression | `comprimir_fichero`, `descomprime_BMP/JPG/MAP/PCX/rle` |
| Font | `CargaLetra`, `menu_fuentes0-2` |
| UI | `ratonboton`, `actualiza_titulo`, `arregla_linea` |
| Installer | `crear_instalacion`, `grabador/EndGrabador` |

Note: `color`, `cursor` in English are the same as Spanish — these are NOT translation targets.

## Naming convention decision

**Adopted: `snake_case` for all functions, variables, structs, typedefs.**
**Macros/enum constants: `UPPER_CASE` (already the convention).**

Exceptions (preserved as-is):
- `OSDEP_*` functions (platform abstraction layer convention)
- `SDL_*`, `Mix_*` (external library API)
- `DIV_*` macros
