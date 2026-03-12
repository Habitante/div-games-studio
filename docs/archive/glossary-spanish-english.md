# DIV Games Studio - Spanish-English Glossary

**Historical reference.** The DIV codebase was originally written in Spanish.
During Phase 2C (2026-03), all identifiers were renamed to English snake_case.
This glossary preserves the old-to-new mapping for anyone reading git history,
the original MSDOS source, or Daniel's LIBRO.DOC handbook.

File paths shown are the **original** names; see `docs/architecture-overview.md`
for the current source tree layout (reorganized in Phase 2C-2).

---

## Video / Display

| Original | Current name | English meaning |
|---|---|---|
| `vga_an` | `vga_width` | VGA width ("ancho" = width) |
| `vga_al` | `vga_height` | VGA height ("alto" = height) |
| `copia` | `backup_buffer` | Virtual screen copy (the working framebuffer) |
| `tapiz` | `wallpaper` | Desktop wallpaper / background texture |
| `tapiz_an` | `wallpaper_width` | Wallpaper width |
| `tapiz_al` | `wallpaper_height` | Wallpaper height |
| `volcado` | `blit_screen()` | Screen dump / blit ("volcar" = to dump/pour) |
| `volcado_completo` | `full_redraw` | Full-screen redraw flag |
| `volcado_parcial` | `blit_partial()` | Partial screen blit (dirty rectangle) |
| `volcados_saltados` | `blits_skipped` | Skipped frame count |
| `saltar_volcado` | `skip_blit` | Skip this frame's display update |
| `fondo_raton` | `mouse_background` | Mouse cursor background save buffer |
| `fondo_edicion` | (removed) | Drawing editing background |

## Palette / Colors

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `dac` | `byte*` | global.h:496 | Palette buffer (768 bytes, 256 RGB triplets; "DAC" from VGA DAC) |
| `dac4` | `byte*` | global.h:497 | Palette with full 0-255 range (vs. VGA 0-63) |
| `paleta` | `byte[768]` | divpalet.c:44 | Palette ("paleta" = palette) |
| `ghost` | `byte*` | global.h:495 | 256x256 transparency blending table |
| `cuad` | `byte*` | global.h:523 | Squared-difference table for color matching ("cuadrado" = squared) |
| `c0..c4` | `byte` | global.h:504 | Environment colors (c0=black, c2=mid, c4=bright) |
| `c01,c12,c23,c34` | `byte` | global.h:505 | Intermediate colors (averages) |
| `c_r,c_g,c_b` | `byte` | global.h:506 | Pure red, green, blue colors |
| `c_r_low,c_g_low,c_b_low` | `byte` | global.h:506 | Dark red, green, blue colors |
| `c_com` | `byte` | global.h:507 | Comment color in editor ("comentario" = comment) |
| `c_sim` | `byte` | global.h:507 | Symbol color in editor ("simbolo" = symbol) |
| `c_res` | `byte` | global.h:507 | Reserved word color ("reservada" = reserved) |
| `c_pre` | `byte` | global.h:507 | Preprocessor color |
| `c_num` | `byte` | global.h:507 | Number color |
| `c_lit` | `byte` | global.h:507 | Literal/string color |
| `regla` | `int` | global.h:582 | Selected color rule/gradient index ("regla" = rule/ruler) |
| `reglas[16]` | `tipo_regla` | global.h:580 | Color gradient rules (for paint tool) |
| `near_regla` | `byte[190]` | global.h:584 | Nearest color in the selected gradient |
| `find_color` | function | divpalet.c | Find nearest palette color to RGB |
| `find_colors` | function | divpalet.c | Find all standard UI colors in palette |
| `calcula_regla` | function | divpalet.c | Calculate/generate a color gradient |

## Window / UI System

