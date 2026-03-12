# DIV Games Studio - VM and Runtime

This document describes the DIV runtime system: the bytecode interpreter,
process scheduling, rendering pipeline, and runtime services.

---

## 1. VM Architecture

The DIV VM is a **stack-based** virtual machine with a single flat memory space
(`mem[]`) shared by all processes. Key properties:

- **32-bit integers only** -- No floating point. Fixed-point is used where
  needed (angles as "millidegrees", 16.16 or 8.24 fixed point for scaling)
- **Single address space** -- All globals, locals, code, and strings live in
  `mem[]`. Processes share globals and have their own local variable blocks.
- **Cooperative multitasking** -- Processes yield explicitly via `FRAME`.
  There is no preemption.
- **Process = sprite** -- Each DIV process corresponds to a sprite on screen.
  The process's local variables include position, graphic, angle, etc.

### Memory Layout at Runtime

```
mem[0..8]              Program header
mem[9..end_struct+67]  Predefined globals (mouse, scroll, m7, joy, setup, etc.)
mem[end_struct+..]     User-defined globals, then code
mem[iloc..]            Local variable template
                       --- process instances follow ---
mem[id_start..]        Process 0 (locals copied from template)
mem[id_start+iloc_len] Process 1
...
mem[id_end]            Last active process
```

Each process occupies `iloc_len` words. The first `iloc_pub_len` words are
copied from the local template; the rest is private data initialized by `lpri`.

### Execution Stack

```c
int pila[long_pila + max_exp + 64];  // 2048 + 512 + 64 = 2624 entries
```

The stack is shared across all processes but each process saves/restores its
stack pointer (`_SP` field). The stack grows upward (increasing `sp`).

### Key Runtime Variables

| Variable | Type | Description |
|----------|------|-------------|
| `mem` | `int*` | Main memory array |
| `memb` | `byte*` | Same memory, byte-addressed (for byte/string ops) |
| `memw` | `word*` | Same memory, word-addressed |
| `ip` | `int` | Instruction pointer (index into `mem[]`) |
| `sp` | `int` | Stack pointer (index into `pila[]`) |
| `bp` | `int` | Base pointer (for parameter passing) |
| `id` | `int` | Current process ID (offset into `mem[]`) |
| `ide` | `int` | Process being executed/painted |
| `process_count` | `int` | Number of living processes |

---

## 2. Interpreter Loop

### Entry: `interpreter()` in `src/runtime/interpreter.c` (line 754)

```c
void interpreter(void) {
    initialization();
    while (process_count && !(kbdFLAGS[_ESC] && kbdFLAGS[_L_CTRL]) && !alt_x) {
        mainloop();
    }
    finalization();
}
```

The game runs until all processes are dead, or Ctrl+Esc / Alt+X is pressed.

### Frame Loop: `mainloop()` at line 665

Each frame:

1. **`frame_start()`** (line 922):
   - Polls keyboard/mouse events via `tecla()`
   - Pauses if app lost focus (`app_paused` flag)
   - Handles screensaver activation
   - Eliminates dead processes (`_Status == 1`)
   - Updates 10 user timers (`timer(0)` through `timer(9)`)
   - Calculates FPS
   - Frame timing: waits until `fractional_clock` (next frame time) or allows frame
     skipping (`max_frame_skips`)
   - Marks all processes as not-yet-executed
   - Reads mouse and joystick state

2. **Execute processes** in a loop:
   ```c
   do {
       exec_process();
   } while (ide);
   ```

3. **`frame_end()`** (line 1133):
   - Restores background
   - Sorts and paints all sprites by Z-order
   - Handles scrolling backgrounds and Mode 7
   - Renders text objects and drawings
   - Updates palette fading
   - Blits to screen via `blit_screen()`

### Process Execution: `exec_process()` at line 744

Selects the highest-priority unexecuted process:

```c
id = id_old;
do {
    if (mem[id+_Status]==2 && !mem[id+_Executed] && mem[id+_Priority]>max) {
        ide = id;
        max = mem[id+_Priority];
    }
    if (id == id_end) id = id_start; else id += iloc_len;
} while (id != id_old);
```

