COMPILER_OPTIONS _EXTENDED_CONDITIONS;
//-----------------------------------------------------------------------------
// TITULO:      CASTLE OF DR MALVADO
// AUTOR:       DANIEL NAVARRO
// FECHA:       01/02/97
//-----------------------------------------------------------------------------

PROGRAM castle_of_dr_malvado;

GLOBAL
    nivel=1;         // Numero de niveles
    vidas=3;         // Numero de vidas
    puntos=0;        // Variable para la puntuaci¢n
    bonos=0;         // Numero de bonos recogidos
    fuente1,fuente2; // Identificadores de las fuentes

    s_pez;           // Identificador para el sonido de los peces
    s_toque;         // Identificador para el sonido de los toques
    s_estrella;      // Identificador para el sonido de estrellas
    s_ara¤a;         // Identificador para el sonido de las ara¤as
    s_oso;           // Identificador para el sonido de los osos
    s_abeja;         // Identificador para el sonido de abejorro
    s_fantasma;      // Identificador para el sonido de fantasma
    s_salto1;        // Identificador para el primer sonido de salto
    s_salto2;        // Identificador para el segundo sonido de salto
    s_caida;         // Identificador para el sonido de ca¡da
    s_burbuja;       // Identificador para el sonido de las burbujas
    s_bonos;         // Identificador para el sonido de recogida de bonos
    s_malvado;       // Identificador para cuando muere el Doctor malvado

    energia_enemigo;    // Variable para el nivel de energ¡a enemigos fin de fase
    id_barra_enemigo;   // Identificador para quitar la barra de energ¡a
    fin_nivel=false;    // Variable para controlar si se ha terminado un nivel
    idtextovida;        // Variables que manejan el texto cuando sale una vida
    contadortextovida;
    contextovida=0;
    idprincipal;
LOCAL
    velocidad_gravedad; // Variable para guardar la velocidad de ca¡da (gravedad)
    velocidad=0;        // Variable para guardar la velocidad horizontal del protagonista
    muerte;             // Variable para guardar si el protagonista esta muerto
    angulo2;            // Variable de uso general  ngulos.
//IMPORT "ss1.dll";
PRIVATE
    pantalla=1;         // Pantalla que se ve como men£
    saliendo=0;         // 1= Se quiere salir del juego
BEGIN

    idprincipal=id;
    set_fps(24,0);      // Velocidad 24 pantallas por segundo
                        //  con un m ximo de saltos de 4 pantallas
    // Carga el fichero de gr ficos general
    load_fpg("malvado\malvado.fpg");

    // Carga los ficheros de fuentes de las letras
    fuente1=load_fnt("malvado\numero.fnt");
    fuente2=load_fnt("malvado\letras.fnt");

    // Carga sonidos
    s_pez=load_pcm("malvado\pez.pcm",0);       // Sonido de peces
    s_toque=load_pcm("malvado\golpe.pcm",0);      // Sonido de toque
    s_estrella=load_pcm("malvado\estrella.pcm",0);  // Sonido de estrellas
    s_ara¤a=load_pcm("malvado\aracnido.pcm",0);   // Sonido de la ara¤a
    s_oso=load_pcm("malvado\oso.pcm",0);       // Sonido del oso
    s_abeja=load_pcm("malvado\abeja.pcm",0);    // Sonido del abejorro
    s_fantasma=load_pcm("malvado\fantasma.pcm",0); // Sonido del fantasma
    s_salto1=load_pcm("malvado\salto1.pcm",0);   // Sonido de salto uno
    s_salto2=load_pcm("malvado\salto2.pcm",0);   // Sonido de salto dos
    s_caida=load_pcm("malvado\caida.pcm",0);     // Sonido de ca¡da
    s_burbuja=load_pcm("malvado\burbuja.pcm",0);    // Sonido de burbujas
    s_bonos=load_pcm("malvado\bonus.pcm",0);    // Sonido de bonos
    s_malvado=load_pcm("malvado\malvado.pcm",0);    // Sonido de muerte de malvado

    LOOP                    // Bucle principal
        timer=1000;         // Se inicializa fuera de tiempo para cambiar nada mas entrar
        pantalla=3;         // Elige la otra pantalla porque cambia nada mas entrar

        REPEAT                              // Bucle de pantallas de presentaci¢n
            IF (timer>500)                  // Si pasa 5 segundos cambia de pantalla
                timer=0;
                IF (pantalla==1)
                    pantalla=3;             //  3:Pantalla cr‚ditos
                ELSE
                    pantalla=1;
                END                         //  1:Pantalla de men£
                fade_off();
                put_screen(0,pantalla);
                fade_on();
            END
            angle+=10000;
            FRAME;
        until (scan_code<>0);
        // Espera a que se pulse una tecla

        // Si has pulsado ESCAPE sal directamente del programa
        IF (key(_esc))
            fade_off();
            exit("Gracias por jugar!",0);
        END

        fade_off();                     // Apaga pantalla
        // Inicializa variables de juego
        // Fase=1, vidas=3, puntuacion=0, bonus=0
        nivel=1;
        vidas=3;
        puntos=0;
        bonos=0;

        REPEAT              // Bucle para pantalla de espera de carga
            fade_on();      // Enciende la pantalla
            put_screen(0,2);// Pinta pantalla de fondo
            write(fuente2,138,77,4,"NIVEL");
            write_int(fuente2,208,77,4,&nivel);
            WHILE (fading)  // Espera mientras inicia todo, que
                FRAME;      // que ser  al acabar el apagado de pantalla
            END
            delete_text(all_text);
            // Coloca marcadores de puntos, vidas y bonos
            objeto(0,222,16,16,-512,0);     // Pinta vidas
            write(fuente1,36,16,4,"x");
            write_int(fuente1,44,16,3,&vidas);
            write_int(fuente1,160,16,4,&puntos);  // Pinta puntuaci¢n
            objeto(0,216,270,17,-512,0);    // Pinta bonos
            write(fuente1,288,16,4,"x");
            write_int(fuente1,296,16,3,&bonos);
            fin_nivel=false;                // Inicializa variable
                                            // controladora de fin de fase
            // Pone textos al iniciar un nivel
            SWITCH (nivel)      // Bucle para niveles
                CASE 1:         // Inicializa nivel uno
                    // Carga el bloque de gr ficos del nivel uno
                    load_fpg("malvado\nivel1.fpg");

                    // Inicializa scroll
                    start_scroll(0,1,1,3,0,4);
                    // Define la zona del scroll a partir de la cual se mueve
                    scroll.region1=define_region(1,96,100,1,1);


                    // Inicializa objetos que tapan otros objetos
                    objeto(1,20,2080,122,-256,1); // Gr ficos para marcos de puertas
                    objeto(1,21,2102,34,-256,1);
                    objeto(1,22,3286,72,32,1);    // Gr fico para tapar burbujas (caldero)

                    // Inicializa objetos de primer plano
                    primer_plano(12,200);        // Planta de junco
                    primer_plano(10,500);        // Se¤al de direcci¢n a castillo
                    primer_plano(11,780);        // Tumba
                    primer_plano(12,1300);       // Planta de junco
                    primer_plano(13,2150);       // Vela
                    primer_plano(13,2800);       // Vela

                    // Inicializa objetos de cadenas de plataformas
                    cadena(2840,8,0,pi/48,pi/4);
                    cadena(3133,8,pi,pi/32,pi/4);
                    cadena(3660,80,0,pi/24,pi/4);

                    // Inicializa el objeto del protagonista
                    jack(8,0);

                    // Inicializa objetos osos
                    oso(82,189,-1);
                    oso(256,274,-2);
                    oso(484,158,-3);
                    oso(580,247,-2);
                    oso(798,189,-3);
                    oso(1053,191,-3);
                    oso(1675,100,-4);
                    oso(2036,175,-6);

                    // Inicializa objetos abejorro
                    voladores(310,0,600,140,110,64,100,64);
                    voladores(310,0,900,140,110,64,160,100);
                    voladores(310,0,1300,170,90,48,200,50);
                    voladores(310,0,1320,150,95,60,150,75);
                    voladores(310,0,1820,100,48,36,64,64);

                    // Inicializa objetos fantasma
                    voladores(320,4,2170,100,48,36,64,64);
                    voladores(320,4,2329,224,100,24,220,32);
                    voladores(320,4,2865,100,48,36,96,96);
                    voladores(320,4,3100,64,100,36,96,96);

                    // Inicializa objetos ara¤a
                    ara¤a(2349,63,65);
                    ara¤a(2218,152,78);
                    ara¤a(2534,0,77);

                    // Inicializa objetos fuego
                    fuego(2280,256,10);
                    fuego(2390,256,11);

                    // Coge puntos de control del mapa para bonos
                    FROM z=0 TO 25;
                        get_point(1,1,z,&x,&y); // Coge punto
                        bono(x,y);              // Pon bonos
                    END
                END

                CASE 2:         // Inicializa nivel dos
                    // Carga fichero de gr ficos para nivel dos
                    load_fpg("malvado\nivel2.fpg");

                    // Inicializa scroll para nivel 2
                    start_scroll(0,1,1,3,0,4);
                    // Selecciona la zona del scroll a partir de donde se mueve
                    scroll.region1=define_region(1,160,150,1,1);

                    // Inicializa plataformas met licas que van en diagonal
                    plataforma_metal(214,1590,128,1443,2);
                    plataforma_metal(108,1249,212,1249,1);
                    plataforma_metal(212,1193,212,1056-58,2);
                    plataforma_metal(108,1193,108,1056+64,1);
                    plataforma_metal(158,582,158,624,5);

                    // Inicializa bola de pinchos
                    bola_pin(157,1121);
                    cadena(160,532,0,pi/24,pi/10);  // Inicializa cadena

                    // Inicializa protagonista
                    jack(8,1971);

                    // Inicializa calabazas
                    calabaza(100,1855,2);
                    calabaza(100,1583,2);
                    calabaza(233,1311,1);
                    calabaza(219,313,2);
                    calabaza(208,189,3);

                    // Inicializa cabezas que escupen
                    cabeza(264,1894,1,24);
                    cabeza(264,1515,1,96);
                    cabeza(264,465,1,26);
                    cabeza(56,1515,0,96);
                    cabeza(56,1298,0,26);
                    cabeza(264,792,1,26);
                    cabeza(56,174,0,24);

                    // Inicializa ara¤as
                    ara¤a(74,1612,100);
                    ara¤a(245,955,274);
                    ara¤a(75,285,83);
                    ara¤a(245,23,71);

                    // Inicializa fantasmas
                    voladores(320,4,158,1760,90,70,80,80);
                    voladores(320,4,158,1145,140,110,80,80);
                    voladores(320,4,158,616,48,64,80,120);
                    voladores(320,4,158,190,64,96,90,110);

                    // Coge puntos de control para bonos
                    FROM z=0 TO 25;
                        get_point(1,1,z,&x,&y); // Coge puntos
                        bono(x,y);              // Pone bonos
                    END
                END

                CASE 3:         // Inicializa fase tres
                    // Carga fichero de gr ficos para fase tres
                    load_fpg("malvado\nivel3.fpg");

                    // Inicializa scroll para fase tres
                    start_scroll(0,1,1,3,0,4);
                    // Define la zona de scroll a partir de donde se mueve
                    scroll.region1=define_region(1,160,100,1,1);

                    // Inicializa protagonista
                    jack(64,0);

                    // Inicializa monstruo fin de fase (doctor malvado)
                    malvado();              // Objeto monstruo
                    energia_enemigo=16;     // Cantidad de energ¡a
                    // Identificador necesario por si pasas con truco
                    id_barra_enemigo=barra_energia(); // Inicializa barra de energ¡a
                END

            END

            fade_off();             // Apaga pantalla
            clear_screen();         // Borra pantalla
            fade_on();              // Enciende pantalla

            REPEAT                  // Bucle principal del juego
                IF (nivel==1)
                    // Crea peces y burbujas decorativos
                    SWITCH (rand(0,24))
                        CASE 0: pez(rand(380,700),292); END
                        CASE 1: pez(rand(1290,1354),288); END
                        CASE 2: pez(rand(1848,1948),250); END
                        CASE 3,4: burbuja(rand(3270,3300),70); END
                    END
                END
                // Si se pulsa a la vez todas las letras que forman la
                // palabra div se pasa de fase
                IF (key(_d) AND key (_i) AND key(_v))
                    fin_nivel=true;
                END
                FRAME;
            // Repite hasta que se pase de nivel o te quedes sin vidas
            UNTIL (fin_nivel OR vidas<0 OR key (_esc))
            IF (key(_esc))
                saliendo=1;
            END
            IF (fin_nivel)        // Si se ha terminado la fase
                nivel++;          // Incrementa la fase
                fin_nivel=false;  // Inicializa la variable de control de fase
            END
            IF (vidas<0)
                write(fuente2,160,100,4,"GAME OVER");
            END
            IF (nivel==4)
                write(fuente2,160,100,4,"BIEN HECHO");
            END

            FRAME(2400);             // Espera un segundo
            fade_off();              // Apaga pantalla
            stop_scroll(0);          // Para scroll
            // Termina procesos de objetos que tapan gr ficos
            signal(TYPE objeto,s_kill);
            // Termina procesos de objetos de primer plano
            signal(TYPE primer_plano,s_kill);
            // Para todos los sonidos
            stop_sound(all_sound);
            // Borra todos los textos que haya
            delete_text(all_text);
            // Descarga fichero de gr ficos
            unload_fpg(1);
            // Quita la barra de energ¡a del enemigo final
            signal(id_barra_enemigo,s_kill);
        // Repite hasta que no tengas vidas o llegue a la fase cuatro
        UNTIL (vidas<0 OR nivel==4 OR key (_esc) OR saliendo)
        saliendo=0;
    END
