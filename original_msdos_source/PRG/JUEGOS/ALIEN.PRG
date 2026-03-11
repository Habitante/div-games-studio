//-----------------------------------------------------------------------------
//TITULO:      ALIEN-SUPRIMER
//AUTOR:       DANIEL MU¤OZ SANTI¤O
//FECHA:       2/10/97
//-----------------------------------------------------------------------------

PROGRAM alien_suprimer;

CONST
    pi_cuartos=PI/4;
    pi_octavos=PI/8;
    pi_medios=PI/2;

GLOBAL
    enemigo_f;              // Si es el enemigo final entonces enemigo_f=0
    puntuacion;             // Puntuaci¢n

    // Identificadores de sonido
    s_alien;                // Voz que dice "ALIEN SUPRIMER"
    s_disparo;              // Disparo del robot
    s_bip;                  // Sonido del cursor, en el men£ principal
    s_explosion;            // Explosi¢n 1
    s_explosion2;           // Explosi¢n 2
    s_disparo_t;            // Disparo del tanque
    s_motor;                // Motor del tanque
    s_pum;                  // Efecto del titulo

    seleccion=0;            // Selecci¢n en el men£ principal
    fichero1;               // Identificador al fichero de gr ficos
    fichero2;               // Identificador al fichero de gr ficos de men£ principal
    fuente_s;               // Identificador a la fuente de letras para vidas y puntuaci¢n
    fuente_m;               // Fuente del men£ principal
    fuente_j;               // Fuente alfab‚tica del juego
    fuente_c;               // Fuente alfab‚tica de la pantalla de cr‚ditos
    fuente_d;               // Fuente alfab‚tica de la pantalla de carga de datos.
    id_menu;                // Identificador del proceso principal
    id_robot;               // Identificador al proceso de piernas del robot
    id_c_robot;             // Identificador al proceso del cuerpo del robot
    id_misiles;             // Identificador al proceso de misiles
    contador_misiles;       // Numero de misiles
    tabla_misiles[30];      // Identificador a los gr ficos de los misiles del marcador
    misiles_libres;         // Siguiente misil libre
    energ¡a_robot;          // energ¡a del robot
    max_x=319;              // Mayor valor x
    max_y=199;              // Mayor valor y;
    nomover;                // Si es cero la pantalla se puede mover.
    freq_t_d;               // Frecuencia de disparo del tanque
    freq_h_d;               // Frecuencia de disparo del helic¢ptero
    esta_muerto=0;          // Variable para controlar el final del juego

LOCAL
    numero_grafico;         // Numero de los gr ficos
    se_mueve;               // Bandera. 1=robot esta movi‚ndose
    x_resol;                // Coordenada x con mas resoluci¢n
    y_resol;                // Coordenada y con mas resoluci¢n
    velocidad;              // Velocidad de uso general
     ngulo0;                // angulo de uso general
    energ¡a;                // energ¡a que queda
    _sonido;                // Bandera. 1=Esta sonando alg£n sonido

BEGIN
    // Ejecuta la animaci¢n inicial
    start_fli("alien\intro.fli",0,0);
    // Repite la animaci¢n hasta que acabe o se pulse una tecla
    WHILE (frame_fli()<>0 AND scan_code==0 AND NOT mouse.left)
        FRAME;
    END
    // Descarga la animaci¢n y apaga la pantalla
    end_fli();
    fade_off();
    id_menu=id;                                 // Coge el identificador del proceso general

    fichero2=load_fpg("alien\suprimer.fpg");    // Carga el fichero de los gr ficos de men£ principal
    fuente_d=load_fnt("alien\aliend.fnt");      // Carga las letras de la pantalla de carga de datos
    load_pal("alien\suprimer.fpg");             // Carga la paleta de colores

    // Pone la pantalla de fondo y el texto necesario
    put_screen(fichero2,2);
    write (fuente_d,160,100,4,"CARGANDO DATOS");
    fade_on();
    // Espera hasta que se encienda la pantalla
    WHILE (fading)
        FRAME;
    END

    // Carga el fichero de gr ficos del juego
    fichero1=load_fpg("alien\alien.fpg");

    // Carga los fuentes con las letras
    fuente_m=load_fnt("alien\alienm.fnt");
    fuente_s=load_fnt("alien\aliens.fnt");
    fuente_j=load_fnt("alien\alienj.fnt");
    fuente_c=load_fnt("alien\alienc.fnt");

    // Carga los sonidos y coge su identificador
    s_disparo=load_pcm("alien\laser6.pcm",0);
    s_disparo_t=load_pcm("alien\esco_at1.pcm",0);
    s_pum=load_pcm("alien\golpe20.pcm",0);
    s_alien=load_pcm("alien\alien.pcm",0);
    s_bip=load_pcm("alien\bip.pcm",0);
    s_explosion=load_pcm("alien\explosio.pcm",0);
    s_explosion2=load_pcm("alien\explosi8.pcm",0);
    s_motor=load_pcm("alien\motor.pcm",1);

    delete_text(all_text);                  // Quita los textos

    intro();                                // Y ejecuta la intro
    FRAME;

    // Menu principal
    // Repite hasta que se selecciona salir
    WHILE (seleccion<>2)

        load_pal("alien\suprimer.fpg");     // Cambia la paleta

        set_mode(m640x480);                 // Selecciona el modo de v¡deo

        // Pone los gr ficos y los mensajes
        put(fichero2,3,320,240);
        put(fichero2,15,320,120);
        write(fuente_m,320,320,4,"JUGAR");
        write(fuente_m,320,380,4,"QUITAR");
        fade_on();

        // Crea el proceso del cursor de selecci¢n de men£
        cursor();
        WHILE (seleccion==0)                // Espera la selecci¢n
            FRAME;
        END
        fade_off();

        IF (seleccion==1)                   // Haz la selecci¢n
            delete_text(all_text);          // Iniciar el juego
            set_mode(m320x200);             // Selecciona el modo de v¡deo
            load_pal("alien\alien.fpg");

            // Inicializa el movimiento de pantalla(scroll)
            // y las variables de coordenadas del mismo
            start_scroll(0,fichero1,1,0,0,0);
            scroll.x0=20;
            scroll.y0=2700;

            pausa();    // Crea el proceso para que halla pausa

            // Reinicia las variables necesarias
            nomover=0;
            enemigo_f=1;
            contador_misiles=6;
            misiles_libres=1;
            freq_t_d=18;
            freq_h_d=40;
            graph=0;
            puntuacion=0;
            // Escribe el texto de puntuacion y enciende la pantalla
            write_int(fuente_s,320,0,2,&puntuacion);
            fade_on();

            // Crea los procesos del robot, del lanzador de enemigos
            // y de los misiles
            id_robot=piernas_robot();
            lanzador_enemigos();
            id_misiles=misil();

            // Repite hasta que se pulsa escape o esta el jugador muerto
            WHILE (NOT key(_esc) & esta_muerto==0)  // Bucle general
                FRAME;
            END

            // Apaga la pantalla y reinicia todo para la siguiente partida
            fade_off();
            esta_muerto=0;
            seleccion=0;
            stop_sound(all_sound);
            delete_text(all_text);
            let_me_alone();
        END
    END
    let_me_alone(); // Seleccionado, el quitar el juego.
    creditos();     // Proceso que pone los creditos
