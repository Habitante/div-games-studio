
//------------------------------------------------------------------------------
// TITULO: HELIO BALL
// AUTOR:  ANTONIO MARCHAL CORTS
// FECHA:  07/07/97
//------------------------------------------------------------------------------

PROGRAM helio_ball;

CONST
    jugador1=1;     // Constantes de los tipos de jugador
    jugador2=2;
    ordenador1=3;
    ordenador2=4;

GLOBAL
    fichero1;       // Identificador a los gr ficos del juego
    fichero2;       // Identificador a los gr ficos de los bonus
    fichero3;       // Identificador a los gr ficos del menu
    idestadio;      // Identificador al gr fico del estadio
    fuente1;        // Identificadores a las letras usadas
    fuente2;
    fuente3;
    fuente4;
    puntos1,puntos2;        // Puntuaci¢n de los jugadores
    qjugador1=jugador1;     // Tipo de jugadores
    qjugador2=jugador2;     // Por defecto jugador1 contra jugador 2
    modojuego=0;            // Modo de juego (0=jugador1 contra jugador2
                            //                1=ordenador1 contra jugador2
                            //                2=jugador1 contra ordenador2
                            //                3=ordenador1 contra ordenador2
    tiempototal=60;         // Tiempo que dura el partido
    tiempojuego;            // Tiempo que queda por jugar
    idnave1;                // Identificadores a las naves
    idnave2;
    idpelota0;              // Identificador a la pelota
    sibonus=0;              // Bandera. 1= Ya hay un bonus
    tiempobonus=-1;         // Tiempo que lleva el bonus
    contador;               // Contador de uso general

    idspuntos;              // Identificadores a los sonidos
    idsgol;
    idschoque1;
    idschoque2;
    idschoque3;

LOCAL
    anguloreal;             // Angulo real de las naves
    velocidadreal;          // Velocidad real de las naves
    velocidadpelota;        // Velocidad real de la pelota
    angulopelota;           // Angulo real de la pelota
    increm_x;               // Necesario para el rebote de las naves
    increm_y;
    vel_inc_x=0;            // Incremento horizontal de la nave
    vel_inc_y=0;            // Incremento vertical de la nave
    cual;                   // Selecciona que tipo de nave es
    vmaxima=2000;           // Velocidad m xima de las naves
    fuerzaotra=0;           // Fuerza de empuje de mas en las naves
    bonus=0;                // Bonus que ha recogido la nave
    aceleraci¢n=400;        // aceleraci¢n de las naves
    frenada=800;            // Frenada de las naves

PRIVATE
    // Textos para las opciones
    textojugador[]="-","TECLAS CURSOR","TECLAS Q-A-R-T","ORDENADOR 1","ORDENADOR 2";
    textotiempo[]="-","060 SEGUNDOS","120 SEGUNDOS","180 SEGUNDOS","240 SEGUNDOS","300 SEGUNDOS";
    idtexto1,idtexto2,idtexto3;     // Identificadores de los textos
    tipotiempo=1;                   // Duraci¢n del partido
    idfondo;                        // Identificador para poner los fondos
    cuentaprorroga;                 // Contador para el texto de la prorroga
    idprorroga;                     // Identificador del texto de prorroga
    idsmenu;                        // Identificadores a sonido
    idsinicio;
    idsfinal;
    boton_aceptar=0;                // Comprueba si se eligio una opci¢n del men£