END

//-----------------------------------------------------------------------------
// Proceso para el protagonista
// Entradas: coordenadas horizontal (x) y vertical (y)
//-----------------------------------------------------------------------------

PROCESS jack(x,y);

PRIVATE
    incx=0;     // Variable para comprobar paredes
    ultima_y=0; // Variable para mirar si sube
    ultima_veloc=0;        // Variable para comprobar paredes
    id2;        // Identificador de uso general

BEGIN
    ctype=c_scroll;             // Mete dentro del scroll
    scroll.camera=id;           // Hace que le siga la c mara
    graph=100;
    priority=1;

    LOOP
        // Lee el teclado y el joystick
        IF ((key(_right) OR joy.right) AND velocidad<8)
            velocidad+=2;           // Sube velocidad a la derecha
            flags=0;                // Mira a la derecha
        ELSE
            IF ((key(_left) OR joy.left) AND velocidad>-8)
                velocidad-=2;       // Sube velocidad a la izquierda
                flags=1;            // Mira a la izquierda
            ELSE                    // Si no hay nada pulsado
                IF (velocidad>0)    // Frena a la derecha
                    velocidad--;
                END
                IF (velocidad<0)    // O frena a la izquierda
                    velocidad++;
                END
            END
        END

        // Mira si el protagonista choca con las paredes
        ultima_veloc=velocidad;                // Guarda la velocidad en 'ultima_veloc'
        IF (ultima_veloc<>0)                   // Y si se mueve
            incx=ultima_veloc/abs(ultima_veloc);          // Halla la direcci¢n
            WHILE (ultima_veloc<>0)            // 'ultima_veloc' sirve de contador
                // Comprueba que no de por debajo
                IF (map_get_pixel(1,2,(x+incx)/2,(y-2)/2)<>120)
                    // Ni que de por encima
                    IF (map_get_pixel(1,2,(x+incx)/2,(y-32)/2)<>120)
                        x+=incx;    // Y si no da, mueve al mu¤eco
                    END
                END
                ultima_veloc-=incx;            // Resta 'ultima_veloc' hasta que sea cero
            END
        END

        // Si choca con una plataforma crea un nuevo protagonista
        IF (id2=collision(TYPE plataforma))
            // Tiene que estar bajando y por encima de la plataforma
            IF (velocidad_gravedad>0 AND abs(x-id2.x)<50 AND y<=id2.y+24)
                jack_plataforma(x-id2.x,id2,50,8);
            END
        END

        // Si choca con una plataforma de metal crea un protagonista nuevo
        IF (id2=collision(TYPE plataforma_metal))
            // Tiene que estar bajando y por encima de la plataforma
            IF (velocidad_gravedad>0 AND abs(x-id2.x)<36 AND y<=id2.y+12)
                jack_plataforma(x-id2.x,id2,36,-7);
            END
        END

        gravedad();             // Funci¢n para hallar la gravedad del protagonista

        IF (en_pantalla())      // Si el protagonista no baja
            IF (velocidad<>0)   // Y se mueve
                graph++;        // Cambia de gr fico para animarlo
                IF (graph>106)
                    graph=100;
                END
            ELSE
                graph=101;      // Si no se mueve ponen el gr fico por defecto
            END
            // Lee que las teclas (Espacio y Control) o el joystick
            IF (key(_space) OR key(_control) OR joy.button1)
                sound(s_salto2,100,rand(256,512));       // Si esta pulsado haz sonido
                velocidad_gravedad=-18;        // y salta
            END
        ELSE                    // Si el protagonista si baja
            graph=108;          // Pon el gr fico de bajar
        END

        IF (ultima_y<y)            // Comprueba si esta bajando
            muerte=true;        // Si baja puede matar
        ELSE
            muerte=false;       // Si sube o esta quieto le pueden matan
        END

        // Comprueba colisi¢n con osos
        IF (id2=collision(TYPE oso))
            puntos+=350;
        ELSE
            // Comprueba colisi¢n con abejas y fantasmas
            IF (id2=collision(TYPE voladores))
                puntos+=575;
            ELSE
                // Comprueba colisi¢n con ara¤as
                IF (id2=collision(TYPE ara¤a))
                    puntos+=825;
                ELSE
                    // Comprueba colisi¢n con cabezas de monstruo
                    IF (id2=collision(TYPE cabeza))
                        puntos+=150;
                    ELSE
                        // Comprueba colisi¢n con calabazas
                        IF (id2=collision(TYPE calabaza))
                            puntos+=425;
                        END
                    END
                END
            END
        END

        IF (id2)        // Comprueba si ha habido cualquier tipo de colisi¢n
            IF (muerte)     // Y si esta bajando y puede matar
                velocidad_gravedad=-12;     // Hace botar al protagonista
                estrellas(id2.x,id2.y);     // Crea estrellas vistosas
                signal(id2,s_kill);         // Elimina al monstruo que ha originado la colisi¢n
                sound(s_burbuja,100,512);   // Y hace ruido de pompas
            ELSE       // Si no puede matar, entonces el protagonista
                muerte_jack();              // Esta muerto
            END
        END

        // Si es fuego, directamente mata al protagonista
        IF (collision(TYPE fuego)) muerte_jack(); END

        // Si son bolas, directamente mata al protagonista
        IF (collision(TYPE bola)) muerte_jack(); END

        // Si es disparo de las cabezas, directamente mata al protagonista
        IF (collision(TYPE disparo_cabeza)) muerte_jack(); END

        // Si es bola de pinchos, directamente mata al protagonista
        IF (collision(TYPE bola_final)) muerte_jack(); END

        // Comprueba si hay colisi¢n con los bonos
        IF (id2=collision(TYPE bono))
            sound(s_bonos,150,256);     // Haz sonido
            bonos++;                    // Incrementa numero de bonos
            puntos+=1000;               // Suma puntos
            estrellas(id2.x,id2.y);     // Crea estrella vistosas
            signal(id2,s_kill);         // Elimina los bonos de pantalla
            IF (bonos>=10)              // Y si tiene mas de 10 bonos
                bonos-=10;              // Resta 10 bonos en la cuenta
                vidas++;                // Y suma una vida
                // Pone un mensaje de vida conseguida
                idtextovida=write(fuente1,160,150,4,"VIDA EXTRA");
                contadortextovida=0;
                contextovida=1;
            END
        END

        // Espera cuatro segundos hasta quitar el mensaje
        IF (contextovida==1)
            contadortextovida++;
        END
        IF (contadortextovida>99 AND contextovida==1)
            delete_text(idtextovida);
            contextovida=2;
        END
        // Actualiza las variables internas del scroll cero
        move_scroll(0);

        IF (nivel==1)                   // En el nivel uno
            IF (y>300)                  // Si ha pasado el borde inferior
                muerte_jack();          // Mata al protagonista
            END
            IF (x>3400)                 // Si ha llegado al extremo derecho
                // Si no ha acabado el nivel y no se ha creado el primer monstruo
                IF (not fin_nivel AND not get_id(TYPE hijo_malvado_cabeza))
                    hijo_malvado();                     // Crea monstruo fin de fase
                    energia_enemigo=16;                 // Crea barra de energ¡a del monstruo
                    id_barra_enemigo=barra_energia();   // Guarda identificador de la barra de
                                                        // para quitarla si pasas fase con truco
                ELSE   // Si esta creado el monstruo
                    // Comprueba si el protagonista a chocado
                    IF (collision(TYPE hijo_malvado))
                        muerte_jack();                  // Y si lo toca lo mata
                    END
                END
            END
        ELSE
            // Si estas en la fase dos y en la posici¢n final
            IF (nivel==2 AND x==58 AND y==53)
                fin_nivel=true;   // Acaba el nivel
            END
        END

        ultima_y=y;   // Guarda y en ultima_y para comprobar luego si baja
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso del protagonista cuando esta encima de una
// plataforma para que el protagonista se mueva a la
// vez que la plataforma.
// Entradas: `xrelativa`         Posici¢n relativa horizontal del
//                               protagonista respecto al centro
//                               de la plataforma
//           `id2`               Identificador de la plataforma
//           `tama¤o_plataforma` Tama¤o (largo) de la plataforma
//           `incy`              Posici¢n relativa vertical del
//                               protagonista respecto al centro
//                               al centro de la plataforma
//-----------------------------------------------------------------------------

