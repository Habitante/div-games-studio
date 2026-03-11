
//------------------------------------------------------------------------------
//TITULO:      NOID
//AUTOR:       DANIEL NAVARRO
//FECHA:       2/15/97
//------------------------------------------------------------------------------

PROGRAM noid;

CONST
    velocidad_r=8;  // Velocidad m xima de la raqueta

GLOBAL
    peque¤o_r[2];   // Identificador para las vidas del marcador

    s_golpe;        // Sonido de toque de ladrillo
    s_pildora;      // Sonido de la p¡ldoras de bonos
    s_fuego;        // Sonido del disparo
    s_metal;        // Sonido de toque con ladrillo met lico
    s_raqueta;      // Sonido de la raqueta

    mapa[16*14];    // Mapa
    vidas;          // Vidas restantes
    nbolas;         // Numero de bolas
    nladrillos;     // Numero de ladrillos que quedan
    camb_dir;       // Cambia el control direccional de la paleta
    rebota;         // Bandera. 1=rebota con ladrillos (pelota normal).
                    //       0=no rebota (superbola)
    fin_juego;      // Bandera 1=Final del juego
    fase;           // Fase actual
    puntuacion;     // Puntuaci¢n
    demo;           // Bandera. 1=demo
    id_bola;        // Identificador de la bola para la demo
    id_texto;       // Identificador del texto del men£

    // Fichero de las fases (mapas)
    fases[]=
             "dat\noid\screen.1",
             "dat\noid\screen.2",
             "dat\noid\screen.3",
             "dat\noid\screen.4",
             "dat\noid\screen.5",
             "dat\noid\screen.6",
             "dat\noid\screen.7",
             "dat\noid\screen.8",
             "dat\noid\screen.9",
             "dat\noid\screen.10",
             "dat\noid\screen.11",
             "dat\noid\screen.12";

LOCAL
    estado=0;           // En el proceso bola: Bandera. 1=hace otra bola. 0=normal
                        // En el proceso ladrillo: numero del contador de gr ficos
    tama¤o_raqueta=4;   // Tama¤o de la raqueta
    parado=0;           // Bandera. 1=Bola parada. 0=bola en movimiento
    velocidad=0;        // Velocidad de la bola

