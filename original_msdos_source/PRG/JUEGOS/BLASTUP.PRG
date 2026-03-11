
//-----------------------------------------------------------------------------
// TITULO:      BLAST EM UP!
// AUTOR:       LUIS SUREDA / MANUEL CABA¥AS
// FECHA:       3/4/97
//-----------------------------------------------------------------------------


PROGRAM blastemup;

CONST
    disparo_max=4;      // Numero de disparo m ximos
    normal_movim=2;     // Velocidad normal de movimiento
    vidas_max=3;        // M ximo numero de vidas

    max_ani=5;          // Numero de animaciones m ximas
    max_trayec=10;      // Numero m ximos de trayectorias

GLOBAL
    fichero1;           // Gr ficos fichero1

    fuente1,fuente2,fuente3,fuente4,fuente5;  // Fuentes de las letras

    sonido1,sonido2,sonido3,sonido4,          // Sonidos del programa
    sonido5,sonido6,sonido7,sonido8;

    estado_programa=1;      // (1->Seleccion 2->Juego 3->Creditos 4->Fin)
    nuevo_estado=0;         // Es el siguiente estado del programa
    velocidad_movim;        // Velocidad de movimiento actual

    // Datos de la nave del jugador
    id_nave_princ;   // Identificador para la nave del jugador
    estado_nave;     // Estado de la nave (1 -> Normal, 2 -> Invulnerable)
    puntuacion;      // Puntuaci¢n del jugador
    n_vidas;         // Numero de vidas actuales

    // Datos de pantalla
    posic_x_bonus[]=495,526,557,588,619;
    posic_x_vidas[vidas_max]=37,73,108;
    id_bonus[5];                // Identificador para bonus
    id_vidas[vidas_max];        // Identificador de vidas extra

    // Datos del grupo de enemigos
    grupo_actual;       // Numero de grupo actual
    ultimo_grupo;       // Numero de grupos
    grupo_pantalla;     // Numero del grupo en pantalla
    vivos[3];           // Numero de enemigos vivos en el grupo [i]
    no_bonus[3];        // Bandera. 1=Cuando alg£n enemigo en el grupo [i] no ha muerto


    bonus_x;     // Posici¢n de los bonus
    bonus_y;

    nivel_actual;  // Cada diez grupos cambian de nivel

    // Numero de im genes, lista de gr ficos
    STRUCT animacion_enemigo[max_ani];
        n_imagenes;         // Numero de im genes
        graficos[31];       // Lista de gr ficos
    END =
        16,200,201,202,203,204,205,206,207,208,209,210,211,
        212,213,214,215,16 DUP(0),          // Enemigo tipo 1

        16,220,221,222,223,224,225,226,227,228,229,230,231,
        232,233,234,235,16 DUP(0),          // Enemigo tipo 2

        20,250,251,252,253,254,254,255,256,257,258,258,257,
        256,255,254,254,253,252,251,250,12 DUP(0),
                                            // Enemigo tipo 3
        32,240,240,240,241,242,243,244,245,246,247,247,247,
        247,247,247,247,247,247,247,247,247,247,247,246,
        245,244,243,242,241,240,240,240,    // Enemigo tipo 4

        8,260,261,262,263,264,265,266,267,24 DUP(0),
                                            // Mina

        1,600,31 DUP(0);                    // Enemigo final


        STRUCT trayectoria[max_trayec];
        n_seccion;              // Numero de secciones
        x_ini;                  // Posici¢n inicial
        y_ini;
            STRUCT trayec[18];  // 19 es el numero de secciones
                imagener_por_seccion;
                velocidad_x;    // Velocidad
                velocidad_y;
                aceleraci¢n_x;  // Aceleraci¢n
                aceleraci¢n_y;
            END
        END=
            19,630,240,
            12,0,-8,0,0,    24,0,8,0,0,         // Trayectoria del enemigo final
            12,0,-8,0,0,    15,-32,-15,0,1,
            30,16,0,0,0,    12,0,8,0,0,
            15,-32,15,0,    -1,30,16,0,0,0,
            12,0,-8,0,0,    30,-16,0,0,0,
            30,16,0,0,0,    15,-32,15,0,-1,
            24,0,-8,0,0,    30,16,0,0,0,
            15,-32,15,0,-1, 30,16,0,0,0,
            15,-32,-15,0,1, 12,0,8,0,0,
            30,16,0,0,0,

            1,650,280,
            80,-35,0,1,0 , 18 DUP(0,0,0,0,0),   // Trayectoria de las minas

            2,300,20,
            18,1,1,1,1, 30,15,15,-5,1,  17(0,0,0,0,0),

            7,0,60,
            22,25,0,0,0,    5,0,10,0,0,      20,-25,10,0,0,
            20,0,-10,0,0,   20,25,10,0,0,    5,0,10,0,0,
            20,-25,0,-1,0,  12(0,0,0,0,0),

            6,630,150,
            5,-8,0,0,0,      8,-6,-9,0,0,    16,-7,9,0,0,
            16,-6,-9,0,0,    16,-7,9,0,0,    20,-10,0,-2,0,
            13(0,0,0,0,0),

            6,630,200,
            5,-8,0,0,0,      8,-6,9,0,0,     16,-7,-9,0,0,
            16,-6,9,0,0,     16,-7,-9,0,0,   20,-10,0,-2,0,
            13(0,0,0,0,0),

            5,630,250,
            26,-15,0,0,0,    12,-12,-10,2,0, 12,15,0,0,0,
            12,12,10,-2,0,   20,-15,0,-1,0,  14(0,0,0,0,0),

            7,630,110,
            10,0,10,0,0,     32,-10,0,0,0,   8,-10,10,0,0,
            8,10,10,0,0,     34,10,0,0,0,    8,0,-10,0,0,
            40,-10,0,-1,0,   12(0,0,0,0,0),

            8,630,0,
            25,-10,25,0,-1,  20,-20,-10,2,0, 20,20,10,-2,0,
            20,-20,-10,2,0,  20,20,10,-2,0,  20,-20,-10,2,0,
            20,20,10,-2,0,   30,-10,0,-2,0,  11(0,0,0,0,0),

            7,0,220,
            22,25,0,0,0,     5,0,10,0,0,     20,-25,10,0,0,
            20,0,-10,0,0,    20,25,10,0,0,   5,0,10,0,0,
            20,-25,0,-1,0,   12(0,0,0,0,0),

            6,640,140,
            8,-10,0,0,3,  16,-10,21,0,-3, 16,-10,-24,0,3,
            16,-10,21,0,-3, 16,-10,-24,0,3,  16,-10,21,0,-3,
            13(0,0,0,0,0);