| Original | Current name | English meaning |
|---|---|---|
| `ventana[96]` / `tventana` | `window[96]` / `twindow` | Window array |
| `v` | `v` (unchanged) | `window[0]` -- the frontmost window |
| `primer_plano` | `foreground` | Foreground/focus state |
| `titulo` | `title` | Window title |
| `nombre` | `name` | Icon name |
| `volcar` | `redraw` | Needs-redraw flag |
| `orden` | `order` | Z-order number |
| `texto[2048]` | `texts[2048]` | Localized text strings |
| `fin_dialogo` | `end_dialog` | End-dialog flag |
| `salir_del_entorno` | `exit_requested` | Exit-environment flag |
| `boton()` | `draw_button()` | Draw button |
| `ratonboton()` | `mouse_button_hit()` | Mouse button hit-test |
| `crear_menu()` | `create_menu()` | Create menu |
| `pinta_menu()` | `paint_menu()` | Draw/paint menu |
| `actualiza_menu()` | `update_menu()` | Update menu |
| `entorno()` | `main_loop()` | Main environment loop |
| `entorno_dialogo()` | `dialog_loop()` | Dialog environment loop |

## Mouse / Input

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `raton` | -- | -- | Mouse ("raton" = mouse) |
| `mouse_x, mouse_y` | `int` | global.h:518 | Mouse position |
| `mouse_b` | `int` | global.h:518 | Mouse button state |
| `mouse_graf` | `int` | global.h:518 | Mouse cursor graphic index |
| `old_mouse_b` | `int` | global.h:521 | Previous mouse button state |
| `wmouse_x, wmouse_y` | `int` | global.h:648 | Mouse position within window |
| `tecla` | function | divkeybo.c | Keyboard poll / event pump ("tecla" = key) |
| `vacia_buffer` | function | divkeybo.c | Empty the keyboard buffer ("vacia" = empties) |
| `arrastrar` | `int` | global.h:836 | Drag state machine ("arrastrar" = to drag) |
| `arrastrar_x/y` | `int` | global.h:836 | Drag start position |
| `arrastrar_graf` | `int` | global.h:836 | Graphic shown during drag |
| `quien_arrastra` | `int` | global.h:840 | Which window is dragging ("quien" = who) |
| `doble_click` | `int` | global.h:834 | Double-click timestamp |
| `shift_status` | `char` | global.h:817 | Keyboard modifier flags |

## Editor / Paint

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `modo` | `int` | global.h:588 | Drawing mode ("modo" = mode): 0=pencil, 1=lines, 2=bezier, etc. |
| `modo_caja` | `int` | global.h:638 | Box/rectangle mode ("caja" = box) |
| `modo_circulo` | `int` | global.h:639 | Circle mode |
| `modo_fill` | `int` | global.h:640 | Fill algorithm mode ("relleno" = fill) |
| `modo_seleccion` | `int` | global.h:641 | Selection mode |
| `barra` | `byte*` | global.h:540 | Paint toolbar buffer ("barra" = bar/toolbar) |
| `barra_x/y/an` | `int` | global.h:586 | Toolbar position and width |
| `mapa` | tmapa* | tventana | Map (image) being edited ("mapa" = map) |
| `map_an, map_al` | `int` | global.h:483 | Map width/height |
| `mab` | `int*` | global.h:494 | Selection bitmask for edited map ("mapa de bits" = bitmap) |
| `mask[256]` | `byte` | global.h:511 | Paint mask ("mascara" = mask) |
| `mask_on` | `int` | global.h:512 | Mask enabled flag |
| `hacer_zoom` | `int` | global.h:552 | Need-to-redraw zoom flag ("hacer" = to do/make) |
| `coord_x, coord_y` | `int` | global.h:544 | Coordinates on the edited map |
| `sel_status` | `int` | global.h:628 | Selection active status |
| `coloreador` | `int` | global.h:339 | Syntax colorizer enabled ("coloreador" = colorizer) |
| `guardar_prg` | function | divedit.c | Save program ("guardar" = to save) |
| `abrir_programa` | function | divedit.c | Open program ("abrir" = to open) |
| `buscar_texto` | function | divedit.c | Find text ("buscar" = to search) |
| `sustituir_texto` | function | divedit.c | Replace text ("sustituir" = to replace) |
| `repinta_ventana` | function | divedit.c | Repaint window ("repintar" = to repaint) |