PROCESS jack_plataforma(xrelativa,id2,tama¤o_plataforma,incy);

BEGIN

    // Duerme al proceso padre(El que le ha llamado)
    // mientras esta este proceso activo
    signal(father,s_sleep);
    ctype=c_scroll;     // Hace que este dentro del scroll
    graph=100;          // Pone el gr fico apropiado
    flags=father.flags; // Coge las banderas (izq./der.) que tenia el padre
    priority=1;         // Le da prioridad uno para que actualice
                        // antes al protagonista que al scroll
    scroll.camera=id;   // Hace que la c mara le siga

    LOOP
        // Lee la tecla de la derecha y el joystick
        IF ((key(_right) OR joy.right) AND velocidad<8)
            velocidad+=2;       // Incrementa la velocidad a la derecha
            flags=0;            // Lo pone mirando para la derecha
        ELSE
            IF ((key(_left) OR joy.left) AND velocidad>-8)
                velocidad-=2;       // Incrementa la velocidad a la izquierda
                flags=1;            // Lo pone mirando para la izquierda
            ELSE                    // Si no se ha pulsado ninguna direcci¢n
                IF (velocidad>0)
                    velocidad--;    // Frena a la derecha
                END
                IF (velocidad<0)
                    velocidad++;    // Frena a la izquierda
                END
            END
        END

        xrelativa+=velocidad;          // Suma velocidad a la posici¢n relativa
        x=xrelativa+id2.x;             // Convierte la posici¢n relativa en real
        y=id2.y+incy;

        IF (velocidad<>0)       // Si el mu¤eco se esta moviendo
           graph++;             // Cambia el gr fico para animarlo
           IF (graph>106)
              graph=100;
           END
        ELSE
            graph=101;          // Si no pon el gr fico de parado
        END

        // Si se pulsa la tecla de salto (espacio o control o joystick)
        IF (key(_space) OR key(_control) OR joy.button1)
            signal(father,s_wakeup);                    // Despierta el proceso anterior (el padre)
            father.x=x;                                 // Actualiza coordenada x
            father.y=y;                                 // Actualiza coordenada y
            father.velocidad=velocidad+id2.velocidad;   // Actualiza la velocidad
            father.velocidad_gravedad=-18;              // Hace botar al protagonista
            father.flags=flags;                         // Actualiza banderas (izq./der.)
            sound(s_salto2,100,rand(256,512));          // Hace sonido de salto dos
            scroll.camera=father;                       // Hace que la c mara le siga
            signal(id,s_kill);                          // Elimina el proceso actual
            FRAME;
        END

        // Si el protagonista esta fuera de la plataforma
        IF (abs(x-id2.x)>=tama¤o_plataforma+4)
            signal(father,s_wakeup);                    // Despierta el proceso anterior
            father.x=x;                                 // Actualiza x
            father.y=y;                                 // Actualiza y
            father.velocidad=velocidad+id2.velocidad;   // Actualiza velocidad
            father.flags=flags;                         // Actualiza banderas (izq./der.)
            father.velocidad_gravedad=0;                // Deja al protagonista suspendido en el aire
            scroll.camera=father;                       // hace que la c mara le siga
            signal(id,s_kill);                          // Elimina el proceso actual
            FRAME;
        END

        // Comprueba si ha tocado las abejas o los fantasmas
        IF (collision(TYPE voladores))
            signal(father,s_kill);      // Elimina el proceso anterior
            muerte_jack();              // Mata al protagonista
        END

        // Comprueba si ha tocado con el primer monstruo
        IF (collision(TYPE hijo_malvado))
            signal(father,s_kill);      // Elimina el proceso anterior
            muerte_jack();              // Mata al protagonista
        END

        // Comprueba si ha tocado con el disparo que lanzan las cabezas
        IF (collision(TYPE disparo_cabeza))
            signal(father,s_kill);      // Elimina el proceso anterior
            muerte_jack();              // Mata al protagonista
        END

        // Comprueba si le ha tocado la bola de pinchos
        IF (collision(TYPE bola_final))
            signal(father,s_kill);      // Elimina el proceso anterior
            muerte_jack();              // Mata al protagonista
         END

        // Actualiza las coordenadas internas del scroll
        move_scroll(0);
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para hacer la muerte del mu¤eco
//-----------------------------------------------------------------------------

