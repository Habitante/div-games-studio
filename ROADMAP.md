# DIV Games Studio — Roadmap

No deadlines, no pressure. Just a living document for when the mood strikes.

## Phase 1 — Archaeology & Cleanup

The codebase is ~150K lines of mid-90s C with Spanish identifiers, single-letter
variables, dead DOS code paths, and 30 years of accumulated `#ifdef` branches for
platforms that no longer exist. Before changing what it does, we need to understand
and tidy what's there.

### Catalogue & document
- [ ] Map out the full call graph from IDE startup to main loop
- [ ] Map out the compiler pipeline (divc.c): lexer → parser → codegen → bytecode
- [ ] Map out the VM interpreter loop (runtime/i.c) and bytecode format
- [ ] Document the process model: creation, scheduling, FRAME yield, destruction
- [ ] Document the rendering pipeline: 8-bit surface → palette blit → SDL2 texture
- [ ] Document the FPG/MAP/FNT/PAL file formats
- [ ] Document the OSDEP abstraction layer contract
- [ ] Identify and list all global variables and their roles (global.h is... a lot)

### Strip dead code
- [ ] Remove CD-ROM/CDDA player code (cdrom.c, divcdrom.c)
- [ ] Remove dead `#ifdef` branches for defunct platforms (GP2X, Pandora, GCW, PSP, Amiga, Atari ST)
- [ ] Remove commented-out code blocks (there are hundreds)
- [ ] Remove unused cmake toolchain files for platforms we won't support
- [ ] Clean up the bundled sdlgfx — only SDL_framerate.c is actually used

### Code quality
- [ ] Fix all compiler warnings (build with -Wall)
- [ ] Rename the worst single-letter variables where meaning is truly lost
- [ ] Add English comments to the most cryptic sections
- [ ] Normalize string handling (mix of char*, byte*, uint8_t* casts everywhere)
- [ ] Audit the `PrintEvent` pattern — check for similar `#ifdef` body bugs

### Build system
- [ ] Clean up CMakeLists.txt — remove dead options, simplify
- [ ] Add a build script / Makefile wrapper for one-command builds
- [ ] Add a DLL copy step to the build so first-time setup is easier
- [ ] Consider adding a .gitignore for the div/ runtime directory

## Phase 2 — Engine Modernization

These are the changes that make DIV relevant again for making actual games.

### Rendering
- [ ] True color (24/32-bit) rendering mode alongside the classic 8-bit palette
- [ ] Native high-resolution support (not just scaling 320x200)
- [ ] PNG/modern image format support in the editors
- [ ] Alpha blending / transparency

### Language & VM
- [ ] Float/fixed-point type support (currently integer-only)
- [ ] Larger address space (currently limited by int-sized pointers)
- [ ] String improvements
- [ ] Better error messages from the compiler

### IDE
- [ ] Resizable window that doesn't require restart
- [ ] Improved text editor (undo, better selection, maybe syntax colors update)
- [ ] File browser improvements
- [ ] Remove or modernize the first-run "user registration" dialog

### Audio
- [ ] Already mostly modernized via SDL2_mixer
- [ ] Clean up legacy PCM/WAV/Judas sound code paths

## Phase 3 — Ship It

The aspirational endgame. A polished, focused tool for making 2D pixel art games.

- [ ] Steam release with curated examples and tutorials
- [ ] Focus on 2D — pixel art games are thriving
- [ ] Steam Workshop for sharing DIV games?
- [ ] Modern gamepad support
- [ ] Export standalone executables (no runtime needed)
- [ ] Maybe a web export (Emscripten runtime already partially exists)

## Non-goals

Things we're deliberately not doing:

- **MODE8 3D expansion** — it was a regret in 1998, no need to double down
- **Mobile ports** — Android existed but the touch UX for an IDE is painful
- **Rewrite in another language** — the C codebase IS DIV; we clean it, not replace it
- **Backwards-incompatible language changes** — old .PRG files should still compile

## Notes

- The codebase MUST be compiled as 32-bit (sizeof(int) == sizeof(void*) assumed everywhere)
- The `texto[]` array / lenguaje.div system makes localization straightforward
- The OSDEP layer is the right abstraction boundary for platform work
- When in doubt about why something is the way it is, ask Daniel