END

//-----------------------------------------------------------------------------
// Proceso creditos
// Muestra los cr‚ditos
//-----------------------------------------------------------------------------

PROCESS creditos();

PRIVATE
    mis_letras;     // Identificador para fuentes de letras
    mi_fichero;     // Identificador para gr ficos

BEGIN
    // Borra todos los textos y la pantalla
    fade_off();
    delete_text(all_text);

    // Carga los ficheros necesariso para los cr‚dicos
    mi_fichero=load_fpg("alien\creditos.fpg");
    mis_letras=load_fnt("alien\alienc.fnt");
    load_pal("alien\creditos.fpg");

    // Pone el fondo de pantalla y los textos necesarios
    put_screen(mi_fichero,1);
    write(mis_letras,320,0,1,"ALIEN SUPRIMER");
    write(mis_letras,320,80,1,"Programado por:");
    write(mis_letras,320,120,1,"Daniel Mu¤oz Santi¤o");
    write(mis_letras,320,180,1,"Graficos by:");
    write(mis_letras,320,220,1,"J. Ricardo Abella");
    write(mis_letras,320,260,1,"Eva Astorga");
    write(mis_letras,320,320,1,"Sonidos:");
    write(mis_letras,320,360,1,"Carlos Illana Alejandro");
    fade_on();

    // Repite hasta que no este pulsada ninguna una tecla
    scan_code=0;
    WHILE (scan_code<>0)
        FRAME;
    END
    // Repite hasta que se pulse una tecla
    WHILE (scan_code==0)
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso robot
// pone el cuerpo del robot y controla los disparos
//-----------------------------------------------------------------------------

PROCESS robot();

PRIVATE
    grafico_disparo=0;          // Cambia los gr ficos seg£n disparo
    puedo_disparar=1;           // Bandera. 1=disparo avaible
    puedo_disparar_misiles=1;   // Bandera. 1=misiles avaible
    id_sonido_disparo=0;        // Identificador de sonido

BEGIN
    ctype=c_scroll;     // Mete al proceso dentro del scroll
    z=-5;
    priority=10;
    file=fichero1;
    LOOP
        // Controla los misiles
        IF (key(_alt) AND contador_misiles>0)
            IF (puedo_disparar_misiles)
                grafico_disparo=1;
                puedo_disparar_misiles=0;
                disparo_misiles();
                contador_misiles--;
            END
        ELSE
            puedo_disparar_misiles=1;
        END
        // Control de disparos
        IF (key(_control))
            IF (puedo_disparar)

                // Para cualqueir sonido de disparo que hubiera anteriormente
                stop_sound(id_sonido_disparo);

                // Pone las variables para no tener disparo continuo
                grafico_disparo=1;
                puedo_disparar=0;

                // Crea los dos disparos
                disparo_robot(x-18,y-21);
                disparo_robot(x+19,y-21);

                // Y hace el sonido del mismo
                id_sonido_disparo=sound(s_disparo,100,256);
            END
        ELSE
            // Si no esta disparando, pone la variable que permite disparar
            puedo_disparar=1;
        END

        // Pone el gr fico dependiendo si se dispara o no
        graph=3+grafico_disparo;
        grafico_disparo=0;
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso piernas_robot
// Maneja todas las acciones del robot
//-----------------------------------------------------------------------------

PROCESS piernas_robot();

PRIVATE
    contador_pasos=0;   // Controla los gr ficos
    siguiente_x_mapa;   // Siguiente coordenada x en el mapa
    siguiente_y_mapa;   // Siguiente coordenada y en el mapa
    obstaculo;          // Bandera
    contador0;          // Contador de uso general
    vidas;              // Vidas restantes
    x_mapa;             // Coordenada X en el mapa
    y_mapa;             // Coordenada Y en el mapa
    incremento_x;       // Incremento X
    incremento_y;       // Incremento Y

BEGIN
    ctype=c_scroll;                     // Introduce al proceso dentro del scroll
    priority=15;
    file=fichero1;
    vidas=3;                            // Crea el gr fico de las vidas
    vida();

    write_int(fuente_s,30,0,0,&vidas);  // Muestra las vidas
    energ¡a_robot=8000;                 // Inicializa la energ¡a del robot
    barra_energ¡a();                    // Muestra la barra de energ¡a

    // Repite hasta que se quede sin vidas
    WHILE (vidas>0)
        x_mapa=scroll.x0+160;           // Inicializa coordenadas
        y_mapa=scroll.y0+180;
        incremento_x=0;                 // Variables temporales para el incremento
        incremento_y=0;
        numero_grafico=0;
        id_c_robot=robot();             // Identificador para eliminar
        REPEAT

            // Reinicializa las variables de incremento
            incremento_x=0;
            incremento_y=0;

            // Control de teclas
            IF (key(_up)) incremento_y=-5; END
            IF (key(_down)) incremento_y=5; END
            IF (key(_left)) incremento_x=-5; END
            IF (key(_right)) incremento_x=5; END

            se_mueve=1;

            SWITCH ((incremento_y*10)+incremento_x) // Selecciona el gr fico de las piernas
                CASE 0: se_mueve=0; END
                CASE -5: numero_grafico=2; END      // Para la izquierda
                CASE 5:  numero_grafico=6; END      // Para la derecha
                CASE -50:numero_grafico=4; END      // Para la arriba
                CASE 50: numero_grafico=0; END      // Para la abajo
                CASE -55:numero_grafico=3; END      // Para la izquierda - arriba
                CASE -45:numero_grafico=5; END      // Para la derecha - arriba
                CASE 45: numero_grafico=1; END      // Para la izquierda - abajo
                CASE 55: numero_grafico=7; END      // Para la derecha - abajo
            END

            // Mira la posici¢n en el mapa
            siguiente_x_mapa=x_mapa+incremento_x;
            siguiente_y_mapa=y_mapa+incremento_y;

            // Comprueba los obst culos
            IF (map_get_pixel(fichero1,2,siguiente_x_mapa/2,(siguiente_y_mapa/2)+12)==24)
                obstaculo=TRUE;
            ELSE
                obstaculo=FALSE;
            END

            // Mueve al robot si se puede mover
            IF (se_mueve<>0 AND obstaculo==FALSE)
                // Cambia el gr fico de las piernas
                IF (contador_pasos<4)
                    contador_pasos=contador_pasos+1;
                ELSE
                    contador_pasos=1;
                END

                // Movimiento horizontal
                IF ((incremento_x>0 AND x_mapa<350) OR (incremento_x<0 AND x_mapa>10))
                    IF ((incremento_x>0 AND scroll.x0<40 AND x_mapa-scroll.x0>60) OR (incremento_x<0 AND scroll.x0>0 AND x_mapa-scroll.x0<260))                        scroll.x0+=incremento_x;
                    END
                    x_mapa=siguiente_x_mapa;
                END

                // Movimiento vertical
                IF ((incremento_y>0 AND y_mapa<2890 AND ((nomover==0 OR enemigo_f==0) OR y_mapa<scroll.y0+190)) OR (incremento_y<0 AND y_mapa>scroll.y0 AND y_mapa>scroll.y0+10))
                    IF ((incremento_y>0 AND scroll.y0<2700 AND y_mapa-scroll.y0>190) OR (incremento_y<0 AND scroll.y0>0 AND y_mapa-scroll.y0<150) AND (nomover==0 OR enemigo_f==0))
                        scroll.y0+=incremento_y;
                    END
                    y_mapa=siguiente_y_mapa;
                END

            ELSE
                contador_pasos=0;
            END

            // Actualiza la posici¢n del gr fico del robot al completo
            x=x_mapa;
            y=y_mapa;
            id_c_robot.x=x;
            id_c_robot.y=y;
            graph=10+(numero_grafico*5)+contador_pasos; // Actualiza el gr fico de las piernas
            FRAME;
        UNTIL (energ¡a_robot<0)     // Repite hasta que muera

        FRAME;
        signal(id_c_robot,s_kill);  // Elimina el cuerpo del robot y
        explosion_2(x,y-16,50);     // Explota

        FRAME;
        IF (se_mueve==1)            // Las piernas del robot andando
                // Cambia el contador de animaciones
                IF (contador_pasos<4)
                    contador_pasos=contador_pasos+1;
                ELSE
                    contador_pasos=1;
                END
                graph=10+(numero_grafico*5)+contador_pasos;
                // Actualiza coordenadas
                x=x+get_distx( ngulo0,incremento_x);
                y=y+get_disty( ngulo0,incremento_y);
                // Da un pantallazo con pausa
                FRAME(1000);
        END

        energ¡a_robot=8000;
        vidas--;                    // Quita una vida
        // Comprueba si se ha queda sin vidas
        IF (vidas>0)
           signal(type barra_energ¡a,s_kill);
           barra_energ¡a();         // Restaura la barra de energ¡a
        ELSE
            write(fuente_j,160,100,4,"FIN DEL JUEGO");
        END

        graph=0;                    // Quita el gr fico y
        explosion_1(x,y);           // pon una explosi¢n
        FRAME(1000);
    END
    // Quita todos los sonido y empieza de nuevo
    // porque significa que se ha quedado sin vidas
    stop_sound(all_sound);
    seleccion=0;
    // Espera hasta que se pulse el espacio
    WHILE (NOT key(_space))
        FRAME;
    END
    esta_muerto=1;                  // Vuelve al men£ principal