## Compiler

All compiler code is now in `src/compiler/compiler.c`.

| Original | Current name | English meaning |
|---|---|---|
| `compilar()` | `compile()` | Compile |
| `lexico()` | `lexer()` | Lexer / tokenizer |
| `sintactico()` | `parser()` | Parser |
| `sentencia()` | `statement()` | Statement parser |
| `expresion()` | `expression()` | Expression parser |
| `pieza` | `current_token` | Current token ("pieza" = piece) |
| `pieza_num` | `token_value` | Numeric value of current token |
| `linea` | `source_line` | Current line number |
| `objeto` (struct) | `object` | Symbol table entry |
| `bloque_actual` | `current_scope` | Current scope / process |
| `bloque_lexico` | `lexical_scope` | Lexical scope block |
| `miembro` | `member` | Current struct member being parsed |
| `acceso_remoto` | `cross_process_access` | Remote (cross-process) access flag |
| `optimizar` | (unchanged) | Optimization enabled flag |
| `inicio_sentencia()` | `begin_statement()` | Mark start of statement |
| `final_sentencia()` | `end_statement()` | Mark end of statement |

## Runtime / VM

Runtime code is in `src/runtime/` (interpreter.c, functions.c, render.c).

| Original | Current name | English meaning |
|---|---|---|
| `interprete()` | `interpreter()` | Interpreter main function |
| `nucleo_exec()` | `core_exec()` | Core execution loop ("nucleo" = nucleus/core) |
| `pila[2624]` | `pila[2624]` (unchanged) | Execution stack ("pila" = stack/pile) |
| `reloj` | `frame_clock` | Clock / timer ("reloj" = clock) |
| `procesos` | `process_count` | Living process count |
| `freloj` | `fractional_clock` | Next frame time target |
| `max_saltos` | `max_frame_skips` | Maximum frame skips allowed |
| `volcado()` | `blit_screen()` | Blit framebuffer to screen |
| `restaura()` | `restore()` | Restore background from backup |
| `pinta_sprite()` | `paint_sprite()` | Paint/render a process's sprite |
| `pinta_textos()` | `paint_texts()` | Render text objects |
| `elimina_proceso()` | `kill_process()` | Kill/remove a process |

## File I/O / Formats

Format code is in `src/formats/` (image.c, fpg.c).

| Original | Current name | English meaning |
|---|---|---|
| `fichero` | -- | File ("fichero" = file) |
| `archivo` | -- | File / archive ("archivo" = file/archive) |
| `graba_MAP()` | `save_map()` | Save MAP file |
| `descomprime_MAP()` | `load_map()` | Decompress/load MAP |
| `es_MAP()` | `es_MAP()` (unchanged) | Is it a MAP file? ("es" = is) |
| `tipo[24]` | `file_types[24]` | File type definitions |
| `abrir` | -- | Open ("abrir" = to open) |
| `guardar` | -- | Save ("guardar" = to save/keep) |
| `grabar` | -- | Write / record ("grabar" = to record) |

## Program Structure

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `tprg` | struct | global.h:724 | Program editor buffer |
| `buffer` | `byte*` | tprg | Source code text buffer |
| `buffer_lon` | `int` | tprg | Buffer length ("longitud" = length) |
| `file_lon` | `int` | tprg | File length |
| `num_lineas` | `int` | tprg | Number of lines ("lineas" = lines) |
| `columna` | `int` | tprg | Column ("columna" = column) |
| `primera_linea` | `int` | tprg | First visible line ("primera" = first) |
| `primera_columna` | `int` | tprg | First visible column |
| `linea_vieja` | `int` | tprg | Previous line ("vieja" = old) |
| `grabado` | `byte` | tmapa | Saved-to-disk flag |
| `descripcion` | `char[32]` | tmapa | Description text |
| `puntos[512]` | `short[]` | tmapa | Control points ("puntos" = points) |
| `codigo` | `int` | tmapa | Graphic code ("codigo" = code) |

