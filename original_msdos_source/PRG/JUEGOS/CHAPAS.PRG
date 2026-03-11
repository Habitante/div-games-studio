
//-----------------------------------------------------------------------------
// TITULO:      WORLD CAPS CHAMPIONSHIP
// AUTOR:       MANUEL CABA¤AS VENERA
// FECHA:       1/6/97
//-----------------------------------------------------------------------------
PROGRAM chapas;

CONST

    // Limites para los lanzamientos
    poder_maximo=38;
    fuerza_minima=2;
    inclinacion_maxima=pi;
    inclinacion_minima=-pi;

    mapa_grafico=100;           // Gr fico del mapa del juego
    ancho_mapa=1134;            // Tama¤o del mapa
    alto_mapa=790;
    mapa_deteccion_choque=101;  // Mapa de durezas

    color_control_1=200;        // Colores especiales para el mapa de durezas
    color_control_2=156;
    color_control_3=115;
    color_meta=246;
    color_fuera=192;
    color_objetos=141;

    vueltas=1;              // Numero de vueltas
    velocidad_anterior=16;  // Velocidad de retorno a la posici¢n previa
    avance_maximo=7;        // M ximo 7 puntos en el lanzamiento
    ultimo_punto=48;        // Ultimo punto de control en el circuito

GLOBAL
    fichero_grafico;    // Fichero gr fico

    fuente1;            // Fuentes de letras
    fuente2;
    fuente3;
    fuente4;

    sonido1;            // Sonidos
    sonido2;
    sonido3;
    canal=-1;           // Canal de sonido. -1= no usado

    estado_programa=1;  // (1=Menu principal 2=Juego 3=Creditos 4=Fin)
    nuevo_estado=0;     // El siguiente estado del programa

    turno=1;            // Siguiente turno
    toques=1;           // Numero de toques
    id_chapa[1]=0,0;    // Identificador del proceso de la chapa
    posicion_inicial_x[1]=325,325;    // Posici¢n inicial en
    posicion_inicial_y[1]=691,659;    // la meta 0->1§, 1->2§
    num_jugadores=0;    // Numero de jugadores
    modo_juego;         // 1=Practica, 2 =Dos jugadores, 3= Jugador vs CPU

    id_camara;          // Identificador de la c mara
    camara_x=0;         // Posici¢n de la c mara
    camara_y=0;
    moviendose=FALSE;   // 1=Cuando el movimiento de pantalla esta activo

    id_proceso_direccion=0;     // Identificador del proceso direcci¢n
    id_proceso_inclinacion=0;   // Identificador del proceso inclinaci¢n
    id_proceso_fuerza=0;        // Identificador del proceso fuerza
    objetivo=0;                 // Identificador de la chapa
    contador0=0;                // Contador de uso general

LOCAL
    fuerza_ini;             // Contador de fuerza inicial
    x_resol=0;              // Posici¢n de la chapa x1000
    y_resol=0;
    mi_turno=0;             // Para saber cuando es nuestro turno
    id_pos_ant=0;           // Identificador del proceso de la posici¢n previa
    fuerza=fuerza_minima;   // Fuerza de lanzamiento
    direccion=0;            // Direcci¢n del lanzamiento(Plano XY)
    inclinacion=0;          // Inclinaci¢n del lanzamiento(Plano XZ)
    inc_inclinacion=0;      // Incremento de la inclinaci¢n cuando se mueve
    movimiento=0;           // Movimientos (0 = pausa, 1 = lanzamiento, 2 = colisi¢n, 3 & 4 = retorno)
    num_lanzamiento=1;      // Numero de lanzamientos
    vuelta_actual=0;        // Vuelta actual
    control[3]=FALSE,FALSE,FALSE;   // Para controlar las vueltas
    fuera=FALSE;            // TRUE= posici¢n incorrecta de la chapa
    colision_chapa=0;       // Se actualiza cuando colisionamos con algo
    animacion=17;           // Animaci¢n de la chapa(17..49)

BEGIN // Programa principal

    priority=1;             // Esto es para que sea lo primero que se ejecute

    // Carga las fuentes de las letras
    fuente1=load_fnt("chapas\titulo.fnt");
    fuente2=load_fnt("chapas\opcion.fnt");
    fuente3=load_fnt("chapas\juego.fnt");
    fuente4=load_fnt("chapas\creditos.fnt");

    // Carga los sonidos
    sonido1=load_pcm("chapas\tic7.pcm",1);       // Fuerza
    sonido2=load_pcm("chapas\metal4.pcm",0);     // Lanzamiento
    sonido3=load_pcm("chapas\metal10.pcm",0);    // Colisi¢n

    LOOP // Bucle del proceso principal

        // Dependiendo del estado del programa ejecuta una parte u otra
        SWITCH (estado_programa);
            CASE 1:             // Estado del programa en titulo (Men£)
                titulo();       // Proceso del tipo titulo que controla el men£
                timer[0]=0;     // Pone un contador de tiempo 0
            END
            CASE 2:             // Aqui es donde esta el juego en s¡
                juego();
            END
            CASE 3:
                creditos();     // Opci¢n de ver cr‚ditos
            END
            CASE 4:             // Opci¢n de salir del juego
                exit("Gracias por jugar!",0);
            END
            DEFAULT:
                // Si no esta en otro estado de programa de -1
                // y el tiempo pasado es mayor que 10 segundos
                // pone el estado del juego en cr‚ditos
                IF (estado_programa==-1 AND timer[0]>1000)
                    nuevo_estado=3;    // Cr‚ditos
                    timer[0]=0;
                END
            END
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso Titulo
// Opciones del men£ principal
//-----------------------------------------------------------------------------

PROCESS titulo();

