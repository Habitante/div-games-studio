# f.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 41 |
| Total commented-out lines | ~80 |
| Safe to remove | 31 blocks, ~48 lines |
| Needs review | 3 blocks, ~10 lines |
| Keep | 7 blocks, ~22 lines |

## Blocks

### Block 1: Lines 71-72
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines of an earlier SDL timing attempt (`reloj = SDL_GetTicks()/10; return reloj;`). Mike wrote then abandoned these in favor of the delta-time approach on lines 74-79.
**Recommendation:** REMOVE

---

### Block 2: Line 78
**Category:** MIKE_ADDED_DEAD
**Summary:** `//reloj=SDL_GetTicks()/10;` -- another abandoned SDL timing attempt, same as Block 1.
**Recommendation:** REMOVE

---

### Block 3: Line 147
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",fprgpath);` -- debug trace in Mike's new `__fpopen()` function (not in original).
**Recommendation:** REMOVE

---

### Block 4: Line 150
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Found %s in prg dir [%s]\n",file, prgpath);` -- debug trace in Mike's new `__fpopen()`.
**Recommendation:** REMOVE

---

### Block 5: Line 185
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",file);` -- debug trace in Mike's rewritten `open_multi()`.
**Recommendation:** REMOVE

---

### Block 6: Line 191
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 7: Line 212
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 8: Line 224
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 9: Line 237
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 10: Line 249
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 11: Line 261
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open %s\n",full);` -- debug trace in `open_multi()`.
**Recommendation:** REMOVE

---

### Block 12: Line 301
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"Trying to open from zip %s\n",file);` -- debug trace in zip file open path.
**Recommendation:** REMOVE

---

### Block 13: Line 384
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout,"trying to read %s from %d files\n",file,npackfiles);` -- debug trace in `read_packfile()`.
**Recommendation:** REMOVE

---

### Block 14: Line 404
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("looking for %s against %s\n",full,packdir[n].filename);` -- debug trace in packfile search.
**Recommendation:** REMOVE

---

### Block 15: Line 572
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Freeing %lx\n ",(g[0].grf[pila[sp]]));//-1330);` -- debug trace in `unload_map()`.
**Recommendation:** REMOVE

---

### Block 16: Line 819
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("num is %d\n",num);` -- debug trace in `load_fpg()`.
**Recommendation:** REMOVE

---

### Block 17: Line 846
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("file_len is %d\n",file_len);` -- debug trace in `load_fpg()`.
**Recommendation:** REMOVE

---

### Block 18: Lines 874-876
**Category:** MIKE_ADDED_DEAD
**Summary:** Three lines: an `#ifdef STDOUTLOG` / `printf("fpg found")` / `#endif` block that was commented out. A working version of `STDOUTLOG` logging already exists via active `#ifdef STDOUTLOG` blocks nearby.
**Recommendation:** REMOVE

---

### Block 19: Line 918
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("len: %d num: %d len: %d\n",len_,num_,ftell(es));` -- debug trace in FPG loader.
**Recommendation:** REMOVE

---

### Block 20: Line 923
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("mem ptr is %x\n",iptr);` -- debug trace in FPG loader.
**Recommendation:** REMOVE

---

### Block 21: Line 944
**Category:** MIKE_ADDED_DEAD
**Summary:** `//if(num<=0 || num>1000) break;` -- an abandoned bounds check in FPG loading loop. Mike wrote and commented this out.
**Recommendation:** REMOVE

---

### Block 22: Lines 1149-1151
**Category:** LEGITIMATE
**Summary:** Section header comment for `Get_angle(id)` using the decorative line style (`//~~~~`). Contains a function name but is clearly a section divider, matching the original style.
**Recommendation:** KEEP

---

### Block 23: Lines 1158-1160
**Category:** LEGITIMATE
**Summary:** Section header comment for `Get_dist(id)` using the decorative line style. Same pattern as Block 22.
**Recommendation:** KEEP

---

### Block 24: Line 1890
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fprintf(stdout, "File saved: %s\n", full);` -- debug trace in `save_file()`.
**Recommendation:** REMOVE

---

### Block 25: Lines 1896-1900 (false positive in scan)
**Category:** LEGITIMATE
**Summary:** The automated scanner flagged lines 1896-2152 as a single block, but this is actually the end of `save_file()` (active code starting at `if(lon!=llon)`) followed by the complete `_save()` function. This is NOT commented-out code; the scanner was confused by the `//` comment at `*unit_size` on line 1896. The real commented blocks within this range are covered separately below.
**Recommendation:** KEEP -- This is active code, not a commented block.

---

### Block 26: Lines 2144-2152
**Category:** MIKE_ADDED_DEAD
**Summary:** A `/* ... */` block inside `stop_sound()` containing channel initialization logic (`int InitChannel=16; if(MusicChannels>InitChannel) ...`). Mike wrote this as an alternative to the original DOS stop_sound logic, then abandoned it.
**Recommendation:** REMOVE

---

### Block 27: Lines 2153-2158
**Category:** MIKE_REMOVED
**Summary:** The `if(pila[sp]==-1)` branch that stops all channels, and the `} else {` / `}` wrapping the single-channel stop. In the original F.CPP (lines 1821-1825), `stop_sound()` had logic to stop all channels when `pila[sp]==-1` vs a single channel otherwise. Mike simplified it to always call `StopSound(pila[sp]-1)` without the -1 check. The original also used `InitChannel` as the loop start (to skip music channels), which Mike's version loses.
**Recommendation:** REVIEW -- The "stop all channels" path (`pila[sp]==-1`) is part of the DIV language API. Removing it breaks `stop_sound(-1)` calls in user programs. Consider restoring the conditional.