LOCAL
    info;           // Informaci¢n general
    energia;        // Energ¡a inicial de la nave
    id_colision;    // Identificador para colisi¢n

BEGIN
    set_mode(m640x480);     // Selecciona el modo de v¡deo

    // Carga las fuentes de las letras
    fuente1=load_fnt("blastup\opciones.fnt");
    fuente2=load_fnt("blastup\marcador.fnt");
    fuente3=load_fnt("blastup\juego.fnt");
    fuente4=load_fnt("blastup\creditos.fnt");

    // Carga sonidos
    sonido1=load_pcm("blastup\faser.pcm",0);     // Disparo
    sonido2=load_pcm("blastup\laser.pcm",0);     // Laser enemigo
    sonido3=load_pcm("blastup\disparo2.pcm",0);  // Misil enemigo
    sonido4=load_pcm("blastup\explosio.pcm",0);  // Explosi¢n
    sonido5=load_pcm("blastup\ding.pcm",0);      // Escudo
    sonido6=load_pcm("blastup\buiu.pcm",0);      // Bonus
    sonido7=load_pcm("blastup\fx1.pcm",0);       // Vida extra
    sonido8=load_pcm("blastup\cosa_a.pcm",0);    // Minas

    // Bucle principal
    LOOP
        SWITCH (estado_programa);
            CASE 1: titulo(); END                // Ver men£
            CASE 2: juego();  END                // Jugar al juego
            CASE 3: creditos(); END              // Ver cr‚ditos
            CASE 4: exit("Gracias por jugar!",0); END  // Salir
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso enemigo
//  Maneja todas las acciones de los enemigos
//-----------------------------------------------------------------------------

PROCESS enemigo(tipo_enemigo,tray,agresividad,n_grupo);

PRIVATE
    n_animacion_s;  // Numero de pasos en animaci¢n
    animacion=0;    // Contador de animaciones
    contador=0;     // Contador de el numero de pasos en la secci¢n actual
    velocidad_x;    // Velocidad
    velocidad_y;

    aceleraci¢n_x;  // Aceleraci¢n
    aceleraci¢n_y;

    numero_seccion=0;   // Numero de secci¢n
    actual_seccion=0;   // Secci¢n actual
    puntos;             // Puntos por matar a un enemigo
    tocado;             // Impactos recibidos

    xtiempo=0;          // Posici¢n de la explosi¢n del enemigo final
    ytiempo=0;

    contador1=0;        // Contadores generales
    contador2=0;

    imagenes;   // Numero de pasos en la secci¢n actual
    muerto=0;   // Bandera. 1=muerto