BEGIN
    set_mode(m640x480);             // Selecciona el modo de v¡deo
    max_process_time=1000;          // Se pone que abarque mas para equipos lentos

    // Carga los ficheros de los gr ficos
    fichero1=load_fpg("heliobal\naves.fpg");
    fichero2=load_fpg("heliobal\items.fpg");
    fichero3=load_fpg("heliobal\menu.fpg");

    // Carga las letras necesarias
    fuente1=load_fnt("heliobal\digital0.fnt");
    fuente2=load_fnt("heliobal\digital1.fnt");
    fuente3=load_fnt("heliobal\digital.fnt");
    fuente4=load_fnt("heliobal\digital2.fnt");

    // Carga los sonidos
    idspuntos=load_pcm("heliobal\bonus.pcm",0);
    idsgol=load_pcm("heliobal\goal.pcm",0);
    idschoque1=load_pcm("heliobal\choque1.pcm",0);
    idschoque2=load_pcm("heliobal\valla.pcm",0);
    idschoque3=load_pcm("heliobal\choque2.pcm",0);

    // Inicialmente es ordenador contra ordenador
    modojuego=3;
    qjugador1=3;
    qjugador2=4;

    WHILE (NOT key(_esc))

        // Carga el sonido y la pantalla con la paleta iniciales
        idsmenu=load_pcm("heliobal\menu.pcm",0);
        idfondo=load_map("heliobal\cartel.map");
        load_pal("heliobal\cartel.map");
        put_screen(0,idfondo);          // Pone la pantalla
        fade_on();                      // Enciende la pantalla y hace el sonido
        sound(idsmenu,75,256);
        WHILE (scan_code==0 AND NOT mouse.left)
            FRAME;
        END                             // Espera a que se pulse 'espacio' o 'escape'
        IF (key(_esc))                  // Si se ha pulsado 'escape' sale del programa
            fade_off();
            creditos();
            FRAME;
        END
        fade_off();                     // Apaga la pantalla
        unload_pcm(idsmenu);            // Y descarga el sonido para tener mas pantalla
        unload_map(idfondo);
        idfondo=load_map("heliobal\ciudad.map");
        load_pal("heliobal\ciudad.map");

        // Pone la pantalla de fondo y escribe los mensajes
        put_screen(0,idfondo);
        idtexto1=write(fuente1,320,37,4,textojugador[qjugador1]);
        write(fuente1,320,87,4,"CONTRA");
        idtexto2=write(fuente1,320,137,4,textojugador[qjugador2]);
        write(fuente1,320,237,4,"TIEMPO");
        idtexto3=write(fuente1,320,287,4,textotiempo[tipotiempo]);

        // Crea las barras del menu
        barra_menu(320,40,997,fichero3,0,5);
        barra_menu(320,40,995,fichero3,4,10);
        barra_menu(320,140,997,fichero3,0,5);
        barra_menu(320,140,995,fichero3,4,10);
        barra_menu(320,290,998,fichero3,0,5);
        barra_menu(320,290,996,fichero3,4,10);
        barra_menu(320,450,994,fichero3,0,5);
        barra_menu(320,450,993,fichero3,4,10);


        // Asignamos el gr fico del rat¢n
        mouse.graph=999;mouse.file=fichero3;
        fade_on();                      // Enciende la pantalla
        boton_aceptar=0;                // Reinicia variable de comprobaci¢n
        WHILE (boton_aceptar==0 AND NOT key(_esc))
            // Comprueba si esta pulsado el rat¢n y en que posici¢n
            IF (mouse.x>20 AND mouse.x<620 AND
                mouse.y>12 AND mouse.y<68
                AND mouse.left)         // Cambia el tipo de jugador de la izquierda
                WHILE(mouse.left) FRAME; END   // Espera a que se suelte el rat¢n
                delete_text(idtexto1);  // Borra el texto antiguo
                IF (qjugador1==1)
                    qjugador1=3;        // Ordenador
                ELSE
                    qjugador1=1;        // Humano
                END
                // Pone el texto nuevo
                idtexto1=write(fuente1,320,37,4,textojugador[qjugador1]);
            END
            // Comprueba si esta pulsado el rat¢n y en que posici¢n
            IF (mouse.x>20 AND mouse.x<620 AND
                mouse.y>112 AND mouse.y<168
                AND mouse.left)         // Cambia el tipo de jugador de la derecha
                WHILE(mouse.left) FRAME; END   // Espera a que se suelte el rat¢n
                delete_text(idtexto2);  // Borra el texto antiguo
                IF (qjugador2==2)
                    qjugador2=4;        // Ordenador
                ELSE
                    qjugador2=2;        // Humano
                END
                // Pone el texto nuevo
                idtexto2=write(fuente1,320,137,4,textojugador[qjugador2]);
            END
            // Comprueba si esta pulsado el rat¢n y en que posici¢n
            IF (mouse.x>113 AND mouse.x<527 AND
                mouse.y>262 AND mouse.y<318
                AND mouse.left)         // Cambia la duraci¢n del partido
                WHILE(mouse.left) FRAME; END   // Espera a que se suelte el rat¢n
                delete_text(idtexto3);  // Borra el texto antiguo
                tipotiempo++;
                IF (tipotiempo>5) tipotiempo=1; END
                // Pone el texto nuevo
                idtexto3=write(fuente1,320,287,4,textotiempo[tipotiempo]);
            END
            // Comprueba si esta pulsado el rat¢n y en que posici¢n
            IF (mouse.x>202 AND mouse.x<438 AND
                mouse.y>422 AND mouse.y<478
                AND mouse.left)         // Mira si se pulsa el bot¢n de aceptar
                WHILE(mouse.left) FRAME; END   // Espera a que se suelte el rat¢n
                boton_aceptar=1;
            END
            tiempototal=tipotiempo*60;  // Reinicializa el tiempo
            // Selecciona el modo de juego dependiendo del tipo de jugador
            IF (qjugador1==1)
                IF (qjugador2==2) modojuego=0; ELSE modojuego=1; END
            ELSE
                IF (qjugador2==2) modojuego=2; ELSE modojuego=3; END
            END
            FRAME;
        END

        // Quitamos el gr fico del rat¢n
        mouse.graph=0;
        IF (key(_esc))  // Si se ha pulsado 'escape' sal del programa
            fade_off();
            creditos();
            FRAME;
        END
        fade_off();
        unload_map(idfondo);    // Descarga la pantalla de fondo por la memoria

        // Carga el gr fico del estadio y su paleta
        idestadio=load_map("heliobal\estadio.map");
        load_pal("heliobal\naves.fpg");
        delete_text(all_text);  // Borra cualquier texto

        // Eliminamos cualquier proceso
        let_me_alone();
        clear_screen();
        iniciacampo();          // Reinicializa campo
        fade_on();

        // Pone los puntos y el tiempo a cero
        timer[0]=0; puntos1=0; puntos2=0;
        tiempojuego=tiempototal-(timer[0]/100);

        // Escribe los texto de los marcadores
        write_int(fuente3,147,-2,2,&puntos1);
        write_int(fuente3,636,-2,2,&puntos2);
        write_int(fuente3,356,-2,2,&tiempojuego);
        marcadores();   // Pone los fondos de los marcadores

        // Carga sonido del inicio del juego
        idsinicio=load_pcm("heliobal\inigame.pcm",0);
        sound(idsinicio,75,256);    // Haz sonido de inicio

        // Repite hasta que se acabe el tiempo
        WHILE (NOT key(_esc) AND tiempojuego>=0)

            // Si hay empate suma 30 segundos mas de prorroga
            IF (tiempojuego<1 AND puntos1==puntos2)
                timer[0]=0;
                tiempototal=30;
                cuentaprorroga=30;
            END

            // Normaliza el tiempo de juego para no imprimir negativos
            IF (tiempojuego<0) tiempojuego=0; END
            IF (cuentaprorroga>0) // Si ha habido prorroga por el texto
                idprorroga=write(fuente2,320,280,4,"PRORROGA!");
            END

            FRAME;

            // Actualiza el tiempo que queda de juego
            tiempojuego=tiempototal-(timer[0]/100);

            // Pone los objetos de bonus
            IF (rand(0,10)==0 AND idnave1.bonus==0 AND idnave2.bonus==0 AND sibonus==0)
                objetos();
            END

            // Si esta el puesto el texto de prorroga lo quita
            IF (cuentaprorroga>0)
                cuentaprorroga--;
                delete_text(idprorroga);
            END
        END

        // Descarga el sonido por motivos de memoria
        unload_pcm(idsinicio);

        // Para los procesos de las naves y la pelota
        signal(idnave1,s_freeze);
        signal(idnave2,s_freeze);
        signal(idpelota0,s_freeze);

        // Carga el sonido de final de partido
        idsfinal=load_pcm("heliobal\timeout.pcm",0);
        IF (tiempojuego<1)
            idsfinal=load_pcm("heliobal\timeout.pcm",0);
            tiempojuego=0;
            contador=0;
            write(fuente2,320,280,4,"FINAL DE PARTIDO!");
            sound(idsfinal,75,256);
            // Espera hasta que se pulsa la barra espaciadora o pasa un tiempo
            WHILE (NOT key(_space) AND contador<30)
                FRAME;
                contador++;
            END
            unload_pcm(idsfinal);
        END
        // Apaga la pantalla
        fade_off();
        // Borra todos los textos y procesos
        delete_text(all_text);
        let_me_alone();
        // Elimina los movimientos de pantalla (scrolles)
        stop_scroll(0);stop_scroll(1);
   END