BEGIN
    set_fps(30,0);              // Numero de pantallas por segundo
    load_fpg("noid\noid.fpg");  // Carga gr ficos
    load_fpg("noid\noid2.fpg"); // Carga pantalla de presentaci¢n

    s_golpe=load_pcm("noid\caida7.pcm",0);    // Carga sonido
    s_pildora=load_pcm("noid\fx34.pcm",0);
    s_fuego=load_pcm("noid\disparo5.pcm",0);
    s_metal=load_pcm("noid\metal10.pcm",0);
    s_raqueta=load_pcm("noid\tubo.pcm",0);

    load_fnt("noid\noid.fnt");      // Carga la fuente de letras

    LOOP
        scan_code=0;
        WHILE (scan_code<>0)        // Espera a que se suelte la tecla
            FRAME;
        END
        // Borra cualquier texto o proceso que existiera
        delete_text(all_text);
        let_me_alone();
        put_screen(1,1);            // Pone la pantalla de fondo
        load_pal("noid\noid2.fpg");
        fade_on();
        // Espera a que se pulse una tecla para continuar
        REPEAT
            FRAME;
        UNTIL (scan_code<>0)
        fade_off();
        load_pal("noid\noid.fpg");
        fin_juego=0;
        demo=1;                                     // Empieza el modo demo
        texto_demo();                               // Crea un proceso que pone un texto explicativo

        write_int(1,310,180,2,&puntuacion);         // Imprime puntuaci¢n

        // Valores iniciales
        fin_juego=0;
        fase=1;
        puntuacion=0;
        vidas=3;

        peque¤o_r[0]=peque¤a_raq(296,16+11*0);      // Imprime las vidas restantes
        peque¤o_r[1]=peque¤a_raq(296,16+11*1);
        peque¤o_r[2]=peque¤a_raq(296,16+11*2);
        scan_code=0;
        REPEAT
            fade_on();              // Enciende la pantalla
            clear_screen();         // Borra la pantalla
            put(0,100+fase%4,0,0);  // Pone los gr ficos de fondo
            put(0,900,319,0);
            nbolas=0;               // Reinicializa variables
            camb_dir=1;
            rebota=1;
            id_bola=bola(raqueta());                // Crea la bola y la raqueta

            // Carga la fase y pone los ladrillos en la pantalla
            load(fases[fase],&mapa);
            nladrillos=0;   // Sirve para contar los ladrillos que se crean y pueden ser destruidos
            FROM y=0 TO 13; // Va creando todos los ladrillos
                FROM x=0 TO 15;
                    IF (mapa[y*16+x]<>0)
                        ladrillo(16+x*16,12+y*8,mapa[y*16+x]);
                        // Si el ladrillo se puede destruir entonces los cuenta
                        IF ((mapa[y*16+x]<20) OR (mapa[y*16+x]>=30))
                            nladrillos++;
                        END
                    END
                END
            END
            write_int(1,310,180,2,&puntuacion); // Imprime puntuaci¢n
            LOOP    // Bucle general
                IF (key (_esc))                 // Comprueba la pulsaci¢n de escape
                    IF (demo)                   // Si esta en modo demo, sale del programa
                        creditos();
                        FRAME;
                    ELSE
                        let_me_alone();         // Reinicia todo y sale al modo demo
                        puntuacion=0;
                        demo=1;
                        fin_juego=1;            // Valores iniciales
                        texto_demo();
                    END
                END
                FRAME;
                // Cuando nladrillos<=0 y no esta cayendo nada, entonces la fase esta completada
                IF (((nladrillos<=0) OR (fin_juego)) AND (NOT (get_id(type pildora))) AND (NOT (get_id(type reduce))))
                    BREAK;
                END
            END

            // Siguiente fase o una vida menos
            signal(TYPE bola,s_kill);
            signal(TYPE raqueta,s_kill);
            signal(TYPE pildora,s_kill);
            signal(TYPE laser,s_kill);
            signal(TYPE caido,s_kill);
            signal(TYPE reduce,s_kill);
            signal(TYPE ladrillo,s_kill);

            fase++;         // Cambia de fase
            fade_off();     // Apaga la pantalla
        UNTIL ((fase>11) OR (fin_juego))
        // Repite hasta que se acabe el juego o se hagan todas las fases
        fin_juego=0;
        fade_off();
        // Borra cualquier texto, la pantalla y la raqueta de la vidas
        delete_text(all_text);
        clear_screen();
        signal(type peque¤a_raq,s_kill);
    END
    signal(id,s_kill_tree);
END

//------------------------------------------------------------------------------
// Proceso raqueta
// Controla la raqueta
//------------------------------------------------------------------------------


PROCESS raqueta();

PRIVATE
    incr_x=0;               // Incremento x
    incr_y=0;               // Incremento y
    fuego_preparado=1;      // Bandera. 1=Disparo disponible
    id2;                    // Identificador general
    id3;                    // Identificador de car cter general