If a process has a partial frame (`_Frame >= 100`), it decrements by 100 and
skips execution. Otherwise, it loads the process's saved IP and stack, then
calls `core_exec()`.

### Bytecode Execution: `core_exec()` at line 810

```c
void core_exec() {
    do {
        switch ((byte)mem[ip++]) {
            #include "debug/kernel.inc"
        }
    } while (1);

    next_process1:
    mem[ide+_Executed] = 1;
    next_process2: ;
}
```

The switch statement is a giant `#include` of `src/runtime/debug/kernel.inc`,
which contains all the `case` handlers. Each case corresponds to an EML opcode.

### Key Opcode Implementations (from `kernel.inc`)

**Stack operations:**
```c
case lcar: pila[++sp] = mem[ip++]; break;     // Push constant
case lasp: sp--; break;                        // Pop/discard
case lptr: pila[sp] = mem[pila[sp]]; break;   // Dereference
case laid: pila[sp] += id; break;              // Add process base address
case lcid: pila[++sp] = id; break;             // Push current process ID
```

**Arithmetic:**
```c
case ladd: pila[sp-1] += pila[sp]; sp--; break;
case lsub: pila[sp-1] -= pila[sp]; sp--; break;
case lmul: pila[sp-1] *= pila[sp]; sp--; break;
case ldiv: pila[sp-1] /= pila[sp]; sp--; break;
case lmod: pila[sp-1] %= pila[sp]; sp--; break;
case lneg: pila[sp] = -pila[sp]; break;
```

**Assignment:**
```c
case lasi: pila[sp-1] = mem[pila[sp-1]] = pila[sp]; sp--; break;
```

**Control flow:**
```c
case ljmp: ip = mem[ip]; break;
case ljpf: if (pila[sp--] & 1) ip++; else ip = mem[ip]; break;
```

Note: `ljpf` tests bit 0 of the value, not zero/non-zero. This means
conditions must produce 0 or 1 (the comparison operators do this).

**Process creation (`lcal`):**
```c
case lcal:
    mem[id+_IP] = ip+1;           // Save caller's IP
    process_count++;               // Increment process count
    ip = mem[ip];                  // Jump to process code
    // Find free process slot
    id = id_start;
    while (mem[id+_Status] && id <= id_end) id += iloc_len;
    if (id > id_end) id_end = id;  // Expand process pool
    memcpy(&mem[id], &mem[iloc], iloc_pub_len << 2);  // Copy local template
    mem[id+_Id] = id;              // Set process ID
    // Link into process tree (Father, Son, BigBro, SmallBro)
    mem[id+_Father] = mem[id+_Caller] = id2;
    break;
```

**FRAME (`lfrm`):**
```c
case lfrm:
    sp = mem[id+_Param] - mem[id+_NumPar];  // Reset stack
    mem[id+_IP] = ip;                         // Save IP for next frame
    mem[id+_IdScan] = 0;                      // Reset scan state
    pila[sp] = id;
    id = mem[id+_Caller];
    if (!(id & 1)) goto next_process1;        // If caller is dead, finish
    ip = mem[id+_IP];                         // Resume caller
    // If this was a FUNCTION, sleep the caller
    if (mem[pila[sp]+_FCount] > 0) {
        mem[pila[sp]+_Caller]++;              // Mark caller as sleeping
        mem[id+_Status] = 3;                  // Sleep caller
        goto next_frm;
    }
    break;
```

This is the cooperative multitasking mechanism: `FRAME` saves the current
process state and returns to the scheduler.

**RETURN (`lret`):**
```c
case lret:
    sp = mem[id+_Param] - mem[id+_NumPar];
    pila[sp] = id;
    id = mem[id+_Caller];
    kill_process(pila[sp]);        // Kill the current process
    if (!(id & 1)) goto next_process1;
    ip = mem[id+_IP];
    break;
```

---

## 3. Process Model

### Process Fields

Each process has these fields at fixed offsets from its base address
(defined in `src/runtime/inter.h`, lines 440-488):

