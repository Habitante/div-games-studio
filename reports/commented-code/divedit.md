# Commented-Out Code Audit: divedit.c

**Source file:** `c:\Src\div\src\divedit.c` (3,364 lines)
**Original DOS file:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVEDIT.CPP` (3,208 lines)
**Date:** 2026-03-07

## Summary

| Category | Count | Action |
|---|---|---|
| ORIGINAL_DEAD | 5 | REMOVE |
| MIKE_REMOVED | 0 | - |
| MIKE_ADDED_DEAD | 0 | - |
| LEGITIMATE | 2 | KEEP |

## Blocks

### Block 1 — Lines 89-116: `/*...*/` tprg struct documentation
```c
  /*

  int an,al;                    // Character width and height of the window
  ...
  int linea_vieja;              // Linea anterior (para el volcado parcial)

  */
```
**Classification:** LEGITIMATE (KEEP)
**Rationale:** This is a documentation comment describing the `tprg` struct layout. Identical in the original (lines 34-61). Serves as a reference for the struct defined elsewhere. Not dead code.

---

### Block 2 — Lines 295-301: `/*...*/` old slider behavior
```c
/*
  if (mouse_graf==13 && !(old_mouse_b&1) &&(mouse_b&1) && wmouse_x!=-1) {
    if (!(v.botones&32)) {
      if (wmouse_y>get_slide_y()) f_avpag(); else f_repag(); v.botones^=32;
    }
  } else if (v.botones&32) v.botones^=32;
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical block exists commented out in the original (lines 240-246). This was an older page-at-a-time slider behavior that Daniel replaced with the current continuous-drag slider. Dead since DOS.

---

### Block 3 — Lines 874-878: `//` commented struct field docs
```c
//  char * buffer;                // Buffer con el fichero cargado
//  int buffer_lon;               // Longitud del buffer
//  int file_lon;                 // Longitud del fichero ( < buffer_lon)
//  char * lptr;                  // Puntero a la linea actual en el fichero
//  char * vptr;                  // Puntero a la primera linea visualizada
```
**Classification:** LEGITIMATE (KEEP)
**Rationale:** Documentation comments above `comprobar_memoria()`, describing the struct fields it operates on. Identical in original (lines 806-810). Serves as inline documentation, not dead code.

---

### Block 4 — Lines 1834-1835: `//` commented-out barra_info code
```c
//  itoa(v.prg->num_lineas,num+1,10); num[0]='('; num[strlen(num)+1]=0; num[strlen(num)]=')';
//  wwrite_in_box(v.ptr,an,an-19,al,3+ancho,10,0,num,c3); ancho+=text_len(num)+1;
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical commented-out code exists in the original (lines 1749-1750). This would have displayed total line count in the status bar. Dead since DOS.

---

### Block 5 — Lines 1976-1977: `//` commented-out actualiza_caja calls (maximizar)
```c
//      actualiza_caja(0,0,vga_an,vga_al);
//      v.volcar=2; vuelca_ventana(0);
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1884-1885). Old full-screen refresh approach replaced by `se_ha_movido_desde()`. Dead since DOS.

---

### Block 6 — Lines 2014-2015: `//` commented-out actualiza_caja calls (des-maximizar)
```c
//      actualiza_caja(0,0,vga_an,vga_al);
//      v.volcar=2; vuelca_ventana(0);
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 1920-1921). Same as Block 5 but in the des-maximize branch. Dead since DOS.

---

### Block 7 — Lines 2267-2275: `/*...*/` old put_char generic loop
```c
/*
    do {
      m=editor_font_an; do {
        if (*si) *ptr=color;
        si++; ptr++;
      } while (--m);
      ptr+=an-editor_font_an;
    } while (--n);
*/
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in original (lines 2163-2171). This was the generic (slow) character rendering loop, replaced by the unrolled switch-case above. Dead since DOS.

---

## Notes

- The `// tecla();` at line 413 was already commented out in the DOS original (line 351). It is a single-line note/annotation, not a code block — classified as not-a-block.
- The `//  strcpy(buscar,"");` at line 2683 and `//strcpy(buscar2/sustituir,"")` at lines 2764-2765 are already commented out in the original (lines 2527, 2608-2609). Single-line annotations — not counted as blocks.
- The `//printf("%c",c);` at line 2211 is a debug print, present in neither the original nor as active code. Single-line debug — not counted.
- The `//    if (iscoment>0) color=ce2; else color=ce4;` at line 2224 is already commented out in the original (line 2120). Single-line — not counted.
- The `//        n-=buffer_grow;` at line 2592 in `programa0_nuevo()` — this function was substantially rewritten by Mike; this single commented line is part of his rewrite. Not counted as a block.