BEGIN
    region=1;           // Elige regi¢n
    info=tipo_enemigo;  // Tipo de enemigo

    // Selecciona el bucle de animaci¢n y el gr fico ha asignar
    n_animacion_s=animacion_enemigo[tipo_enemigo].n_imagenes;
    graph=animacion_enemigo[tipo_enemigo].graficos[animacion];

    // Elige los puntos que dara el enemigo
    puntos=(tipo_enemigo+3)*50;
    energia=tipo_enemigo+1;                 // Elige la energ¡a del enemigo
    IF (tipo_enemigo==5) energia=40; END    // Para el enemigo final

    // Elige la secci¢n y el n£mero de imagenes de por segundo
    numero_seccion=trayectoria[tray].n_seccion;
    imagenes=trayectoria[tray].trayec[actual_seccion].imagener_por_seccion;

    velocidad_x=trayectoria[tray].trayec[actual_seccion].velocidad_x;    // Velocidad de secci¢n x
    velocidad_y=trayectoria[tray].trayec[actual_seccion].velocidad_y;    // Velocidad de secci¢n y
    aceleraci¢n_x=trayectoria[tray].trayec[actual_seccion].aceleraci¢n_x;    // Aceleraci¢n de secci¢n x
    aceleraci¢n_y=trayectoria[tray].trayec[actual_seccion].aceleraci¢n_y;    // Aceleraci¢n de secci¢n y

    x=trayectoria[tray].x_ini;      // Selecciona las coordenadas iniciales
    y=trayectoria[tray].y_ini;

    IF (tipo_enemigo==4)            // Las minas son colocadas al azar
        y=rand(50,430);
        sound(sonido8,512,256);

    END

    vivos[n_grupo]++;               // Incremento el n£mero de enemigos del grupo

    LOOP    // Bucle principal

        // Movimiento enemigo
        IF (contador>=imagenes)
            actual_seccion++;
            contador=0;
            IF (actual_seccion>=numero_seccion)
                actual_seccion=0;
            END

            // Nueva secci¢n
            imagenes=trayectoria[tray].trayec[actual_seccion].imagener_por_seccion;
            velocidad_x=trayectoria[tray].trayec[actual_seccion].velocidad_x;    // Velocidad secci¢n x
            velocidad_y=trayectoria[tray].trayec[actual_seccion].velocidad_y;    // Velocidad secci¢n y
            aceleraci¢n_x=trayectoria[tray].trayec[actual_seccion].aceleraci¢n_x;    // Aceleraci¢n secci¢n x
            aceleraci¢n_y=trayectoria[tray].trayec[actual_seccion].aceleraci¢n_y;    // Aceleraci¢n secci¢n y
        END
        velocidad_x+=aceleraci¢n_x;
        velocidad_y+=aceleraci¢n_y;
        x+=velocidad_x;
        y+=velocidad_y;

        // Anima el gr fico
        animacion++;
        IF (animacion>=n_animacion_s)
            animacion=0;
        END
        graph=animacion_enemigo[tipo_enemigo].graficos[animacion];

        tocado=0;   // Reinicia la variable que controla si se ha tocado a un enemigo

        // Comprueba si el enemigo choca con el escudo de bolas
        IF (energia>tocado AND (id_colision=collision(TYPE escudo_bolas)))
            explosion(rand(0,2),rand(0,19)+30);
            IF (info!=5)        // El enemigo final no es afectado por el escudo
                tocado++;       // El enemigo ha sido tocado por el escudo
            END
        END

        // Comprueba si el enemigo choca con una bala
        IF (energia>tocado AND (id_colision=collision(TYPE bala)))
            explosion(rand(0,2),rand(0,19)+20);
            signal(id_colision,s_kill);     // Quita la bala
            tocado++;                       // El enemigo a sido tocado
        END

        IF (tocado)
            IF (graph==240)         // Enemigo es invulnerable
                graph=248;
                sound(sonido5,256,256);
            ELSE
                IF (graph==600)
                    graph=601;
                END
                energia-=tocado;    // EL enemigo ha sido tocado
            END
        END

        // Comprueba si el enemigo esta muerto
        IF (energia<=0)
            muerto=1;
            IF (info!=5)
                explosion(rand(0,2),80);
            ELSE                // Enemigo final muerto
                xtiempo=x;      // Guarda las coordenadas para dejar la explosi¢n
                ytiempo=y;
                FROM contador1=0 TO 19;
                    FROM contador2=0 TO 4;
                        x=xtiempo+rand(0,99)-50;
                        y=ytiempo+rand(0,99)-50;
                        explosion(rand(0,2),contador1*5+contador2+1);
                    END
                    // Actualiza las coordenadas
                    x=xtiempo;
                    y=ytiempo;
                    FRAME;
                END
            END
            puntuacion+=puntos;  // Puntos para el jugador
        END
        // Crea un valor aleatorio y lo comprueba con la agresividad
        IF (rand(0,199)<agresividad)
            // El enemigo tipo 4 no dispara, los dem s si
            IF ((tipo_enemigo!=4) AND (graph<>247))
                bala_enemigo(tipo_enemigo,x,y,get_angle(id_nave_princ));
            END
        END

        FRAME;
        contador++;
        // Comprueba si ha desaparecido este proceso de pantalla o esta muerto
        IF ((out_region(id,1)) OR (muerto))
            IF (muerto==0)
                no_bonus[n_grupo]=1;
            END
            vivos[n_grupo]--;       // Resta un enemigo
            IF ((vivos[n_grupo]==0) AND (no_bonus[n_grupo]==0))
                bonus_x=x;          // Selecci¢n la posici¢n de los bonus en la pantalla
                bonus_y=y;
            END
            RETURN();               // El proceso acaba, saliendo con RETURN
        END
    END
END