END

//------------------------------------------------------------------------------
// Proceso pelota
// Controla la pelota
//------------------------------------------------------------------------------

PROCESS pelota()

PRIVATE
    idnave;             // Identificador de las naves
    sumaangx,sumaangy;  // Usados para sumar  ngulos
    angx,angy;
    vel_real;           // Para uso temporal guarda la velocidad real
    ultix,ultiy;        // Ultimas coordenadas del objeto

BEGIN
    ctype=c_scroll;     // Introduce al proceso dentro del scroll
    graph=50;           // Elige el gr fico y las coordenadas iniciales
    x=640;
    y=rand(400,560);
    ultix=x;ultiy=y;    // Inicializa el gr fico
    LOOP
        // Mueve la pelota
        x+=get_distx(angulopelota,velocidadpelota/200);
        y+=get_disty(angulopelota,velocidadpelota/200);

        // Frena la pelota
        IF (velocidadpelota>0)
            velocidadpelota-=100;
        ELSE
            velocidadpelota=0;
        END

        // Detecta si ha tocado a la nave
        IF (idnave=collision(TYPE nave))
            sound(idschoque3,rand(5,50),256);   // Hace sonido

            // Coge la velocidad de la nave mas un incremento
            velocidadpelota=(idnave.velocidadreal)+1200;

            // Mira si la nave es controlada por el jugador
            IF (idnave.cual==jugador1 OR idnave.cual==jugador2)
                vel_real=idnave.velocidadreal;
                IF (vel_real>400) vel_real=400; END

                // Suma el  ngulo que lleva la nave con el que
                // forman la pelota y la nave
                sumaangx=get_distx(get_angle(idnave)+180000,400);
                sumaangy=get_disty(get_angle(idnave)+180000,400);
                angx=get_distx(idnave.anguloreal,vel_real);
                angy=get_disty(idnave.anguloreal,vel_real);
                sumaangx+=angx; sumaangy+=angy;

                // Mueve la pelota y actualiza el  ngulo
                angulopelota=fget_angle(0,0,sumaangx,sumaangy);
                x+=get_distx(angulopelota,velocidadpelota/200);
                y+=get_disty(angulopelota,velocidadpelota/200);
            ELSE        // O controlada por el ordenador
                angulopelota=(get_angle(idnave)+180000) mod 360000;
            END
        END

        // Comprueba si rebota con las paredes superior e inferior
        IF (y<195 OR y>758)
            angulopelota=(fget_angle(0,0,(get_distx(angulopelota,200)),-(get_disty(angulopelota,200))));
            velocidadpelota+=100;
            x+=get_distx(angulopelota,velocidadpelota/200);
            y+=get_disty(angulopelota,velocidadpelota/200);
            IF (y<195) y=196; END
            IF (y>758) y=757; END
        END

        // Comprueba si esta en los lados derecho o izquierdo del campo
        IF (x<131 OR x>1069)
            // Comprueba si estaba dentro de la porter¡a
            IF  (ultiy=>400 AND ultiy<=544)
               // Comprueba el rebote con los laterales de la porter¡a
               IF (y<400 OR y>544)
                    angulopelota=(fget_angle(0,0,(get_distx(angulopelota,200)),-(get_disty(angulopelota,200))));
                    velocidadpelota+=100;
                    x+=get_distx(angulopelota,velocidadpelota/200);
                    y+=get_disty(angulopelota,velocidadpelota/200);
                    IF (y<400) y=401; END
                    IF (y>544) y=543; END
               END
               // Comprueba si se ha metido gol
               IF (x<75 OR x>1126)
                   IF (x<76) puntos2+=50; ELSE puntos1+=50; END
                   golmarcado();    // Se ha metido gol
               END
        ELSE
            // Realiza el rebote con los laterales derecho o izquierdo del campo
            angulopelota=(fget_angle(0,0,-(get_distx(angulopelota,200)),(get_disty(angulopelota,200))));
            velocidadpelota+=100;
            x+=get_distx(angulopelota,velocidadpelota/200);
            y+=get_disty(angulopelota,velocidadpelota/200);
            IF (x<131) x=132; END
            IF (x>1069) x=1068; END
           END
        END
        FRAME(50);
        ultix=x;ultiy=y;    // Se guarda para la comprobaciones con la porter¡a
    END