| Offset | Name | Description |
|--------|------|-------------|
| 0 | `_Id` | Process ID (equals its base address in mem[]) |
| 1 | `_IdScan` | Scan pointer for collision/iteration |
| 2 | `_Bloque` | Process type ID (for `collision()`) |
| 3 | `_BlScan` | Last scanned process type |
| 4 | `_Status` | 0=dead, 1=killed, 2=alive, 3=sleeping, 4=frozen |
| 5 | `_NumPar` | Number of parameters |
| 6 | `_Param` | Stack base pointer for parameters |
| 7 | `_IP` | Saved instruction pointer |
| 8 | `_SP` | Saved stack pointer |
| 9 | `_Executed` | 1 if already executed this frame |
| 10 | `_Painted` | 1 if already painted this frame |
| 13 | `_Frame` | Frame accumulator (for `frame(N)`) |
| 14-17 | `_x0`..`_y1` | Bounding box of last painted sprite |
| 18 | `_FCount` | Function call depth (>0 = is a function) |
| 19 | `_Caller` | ID of the calling process |
| 20 | `_Father` | ID of the parent process |
| 21 | `_Son` | ID of the last child created |
| 22 | `_SmallBro` | Younger sibling process |
| 23 | `_BigBro` | Older sibling process |
| 24 | `_Priority` | Execution priority (higher = executes first) |
| 25 | `_Ctype` | Coordinate type: 0=screen, 1=scroll, 2=mode7 |
| 26 | `_X` | X coordinate |
| 27 | `_Y` | Y coordinate |
| 28 | `_Z` | Z depth (higher = painted first = behind) |
| 29 | `_Graph` | Graphic code (index into FPG) |
| 30 | `_Flags` | Bit 0: horizontal flip, Bit 1: vertical flip, Bit 2: ghost |
| 31 | `_Size` | Scale percentage (100 = normal) |
| 32 | `_Angle` | Rotation angle in millidegrees (0-360000) |
| 33 | `_Region` | Clipping region |
| 34 | `_File` | FPG file index (0 = default) |
| 35 | `_XGraph` | Pointer to animation table |
| 36 | `_Height` | Height for Mode 7 rendering |
| 37 | `_Cnumber` | Scroll/Mode7 layer index |
| 38 | `_Resolution` | Coordinate resolution divisor |

### Process Lifecycle

1. **Creation** (`lcal`): Template copied, linked into family tree (Father/Son/
   BigBro/SmallBro), Status set to 2 (alive).
2. **Execution**: Selected by priority each frame, runs until FRAME or RETURN.
3. **Sleeping** (Status=3): Paused by a function call; wakes when the function
   returns.
4. **Frozen** (Status=4): Paused by `signal(id, s_freeze)`. Still painted but
   not executed.
5. **Killed** (Status=1): Marked for deletion; removed at next `frame_start()`.
6. **Dead** (Status=0): Slot is free for reuse.

### Process Tree

Processes form a tree structure:
- `_Father` points to the process that created this one
- `_Son` points to the most recently created child
- `_BigBro` / `_SmallBro` link siblings in creation order

The tree is used by `signal()` with `s_tree` variants to send signals
(kill/sleep/freeze/wake) to an entire subtree.

### Process Communication

Processes communicate through:
- **Shared globals** -- All processes can read/write global variables
- **Remote access** -- `process_id.variable` reads/writes another process's
  local variables (the `.` operator with an ID)
- **`collision(type)`** -- Checks if the current process's sprite overlaps any
  process of the given type
- **`signal(id, signal_type)`** -- Send control signals to other processes

---

## 4. Runtime Services

### Built-in Functions: `src/runtime/functions.c`

`function()` is called by the `lfun` opcode. It reads `mem[ip-1]` to get the
function code, then dispatches to the implementation. Functions pop their
arguments from `pila[]` and push results back.

Key functions include:

