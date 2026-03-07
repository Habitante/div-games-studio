# Commented-Out Code Audit: divmouse.c

**Source file:** `c:\Src\div\src\divmouse.c` (661 lines)
**Original DOS file:** `F:\PrevBackup\Old Backups\Backups\Back01\DIV\DIVMOUSE.CPP` (208 lines)
**Date:** 2026-03-07

## Summary

| Category | Count | Action |
|---|---|---|
| ORIGINAL_DEAD | 0 | - |
| MIKE_REMOVED | 2 | REVIEW |
| MIKE_ADDED_DEAD | 5 | REMOVE |
| LEGITIMATE | 0 | - |

## Blocks

### Block 1 — Lines 76-83: `/*...*/` spacebar-as-mouse-button (read_mouse, shift branch)
```c
/*			if (key(_SPC)) {
				if (mouse_b!=0xfffd) {
					mouse_b=0xfffd;
				}
			} else if (mouse_b==0xfffd) {
				mouse_b=0;
			}
			* */
```
**Classification:** MIKE_REMOVED (REVIEW)
**Rationale:** In the original (lines 59-63), this spacebar-as-mouse-click code is ACTIVE. Mike commented it out during the SDL2 port. The code maps Spacebar to a pseudo mouse button (0xfffd) when the mouse has physically moved. This was used in the paint editor for drawing with keyboard. Note: a duplicate of this logic also exists at Block 2.

---

### Block 2 — Lines 139-153: Two `/*...*/` blocks — spacebar experiments (keyboard movement branch)
```c
/*
			if ( key(_SPC)) {
				m_b |= 16;
			} else if ( m_b&1 ){
				m_b &= ~16;;
			}
			* */
/*			if (key(_SPC)) {
				if (mouse_b!=0xfffd) {
					mouse_b=0xfffd;
				}
			} else if (mouse_b==0xfffd) {
				mouse_b=0;
			}
*/
```
**Classification:** MIKE_REMOVED (REVIEW) — the second sub-block; MIKE_ADDED_DEAD (REMOVE) — the first sub-block.
**Rationale:** The second sub-block (lines 146-153) is identical to the original (lines 95-99), which was ACTIVE. Mike commented it out. The first sub-block (lines 139-145) uses `m_b |= 16` which does not exist in the original — Mike wrote it as an alternative approach, then commented it out.

**What this code does:** The original spacebar logic (`mouse_b=0xfffd`) allowed keyboard cursor movement + spacebar to simulate mouse clicks in the paint editor. Without this, keyboard-only painting doesn't work. Should be reviewed for restoration.

---

### Block 3 — Lines 87-88: `//` commented-out mouse scaling (SDL2 block)
```c
		// mouse_x *= (vga_an / vwidth);
		// mouse_y *= (vga_al / vheight);
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in the original. Mike wrote this integer-division version of mouse scaling, then replaced it with the float version on lines 89-90 which is active. Dead experimental code.

---

### Block 4 — Lines 328-329: `//` commented-out vwidth/vheight assignments
```c
		    //vwidth = event.window.data1;
            //vheight = event.window.data2;
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in the original (entire PrintEvent function is Mike's). These were replaced by `vwidth = vga_an; vheight = vga_al;` on lines 324-325. Dead experimental code.

---

### Block 5 — Lines 472-473: `//` commented-out relative mouse movement
```c
//				m_x+=event.motion.xrel;
//				m_y+=event.motion.yrel;
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in original. Mike's earlier approach to mouse motion (relative deltas), replaced by the conditional absolute/relative logic on lines 464-470.

---

### Block 6 — Lines 500-508: `/*...*/` mouse button struct assignments
```c
/*				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse->right = 1;
				}
				if(event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouse->middle = 1;
				}
				* */
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in the original. References a `mouse->` struct that doesn't exist in DIV. Mike's early experiment with a different mouse abstraction. Dead scaffolding.

---

### Block 7 — Lines 580-582: `//` commented-out quit-on-left-click
```c
//                if (event.button.button == SDL_BUTTON_LEFT)
                    /* Quit the application */
  //                  quit = 1;
```
**Classification:** MIKE_ADDED_DEAD (REMOVE)
**Rationale:** Not in original. Early SDL scaffolding that would quit on left click. Dead.

---

## Review Items

### REVIEW: Spacebar-as-mouse-button (Blocks 1 and 2, second sub-block)
The original DOS code used `mouse_b=0xfffd` to simulate a mouse click via spacebar. This enabled keyboard-only cursor movement + spacebar clicking in the paint editor. Mike commented this out, likely because it interacted badly with SDL2's key handling. This functionality should be reviewed for restoration — it's needed for keyboard-driven paint editing.

## Notes

- Lines 291, 390, 397-398, 509, 510, 542, 552, 646, 656 contain single-line `//` debug prints (`fprintf`, `printf`, `SDL_putenv`) — all Mike-added debugging. Not counted as code blocks but should be cleaned up.
- Line 590: `//SDL_SetVideoMode(...)` and line 591: `//bW = buffer->w; bH = buffer->h;` — Mike's SDL1 remnants, single-line each.