PROCESS muerte_jack()

PRIVATE
    incr_y=-12;                          // Variable para mover gr fico poco a poco

BEGIN
    ctype=c_scroll;                      // Lo mete dentro del scroll
    graph=116;                           // Pone el gr fico apropiado
    priority=1;                          // Le da prioridad m xima
    signal(father,s_sleep);              // Duerme al proceso padre (el del protagonista)
    x=father.x;                          // Coge x del protagonista (proceso padre)
    y=father.y;                          // Coge y

    // Si esta en el nivel uno
    IF (nivel==1)
        scroll.camera=id;                // Hace que la c mara le siga
    END
    y-=12;                               // Mueve el gr fico hacia abajo

    REPEAT
        y+=incr_y;                           // Sigue bajando el gr fico
        incr_y++;                            // Cada vez mas despacio
        angle+=pi/16;                    // A la vez se gira
        size+=4;                         // Y se re-escala
        FRAME;
    UNTIL (out_region(id,0));              // Mientras o salga de pantalla
    // Apaga pantalla si le quedan vidas
    IF (vidas>0)
        fade_off();
    ELSE
        idprincipal.x=father.x;
        idprincipal.y=father.y;
        scroll.camera=idprincipal;
    END
    vidas--;
    IF (vidas>-1)                       // Si no quedan vidas
        SWITCH (nivel)                  // Dependiendo de la fase
            CASE 1:

                SWITCH(father.x)         // Y donde se haya quedado
                    CASE 0..799:         // crea un nuevo protagonista
                        jack(8,0);       // en la posici¢n adecuada
                    END
                    CASE 800..1959:
                        jack(800,0);
                    END
                    CASE 1960..2531:
                        jack(1960,0);
                    END
                    CASE 2532..3283:
                        jack(2532,0);
                    END
                    DEFAULT:
                        jack(3284,0);
                    END
                END
            END
            CASE 2:
                SWITCH(father.y)
                    CASE 0..390:
                        jack(203,422);
                    END
                    CASE 391..950:
                        jack(68,921);
                    END
                    CASE 951..1400:
                        jack(72,1392);
                    END
                    CASE 1401..2000:
                        jack(8,1971);
                    END
                END
            END
        END
        signal(father,s_kill);          // Elimina el protagonista anterior
        //enciende pantalla
        fade_on();
    ELSE
        signal(TYPE barra_energia,s_kill_tree); // Elimina la barra de final de
    END                                         // fase si hace falta.

END

//-----------------------------------------------------------------------------
// Proceso para la muerte del mu¤eco especial para la pantalla
// del monstruo final con las bombas
//-----------------------------------------------------------------------------

PROCESS muerte_jack2();

PRIVATE
    id2;                      // Identificador para protagonista anterior
    incr_y=-12;               // Variable para bajar gr fico poco a poco


BEGIN

    ctype=c_scroll;           // Integra el gr fico en el scroll
    graph=116;                // Coge el gr fico de muerto
    id2=get_id(TYPE jack);    // Coge identificador de protagonista
    signal(id2,s_sleep);      // Y du‚rmelo.
    x=id2.x;                  // Coge la x del protagonista
    y=id2.y-12;               // Coge la y pero un poco mas abajo

    REPEAT
        y+=incr_y;                // Mueve gr fico hacia abajo
        incr_y++;                 // Cada vez mas lento
        angle+=pi/16;         // Rotaci¢n del gr fico
        size+=4;              // Y re-escalado
        FRAME;
    UNTIL (out_region(id,0))    // Hasta que se salga de pantalla
    // Apaga pantalla si le quedan vidas
    IF (vidas>0)
        fade_off();
    ELSE
        idprincipal.x=father.x;
        idprincipal.y=father.y;
        scroll.camera=idprincipal;
    END
    vidas--;                  // Quita una vida
    IF (vidas>-1)             // Y si todav¡a tienes vidas
        fade_on();            // Encienda la pantalla
        jack(320,0);          // Crea un nuevo protagonista
        signal(id2,s_kill);   // Y elimina el anterior
    ELSE
        signal(TYPE barra_energia,s_kill_tree); // Si se acaba el juego
    END                                         // quita la barra del enemigo final
END

//-----------------------------------------------------------------------------
// Proceso para calcular los suelos y la gravedad de mu¤eco
//-----------------------------------------------------------------------------

PROCESS gravedad();

PRIVATE
    vgravedad;        // Variable para guardar temporalmente la gravedad

BEGIN
    // Coge la coordenada y a partir de la gravedad del protagonista (padre)
    vgravedad=(father.velocidad_gravedad+=2);   // Pero incrementada en 2
    IF (vgravedad>16)                           // Si es mayor de 16
        father.velocidad_gravedad=16;           // Haz que sea 16 la gravedad del protagonista
        vgravedad=16;                           // Y tambi‚n la variable temporal
    END

    IF (vgravedad<0)                        // Si la gravedad es negativa
        WHILE (vgravedad++!=0)              // Mientras no sea cero, la incrementa
            // Si no se da con el techo
            IF (map_get_pixel(1,2,father.x/2,(father.y-24)/2)<>252)
                father.y--;                 // Haz que el protagonista suba
            ELSE                            // Si no pon la gravedad a cero
                father.velocidad_gravedad=0;
                BREAK;
            END
        END
    ELSE                                    // Si la gravedad es cero o positiva
        father.y+=vgravedad;                // Se suma a la coordenada del protagonista
        // Comprueba el rango vertical del movimiento del protagonista
        FROM vgravedad=-16 TO 7 STEP 2;     // Acaba el proceso si se toca el suelo
            IF (map_get_pixel(1,2,father.x/2,(father.y+vgravedad)/2)==136)
                BREAK;                      // Sal si se encuentra con el suelo
            END
        END
        IF (vgravedad<8)                    // Si se encuentra con el suelo
            father.y+=vgravedad;            // Se actualiza con el m ximo que se pueda mover
            father.velocidad_gravedad=0;    // Y se pone la gravedad a 0
        END
    END

END

//-----------------------------------------------------------------------------
// Proceso para comprobar si se toca suelo
// Salidas: True/False, seg£n si se toca suelo o no
//-----------------------------------------------------------------------------

PROCESS en_pantalla()

BEGIN

    IF (map_get_pixel(1,2,father.x/2,father.y/2)==136)
        RETURN(TRUE);   // Si se toca suelo el proceso devuelve true
    ELSE
        RETURN(FALSE);  // Si no, devuelve false
    END

END

//-----------------------------------------------------------------------------
// Proceso para los objetos de primer plano
// Entradas: 'graph'        numero de gr fico
//           'xrelativa'    posici¢n horizontal relativa (x) del
//                          gr fico respecto al scroll
//-----------------------------------------------------------------------------

PROCESS primer_plano(graph,xrelativa);

BEGIN
    file=1;
    LOOP
        y=400-scroll.y0*2;
        x=(xrelativa-scroll.x0)*2;  // Halla la posici¢n relativa al scroll
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos que tapan otros
// Entradas: 'file'  fichero de gr ficos que se usa
//           'graph' numero de gr fico
//           'x' 'y' Coordenadas 'x' e 'y' del gr fico
//           'z'     Profundidad del gr fico en pantalla
//           'ctype' Variable para ver si esta dentro del scroll
//-----------------------------------------------------------------------------

PROCESS objeto(file,graph,x,y,z,ctype);

BEGIN
    LOOP
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos de los bonos
// Entradas: 'x' 'y' Coordenadas de los objetos
//-----------------------------------------------------------------------------

PROCESS bono(x,y);

BEGIN
    ctype=c_scroll;         // Haz que este dentro del scroll
    graph=216;              // Pon el gr fico inicial del gr fico

    LOOP                    // Realiza la animaci¢n de los gr ficos
        graph++;
        IF (graph>220)
            graph=216;
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos de las estrellas
// Entradas: 'x' 'y' Coordenadas de inicio de las estrellas
//-----------------------------------------------------------------------------