END

//------------------------------------------------------------------------------
// Proceso nave
// Controla las naves de los jugadores
//------------------------------------------------------------------------------

PROCESS nave(cual)

PRIVATE
    marco;                  // Variable para sacar el  ngulo de la nave
    idnave;                 // Identificador de la otra nave
    tvelocidad0;            // Suma de las velocidades de las naves antes de chocar
    tvelocidad1;            // Suma de las velocidades de las naves despu‚s de chocar
    anguloa;                // Angulo entre las naves
    suma_veloc_x,suma_veloc_y;  // Suma de los incrementos x e y de las naves
    tdistancia;             // Distancia entre las dos naves
    idbola;                 // Identificador de la pelota
    inigraph;               // Gr fico inicial de las naves
    angulografico;          // Angulo al que tienden los gr ficos de las naves
    angulofinalg;           // Angulo de los gr ficos de las naves
    sumaangx,sumaangy;      // Suma de los  ngulos de las naves y los formados
    angx,angy;              // por la pelota y la nave
    vel_real;               // Variable temporal para truncar la velocidad
    ultix,ultiy;            // Ultima posici¢n de las naves
    posxpelota,posypelota;  // Posici¢n de donde deben ir las naves controladas por el ordenador
BEGIN
    ctype=c_scroll;         // Introduce al proceso dentro del scroll
    sombra();               // Pone la sombra

    // Pone la posici¢n, los gr ficos y el  ngulo iniciales de las naves
    IF (cual==jugador1 OR cual==ordenador1)
        graph=1; inigraph=1;
        x=580; y=480;
        angle=0;
    ELSE
        graph=25; inigraph=25;
        x=700; y=480;
        angle=180000;
    END
    ultix=x; ultiy=y;   // Guarda la £ltima posici¢n de la nave
    LOOP
        // Maneja el control de la nave dependiendo del tipo de jugador
        SWITCH (cual)
            CASE 1:         // Lee el teclado del jugador 1
                IF (key(_right))
                    IF (vel_inc_x>=0) vel_inc_x+=aceleraci¢n; ELSE vel_inc_x+=frenada; END
                END
                IF (key(_left))
                    IF (vel_inc_x<=0) vel_inc_x-=aceleraci¢n; ELSE vel_inc_x-=frenada; END
                END
                IF (key(_up))
                    IF (vel_inc_y<=0) vel_inc_y-=aceleraci¢n; ELSE vel_inc_y-=frenada; END
                END
                IF (key(_down))
                    IF (vel_inc_y>=0) vel_inc_y+=aceleraci¢n; ELSE vel_inc_y+=frenada; END
                END
            END
            CASE 2:         // Lee el teclado del jugador 2
                IF (key(_t))
                    IF (vel_inc_x>=0) vel_inc_x+=aceleraci¢n; ELSE vel_inc_x+=frenada; END
                END
                IF (key(_r))
                    IF (vel_inc_x<=0) vel_inc_x-=aceleraci¢n; ELSE vel_inc_x-=frenada; END
                END
                IF (key(_q))
                    IF (vel_inc_y<=0) vel_inc_y-=aceleraci¢n; ELSE vel_inc_y-=frenada; END
                END
                IF (key(_a))
                    IF (vel_inc_y>=0) vel_inc_y+=aceleraci¢n; ELSE vel_inc_y+=frenada; END
                END
            END
            CASE 3:         // Inteligencia del ordenador 1
                posxpelota=idpelota0.x+(get_distx(fget_angle(1130,480,idpelota0.x,idpelota0.y),25));
                posypelota=idpelota0.y+(get_disty(fget_angle(1130,480,idpelota0.x,idpelota0.y),25));
                anguloreal=near_angle(anguloreal,fget_angle(x,y,posxpelota,posypelota),15000);
                velocidadreal+=100;
                vel_inc_x=get_distx(anguloreal,velocidadreal);
                vel_inc_y=get_disty(anguloreal,velocidadreal);
            END
            CASE 4:         // Inteligencia del ordenador 2
                posxpelota=idpelota0.x+(get_distx(fget_angle(80,480,idpelota0.x,idpelota0.y),25));
                posypelota=idpelota0.y+(get_disty(fget_angle(80,480,idpelota0.x,idpelota0.y),25));
                anguloreal=near_angle(anguloreal, fget_angle(x,y,posxpelota,posypelota),15000);
                velocidadreal+=100;
                vel_inc_x=get_distx(anguloreal,velocidadreal);
                vel_inc_y=get_disty(anguloreal,velocidadreal);
            END
        END

        // Mueve la nave
        x+=(vel_inc_x/200);
        y+=(vel_inc_y/200);

        // Y la frena un poco
        IF (vel_inc_x>0) vel_inc_x-=40; END
        IF (vel_inc_y>0) vel_inc_y-=40; END
        IF (vel_inc_x<0) vel_inc_x+=40; END
        IF (vel_inc_y<0) vel_inc_y+=40; END

        // Trunca la velocidad con la velocidad m xima
        IF (fget_dist(0,0,vel_inc_x,vel_inc_y)>vmaxima)
            // Lo hace de manera circular
            marco=fget_angle(0,0,vel_inc_x,vel_inc_y);
            vel_inc_x=get_distx(marco,vmaxima);
            vel_inc_y=get_disty(marco,vmaxima);
        END

        // Comprueba el choque entre las naves
        IF ((idnave=collision(TYPE nave)) AND id<>idnave)

            // Realiza un sonido y pone una explosi¢n entre las dos naves
            sound(idschoque1,rand(15,65),256);
            explosi¢n(x+get_distx(get_angle(idnave),get_dist(idnave)/2),y+get_disty(get_angle(idnave),get_dist(idnave)/2));

            // Actualiza las variables de  ngulo y velocidad de las naves
            anguloreal=fget_angle(0,0,vel_inc_x,vel_inc_y);
            velocidadreal=fget_dist(0,0,vel_inc_x,vel_inc_y);
            idnave.anguloreal=fget_angle(0,0,idnave.vel_inc_x,idnave.vel_inc_y);
            idnave.velocidadreal=fget_dist(0,0,idnave.vel_inc_x,idnave.vel_inc_y);

            // Incrementa las velocidad por el choque y las sumas
            velocidadreal+=100;idnave.velocidadreal+=100;
            tvelocidad0=velocidadreal+idnave.velocidadreal;

            // Haya el  ngulo entre las naves
            anguloa=get_angle(idnave);

            // Calcula los incrementos verticales y horizontales
            increm_x=get_distx(anguloreal,velocidadreal);
            increm_y=get_disty(anguloreal,velocidadreal);
            idnave.increm_x=get_distx(idnave.anguloreal,idnave.velocidadreal);
            idnave.increm_y=get_disty(idnave.anguloreal,idnave.velocidadreal);

            // Y sumas los incrementos(=  ngulos de las naves)
            suma_veloc_x=increm_x+idnave.increm_x; suma_veloc_y=increm_y+idnave.increm_y;
            tdistancia=fget_dist(0,0,suma_veloc_x,suma_veloc_y);

            // A una nave se la manda en una direcci¢n
            increm_x-=get_distx(anguloa,tdistancia);
            increm_y-=get_disty(anguloa,tdistancia);

            // Y a la otra en la direcci¢n contraria
            idnave.increm_x+=get_distx(anguloa,tdistancia);
            idnave.increm_y+=get_disty(anguloa,tdistancia);

            // Se actualiza las variables de  ngulo y velocidad
            anguloreal=fget_angle(0,0,increm_x,increm_y);
            velocidadreal=fget_dist(0,0,increm_x,increm_y);
            idnave.anguloreal=fget_angle(0,0,idnave.increm_x,idnave.increm_y);
            idnave.velocidadreal=fget_dist(0,0,idnave.increm_x,idnave.increm_y);

            // Y se iguala a las velocidades iniciales
            tvelocidad1=velocidadreal+idnave.velocidadreal;
            IF (tvelocidad1==0) tvelocidad1=1; END

            // Se actualiza las velocidades de las naves
            velocidadreal=tvelocidad0*velocidadreal/tvelocidad1;
            idnave.velocidadreal=tvelocidad0*idnave.velocidadreal/tvelocidad1;

            // Y se suma un incremento si la otra nave tiene el icono de fuerza
            velocidadreal+=idnave.fuerzaotra;
            idnave.velocidadreal+=fuerzaotra;

            // Se actualiza los incrementos vertical y horizontal
            vel_inc_x=get_distx(anguloreal,velocidadreal);
            vel_inc_y=get_disty(anguloreal,velocidadreal);
            idnave.vel_inc_x=get_distx(idnave.anguloreal,idnave.velocidadreal);
            idnave.vel_inc_y=get_disty(idnave.anguloreal,idnave.velocidadreal);
        END

        // Se actualizan la velocidad y el  ngulo de las naves seg£n los incrementos
        anguloreal=fget_angle(0,0,vel_inc_x,vel_inc_y);
        velocidadreal=fget_dist(0,0,vel_inc_x,vel_inc_y);

        // Si la nave se mueve, se va girando el gr fico hacia su verdadera direcci¢n
        IF (velocidadreal>200)
            angle=near_angle(angle,anguloreal,10000);
            angulografico=anguloreal-angle;
            angulofinalg=near_angle(angulofinalg,angulografico,15000);
            WHILE (angulofinalg<0) angulofinalg+=360000; END
            WHILE (angulofinalg>=360000) angulofinalg-=360000; END
            graph=inigraph+(angulofinalg/15000);
        END

        // Se comprueba si se ha chocado con la pelota
        IF (idbola=collision(TYPE pelota))

            // Se realiza un sonido
            sound(idschoque3,rand(5,50),256);

            // Y se incrementa la velocidad de la pelota seg£n la de la nave
            idbola.velocidadpelota=velocidadreal+1200;

            // Se comprueba si la ha tocado el jugador o el ordenador
            IF (cual==jugador1 OR cual==jugador2)

                // Si es el jugador se halla la suma de los  ngulos
                // de la direcci¢n de la nave y el  ngulo formado entre ambos
                vel_real=velocidadreal;
                IF (vel_real>400) vel_real=400; END
                sumaangx=get_distx((get_angle(idbola)),400);
                sumaangy=get_disty((get_angle(idbola)),400);
                angx=get_distx(anguloreal,vel_real);
                angy=get_disty(anguloreal,vel_real);
                sumaangx+=angx; sumaangy+=angy;
                idbola.angulopelota=fget_angle(0,0,sumaangx,sumaangy);
            ELSE
                // Si es el ordenador se coge el  ngulo entre ambos
                idbola.angulopelota=get_angle(idbola);
            END
        END

        // Comprueba el rebote con el borde superior o inferior
        IF (y<219 OR y>733)
            sound(idschoque2,rand(5,50),256);
            vel_inc_y=-vel_inc_y;
            IF (y<219) y=220; explosi¢n(x,y-25); END
            IF (y>733) y=732; explosi¢n(x,y+25); END
        END

        // Se comprueba si esta en los laterales
        IF (x<154 OR x>1046)
            // Se comprueba si estaba dentro de la porter¡a
            IF  (ultiy=>425 AND ultiy<=531)

                // Se chequea si choca con los bordes de la porter¡a
                IF (y<425 OR y>526)
                    sound(idschoque2,rand(5,50),256);
                    vel_inc_y=-vel_inc_y;
                    IF (y<425) y=426; explosi¢n(x,y-25); END
                    IF (y>526) y=525; explosi¢n(x,y+25); END
                END

                // Se comprueba si choca con el fondo de la porter¡a
                IF (x<80 OR x>1120)
                    sound(idschoque2,rand(5,50),256);
                    vel_inc_x=-vel_inc_x;
                    IF (x<80) x=81; explosi¢n(x-25,y); END
                    IF (x>1120) x=1119; explosi¢n(x+25,y); END
                END
            ELSE

                // Esta chocando con los laterales del campo
                vel_inc_x=-vel_inc_x;
                sound(idschoque2,rand(5,50),256);
                IF (x<154) x=155; explosi¢n(x-25,y); END
                IF (x>1046) x=1045; explosi¢n(x+25,y); END
           END
        END

        FRAME(50);
        ultix=x; ultiy=y;
    END
