# divmixer.c - Commented Code Audit

| Metric | Value |
|--------|-------|
| Total blocks | 8 |
| Total commented-out lines | ~52 |
| Safe to remove | 5 blocks, ~28 lines |
| Needs review | 2 blocks, ~21 lines |
| Keep | 1 block |

## Blocks

### Block 1: Lines 36-44
**Category:** ORIGINAL_DEAD
**Summary:** `/* OJO !!! ... */` block inside `SetMasterVolume()` switch statement. GUSMAX case for GUS MAX sound card volume control via I/O ports. Already commented out in the original DOS source (lines 35-44 in original) with the same "OJO !!!" annotation.
**Recommendation:** REMOVE

---

### Block 2: Lines 77-86
**Category:** ORIGINAL_DEAD
**Summary:** `/* OJO !!! ... */` block inside `SetVocVolume()` switch statement. GUSMAX case for GUS MAX voice volume. Already commented out in original (lines 74-84).
**Recommendation:** REMOVE

---

### Block 3: Lines 119-129
**Category:** ORIGINAL_DEAD
**Summary:** `/* OJO !!! ... */` block inside `SetCDVolume()` switch statement. GUSMAX case for GUS MAX CD volume. Already commented out in original (lines 113-123).
**Recommendation:** REMOVE

---

### Block 4: Lines 170-178
**Category:** ORIGINAL_DEAD
**Summary:** Inline assembly (`asm mov dx,...`) version of `MIX_SetInput()` in a block comment. The C equivalent (`outp()` calls) is above it. Already commented out in original (lines 152-160).
**Recommendation:** REMOVE

---

### Block 5: Lines 191-207
**Category:** ORIGINAL_DEAD
**Summary:** Inline assembly version of `MIX_GetVolume()`. The C equivalent is above it. Already commented out in original (lines 171-187).
**Recommendation:** REMOVE

---

### Block 6: Lines 220-231
**Category:** ORIGINAL_DEAD
**Summary:** Inline assembly version of `MIX_SetVolume()`. The C equivalent is above it. Already commented out in original (lines 198-209).
**Recommendation:** REMOVE -- Note: all three of these asm blocks (4, 5, 6) were the original DOS inline assembly that Daniel replaced with C `outp()`/`inp()` equivalents. The asm was kept as reference.

---

### Block 7: Lines 238, 252
**Category:** MIKE_REMOVED
**Summary:** `if(judascfg_device==DEV_NOSOUND) return;` guards in `set_mixer()` and `set_init_mixer()`. In the original these were active (lines 215, 229). Mike commented them out because `judascfg_device` is not used in the SDL2 port.
**Recommendation:** REVIEW -- these are early-return guards. In the SDL2 port, `InitMixer()` now calls `Mix_VolumeMusic` and `Mix_Volume` directly (lines 138-139), so these guards are no longer needed. Safe to remove.

---

### Block 8: Lines 254-266
**Category:** MIKE_REMOVED
**Summary:** In `set_init_mixer()`, the entire SB Pro/SB16 hardware mixer query block (`MIX_GetVolume` calls to read hardware mixer levels and configure accordingly) and the `if(judascfg_device == DEV_SBPRO || ...)` conditional. In the original (lines 231-247) this was active code. Mike commented it out, leaving only the `else` branch which uses saved Setupfile values.
**Recommendation:** REVIEW -- the hardware mixer query path is DOS-only and will never work on SDL2. The remaining `else` branch (reading from Setupfile) is correct for SDL2. The commented block should be removed, along with the now-unused local variables `fx_l`, `fx_r`, `cd_l`, `cd_r`, `ma_l`, `ma_r` declared at lines 248-250.

---

## Review Items

### Block 7 (Lines 238, 252): `judascfg_device==DEV_NOSOUND` guards
Safe to remove. The SDL2 port does not use `judascfg_device` for anything meaningful in these mixer functions.

### Block 8 (Lines 254-266): Hardware mixer query in `set_init_mixer()`
The DOS hardware mixer query path is dead code in the SDL2 port. Remove the commented block and the 6 unused local variables (`fx_l`, `fx_r`, `cd_l`, `cd_r`, `ma_l`, `ma_r`).

**Architecture note:** The entire file operates under `#ifdef NOTYET` guards for all the hardware I/O port functions (`SetMasterVolume`, `SetVocVolume`, `SetCDVolume`, `InitMixer`, `MIX_Reset`, `MIX_SetInput`, `MIX_GetVolume`, `MIX_SetVolume`). These function bodies are effectively dead. Only `InitMixer()` has live SDL2 code (lines 136-139: `Mix_VolumeMusic` and `Mix_Volume`). The commented-out assembly blocks inside these already-dead functions are doubly dead but harmless to leave until the NOTYET blocks themselves are cleaned up.