| Function | Code | Description |
|----------|------|-------------|
| `signal` | -- | Send signal to process(es) |
| `collision` | -- | Sprite collision detection |
| `get_id` | -- | Find processes by type |
| `get_dist` | -- | Distance between two processes |
| `get_angle` | -- | Angle between two processes |
| `get_distx`/`get_disty` | -- | Distance component from angle |
| `rand` | -- | Random number in range |
| `load_fpg` | -- | Load FPG file at runtime |
| `load_map` | -- | Load MAP file |
| `load_pal` | -- | Load palette |
| `load_pcm`/`load_wav` | -- | Load sound |
| `sound` | -- | Play sound effect |
| `fade_on`/`fade_off` | -- | Palette fade in/out |
| `write` | -- | Create on-screen text object |
| `delete_text` | -- | Remove text object |
| `scroll_start` | -- | Start a scrolling background |
| `start_mode7` | -- | Start Mode 7 rendering |
| `key` | -- | Check if a key is pressed |
| `fopen`/`fread`/`fwrite` | -- | File I/O |

### Timer System

```c
#define timer(x) mem[end_struct + x]    // 10 timers, timer(0) through timer(9)
```

Timers are updated every frame in `frame_start()`. They count in centiseconds
(1/100th of a second), derived from `SDL_GetTicks()`.

### Sound System: `src/shared/run/sound.c`

The runtime uses SDL2_mixer for audio:
- Sound effects loaded as `Mix_Chunk*` via `Mix_LoadWAV_RW()`
- Music modules loaded via `Mix_LoadMUS()` or `Mix_LoadMUSType_RW()`
- Raw PCM data stored in `pcminfo.sound_data`, converted to `Mix_Chunk`
  for playback
- 32 channels available (`channel(0)` through `channel(31)`)

### Clock: `get_reloj()` in `src/runtime/functions.c` (line 70)

```c
int get_reloj(void) {
    n_reloj = OSDEP_GetTicks();
    frame_clock += (n_reloj - o_reloj);
    o_reloj = n_reloj;
    return frame_clock;
}
```

Returns milliseconds since start. The `frame_clock` variable accumulates
time deltas to avoid issues with SDL tick counter wrapping.

---

## 5. Rendering Pipeline

### Overview

The rendering pipeline is entirely 8-bit paletted. Each frame:

1. **Restore background** -- Copy `back_buffer` (clean background) back to `screen_buffer`
   (working framebuffer)
2. **Sort and paint** -- All visible entities are painted in Z-order
3. **Blit to display** -- The 8-bit `screen_buffer` is converted to 32-bit and
   presented via SDL2

### Sprite Sorting and Painting: `frame_end()` at line 1133

The rendering uses a priority-based selection loop (not a pre-sorted array):

```c
do {
    ide = 0; max = 0x80000000;

    // Find highest-Z unexecuted screen-space process
    for (id = id_start; id <= id_end; id += iloc_len)
        if ((mem[id+_Status]==2 || mem[id+_Status]==4) &&
            mem[id+_Ctype]==0 && !mem[id+_Executed] && mem[id+_Z] > max) {
            ide = id; max = mem[id+_Z];
        }

    // Also check Mode 7 layers, scroll layers, text objects, mouse, drawings
    // against the same Z value

    // Paint whichever has the highest Z
    if (otheride) { ... }
    else if (scrollide) { scroll_simple() or scroll_parallax(); }
    else if (m7ide) { paint_m7(); }
    else if (ide) { paint_sprite(); }

} while (ide || m7ide || scrollide || otheride);
```

This means entities are painted from highest Z to lowest Z (back to front).
Higher `_Z` values appear behind lower ones.

### Sprite Rendering: `paint_sprite()` in `src/runtime/render.c` (line 869)

For each process with a visible graphic:

1. Look up the graphic from `g[file].grf[graph]`
2. Determine the graphic center (control point 0, or center of image)
3. Determine the clipping region
4. Handle `_XGraph` (animation tables): select the correct frame based on
   `_Angle`
5. Choose rendering method:
   - **`sp_normal()`** -- No rotation or scaling. Direct byte copy with optional
     horizontal/vertical flip and ghost transparency.
   - **`sp_clipped()`** -- Same as normal but with clipping.
   - **`sp_scaled()`** -- Scaled rendering using fixed-point stepping.
   - **`sp_rotated()`** -- Rotated rendering using sine/cosine lookup tables
     (4096-entry table for full rotation).
6. Save the painted region in `mem[ide+_x0.._y1]` for partial screen updates.

