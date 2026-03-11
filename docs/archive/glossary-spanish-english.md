# DIV Games Studio - Spanish-English Glossary

The DIV codebase was originally written in Spanish. This glossary maps the most
important Spanish identifiers and terms to their English meanings, grouped by
category.

---

## Video / Display

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `vga_an` | `int` | global.h:484 | VGA width ("ancho" = width) |
| `vga_al` | `int` | global.h:484 | VGA height ("alto" = height) |
| `copia` | `byte*` | global.h:527 | Virtual screen copy (the working framebuffer) |
| `tapiz` | `byte*` | global.h:490 | Desktop wallpaper / background texture ("tapiz" = tapestry/wallpaper) |
| `tapiz_an` | `int` | global.h:516 | Wallpaper width |
| `tapiz_al` | `int` | global.h:516 | Wallpaper height |
| `mapa_tapiz` | `byte*` | global.h:491 | Wallpaper bitmap start pointer |
| `volcado` | function | divvideo.c | Screen dump / blit ("volcar" = to dump/pour) |
| `volcado_completo` | `int` | global.h:644 | Full-screen redraw flag |
| `volcado_parcial` | function | divvideo.c | Partial screen blit (dirty rectangle) |
| `volcados_parciales` | `int` | div.c:131 | Partial updates enabled flag |
| `volcados_saltados` | `int` | i.c | Skipped frame count |
| `saltar_volcado` | `int` | i.c | Skip this frame's display update |
| `retrazo` | function | v.c | VSync wait ("retrazo" = retrace) |
| `fondo_raton` | `byte*` | global.h:590 | Mouse cursor background save buffer ("fondo" = background, "raton" = mouse) |
| `fondo_resaltado` | `int` | global.h:509 | Highlighted background flag ("resaltado" = highlighted) |
| `fondo_edicion` | function | divbasic.c | Drawing editing background |

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

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `ventana[96]` | `tventana` | global.h:706 | Window array ("ventana" = window) |
| `v` | macro | global.h:707 | `ventana[0]` -- the frontmost window |
| `primer_plano` | `int` | tventana | Foreground/focus state ("primer plano" = foreground) |
| `titulo` | `byte*` | tventana | Window title ("titulo" = title) |
| `nombre` | `byte*` | tventana | Icon name ("nombre" = name) |
| `volcar` | `int` | tventana | Needs-redraw flag ("volcar" = to dump/blit) |
| `estado` | `int` | tventana | Window state ("estado" = state) |
| `orden` | `int` | tventana | Z-order number ("orden" = order) |
| `botones` | `int` | tventana | Button press state ("botones" = buttons) |
| `lado` | `int` | tventana | Side: 0=right, 1=left ("lado" = side) |
| `texto[2048]` | `byte*[]` | global.h:646 | Localized text strings ("texto" = text) |
| `fin_dialogo` | `int` | global.h:832 | End-dialog flag ("fin" = end) |
| `fin_ventana` | `int` | div.c:102 | End-window flag |
| `cierra_ventana` | function | div.c | Close window ("cierra" = closes) |
| `nueva_ventana` | function | div.c | Open new window ("nueva" = new) |
| `mueve_ventana` | function | div.c | Move window ("mueve" = moves) |
| `dialogo` | function | div.c | Open modal dialog ("dialogo" = dialog) |
| `entorno` | function | div.c | Main environment loop ("entorno" = environment/surroundings) |
| `entorno_dialogo` | function | div.c | Dialog environment loop |
| `salir_del_entorno` | `int` | global.h:798 | Exit-environment flag ("salir" = to exit) |
| `boton` | function | divwindo.c | Draw button ("boton" = button) |
| `ratonboton` | function | divwindo.c | Mouse button hit-test ("raton" = mouse) |
| `crear_menu` | function | divhandl.c | Create menu ("crear" = to create) |
| `pinta_menu` | function | divhandl.c | Draw/paint menu ("pintar" = to paint) |
| `actualiza_menu` | function | divhandl.c | Update menu ("actualizar" = to update) |

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

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `compilar` | function | divc.c:992 | Compile ("compilar" = to compile) |
| `lexico` | function | divc.c:1742 | Lexer / tokenizer ("lexico" = lexical) |
| `sintactico` | function | divc.c:3160 | Parser ("sintactico" = syntactic) |
| `sentencia` | function | divc.c:4853 | Statement parser ("sentencia" = statement/sentence) |
| `expresion` | function | divc.c:5265 | Expression parser |
| `condicion` | function | divc.c | Condition parser |
| `constante` | function | divc.c | Constant expression evaluator |
| `pieza` | `int` | divc.c:871 | Current token ("pieza" = piece/token) |
| `pieza_num` | `int` | divc.c:871 | Numeric value of current token |
| `linea` | `int` | divc.c:878 | Current line number ("linea" = line) |
| `objeto` | struct | divc.c:756 | Symbol table entry ("objeto" = object) |
| `bloque_actual` | `objeto*` | divc.c:873 | Current scope / process ("bloque" = block, "actual" = current) |
| `bloque_lexico` | `objeto*` | divc.c:874 | Lexical scope block |
| `miembro` / `member` | `objeto*` | divc.c:876 | Current struct member being parsed |
| `acceso_remoto` | `int` | divc.c:886 | Remote (cross-process) access flag |
| `numero_error` | `int` | divc.c:940 | Error number (-1 = no error) |
| `linea_error` | `int` | divc.c:941 | Error line |
| `columna_error` | `int` | divc.c:942 | Error column ("columna" = column) |
| `optimizar` | `int` | divc.c:725 | Optimization enabled flag |
| `comprueba_rango` | `int` | divc.c:721 | Range checking enabled ("comprobar" = to check, "rango" = range) |
| `hacer_strfix` | `int` | divc.c:724 | String auto-padding enabled |
| `inicio_sentencia` | function | divc.c | Mark start of statement ("inicio" = start) |
| `final_sentencia` | function | divc.c | Mark end of statement ("final" = end) |
| `grabar_sentencia` | function | divc.c | Record statement debug info ("grabar" = to record/save) |