BEGIN
    estado_programa=-1;     // Cambia el estado del programa a -1 para
                            // llamarse solo una vez
    nuevo_estado=0;
    set_mode(m640x480);     // Selecciona modo de v¡deo
    // Carga los gr ficos necesarios para el juego
    fichero_grafico=load_fpg("chapas\porchapa.fpg");
    put_screen(fichero_grafico,1);      // Pon la pantalla de fondo
    load_pal("chapas\porchapa.fpg");    // Carga la paleta de colores
    fade_on();

    // Opciones del men£ principal
    write(fuente2,160,220,3,"1 Practicar");
    write(fuente2,150,260,3,"2 Jugador contra Jugador");
    write(fuente2,150,300,3,"3 Jugador contra Ordenador");
    write(fuente2,150,380,3,"ESC Salir al DOS");
    write(fuente2,320,460,4,"(c) 1997 Hammer Technologies");
    write(fuente1,320,60,4,"WORLD CHAPAS");
    write(fuente1,320,120,4,"CHAMPIONSHIP");

    WHILE (nuevo_estado==0) // Repite hasta que se cambie de estado

        // Elige el modo de juego seg£n la tecla pulsada
        IF (key(_1))            // Tipo de juego eligido de Practica
            num_jugadores=1;
            modo_juego=1;
            nuevo_estado=2;
        END
        IF (key(_2))            // Tipo de juego eligido
            num_jugadores=2;    // Jugador contra Jugador
            modo_juego=2;
            nuevo_estado=2;
        END
        IF (key(_3))            // Tipo de juego eligido
            num_jugadores=2;    // Jugador contra ordenador
            modo_juego=3;
            nuevo_estado=2;
        END
        IF (key(_esc))          // Salir del programa
            nuevo_estado=4;
        END

        FRAME;

    END

    fade_off();                     // Apaga la pantalla

    unload_fpg(fichero_grafico);    // Descarga los gr ficos
    delete_text(all_text);          // Borra todos los textos de pantalla
    estado_programa=nuevo_estado;   // Cambia el programa al nuevo estado
END

//-----------------------------------------------------------------------------
//  Proceso Cr‚ditos
//  Muestra los cr‚ditos del juego World Caps Championship
//-----------------------------------------------------------------------------

PROCESS creditos();

PRIVATE
    // Identificadores de textos
    id_texto_titulo;
    id_texto_prog_1;
    id_texto_prog_2;
    id_texto_graf_1;
    id_texto_graf_2;
    id_texto_musi_1;
    id_texto_musi_2;
    id_copia_texto;

BEGIN
    estado_programa=-3;
    set_mode(m640x480);                 // Selecciona el modo de v¡deo
    fichero_grafico=load_fpg("chapas\creditos.fpg");    // Carga gr ficos
    put_screen(fichero_grafico,1);      // Pone pantalla de fondo
    load_pal("chapas\creditos.fpg");    // Carga paleta de colores
    fade_on();                          // Enciende la pantalla
    WHILE (fading)                      // Espera a que se encienda
        FRAME;
    END

    x=120;                              // Utiliza las coordenadas destinadas a gr ficos
    y=480;                              // para mover los textos

    // Pone los textos
    id_texto_titulo=write(fuente4,320,y,1,"WORLD CHAPAS CHAMPIONSHIP");
    id_texto_prog_1=write(fuente4,x,y+80,0,"Programaci¢n:");
    id_texto_prog_2=write(fuente4,x,y+120,0,"  Manuel Caba¤as Venera");
    id_texto_graf_1=write(fuente4,x,y+200,0,"Gr ficos:");
    id_texto_graf_2=write(fuente4,x,y+240,0,"  Pablo de la Sierra");
    id_texto_musi_1=write(fuente4,x,y+320,0,"Idea original:");
    id_texto_musi_2=write(fuente4,x,y+360,0,"  Luis F. Fernandez");
    id_copia_texto=write(fuente4,320,y+440,1,"(c) Daniel Navarro - DIV Games Studio ");
    scan_code=0;
    FRAME;

    REPEAT
        // Si se pulsa una tecla o han aparecido todos los textos, termina
        IF (scan_code<>0 OR y<-480) BREAK; END

        y-=2;   // Va moviendo el texto hacia arriba
        move_text(id_texto_titulo,320,y);
        move_text(id_texto_prog_1,x,y+80);
        move_text(id_texto_prog_2,x,y+120);
        move_text(id_texto_graf_1,x,y+200);
        move_text(id_texto_graf_2,x,y+240);
        move_text(id_texto_musi_1,x,y+320);
        move_text(id_texto_musi_2,x,y+360);
        move_text(id_copia_texto,320,y+440);
        FRAME;
    UNTIL (estado_programa==1)
    fade_off();
    // Borra los textos y descarga el fichero de gr ficos
    delete_text(all_text);
    unload_fpg(fichero_grafico);
    estado_programa=1;    // Retorna al men£ principal a estado de programa titulo
END

//-----------------------------------------------------------------------------
//  Proceso juego
//  Selecciona las variables de acuerdo al modo de juego
//-----------------------------------------------------------------------------

PROCESS juego();