### Normal Sprite: `sp_normal()` at line 953

```c
void sp_normal(byte * p, int x, int y, int an, int al, int flags) {
    byte *q = screen_buffer + y*vga_width + x;
    switch (flags & 7) {
        case 0: // No flip, no ghost
            while (al--) {
                for (n=0; n<an; n++) if (p[n]) q[n] = p[n];
                p += an; q += vga_width;
            }
            break;
        case 4: // Ghost transparency (50% blend using ghost table)
            ...
        case 1: // Horizontal flip
            ...
        // etc.
    }
}
```

Color 0 is always transparent. The `ghost` flag (bit 2 of `_Flags`) uses the
256x256 ghost table to blend sprite pixels with background pixels.

### Scroll System: `scroll_simple()` at line 194

Implements hardware-style scrolling backgrounds:
- `scroll[N]` struct has: camera process, speed, region, x/y offset
- The camera process's position determines the scroll viewport
- Background maps tile/wrap as the viewport moves
- Sprites with `_Ctype==1` are drawn relative to the scroll position

### Mode 7: `paint_m7()` and `paint_mode7()`

Mode 7-style floor/ceiling rendering (like SNES):
- Uses `m7[N]` struct: camera, height, distance, horizon, focus, color
- Renders a perspective-projected ground plane from a 2D tile map
- Uses 16.16 fixed-point math for the projection
- Sprites are scaled based on their distance from the camera

### Video Output: `src/shared/run/video.c`

**Runtime rendering path:**

```
screen_buffer (byte* working buffer)
  |
  v
blit_screen(screen_buffer) or blit_partial()  -- copies to OSDEP_buffer8
  |
  v
OSDEP_Flip()
  |-- SDL_BlitSurface(buffer8 -> buffer32)  -- palette index to RGBA
  |-- SDL_UpdateTexture(texture, buffer32)
  |-- SDL_RenderCopy(renderer, texture)
  |-- SDL_RenderPresent()
```

**Palette management:**

```c
void update_palette(void);  // Applies fade (dacout_r/g/b) to base palette (dac[])
void set_dac(void);         // Calls OSDEP_SetPalette() with the modified palette
```

The palette fading system smoothly interpolates `now_dacout_r/g/b` toward
target `dacout_r/g/b` values at a rate controlled by `dacout_speed`.

**`fade_off()`** sets target to (0,0,0) for a fade-to-black.
**`fade_on()`** sets target to (64,64,64) for full brightness.

### Partial Screen Updates

When `dump_type == 0`, the runtime tracks dirty rectangles. Each painted sprite
records its bounding box in `mem[ide+_x0.._y1]`. The `restore()` function
only copies back the regions that were modified, and `blit_partial()` only
blits the changed regions. This optimization is important for the original DOS
version but less critical with SDL2.

---

## 6. Predefined Global Structures

These structs are defined in `src/runtime/inter.h` and accessible from DIV
programs:

### `mouse` struct (line 317)
```
x, y, z, file, graph, angle, size, flags, region, left, middle, right, cursor, speed
```

### `scroll[10]` struct (line 323)
```
z, camera, ratio, speed, region1, region2, x0, y0, x1, y1
```

### `m7[10]` struct (Mode 7, line 329)
```
z, camera, height, distance, horizon, focus, color
```

### `joy` struct (line 335)
```
button1, button2, button3, button4, left, right, up, down
```

### `setup` struct (line 342)
```
card, port, irq, dma, dma2, master, sound_fx, cd_audio, mixer, mixrate, mixmode
```

### Other predefined globals (line 404+)

| Name | Description |
|------|-------------|
| `timer(0..9)` | 10 centisecond timers |
| `text_z` | Z-depth for text objects |
| `fading` | 1 if a fade is in progress |
| `shift_status` | Keyboard modifier state |
| `ascii` | Last ASCII key pressed |
| `scan_code` | Last scan code pressed |
| `fps` | Current frames per second |
| `max_process_time` | Maximum ticks per process before timeout |
| `channel(0..31)` | Sound channel status |
| `draw_z` | Z-depth for drawing primitives |
