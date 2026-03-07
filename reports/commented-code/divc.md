# divc.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 17 |
| Total commented-out lines | ~40 |
| Safe to remove | 14 blocks, ~32 lines |
| Needs review | 0 blocks |
| Keep | 3 blocks, ~8 lines |

**Note on lines 1-200:** The first 200 lines are developer design notes, changelog, and
technical discussion about pointer/offset strategies. These are all LEGITIMATE documentation
comments (historical architecture notes by Daniel Navarro). They contain semicolons and braces
in the context of describing language grammar, not dead code. Not counted in this audit.

**Note on BNF grammar comments:** Lines 3194, 3286, 3347-3354, 3710-3714 contain BNF-style
grammar descriptions of the parser rules (e.g., `// <variable> := p_id [ p_asig <exp> ] {;}`).
These are LEGITIMATE documentation embedded in the parser, matching the original source.

## Blocks

### Block 1: Line 212
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//#define listados // Para generar los listados de objetos y EML (LST/TAB/EML)`
**Summary:** Disabled `#define` for listing generator (LST/TAB/EML output). Already commented out in original DIVC.CPP:212. The listing functions (`l_objetos`, `l_ensamblador`) are only compiled under `#ifdef listados`.
**Recommendation:** REMOVE

---

### Block 2: Line 220
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//extern char ExeGen[_MAX_PATH];`
**Summary:** Dead extern declaration for a custom executable output path. Already commented out in original DIVC.CPP:218. Related to block 5.
**Recommendation:** REMOVE

---

### Block 3: Lines 1036-1039
**Category:** MIKE_ADDED_DEAD
**Lines:** 4
```c
/*
  for (n=0;n<256;n++)
	printf("vhash[%d] = %x\n",n,vhash[n]);
*/
```
**Summary:** Debug printf dumping the hash table. NOT in original DIVC.CPP (original has no code between `lex_case[cr]=...` and `inicio_objetos=...`). Mike added this during SDL2 port debugging, then commented it out.
**Recommendation:** REMOVE

---

### Block 4: Lines 1130-1131
**Category:** ORIGINAL_DEAD
**Lines:** 2
**Code:** Commented-out `fwrite(mem,4,imem,f); fwrite(loc,4,iloc,f);` and `fclose(f);` inside the `setup.ovl` writer.
**Summary:** Original uncompressed write path, replaced by zlib-compressed output. Already commented out in original DIVC.CPP:1047-1048.
**Recommendation:** REMOVE

---

### Block 5: Line 1163
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//if ((f=fopen(ExeGen,"wb"))!=NULL) {`
**Summary:** Alternative file-open using the ExeGen path variable. Already commented out in original DIVC.CPP:1075. Related to block 2.
**Recommendation:** REMOVE

---

### Block 6: Lines 1170-1171
**Category:** ORIGINAL_DEAD
**Lines:** 2
**Code:** Same pattern as block 4 -- commented-out `fwrite`/`fclose` inside the `EXEC.EXE` writer.
**Summary:** Original uncompressed write path, replaced by zlib-compressed output. Already commented out in original DIVC.CPP:1082-1083.
**Recommendation:** REMOVE

---

### Block 7: Line 1200
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//fwrite(mem,4,imem,f); fwrite(loc,4,iloc,f);`
**Summary:** Same dead fwrite in the error-handling else branch. Already commented out in original DIVC.CPP:1107.
**Recommendation:** REMOVE

---