BEGIN
    estado_programa=-2;             // Cambia el estado del programa para ejecutar este proceso solo una vez
    // Reinicia las varibles usadas en el juego
    nuevo_estado=0;
    turno=1;
    toques=1;
    objetivo=0;

    set_mode(m320x200);             // Selecciona el modo de v¡deo
    fichero_grafico=load_fpg("chapas\chapas.fpg");      // Carga el fichero de gr ficos

    // Inicia el movimiento de pantalla (scroll)
    start_scroll(0,fichero_grafico,mapa_grafico,0,0,3); // Plano 0, Regi¢n 0, todas las direcciones
    load_pal("chapas\chapas.fpg");  // Carga la paleta de colores correcta
    fade_on();                      // Enciende la pantalla

    moviendose=TRUE;        // La c mara no esta parada

    comienzo_juego();       // Comienza los procesos
    // De estos procesos coge los identificadores para uso futuro
    id_camara=camara_principal();
    id_proceso_direccion=direccion_chapa();
    id_proceso_inclinacion=inclinacion_chapa();
    id_proceso_fuerza=fuerza_chapa();

    // Mueve la camara hasta la primera chapa
    cambia_movimiento(id_chapa[0].x-160,id_chapa[0].y-100); // Cambia la c mara mirando hacia la primera chapa

    // Muestra los datos del juego
    write(fuente3,2,2,0,"Turno");
    write_int(fuente3,70,2,0,&turno);
    write(fuente3,2,22,0,"Tiros");
    write_int(fuente3,70,22,0,&toques);

    FRAME;
    LOOP
        // Comprueba si alguno ha llegado a la l¡nea de meta
        contador0=0;
        WHILE (contador0<num_jugadores)
            IF (id_chapa[contador0].vuelta_actual<vueltas)
                contador0++;
            ELSE
                BREAK;
            END
        END

        // Mira si se acabo el juego
        IF (contador0<num_jugadores)
            nuevo_estado=1;     // Vuelve al men£ principal
            BREAK;
        END

        // Mira si se pulso la tecla escape
        IF (key(_esc))
            nuevo_estado=1;    // Vuelve al men£ principal
            BREAK;
        END

        FRAME;

    END

    // Elimina todos los procesos
    signal(type chapa_jugador,s_kill_tree);
    signal(type rival,s_kill_tree);
    signal(id_camara,s_kill);
    signal(id_proceso_direccion,s_kill_tree);
    signal(id_proceso_inclinacion,s_kill_tree);
    signal(id_proceso_fuerza,s_kill_tree);

    stop_scroll(0);             // Para el movimiento de pantalla (scroll)
    fade_off();                 // Apaga la pantalla
    delete_text(all_text);      // Elimina todos los textos
    unload_fpg(fichero_grafico);// Descarga el fichero de gr ficos
    estado_programa=nuevo_estado;
END

//-----------------------------------------------------------------------------
//  Proceso comienzo_juego
//  Carga los valores iniciales
//-----------------------------------------------------------------------------

PROCESS comienzo_juego();

PRIVATE
    jugador;     // Jugador que empieza (0=jugador 1, 1=jugador 2 o CPU)

BEGIN

    // Elige que jugador empezara
    IF (num_jugadores==1)                       // Mira primero si solo juega un jugador
        id_chapa[0]=chapa_jugador(1);           // Si es solo uno, empieza ‚l
    ELSE                                        // Si no el n£mero de jugadores es 2
        jugador=rand(0,num_jugadores-1);        // Elige al azar quien empieza
        IF (jugador==0)                         // Si ha salido 0, empieza el primer jugador
            id_chapa[0]=chapa_jugador(1);       // Se asigna la primera chapa como primero
            IF (modo_juego==2)                  // Comprueba el modo de juego
                id_chapa[1]=chapa_jugador(2);   // El segundo es el otro jugador
            ELSE
                id_chapa[1]=rival(2);           // O si no es el segundo es el ordenador
            END
        ELSE                                    // Si no ha salido 1, empieza el segundo jugador
            IF (modo_juego==2)                  // Comprueba el modo de juego
                id_chapa[1]=chapa_jugador(1);   // El segundo en tirar es el primer jugador
            ELSE
                id_chapa[1]=rival(1);           // El segundo en tirar es el ordenador
            END
            id_chapa[0]=chapa_jugador(2);       // El primero en tirar es el segundo jugador
        END
    END
END

//-----------------------------------------------------------------------------
//  Proceso posicion_chequeo
//  Comprueba la posici¢n de la chapa
//-----------------------------------------------------------------------------

PROCESS posicion_chequeo(identificador);

PRIVATE
    color;  // Color del mapa de texturas

BEGIN

    // Coge el color del mapa de texturas
    color=map_get_pixel(fichero_grafico,mapa_deteccion_choque,(identificador.x)/2,(identificador.y)/2);

    // Realiza la acci¢n en consecuencia del color cogido
    SWITCH (color)
        CASE color_fuera:       // Ha salido fuera
            identificador.fuera=TRUE;
        END
        CASE color_objetos:
            identificador.fuera=TRUE;
        END                     // Ha dado con un objeto (= fuera)
        CASE color_control_1:
            // Comprueba que no ha salido fuera
            IF (NOT identificador.fuera)
                identificador.control[0]=TRUE;
            END
        END                     // Ha llegado al control zona 1
        CASE color_control_2:
            // Comprueba que no ha salido fuera y que paso por el control zona 1
            IF (NOT identificador.fuera AND identificador.control[0])
                identificador.control[1]=TRUE;
            END
        END                     // Ha llegado al control zona 2
        CASE color_control_3:
            // Comprueba que no ha salido fuera y que paso por el control zona 2
            IF (NOT identificador.fuera AND identificador.control[1])
                identificador.control[2]=TRUE;
            END
        END                     // Ha llegado al control zona 3
        CASE color_meta:
            // Comprueba que no ha salido fuera y que paso por el control zona 3
            IF (NOT identificador.fuera AND identificador.control[2])
                identificador.control[2]=identificador.control[1]=identificador.control[0]=FALSE;
                identificador.vuelta_actual++;
            END                 // Ha llegado a la zona de meta
        END
    END
END

//-----------------------------------------------------------------------------
//  Proceso calcula_movimiento
//  Calcula la posici¢n de la chapa dependiendo de los datos de la chapa
//-----------------------------------------------------------------------------

PROCESS calcula_movimiento();

PRIVATE
    angulo_anterior;     // Ultimo  ngulo
    distancia_anterior;  // Ultima distancia

