# Commented-Out Code Audit: divbrow.c

**Source file:** `c:\Src\div\src\divbrow.c` (2,096 lines)
**Original DOS file:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVBROW.CPP` (2,017 lines)
**Date:** 2026-03-07

## Summary

| Category | Count | Action |
|---|---|---|
| ORIGINAL_DEAD | 6 | REMOVE |
| MIKE_REMOVED | 1 | REVIEW |
| MIKE_ADDED_DEAD | 2 | REMOVE |
| LEGITIMATE | 1 | KEEP |

## Blocks

### Block 1 — Lines 49-63: `/*...*/` struct t_listboxbr documentation
```c
/*struct t_listboxbr{
  int x,y;              // Posicion del listbox en la ventana
  ...
  int creada;           // Indica si ya esta creada la lista en pantalla
};*/
```
**Classification:** LEGITIMATE (KEEP)
**Rationale:** Identical in original (lines 28-42). Documentation comment showing the layout of a struct defined elsewhere. Not dead code — serves as inline reference.

---

### Block 2 — Line 597: `//return;`
```c
//return;
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in the original. Mike's debug early-return in `crear_un_thumb_FNT()`, left commented out. Dead scaffolding.

---

### Block 3 — Line 872: `//  fseek(fifs,16,SEEK_SET);`
```c
//  fseek(fifs,16,SEEK_SET);
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (line 831). Already commented out in DOS — was replaced by `fseek(fifs,4,SEEK_SET)` on the next line.

---

### Block 4 — Line 1024: `//    printf("thumb[num].filesize:%d, mem:%d\n", ...);`
```c
//    printf("thumb[num].filesize:%d, mem:%d\n", thumb[num].filesize, mem);
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (line 977). Debug print already commented out in DOS.

---

### Block 5 — Line 1423: `//if (v_thumb==7) v_thumb=0; // OJO !!! Prueba para s3m`
```c
//if (v_thumb==7) v_thumb=0; // OJO !!! Prueba para s3m
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (line 1371). Debug/test code already commented out in DOS.

---

### Block 6 — Line 1521: `false /*judascfg_device == DEV_NOSOUND*/`
```c
  if(v_thumb==7 && false /*judascfg_device == DEV_NOSOUND*/) opc_pru=0;
```
**Classification:** MIKE_REMOVED (REVIEW)
**Rationale:** In the original (line 1469), this reads `if(v_thumb==7 && judascfg_device == DEV_NOSOUND) opc_pru=0;` — active code. Mike replaced the Judas sound check with `false` and commented out the original condition. The purpose was to disable the sound preview button when no sound device is available. With SDL2_mixer, this should be replaced with the equivalent SDL2 check (e.g., checking if Mix_OpenAudio succeeded).

---

### Block 7 — Line 1566: `//FILE *f;`
```c
//FILE *f;
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (line 1504). Unused variable declaration already commented out in DOS.

---

### Block 8 — Lines 1628-1634: `/*...*/` S3M test code
```c
/*
      if(v_thumb==7) // OJO !!! Prueba para s3m
      {
        fin_dialogo=1;
        break;
      }
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1566-1572). Debug/test code that would force-close the browser for PCM type. Already commented out in DOS.

---

### Block 9 — Lines 1746-1749, 1753: `//` commented-out Judas sound calls
```c
//              smp=SI->sample;
...
//              judas_playsample(smp, 0, SI->SoundFreq, 64*256, MIDDLE);
//              free(SI);
...
//        strcpy(full,archivo+(larchivosbr.zona-10+larchivosbr.inicial)*an_archivo);
```
**Classification:** MIKE_ADDED_DEAD (REMOVE) — for lines 1746, 1748-1749 (Mike commented out active DOS code and replaced with SDL2_mixer calls).
**Note:** Line 1753 is ORIGINAL_DEAD — identical in original (line 1679), already commented out in DOS.
**Rationale:** In the original (lines 1673-1675), the Judas sound playback code is ACTIVE. Mike commented it out and replaced it with `Mix_PlayChannel(0,smp,0)` on line 1747. Since the replacement is already in place, the old commented-out Judas calls are dead. Line 1753 (`strcpy(full,...)`) was already commented out in DOS.

---

### Block 10 — Lines 1845-1848: `/*...*/` StopSound/UnloadSound
```c
/*
    StopSound(chn);
    UnloadSound(ns);
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1768-1771). Already commented out in DOS. These are stubs for a sound system cleanup that was never fully implemented.

---

### Block 11 — Lines 1861-1864: `/*...*/` StopSong/UnloadSong
```c
/*
    StopSong();
    UnloadSong();
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1784-1787). Already commented out in DOS. Same as Block 10 — stubs for song cleanup.

---

### Block 12 — Lines 1909-1914: `/*...*/` default extension selection
```c
/*
  if (tipo[v_tipo].defecto>=0 && tipo[v_tipo].defecto<lextbr.maximo)
    strcpy(input,&ext[an_ext*tipo[v_tipo].defecto]);
  else strcpy(input,&ext[0]);
  strcpy(mascara,input);
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1830-1835). Already commented out in DOS. Old extension-selection logic in `leer_directorio()`.

---

## Review Items

### REVIEW: Sound device check (Block 6)
The original `judascfg_device == DEV_NOSOUND` check disabled the sound preview button when no sound hardware was available. Mike replaced it with `false`, effectively always enabling the preview button. This should be replaced with the SDL2_mixer equivalent — e.g., checking a global flag set when `Mix_OpenAudio()` succeeds or fails.

## Notes

- Lines 1191, 1209, 1268 contain commented-out `//printf(...)` debug prints — all Mike-added (not in DOS original). Single-line debug scaffolding.
- Lines 1180, 1199 contain `//Paso 1 de 16 a 8` and `//Paso 2 de Estereo a mono` — these are section comment headers, not code. LEGITIMATE.
- The `#ifdef NOTYET` block at lines 1868-1873 contains active-in-original Judas sound cleanup code (judas_stopsample, judas_freesample). This is conditionally compiled out, not commented out, so not counted here but should be addressed when cleaning up the Judas→SDL2_mixer migration.