PROCESS estrellas(x,y);

PRIVATE
    incr_x;                    // Incremento horizontal
    n_estrellas;               // Numero de estrellas (contador)

BEGIN
    ctype=c_scroll;            // Haz que este dentro del scroll
    graph=200;                 // Pon el gr fico inicial de la estrella

    FROM n_estrellas=0 TO 11;            // Crea 12 estrellas iguales
        CLONE
            BREAK;
        END
    END

    incr_x=rand(-6,6);                      // Elige al azar el incremento horizontal
    velocidad_gravedad=rand(-24,-8);    // Y tambi‚n el vertical
    n_estrellas=32;

    WHILE (n_estrellas-->0);          // Mientras queden estrellas (n_estrellas = numero de estrellas)
        // Realiza la animaci¢n de los gr ficos
        graph++;
        IF (graph>204) graph=200; END
        x+=incr_x;                      // Mueve las estrellas horizontalmente
        y+=velocidad_gravedad;      // Y verticalmente
        velocidad_gravedad+=2;      // Y reduce el incremento vertical (gravedad)
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos de los osos
// Entradas:  'x' 'y' Coordenadas x e y de objeto
//            'incr_x'    Incremento horizontal (a der. e izq.
//-----------------------------------------------------------------------------

PROCESS oso(x,y,incr_x);

BEGIN
    graph=300;      // N£mero de gr fico
    ctype=c_scroll; // Lo mete dentro del scroll
    LOOP

        // Haz un sonido de oso de vez en cuando (rand)
        IF (rand(0,48)==0) suena_sonido(s_oso,50,256); END
        x+=incr_x;             // Mueve el gr fico horizontalmente

        // Si ha llegado al final de la plataforma
        IF (NOT en_pantalla())
            incr_x=-incr_x;        // Cambia el incremento horizontal
            x+=incr_x*2;       // Mueve un poco el gr fico
            // Da la vuelta al gr fico
            IF (flags==0)
                flags=1;
            ELSE
                flags=0;
            END
        END

        // Realiza la animaci¢n de los gr ficos
        graph++;
            IF (graph>304) graph=300; END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos de las calabazas
// Entradas: 'x'        Coordenada x del objeto
//           'ultima_y' Coordenada y del objeto especial
//           'incr_x'       Incremento horizontal (direcci¢n izq/der)
//-----------------------------------------------------------------------------

PROCESS calabaza(x,ultima_y,incr_x);

PRIVATE
    anim;                   // Hace que la animaci¢n sea intermitente
    salto=-5;               // Variable para procesar los saltos de la calabaza
    y_temp;                 // Variable temporal de y

BEGIN
    graph=340;              // Selecciona el gr fico inicial
    ctype=c_scroll;         // Introduce el objeto dentro del scroll
    y=ultima_y;             // Carga 'y' con la entrada de proceso ('y' especial)

    LOOP
        // Haz sonido del oso de vez en cuando (rand)
        IF (rand(0,48)==0) suena_sonido(s_oso,30,512); END
        x+=incr_x;              // Mueve al gr fico horizontalmente
        y_temp=y;           // Guarda temporalmente 'y' para comprobar plataformas
        y=ultima_y;            // Y lo carga con la 'y' inicial

        // Si sale fuera de la plataforma donde este
        IF (NOT en_pantalla())
            incr_x=-incr_x;         // Cambia la direcci¢n
            x+=incr_x*2;        // Mu‚velo un poco
        END

        y=y_temp;           // Recupera la 'y' actualizada
        y+=salto;           // Suma a 'y' el salto
        salto++;            // Incrementa 'y' para mover el gr fico
        IF (salto==6)       // Comprueba si se ha llegado al limite
            salto=-5;       // Para empezar otra vez
        END


        IF (anim)           // Comprueba si toca animar al gr fico
            graph++;        // Animaci¢n de gr ficos
            IF (graph>344)
                graph=340;
            END
        END

        anim=NOT anim;      // Hace que la animaci¢n sea mas lenta
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para objetos de cabezas que escupen
// Entradas: 'x' 'y'  Coordenadas del gr fico
//           'flags'  Direcci¢n hacia donde mira el gr fico
//           'pausa'  Numero de frames que esta en pausa
//-----------------------------------------------------------------------------

PROCESS cabeza(x,y,flags,pausa);

BEGIN
    ctype=c_scroll;                 // Haz que este dentro del scroll
    z=2;                            // Pon al gr fico por delante de otros

    LOOP
        graph=350;                  // Elige el gr fico inicial
        FRAME(pausa*100);           // Espera los frames indicados por pausa

        WHILE (graph<355)           // An¡malo hasta que se llegue al limite
            graph++;
            FRAME(200);             // Espera dos frames
        END

        // Haz sonido de lanzamiento de gotas horizontales
        suena_sonido(s_toque,20,768);
        // Haz que lance gotas horizontales
        disparo_cabeza(x+20-flags*40,y,4-flags*8,flags);
    END

END

//-----------------------------------------------------------------------------
// Proceso para lanzamiento de gotas horizontales
// Entradas: 'x' 'y' Coordenadas del objeto
//           'incx'  Incremento horizontal (direcci¢n)
//           'flags' Posici¢n hacia la que mira el gr fico
//                   horizontalmente
//-----------------------------------------------------------------------------

PROCESS disparo_cabeza(x,y,incx,flags);

BEGIN
    ctype=c_scroll;               // Hace que este dentro del scroll
    graph=356;                    // Elige el gr fico apropiado

    REPEAT
        x+=incx;                  // Mu‚velo horizontalmente
        FRAME;
    UNTIL (out_region(id,0))      // Hasta que salga de pantalla

END

//-----------------------------------------------------------------------------
// Proceso para objetos de los abejorros y fantasmas
// Entradas:  'gr'            Tipo de gr fico (abejorro/fantasma)
//            'flags'         Tipo de transparencia
//            'xrelativa'     Posici¢n x de referencia
//            'yrelativa'     Posici¢n y de referencia
//            'incx' 'incy'   Frecuencia de giro en los dos ejes
//            'distx' 'disty' Distancia del giro en los ejes
//-----------------------------------------------------------------------------

PROCESS voladores(gr,flags,xrelativa,yrelativa,incx,incy,distx,disty);

PRIVATE
    angulo3;
    id2;

BEGIN
    graph=gr;            // Elige el gr fico inicial apropiado
    ctype=c_scroll;      // haz que este dentro del scroll

    LOOP
        // Haz un sonido de vez en cuando
        IF (rand(0,48)==0)
            // Sonido de fantasmas
            IF (flags==4)
                suena_sonido(s_fantasma,50,256);
            // Sonido de abejorros
            ELSE
                suena_sonido(s_abeja,150,256);
            END
        END

        angulo2+=pi/incx;                 // Halla el tama¤o de los incrementos
        angulo3+=pi/incy;                 // que se har n en las dos coordenadas
        x=xrelativa+get_distx(angulo2,distx);    // Incrementa la coordenada x
        y=yrelativa+get_disty(angulo3,disty);    // Incrementa la coordenada y

        // Realiza la animaci¢n de los gr ficos
        graph++;
        IF (graph==gr+6)
            graph=gr;
        END
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para los objetos de tipo burbuja
// Entradas: 'x' 'y' Coordenadas del objeto en pantalla
//-----------------------------------------------------------------------------

PROCESS burbuja(x,y);

BEGIN
    z=64;                // Pone los gr ficos un poco detr s
                         // para as¡ poder taparlos con la olla
    graph=430;           // Elige el gr fico apropiado
    ctype=c_scroll;      // Pon el objeto dentro del scroll
    WHILE (size>0)       // Mientras el tama¤o(re-escalado) sea mayor que cero
        y-=2;            // Mueve el objeto hacia arriba
        size-=2;         // Y reduce su tama¤o.
        FRAME;
    END
END

//-----------------------------------------------------------------------------
// Proceso para objetos tipo pez.
// Entradas:  'x'  Coordenada horizontal del gr fico
//            'yrelativa' Coordenada vertical especial del gr fico
//-----------------------------------------------------------------------------

PROCESS pez(x,yrelativa);

PRIVATE
    incr_y;                 // Incremento vertical del gr fico