END

//-----------------------------------------------------------------------------
// Proceso vida
// Muestra un peque¤o robot
//-----------------------------------------------------------------------------

PROCESS vida();

BEGIN
    file=fichero1;graph=3;  // Selecciona el gr fico a usar
    x=15; y=17;             // Y las coordenadas del mismo
    size=70;                // Gr fico mas peque¤o
    LOOP FRAME; END         // Imprime el gr fico de la vidas
END

//-----------------------------------------------------------------------------
// Proceso disparo_robot
// Controla los disparo del robot
//-----------------------------------------------------------------------------

PROCESS disparo_robot(x,y);

PRIVATE
    toque;          // Identificador a los enemigos

BEGIN
    ctype=c_scroll; // Pone el proceso dentro del scroll
    z=10;
    priority=5;
    file=fichero1;  // Selecciona el gr fico a usar
    graph=100;

    // Comprueba si el disparo ha tocado alg£n enemigo
    WHILE (NOT out_region(id,0))
        // Si se toca a cualquier tipo de helicoptero sale del bucle
        IF (toque=collision(TYPE helicoptero1))
            BREAK;
        ELSE
            IF (toque=collision(TYPE helicoptero2))
                BREAK;
            ELSE
                IF (toque=collision(TYPE helicoptero3))
                    BREAK;
                END
            END
        END
        y-=12;      // Mueve hacia arriba el disparo
        FRAME;
    END

    // Si ha tocado alg£n enemigo, b jale la energ¡a
    IF (toque)
        toque.energ¡a-=300;
        impacto(x,y);   // Crea un proceso del tipo impacto
    END
END

//-----------------------------------------------------------------------------
// Proceso disparo_misiles
// Muestra los misiles del robot
//-----------------------------------------------------------------------------

PROCESS disparo_misiles();

PRIVATE
    j=0;    // Contador

BEGIN
    ctype=c_scroll;     // Introduce al proceso dentro del scroll
    z=father.z-1;
    priority=5;
    file=fichero1;
    graph=103;          // Selecciona el gr fico
    x=father.x;         // Coge las coordenadas respecto al padre, que es el
    y=father.y;         // proceso que lo llamo en este caso el robot
    humo(25,x,y);       // Crea  un humo que siga al misil

    // Mueve los misiles mediante un bucle
    FROM j=0 TO 10;
        y-=12;
        z=y;
        FRAME;
    END

    explosion_3(x,y);   // Crea una explosi¢n que elimina a los enemigos
END

//-----------------------------------------------------------------------------
// Proceso disparo_tanque
// Controla los disparo del tanque
//-----------------------------------------------------------------------------

PROCESS disparo_tanque();

PRIVATE
    angulo_a_robot;     // Angulo entre el robot y el tanque
    x1=0;               // Variable para crear la trayectoria y posici¢n iniciales
    y1=0;
    x2=0;
    y2=0;

BEGIN
    ctype=c_scroll;         // Introduce al proceso dentro del scroll
    file=fichero1;
    graph=104;              // Selecciona gr ficos

    // Coge la posici¢n del punto de control de la torre del tanque
    // para crear la trayectoria inicial
    get_point(fichero1,father.graph,1,&x1,&y1);
    get_point(fichero1,father.graph,2,&x2,&y2);

    // Selecciona una trayectoria hacia el robot
    angulo_a_robot=fget_angle(x1,y1,x2,y2);

    // Coge la posici¢n del punto de control de la torre del tanque
    // para crear la posici¢n inicial
    get_point(fichero1,father.graph,0,&x1,&y1);
    get_point(fichero1,father.graph,2,&x2,&y2);

    // Selecciona la posici¢n inicial
    x=(x2-x1)+(father.x);
    y=(y2-y1)+(father.y);

    sound(s_disparo_t,100,512); // Hace el sonido

    velocidad=5;                // Selecciona velocidad inicial
    // Repite mientras este en pantalla o le toque el robot
    WHILE (NOT out_region(id,0) AND NOT collision(TYPE robot) AND NOT collision(TYPE piernas_robot))
        x=x+get_distx(angulo_a_robot,velocidad);
        y=y+get_disty(angulo_a_robot,velocidad);
        angle=angulo_a_robot;
        FRAME;
    END
    // Si esta en pantalla es que ha tocado al robot y le decrementa la energ¡a
    IF (NOT out_region(id,0))
        impacto(x,y);
        energ¡a_robot-=1000;
    END
