# The DIV Programming Language — A Primer

DIV is a programming language designed for making 2D games. It was created
in 1997 by Daniel Navarro at Hammer Technologies (Spain) and shipped as
part of DIV Games Studio, an integrated IDE with code editor, paint editor,
sprite/font/sound editors, compiler, and runtime — all in one package.

This primer covers the core ideas that make DIV unique. It's not exhaustive
— just enough to understand why the language works so well for the games
it targets.

---

## Everything is a process

The central idea in DIV: **every game entity is a PROCESS**. A process is
simultaneously a unit of execution (like a coroutine) *and* a sprite on
screen. When you define a process, you're defining both its behavior and
its visual presence.

```
PROCESS ship(x, y);
BEGIN
    graph = 1;          // sprite graphic #1
    LOOP
        IF (key(_right)) angle -= pi/16; END
        IF (key(_left))  angle += pi/16; END
        IF (key(_up))
            x += get_distx(angle, 5);
            y += get_disty(angle, 5);
        END
        FRAME;          // yield — render me, then resume next tick
    END
END
```

That's a controllable spaceship. `x`, `y`, `angle`, `graph`, and `size`
are **built-in local variables** that every process has — they control
where and how the process is drawn. You don't call a render function.
You just set `x` and `y`, and the runtime draws you.

## FRAME — the heartbeat

`FRAME` is the cooperative yield point. When a process hits FRAME:

1. It pauses execution
2. The runtime runs all other processes until they also hit FRAME
3. All processes are rendered as sprites (sorted by `z` depth)
4. The screen flips
5. Everyone resumes where they left off

This is the entire game loop. There's no `update()` callback, no
`draw()` method, no event pump to write. Each process is its own
self-contained loop that runs forever (or until killed), yielding
every frame.

## Creating processes

Creating a process looks like calling a function:

```
ship(320, 240);                     // spawn a ship at center screen
asteroid(-16, -16, 3);              // spawn an asteroid off-screen
firing_ship(x, y, angle);          // spawn a bullet from current position
```

Each call creates a new concurrent instance. The parameters map to the
process's declared arguments (which are also its local variables). Every
process gets a unique ID — the return value of the call.

## Collision detection

Processes can detect collisions with other processes by type:

```
PROCESS asteroid(x, y, graph);
BEGIN
    LOOP
        IF (id2 = collision(TYPE firing_ship))  // hit by a bullet?
            signal(id2, s_kill);                // destroy the bullet
            score += 100;
            signal(ID, s_kill);                 // destroy myself
        END
        IF (collision(TYPE ship))               // hit the player?
            // game over logic
        END
        x += get_distx(angle, speed);
        y += get_disty(angle, speed);
        FRAME;
    END
END
```

`collision(TYPE process_name)` checks pixel-perfect overlap between
the current process and any instance of the named type. It returns the
colliding process's ID (or 0 if no collision).

## Signals — inter-process communication

Processes talk to each other through signals:

```
signal(id, s_kill);             // kill a specific process
signal(TYPE asteroid, s_kill);  // kill ALL asteroids
signal(id, s_sleep);            // pause a process (stops running and drawing)
signal(id, s_freeze);           // freeze a process (stops running, still draws)
signal(id, s_wakeup);           // resume a sleeping/frozen process
let_me_alone();                 // kill everything except me
```

## Cross-process access

Every process's local variables are accessible from outside via its ID:

```
IF (id2 = collision(TYPE ship))
    piece(id2.x, id2.y, id2.angle, 6);   // spawn debris at the ship's position
END
```

`id2.x` reads the `x` variable of the process whose ID is `id2`. This
is how processes inspect each other's state — no getters, no message
passing, just direct field access.

## Program structure

A complete DIV program looks like this:

```
PROGRAM steroid;

GLOBAL
    score = 0;          // shared by all processes
    lives = 3;

BEGIN
    set_mode(m640x480);
    load_fpg("steroid\steroid.fpg");    // load sprite graphics

    ship(320, 240);                      // spawn the player
    FROM number0 = 0 TO 4;
        asteroid(-16, -16, 3);           // spawn 5 asteroids
    END

    LOOP
        IF (lives == 0) BREAK; END
        FRAME;
    END
END
```

The PROGRAM block is the main process. It sets up resources, spawns
initial processes, and runs the game loop. When it ends, the program
exits.

## Data scopes

There are three scopes for variables:

- **GLOBAL** — shared across all processes. Declare once, read/write anywhere.
- **LOCAL** — per-process, but readable from outside via `id.field`. The
  built-ins (`x`, `y`, `graph`, `angle`, `size`, `z`, `flags`, etc.)
  are all locals.
- **PRIVATE** — per-process, only accessible from inside that process.

```
PROCESS asteroid(x, y, graph);    // x, y, graph are locals (parameters)
PRIVATE
    speed;                         // only this asteroid can see its speed
    angle2;
BEGIN
    speed = graph + level;         // 'level' is global
    angle2 = rand(0, 2*pi);
    ...
END
```

## Built-in process locals

Every process automatically has these fields (among others):

| Local | Purpose |
|-------|---------|
| `x`, `y` | Screen position |
| `z` | Depth sorting (lower = in front) |
| `graph` | Sprite graphic number within the current FPG |
| `file` | FPG file to use (0 = first loaded) |
| `angle` | Rotation (in thousandths of a degree; `pi` = 180000) |
| `size` | Scale percentage (100 = normal) |
| `flags` | Flip/transparency flags |
| `priority` | Execution order (higher = runs first) |
| `father` | ID of the process that created this one |
| `son` | ID of last created child process |

## Control flow

DIV offers the usual constructs with Pascal-influenced syntax:

```
IF (condition) ... ELSE ... END

SWITCH (value)
    CASE 1: ... END
    CASE 2,3: ... END
    DEFAULT: ... END
END

WHILE (condition) ... END
REPEAT ... UNTIL (condition)
FOR (init; condition; step) ... END
FROM x = 0 TO 99; ... END

LOOP ... END                     // infinite loop (break with BREAK)
```

Semicolons separate statements. `END` closes every block.

## A complete game in ~150 lines of logic

The Asteroids example (`Steroid.prg`) that ships with DIV implements:

- A rotating, thrust-driven **ship** with screen wrapping
- **Bullets** that fly in the ship's direction and expire after 20 frames
- **Asteroids** that split into smaller pieces when hit
- **Debris particles** that shrink and fade when the ship explodes
- **Hyperspace** teleportation with a visual effect
- Score tracking, lives, level progression, fade transitions

All in about 370 lines, with every game entity as its own process.
The ship process handles input and physics. The asteroid process handles
movement and collision. The bullet process flies straight and dies.
The debris process spins and shrinks. Each is independent, each is
self-contained, and they interact through collision checks and signals.

That's the core of DIV: **define your game objects as processes, give
them behavior in a loop, yield with FRAME, and let the runtime handle
the rest.**