BEGIN
    graph=3;                // Empieza con una raqueta normal
    x=140;                  // Coordenadas iniciales
    y=188;
    // Espera si no hay bola
    WHILE(NOT id_bola) FRAME; END
    LOOP

        IF (NOT demo)       // El jugador controla la raqueta

            IF (camb_dir==1)
                // Comprueba la pulsaci¢n de las teclas de los cursores
                IF (key(_left) AND incr_x>-velocidad_r)
                    incr_x-=4;  // Acelera
                ELSE
                    IF (key(_right) AND incr_x<velocidad_r)
                        incr_x+=4;  // Acelera
                    ELSE            // Ninguna tecla pulsada
                        IF (incr_x>0)
                            incr_x-=2;
                        END         // Frena la raqueta
                        IF (incr_x<0)
                            incr_x+=2;
                        END
                    END
                END
            END
            IF (camb_dir==-1)   // Mira si se ha cogido el icono de invertir mandos
                // Lee los mandos al reves
                IF (key(_right) AND incr_x>-velocidad_r)
                    incr_x-=4;  // Acelera la raqueta
                ELSE
                    IF (key(_left) AND incr_x<velocidad_r)
                        incr_x+=4;
                    ELSE        // Ninguna tecla pulsada
                        IF (incr_x>0)
                            incr_x-=2;
                        END     // Frena la raqueta
                        IF (incr_x<0)
                            incr_x+=2;
                        END
                    END
                END
            END

        ELSE  // El ordenador controla la raqueta (modo demo)
            IF ((x>7) AND (x<310)) x=id_bola.x; END
            IF (scan_code<>0)     // El jugador toma el control de la raqueta
                delete_text(all_text);          // Borra textos
                signal(type texto_demo,s_kill); // Elimina el proceso que pone el texto explicativo
                demo=0;                         // Quita el modo demo
                nladrillos=0;                   // Reinicia variables
                fase=-1;
                vidas=3;
                puntuacion=0;
                write_int(1,310,180,2,&puntuacion); // Imprime puntuaci¢n
            END
            IF (puntuacion>200) // Si hace m s de 200 puntos cambia de fase (solo modo demo)
                nladrillos=0;
                fase=rand(-1,10);
                puntuacion=0;
            END
        END

        // Comprueba si se pulsa las teclas de disparo y se tiene la modalidad de disparo
        IF ( (key(_space) OR key (_control) OR (demo AND rand(0,10)==0)) AND graph==6)
            IF (fuego_preparado==1) // Si no ha disparado esta vez
                laser(x-16,y-8);    // Dispara los lasers...
                laser(x+16,y-8);
                IF (NOT demo) fuego_preparado=0; END // Y lo deja preparado para disparar de uno en uno
            END
        ELSE
            fuego_preparado=1;      // Permite volver a disparar
        END

        // Comprueba cuando coges una p¡ldora de bonos
        IF (id2=collision(TYPE pildora))
            IF (id2.size==100)                  // Comprueba que no se halla cogido antes
                sound(s_pildora,80,256);
                puntuacion+=50;                 // Suma puntos
                SWITCH (id2.graph);             // Comprueba que tipo de pildora es
                    CASE 200:                   // P¡ldora de extensi¢n
                        IF (tama¤o_raqueta<8)   // Comprueba que extensi¢n tiene actualmente
                            tama¤o_raqueta+=4;
                        END
                        IF (tama¤o_raqueta==4)  // Coloca el gr fico dependiendo del tama¤o
                            graph=3;
                        ELSE
                            graph=4;
                        END
                        // Deja que la bola se mueva si es que estaba en modo pegamento
                        WHILE (id3=get_id(TYPE bola))
                            id3.parado=0;
                        END
                    END
                    CASE 201:               // P¡ldora pegamento
                        tama¤o_raqueta=4;   // Pone el tama¤o en normal
                        graph=5;            // Selecciona el gr fico necesario
                    END
                    CASE 202:               // P¡ldora de disparo
                        tama¤o_raqueta=4;   // Pone el tama¤o en normal
                        graph=6;            // Selecciona el gr fico necesario
                        WHILE (id3=get_id(TYPE bola))
                            id3.parado=0;   // Quita el modo pegamento
                        END
                    END
                    CASE 203:               // P¡ldora de vida extra
                        IF (vidas<3)        // Si se tienen menos de 3 vidas, crea una m s
                            vidas++;
                            peque¤o_r[vidas-1]=peque¤a_raq(296,16+11*(vidas-1));
                        END;
                    END
                    CASE 204:               // P¡ldora de controles invertidos
                        camb_dir=camb_dir*-1;
                    END
                    CASE 205:               // Reduce la raqueta
                        IF (tama¤o_raqueta>0)   // Reduce el tama¤o de la raqueta si se puede
                            tama¤o_raqueta-=4;
                        END
                        IF (tama¤o_raqueta==0)  // Selecciona el gr fico necesario
                            graph=2;
                        ELSE
                            graph=3;
                        END
                        WHILE (id3=get_id(TYPE bola))
                            id3.parado=0;       // Quita el modo pegamento si existiera
                        END
                    END
                    CASE 206:               // P¡ldora de velocidad
                        // Decrementa la velocidad de la bola si se puede
                        WHILE (id3=get_id(TYPE bola))
                            IF (id3.velocidad>800)
                                id3.velocidad-=400;
                            ELSE
                                id3.velocidad=400;
                            END
                        END
                    END
                    CASE 207:                   // P¡ldora de superbola
                        rebota=rebota XOR 1;    // Quita el rebote con los ladrillos
                        WHILE (id3=get_id(TYPE bola))
                            id3.graph=1+(1-rebota)*8;    // Cambia el gr fico
                        END
                    END
                    CASE 208:               // Cambia los ladrillos indestructibles
                        FRAME(0);           // Actualiza procesos
                        // Va mirando los ladrillos y cambia los indestructibles
                        WHILE (id3=get_id(TYPE ladrillo))
                            IF ((id3.graph>=20) AND (id3.graph<30))
                                ladrillo(id3.x,id3.y,14);
                                signal(id3,s_kill);
                                id3.graph=14;
                                nladrillos++;
                            END
                        END
                    END

                    CASE 209:               // P¡ldora de multibola
                        get_id(TYPE bola).estado=1;
                    END

                END
            END
            id2.estado=1;   // Quita ese bonus
        END
        IF (NOT demo) x+=incr_x; END    // Si no esta en demo mueve la paleta

        // Limites de la pantalla seg£n el tama¤o
        IF (x<23+tama¤o_raqueta)
            x=23+tama¤o_raqueta;
            incr_x=0;
        END
        IF (x>248-tama¤o_raqueta)
            x=248-tama¤o_raqueta;
            incr_x=0;
        END
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso bola
// Mueve la bola y comprueba las colisiones
//------------------------------------------------------------------------------