//-----------------------------------------------------------------------------
//  Proceso grupo
//  Crea un horda de enemigos
//-----------------------------------------------------------------------------

PROCESS grupo(n_grupo);

PRIVATE
    n_enemigos_g;           // Numero de enemigos en el grupo
    contador_enemigos=0;    // Contador de enemigos en el grupo
    pausa_contador=1;       // Contador de tiempo de espera entre enemigos
    tipo_enem=0;            // Tipo de enemigo
    trayectoria1;           // Trayectoria
    pausa_enemigo;          // Pausa entre enemigo
    agresividad=0;          // Agresividad

BEGIN
    grupo_pantalla++;       // Este es de otro grupo
    vivos[n_grupo]=0;       // Numero de enemigos vivos en el grupo
    no_bonus[n_grupo]=0;    // Pon bonus para el grupo
    IF (grupo_actual==ultimo_grupo)     // Enemigo final
        tipo_enem=5;
        trayectoria1=0;
        n_enemigos_g=0;
        agresividad=30;
    ELSE
        n_enemigos_g=rand(2,5);         // Numero de enemigos
        tipo_enem=rand(0,4);            // 5 es el enemigo final
        IF (tipo_enem==4)               // Cuando el enemigo es una mina
            trayectoria1=1;             // Fija una trayectoria fija
            n_enemigos_g+=3;            // Haz mas minas
        ELSE
            trayectoria1=rand(2,max_trayec);    // Pon la trayectoria
            agresividad=rand(0,5-grupo_pantalla)+nivel_actual;    // Pon la agresividad
        END
        pausa_enemigo=rand(5,15);
    END

    REPEAT
        pausa_contador--;               // Bucle de retardo
        IF (pausa_contador<=0)
            // Crea un enemigo
            enemigo(tipo_enem,trayectoria1,agresividad,n_grupo);
            pausa_contador=pausa_enemigo;
            contador_enemigos++;
        END
        FRAME;
    UNTIL (contador_enemigos>n_enemigos_g)
    // Repite hasta que cree todos los enemigos necesarios
    REPEAT
        FRAME;
    UNTIL (vivos[n_grupo]==0)       // Espera al ultimo enemigo
    FRAME;

    IF (no_bonus[n_grupo]==0)       // Crea los bonus
        IF (rand(0,2)==0)
            bonus(bonus_x,bonus_y,7);
        ELSE
            bonus(bonus_x,bonus_y,rand(1,7));
        END
    END
    grupo_pantalla--;               // Cambia de grupo

END

//-----------------------------------------------------------------------------
//  Proceso juego
//  Controla los grupos y los bonus
//-----------------------------------------------------------------------------

PROCESS juego();

PRIVATE
    contador1=0;    // Contador general
    pausa_grupo;    // Pausa entre grupos

