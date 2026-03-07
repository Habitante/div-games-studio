# divinsta.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 10 |
| Total commented-out lines | ~67 |
| Safe to remove | 7 blocks, ~56 lines |
| Needs review | 3 blocks, ~11 lines |
| Keep | 0 blocks |

## Blocks

### Block 1: Line 329
**Category:** MIKE_REMOVED
**Summary:** `Unid[0]=toupper(tipo[1].path[0]);` -- sets the temp drive letter from the project path. Was active in DOS original (line 310). Mike commented it out and replaced line 328 with `strcpy(Unid,(char *)&tipo[8]);` which is a different (possibly wrong) approach to temp directory assignment.
**Recommendation:** REVIEW

---

### Block 2: Lines 440-443
**Category:** MIKE_ADDED_DEAD
**Summary:** A `while(fl++<1000)` loop zeroing lst[] entries, written by Mike as an alternative to memset. He then wrote the memset on line 438 and commented this out. Not in DOS original.
**Recommendation:** REMOVE

---

### Block 3: Line 445
**Category:** MIKE_ADDED_DEAD
**Summary:** `memset(lst,0,1000*sizeof(memptrsize));` -- Mike's earlier attempt using memptrsize type, replaced by `memset(lst,0,1000*sizeof(int))` on line 438. Not in DOS original.
**Recommendation:** REMOVE

---

### Block 4: Lines 454, 477, 768
**Category:** MIKE_ADDED_DEAD
**Summary:** Debug printf statements (`printf("lst[3] %x\n",...)`; `printf("next bit commented\n")`; `printf("cwork %s\n",cWork)`) added by Mike for debugging. Not in DOS original.
**Recommendation:** REMOVE

---

### Block 5: Lines 478-525
**Category:** MIKE_REMOVED
**Summary:** The entire install thumbnail creation code block (creating xlat table, downscaling 640x480 image, antialiasing via ghost table). Was active in DOS original (lines 446-491 in DIVINSTA.CPP). Mike wrapped it in `/* */` with a debug printf above. This is the core of `crear_imagen_install()` and its removal means install thumbnails never render.
**Recommendation:** REVIEW -- this is functional code that should be restored or the function should be marked as broken.

---

### Block 6: Line 734
**Category:** MIKE_REMOVED
**Summary:** `create_install_image(ifile1,0);` -- call to pre-create the install image when dialog opens. Was active in DOS original as `crear_imagen_install(ifile1,0)` (line 640). Mike commented it out, likely because the thumbnail code (Block 5) was also disabled.
**Recommendation:** REVIEW -- should be restored alongside Block 5.

---

### Block 7: Lines 763-765, 775, 777
**Category:** MIKE_REMOVED
**Summary:** DOS path construction code (`strcpy(cWork,drive); strcat(cWork,dir); cWork[x+2]=0;`) and destination path setup (`strcpy(full,drive); strcat(full,dir)`) and `_dos_setdrive(my_drive,&_drive)`. All were active in DOS original (lines 668-676). Mike replaced them with a simpler `strcpy(cWork,full)` on line 766. The entire installer is DOS-specific and non-functional on SDL2 anyway; these commented lines are part of that dead feature.
**Recommendation:** REMOVE (entire installer module is DOS-only dead code)

---

### Block 8: Line 835
**Category:** MIKE_REMOVED
**Summary:** `strupr(MiHeaderSetup[x].name);` -- uppercasing setup filenames. Was active in DOS original (line 723). Mike commented it out, likely because SDL2 port uses case-sensitive filesystems.
**Recommendation:** REMOVE (installer is non-functional anyway; strupr behavior is platform-dependent)

---

### Block 9: Line 865
**Category:** MIKE_REMOVED
**Summary:** `strupr(__ins);` -- uppercasing filenames in the install list. Was active in DOS original (line 753). Same rationale as Block 8.
**Recommendation:** REMOVE (same as Block 8)

---

### Block 10: Lines 1098-1099
**Category:** ORIGINAL_DEAD
**Summary:** `if (pentium) fin=fopen("install/div32run.ins","rb"); else fin=fopen("install/div32run.386","rb");` -- already commented out in DOS original (lines 975-976). Was replaced by the unconditional DLL open on the line above.
**Recommendation:** REMOVE

---

## Review Items

1. **Block 1 (line 329):** Mike replaced the temp directory drive letter logic with `strcpy(Unid,(char *)&tipo[8])` which casts a struct member to char*. The original `Unid[0]=toupper(tipo[1].path[0])` correctly set the drive letter. This may be a bug in Mike's port, but the entire installer is non-functional on SDL2, so it is low priority.

2. **Block 5 (lines 478-525):** The entire thumbnail rendering body of `crear_imagen_install()` is commented out. If the installer feature is ever restored, this code must be uncommented. For now it is dead alongside the rest of the installer.

3. **Block 6 (line 734):** Dependent on Block 5 -- the call site for the disabled thumbnail code.