PROCESS bola(id_raqueta);

PRIVATE
    incr_x;         // Incremento x
    incr_y;         // Incremento y
    x_resol;        // 'x' con dos decimales
    y_resol;        // 'y' con dos decimales
    ultima_x_resol; // Ultima posici¢n x_resol
    ultima_y_resol; // Ultima posici¢n y_resol
     ngulo0;        // Movimiento del  ngulo
    avelocidad;     // Velocidad acumulada
    dist_raqueta;   // Distancia entre la raqueta y la bola
    pos_bol_raq;    // Posici¢n de la bola en la raqueta
    id_ladrillo;    // Identificador de ladrillo

BEGIN
    z=-3;
    graph=1+(1-rebota)*8;   // Gr fico de la bola
    velocidad=400;          // Inicia variables propias del proceso
     ngulo0=3*pi/8;

    LOOP

        IF (NOT parado)     // Si no esta en modo pegamento bola nueva
            nbolas++;
            parado=1;
            pos_bol_raq=0;
        ELSE
            pos_bol_raq=x-id_raqueta.x; // Pegamento
        END

        WHILE (parado)  // Repite mientras este en modo pegamento
            // Coge las coordenadas respecto a la raqueta
            x=id_raqueta.x+pos_bol_raq;
            y=id_raqueta.y-8;
            // Si se pulsa la tecla espacio
            IF (key(_space) OR key (_control) OR demo)  // Lanza la bola
                parado=0;
                x_resol=x*100;
                y_resol=y*100;
            END
            FRAME;
        END

        REPEAT

            IF (estado==1)       // Multibola
                nbolas+=2;
                CLONE           // Crea dos bolas m s en otros  ngulos
                     ngulo0+=pi/32;
                    CLONE
                         ngulo0+=pi/32;
                    END
                END
                estado=0;
            END

            avelocidad+=velocidad;  // En cada impresi¢n de pantalla suma avelocidad con velocidad

            WHILE (avelocidad>100)  // Comprueba la trayectoria y la colisi¢n con los ladrillos

                avelocidad-=100;

                ultima_x_resol=x_resol;
                ultima_y_resol=y_resol;

                incr_x=get_distx( ngulo0,100);
                x_resol+=incr_x;
                incr_y=get_disty( ngulo0,100);
                y_resol+=incr_y;

                // Toca la raqueta
                dist_raqueta=x-id_raqueta.x;

                // Toca la bola al ojo de la raqueta
                IF (y_resol>=18000 AND y_resol<=18800 AND incr_y>0 AND (dist_raqueta>-18-id_raqueta.tama¤o_raqueta AND dist_raqueta<18+id_raqueta.tama¤o_raqueta))

                     ngulo0=fget_angle(0,0,incr_x,-incr_y)-(dist_raqueta*pi/60);

                    // Angulo demasiado verticales no est n permitidos
                    IF ( ngulo0<pi/8)
                         ngulo0=pi/8;
                    END
                    IF ( ngulo0>pi*7/8)
                         ngulo0=pi*7/8;
                    END

                    IF (id_raqueta.graph==5) // Raqueta pegamento
                        parado=1;
                        BREAK;
                    END

                    sound(s_raqueta,100,256);
                END

                // Colisiona con el lado horizontal del tablero
                IF (y_resol<=1200 AND incr_y<0)
                     ngulo0=fget_angle(0,0,incr_x,-incr_y);
                    sound(s_raqueta,80,500);
                END

                // Colisiona con los lados verticales de la pantalla
                IF ((x_resol<=1200 AND incr_x<0) OR (x_resol>=26000 AND incr_x>0))
                     ngulo0=fget_angle(0,0,-incr_x,incr_y);
                    sound(s_raqueta,80,600);
                END

                // Actualiza las coordenadas reales
                x=x_resol/100;
                y=y_resol/100;

                // Comprueba si choca con un ladrillo
                IF (id_ladrillo=collision(TYPE ladrillo))
                    puntuacion+=5;      // Suma puntuaci¢n y velocidad a la bola
                    velocidad+=4;
                    FRAME(0);           // Limpia comprobaciones de colisiones para volverlas a coger
                    y=ultima_y_resol/100;

                    // Colisiona con el lado vertical de un ladrillo
                    IF (id_ladrillo==collision(TYPE ladrillo))
                            IF (rebota OR (id_ladrillo.graph>=20 AND id_ladrillo.graph<30))
                            sound(s_metal,100,256);
                            // Cambia el  ngulo de manera vertical
                             ngulo0=fget_angle(0,0,-incr_x,incr_y)+rand(-4000,4000);
                            IF (( ngulo0<pi/8) AND ( ngulo0>-pi/8))
                                 ngulo0=fget_angle(0,0,-incr_x,incr_y);
                            END
                            IF (( ngulo0>pi*7/8) AND ( ngulo0<pi*9/8))
                                 ngulo0=fget_angle(0,0,-incr_x,incr_y);
                            END
                        END

                    ELSE
                        IF (rebota OR (id_ladrillo.graph>=20 AND id_ladrillo.graph<30))
                            sound(s_metal,100,256);

                            // Colisiona con el lado horizontal del ladrillo
                             ngulo0=fget_angle(0,0,incr_x,-incr_y)+rand(-4000,4000);
                            IF (( ngulo0<pi/8) AND ( ngulo0>-pi/8))
                                 ngulo0=fget_angle(0,0,incr_x,-incr_y);
                            END
                            IF (( ngulo0>pi*7/8) AND ( ngulo0<pi*9/8))
                                 ngulo0=fget_angle(0,0,incr_x,-incr_y);
                            END
                        END

                    END

                    x_resol=ultima_x_resol;     // Empuja la bola desde el ladrillo
                    y_resol=ultima_y_resol;

                    IF (id_ladrillo.graph>=20 AND id_ladrillo.graph<30)
                        id_ladrillo.estado=1;   // Ladrillos que no caen (indestructibles)
                    ELSE
                        // Ladrillos a los que tienes que dar varios toques
                        IF (id_ladrillo.graph>=30 AND id_ladrillo.graph<33)
                            id_ladrillo.graph++;    // Cambia el gr fico
                            IF (id_ladrillo.graph==33)  // Si ha llegado a roto, elimina el ladrillo
                                signal(id_ladrillo,s_kill);
                                nladrillos--;
                            END
                        ELSE// Aqui se comprueban ladrillos normales
                            // Aleatoriamente va creando pildoras
                            IF (rand(0,100)<20)
                                pildora(id_ladrillo.x,id_ladrillo.y,rand(202,209));
                            END
                            // Crea un ladrillo de los que caen
                            caido(id_ladrillo.x,id_ladrillo.y,id_ladrillo.graph);
                            // Elimina el ladrillo
                            signal(id_ladrillo,s_kill);
                            nladrillos--;

                        END
                    END

                END

                FRAME(0);   // Actualiza los procesos
            END
            x=x_resol/100;
            y=y_resol/100;
            FRAME;
        UNTIL (y>200 OR parado) // Repite hasta que se salga de pantalla o en modo pegamento

        IF (NOT parado)     // Bola perdida
            // Reinicia esta bola
            velocidad=400;
             ngulo0=3*pi/8;
            nbolas--;       // Resta uno al contador de bolas
            IF (nbolas>0)   // Si se tienen m s bolas se sale
                BREAK;
            ELSE            // Quita una raqueta cuando es la ultima bola
                fade_off();
                fade_on();
                camb_dir=1;
                rebota=1;
                graph=1;
                id_raqueta.graph=3;
                IF(vidas>0)
                    signal(peque¤o_r[vidas-1],s_kill);
                END
                signal(type pildora,s_kill);
                IF (NOT demo)   // Si no esta en modo demo quita una vida
                    vidas--;
                    IF (vidas<0)    // Si no quedan vidas se acaba el juego
                        fin_juego=1;
                        BREAK;
                    END
                END
            END
        END

    END