BEGIN

    // Coge el tipo de movimiento del proceso que lo llamo
    SWITCH (father.movimiento);

        CASE 1,2:   // Lanzamiento o colisi¢n

            // Mueve la chapa 2 puntos
            father.x_resol+=get_distx(father.direccion,2000);
            father.y_resol+=get_disty(father.direccion,2000);
            // Comprueba si se ha salido o ha llegado a un punto de meta
            posicion_chequeo(father);
            // Actualiza las coordenadas seg£n los resultados
            father.x=father.x_resol/1000;
            father.y=father.y_resol/1000;
        END

        CASE 3,4:   // Retorna de lanzamiento erroneo
            // Coge las coordenadas del proceso que lo llamo (la chapa)
            x=father.x;
            y=father.y;
            // Coge el angulo y la distancia que han sido erroneos
            angulo_anterior=get_angle(father.id_pos_ant);
            distancia_anterior=get_dist(father.id_pos_ant);
            // Comprueba si ha llegado ya a su posici¢n anterior
            IF (distancia_anterior<velocidad_anterior)
                // Actualiza las coordenadas de la chapa
                father.x=father.id_pos_ant.x;
                father.y=father.id_pos_ant.y;
                // Quita el identificador de fuera y la direcci¢n
                father.fuera=FALSE;
                father.direccion=0;
                // Se cambia el turno
                IF (father.mi_turno==turno AND father.movimiento==3)
                    turno++;
                    IF (turno>num_jugadores) turno=1; END
                END
                father.movimiento=0;
                signal(father.id_pos_ant,s_kill);   // Quita la posici¢n anterior (erronea)
            ELSE
                // Si no mueve la chapa hasta su posici¢n anterior
                father.x+=get_distx(angulo_anterior,velocidad_anterior);
                father.y+=get_disty(angulo_anterior,velocidad_anterior);
            END
            // Actualiza coordenadas
            father.x_resol=father.x*1000;
            father.y_resol=father.y*1000;
        END
        DEFAULT:
        END
    END

    // Mira el limite izquierdo del mapa
    IF (father.x<10)
        father.movimiento=3;      // Retorna el movimiento (tiro)
        father.fuerza=0;
        father.x=10;
        father.x_resol=10000;
        father.direccion=0;
        father.inclinacion=0;
    END

    // Mira el limite derecho del mapa
    IF (father.x>ancho_mapa-10)
        father.movimiento=3;      // Retorna el movimiento (tiro)
        father.fuerza=0;
        father.x=ancho_mapa-10;
        father.x_resol=(ancho_mapa-10)*1000;
        father.direccion=0;
        father.inclinacion=0;
    END

    // Mira el limite superior de la pantalla
    IF (father.y<10)
        father.movimiento=3;      // Quita el ultimo movimiento
        father.fuerza=0;
        father.y=10;
        father.y_resol=10000;
        father.direccion=0;
        father.inclinacion=0;
    END

    // Comprueba el limite superior de la pantalla
    IF (father.y>alto_mapa-10)
        father.movimiento=3;      // Retorna el movimiento
        father.fuerza=0;
        father.y=alto_mapa-10;
        father.y_resol=(alto_mapa-10)*1000;
        father.direccion=0;
        father.inclinacion=0;
    END

END

//-----------------------------------------------------------------------------
//  Proceso cambia_movimiento
//  Realiza los movimientos de la c mara
//-----------------------------------------------------------------------------

PROCESS cambia_movimiento(x,y);

BEGIN

    // El movimiento esta limitado a la resoluci¢n
    // de la pantalla (320x200) y al tama¤o del mapa
    IF (x<0)                    // Comprueba el lado izquierdo
        x=0;
    ELSE                        // Comprueba el lado derecho
        IF (x>ancho_mapa-320)
            x=ancho_mapa-320;
        END
    END
    IF (y<0)                    // Comprueba el lado superior
        y=0;
    ELSE                        // Comprueba el lado inferior
        IF (y>alto_mapa-200)
            y=alto_mapa-200;
        END
    END

    // Actualiza la c mara dependiendo de las coordenadas de este proceso
    camara_x=x;
    camara_y=y;
END

//-----------------------------------------------------------------------------
//  Proceso direccion_chapa
//  Muestra la direcci¢n de la chapa
//-----------------------------------------------------------------------------

PROCESS direccion_chapa();

BEGIN
    file=fichero_grafico;   // Selecciona el fichero gr fico
    ctype=c_scroll;         // Introduce al proceso dentro del scroll
    // Cambia distinta variables predefinidas
    z=-1;
    priority=-1;
    size=50;

    LOOP
         // El indicador esta rotando alrededor de la chapa
        IF (objetivo<>0)

            // El objetivo es es identificador de la chapa
            // y el indicador coge sus coordenadas respecto a la misma
            x=objetivo.x+get_distx(objetivo.direccion,24);
            y=objetivo.y+get_disty(objetivo.direccion,24);

            // Coge el  ngulo respecto a la chapa
            angle=objetivo.direccion;

            // Cambia el gr fico dependiendo de la chapa
            IF (objetivo.mi_turno==1)
                graph=400;
            ELSE
                graph=401;
            END
        ELSE
            graph=0;        // Ning£n indicador es mostrado esta moviendose
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso inclinacion_fondo
//  Muestra el fondo de pantalla de la inclinaci¢n de la chapa
//-----------------------------------------------------------------------------

PROCESS inclinacion_fondo();

BEGIN
    file=fichero_grafico;
    graph=200;              // Elige el gr fico
    x=father.x;             // Coge las coordenadas del proceso que los llamo
    y=father.y;
    z=-2;
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso inclinacion_chapa
//  Muestra el marcador de la inclinaci¢n de la chapa
//-----------------------------------------------------------------------------

PROCESS inclinacion_chapa();

BEGIN
    file=fichero_grafico;
    graph=201;          // Elige el gr fico a usar
    x=281;              // Pone la coordenadas
    y=38;
    z=-3;               // Ponlo por encima del fondo
    priority=-1;
    inclinacion_fondo();// Crea el fondo del marcador

    LOOP
        // Comprueba si se esta tirando
        IF (objetivo<>0)
            // Cambia el  ngulo en relaci¢n a la inclinaci¢n elegida
            angle=(objetivo.inclinacion/2);
        ELSE
            // Se esta tirando, no eligieno inclinaci¢n
            angle=0;
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso fondo_fuerza
//  Muestra el gr fico de fondo de la fuerza de la chapa
//-----------------------------------------------------------------------------