BEGIN
    z=-10;
    chapuzon(x,yrelativa);  // Pone un chapuz¢n de agua
    graph=410;              // Coge el gr fico inicial del pez
    ctype=c_scroll;         // Pone al objeto dentro del scroll
    y=yrelativa;            // Actualiza y con la entrada del proceso
    incr_y=-12;             // Inicializa el incremento (hacia arriba)
    // Haz sonido para peces
    suena_sonido(s_pez,25,512);

    // Mientras no se llegue al limite de movimiento
    WHILE (incr_y<12)
        // Realiza la animaci¢n del gr fico, mientras se este moviendo (incr_y<>0)
        IF (incr_y AND graph<416)
            graph++;
        END
        y+=incr_y;          // Mueve al gr fico verticalmente
        incr_y++;           // Actualiza el incremento vertical
        FRAME;
    END
    chapuzon(x,yrelativa);  // Pone un chapuz¢n de agua
END

//-----------------------------------------------------------------------------
// Proceso para reproducir sonidos
// Entradas: 'x'          Identificador del sonido
//           'volumen'    Volumen del sonido
//           'frecuencia' Frecuencia del sonido
//-----------------------------------------------------------------------------

PROCESS suena_sonido(x,volumen,frecuencia);

BEGIN
    // Si el gr fico esta dentro de la pantalla realiza el sonido
    IF (NOT (out_region(father,0)))
        sound(x,volumen,frecuencia);
    END
END

//------------------------------------------------------------------------------
// Proceso para poner objetos de chapuz¢n de agua
// Entradas: 'x' 'y' Coordenadas del objeto
//------------------------------------------------------------------------------

PROCESS chapuzon(x,y);

BEGIN
    ctype=c_scroll;     // Pon el objeto dentro del scroll
    // Realiza la animaci¢n de los gr ficos
    FROM graph=400 TO 405;
        FRAME(200);     // Espera dos frames
    END
END

//-----------------------------------------------------------------------------
// Proceso para poner los objetos ara¤a
// Entradas: 'x' 'y'  Coordenadas del objeto
//           'height' Altura del objeto
//-----------------------------------------------------------------------------

PROCESS ara¤a(x,y,height);

PRIVATE
    contador;                             // Contador de uso general

BEGIN
    z=-11;
    ctype=c_scroll;                // Hace que este dentro del scroll
    graph=330;                     // Elige el gr fico inicial
    height=(height/4)*4;           // Hace height m£ltiplo de cuatro

    LOOP
        FROM contador=0 TO 47;
            // Realiza la animaci¢n de la ara¤a de vez en cuando (rand)
            IF (rand(0,10)==0)
                graph++;
                IF (graph>335) graph=330; END
            END
            FRAME;
        END

        // Haz el sonido de la ara¤a
        suena_sonido(s_ara¤a,200,256);
        // Baja el objeto de 4 en 4 con el gr fico inm¢vil
        FOR (contador=0;contador<height;contador+=4);
            y+=4;
            FRAME;
        END

        // Sube el objeto de 1 en 1 con animaci¢n de gr ficos
        FOR (contador=0;contador<height;contador++);
            y--;
            ++graph;
            IF (graph>335)
                graph=330;
            END
            FRAME;
        END
    END
END

//-----------------------------------------------------------------------------
// Proceso para los objetos de las cadenas
// Entradas: 'x' 'y' Coordenadas del objeto
//           'ang'   Angulo inicial del objeto
//           'iang'  Velocidad de incremento dentro del  ngulo
//           'foco'  Tama¤o total de  ngulo (min-max)
//-----------------------------------------------------------------------------

PROCESS cadena(x,y,ang,iang,foco);

BEGIN
    priority=10;      // Pone un poco menos de prioridad que la plataforma
    ctype=c_scroll;   // Pone el gr fico dentro del scroll
    file=1;           // Escoge el fichero de gr ficos 1
    graph=14;         // Coge el gr fico de la cadena
    z=20;             // Pone el gr fico por detr s de la plataforma
    plataforma();     // Crea una plataforma

    LOOP
        ang+=iang;    // Incrementa el  ngulo
        // Coge el  ngulo del coseno para hacer movimiento pendular con distintas velocidades
        angle=get_distx(ang,foco);
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para objetos plataformas
//-----------------------------------------------------------------------------

PROCESS plataforma()

BEGIN
    priority=9;         // Pone mas prioridad que a la cadena
    ctype=c_scroll;     // Pone el objeto dentro del scroll
    file=1;             // Usa el fichero de gr ficos 1
    graph=15;           // Coge el gr fico de la plataforma
    z=19;               // Pone el gr fico por encima de la cadena

    LOOP
        // Coge las coordenadas de la plataforma respecto a la cadena
        velocidad=father.x+get_distx(father.angle-pi/2,130)-x;
        x+=velocidad;
        y=father.y+get_disty(father.angle-pi/2,130);
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para objetos de plataformas met licas
// Entradas: 'x1' 'y1' Coordenadas iniciales de la plataforma
//           'x2' 'y2' Coordenadas finales de la plataforma
//           'ipaso'   Incremento (velocidad) en %
//-----------------------------------------------------------------------------

PROCESS plataforma_metal(x1,y1,x2,y2,ipaso);

PRIVATE
    paso;            // Contador de proceso de movimiento (indica por donde va)

BEGIN
    priority=10;     // Le da una prioridad baja
    ctype=c_scroll;  // Pone el gr fico en el scroll
    file=1;          // Elige el fichero de gr ficos uno
    graph=16;        // Elige el gr fico de la plataforma met lica
    x=x1;            // Introduce las coordenadas iniciales
    y=y1;

    LOOP
        // Actualiza las nuevas coordenadas
        x=(x1*paso+x2*(100-paso))/100;
        y=(y1*paso+y2*(100-paso))/100;
        // Suma el incremento necesario
        paso+=ipaso;
        // Si se ha llegado al cien por cien
        IF (paso MOD 100==0)
            ipaso=-ipaso;              // Cambia la direcci¢n
            suena_sonido(s_estrella,12,512);  // Haz sonido de estrellas
        END
    FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para el objeto del monstruo de la primera fase
//-----------------------------------------------------------------------------

PROCESS hijo_malvado();

PRIVATE
    contador;                             // Contador de car cter general
    regionx=96;                    // Necesario para centrar la pantalla

BEGIN
    file=1;                        // Elige el primer fichero de gr ficos
    ctype=c_scroll;                // Introduce el gr fico dentro del scroll
    x=3800;                        // Coge coordenadas
    y=261;
    z=8;                           // Elige la profundidad (detr s)
    graph=104;                     // Elige el gr fico inicial
    hijo_malvado_cabeza();         // Crea la cabeza del monstruo

    // Modifica la regi¢n de movimiento de scroll
    // para centrarla, con el mu¤eco, en el centro de la pantalla
    WHILE (regionx<160); // La modifica poco a poco (suavemente)
        define_region(1,++regionx,100,1,1);
        FRAME;
    END

    LOOP
        graph=104;                  // Elige gr fico inicial
        flags=0;                    // Sin ning£n tipo de espejo
        FRAME(5000);                // Pon el gr fico y espera 50 frames (un segundo)
        graph=105;                  // Pone el gr fico de lanzando bola
        FRAME(400);                 // Imprime y espera 4 frames
        graph=106;                  // Pone el gr fico de ya haber lanzado bola
        bola(x-16,y-30,-6);         // Crea bola de fuego
        FRAME(400);                 // Espera 4 frames
        graph=105;                  // Pone el gr fico como recogi‚ndose
        FRAME(400);                 // Espera 4 frames
        graph=104;                  // Pone el gr fico inicial
        FRAME(5000);                // Espera 50 frames
        graph=107;                  // Pone el gr fico para saltar
        // Mueve el gr fico (salta)
        FROM contador=-16 TO 16;
            y+=contador;
            x-=10;
            FRAME;
        END
        // Haz sonido de toque
        sound(s_toque,150,256);
        graph=104;                 // Gr fico inicial
        flags=1;                   // Hace espejo para que mire el protagonista
        FRAME(5000);               // Repite el proceso de arriba
        graph=105;
        FRAME(400);
        graph=106;
        bola(x+16,y-30,6);
        FRAME(400);
        graph=105;
        FRAME(400);
        graph=104;
        FRAME(5000);
        graph=107;
        FROM contador=-16 TO 16;
            y+=contador;
            x+=10;
            FRAME;
        END
        // Realizada sonido de toque
        sound(s_toque,150,256);
    END
END

//-----------------------------------------------------------------------------
// Proceso para objeto de cabeza de monstruo primera fase
//-----------------------------------------------------------------------------

PROCESS hijo_malvado_cabeza();