END

//------------------------------------------------------------------------------
// Proceso golmarcado
// Maneja los goles marcados
//------------------------------------------------------------------------------

PROCESS golmarcado()

PRIVATE
    idtexto;        // Identificador del texto 'gooool'

BEGIN
    sound(idsgol,75,256);                       // Realiza el sonido
    signal(TYPE pelota,s_freeze);               // Para los procesos de la pelota
    signal(TYPE nave,s_freeze);                 // Y las naves
    idtexto=write(fuente2,320,240,4,"GOOOOL!"); // Pone el texto 'GOOOOL!'
    contador=0;
    WHILE (contador<10 AND NOT key(_space))     // Espera un tiempo o la pulsaci¢n de espacio
        contador++;
        FRAME;
    END
    fade_off();
    signal(TYPE pelota,s_kill);                 // Destruye los procesos antiguos
    signal(TYPE nave,s_kill);
    signal(TYPE porterias,s_kill);
    signal(TYPE objetos,s_kill);
    signal(TYPE objetosmarcador,s_kill);
    signal(TYPE sombra,s_kill);
    delete_text(idtexto);                       // Borra el texto
    iniciacampo();                              // Y crea objetos nuevos
    fade_on();                                  // Enciende la pantalla
END

//------------------------------------------------------------------------------
// Proceso explosi¢n
// Imprime las explosiones cuando hay un choque
//------------------------------------------------------------------------------