PROCESS fondo_fuerza();

BEGIN
    file=fichero_grafico;
    graph=300;              // Se selecciona el gr ficos
    x=father.x;             // Coge las coordenadas del proceso que lo llamo
    y=father.y;
    z=-2;
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso fuerza_chapa
//  Muestra el indicador de fuerza de la chapa
//-----------------------------------------------------------------------------


PROCESS fuerza_chapa();

BEGIN
    file=fichero_grafico;
    graph=301;      // Selecciona el gr fico a usar
    x=281;          // Selecciona las coordenadas
    y=161;
    z=-3;           // Ponlo por encima del fondo que tiene
    priority=-1;
    fondo_fuerza(); // Crea el fondo del marcador
    LOOP
        IF (objetivo<>0)
            // Selecciona el  ngulo respecto a la fuerza
            angle=-((objetivo.fuerza-fuerza_minima)*3*pi)/(2*(poder_maximo-fuerza_minima));
        ELSE
            // Se esta tirando y no eligiendo la fuerza
            angle=0;
        END
        FRAME;
    END
END

//----------------------------------------------------------------------------
//  Proceso chapa_jugador
//  Maneja las acciones de la chapa
//----------------------------------------------------------------------------

PROCESS chapa_jugador(mi_turno);

PRIVATE
    // Para la barra de lanzamientos
    fuerza_cargada=FALSE;   // SPACE o ENTER esta pulsada
    fuerza_arriba=TRUE;     // Oscila entre MINIMO-MAXIMO (Arriba o Abajo)

BEGIN

    file=fichero_grafico;
    graph=17;               // Se selecciona el gr fico
    flags=0;
    // Coge las coordenadas de la tabla
    x=posicion_inicial_x[mi_turno-1];
    y=posicion_inicial_y[mi_turno-1];
    // Crea otras coordenadas pero multiplicadas por 1000
    x_resol=x*1000;
    y_resol=y*1000;
    ctype=c_scroll;           // Coordenadas absolutas sobre el mapa
    FRAME;

    LOOP

        // Lee las teclas si es el turno apropiado
        IF (turno==mi_turno AND movimiento==0)      // Prepara el lanzamiento
            objetivo=id;                            // Guarda en la variable objetivo el identificador
            WHILE (moviendose)                      // Repite hasta que acabe de moverse
                FRAME;
            END

            IF (key(_o) OR key(_left))      // Selecciona direcci¢n
                direccion=direccion+(pi/32);
            END
            IF (key(_p) OR key(_right))
                direccion=direccion-(pi/32);
            END
            IF (key(_a) OR key(_down))      // Selecciona inclinaci¢n
                inclinacion=inclinacion+pi/8;
                IF (inclinacion>inclinacion_maxima)
                    inclinacion=inclinacion_maxima;
                END
            END
            IF (key(_q) OR key(_up))
                inclinacion=inclinacion-pi/8;
                IF (inclinacion<inclinacion_minima)
                    inclinacion=inclinacion_minima;
                END
            END

            // Incrementa fuerza
            IF (key(_space) OR key(_enter))
                IF (fuerza==fuerza_minima AND canal==-1)
                    canal=sound(sonido1,256,492);
                END
                fuerza_cargada=TRUE;
                IF (fuerza_arriba)
                    fuerza=fuerza+2;
                    IF (fuerza>poder_maximo)
                        fuerza_arriba=FALSE;
                        fuerza=poder_maximo;
                    END
                ELSE
                    fuerza=fuerza-2;
                    IF (fuerza<fuerza_minima)
                        fuerza=fuerza_minima;
                        fuerza_arriba=TRUE;
                    END
                END
                change_sound(canal,256,512-fuerza*10);
            END

            // Cuando no esta pulsado ENTER ni SPACE y tiene fuerza, comienza el movimiento
            IF ((NOT key(_space) AND NOT key(_enter)) AND fuerza_cargada)
                stop_sound(canal);          // Detiene cualquier sonido
                canal=-1;                   // Y pone un sonido nuevo
                sound(sonido2,132+fuerza*30,700);
                id_pos_ant=punto(x,y);      // Graba la posici¢n actual
                num_lanzamiento++;          // Incrementa el contador de lanzamientos
                fuerza_cargada=FALSE;       // Reinicia las variables necesarias
                fuera=FALSE;
                movimiento=1;               // Esta moviendose
                fuerza_arriba=TRUE;
                inc_inclinacion=(-inclinacion*2)/fuerza;
                objetivo=0;                 // Indica que ya no esta eligiendo parametros de tiro
                fuerza_ini=fuerza;
            END
            toques=num_lanzamiento;         // Para mostrarlo en la pantalla
        END

        IF (movimiento<>0)
            animacion=66-animacion;         // Anima la chapa
            SWITCH (movimiento)
                CASE 1,2:                   // Tiro o colisi¢n
                    inclinacion+=inc_inclinacion;
                    direccion+=inc_inclinacion;

                    // Lanzamos de 2 en 2 pixels (puntos)
                    FOR (contador0=0;contador0<fuerza;contador0+=2)
                        calcula_movimiento();

                        // Comprueba colisiones
                        IF(fuera AND fuerza==fuerza_ini)
                            fuera=FALSE;    // Ya no esta fuera
                        END
                        IF (movimiento==1 AND modo_juego==3 AND (colision_chapa=collision(TYPE rival)))
                            impacto();      // Ha chocado contra la chapa del ordenador
                        END
                        IF (modo_juego==2)
                            WHILE ((colision_chapa=collision(TYPE chapa_jugador)))
                                // Ha chocado con el otro jugador
                                IF (colision_chapa<>id) impacto(); END
                            END
                        END
                    END
                    fuerza-=2;
                    // Decrementa fuerza y comprueba si ha acabado
                    IF (fuerza<=0)
                        fuerza=fuerza_minima;
                        IF (fuera)
                            IF (movimiento<3) // Movimiento anterior (retorna movimiento)
                                movimiento+=2;
                            END
                        ELSE
                            IF (turno==mi_turno AND movimiento==1)  // Cambia el turno
                                turno++;
                                IF (turno>num_jugadores)
                                    turno=1;
                                END
                            END
                            signal(id_pos_ant,s_kill);
                            movimiento=0;
                        END
                    END
                END
                CASE 3:
                    calcula_movimiento();   // Antes del lanzamiento
                END
                CASE 4:
                    calcula_movimiento();   // Antes de la colisi¢n
                    IF (modo_juego==3 AND (colision_chapa=collision(TYPE rival)))
                        IF (colision_chapa.movimiento<>3)
                            impacto();      // Ha chocado contra el ordenador
                        END
                    END
                    IF (modo_juego==2)
                        WHILE ((colision_chapa=collision(TYPE chapa_jugador)))
                            IF (colision_chapa<>id AND colision_chapa.movimiento<>3)
                                impacto();  // Ha chocado contra la otra chapa
                            END
                        END
                    END
                END
            END
        END

        // Limita la direcci¢n entre PI y -PI
        IF (direccion>pi)
            direccion-=2*pi;
        END
        IF (direccion<-pi)
            direccion+=2*pi;
        END
        // Calcula el gr fico de la chapa de acuerdo al  ngulo y la animaci¢n
        cambia_grafico(id);

        IF (turno==mi_turno)
            IF (movimiento==0)
                camara();       // C mara el¡ptica de selecci¢n
            ELSE
                camara_2();     // C mara que sigue a la chapa
            END
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso impacto
//  Acciones de colisi¢n
//-----------------------------------------------------------------------------