BEGIN
    clear_screen();                     // Borra la pantalla
    set_fps(12,0);                      // Selecciona las imagenes por segundo
    estado_programa=-2;                 // Pone el estado del programa en jugando
    // Carga los ficheros de gr ficos necesarios
    load_pal("blastup\juego.fpg");
    fichero1=load_fpg("blastup\juego.fpg");
    put(fichero1,2,320,20);             // Marcador superior
    put(fichero1,3,320,460);            // Marcador inferior
    define_region(1,0,41,640,398);      // Regi¢n en movimiento
    start_scroll(0,fichero1,1,0,1,3);   // Inicializa el movimiento de pantalla
    fade_on();
    FRAME;
    n_vidas=vidas_max;                  // Inicializa el n£mero de vidas
    puntuacion=0;                       // y tambi‚n la puntuaci¢n
    id_nave_princ=nave_principal();     // Crea la nave del jugador

    FOR (contador1=0;contador1<n_vidas;contador1++)
        id_vidas[contador1]=vida(contador1);   // Crea las naves peque¤as (vidas)
    END

    FROM contador1=0 TO 4;                     // Reinicia la tabla de bonus
        id_bonus[contador1]=0;
    END

    write_int(fuente2,360,454,2,&puntuacion);   // Escribe la puntuaci¢n

    // Inicializa otras variable
    ultimo_grupo=109;                   // N£mero de grupos 0..109
    grupo_actual=0;                     // Grupo actual
    grupo_pantalla=0;                   // Grupos en pantalla
    nivel_actual=0;                     // Fase  actual
    pausa_grupo=200;                    // Pausa entre grupos

    contador1=0;
    velocidad_movim=normal_movim;
    LOOP

        // Enemigo final esta muerto
        IF ((grupo_actual>ultimo_grupo) AND (grupo_pantalla==0))
            nuevo_estado=0;
            write(fuente3,320,220,4,"ENHORABUENA");
            BREAK;
        END
        // Comprueba si hay que esperar hasta que salga un grupo de naves
        IF (grupo_actual<=ultimo_grupo)
            pausa_grupo++;
        END

        IF (((pausa_grupo*(nivel_actual+1)*3/2)>500) AND (grupo_pantalla<=(nivel_actual/3)))
            pausa_grupo=rand(200,450);
            IF ((grupo_actual MOD 10)==0)       // Mira si ha cambiado de nivel
                titulo_nivel(nivel_actual+1);
                nivel_actual++;                 // Cambia de nivel
                FROM contador1=0 TO 99;
                    IF (key(_esc))              // Comprueba si se pulsa la tecla escape
                        nuevo_estado=1;
                        write(fuente3,320,240,4,"FIN DE JUEGO");
                        FRAME(2000);
                        stop_scroll(0);
                        FRAME;
                        fade_off();
                        estado_programa=nuevo_estado;
                        delete_text(all_text);
                        signal(id,s_kill_tree);
                        signal(type bonus,s_kill);
                        unload_fpg(fichero1);
                        clear_screen();
                        fade_on();
                        RETURN;
                    END
                    FRAME;
                    scroll.x0+=velocidad_movim;
                END
            END
            contador1=0;

            // Buscando un hueco libre en vivos
            WHILE (vivos[contador1]>0) contador1++; END

            grupo(contador1);       // Crea un grupo
            grupo_actual++;         // Incrementa el contador de grupos
        END

        // Para el movimiento de pantalla cuando se llegue al enemigo final
        IF (grupo_actual>ultimo_grupo)
            velocidad_movim=0;
        ELSE
            velocidad_movim=normal_movim;
        END

        scroll.x0+=velocidad_movim;  // Realiza el movimiento de pantalla

        // Mira si se ha conseguido una vida extra
        contador1=0;
        WHILE (id_bonus[contador1] AND contador1<5)
            contador1++;
        END

        IF (contador1==5)           // Se ha conseguido una vida extra
            sound(sonido7,256,256);

            // Borra las letras del marcador
            FROM contador1=0 TO 4;
                signal(id_bonus[contador1],s_kill);
                id_bonus[contador1]=0;
            END

            // Crea un escudo cuando se tenga el m ximo de vidas extras
            IF (n_vidas<vidas_max)
                id_vidas[n_vidas]=vida(n_vidas);
                n_vidas++;
            ELSE
                escudo();
            END
        END
        FRAME;
        // El juego se acaba si se pulsa escape o no quedan vidas
        IF (n_vidas<0 OR key(_esc))
            nuevo_estado=1;
            write(fuente3,320,240,4,"FIN DE JUEGO");
            BREAK;
        END
    END

    FRAME(1000);                    // Hace una peque¤a pausa al final
    stop_scroll(0);                 // Para el movimiento de pantalla
    FRAME;
    fade_off();                     // Apaga la pantalla
    estado_programa=nuevo_estado;   // Pone el estado del program en menu
    delete_text(all_text);          // Borra todos los textos
    signal(id,s_kill_tree);         // Borra cualquier proceso
    signal(type bonus,s_kill);
    unload_fpg(fichero1);           // Descarga el fichero de gr ficos
    clear_screen();                 // Borra la pantalla
    fade_on();
END

//-----------------------------------------------------------------------------
//  Proceso Titulo
//  Opciones de men£ general
//-----------------------------------------------------------------------------

PROCESS titulo();

BEGIN
    estado_programa=-1;                         // Cambia el estado del programa
    nuevo_estado=0;
    fichero1=load_fpg("blastup\titulo.fpg");    // Carga los gr ficos
    put_screen(fichero1,1);                     // Pone la pantalla de fondo
    load_pal("blastup\titulo.fpg");             // Carga la paleta de colores
    FRAME;
    fade_on();

    // Textos del Men£ General
    write(fuente1,8,340,0,"1 Jugar");
    write(fuente1,8,370,0,"2 Creditos");
    write(fuente1,8,400,0,"3 Salir al DOS");
    write(fuente1,320,460,4,"(c) 1997 DIV Games Studio - Hammer Technologies");

    // Bucle del men£ general
    WHILE (nuevo_estado==0)

        // Lee las teclas y pone el estado en consecuencia
        IF (key(_1) OR key (_space) OR key (_enter)) nuevo_estado=2; END
        IF (key(_2)) nuevo_estado=3; END
        IF (key(_3) OR key (_esc)) nuevo_estado=4; END

        FRAME;
    END

    // Salir del men£ general
    fade_off();
    unload_fpg(fichero1);           // Descarga gr ficos
    delete_text(all_text);          // Borra textos
    estado_programa=nuevo_estado;   // Cambia el programa al nuevo estado
END

//-----------------------------------------------------------------------------
//  Proceso cr‚ditos
//  Muestra los cr‚ditos del programa
//-----------------------------------------------------------------------------

PROCESS creditos();

