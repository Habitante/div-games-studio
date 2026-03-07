# divforma.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 3 |
| Total commented-out lines | ~4 |
| Safe to remove | 3 blocks, ~4 lines |
| Needs review | 0 blocks |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 236
**Category:** ORIGINAL_DEAD
**Summary:** `//unsigned int fExit=0;` -- unused variable declaration in `descomprime_PCX()`. Already commented out in DOS original (line 216 of DIVFORMA.CPP).
**Recommendation:** REMOVE

---

### Block 2: Line 625
**Category:** ORIGINAL_DEAD
**Summary:** `//if(InfoHeader.biCompression) if(InfoHeader.biCompression!=BI_RLE8) return(0);` -- an early-reject for non-RLE8 BMP compression. Already commented out in DOS original (line 605). The code below handles both RLE8 and RLE4 compression properly, making this redundant guard unnecessary.
**Recommendation:** REMOVE

---

### Block 3: Lines 1159, 1184
**Category:** ORIGINAL_DEAD
**Summary:** `//  fseek(file,48,SEEK_SET);` in both `cargadac_FNT()` and `cargadac_FPG()`. Already commented out in DOS original (lines 1117 and 1142). The fseek to byte 48 was unnecessary because after reading the 8-byte header, the file pointer is already at offset 8 where the 768-byte palette begins (the FNT/FPG header is exactly 8 bytes before the palette).
**Recommendation:** REMOVE

---

## Review Items

None. All commented-out code in this file was already dead in the DOS original.