PROCESS impacto();

PRIVATE
     ngulo0;    // Guarda  ngulos entre chapas

BEGIN
    // Hace el sonido de choque
    sound(sonido3,512,512);

    // Actualiza las coordenadas con el proceso que lo llamo
    x=father.x;
    y=father.y;

    // Coge los datos de la chapa con la que ha chocado
    father.colision_chapa.id_pos_ant=punto(father.colision_chapa.x,father.colision_chapa.y);
     ngulo0=get_angle(father.colision_chapa);            // Separa las chapas

    // Halla las nuevas coordenadas de la chapa y las actualiza
    father.colision_chapa.x=x+get_distx( ngulo0,28);
    father.colision_chapa.x_resol=father.colision_chapa.x*1000;
    father.colision_chapa.y=y+get_disty( ngulo0,28);
    father.colision_chapa.y_resol=father.colision_chapa.y*1000;
    father.colision_chapa.fuera=FALSE;

    // Comprueba que no  halla chocado, ni llegado a la meta
    posicion_chequeo(father.colision_chapa);

    // Si todavia le queda fuerza a la chapa que choco
    IF (father.fuerza>fuerza_minima)
        // Actualiza los datos de la chapa que choco
        father.colision_chapa.fuerza=father.fuerza/2;
        father.colision_chapa.direccion= ngulo0;
        father.colision_chapa.movimiento=2;            // Impacto
        father.colision_chapa.inclinacion=0;
        father.colision_chapa.inc_inclinacion=0;

        // Modifica los datos de la chapa que choco
        father.fuerza-=(father.fuerza/2);
        father.inc_inclinacion=(-father.inclinacion*2)/father.fuerza;
        father.direccion-=( ngulo0-father.direccion);
    ELSE
        IF (NOT father.colision_chapa.fuera)
            signal(father.colision_chapa.id_pos_ant,s_kill);     // Elimina la posici¢n anterior
        ELSE
            father.colision_chapa.movimiento=4;        // Deja el movimiento en colisi¢n
        END
    END
END

//-----------------------------------------------------------------------------
//  Proceso cambia_grafico
//  Elige y pone el gr fico de la chapa
//-----------------------------------------------------------------------------

PROCESS cambia_grafico(identificador);

BEGIN

    // Cambia el gr fico dependiendo de la inclinacion
    identificador.graph=(identificador.inclinacion)*8/pi;
    IF (identificador.graph<-16)
        identificador.graph=-16;
    END
    IF (identificador.graph>15)
        identificador.graph=15;
    END
    // Si la inclinacion es 0 la direcci¢n no es usada (graph=0) pero se usa en la animaci¢n
    IF ((identificador.graph)<>0)
        identificador.angle=identificador.direccion;
        identificador.graph+=17;
    ELSE
        identificador.angle=0;
        identificador.graph+=identificador.animacion;
    END
END

//-----------------------------------------------------------------------------
//  Proceso camara
//  Coloca la c mara
//-----------------------------------------------------------------------------

PROCESS camara();

BEGIN
    // Coge las coordenadas del proceso que lo llamo en forma circular
    x=father.x+get_distx(father.direccion,140)-160;
    y=father.y+get_disty(father.direccion,80)-100;
    // Cuadra la c mara con los bordes de pantalla
    cambia_movimiento(x,y);
END

//-----------------------------------------------------------------------------
//  Proceso camara_2
//  Coloca la c mara
//-----------------------------------------------------------------------------

PROCESS camara_2();

PRIVATE
    inc_x;  // Distancia x entre la c mara y la chapa
    inc_y;  // Distancia y entre la c mara y la chapa