END

//-----------------------------------------------------------------------------
// Proceso disparo_helicoptero
// Controla los misiles que tiran los helic¢pteros
//-----------------------------------------------------------------------------

PROCESS disparo_helicoptero();

PRIVATE
    angulo_a_robot;     // Angulo entre el robot y el helic¢ptero
    id_humo;            // Identificador del humo del misil

BEGIN
    z=-48;
    ctype=c_scroll;             // Introduce al proceso dentro del scroll
    file=fichero1;
    graph=102;                  // Selecciona gr fico del objeto
    sound(s_explosion,64,512);  // Hace el sonido de la explosion
    x_resol=father.x*10000;     // Coge las coordenadas respeto al proceso
    y_resol=father.y*10000;     // que lo llamo que es el helicoptero
    x=x_resol/10000;
    y=y_resol/10000;
    angle=get_angle(id_robot);  // Coge el angulo hacia el robot
    id_humo=humo(25,x,y);       // Crea humo que siga al misil del helic¢ptero
    velocidad=40000;

    // Repite mientras este en pantalla o toque al robot
    WHILE (NOT out_region(id,0) AND NOT collision(TYPE robot) AND NOT collision(TYPE piernas_robot))
        // Se va moviendo dependiendo del angulo elegido
        x_resol=x_resol+get_distx(angle,velocidad);
        y_resol=y_resol+get_disty(angle,velocidad);
        x=x_resol/10000;
        y=y_resol/10000;
        IF (velocidad<80000)        // Incrementa la velocidad del misil
            velocidad+=5000;
        END
        FRAME;
    END
    // Si esta en pantalla es que ha tocado al robot y le decremento la energ¡a
    IF (NOT out_region(id,0))
        impacto(x,y);
        energ¡a_robot-=1000;
    END
    signal(id_humo,s_kill_tree);    // Elimina el humo del misil
END

//-----------------------------------------------------------------------------
// Proceso humo
// Muestra el humo que sigue a los misiles de los helic¢pteros
//-----------------------------------------------------------------------------

PROCESS humo(size,x_resol,y_resol);

BEGIN
    // Si es muy grande crea otros mas peque¤os
    IF (size>5)
        humo(size-5,x_resol,y_resol);
    END
    ctype=c_scroll;         // Lo introduce dentro del scroll
    graph=105;
    file=fichero1;          // Selecciona los gr ficos de los objetos
    flags=4;                // Transparente
    LOOP
        x=x_resol;
        y=y_resol;
        IF (father)         // Sigue al proceso que lo llamo
            x_resol=father.x;
            y_resol=father.y;
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso lanzador_enemigos
// Crea los enemigos
//-----------------------------------------------------------------------------

PROCESS lanzador_enemigos();

BEGIN
    // Va creando los distintos enemigos

    // Primero crea los proceso estaticos
    bunker(90,54,1506);         // (gr fico, x,y)
    bunker(90,32,1184);         // gr fico:
    bunker(90,122,463);         //  90 >>> left
    bunker(92,329,2248);        //  92 >>> right
    bunker(92,306,1942);
    bunker(92,306,1234);

    ca¤on_tanque(50,2600,0);     // (x,y, direcci¢n)
    ca¤on_tanque(359,2539,-1);   // direcci¢n:
    ca¤on_tanque(359,2484,0);    //  0  >>> est tico
    ca¤on_tanque(100,2146,0);    //  1  >>> hacia la derecha
    ca¤on_tanque(322,2146,0);    // -1  >>> hacia la izquierda
    ca¤on_tanque(75,2080,1);
    ca¤on_tanque(225,1767,0);
    ca¤on_tanque(129,1767,0);
    ca¤on_tanque(140,1629,1);
    ca¤on_tanque(319,1375,-1);
    ca¤on_tanque(169,1014,0);
    ca¤on_tanque(273,1014,0);
    ca¤on_tanque(65,1014,0);
    ca¤on_tanque(253,871,-1);
    ca¤on_tanque(137,600,-1);
    ca¤on_tanque(215,600,1);
    ca¤on_tanque(286,359,0);
    ca¤on_tanque(225,302,0);
    ca¤on_tanque(142,258,0);

    enemigo_final();

    // Espera a la una posici¢n concreta del mapa
    WHILE (scroll.y0>2410)
        FRAME;
    END
    // Y va creando nuevos enemigos
    helicoptero3();
    WHILE (scroll.y0>2210)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>2110)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>1990)
        FRAME;
    END
    helicoptero2();
    WHILE (scroll.y0>1850)
        FRAME;
    END
    helicoptero1();
    WHILE (scroll.y0>1700)
        FRAME;
    END
    helicoptero1();
    freq_t_d=16;
    freq_h_d=30;
    WHILE (scroll.y0>1390)
        FRAME;
    END
    helicoptero2();
    helicoptero3();
    WHILE (scroll.y0>1280)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>1110)
        FRAME;
    END
    helicoptero1();
    WHILE (scroll.y0>930)
        FRAME;
    END
    helicoptero1();
    helicoptero3();
    WHILE (scroll.y0>770)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>670)
        FRAME;
    END
    helicoptero1();
    freq_t_d=14;    // Incrementa la dificultad
    freq_h_d=25;
    WHILE (scroll.y0>620)
        FRAME;
    END
    helicoptero1();
    WHILE (scroll.y0>490)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>350)
        FRAME;
    END
    helicoptero1();
    helicoptero3();
    WHILE (scroll.y0>300)
        FRAME;
    END
    helicoptero3();
    WHILE (scroll.y0>260)
        FRAME;
    END
    helicoptero3();

END

//-----------------------------------------------------------------------------
// Proceso explosion_1
// Muestra una peque¤a explosi¢n
//-----------------------------------------------------------------------------

PROCESS explosion_1(x,y);

BEGIN
    ctype=c_scroll;                 // Introduce al proceso dentro del scroll
    file=fichero1;
    size=50;
    graph=301;
    z=-50;                          // Selecciona el gr fico inicial
    sound(s_explosion,100,256);     // Suena un sonido de explosion
    WHILE (graph<=315)
        FRAME;
        graph=graph+1;              // An¡malo
    END
END

//-----------------------------------------------------------------------------
// Proceso explosion_2
// Muestra una explosi¢n
//-----------------------------------------------------------------------------

PROCESS explosion_2(x,y,size);

BEGIN
    ctype=c_scroll;                 // Introduce al proceso dentro del scroll
    file=fichero1;
    graph=321;
    z=-50;                          // Selecciona el gr fico inicial
    sound(s_explosion2,100,300);    // Suena un sonido de explosion
    WHILE (graph<=359)
        FRAME;
        graph=graph+1;              // An¡malo
    END
END

//-----------------------------------------------------------------------------
// Proceso explosion_3
// Muestra una gran explosi¢n
//-----------------------------------------------------------------------------

PROCESS explosion_3(x,y);

