# Review Items -- Commented Code Audit

All 28 items resolved.

Resolved: 9 items on 2026-03-08 (SoundActive shims, stop_sound, false alarm),
5 more (item 4 memory leak fixed, items 14, 17, 18, 19 already cleaned up),
item 3 (io.h include fixed with platform guard),
item 6 (ChangeSoundFreq ported to SDL2_mixer),
item 8 (fade_off blocking semantics restored via fade_wait helper),
items 1, 2, 5, 7 (PCM session save/load, OpenDesktopSong, PasteNewSounds ported to SDL2_mixer),
item 9 (moved to ROADMAP Phase 1 "Fix video mode / display system"),
item 10 (check_free removed — obsolete disk space check),
item 11 (video mode resize: added SDL_SetWindowSize),
items 12, 13 (_fullpath: restored realpath with strcpy fallback),
item 15 (JPEG support enabled: HAS_JPEG=1, wallpaper case restored, header guard added),
item 16 (dead command_path/shell removed).