BEGIN
    // Coge los incrementos horizontales y verticales
    inc_x=father.x-scroll.x0;
    inc_y=father.y-scroll.y0;

    // Comprueba el movimiento horizontal
    IF (inc_x>240 AND (father.direccion>-pi/2 OR father.direccion<pi/2 OR father.movimiento==3))
        // Movimiento a la derecha
        inc_x-=240;
    ELSE
        IF (inc_x<80 AND (father.direccion<-pi/2 OR father.direccion>pi/2 OR father.movimiento==3))
            // Movimiento a la izquierda
            inc_x-=80;
        ELSE
            inc_x=0;    // Si da con un borde no hay movimientos
        END
    END

    // Comprueba el movimiento vertical

    IF (inc_y>120 AND (father.direccion>-pi OR father.direccion<0 OR father.movimiento==3))
        // Movimiento hacia abajo
        inc_y-=120;
    ELSE
        IF (inc_y<80 AND (father.direccion>0 OR father.direccion<pi OR father.movimiento==3))
        // Movimiento de pantalla hacia arriba
            inc_y-=80;
        ELSE
            inc_y=0;    // Si da con el borde no hay movimientos
        END
    END

    // Limita el movimiento al tama¤o del mapa
    IF (scroll.x0+inc_x>ancho_mapa-320)
        inc_x=ancho_mapa-320-scroll.x0;
    END
    IF (scroll.x0+inc_x<0)
        inc_x=-scroll.x0;
    END
    IF (scroll.y0+inc_y>alto_mapa-200)
        inc_y=alto_mapa-200-scroll.y0;
    END
    IF (scroll.y0+inc_y<0)
        inc_y=-scroll.y0;
    END
    // Cambia el movimiento
    cambia_movimiento(scroll.x0+inc_x,scroll.y0+inc_y);
END

//-----------------------------------------------------------------------------
//  Proceso punto_control
//  Esto es usado para calcular distancia, y guarda puntos de referencia
//-----------------------------------------------------------------------------

PROCESS punto_control(punto_n);

BEGIN
    // Coge las coordenadas del punto de control del mapa
    get_point(fichero_grafico,mapa_grafico,punto_n,&x,&y);
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso rival
//  Controla la chapa de la CPU
//-----------------------------------------------------------------------------
PROCESS rival(mi_turno);

PRIVATE
    temporal_x;         // Crea variables temporales para simular el movimiento
    temporal_y;
    temporal_vuelta;
    temporal_control[2];
    temporal_fuerza;
    temporal_direccion;
    id_punto_control;
    punto_actual=ultimo_punto;
    punto_avance=0;     // Puntos de lanzamiento actual
    encabeza=1;         // Mira detr s
    distancia0;
    distancia1;
    distancia2;
    falta=FALSE;        // Lanzamiento fallido