## Configuration / Setup

| Original | Current name | English meaning |
|---|---|---|
| `Interpretando` | `returning_from_runtime` | Returning from interpreter |
| `siguiente_orden` | `next_order` | Next window order number |
| `modo_de_retorno` | `return_mode` | Return mode: 0=quit, 1=run, 3=test fail |
| `tipo[24]` / `ttipo` | `file_types[24]` / `file_type_info` | File type definitions |
| `mascara[512]` | `file_filter[512]` | File filter mask |

## Common Spanish Words in Identifiers

Useful for reading git history, the original MSDOS source, or LIBRO.DOC.

| Spanish | English | Example (original → current) |
|---|---|---|
| `an` / `ancho` | width | `vga_an` → `vga_width` |
| `al` / `alto` | height | `vga_al` → `vga_height` |
| `bloque` | block / scope | `bloque_actual` → `current_scope` |
| `buscar` | to search | `buscar_texto()` → (removed) |
| `caja` | box | `modo_caja` → `box_mode` |
| `cerrar` | to close | `p_cerrar` (token, unchanged) |
| `crear` | to create | `crear_menu()` → `create_menu()` |
| `entorno` | environment | `entorno()` → `main_loop()` |
| `fondo` | background | `fondo_raton` → `mouse_background` |
| `fuente` | font / source | `divfont.c` → `editor/font.c` |
| `guardar` | to save | `guardar_prg()` → `save_program()` |
| `linea` | line | `linea` → `source_line` (compiler) |
| `mapa` | map / image | `tmapa` (unchanged struct name) |
| `paleta` | palette | `paleta` → `palette` |
| `pieza` | piece / token | `pieza` → `current_token` |
| `pila` | stack | `pila[]` (unchanged) |
| `pintar` | to paint | `pinta_sprite()` → `paint_sprite()` |
| `ratón` | mouse | `read_mouse()` (unchanged) |
| `reloj` | clock | `reloj` → `frame_clock` |
| `sonido` | sound | `divsound.c` → `shared/run/sound.c` |
| `tapiz` | wallpaper | `tapiz` → `wallpaper` |
| `tecla` | key | `tecla()` → `poll_keyboard()` |
| `ventana` | window | `ventana[]` → `window[]` |
| `volcado` | dump / blit | `volcado()` → `blit_screen()` |

## Process Fields (Local Variables)

These are defined in `src/runtime/inter.h` and are accessible from DIV programs.
While the internal C names use English prefixes (`_X`, `_Y`, etc.), some use
Spanish:

| Field | Offset | Spanish origin | English meaning |
|---|---|---|---|
| `_Id` | 0 | -- | Process identifier |
| `_Bloque` | 2 | "bloque" = block | Process type block (for collision grouping) |
| `_Status` | 4 | -- | Process lifecycle status |
| `_Executed` | 9 | -- | Already-executed-this-frame flag |
| `_Painted` | 10 | -- | Already-painted-this-frame flag |
| `_Caller` | 19 | -- | Calling process ID |
| `_Father` | 20 | -- | Parent process ID ("padre" = father) |
| `_Son` | 21 | -- | Last child ("hijo" = son) |
| `_SmallBro` | 22 | -- | Younger sibling ("hermano menor") |
| `_BigBro` | 23 | -- | Older sibling ("hermano mayor") |
| `_Priority` | 24 | -- | Execution priority ("prioridad") |
| `_Ctype` | 25 | -- | Coordinate type |
| `_Resolution` | 38 | -- | Coordinate resolution divisor |
