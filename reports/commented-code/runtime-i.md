# i.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 54 |
| Total commented-out lines | ~83 |
| Safe to remove | 51 blocks, ~76 lines |
| Needs review | 1 block, ~1 line |
| Keep | 2 blocks, ~5 lines |

## Blocks

### Block 1: Line 133
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("first id: %x %d\n",ls, ls->pid);` -- debug trace in Mike's linked-list process finder `find_proc()`. This entire `#ifdef LLPROC` section is Mike's experimental linked-list process system.
**Recommendation:** REMOVE

---

### Block 2: Lines 135-136
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines of an early-return check in `find_proc()`: `//if(ls->lnext==NULL)\n//return NULL;`. Mike wrote and abandoned.
**Recommendation:** REMOVE

---

### Block 3: Line 139
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("%d %d\n",ls->pid,id);` -- debug trace in `find_proc()` loop.
**Recommendation:** REMOVE

---

### Block 4: Line 185
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Looking for next %x %x\n",ff,ff->lnext);` -- debug trace in `dirty()` / linked-list reindexing.
**Recommendation:** REMOVE

---

### Block 5: Line 188
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Z %d is MORE than %d\n",ff->z,mem[did+checklist]);` -- debug trace in linked-list sort.
**Recommendation:** REMOVE

---

### Block 6: Line 190
**Category:** MIKE_ADDED_DEAD
**Summary:** `//break;` -- abandoned control flow in linked-list code.
**Recommendation:** REMOVE

---

### Block 7: Line 197
**Category:** MIKE_ADDED_DEAD
**Summary:** `//break;` -- another abandoned break in linked-list code.
**Recommendation:** REMOVE

---

### Block 8: Line 214
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Re-indexing for (oldid) %d id %d %d %d\n",...);` -- debug trace in linked-list reindexing.
**Recommendation:** REMOVE

---

### Block 9: Line 217
**Category:** MIKE_ADDED_DEAD
**Summary:** `//->lprev;` -- incomplete expression fragment, leftover from linked-list editing.
**Recommendation:** REMOVE

---

### Block 10: Lines 232-233
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines of abandoned linked-list pointer assignments: `//fs->next = pf->pid;\n//fs->lnext = pf;`.
**Recommendation:** REMOVE

---

### Block 11: Line 310
**Category:** MIKE_ADDED_DEAD
**Summary:** `//free(n);` -- abandoned free call in linked-list insert code. In the original, there is no linked-list code at all.
**Recommendation:** REMOVE

---

### Block 12: Line 376
**Category:** ORIGINAL_DEAD
**Summary:** `//  FILE * f=NULL;` in `inicializacion()`. Already commented out identically in the original I.CPP (line 152). Was used for reading `sound.cfg`, which was also commented out.
**Recommendation:** REMOVE

---

### Block 13: Lines 392-403
**Category:** ORIGINAL_DEAD
**Summary:** A 12-line `/* ... */` block containing `sound.cfg` reading code and Juanjo's sound card detection (`DetectBlaster`, `DetectGUS`). Identical to the original I.CPP (lines 165-176). DOS-era sound card auto-detection, already dead in the original.
**Recommendation:** REMOVE

---

### Block 14: Line 430
**Category:** ORIGINAL_DEAD
**Summary:** `//  if((texto=...)malloc(...))` -- old dynamic allocation of the texto array that Daniel commented out. Identical to original I.CPP line 203. The array is now statically allocated.
**Recommendation:** REMOVE

---

### Block 15: Line 478
**Category:** ORIGINAL_DEAD
**Summary:** `//  _bios_timeofday(_TIME_GETCLOCK,(long*)&ip);` -- DOS BIOS time call. Commented out by Mike, replaced with `time(&dtime)` on line 476. The original had this active (line 241).
**Recommendation:** REMOVE -- replaced by `time()` call.

---

### Block 16: Line 481
**Category:** ORIGINAL_DEAD
**Summary:** `//_setvideomode(_MRES256COLOR);` -- DOS video mode call. Already commented out in the original I.CPP (line 242). Was a Watcom C graphics call.
**Recommendation:** REMOVE

---

### Block 17: Lines 503-505
**Category:** MIKE_ADDED_DEAD
**Summary:** Three lines: `#ifdef __EMSCRIPTEN__` / `tabla_ghost()` / `#endif`, all commented out. Mike wrote this Emscripten-specific ghost table initialization, then removed it.
**Recommendation:** REMOVE

---

### Block 18: Lines 648-651
**Category:** ORIGINAL_DEAD
**Summary:** Four-line comment documenting the multi-stack system functions (`guarda_pila`, `carga_pila`, `actualiza_pila`). Identical to original I.CPP lines 440-443. Contains function call syntax but is documentation.
**Recommendation:** KEEP -- This is API documentation for the stack management system, not dead code.

---

### Block 19: Line 653
**Category:** ORIGINAL_DEAD
**Summary:** `// * (int *) mem[id+_SP] = {SP1,SP2,DATOS...}` -- documents the stack memory layout. Identical to original (line 445).
**Recommendation:** KEEP -- Documentation of data layout.