BEGIN
    file=fichero_grafico;
    graph=17;           // Selecciona el gr fico
    ctype=c_scroll;     // Introduce al proceso dentro del scroll
    // Selecciona la posici¢n inicial seg£n el turno
    x=posicion_inicial_x[mi_turno-1];
    y=posicion_inicial_y[mi_turno-1];
    x_resol=x*1000;
    y_resol=y*1000;          // Guarda las coordenadas multiplicadas por 1000

    LOOP
        IF (turno==mi_turno AND movimiento==0)
            cambia_movimiento(x-160,y-100);
            toques=num_lanzamiento;
            FRAME;          // Actualiza la c mara
            WHILE (moviendose)
                FRAME;
            END

            // Calcula el punto de control mas cercano a la chapa
            // Un punto que este delante del punto actual
            IF (punto_actual>1)
                id_punto_control=punto_control(punto_actual-1);
            ELSE
                id_punto_control=punto_control(ultimo_punto);
            END
            distancia0=get_dist(id_punto_control);
            signal(id_punto_control,s_kill);

            // Punto presente
            id_punto_control=punto_control(punto_actual);
            distancia1=get_dist(id_punto_control);
            signal(id_punto_control,s_kill);

            // Un punto delante del punto actual
            IF (punto_actual<ultimo_punto)
                id_punto_control=punto_control(punto_actual+1);
            ELSE
                id_punto_control=punto_control(1);
            END
            distancia2=get_dist(id_punto_control);
            signal(id_punto_control,s_kill);
            encabeza=1;

            WHILE (encabeza<>0)
                IF (encabeza==1)           // Delante
                    punto_actual++;
                    IF (punto_actual>ultimo_punto)
                        punto_actual=1;
                    END
                    distancia0=distancia1;
                    distancia1=distancia2;

                    // Un punto despu‚s del punto actual
                    IF (punto_actual<ultimo_punto)
                        id_punto_control=punto_control(punto_actual+1);
                    ELSE
                        id_punto_control=punto_control(1);
                    END
                    distancia2=get_dist(id_punto_control);
                    signal(id_punto_control,s_kill);

                ELSE                    // Detr s
                    punto_actual--;
                    IF (punto_actual<1)
                        punto_actual=ultimo_punto;
                    END
                    distancia2=distancia1;
                    distancia1=distancia0;

                    // Un punto despu‚s del punto actual
                    IF (punto_actual>1)
                        id_punto_control=punto_control(punto_actual-1);
                    ELSE
                        id_punto_control=punto_control(ultimo_punto);
                    END
                    distancia0=get_dist(id_punto_control);
                    signal(id_punto_control,s_kill);
                END

                // Comprueba que puede seguir o que ha llegado a meta
                IF (encabeza==1)        // Avanzando
                    IF (distancia0<distancia2)
                        punto_actual--;
                        IF (punto_actual<1)
                            punto_actual=ultimo_punto;
                        END
                        encabeza=0;     // Hemos terminado
                    END
                ELSE                    // Atrasando
                    IF (distancia0>distancia2)
                        punto_actual++;
                        IF (punto_actual>ultimo_punto)
                            punto_actual=1;
                        END
                        encabeza=0;     // Hemos terminado
                    END
                END
            END
            // Si ese punto se puede tirar, lo elige como punto maximo
            IF (NOT falta)
                punto_avance=avance_maximo;
            END

            REPEAT
                FRAME;
                // Haya el identificador del punto a donde tirar
                IF ((punto_actual+punto_avance)>ultimo_punto)
                    id_punto_control=punto_control(punto_actual+punto_avance-ultimo_punto);
                ELSE
                    id_punto_control=punto_control(punto_actual+punto_avance);
                END

                // Calcula la distancia
                distancia0=get_dist(id_punto_control);

                // Calcula la direcci¢n
                direccion=get_angle(id_punto_control);
                signal(id_punto_control,s_kill);

                // Calcula la fuerza
                fuerza=fuerza_minima;
                WHILE (distancia0>((fuerza+2)*fuerza/4))
                    fuerza+=2;
                    IF (fuerza>poder_maximo)
                        BREAK;
                    END
                END

                IF (fuerza<=poder_maximo)   // Simula el lanzamiento
                    movimiento=1;
                    fuera=FALSE;
                    temporal_x=x;
                    temporal_y=y;
                    temporal_vuelta=vuelta_actual;
                    temporal_control[0]=control[0];
                    temporal_control[1]=control[1];
                    temporal_control[2]=control[2];
                    temporal_fuerza=fuerza;
                    temporal_direccion=direccion;

                    // Simula el lanzamiento
                    WHILE (movimiento==1)
                        inclinacion+=inc_inclinacion;
                        direccion+=inc_inclinacion;

                        // Lanzamos de 2 en 2 pixels(puntos)
                        FOR (contador0=0;contador0<fuerza;contador0+=2)
                            calcula_movimiento();
                            IF (fuera)
                                movimiento=0;   // Movimiento invalido. Para la simulaci¢n
                                BREAK;
                            END
                        END

                        fuerza-=2;          // Decrementa la fuerza
                        IF (fuerza<=0)
                            fuerza=fuerza_minima;
                            movimiento=0;   // Fin del movimiento
                        END
                    END
                    // Actualiza los datos de la chapa
                    x=temporal_x;
                    y=temporal_y;
                    x_resol=x*1000;
                    y_resol=y*1000;
                    vuelta_actual=temporal_vuelta;
                    control[0]=temporal_control[0];
                    control[1]=temporal_control[1];
                    control[2]=temporal_control[2];
                ELSE
                    fuera=TRUE;             // Lanzamiento invalido (mucha fuerza)
                END
                signal(id_punto_control,s_kill);
                IF (fuera) punto_avance--; END
            UNTIL (fuera==FALSE OR punto_avance==0)

            FRAME;
            // Incrementa el n£mero de tiros
            num_lanzamiento++;
            // Reinicia variables
            fuerza=temporal_fuerza;
            direccion=temporal_direccion;
            inclinacion=0;
            movimiento=1;
            id_pos_ant=punto(x,y);              // Recoge la posici¢n actual
            fuera=FALSE;
            falta=FALSE;
            toques=num_lanzamiento;
            sound(sonido2,132+fuerza*30,700);   // Sonido de lanzamiento
        END

        IF (movimiento<>0)
            animacion=66-animacion;             // Anima la chapa
            SWITCH (movimiento);

                CASE 1,2:
                    inclinacion+=inc_inclinacion;   // Lanzamiento o colisi¢n
                    direccion+=inc_inclinacion;

                    // Lanzamos de 2 en 2 pixels(puntos)
                    FOR (contador0=0;contador0<fuerza;contador0+=2)
                        calcula_movimiento();

                        // Comprueba colisiones
                        IF (movimiento==1)
                            IF ((colision_chapa=collision(TYPE chapa_jugador)))
                                impacto();
                            END
                        END
                    END

                    fuerza-=2;       // Decrementa la fuerza
                    IF (fuerza<=0)   // hasta que se para
                        fuerza=fuerza_minima;

                        IF (fuera)    // Vuelve al movimiento anterior
                            IF (movimiento<3) movimiento+=2; END
                        ELSE
                            IF (turno==mi_turno AND movimiento==1)
                                turno++;
                                IF (turno>num_jugadores) turno=1; END
                            END
                            signal(id_pos_ant,s_kill); // Elimina la posici¢n anterior
                            movimiento=0;
                        END
                    END
                END

                CASE 3:
                    calcula_movimiento(); // Vuelve al lanzamiento anterior
                    falta=TRUE;
                END

                CASE 4:
                    calcula_movimiento();
                    IF ((colision_chapa=collision(TYPE chapa_jugador)))
                        impacto();       // Ha chocado con otra chapa
                    END
                END
            END

        END

        // Calcula el gr fico de la chapa
        cambia_grafico(id);
        // Selecciona la camara que sigue a la chapa
        IF (turno==mi_turno AND movimiento<>0) camara_2(); END
        FRAME;
    END
END

//---------------------------------------------------------------------
// Proceso punto
// Es usado para guarda puntos y hallar distancia
//---------------------------------------------------------------------

PROCESS punto(x,y);

BEGIN
    LOOP        // Graba punto en x,y
        FRAME;  // No se cambia por fget... por el uso de variables que guardan identificadores
    END
END

//-----------------------------------------------------------------------------
//  Proceso camara_principal
//  Controla el movimiento del juego
//-----------------------------------------------------------------------------

PROCESS camara_principal();

PRIVATE

    distancia;      // Guarda la distancia de donde esta a donde deberia estar
    inc_x;          // Incremento en el movimiento horizontal y vertical
    inc_y;
    angulo_avance;  // Guarda el  ngulo de avance de la c mara

BEGIN
    priority=-2;    // Es el ultimo proceso en ejecutarse

    scroll.x0=0;    // Pone las coordenadas a cero la primera vez
    scroll.y0=0;

    LOOP
        x=scroll.x0;// Coge las coordenadas de donde esta ahora la c mara
        y=scroll.y0;

            // Calcula la distancia a donde deber¡a estar
            distancia=fget_dist(x,y,camara_x,camara_y);

            // Si cerca no se mueve sino que se actualiza directamente
            IF (distancia<=20)
                scroll.x0=camara_x;
                scroll.y0=camara_y;
                moviendose=0;
            ELSE    // Si no se mueve la camara en la direcci¢n correcta
                moviendose=1;
                angulo_avance=fget_angle(x,y,camara_x,camara_y);
                inc_x=get_distx(angulo_avance,20);
                inc_y=get_disty(angulo_avance,20);
                scroll.x0+=inc_x;
                scroll.y0+=inc_y;
            END
        FRAME;

    END
END