PROCESS explosi¢n (x,y)

BEGIN
    ctype=c_scroll; // Introduce al proceso dentro del scroll
    flags=4;        // Transparente
    graph=51;       // Elige el gr fico
    z=-40;          // lo pone por encima
    WHILE (graph<58) graph++; FRAME; END    // Realiza la animaci¢n
END

//------------------------------------------------------------------------------
// Proceso marcadores
// Imprime los fondos de los marcadores
//------------------------------------------------------------------------------

PROCESS marcadores()

BEGIN
    x=320;              // Selecciona coordenadas
    y=20;
    graph=60;           // Pone el fondo del marcador de tiempo
    z=-1;
    CLONE               // Crea otro marcador
        graph=59;       // Pone el fondo de marcador del jugador 1
        x=76;           // Cambiando la posici¢n y el gr fico
        CLONE           // Crea otro marcador
            x=564;      // El del jugador 2 cambiando la posici¢n
        END
    END
    LOOP FRAME; END
END

//------------------------------------------------------------------------------
// Proceso iniciacampo
// Reinicializa todos los procesos
//------------------------------------------------------------------------------

PROCESS iniciacampo()

BEGIN
    SWITCH (modojuego)      // Inicia el juego dependiendo de los jugadores

        CASE 0:             // Jugador 1 contra jugador 2
            define_region(1,0,0,319,480);       // Define dos regiones de pantalla
            define_region(2,322,0,319,480);
            start_scroll(0,0,idestadio,0,2,0);  // Y dos movimientos de las mismas
            start_scroll(1,0,idestadio,0,1,0);
            idpelota0=pelota();                 // Crea la pelota
            idnave1=nave(qjugador1);            // Y las dos naves
            idnave2=nave(qjugador2);
            scroll[0].camera=idnave1;           // Hace que las c maras sigan a las naves
            scroll[1].camera=idnave2;
        END
        CASE 1:             // Jugador 1 contra ordenador
            start_scroll(0,0,idestadio,0,0,0);  // Crea el movimiento de pantalla
            idpelota0=pelota();                 // Crea la pelota
            idnave1=nave(qjugador1);            // Y las dos naves
            idnave2=nave(qjugador2);
            scroll[0].camera=idnave1;           // Hace que la c mara siga al jugador
        END
        CASE 2:     // Ordenador contra jugador 2
            start_scroll(0,0,idestadio,0,0,0);  // Crea el movimiento de pantalla
            idpelota0=pelota();                 // Crea la pelota
            idnave1=nave(qjugador1);            // Y las dos naves
            idnave2=nave(qjugador2);
            scroll[0].camera=idnave2;           // Hace que la c mara siga al jugador
        END
        CASE 3:     // Ordenador contra ordenador
            start_scroll(0,0,idestadio,0,0,0);  // Crea el movimiento de pantalla
            idpelota0=pelota();                 // Crea la pelota
            idnave1=nave(qjugador1);            // Y las dos naves
            idnave2=nave(qjugador2);
            scroll[0].camera=idpelota0;         // Hace que la c mara siga a la pelota
        END
    END
    porter¡as();            // Crea las porter¡as
    objetosmarcador();      // Y los fondos de los marcadores