PRIVATE
     ngulo0;                               // Variable para mover la cabeza en forma de elipse
    id2;                               // Identificador del protagonista y del cuerpo al final (monstruo muerto)
    // Crea una cadena de variables de 50 posiciones para la animaci¢n de los gr ficos
    anim[]=40 DUP (100),101,101,102,102,103,103,102,102,101,101;
    ianim;                             // Contador dentro de la cadena de animaci¢n
    incy=0;                            // Variable para bajar y subir la cabeza
                                       // cuando el protagonista la toca y quita el cuerpo
                                       // cuando se acaba la fase
BEGIN
    file=1;                            // Elige el fichero de gr ficos 1
    graph=100;                         // Pone el gr fico inicial
    ctype=c_scroll;                    // Pone el gr fico dentro del scroll
    z=4;                               // Profundidad por delante del cuerpo pero detr s del protagonista
    REPEAT
        // Pone la cabeza al rev‚s del cuerpo para colocarla
        IF (father.flags==1) flags=0; ELSE flags=1; END
        x=father.x+get_distx( ngulo0,6);   // Coge las coordenadas respecto al cuerpo
        y=father.y-70+get_disty( ngulo0,4);// pero en con un movimiento tipo elipse
        graph=anim[ianim];             // Coge el gr fico necesario para la animaci¢n
        ianim=(ianim+1) mod 50;        // Incrementa el contador dentro de la cadena de animaci¢n

        // Haz sonido de oso cada 40 animaciones
        IF (ianim==40) sound(s_oso,100,128); END
         ngulo0+=pi/48;

        // Comprueba si se ha tocado al protagonista
        IF (id2=collision(TYPE jack))
            // Si el protagonista esta mas alto que el monstruo
            // y si el protagonista esta vivo
            IF (id2.muerte AND id2.y<=y)
                incy=64;                    // Baja la cabeza del monstruo
                id2.velocidad_gravedad=-26; // Cambia la gravedad
                puntos++;                   // Suma puntuaci¢n
                energia_enemigo--;          // Quita energ¡a al monstruo
                estrellas(x,y);             // Pone estrellas vistosas
                sound(s_burbuja,100,256);   // Realiza sonido de burbuja
            END
        END

        // Sube la cabeza del monstruo si es que esta bajada
        IF (incy>0)
            y+=incy/2;
            FRAME;
            incy--;
        ELSE
            FRAME;
        END
    UNTIL (energia_enemigo==0)      // Repite mientras el monstruo tenga energ¡a

    s_malvado=load_pcm("malvado\malvado.pcm",0); // Sonido Muerte de Doctor
    // Se crea un cuerpo de monstruo
    id2=objeto(1,104,father.x,father.y,8,1);
    signal(father,s_kill);      // Y elimina el anterior
    incy=-24;                   // Incremento de bajada del cuerpo
    sound(s_malvado,128,256);   // Haz sonido
    REPEAT
        id2.y+=(incy/2);             // Mueve el cuerpo
        incy++;                      // Reduce velocidad de ca¡da
        y=id2.y-56;                   // Mueve la cabeza
        IF (incy MOD 8==0)           // Crea estrella cada 8 animaciones
            estrellas(x,y);
        END
        FRAME;
    UNTIL (y>350);                   //Mientras la cabeza no llegue al suelo

    // Pone la variable a uno para indicar que se ha acabo el nivel
    fin_nivel=true;
END


//-----------------------------------------------------------------------------
// Proceso para el objeto de monstruo final tercera fase
//-----------------------------------------------------------------------------
PROCESS malvado();

PRIVATE
    contador;                              // Contador de car cter general
    idcabeza;                       // Identificador para la cabeza del monstruo

BEGIN
    file=1;                         // Coge el fichero de gr ficos uno
    ctype=c_scroll;                 // Pone el objeto en  el scroll
    x=552;                          // Inicializa coordenadas del objeto
    y=230;
    z=8;                            // Pone el cuerpo detr s (profundidad) de la cabeza
    idcabeza=malvado_cabeza();      // Crea cabeza del monstruo

    LOOP
        graph=200;                 // Pone gr fico inicial
        flags=0;                   // Pone mirando al protagonista (por defecto)
        FRAME(5000);               // Espera 50 frames (1 segundo aprox.)
        graph=203;                 // Cambia gr fico para animarlo
        FRAME(300);                // Espera 3 frames
        graph=204;                 // Cambia gr fico para animarlo
        FRAME(300);                // Espera 3 frames
        graph=205;                 // Cambia gr fico a monstruo tirando bombas
        bomba(x-38,y+24,-6);       // Crea un objeto bomba
        FRAME(400);                // Espera 4 frames
        graph=200;                 // Cambia gr fico para animarlo (inicial)
        FRAME(5000);               // Espera 50 frames
        graph=201;                 // Cambia gr fico
        idcabeza.graph=101;        // Cambia gr fico de la cabeza del monstruo
        FROM contador=-16 TO 16;
            // Si esta en el medio de la animaci¢n
            IF (contador==0)
                graph=202;              // Cambia gr fico del cuerpo
                idcabeza.graph=102;     // Cambia el gr fico de la cabeza
            END
            // Mueve al monstruo (salto)
            y+=contador;
            x-=14;
            FRAME;
        END

        idcabeza.graph=100;         // Cambia gr fico de la cabeza
        sound(s_toque,150,256);     // Haz sonido de toque
        graph=200;                  // Gr fico inicial
        flags=1;                    // Lo pone mirando hacia el mu¤eco
        FRAME(5000);                // Repite la animaci¢n de anterior
        graph=203;
        FRAME(300);
        graph=204;
        FRAME(300);
        graph=205;
        bomba(x+38,y+24,6);         // Crea bomba
        FRAME(400);
        graph=200;
        FRAME(5000);
        // Vuelve a saltar hacia el otro lado
        graph=201;
        idcabeza.graph=101;
        FROM contador=-16 TO 16;
            IF (contador==0)
                graph=202;
                idcabeza.graph=102;
            END
            y+=contador;
            x+=14;
            FRAME;
        END

        idcabeza.graph=100;
        // Realiza sonido de toque
        sound(s_toque,150,256);
    END
END


//-----------------------------------------------------------------------------
// Proceso para el objeto de la cabeza del monstruo
//-----------------------------------------------------------------------------


PROCESS malvado_cabeza();

PRIVATE
     ngulo0;                                 // Para movimiento tipo elipse
    id2;                                     // Identificador para crear un cuerpo nuevo
    incy;                                    // Para bajar la cabeza y quitar el monstruo al final
                                             // si el protagonista lo toca o lo mata
BEGIN
    file=1;                                  // Elige el fichero de gr ficos 1
    graph=100;                               // Elige el gr fico inicial
    ctype=c_scroll;                          // Pone al objeto dentro del scroll
    z=4;                                     // Pone la cabeza por encima del cuerpo

    REPEAT
        flags=father.flags;                      // Coge la posici¢n (der/izq) del cuerpo
        x=father.x-18+flags*36+get_distx( ngulo0,6); // Coge las coordenadas respecto al cuerpo
        y=father.y-32+get_disty( ngulo0,4);          // pero con movimiento tipo elipse
         ngulo0+=pi/48;                              // Incrementa el movimiento tipo elipse
        FRAME;
    UNTIL (energia_enemigo==0)                   // Mientras el monstruo tenga energ¡a

    s_malvado=load_pcm("malvado\malvado.pcm",0); // Sonido Muerte de Doctor
    // Crea un nuevo cuerpo de monstruo
    id2=objeto(1,202,father.x,father.y,8,1);
    // Para eliminar el cuerpo de la cabeza
    signal(father,s_kill);
    incy=-24;                  // Selecciona velocidad de bajada(incremento
    graph=103;                 // Elige un gr fico de cabeza muerta
    flags=0;                   // Quita cualquier tipo de efecto espejo
    sound(s_malvado,100,256);  // Haz sonido
    REPEAT
        id2.y+=incy/2;         // Mueve el cuerpo (hacia abajo)
        incy++;                // Cada vez mas despacio
        y=id2.y-32;            // Mueve la cabeza hacia abajo
        x=id2.x-18;            // Y a la izquierda
        // Pon estrellas cada ocho animaciones
        IF (incy mod 8==0)
            estrellas(x,y);
        END
        FRAME;
    UNTIL (y>350);             // Repite mientras la cabeza no toque el suelo
    fin_nivel=true;            // Indica que se ha acabado el nivel
END

