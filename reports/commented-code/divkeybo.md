# Commented-Out Code Audit: divkeybo.c

**Source file:** `c:\Src\div\src\divkeybo.c` (226 lines)
**Original DOS file:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVKEYBO.CPP` (196 lines)
**Date:** 2026-03-07

## Summary

| Category | Count | Action |
|---|---|---|
| ORIGINAL_DEAD | 1 | REMOVE |
| MIKE_REMOVED | 0 | - |
| MIKE_ADDED_DEAD | 3 | REMOVE |
| LEGITIMATE | 0 | - |

## Blocks

### Block 1 — Lines 12-18: `/*...*/` DOS IRQ handler typedefs
```c
/*
typedef void (__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;
*/
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** In the original (lines 12-16), these declarations are ACTIVE code (not commented out). Mike wrapped them in `/*...*/` rather than using `#ifdef NOTYET` like the rest of the DOS-specific code. However, these are DOS IRQ types that will never be used in SDL2. Since the surrounding code already uses `#ifdef NOTYET` for the same purpose, this block is redundant dead code — the `#ifdef NOTYET` blocks handle all the DOS IRQ logic.

---

### Block 2 — Lines 57: `//` commented-out kbhit/getch (inside `#ifdef NOTYET`)
```c
//    if (kbhit()) getch();
```
**Classification:** ORIGINAL_DEAD (REMOVE)
**Rationale:** Identical in the original (line 52). Already commented out in DOS. Inside a `#ifdef NOTYET` block so doubly dead.

---

### Block 3 — Lines 136-138: `//` commented-out debug prints in tecla()
```c
//printf("%d %d\n",mclock,reloj);

//mclock++;
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in original. Mike's debug output for clock timing. Dead scaffolding.

---

### Block 4 — Lines 139-204: `#ifdef NOTYET` block containing SDL1 event loop + DOS BIOS calls
This is a large block guarded by `#ifdef NOTYET` (which is never defined). Inside it, there are additional commented-out lines:
- Line 140: `//printf("Reading keyboard\n");`
- Line 167: `//scan_code = event.key.keysym.scancode;`
- Line 179: `//m_b = 1;`
- Line 184: `//m_b = 0;`

**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** The entire `#ifdef NOTYET` block is dead (NOTYET is never defined). It contains Mike's first attempt at an SDL1 keyboard handler mixed with DOS BIOS calls (int386x, FP_SEG, etc.). The real keyboard handling moved to `read_mouse2()` in divmouse.c. This entire block should be removed. In the original, `tecla()` is a straightforward function (lines 164-178) that calls `tecla_bios()` and reads the keyboard buffer — completely different from this dead SDL1 hybrid.

---

## Notes

- Lines 31-34 have `#ifdef NOTYET` around the DOS BIOS keyboard buffer addresses (`kb_start`, `kb_end`). These are not commented-out code per se, but conditionally compiled dead code (NOTYET is never defined). Same pattern as the `IrqHandler`, `GetIRQVector`, `SetIRQVector`, `kbdInit`, `kbdReset`, and `vacia_buffer` functions which all have `#ifdef NOTYET` guards around their DOS implementations.
- Line 131: `//extern float m_x=0.0,m_y;` — single-line commented-out extern, Mike's debug note. Not counted as a block.
- Line 213: `//printf("vacia_buffer\n");` — single debug print inside `vacia_buffer()`. Mike-added.
- The whole file is a DOS-to-SDL2 adaptation. The active code (kbdInit, kbdReset, tecla_bios, tecla, vacia_buffer) has been gutted to empty/minimal implementations. The DOS code remains under `#ifdef NOTYET` as reference but should eventually be removed entirely.