END

//------------------------------------------------------------------------------
// Proceso ladrillo
// Muestra los ladrillos
//------------------------------------------------------------------------------

PROCESS ladrillo(x,y,graph);

PRIVATE
    incr_x=1;               // Incremento x de los ladrillos rojos

BEGIN
    z=-1;
    SWITCH (graph);         // Comprueba que tipo de ladrillo es
        CASE 15:            // Ladrillos se mueven
            LOOP
                // Mueve los ladrillos
                IF (x==16) incr_x=1; END
                IF (x==256) incr_x=-1; END
                x+=incr_x;
                // Comprueba que no choca con otros ladrillos
                IF (collision(type ladrillo))
                    incr_x=-incr_x;
                    x+=incr_x;
                END
                FRAME;
            END
        END
        CASE 20:            // Ladrillos indestructibles
            LOOP
                IF (estado>0)   // Si se tocan hace una peque¤a animaci¢n
                    SWITCH (estado++);
                        CASE 1:
                            graph=21;
                        END
                        CASE 2:
                            graph=22;
                        END
                        CASE 3:
                            graph=21;
                        END
                        CASE 4:
                            graph=20;
                            estado=0;
                        END
                    END
                END
                FRAME;
            END
        END
        DEFAULT:    // Por defecto entra en un bucle infinito
            LOOP
                FRAME;
            END
        END
    END