BEGIN
    z=-50;
    ctype=c_scroll;                         // Introduce al proceso dentro del scroll
    file=fichero1;
    size=100;                               // Dimensiones normales
    sound(s_explosion2,150,300);            // Suena
    CLONE                                   // Otra explosi¢n
        size=30+rand(0,35);                 // Mas peque¤a (dimensiones al azar)
        x=x+rand(0,60)-30;                  // Otra posici¢n (al azar)
        y=y+rand(0,60)-30;
        angle=rand(0,2*pi)+1;               // Otro angulo (al azar)
        CLONE                               // Otra explosi¢n
            z=z-1;                          // Otra profundidad
            size=30+rand(0,35);             // Mas peque¤a
            x=x+rand(0,60)-30;              // Otra posici¢n
            y=y+rand(0,60)-30;
            angle=rand(0,2*pi)+1;           // Otro angulo
            CLONE                           // Otra explosi¢n
                z=z-1;                      // Otra profundidad
                size=60;                    // Otra dimensiones
                x=x+rand(0,60)-30;          // Otra posici¢n
                y=y+rand(0,60)-30;
                angle=rand(0,2*pi)+1;       // Otro angulo
            END
        END
    END
    graph=321;                              // Selecciona gr fico inicial
    WHILE (graph<=359)
        FRAME;
        graph=graph+1;                      // Anima los gr ficos
    END
END

//-----------------------------------------------------------------------------
// Proceso ca¤on_tanque
// Controla los movimientos del ca¤on del tanque
//-----------------------------------------------------------------------------

PROCESS ca¤on_tanque(x,y,direction);

PRIVATE
    id_tanque;          // Identificador del tanque
    id_s_motor;         // Identificador de sonido
    disparo=0;          // Contador de la frecuencia de disparo
    // Secuencia de los gr ficos
    tabla_graficos[]=16,60,59,58,57,56,55,54,53,52,68,67,65,64,63,62,61;

BEGIN
    disparo=freq_t_d;
    xgraph=&tabla_graficos;     // pone los gr ficos de acuerdo al angulo
    _sonido=1;
    file=fichero1;
    id_tanque=tanque();         // Crea el tanque
    ctype=c_scroll;
    z=10;
    energ¡a=2500;
    graph=64;                   // Selecciona el gr fico inicial
    WHILE (out_region(id,0))    // Espera a que este dentro de pantalla
        FRAME;
    END
    id_s_motor=sound(s_motor,130,256);
    WHILE (energ¡a>0)           // Repite mientras haya energ¡a

        // Controla el sonido
        IF (out_region(id,0))
            IF (_sonido)
                stop_sound(id_s_motor);
                _sonido=0;
            END
        ELSE
            IF (NOT _sonido)
                id_s_motor=sound(s_motor,130,256);
                _sonido=1;
            END
        END

        // Comprueba si ha tocado alg£n obst culo del escenario
        IF (map_get_pixel(fichero1,101,x+10,y)<>58)
            x+=direction;
        END
        angle=get_angle(id_robot);  // Pone el gr fico del ca¤on
        IF (_sonido)
            IF (disparo==0)
                disparo_tanque();
                disparo=freq_t_d;
            ELSE
                disparo--;
            END
        END
        FRAME;
    END
    // El ca¤on del tanque ha sido destruido
    signal(id_tanque,s_kill);
    stop_sound(id_s_motor);
    IF (energ¡a<=0)
        humo_tanque(x,y);
        explosion_1(x,y);
        puntuacion+=100;
    END
END

//-----------------------------------------------------------------------------
// Proceso tanque
// Muestra el cuerpo del tanque y detecta algunas colisiones
//-----------------------------------------------------------------------------

PROCESS tanque();

PRIVATE
    si_me_tocan;  // Identificador

BEGIN
    z=15;
    ctype=c_scroll;     // Introduce al proceso dentro scroll
    graph=50;
    file=fichero1;      // Selecciona gr fico
    LOOP
        // Comprueba si le ha tocado el disparo del robot
        IF (si_me_tocan=collision(TYPE disparo_robot))
            impacto(si_me_tocan.x,si_me_tocan.y);
            signal(si_me_tocan,s_kill);
            // Quien tiene la energia es el proceso que llamo a este
            father.energ¡a=father.energ¡a-175;
        ELSE
            IF (si_me_tocan=collision(TYPE explosion_3))
                father.energ¡a=father.energ¡a-100;
            END
        END
        x=father.x;                 // Coge la posici¢n de la torre del tanque
        y=father.y;
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso humo_tanque
// Muestra el humo de los tanques
//-----------------------------------------------------------------------------

PROCESS humo_tanque(x,y);

PRIVATE
    si_me_tocan;    // Identificador de proceso

BEGIN
    z=100;
    ctype=c_scroll;
    graph=50;
    file=fichero1;
    energ¡a=1400;   // Selecciona gr fico

    // Comprueba si ha tocado al disparo del robot
    WHILE (energ¡a>0)
        humo2(x+rand(0,8)-4,y+rand(0,8)-4);     // Crea otros humos
        IF (si_me_tocan=collision(TYPE disparo_robot))
            impacto(si_me_tocan.x,si_me_tocan.y);
            signal(si_me_tocan,s_kill);
            energ¡a=energ¡a-175;
        ELSE
            // Comprueba si le ha tocado la explosion de un misil
            IF (collision(TYPE explosion_3))
                energ¡a-=100;
            END
        END
        FRAME;
    END

    // El tanque es destruido
    explosion_2(x,y,100);
    puntuacion+=50;
END

//-----------------------------------------------------------------------------
// Proceso humo2
// Muestra y eleva el humo
//-----------------------------------------------------------------------------

PROCESS humo2(x,y);

BEGIN
    size=80;
    ctype=c_scroll;             // Introduce al proceso dentro del scroll
    graph=105;
    file=fichero1;
    flags=4;                    // Selecciona el gr fico
    REPEAT                      // An¡malo
        x+=rand(-1,1);
        y-=4;
        size-=8;
        FRAME;
    UNTIL (size<=0)            // Mientras tenga tama¤o
END

//-----------------------------------------------------------------------------
// Proceso impacto
// Muestra una peque¤a explosi¢n
//-----------------------------------------------------------------------------

PROCESS impacto(x,y);

BEGIN
    sound(s_explosion,48,1024);     // Haz sonido
    ctype=c_scroll;                 // Introcude al proceso dentro del scroll
    file=fichero1;
    graph=105;
    size=26;                        // Selecciona gr fico
    REPEAT                          // An¡malo
        size-=2;
        FRAME;
    UNTIL (size==2)                 // Mientras tenga tama¤o
END

//-----------------------------------------------------------------------------
// Proceso bunker
// Muestra y controla los b£nkers
//-----------------------------------------------------------------------------

PROCESS bunker(graph,x,y);

PRIVATE
    si_me_tocan;    // Identificador al disparo del robot
    disparo;        // Contador de la frecuencia de disparo