BEGIN
    estado_programa=-3;
    load_pal("blastup\creditos.fpg");               // Selecciona paleta de colores
    fichero1=load_fpg("blastup\creditos.fpg");      // Selecciona gr ficos
    put_screen(fichero1,1);                         // Muestra el fondo de pantalla
    FRAME;
    fade_on();

    // Escribe los cr‚ditos
    write(fuente4,40,0,0,"Programadores:");
    write(fuente4,100,40,0,"Manuel Caba¤as");
    write(fuente4,100,80,0,"Luis Sureda");
    write(fuente4,40,300,0,"Graficos:");
    write(fuente4,100,340,0,"Jos‚ Fern ndez");
    write(fuente4,40,390,0,"Musica:");
    write(fuente4,100,430,0,"Moises D¡az Toledano");

    FRAME;

    scan_code=0;
    // Espera a pulsar una tecla
    WHILE (scan_code==0)
        FRAME;
    END
    fade_off();
    delete_text(all_text);
    unload_fpg(fichero1);
    estado_programa=1;        // Retorna al men£ general
END

//-----------------------------------------------------------------------------
//  Proceso nave_principal
//  Maneja todas las acciones de la nave principal ( nave del jugador)
//-----------------------------------------------------------------------------

PROCESS nave_principal();

PRIVATE

    disparando=0;   // Evita el disparo continuo

    banco;          // Maneja el movimiento y la inercia
    inc_x=0;        // Incrementos horizontal y verical
    inc_y=0;

    tipo_disparo;   // Tipo de disparo de la nave (0..disparo_max)

BEGIN
    fichero1=fichero1;
    graph=100;     // Selecciona gr fico
    x=32;          // y coordenadas
    y=240;
    z=-2;
    region=1;
    // Inicializa variables
    estado_nave=1;
    tipo_disparo=0;
    energia=1;
    // Imprime el tipo de disparo seleccionado
    write_int(fuente2,600,448,2,&tipo_disparo);  // Tipo de disparo en el marcador inferior

    FRAME;

    LOOP

        // Movimiento de inercia
        IF (inc_x>0) inc_x-=4; END
        IF (inc_x<0) inc_x+=4; END
        IF (inc_y>0) inc_y-=2; END
        IF (inc_y<0) inc_y+=2; END

        // Control de la nave
        IF (key(_q) OR key(_up))
            inc_y=-12;
            banco--;
        END
        IF (key(_a) OR key(_down))
            inc_y=12;
            banco++;
        END
        IF (key(_o) OR key(_left))
            inc_x=-24;
        END
        IF (key(_p) OR key(_right))
            inc_x=24;
        END

        // Limitando el banco de inercia
        IF (banco>15) banco-=16; END
        IF (banco<-16) banco+=16; END

        // Banco de inercia
        IF (inc_y>-6 AND inc_y<6)
            IF (banco>0) banco--; END
            IF (banco<0) banco++; END
        END

        // Cambia el gr fico
        IF (banco<0)
            graph=116+banco;
        ELSE
            graph=100+banco;
        END

        // Limita la posici¢n de la nave
        IF ((y+inc_y)<66)
            y=66;
            inc_y=0;
        END
        IF ((y+inc_y)>414)
            y=414;
            inc_y=0;
        END
        IF ((x+inc_x)<30)
            x=30;
            inc_x=0;
        END
        IF ((x+inc_x)>610)
            x=610;
            inc_x=0;
        END
        x+=inc_x;
        y+=inc_y;

        // Control del disparo
        IF (key(_space) OR key(_enter) OR key(_control))
            IF (disparando==0)
                disparo(tipo_disparo);
            END
            disparando++;      // Tecla pulsada
        ELSE
            disparando=0;      // Tecla soltada
        END

        // Mira si hay colisi¢n con el enemigo
        IF (energia>0 AND (id_colision=collision(TYPE enemigo)))
            IF (id_colision.info!=5)        // El enemigo final no afecta
                IF (id_colision.graph!=240) // Entonces  rmalo
                    id_colision.energia--;
                END
            END
            // Decrementa la energ¡a si no tienes escudo
            IF (estado_nave!=2) energia--; END
        END

        // Mira si hay colisi¢n con los misiles o el laser enemigo
        IF (energia>0 AND (id_colision=collision(TYPE bala_enemigo)))
            signal(id_colision,s_kill);
            // Decrementa energ¡a si no hay escudo
            IF (estado_nave!=2) energia--; END
        END

        // Se comprueba si la nave ha muerto
        IF (energia<=0)
            n_vidas--;
            IF (tipo_disparo>0) tipo_disparo--; END
            graph=0;                            // La nave desaparece en este momento
            explosion(rand(0,2),100);
            IF (n_vidas<0) BREAK; END           // No hay mas vidas
            signal(id_vidas[n_vidas],s_kill);   // Una vida menos en el marcador
            FRAME(200);
            escudo();                           // Inicia un escudo
            graph=100;                          // La nave aparece otra vez
            energia=1;
        END

        // Mira si se ha recogido bonus
        IF ((id_colision=collision(TYPE bonus)))
            sound(sonido6,256,256);
            SWITCH (id_colision.info);
                CASE 1..5:          // Bonus
                    letra(id_colision.info);
                END
                CASE 6:             // $
                    puntuacion+=1000;
                END
                CASE 7:             // Atomo
                    tipo_disparo++;

                    // Si tiene el tipo de disparo m ximo, coloca entonces un escudo
                    IF (tipo_disparo>disparo_max)
                        tipo_disparo=disparo_max;
                        escudo();
                    END

                END
            END
            signal(id_colision,s_kill);     // Borra los bonus de la pantalla
        END

        FRAME;
    END
    signal(id,s_kill_tree);                 // Borra el escudo