## Runtime / VM

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `interprete` | function | i.c:709 | Interpreter main function |
| `nucleo_exec` | function | i.c:806 | Core execution loop ("nucleo" = nucleus/core) |
| `pila[2624]` | `int[]` | inter.h:493 | Execution stack ("pila" = stack/pile) |
| `reloj` | `int` | -- | Clock / timer ("reloj" = clock) |
| `procesos` | `int` | -- | Living process count ("procesos" = processes) |
| `ejecutar_programa` | `int` | -- | Execute program flag |
| `ultimo_reloj` | `int` | i.c | Last clock value ("ultimo" = last) |
| `volcado` | function | v.c | Blit framebuffer to screen |
| `restaura` / `restore` | function | v.c | Restore background from backup |
| `pinta_sprite` | function | s.c:864 | Paint/render a process's sprite ("pintar" = to paint) |
| `pinta_textos` | function | s.c:1739 | Render text objects |
| `elimina_proceso` | function | i.c | Kill/remove a process ("eliminar" = to eliminate) |

## File I/O / Formats

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `fichero` | -- | -- | File ("fichero" = file) |
| `archivo` | -- | -- | File / archive ("archivo" = file/archive) |
| `graba_MAP` | function | divforma.c | Save MAP file ("grabar" = to record/save) |
| `descomprime_MAP` | function | divforma.c | Decompress/load MAP ("descomprimir" = to decompress) |
| `es_MAP` | function | divforma.c | Is it a MAP file? ("es" = is) |
| `empaquetable` | `int` | divc.c | Packable (into executable) flag |
| `cargadac_*` | functions | divpalet.c | Load palette from file ("cargar" = to load) |
| `abrir` | -- | -- | Open ("abrir" = to open) |
| `guardar` | -- | -- | Save ("guardar" = to save/keep) |
| `cerrar` | -- | -- | Close ("cerrar" = to close) |
| `leer` | -- | -- | Read ("leer" = to read) |
| `grabar` | -- | -- | Write / record ("grabar" = to record) |

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

| Identifier | Type | Defined in | English meaning |
|---|---|---|---|
| `Interpretando` | `int` | global.h:328 | Returning from interpreter (1=yes, came back from running a program) |
| `siguiente_orden` | `int` | global.h:321 | Next window order number ("siguiente" = next, "orden" = order) |
| `modo_de_retorno` | `int` | global.h:322 | Return mode: 0=quit, 1=run, 3=test fail ("retorno" = return) |
| `primera_vez` | `int` | div.c:122 | First time running flag ("primera vez" = first time) |
| `compilemode` | `int` | global.h:331 | Command-line compiler mode |
| `CopiaDesktop` | `int` | global.h:330 | Desktop session save/restore enabled |
| `MustCreate` | `int` | global.h:319 | Must create window flag |
| `unidades[27]` | `char[]` | global.h:815 | Drive letters ("unidades" = drives/units) |
| `tipo[24]` | `ttipo` | global.h:808 | File type definitions ("tipo" = type) |
| `mascara[512]` | `char[]` | global.h:820 | File filter mask ("mascara" = mask) |

## Common Spanish Words in Identifiers

| Spanish | English | Example usage |
|---|---|---|
| `an` / `ancho` | width | `vga_an`, `map_an`, `v.an` |
| `al` / `alto` | height | `vga_al`, `map_al`, `v.al` |
| `buscar` | to search | `buscar_texto()` |
| `caja` | box | `modo_caja`, `actualiza_caja()` |
| `cerrar` | to close | `cierra_ventana()`, `p_cerrar` |
| `crear` | to create | `crear_menu()`, `crear_ghost()` |
| `dibujo` | drawing | -- |
| `entorno` | environment | `entorno()`, `salir_del_entorno` |
| `fondo` | background | `fondo_raton`, `fondo_edicion()` |
| `fuente` | font / source | `divfont.c` |
| `grande` | large | `grande.fon` (large UI font) |
| `guardar` | to save | `guardar_prg()`, `guardar_mapa()` |
| `inicializacion` | initialization | `inicializacion()` |
| `linea` | line | `linea`, `num_lineas` |
| `mapa` | map / image | `tmapa`, `map_an`, `map_al` |
| `nuevo` | new | `nueva_ventana()`, `nuevo_mapa()` |
| `paleta` | palette | `set_paleta()`, `dac` |
| `pantalla` | screen | -- |
| `pequeño` | small | `pequeno.fon` (small UI font) |
| `pieza` | piece / token | `pieza` (compiler token) |
| `pila` | stack | `pila[]` (execution stack) |
| `pintar` | to paint | `pinta_sprite()`, `pinta_menu()` |
| `plano` | plane / layer | `primer_plano` (foreground) |
| `ratón` | mouse | `fondo_raton`, `read_mouse()` |
| `reloj` | clock | `reloj`, `system_clock` |
| `sonido` | sound | `divsound.c` |
| `tapiz` | wallpaper | `tapiz`, `tapiz_an` |
| `tecla` | key | `tecla()` (poll keys) |
| `ventana` | window | `ventana[]`, `tventana` |
| `volcado` | dump / blit | `volcado()`, `volcado_parcial()` |

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