BEGIN
    file=fichero1;
    ctype=c_scroll; // Introduce al proceso dentro del scroll
    z=10;
    energ¡a=3000;   // Selecciona los gr ficos

    WHILE (out_region(id,0))    // Espera ha estar en pantalla
        FRAME;
    END

    WHILE (energ¡a>0)

        // Comprueba si le ha tocado el disparo del robot
        IF (si_me_tocan=collision(TYPE disparo_robot))
            impacto(si_me_tocan.x,si_me_tocan.y);
            signal(si_me_tocan,s_kill);
            energ¡a-=175;
        ELSE
            IF (collision(TYPE explosion_3))
                energ¡a-=100;
            END
        END

        // Comprueba si esta fuera de pantalla
        IF (NOT out_region(id,0))
            IF (disparo--==0)
                disparo=10;
                IF (graph==90)
                    disparo_bunker(x+38,y+26,94,8,8);
                ELSE
                    disparo_bunker(x-32,y+28,95,-8,8);
                END
                sound(s_disparo_t,100,512);
            END
        END
        FRAME;
    END

    // El bunker es destruido y...
    graph++;
    explosion_2(x,y,100);
    puntuacion+=200;

    LOOP    // Todo el rato esta echando humo
        humo2(x+rand(0,8)-4,y+rand(0,8)-4);
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso disparo_bunker
// Muestra los proyectiles del bunker
//-----------------------------------------------------------------------------

PROCESS disparo_bunker(x,y,graph,ix,iy);

BEGIN
    file=fichero1;
    ctype=c_scroll;
    z=10;
    // Repite si el objeto esta dentro de pantalla y no lo ha tocado el robot
    WHILE (NOT out_region(id,0) AND NOT collision(TYPE robot) AND NOT collision(TYPE piernas_robot))
        x+=ix;
        y+=iy;
        FRAME;
    END
    // Si el objeto esta dentro de pantalla es que ha tocado al robot, entonces qu¡tale energ¡a
    IF (NOT out_region(id,0))
        impacto(x,y);
        energ¡a_robot-=1000;
    END
END

//-----------------------------------------------------------------------------
// Proceso misiles
// Muestra los misiles del robot disponibles
//-----------------------------------------------------------------------------

PROCESS misiles(x,y);

BEGIN
    file=fichero1;
    graph=103;
    size=100;               // Selecciona gr fico
    LOOP
        FRAME;              // pone gr fico
    END
END

//-----------------------------------------------------------------------------
// Proceso misil
// Controla los misiles del robot disponibles
//-----------------------------------------------------------------------------

PROCESS misil();

PRIVATE
    xm=65;      // Posici¢n x del misil
    ym=10;      // Posici¢n y del misil
    c=0;        // Contador

BEGIN
    // Muestra el gr fico en pantalla
    FOR (c=1;c<=contador_misiles;c++)
        tabla_misiles[misiles_libres++]=misiles(xm,ym);
        xm+=7;
    END
    LOOP
        c=contador_misiles;
        WHILE (c==contador_misiles)  // Repite mientras haya misiles
            FRAME;
        END
        // Elimina el identificador al misil
        signal(tabla_misiles[--misiles_libres],s_kill);
        xm-=7;
    END
END

//-----------------------------------------------------------------------------
// Proceso barra_energ¡a
// Muestra la barra de energ¡a
//-----------------------------------------------------------------------------

PROCESS barra_energ¡a();

PRIVATE
    wide;       // Ancho de la barra

BEGIN
    file=fichero1;
    region=1;
    x=180;
    y=10;
    z=-100;
    graph=101;                  // Selecciona gr fico
    wide=120;                   // Inicializa la barra
    energ¡a=energ¡a_robot;
    define_region(1,119,0,wide+1,20); // Define la regi¢n visible

    LOOP
        // Repite mientras haya energia
        WHILE (energ¡a==energ¡a_robot)
            FRAME;
        END
            wide=wide-16;
            IF (wide<0) wide=0; END
            define_region(1,119,0,wide+1,20); // Reescala la zona visible
            energ¡a=energ¡a_robot;
    END

END

//-----------------------------------------------------------------------------
// Proceso pausa
// Controla la pausa
//-----------------------------------------------------------------------------

PROCESS pausa();

PRIVATE
    texto_pausa;     // Identificador del texto

BEGIN
    LOOP
        WHILE (NOT key(_p))         // Espera a que pulsen la 'p'
            FRAME;
        END
        WHILE (KEY(_p)) FRAME; END  // Espera a que suelten la 'p'
        texto_pausa=write(fuente_j,160,100,4,"PAUSA");
        FRAME;
        REPEAT  UNTIL (KEY(_p));    // Haz la pausa
        delete_text(texto_pausa);
        WHILE (KEY(_p)) END         // Espera ha que suelte la 'p'

    END
END

//-----------------------------------------------------------------------------
// Proceso helices
// Muestra las h‚lices de los helic¢pteros
//-----------------------------------------------------------------------------

PROCESS helices();

PRIVATE
    sign=1; // Direcci¢n de giro

BEGIN
    ctype=c_scroll;
    size=father.size;
    graph=86;
    flags=4;                // Transparente
    file=fichero1;          // Selecciona gr fico
    z=father.z-1;           // por encima del helic¢ptero
    angle=0;
    priority=1;

    CLONE                   // Crea otra h‚lice
        angle=pi_cuartos;   // En otro angulo
        sign=-1;            // Con otra direcci¢n
    END

    LOOP                    // Anima todo
        x=father.x;
        y=father.y;
        angle=angle+pi_octavos*sign;
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso helicoptero1
// Controla el tipo helicoptero1
//-----------------------------------------------------------------------------

PROCESS helicoptero1();

PRIVATE
    id_helices; // Identificadores de las h‚lices
    _cx;        // Ultima x
    _cy;        // Ultima y
    p;          // posici¢n relativa x
    disparo;    // Contador de la frecuencia de disparo
    tabla_graficos[]=16,81,80,79,78,77,76,75,74,73,72,71,70,85,84,83,82;