END

//-----------------------------------------------------------------------------
//  Proceso vida
//  Muestra una peque¤a nave en el marcador inferior
//  Posiciones posibles: 1..vidas_max
//-----------------------------------------------------------------------------

PROCESS vida(posicion);

BEGIN
    y=460;
    x=posic_x_vidas[posicion];      // Pon en coordenadas
    graph=450;                      // y selecciona gr fico
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso letra
//  Muestra una letra de bonus en el marcador superior
//  Posiciones posibles: 1..5
//-----------------------------------------------------------------------------

PROCESS letra(posicion);

BEGIN

    // Si la letra ya esta puesta entonces retorna
    IF (id_bonus[posicion-1])
        RETURN();
    END
    id_bonus[posicion-1]=id;            // Guarda el identificador para luego poder eliminarlo
    y=21;
    x=posic_x_bonus[posicion-1];        // Selecciona coordenadas
    graph=320+posicion*10;              // y gr ficos
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
//  Proceso BONUS
//  Imprime los bonus del jugador
//      1..5 BONUS
//      6    $
//      7    Atomo
//-----------------------------------------------------------------------------

PROCESS bonus(x,y,info);

PRIVATE
    animacion=0;        // Contador de animaci¢n
    grafico_base=0;     // Primer gr fico de la animaci¢n

BEGIN
    z=1;
    priority=1;
    fichero1=fichero1;
    region=1;           // Pon coordenadas
    SWITCH (info);      // y prepara los gr ficos
        CASE 1..5:
            grafico_base=320+info*10;
        END
        CASE 6:
            grafico_base=300;
        END
        CASE 7:
            grafico_base=320;
        END
        DEFAULT:
            RETURN();
        END
    END
    REPEAT
        x-=6;           // Mueve el gr fico hacia la izquierda
        graph=grafico_base+animacion;
        FRAME;
        animacion++;    // Realiza la animaci¢n
        IF (animacion==8)
            animacion=0;
        END
    UNTIL (out_region(id,1))

END

//-----------------------------------------------------------------------------
//  Proceso disparo
//  Maneja del numero de balas de acuerdo con el tipo_disparo
//  Posibles entradas (seg£n tipo_disparo): 0..disparo_max
//-----------------------------------------------------------------------------

PROCESS disparo(info);

BEGIN
    sound(sonido1,32*(info+1),256); // Realiza sonido

    SWITCH (info);          // Comprueba el n£mero de balas
        CASE 0:             // Una bala
            bala(father.x,father.y,0);
        END
        CASE 1:             // Dos balas
            bala(father.x,father.y-6,0);
            bala(father.x,father.y+6,0);
        END
        CASE 2:             // Dos balas con otra trayectoria
            bala(father.x,father.y-6,pi/32);
            bala(father.x,father.y+6,-pi/32);
        END
        CASE 3:             // Tres balas
            bala(father.x,father.y-8,pi/32);
            bala(father.x,father.y,0);
            bala(father.x,father.y+8,-pi/32);
        END
        CASE 4:             // Cuatro balas
            bala(father.x,father.y-8,pi/32);
            bala(father.x,father.y-4,0);
            bala(father.x,father.y+4,0);
            bala(father.x,father.y+8,-pi/32);
        END
    END
END

//-----------------------------------------------------------------------------
//  Proceso bala
//  Disparo de la nave principal
//-----------------------------------------------------------------------------
PROCESS bala (x,y,ang)

BEGIN
    fichero1=fichero1;
    region=1;
    z=1;
    graph=400;      // Selecciona gr ficos
    x+=34;
    REPEAT
        FRAME;      // Maneja coordenadas
        IF (ang)    // Seg£n el  ngulo seleccionado
           x+=get_distx(ang,36);
           y+=get_disty(ang,36);
        ELSE
           x+=36;
        END
    UNTIL (out_region(id,1))
END

//-----------------------------------------------------------------------------
//  Proceso bala_enemigo
//  Disparo de las naves enemigas
//-----------------------------------------------------------------------------
PROCESS bala_enemigo (info,x,y,bala_angle)