### Block 8: Lines 1868-1869
**Category:** MIKE_ADDED_DEAD
**Lines:** 2
```c
//      fprintf(stdout,"FILE: %s\n",(char *)ivnom.b);
//	      fprintf(stdout,"LOOKING FOR FILE: %s [%s] [%s]\n",(char *)ivnom.b,full,(char *)&tipo[8]);
```
**Summary:** Debug fprintf for file inclusion. NOT in original DIVC.CPP. Mike added these during SDL2 port to debug file path resolution, then commented them out. (Note: line 1872 has an ACTIVE `fprintf(stdout,"FOUND FILE: ..."` that was also added by Mike and left active -- that's a separate issue, not part of this audit.)
**Recommendation:** REMOVE

---

### Block 9: Line 2080
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `// (*ob).psgl.len_item <- (*((*ob).psgl.ostruct)).len_item;`
**Summary:** A commented-out assignment for struct pointer len_item initialization. Already commented out in original DIVC.CPP:1948. Appears to be an alternative approach Daniel considered but didn't use.
**Recommendation:** REMOVE

---

### Block 10: Lines 4682, 4702, 4728, 4764, 4807, 4836 (6 scattered single lines)
**Category:** ORIGINAL_DEAD
**Lines:** 6
**Code:** Six `//test_buffer(&mem,&imem_max,imem);` calls inside `tglo_init2()`.
**Summary:** Buffer overflow checks that were intentionally disabled because `tglo_init` cannot call `test_buffer` (as noted by the comment at line 4631/original:4631: `// OJO!!! tglo_init no puede llamar a test_buffer !!!`). All six are already commented out in original DIVC.CPP:4654,4674,4700,4736,4779,4808. Note: line 4836 was modified by Mike to use `(memptrsize)` casts instead of `(int)`, but it's still dead code.
**Recommendation:** REMOVE

---

### Block 11: Lines 5470-5471
**Category:** ORIGINAL_DEAD
**Lines:** 2
```c
//    case erango:
//      if (pila[i]<0||pila[i]>(*e).valor) c_error(4,31); break;
```
**Summary:** Disabled range-check case in constant expression evaluator. Already commented out in original DIVC.CPP:5442-5443. Preceded by an explanatory comment (line 5490-5491) about why it was removed (conflict with FROM expressions).
**Recommendation:** REMOVE

---

### Block 12: Line 5493
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//      case p_pointer: pila[i]=mem[pila[i]]; break;`
**Summary:** Disabled pointer dereference in constant expression evaluator. Already commented out in original DIVC.CPP:5465. Same rationale as block 11.
**Recommendation:** REMOVE

---

### Block 13: Line 7313
**Category:** MIKE_ADDED_DEAD
**Lines:** 1
**Code:** `//fprintf(stdout,"Trying to open %s\n",file);`
**Summary:** Debug fprintf in `div_open_file()`. NOT in original DIVC.CPP. Mike added this during SDL2 port to debug file opening, then commented it out.
**Recommendation:** REMOVE

---

### Block 14: Lines 7445-7453
**Category:** ORIGINAL_DEAD
**Lines:** 7 (with gaps for active code on lines 7447-7448, 7451, 7454)
```c
      //      strcpy(e_msg,texto[207]);
      //      itoa(numero_error,e_msg+strlen(e_msg),10);
//      if (numero_error>=10) {
        get_error(500+numero_error);    // <-- active replacement
      //        strcat(e_msg,texto[208]);
      //        itoa(linea_error,e_msg+strlen(e_msg),10);
//      }
      //      strcat(e_msg,".");
      //      mensaje_compilacion(e_msg);
      mensaje_compilacion(cerror);      // <-- active replacement
```
**Summary:** Old error message construction using `e_msg` string concatenation, replaced by `get_error()`/`cerror` system. Already commented out in original DIVC.CPP:7297-7305 (identical pattern). The replacement `get_error()` call is active in both files.
**Recommendation:** REMOVE

---

### Block 15: Lines 7490-7491
**Category:** KEEP (LEGITIMATE)
**Lines:** 2
**Code:**
```c
// void g1(int op) { mem[imem++]=op; }
// void g2(int op, int pa) { mem[imem++]=op; mem[imem++]=pa; }
```
**Summary:** These show the unoptimized versions of `g1`/`g2` as documentation, immediately above the actual implementations that add peephole optimization dispatch. Present in original DIVC.CPP:7342-7343. They serve as a useful reference showing what the functions would do without optimization.
**Recommendation:** KEEP -- useful documentation of the non-optimized code path

---

### Block 16: Lines 7499-7503
**Category:** KEEP (LEGITIMATE)
**Lines:** 5
```c
//struct {      // Peephole, "mirilla" de optimizacion
//  int dir;    // Direccion
//  int param;  // Indica el numero de parametros de la instruccion
//  int op;     // Opcode
//} code[16];   // En code[15] debe quedar siempre la ultima instruccion generada
```
**Summary:** Documents the `code[16]` peephole struct layout. The actual struct is defined elsewhere (likely global.h), and this comment shows the shape of the data the optimizer uses. Present in original DIVC.CPP:7351-7355.
**Recommendation:** KEEP -- documents the peephole optimizer's data structure in context

---

### Block 17: Line 7516
**Category:** ORIGINAL_DEAD
**Lines:** 1
**Code:** `//  int n;`
**Summary:** Unused local variable in `gen()`. Already commented out in original DIVC.CPP:7368.
**Recommendation:** REMOVE

---

## Additional Observation

**Active debug fprintf at line 1872:** While not a commented-out block, line 1872 contains an
active `fprintf(stdout,"FOUND FILE: [%s] [%s] [%s]\n",...)` that Mike added during the SDL2 port.
This is a leftover debug print that should be removed or wrapped in `#ifdef DEBUG`. It was NOT
in the original DIVC.CPP.

## Summary

This file is remarkably clean for its size (7,836 lines). Only ~40 lines of commented-out code
across 17 blocks, and 14 of those blocks (32 lines) are safe to remove outright:

- **11 blocks are ORIGINAL_DEAD** -- already commented out by Daniel in the DOS-era source,
  mostly related to the transition from uncompressed to zlib-compressed executables, disabled
  buffer checks in `tglo_init`, and superseded error message formatting.
- **3 blocks are MIKE_ADDED_DEAD** -- debug fprintf/printf statements Mike added during the
  SDL2 port and then commented out (total: 7 lines).
- **3 blocks are KEEP** -- legitimate documentation showing unoptimized code paths and data
  structure layouts.
- **0 blocks need REVIEW** -- no cases where Mike commented out active original code.