BEGIN
    priority=2;
     ngulo0=pi;
    disparo=rand(0,freq_h_d*2-1);   // Haz una pausa al azar hasta que salga el helic¢ptero
    p=20;
    z=-48;
    CLONE                           // Crea otro helic¢ptero
         ngulo0=0;
        p=300;
    END
    xgraph=&tabla_graficos;         // Selecciona la tabla de gr ficos

    nomover++;
    ctype=c_scroll;
    file=fichero1;
    size=100;
    graph=11;
    id_helices=helices();   // Crea las h‚lices
    energ¡a=1500;           // energ¡a del helic¢ptero
    x=scroll.x0+160;        // posiciona al helic¢ptero
    y=scroll.y0-10;

    _cx=x;

    // Mueve el helic¢ptero mientras este en pantalla o no haya muerto
    WHILE (energ¡a>0 AND y<scroll.y0+220)
        y++;
        x=_cx+get_distx( ngulo0,120);
        FRAME;
         ngulo0+=pi_cuartos/32;
        angle=get_angle(id_robot);
        IF (disparo==0)            // Dispara
            disparo_helicoptero();
            disparo=freq_h_d*2;
        ELSE
            disparo--;
        END
    END

    IF (energ¡a>0)
        angle=get_angle(id_robot);      // Cambia la direcci¢n
         ngulo0=fget_angle(x,y,scroll.x0+p,scroll.y0+20);
    END

    WHILE (energ¡a>0 AND y>scroll.y0+20)
        x=x+get_distx( ngulo0,4);
        y=y+get_disty( ngulo0,4);
        angle=get_angle(id_robot);      // Cambia la direcci¢n
        // Comprueba si dispara
        IF (disparo==0)
            disparo_helicoptero();
            disparo=freq_h_d;
        ELSE
            disparo--;
        END

        FRAME;
    END

    IF (energ¡a>0)
         ngulo0=get_angle(id_robot);        // Cambia la direcci¢n
    END

    // Repite mientras este en pantalla o tenga energ¡a
    WHILE (energ¡a>0 AND NOT out_region(id,0))
        x=x+get_distx( ngulo0,7);
        y=y+get_disty( ngulo0,7);
        angle=get_angle(id_robot);
        IF (disparo==0)
            disparo_helicoptero();
            disparo=freq_h_d;
        ELSE
            disparo--;
        END
        FRAME;
    END

    // Si no tiene energ¡a destruye al helic¢ptero
    IF (energ¡a<=0)
        explosion_1(x,y);
        puntuacion+=125;
    END

    signal(id_helices,s_kill_tree);
    nomover--;
END

//-----------------------------------------------------------------------------
// Proceso helicoptero2
// Controla los helic¢pteros tipo 2
//-----------------------------------------------------------------------------

PROCESS helicoptero2();

PRIVATE
    id_helices=0;   // Identificador de las h‚lices
    disparo;        // Contador de frecuencia de disparo
    tabla_graficos[]=16,81,80,79,78,77,76,75,74,73,72,71,70,85,84,83,82;
    inc_x=1;    // Direcci¢n horizontal
    inc_y=1;    // Direcci¢n vertical


BEGIN
    priority=2;
    x=scroll.x0+20;
    y=scroll.y0-10;
    disparo=rand(0,freq_h_d*2-1);
    z=-48;
    CLONE       // Crea otro helic¢ptero
        x+=300;
        y=scroll.y0-10;
    END

    xgraph=&tabla_graficos;

    nomover++;
    ctype=c_scroll;         // Introduce al proceso dentro del scroll
    file=fichero1;
    size=100;
    id_helices=helices();   // Crea las h‚lices
    energ¡a=1500;

    REPEAT
        // Mueve el helic¢ptero al azar
        IF (x<scroll.x0+20)  inc_x=rand(2,8);  END
        IF (x>scroll.x0+300) inc_x=rand(-8,-2); END
        IF (y<scroll.y0+20) inc_y=rand(2,4); END
        IF (y>scroll.y0+70) inc_y=rand(-4,-2); END

        x+=inc_x;
        y+=inc_y;

        IF (disparo==0)
            disparo_helicoptero();
            disparo=freq_h_d*rand(1,3);
        ELSE
            disparo--;
        END

        angle=get_angle(id_robot);
        FRAME;
    UNTIL (energ¡a<0 OR rand(1,200)>199)

    // Repite mientras este en pantalla o tenga energ¡a
    WHILE (energ¡a>0 AND NOT out_region(id,0))
        x+=inc_x;
        y+=inc_y;
        angle=get_angle(id_robot);
        IF (disparo==0)
            disparo_helicoptero();
            disparo=freq_h_d;
        ELSE
            disparo--;
        END
        FRAME;
    END

    // Si no tiene energ¡a elimina al helic¢ptero
    IF (energ¡a<=0)
        explosion_1(x,y);
        puntuacion+=125;
    END

    signal(id_helices,s_kill_tree);
    nomover--;

END

//-----------------------------------------------------------------------------
// Proceso helicoptero3
// Controla los helic¢pteros tipo 3
//-----------------------------------------------------------------------------

PROCESS helicoptero3();

PRIVATE
    id_helices=0;   // Identificador de las h‚lices
    disparo=0;      // Contador de frecuencia de disparo
    // Tabla con la animaci¢n de los gr ficos
    tabla_graficos[]=16,81,80,79,78,77,76,75,74,73,72,71,70,85,84,83,82;
    inc_x=1;        // Direcci¢n horizontal
    inc_y=1;        // Direcci¢n vertical
    posicion_x;

BEGIN
    posicion_x=rand(10,140);
    priority=2;
    x=scroll.x0+posicion_x;
    y=scroll.y0-10;     // Inicializa las coordenadas
    z=-48;
    CLONE               // Crea otro helic¢ptero
        x=scroll.x0+320-posicion_x;
    END

    xgraph=&tabla_graficos;  // Selecciona la tabla de gr ficos

    disparo=rand(freq_h_d,freq_h_d*2);

    ctype=c_scroll;          // Introduce al proceso dentro del scroll
    file=fichero1;
    id_helices=helices();
    energ¡a=1500;

    angle=get_angle(id_robot);
     ngulo0=angle;

    // Comprueba que este en pantalla y que tenga energ¡a
    WHILE (energ¡a>0 AND y<scroll.y0+220)
        x+=get_distx( ngulo0,2);
        y+=get_disty( ngulo0,2);
        angle=get_angle(id_robot);
        IF (disparo==0)
            disparo_helicoptero();
            disparo=freq_h_d;
        ELSE
            disparo--;
        END
        FRAME;
    END
    // Si no tiene energ¡a destruye el helic¢ptero
    IF (energ¡a<=0)
        explosion_1(x,y);
        puntuacion+=125;
    END

    signal(id_helices,s_kill_tree);

END

//-----------------------------------------------------------------------------
// Proceso intro
// Maneja la introducci¢n al juego
//-----------------------------------------------------------------------------

PROCESS intro();

PRIVATE
    j;          // Contador
    id_sonido;  // Identificador de sonido

BEGIN
    signal(father,s_sleep);
    FRAME(200);
    file=fichero2;
    graph=10;               // Selecciona los gr ficos
    x=160;
    y=100;
    size=500;               // Selecciona coordenadas y tama¤o
    angle=0;
    fade_on();
    // Redimensiona los gr ficos
    FROM size=500 TO 110 STEP -10;
        angle=angle+(pi/20);
        IF (key(_space))    // Si se pulsa 'espacio', salta la intro
            BREAK;
        END
        FRAME;
    END

    IF (NOT key(_space))
        sound(s_pum,250,256);
        FROM j=0 TO 75;
            IF (j==30)
                id_sonido=sound(s_alien,100,256);
            END
            IF (key(_space))    // Si se pulsa 'espacio', salta la intro
                BREAK;
            END
            FRAME;
        END
        stop_sound(id_sonido);
    END

    fade_off();                 // Borra la pantalla y para los sonidos
    clear_screen();
    signal(father,s_wakeup);
END

//-----------------------------------------------------------------------------
// Proceso cursor
// Controla el cursor del men£
//-----------------------------------------------------------------------------