END

//------------------------------------------------------------------------------
// Proceso porter¡as
// Imprime las porter¡as que se imprimen encima
//------------------------------------------------------------------------------

PROCESS porter¡as ()

BEGIN
    ctype=c_scroll; // Introduce al proceso dentro del scroll
    z=-50;
    x=77;           // Selecciona las coordenadas y el gr fico
    y=477;
    graph=61;       // Elige el gr fico y crea la porter¡a izquierda
    CLONE           // Crea otra porter¡a, la de la derecha
        graph=62;   // Cambiando las coordenadas y el gr fico
        x=1122;
        y=478;
    END
    LOOP FRAME; END

END

//------------------------------------------------------------------------------
// Proceso sombra
// Maneja las sombras de las naves
//------------------------------------------------------------------------------

PROCESS sombra()

BEGIN
    ctype=c_scroll; // Introduce al proceso dentro del scroll
    x=father.x;     // Coge las coordenadas del proceso que los llamo
    y=father.y;
    flags=4;        // Lo hace transparente
    z=1;
    graph=63;               // Elige el gr fico
    LOOP
        x=father.x;
        y=father.y;         // Sigue al proceso que lo llamo
        angle=father.angle; // Coge el  ngulo del padre
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso objetos
// Controla los objetos de los bonus
//------------------------------------------------------------------------------

PROCESS objetos ()

PRIVATE
    tipobonus;          // Tipo del bonus: 0-> Puntos
                        //                 1-> Mejor control
                        //                 2-> Mas fuerza de empuje
                        //                 3-> Para a la otra nave
    graficoinicio;      // Gr fico de inicio del objeto
    idnave;             // Identificador de las naves
BEGIN
    ctype=c_scroll;     // Hace que el proceso tenga las coordenadas respecto al scroll
    z=10;
    file=fichero2;      // Elige el fichero de los gr ficos
    tipobonus=rand(0,3);// Y un bonus al azar
    sibonus=1;
    graph=(tipobonus*8)+1;  // Selecciona el gr fico dependiendo del tipo de bonus
    graficoinicio=graph;    // Inicializa el gr fico del objeto
    REPEAT                  // Espera a que no se ponga encima de nada
        x=rand(142,1057);
        y=rand(206,747);
    UNTIL (NOT collision(TYPE nave) AND NOT collision (TYPE pelota))
    size=0;
    timer[1]=0;                             // Pone el contador de tiempo a 0
    WHILE (timer[1]<1500)                   // Lo tiene en pantalla 15 segundos
        IF (size<100) size+=5; END          // Hace que aparezca agrand ndose
        angle+=5000;                        // Lo gira a cada paso
        graph++;                            // Realiza la animacion1
        IF (graph>graficoinicio+7) graph=graficoinicio; END
        FRAME;
        IF (idnave=collision(TYPE nave))    // Mira si ha chocado con una nave
            sound(idspuntos,100,256);
            timer[2]=0;
            explosi¢n(x,y);
            idnave.bonus=tipobonus+1;       // Actualiza la variables bonus de las naves
            IF (idnave.cual==jugador1 OR idnave.cual==ordenador1)
                idnave2.bonus=0;
            ELSE
                idnave1.bonus=0;
            END
            SWITCH (tipobonus)              // Mira que tipo de bonus es
                CASE 0:                     // Da puntos
                    IF (idnave.cual==jugador1 OR idnave.cual==ordenador1)
                        puntos1+=10;
                    ELSE
                        puntos2+=10;
                    END
                    tiempobonus=100;        // Dura un segundo el efecto
                    break;
                END
                CASE 1:                     // Hace que se controle mejor la nave
                    idnave.vmaxima=3000;    // Mayor velocidad
                    idnave.aceleraci¢n=800; // Mas aceleraci¢n
                    idnave.frenada=1600;    // Mas frenada
                    tiempobonus=500;        // Dura 5 segundos
                    break;
                END
                CASE 2:                     // Hace que la fuerza de empuje sea mayor
                    idnave.fuerzaotra=2000;
                    tiempobonus=1500;       // Dura 15 segundos
                    break;
                END
                CASE 3:                     // Para a la otra nave
                    IF (idnave.cual==jugador1 OR idnave.cual==ordenador1)
                        signal(idnave2,s_freeze);
                    ELSE
                        signal(idnave1,s_freeze);
                    END
                    tiempobonus=200;        // Dura dos segundos
                    break;
                END
            END
        END
    END
    explosi¢n(x,y);     // Si el icono desaparece realiza una explosi¢n
    sibonus=0;          // Lo deja preparado para que aparezcan m s bonus