END

//------------------------------------------------------------------------------
// Proceso pildora
// Muestra las p¡ldoras de bonos
//------------------------------------------------------------------------------

PROCESS pildora(x,y,graph);

BEGIN
    z=-2;
    // Mueve para abajo hasta que se coga o desaparezca de pantalla
    WHILE (NOT out_region(id,0) AND estado==0)
        y+=2;
        FRAME;
    END
    // Reduce de tama¤o cuando se coge al ladrillo o desaparece
    WHILE (NOT out_region(id,0) AND size>0)
        size-=10;
        y+=1;
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso laser
// Maneja el disparo
//------------------------------------------------------------------------------

PROCESS laser(x,y);

PRIVATE
    id_ladrillo;            // Identificador de ladrillo

BEGIN
    graph=8;                // Selecciona gr fico
    sound(s_fuego,80,400);  // Y realiza sonido
    WHILE (y>8)
        // Comprueba si toca a un ladrillo
        IF (id_ladrillo=collision(TYPE ladrillo))
            signal(id,s_kill);  // Elimina el disparo

            // Ladrillos indestructibles
            IF (id_ladrillo.graph>=20 AND id_ladrillo.graph<30)
                // Pone la animaci¢n
                id_ladrillo.estado=1;
            ELSE

                // Ladrillo de muro
                IF (id_ladrillo.graph>=30 AND id_ladrillo.graph<40)
                    // Adelanta un paso la destrucci¢n
                    id_ladrillo.graph++;
                    // Comprueba si ha llegado al final
                    IF (id_ladrillo.graph==33)
                        // Destruye el ladrillo
                        nladrillos--;
                        // Para que desaparezca en pantalla se reduce el tama¤o
                        reduce(id_ladrillo.x,id_ladrillo.y,id_ladrillo.graph);
                        signal(id_ladrillo,s_kill);
                    END
                ELSE
                    // En los ladrillos normales de vez en cuando crea un bonus
                    IF (rand(0,100)<20)
                        pildora(id_ladrillo.x,id_ladrillo.y,rand(200,209));
                    END
                    // Elimina el ladrillo y quita la visualizaci¢n reduciendolo
                    reduce(id_ladrillo.x,id_ladrillo.y,id_ladrillo.graph);
                    signal(id_ladrillo,s_kill);
                    nladrillos--;
                END
            END
        END
        y-=8;   // Si no ha colisionado se mueve hacia arriba
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso caido
// Mueve hacia abajo los ladrillo ca¡dos
//------------------------------------------------------------------------------

