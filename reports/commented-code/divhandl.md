# divhandl.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 19 |
| Total commented-out lines | ~76 |
| Safe to remove | 15 blocks, ~63 lines |
| Needs review | 4 blocks, ~13 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 129
**Category:** ORIGINAL_DEAD
**Summary:** `// do { read_mouse(); } while (mouse_b&1);` in menu_principal2(). Also commented out in the original (line 84, as end-of-line comment). Old busy-wait for mouse release, no longer needed.
**Recommendation:** REMOVE

---

### Block 2: Line 845
**Category:** ORIGINAL_DEAD
**Summary:** `//void Preview_2(void);` -- commented-out forward declaration. Also commented out in the original (line 916).
**Recommendation:** REMOVE

---

### Block 3: Line 876
**Category:** ORIGINAL_DEAD
**Summary:** `//v_titulo=texto[91];` in menu_fuentes2() case 3 (close font). Also commented out in the original (line 947). Was probably a confirmation dialog title that was removed.
**Recommendation:** REMOVE

---

### Block 4: Lines 1094-1100
**Category:** ORIGINAL_DEAD
**Summary:** Six commented-out variable declarations in menu_sonidos2(): `//static int ns,chn;`, `//static bload=0;`, `//byte * p;`, `//FILE * f;`, `//char cwork[256];`, `//int x;`. All also commented out in the original (lines 1165-1171). Old unused variables from earlier sound code.
**Recommendation:** REMOVE

---

### Block 5: Lines 1111, 1120-1121
**Category:** MIKE_REMOVED
**Summary:** Mike replaced `judascfg_device == DEV_NOSOUND` with `false /*...*/` and `judascfg_device != DEV_SBPRO && judascfg_device != DEV_SB16` with `false /*...*/` in the sound recording case. In the original (lines 1182, 1191-1192), these were active if-conditions checking the Judas sound library device type. Mike stubbed them to `false` since Judas was removed, making the error-path code blocks unreachable dead code.
**Recommendation:** REVIEW -- The entire case 1 (record sound) and case 8 blocks contain dead paths. The `false` guards make the error dialogs unreachable. Consider removing the dead branches entirely.

---

### Block 6: Lines 1126-1127, 1129, 1132
**Category:** MIKE_REMOVED
**Summary:** `//judas_stopsample(0);`, `//sbmalloc();`, replacement of `aligned[0]!=NULL && aligned[1]!=NULL` with `true /*...*/`, and `//sbfree();`. In the original (lines 1197-1201), these were active calls to Judas sound library functions. Mike commented them out and replaced conditions with `true` since Judas was removed from the SDL2 port.
**Recommendation:** REVIEW -- These stubs should be cleaned up. The `true /*...*/` guard makes the condition meaningless. `sbmalloc/sbfree` calls should be removed entirely rather than left commented.

---

### Block 7: Line 1180
**Category:** ORIGINAL_DEAD
**Summary:** `//wup(n); repinta_ventana(); wdown(n); vuelca_ventana(n);` in menu_sonidos2() case 5 (save sound as). Also commented out in the original (line 1249). Was replaced by the `OpenSoundFile()` call on the next line.
**Recommendation:** REMOVE

---

### Block 8: Lines 1191, 1204
**Category:** MIKE_REMOVED
**Summary:** `//judas_stopsample(0);` (line 1191) and `false /*judascfg_device == DEV_NOSOUND*/` (line 1204). In the original (lines 1259, 1271), `judas_stopsample(0)` was an active call and `judascfg_device == DEV_NOSOUND` was an active condition. Mike commented/stubbed these during Judas removal.
**Recommendation:** REVIEW -- Same as blocks 5-6. The `judas_stopsample` should just be deleted, and the `false` branch makes the error dialog dead code.

---

### Block 9: Line 1346
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("state: %d\n",v.estado);` -- debug printf added by Mike during SDL2 port development, then commented out. Not present in the original DOS source.
**Recommendation:** REMOVE

---

### Block 10: Line 1353
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("menu text: %s\n",p);` -- debug printf added by Mike, then commented out. Not in the original.
**Recommendation:** REMOVE

---

### Block 11: Line 1358
**Category:** ORIGINAL_DEAD
**Summary:** `//wwrite(ptr,an,al,3,2+abs(v.estado)*9,0,p,c3);` in actualiza_menu(). Also commented out in the original (line 1405). Was replaced by the shortcut-key-aware rendering code below it (the `strchr(p,'[')` block).
**Recommendation:** REMOVE

---