---

### Block 20: Lines 668-669
**Category:** MIKE_ADDED_DEAD
**Summary:** `//if(stacks>=65535)\n//exit("out of memory");` -- abandoned overflow check in Mike's rewritten `guarda_pila()`.
**Recommendation:** REMOVE

---

### Block 21: Line 674
**Category:** MIKE_ADDED_DEAD
**Summary:** `//(memptrsize)p;` -- abandoned cast expression in `guarda_pila()`.
**Recommendation:** REMOVE

---

### Block 22: Line 689
**Category:** MIKE_ADDED_DEAD
**Summary:** `//    free(stack[mem[id+_SP]]);` -- commented-out free in `carga_pila()`. This is a known memory leak (documented in MEMORY.md as a landmine at line 778, but actually at line 689). The `free(p)` on line 693 frees the pointer, but `stack[mem[id+_SP]]` is zeroed without being freed on line 690.
**Recommendation:** REVIEW -- This is the documented memory leak in stack management. The `free()` was commented out by Mike but the stack slot is just zeroed. Need to verify whether `p` (freed on line 693) and `stack[mem[id+_SP]]` point to the same allocation. If they do (which appears to be the case since `p=stack[mem[id+_SP]]` on line 684), then the free on 693 handles it and this commented free is actually a double-free bug that Mike correctly disabled. If so, mark as REMOVE and close the "memory leak" landmine.

---

### Block 23: Line 750
**Category:** MIKE_ADDED_DEAD
**Summary:** `//    running = 0;` inside `#ifdef EMSCRIPTEN` in `mainloop()`. Mike's abandoned Emscripten flag.
**Recommendation:** REMOVE

---

### Block 24: Lines 758-760
**Category:** MIKE_ADDED_DEAD
**Summary:** Three lines: `#ifndef __EMSCRIPTEN__` / `madewith()` / `#endif`, all commented out. Mike disabled the "made with DIV" splash screen call.
**Recommendation:** REMOVE

---

### Block 25: Lines 775-776
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines of Emscripten main loop manipulation: `emscripten_cancel_main_loop()` and `emscripten_set_main_loop()`. Inside the `es_fps()` function. Mike wrote and abandoned.
**Recommendation:** REMOVE

---

### Block 26: Line 1094
**Category:** MIKE_ADDED_DEAD
**Summary:** `//usleep(((int)freloj-old_reloj)-1);` -- abandoned timing code in `frame_start()`. Mike replaced with `sched_yield()` on the line above.
**Recommendation:** REMOVE

---

### Block 27: Lines 1360-1366
**Category:** MIKE_ADDED_DEAD
**Summary:** A 7-line `/* ... */` block containing a debug printf that traces the paint-loop state (`ide`, `m7ide`, `scrollide`, `otheride`). Mike wrote this for debugging the rendering loop, then commented it out.
**Recommendation:** REMOVE

---

### Block 28: Line 1374
**Category:** MIKE_ADDED_DEAD
**Summary:** `// && mem[id+_Z]>max) { ide=id; max=mem[id+_Z]; }` -- fragment of Z-ordering code in the `#else // IFDEFNOTYET` branch. Mike experimented with an alternative rendering order, abandoned it.
**Recommendation:** REMOVE

---

### Block 29: Line 1588
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("text_font = %x\n",text_font);` -- debug trace in `finalizacion()`.
**Recommendation:** REMOVE

---

### Block 30: Line 1592
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("graf_ptr = %x\n",text_font);` -- debug trace in `finalizacion()`. Note: prints `text_font` but claims it's `graf_ptr` (copy-paste error by Mike).
**Recommendation:** REMOVE

---

### Block 31: Lines 1693-1694
**Category:** ORIGINAL_DEAD
**Summary:** Two lines of a commented-out printf showing process count at exit. Identical to original I.CPP lines 1204-1205 (and 1165-1166). Was a developer diagnostic.
**Recommendation:** REMOVE

---

### Block 32: Line 1745
**Category:** ORIGINAL_DEAD
**Summary:** `//  if (end_extern!=NULL) end_extern();` -- callback to external cleanup function. Already commented out in the original I.CPP (line 1245). The DLL system was responsible for cleanup via a different mechanism.
**Recommendation:** REMOVE

---

### Block 33: Lines 1792-1793
**Category:** MIKE_ADDED_DEAD
**Summary:** Two lines: `//freopen("CON", "w", stdout);` and `//freopen("CON", "w", stderr);` -- Win32 console redirection. Mike wrote and commented out.
**Recommendation:** REMOVE

---

### Block 34: Lines 1818-1819
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("searching for magic\n");` and `// search for magic..` -- debug trace in the self-extracting EXE detection code.
**Recommendation:** REMOVE

---