END

//------------------------------------------------------------------------------
// Proceso objetosmarcador
// Controla los objetos cogidos que est n ya en el marcador
//------------------------------------------------------------------------------

PROCESS objetosmarcador()

PRIVATE
    iniciografico;      // Gr fico inicial del objeto
    ultimoobjeto1=0;    // Ultimo objeto recogido por el jugador de la izquierda
    ultimoobjeto2=0;    // Ultimo objeto recogido por el jugador de la derecha
    idnave;             // Identificador de la nave
    tipobonus;          // Tipo de bonus cogido

BEGIN
    z=-10; y=20;        // Selecciona algunas variables y el fichero de gr ficos
    file=fichero2;
    LOOP
        // Mira si lo a cogido el jugador de la izquierda
        IF (idnave1.bonus<>0 AND idnave1.bonus<>ultimoobjeto1)
            iniciografico=((idnave1.bonus-1)*8)+1;
            graph=iniciografico;
            x=177;
            ultimoobjeto1=idnave1.bonus;
            ultimoobjeto2=0;
            timer[2]=0;
        END

        // Mira si lo ha cogido el jugador de la derecha
        IF (idnave2.bonus<>0 AND idnave2.bonus<>ultimoobjeto2)
            iniciografico=((idnave2.bonus-1)*8)+1;
            graph=iniciografico;
            x=463;
            ultimoobjeto2=idnave2.bonus;
            ultimoobjeto1=0;
            timer[2]=0;
        END

        // Si lo tiene alguien anima el gr fico
        IF (idnave1.bonus<>0  OR idnave2.bonus<>0)
            graph++;
            IF (graph>iniciografico+7) graph=iniciografico; END
        END
        FRAME;

        // Comprueba que no ha pasado el tiempo de efecto y que hay bonus
        IF (timer[2]>tiempobonus AND tiempobonus<>-1)
            tiempobonus=-1;
            IF (idnave1.bonus<>0)
                idnave=idnave1;
            ELSE
                idnave=idnave2;
            END
            tipobonus=(idnave.bonus)-1;
            SWITCH (tipobonus)          // Quita el efecto del bonus
                CASE 0:     // Reinicializa variables
                    idnave.bonus=0;
                    timer[2]=0;         // Reinicia tiempo
                    graph=0;            // Quita gr fico
                END
                CASE 1:     // Pone velocidad, frenada y aceleraci¢n como estaban
                    idnave.vmaxima=2000;
                    idnave.aceleraci¢n=400;
                    idnave.frenada=800;
                    idnave.bonus=0;
                    timer[2]=0;         // Reinicia tiempo
                    graph=0;            // Quita gr fico
                END
                CASE 2:     // Deja la fuerza de choque a cero
                    idnave.fuerzaotra=0;
                    idnave.bonus=0;
                    timer[2]=0;         // Reinicia tiempo
                    graph=0;            // Quita gr fico
                END
                CASE 3:     // Hace que la otra nave se mueva
                    IF (idnave.cual==jugador1 OR idnave.cual==ordenador1)
                        signal(idnave2,s_wakeup);
                    ELSE
                        signal(idnave1,s_wakeup);
                    END
                    idnave.bonus=0;
                    timer[2]=0;         // Reinicia tiempo
                    graph=0;            // Quita gr fico
                END
            END
        END
   END
END

//------------------------------------------------------------------------------
// Proceso creditos
// Pone los creditos del programa y sale del juego
//------------------------------------------------------------------------------

PROCESS creditos()

PRIVATE
    idfondo;    // Identificador del gr fico de fondo

BEGIN
    // Elimina cualquier proceso o texto que hubiera
    let_me_alone();
    delete_text(all_text);

    // Carga la pantalla de fondo con su paleta y la pone en pantalla
    idfondo=load_map("heliobal\creditos.map");
    load_pal("heliobal\creditos.map");
    put_screen(0,idfondo);

    // Escribe los textos de los creditos
    write(fuente4,320,45,4,"PROGRAMADOR:");
    write(fuente4,320,90,4,"ANTONIO MARCHAL.");

    write(fuente4,320,145,4,"GRAFICOS POR:");
    write(fuente4,320,190,4,"JOSE FERNANDEZ");
    write(fuente4,320,230,4,"RAFAEL BARRASO");

    write(fuente4,320,290,4,"SONIDOS:");
    write(fuente4,320,335,4,"ANTONIO MARCHAL");
    fade_on();  // Enciende la pantalla

    // Espera hasta que se pulse una tecla
    scan_code=0;
    WHILE (scan_code==0)
        FRAME;
    END
    fade_off(); // Apaga la pantalla
    exit("Gracias por jugar!",0);   // Sale del programa
END

//------------------------------------------------------------------------------
// Proceso barra_menu
// Imprime las barras de las barras del menu...
// Entradas: Variables predefinidas del sistema
//------------------------------------------------------------------------------

PROCESS barra_menu(x,y,graph,file,flags,z)

BEGIN
    LOOP
        FRAME;
    END
END