### Block 12: Line 1407
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("menu text: %s\n",p);` -- another debug printf added by Mike, then commented out. Not in the original.
**Recommendation:** REMOVE

---

### Block 13: Lines 1544-1553
**Category:** MIKE_REMOVED
**Summary:** `/* ... */` block in read_mouse3() containing DOS-specific int386 interrupt call to read mouse button state (`union REGS`, `int386(0x33,...)`), plus a `// TODO - ADD SDL MOUSE INPUTS` comment. In the original (lines 1579-1584), this was the active implementation. Mike commented it out during the SDL2 port. The function body is now empty.
**Recommendation:** REMOVE -- The function is called nowhere useful now. Mouse input comes through SDL events in divmouse.c. Consider removing the entire empty function.

---

### Block 14: Lines 2163-2164, 2168-2169
**Category:** MIKE_REMOVED
**Summary:** Four commented-out `#define` lines for `max_archivos`, `an_archivo`, `max_directorios`, `an_directorio`. In the original (lines 2195-2196), these were ACTIVE defines with different values (e.g., `an_archivo (12+1)`). Mike moved them to a header with new values (e.g., `(255)`) and commented out the originals here.
**Recommendation:** REMOVE

---

### Block 15: Line 2297
**Category:** MIKE_ADDED_DEAD
**Summary:** `//printf("zona: %d\n",l->zona);` -- debug printf added by Mike, then commented out. Not in the original.
**Recommendation:** REMOVE

---

### Block 16: Lines 2350-2388
**Category:** ORIGINAL_DEAD
**Summary:** `/* ... */` block containing old `imprime_ruta()` and `dir_abrir()` functions -- the original DOS file browser dialog using `_dos_findfirst/_dos_findnext`. Also commented out in the original (lines 2381-2418). Was replaced by the `browser` system (`dir_abrirbr`, etc.).
**Recommendation:** REMOVE

---

### Block 17: Line 2428
**Category:** MIKE_REMOVED
**Summary:** `//    getcwd(full,PATH_MAX);` in analizar_input(). In the original (line 2456), this was active code: `getcwd(full,PATH_MAX+1);`. Mike commented it out and also changed the condition on the next line from `if (full[0]==drive[0])` to `if ( true || (full[0]==drive[0]))` to bypass the drive-letter check that depended on the getcwd result. This is part of the DOS-to-SDL path handling adaptation.
**Recommendation:** REVIEW -- The `true ||` bypass and commented getcwd should be cleaned up. Either implement cross-platform path logic or simplify the condition.

---

### Block 18: Lines 2566, 2641, 3043
**Category:** ORIGINAL_DEAD (individual single-line comments)
**Summary:** Three scattered single-line commented-out code statements, all also commented out in the original DOS source:
- Line 2566: `//v.item[0].estado&=-3; v.item[1].estado&=-3;` (original line 2575)
- Line 2641: `//int filesize;` (original line 2650)
- Line 3043: `//CACA` (original line 3081 -- Daniel's placeholder/debug marker)
**Recommendation:** REMOVE

---

### Block 19: Lines 2772, 3582
**Category:** Mixed -- see details below
**Summary:** Two additional single-line commented-out statements:
- Line 2772: `//memset(v_mapa->map,0,map_an*map_al+map_an);` -- MIKE_REMOVED. In the original (line 2778), this was active code. Mike commented it out because the buffer is immediately overwritten by decompression. Safe to remove.
- Line 3582: `//wwrite(v.ptr,an,al,x,11+8*20,1,texto[484],c4);` -- MIKE_ADDED_DEAD. The original about1() (line 3619) has no such line; the about dialog ends at texto[483]. Mike added this to show an extra credit line, then commented it out.
**Recommendation:** REMOVE

---

## Review Items

### Blocks 5, 6, 8 (Judas sound library stubs)
These three blocks are all related to Mike's removal of the Judas sound library. The pattern is:
- Active `judascfg_device` comparisons replaced with `false /*...*/`, making error-dialog branches unreachable
- Active `aligned[0]!=NULL` checks replaced with `true /*...*/`, making the else-branch dead
- Active `judas_stopsample(0)` / `sbmalloc()` / `sbfree()` calls commented out

**What the code does:** Guards for sound hardware availability (no sound card, wrong card type), sound sample memory allocation, and sound playback control.

**Recommended cleanup:** Remove all the `false`/`true` shims and dead branches. The sound recording (case 1) and song-open (case 8) paths need real SDL2_mixer equivalents or should show an honest "not implemented" message. The `//judas_stopsample(0)` calls can simply be deleted -- SDL2_mixer handles this differently.

### Block 13 (read_mouse3 -- empty function)
The entire function body is now commented-out DOS code with a TODO note. The function should either be deleted entirely (if unused) or filled with an SDL2 implementation (if still called). Check callers before removing.

### Block 17 (getcwd + true|| bypass)
`//getcwd(full,PATH_MAX);` and `if ( true || ...)` bypass at line 2429. The path logic in analizar_input() needs proper cross-platform handling. The `true ||` makes the drive-check dead code while keeping it syntactically present.