---

### Block 28: Line 2196
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Requesting song: %s\n",...);` -- debug trace in `load_song()`.
**Recommendation:** REMOVE

---

### Block 29: Line 2217
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Loading Song\n");` -- debug trace in `load_song()`.
**Recommendation:** REMOVE

---

### Block 30: Lines 2303-2306
**Category:** MIKE_ADDED_DEAD
**Summary:** Four lines of Emscripten main-loop manipulation code that Mike wrote and commented out inside `set_fps()`. Guarded by `#ifdef __EMSCRIPTEN__`.
**Recommendation:** REMOVE

---

### Block 31: Lines 2327-2328
**Category:** ORIGINAL_DEAD
**Summary:** Two lines of old `StartFLI()` call with different signature. Already commented out in the original F.CPP (lines 1983-1984). Daniel replaced the direct call with an open-file-then-call pattern.
**Recommendation:** REMOVE

---

### Block 32: Lines 2410-2412
**Category:** LEGITIMATE
**Summary:** Section header comment for `Fget_dist(x0,y0,x1,y1)`. Same decorative-divider pattern as Blocks 22-23.
**Recommendation:** KEEP

---

### Block 33: Lines 2424-2426
**Category:** LEGITIMATE (section header)
**Summary:** Section header comment for `Fget_angle(x0,y0,x1,y1)`. Decorative divider.
**Recommendation:** KEEP -- but the scanner picked this up as code due to the semicolons in the parameter list. It's a section header.

---

### Block 34: Lines 2567-2569
**Category:** MIKE_REMOVED
**Summary:** Inside `fade_off()`, Mike commented out the busy-wait loop `{ set_paleta(); set_dac(); }` that was active in the original F.CPP (line 2235-2236: `while (...) { set_paleta(); set_dac(); //LoopSound(); }`). Mike changed the original blocking fade to a non-blocking approach (just sets `fading=1`).
**Recommendation:** REVIEW -- The original `fade_off()` was a blocking call that waited until the fade completed. Mike made it non-blocking by commenting out the loop body and the `while` (converting it to an `if`). This changes the semantics: DIV programs that call `fade_off()` expect it to block until the screen is black. The `//LoopSound()` was already dead in the original. The `set_paleta(); set_dac();` calls need human review to determine if the non-blocking approach is intentional for SDL2.

---

### Block 35: Line 2572
**Category:** MIKE_REMOVED
**Summary:** `//max_reloj+=get_reloj()-old_reloj;` at end of `fade_off()`. Was active in the original (line 2239: `max_reloj+=reloj-old_reloj;`). Mike removed the timing correction when making fade_off non-blocking.
**Recommendation:** REVIEW -- Coupled with Block 34. If the blocking fade is restored, this timing correction should be restored too (using `get_reloj()` instead of the original `reloj`).

---

### Block 36: Line 2636
**Category:** ORIGINAL_DEAD
**Summary:** `//EndSound();` in `_exit_dos()`. Already commented out in the original F.CPP (line 2307). Sound cleanup was apparently handled elsewhere.
**Recommendation:** REMOVE

---

### Block 37: Lines 2813-2814
**Category:** MIKE_ADDED_DEAD
**Summary:** `//if(joy_status)\n//joy->button1=1;` -- test stub Mike wrote inside SDL joystick section of `read_joy()`, then commented out.
**Recommendation:** REMOVE

---

### Block 38: Lines 3287-3289
**Category:** LEGITIMATE
**Summary:** Section header comment for `fclose(handle)`. Decorative divider, same style as throughout the file.
**Recommendation:** KEEP

---

### Block 39: Line 3644
**Category:** ORIGINAL_DEAD
**Summary:** `//if(status!=_HEAPEND) return -1;` inside `TotalFreeMem()`. Present in the original (implied by the `_heapwalk` code structure). This was a safety check for DOS heap walking.
**Recommendation:** REMOVE -- DOS heap API is not available on SDL2 platforms.

---

### Block 40: Lines 4008-4011
**Category:** ORIGINAL_DEAD
**Summary:** Four-line comment block documenting the expression evaluator (`Evaluador de expresiones`). Identical in the original F.CPP (lines 3606-3609). Uses decorative line borders and describes the `calculate()` function.
**Recommendation:** KEEP -- This is documentation, not dead code. It describes the expression parser below it.

---

### Block 41: Line 4626
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("func: %s\n",fname[mem[ip+1]]);` -- debug trace at the top of the `function()` dispatch switch.
**Recommendation:** REMOVE

---

## Review Items

### 1. stop_sound() "stop all channels" path (Block 27, lines 2153-2158)
The original DOS `stop_sound()` had a branch: when `pila[sp]==-1`, it stopped all sound channels (looping from `InitChannel` to `CHANNELS`). Mike commented this out, leaving only `StopSound(pila[sp]-1)`. This means `stop_sound(-1)` in DIV programs now calls `StopSound(-2)` which is likely an out-of-bounds error. **Decision needed**: restore the -1 check, or confirm that no DIV programs use `stop_sound(-1)`.

### 2. fade_off() blocking behavior (Blocks 34-35, lines 2567-2572)
The original `fade_off()` was a blocking call: it looped `while (color != target) { set_paleta(); set_dac(); }` until the fade completed, then adjusted timing. Mike converted it to non-blocking by commenting out the loop body and timer adjustment, just setting `fading=1`. This changes semantics for any DIV program relying on `fade_off()` completing before the next statement executes. **Decision needed**: is non-blocking fade intentional for SDL2, or should it be restored to blocking behavior? The fading system already handles gradual fade in `frame_end()`, so non-blocking may actually be correct for the SDL2 event-driven model.