PROCESS caido (x,y,graph)

PRIVATE
    iangulo;      // Incremento de  ngulo

BEGIN
    // Hace una sonido
    sound(s_golpe,200,256);
    // Elige un incremento del  ngulo aleatoriamente
    iangulo=rand(-pi/8,pi/8);

    REPEAT
        FRAME;
        y+=5;                   // Mueve el gr fico hacia abajo
        angle+=iangulo;         // Mueve el  ngulo con el incremento elegido
    UNTIL (out_region(id,0))    // Repite hasta que desaparezca de pantalla

END

//------------------------------------------------------------------------------
// Proceso reduce
// Reduce el ladrillo
//------------------------------------------------------------------------------

PROCESS reduce(x,y,graph)

BEGIN
    REPEAT
        FRAME;
        size-=6;    // Cambia tama¤o
    UNTIL (size<0)  // Repite hasta que desaparezca por ser peque¤o
END

//------------------------------------------------------------------------------
// Proceso peque¤a_raq
// Muestra la raqueta peque¤as (vidas)
//------------------------------------------------------------------------------

PROCESS peque¤a_raq(x,y)

BEGIN
   graph=7;     // Selecciona gr fico
   LOOP         // Entra en un bucle infinito
        FRAME;
   END
END

//------------------------------------------------------------------------------
// Proceso texto_demo
// Imprime el texto de la demo
//------------------------------------------------------------------------------

PROCESS texto_demo()

BEGIN
    LOOP
        // Escribe el texto
        id_texto=write(1,90,100,0,"PULSA UNA TECLA PARA JUGAR");;
        FRAME (2000);           // Espera 20 FRAMEs(impresi¢n de pantalla)
        delete_text(id_texto);  // Borra el texto
        FRAME (2000);           // Espera 20 impresiones de pantalla
    END
END

//------------------------------------------------------------------------------
// Proceso creditos
// Imprime los mensajes de creditos
//------------------------------------------------------------------------------
PROCESS creditos()

PRIVATE
    fuentec;    // Fuente usada en los cr‚ditos

BEGIN
    fade_off();
    delete_text(all_text);  // Borra otros textos o procesos
    let_me_alone();
    put_screen(1,2);        // Muestra la pantalla de presentacion
    load_pal("noid\noid2.fpg");
    // Carga los ficheros de fuentes de letras
    fuentec=load_fnt("noid\noid2.fnt");

    // Imprime textos
    write(fuentec,160,30,4,"- CREDITOS -");
    write(fuentec,160,60,4,"PROGRAMADOR: LUIS SUREDA");
    write(fuentec,160,80,4,"GRAFICOS: JOSE FERNANDEZ");
    write(fuentec,160,100,4,"SONIDOS: CARLOS ILLANA");
    write(fuentec,160,120,4,"COPYRIGHT 1997");
    write(fuentec,160,140,4,"DIV GAMES STUDIO");
    fade_on();

    // Espera hasta que se pulse cualquier tecla
    WHILE (scan_code==0)
        FRAME;
    END
    fade_off();                     // Apaga la pantalla
    exit("Gracias por jugar!",0);   // Sale del programa
END