BEGIN
    fichero1=fichero1;
    region=1;
    z=1;
    SWITCH (info)       // Tipo de movimiento y gr fico del disparo
        case 0 :
            graph=408;
            x-=32;
            y+=6;
            sound(sonido2,256,256);
        END
        case 1 :
            graph=402;
            x-=32;
            sound(sonido2,256,256);
        END
        case 2 :
            graph=403;
            x-=32;
            sound(sonido2,256,256);
        END
        case 3 :
            graph=404;
            x-=32;
            y+=6;
            sound(sonido3,256,256);
        END
        case 5 :
            graph=410;
            x-=90;
            y-=18;
            sound(sonido2,256,256);
        END
        default :
            RETURN();
        END
    END

    REPEAT
        FRAME;
        // Mueve el disparo dependiendo del tipo
        SWITCH (info)
            case 0 :
                x+=get_distx(bala_angle,8);
                y+=get_disty(bala_angle,8);
            END
            case 1 :
                x-=20;
            END
            case 2 :
                x+=get_distx(bala_angle,10);
                y+=get_disty(bala_angle,10);
            END
            case 3 :
                x-=24;
            END
            case 5 :
                x+=get_distx(bala_angle,14);
                y+=get_disty(bala_angle,14);
                IF (graph==410)
                    graph=411;
                ELSE
                    graph=410;
                END
            END
        END

        IF (id_colision=collision(type escudo_bolas))
           // Genera una explosi¢n y acaba este proceso
           explosion(rand(0,2),rand(0,19)+30);
           RETURN();
        END

        // Efecto de humo para los misiles
        IF (info==3) humo(); END

    // Acaba si sale de pantalla
    UNTIL(out_region(id,1));
END


//-----------------------------------------------------------------------------
//  Proceso humo
//  Efecto de humo para los misiles
//-----------------------------------------------------------------------------

PROCESS humo();

BEGIN
    fichero1=fichero1;
    region=1;
    z=2;
    flags=4;                    // Se hace transparente
    graph=405;                  // Pone el gr fico
    x=father.x+12+rand(0,4);    // y las coordenadas iniciales
    y=father.y;
    size=100;
    REPEAT                      // Hace la animaci¢n
        FRAME;
        size-=20+rand(0,9);
    UNTIL (size<10)
END

//-----------------------------------------------------------------------------
//  Proceso explosi¢n
//  Pinta y anima una explosi¢n
//  Tipos de explosi¢n: 0..2
//-----------------------------------------------------------------------------

PROCESS explosion(info,size);

PRIVATE
    animacion;        // Contador de la animaci¢n
    grafico_base;     // Primera gr fico de la animaci¢n

BEGIN
    // Inicializa variables
    animacion=0;
    fichero=fichero1;
    region=1;
    x=father.x+rand(0,14)-7;
    y=father.y+rand(0,14)-7;    // Posicionalo de forma aleatoria
    z=-1;
    grafico_base=500+info*10;
    graph=grafico_base;         // Selecciona el gr fico inicial
    sound(sonido4,size*3,256);
    REPEAT                      // Realiza la animaci¢n
        FRAME;
        x-=6;
        graph++;
        animacion++;
    UNTIL (animacion==6)
END

//-----------------------------------------------------------------------------
//  Proceso escudo
//  Crea un escudo de 8 bolas alrededor de la nave en forma de elipse
//-----------------------------------------------------------------------------

PROCESS escudo();

BEGIN
    estado_nave=2;                  // Nave invulnerable
    FROM info=0 TO 7;
        escudo_bolas(info);         // Crea ocho bolas del escudo
    END
END

//-----------------------------------------------------------------------------
//  Proceso escudo_bolas
//  Controla las bolas del escudo
//-----------------------------------------------------------------------------

PROCESS escudo_bolas(info);

PRIVATE
    contador=0;     // Contador de uso general
     ngulo_giro;    // Angulo de giro

BEGIN
     ngulo_giro=info*(pi/4);    // Va cambiando el  ngulo seg£n n£mero de bola
    region=1;
    // Coge las coordenadas a partir de la nave y en forma circular
    x=id_nave_princ.x+get_distx( ngulo_giro,70);
    y=id_nave_princ.y+get_disty( ngulo_giro,50);
    z=2;

    IF (info)       // Anima las bolas gr ficamente
        graph=406;
    ELSE
        graph=407;
    END
    FRAME;
    REPEAT
         ngulo_giro-=pi/16;
        contador++;
        IF (graph==406)         // Continua realizando la animaci¢n
            graph=407;
        ELSE
            graph=406;
        END
            x=id_nave_princ.x+get_distx( ngulo_giro,70);
            y=id_nave_princ.y+get_disty( ngulo_giro,50);
        FRAME;
    UNTIL (contador>72+info*4)
    REPEAT
        x-=30;                  // El escudo se acaba, se quitan las bolas
        FRAME;
    UNTIL (x<0)
    IF (info==7) estado_nave=1; END  // La nave ahora es vulnerable
END

//-----------------------------------------------------------------------------
//  Proceso titulo_nivel
//  Muestra el titulo que sale al cambiar de nivel
//-----------------------------------------------------------------------------

PROCESS titulo_nivel(info);

PRIVATE
    id_text;    // Identificador de texto
    id_n£mero;  // Identificador de texto
    contador=5; // Contador

BEGIN
    REPEAT

        // Escribe el texto
        id_text=write(fuente3,340,140,5,"NIVEL");
        id_n£mero=write_int(fuente3,370,140,3,&info);
        FRAME(400); // Espera 4 pantallazos

        // Borra el texto
        delete_text(id_text);
        delete_text(id_n£mero);
        FRAME(400); // Espera 4 pantallazos
        contador--;
    // Se hace hasta 5 veces
    UNTIL (contador<1)
END