### Block 35: Line 1837
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("failed: %s\n",buf);` -- debug trace when magic-byte detection fails.
**Recommendation:** REMOVE

---

### Block 36: Line 1874
**Category:** MIKE_ADDED_DEAD
**Summary:** `//jschar=emscripten_run_script_string("$('#exename').text()");` -- abandoned Emscripten JS interop code.
**Recommendation:** REMOVE

---

### Block 37: Line 1876
**Category:** MIKE_ADDED_DEAD
**Summary:** `//emscripten_wget (jschar, "exe");` -- abandoned Emscripten file download code.
**Recommendation:** REMOVE

---

### Block 38: Line 1915
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("%x %s\n",f,argv[1]);` -- debug trace in EXE loading.
**Recommendation:** REMOVE

---

### Block 39: Line 1949
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("pack len: %d\n",len);` -- debug trace showing file length.
**Recommendation:** REMOVE

---

### Block 40: Lines 1963-1967
**Category:** MIKE_ADDED_DEAD
**Summary:** Five lines of abandoned DIV version detection and file length code. Mike wrote this while experimenting with DIV1/DIV2 discrimination logic.
**Recommendation:** REMOVE

---

### Block 41: Line 1969
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("div ver: [%d] [%c]\n",DIV_VER,DIV_VER);` -- debug trace.
**Recommendation:** REMOVE

---

### Block 42: Line 1975
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("Cannot load DIV1 exe (yet!)\n");` -- debug trace in DIV1 EXE rejection path.
**Recommendation:** REMOVE

---

### Block 43: Lines 1980-1981
**Category:** MIKE_ADDED_DEAD
**Summary:** `//case -112:` and `//printf("Found Div Windows exe\n");` -- abandoned case label and debug trace for Windows EXE format detection.
**Recommendation:** REMOVE

---

### Block 44: Lines 2006-2007
**Category:** MIKE_ADDED_DEAD
**Summary:** Two debug printf traces inside `#ifdef PRGDUMP` for dumping memory offsets.
**Recommendation:** REMOVE

---

### Block 45: Lines 2030-2031
**Category:** MIKE_ADDED_DEAD
**Summary:** `//rewind(f);` and `//printf("FILE AT: %d\n",ftell(f));` -- abandoned file positioning debug code in DIV1 loader.
**Recommendation:** REMOVE

---

### Block 46: Line 2033
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("FILE AT: %d %d\n",ftell(f),len);` -- debug trace.
**Recommendation:** REMOVE

---

### Block 47: Lines 2039-2044
**Category:** MIKE_ADDED_DEAD
**Summary:** Six lines of abandoned memory dump / copy code in DIV1 loader: a debug loop, `memcpy` to mem, and a comment about it.
**Recommendation:** REMOVE

---

### Block 48: Line 2046
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("ptrsize %d\nmem: %x iloc: ...")` -- debug trace dumping memory layout.
**Recommendation:** REMOVE

---

### Block 49: Line 2050
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("first op: %d\n",mem[mem[1]]);` -- debug trace showing first opcode.
**Recommendation:** REMOVE

---

### Block 50: Lines 2058-2059
**Category:** MIKE_ADDED_DEAD
**Summary:** Two debug printf traces for offset/text dumping in DIV2 loader path.
**Recommendation:** REMOVE

---

### Block 51: Line 2061
**Category:** MIKE_ADDED_DEAD
**Summary:** `//exit(0);` -- abandoned early exit in loader.
**Recommendation:** REMOVE

---

### Block 52: Line 2111
**Category:** MIKE_ADDED_DEAD
**Summary:** `//fwrite((uint32_t *)&mem[9],1,len_descomp,m);` -- abandoned bytecode dump line inside `#ifdef DUMP_BYTECODE`. The active version on line 2112 writes from `mem` instead of `&mem[9]`.
**Recommendation:** REMOVE

---

### Block 53: Line 2209
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("packdir: %x %d\n",packdir,nfiles);` -- debug trace in `is_pak()`.
**Recommendation:** REMOVE

---

### Block 54: Line 2315
**Category:** ORIGINAL_DEAD
**Summary:** `//int xxx_xxx_xxx;` -- a mysterious unused variable declaration. Present identically in the original I.CPP (line 1467). Was likely a placeholder or debug aid by Daniel.
**Recommendation:** REMOVE

---

## Review Items

### 1. Memory leak in carga_pila() (Block 22, line 689)
`//free(stack[mem[id+_SP]]);` was commented out by Mike. Looking at the code flow:
- Line 684: `p=stack[mem[id+_SP]];` -- `p` gets the pointer stored in `stack[]`
- Line 690: `stack[mem[id+_SP]]=0;` -- slot is zeroed
- Line 693: `free(p);` -- the pointer is freed via `p`

Since `p` holds the same pointer that was in `stack[mem[id+_SP]]`, the `free(p)` on line 693 correctly frees the memory. The commented-out `free()` on line 689 would have been a **double-free** if uncommented (freeing the same pointer twice). Mike was correct to comment it out. This should be classified as REMOVE and the "memory leak" landmine in MEMORY.md should be closed -- there is no leak here, Mike fixed it by using `p` as the intermediary.