PROCESS cursor();

PRIVATE
    c;

BEGIN
    file=fichero2;
    graph=1;
    x=180;
    y=320;
    c=3;
    // Reiniciamos selecci¢n
    seleccion=0;
    WHILE (NOT key(_space) AND NOT key(_control) AND NOT key(_enter) AND seleccion==0)
        // Mueve el cursor si se pulsa los cursores
        IF (key(_up) AND y>320)
            y=320;
            sound(s_bip,100,256);
            FRAME;
        END
        IF (key(_down) AND y<380)
            y=380;
            sound(s_bip,100,256);
        END
        x+=c;
        IF (x==210)
            c=-3;
        END
        IF (x==180)
            c=3;
        END
        FRAME;
        // Si se pulsa [ESCAPE] se selecciona la opci¢n de salir
        IF (key(_esc))
            seleccion=2;
            y=380;
        END

    END
    sound(s_disparo,100,256);
    // Comprueba la selecci¢n del cursor dependiendo de su posicion
    SWITCH (y);
        CASE 320:
            seleccion=1;
        END
        CASE 380:
            seleccion=2;
        END
    END
END

//-----------------------------------------------------------------------------
// Proceso enemigo_final
// Controla al enemigo del final del juego
//-----------------------------------------------------------------------------

PROCESS enemigo_final();

PRIVATE
    espera;
    disparo_1=10;   // Contador de la frecuencia de disparo 1
    disparo_2=20;   // Contador de la frecuencia de disparo 2
    si_me_tocan;    // Identificador de proceso
    contador0;              // Contador
    tabla_graficos[]=8,402,402,402,400,400,400,401,402; // Secuencia de gr ficos

BEGIN
    file=fichero1;
    x=180;
    y=130;
    ctype=c_scroll;                        // Selecciona la posici¢n
    energ¡a=30000;
    xgraph=&tabla_graficos;         // Selecciona la tabla de gr ficos
    WHILE (get_dist(id_robot)>160)
        FRAME;
    END
    enemigo_f=0;
    WHILE (energ¡a>0)
        IF (si_me_tocan=collision(TYPE disparo_robot))
            impacto(si_me_tocan.x,si_me_tocan.y);
            signal(si_me_tocan,s_kill);
            energ¡a=energ¡a-175;
        ELSE
            // Comprueba si ha tocado con el misil del robot( explosion3=sigue al misil)
            IF (si_me_tocan=collision(TYPE explosion_3))
               energ¡a=energ¡a-100;
            END
        END
        FRAME;

        angle=get_angle(id_robot); // pone el gr fico del ca¤on
        // Crea helic¢pteros
        IF (nomover==0 AND get_dist(id_robot)<200)
            helicoptero2();
        END
        // Dispara
        IF (disparo_1==0)
            disparo_final(x,y);
            disparo_1=30;
        ELSE
            disparo_1--;
        END
        // Lanza misil
        IF (disparo_2==0)
            misiles_final(x,y,3);
            misiles_final(x,y,2);
            disparo_2=25;
        ELSE
            disparo_2--;
        END

        // Comprueba si le ha tocado un disparo del robot
        FRAME(0);
    END
    // El enemigo es destruido
    explosion_3(x,y);
    FRAME(400);
    explosion_3(x+30,y-30);
    FRAME(500);
    explosion_3(x-30,y+30);
    graph=0;
    xgraph=0;  // Borra el gr fico
    WHILE (nomover>0)
        FRAME;
    END
    // Imprime el mensaje de final de juego
    write(fuente_j,160,100,4,"CONSEGUIDO");
    // Espera a presionar 'espacio' o 'enter'
    espera=0;
    REPEAT
        espera++;
        FRAME;
    UNTIL (espera==100 OR key(_space) OR key(_enter) OR key(_esc))

    // Acaba el juego
    delete_text(all_text);
    let_me_alone();
    signal(id_menu,s_wakeup);
    stop_sound(all_sound);
    esta_muerto=1;
    seleccion=0;
    FRAME;

END

//-----------------------------------------------------------------------------
// Proceso disparo_final
// Controla el disparo de enemigo final
//-----------------------------------------------------------------------------

PROCESS disparo_final(x,y);

PRIVATE
    x1=0;
    y1=0;

BEGIN
    file=fichero1;
    graph=106;
    ctype=c_scroll;                            // Selecciona gr fico
    get_point(fichero1,father.graph,1,&x1,&y1);
    x=x+x1-110;
    y=y+y1-90;                          // Selecciona posici¢n
     ngulo0=father.angle;
    x_resol=x*1000;
    y_resol=y*1000;

    WHILE (NOT out_region(id,0) AND NOT collision(TYPE robot) AND NOT collision(TYPE piernas_robot))
        x_resol=x_resol+get_distx( ngulo0,3000);
        y_resol=y_resol+get_disty( ngulo0,3000);      // Selecciona direcci¢n

        x=x_resol/1000;
        y=y_resol/1000;
        FRAME;
    END

    IF (NOT out_region(id,0))           // Comprueba que este dentro de pantalla
        impacto(x,y);
        energ¡a_robot-=2000;
    END
END

//-----------------------------------------------------------------------------
// Proceso misiles_final
// Controla los misiles del enemigo final
//-----------------------------------------------------------------------------

PROCESS misiles_final(x,y,n);

PRIVATE
    id_humo;    // Identificador de humo
    x1;         // x del punto de control
    y1;         // y del punto de control
    inc_ ngulo;        // Incremento del angulo

BEGIN
    ctype=c_scroll;
    file=fichero1;
    graph=102;                                  // Selecciona gr fico
    get_point(fichero1,father.graph,n,&x1,&y1); // Coge el punto de control
    x=x+x1-110;
    y=y+y1-90;                                  // Selecciona posici¢n y...
    angle=father.angle;                         // Direcci¢n
    inc_ ngulo=get_angle(id_robot)/5;
    sound(s_explosion,64,512);
    x_resol=x*10000;
    y_resol=y*10000;

    id_humo=humo(25,x,y);                 // Crea un humo que le siga

    velocidad=40000;                      // pone velocidad inicial
    // Repite mientras este en pantalla y no haya tocada el robot
    WHILE (NOT out_region(id,0) AND NOT collision(TYPE robot) AND NOT collision(TYPE piernas_robot))
        x_resol=x_resol+get_distx(angle,velocidad);
        y_resol=y_resol+get_disty(angle,velocidad);   // Actualiza las coordenadas
        IF (n==2)                           // y la direcci¢n
            angle+=pi_octavos/inc_ ngulo;
        ELSE
            angle-=pi_octavos/inc_ ngulo;
        END

        x=x_resol/10000;
        y=y_resol/10000;
        IF (velocidad<80000)
            velocidad+=5000;
        END
        FRAME;
    END
    // Comprueba que haya tocado al robot
    IF (NOT out_region(id,0))
        impacto(x,y);
        energ¡a_robot-=1000;
    END
    signal(id_humo,s_kill_tree);
END