//-----------------------------------------------------------------------------
// Proceso para el objeto las bolas que tira el monstruo fase1
// Entradas: 'x' 'y'  coordenadas del objeto
//           'incr_x'     Incremento de movimiento (izq.der.)
//-----------------------------------------------------------------------------

PROCESS bola(x,y,incr_x);

BEGIN
    file=1;                             // Elige el fichero de gr ficos 1
    graph=108;                          // Pon el gr fico de bola de fuego
    ctype=c_scroll;                     // Incl£yelo dentro del scroll
    z=4;                                // Por delante del cuerpo y la cabeza pero detr s del protagonista
    sound(s_toque,100,128);             // Haz sonido de toque

    REPEAT
        x+=incr_x;                          // Mueve el objeto
        graph++;
        // Realiza la animaci¢n de gr ficos
        IF(graph>110) graph=108; END
        FRAME;
    UNTIL (x<3000 OR x>4000)           // Repite hasta que se salga de pantalla

END

//-----------------------------------------------------------------------------
// Proceso para las bombas que tira el monstruo final fase 3
// Entradas: 'x' 'y' Coordenadas del objeto
//           'inc'   Incremento horizontal (der/izq)
//-----------------------------------------------------------------------------


PROCESS bomba(x,y,incr_x);

PRIVATE
    incr_y;                                 // Posici¢n vertical de la bomba
    iymax;                              // Posici¢n m xima vertical de la bomba
    id2;                                // Identificador para hallar colisiones
    toque=false;                        // Variable para comprobar si la bomba la tira el monstruo
                                        // o la tira el protagonista
BEGIN
    file=1;                             // Elige los gr ficos de fichero 1
    graph=300;                          // Pon gr fico inicial
    ctype=c_scroll;                     // Ponlo dentro del scroll
    z=4;                                // Pon por detr s del protagonista y delante del monstruo
    iymax=rand(-10,-22);                // Altura m xima de la bomba al azar (rand)
    incr_y=iymax;                       // Empieza en la altura m xima
    sound(s_toque,50,128);              // Realiza sonido de toque

    REPEAT
        y+=incr_y;                      // Mueve la bomba
        incr_y+=2;
        // Hace que la bomba bote
        IF (incr_y>-iymax)
            iymax+=2;
            incr_y=iymax;
            sound(s_toque,25,768);      // Pon sonido de toque
        END
        x+=incr_x;

        // Si la bomba la tira el monstruo
        IF (NOT toque)
            // Si la toca el protagonista
            IF (id2=collision(TYPE jack))
                toque=TRUE;
                // Si el protagonista esta vivo
                IF (id2.muerte)
                    incr_x=-incr_x;                     // Cambia la direcci¢n de la bomba
                    id2.velocidad_gravedad=-20; // Hace botar al protagonista
                    estrellas(x,y);             // Pone estrellas
                    puntos++;                   // Suma puntos
                    sound(s_burbuja,25,512);    // Pone sonido de pompa
                ELSE
                    muerte_jack2();             // Si no mata al protagonista
                END
            END
        END

        // La bomba toca al monstruo le quita energ¡a
        IF (id2=collision(TYPE malvado))
            energia_enemigo--;
            BREAK;
        END

        FRAME;
    // Repite el proceso hasta que la bomba salga de pantalla
    UNTIL (x<64 OR x>640-64 OR iymax>=0)

    // Sonido de toque
    sound(s_toque,200,128);
    // Realiza animaci¢n
    FROM graph=301 TO 309;
        frame(200);                             // Espera dos frames por gr fico
    END

END

//-----------------------------------------------------------------------------
// Proceso para el objeto fuego
// Entradas: 'x' 'y' Coordenadas del objeto
//           'inc'   M ximo movimiento que realizara el objeto
//-----------------------------------------------------------------------------

PROCESS fuego(x,y,inc)

PRIVATE
    contador;                           // Contador de uso general

BEGIN
    ctype=c_scroll;                     // Pon el gr fico dentro del scroll
    LOOP
        graph=0;                        // Elige un gr fico nulo
        flags=0;                        // Sin ning£n tipo de efecto espejo
        FROM contador=0 TO 23;          // Pon 24 gr ficos
            FRAME;
        END

        graph=420;                      // Elige gr fico inicial
        suena_sonido(s_toque,50,128);   // Haz sonido de toque

        FOR (contador=-inc;contador<=inc;contador++);        // Realiza movimiento hasta el incremento (subida)
            IF (graph==420)             // Alterna los gr ficos
                graph=421;              // Si esta quitado lo pone
            ELSE                        // Y si esta puesto
                graph=420;              // Lo quita
            END
            IF (contador==1)            // Si ha llegado arriba
                flags=2;                // Da la vuelta al gr fico (mirando hacia abajo)
            END
            FRAME;
            y+=contador;                // Mueve objeto
        END

    END
END

//-----------------------------------------------------------------------------
// Proceso para el objeto de la barra de energ¡a
//-----------------------------------------------------------------------------

PROCESS barra_energia();

BEGIN
    graph=10;                               // Elige el gr fico inicial
    x=160;                                  // Pon coordenadas
    y=220;
    z=-9;                                   // Pon la barra por encima de los gr ficos

    WHILE (energia_enemigo>0)               // Mientras se tenga energ¡a
        // Sube barra energ¡a desde fuera de la pantalla a su posici¢n
        IF (y>190)
            y--;
        ELSE
          IF (region!=10)                   // Si no tiene regi¢n cr‚ala
            region=10;
            objeto(0,11,160,190,-10,0);     // Crea detr s el gr fico de la barra vac¡a
          END
        END
        // Define la regi¢n respecto al tama¤o de la barra de energ¡a
        // y as¡ solo ver la zona que representa a la misma
        define_region(10,120,185,1+energia_enemigo*5,10);
        FRAME;
    END

    // Elimina el proceso de la barra que hay detr s (barra vac¡a)
    signal(son,s_kill);
END

//-----------------------------------------------------------------------------
// Proceso para el objeto de la conexi¢n bola de pinchos
// Entradas: 'x' 'y' Coordenadas del objeto
//-----------------------------------------------------------------------------

PROCESS bola_pin(x,y)

BEGIN
    file=1;                    // Elige el primer fichero de gr ficos
    graph=20;                  // Elige el gr fico inicial
    ctype=c_scroll;            // Pone el objeto dentro del scroll
    priority=100;              // Pone la prioridad baja para ligar a la cadena
    cadena_bola(id,20);        // Crea cadena de pinchos
    z=-20;                     // Pone por encima de la pantalla

    LOOP
        angulo2+=pi/64;        // Mueve la cadena y la bola de pinchos
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para el objeto de cadena de bola de pinchos
// Entradas: 'primero'     Identificador a la conexi¢n
//           'distancia'  Distancia a la conexi¢n
//-----------------------------------------------------------------------------

PROCESS cadena_bola(primero,distancia)

BEGIN
    file=1;                             // Elige el primer fichero de gr ficos
    graph=21;                           // Elige el gr fico inicial
    ctype=c_scroll;                     // Pone el objeto dentro del scroll
    z=-10;                              // Pone la cadena por detr s de la conexi¢n

    IF (distancia<80)                       // Si el tama¤o restante es lo suficientemente grande
        cadena_bola(primero,distancia+20);  // Crea otra bola de cadena
    ELSE
        bola_final(primero,distancia+32);   // Si no crea la bola de pinchos final
    END

    LOOP
        angulo2=primero.angulo2;                     // Coge el  ngulo de la conexi¢n
        x=primero.x+get_distx(angulo2,distancia);    // Halla las coordenadas respecto
        y=primero.y+get_disty(angulo2,distancia);    // a la conexi¢n y al  ngulo
        FRAME;
    END

END

//-----------------------------------------------------------------------------
// Proceso para el objeto de la bola de pinchos final
// Entradas: 'primero'     Identificador a la conexi¢n
//           'distancia'  Distancia a la conexi¢n
//-----------------------------------------------------------------------------

PROCESS bola_final(primero,distancia)

BEGIN
    file=1;                                      // Elige el fichero de gr ficos 1
    graph=22;                                    // Pon el gr fico inicial
    ctype=c_scroll;                              // Dentro del scroll
    z=-20;                                       // Por delante de la cadena

    LOOP
        angulo2=primero.angulo2;                    // Coge el  ngulo de la conexi¢n
        x=primero.x+get_distx(angulo2,distancia);   // Actualiza coordenadas respecto a
        y=primero.y+get_disty(angulo2,distancia);   // la distancia a la conexi¢n y el  ngulo
        // Haz sonido de estrellas;
        suena_sonido(s_estrella,10,768);
        FRAME;
    END

END




