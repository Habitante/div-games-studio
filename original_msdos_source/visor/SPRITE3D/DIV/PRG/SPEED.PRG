
//------------------------------------------------------------------------------
//TITULO:      SPEED_FOR_DUMMIES
//AUTOR:       DANIEL NAVARRO
//FECHA        1/15/97
//------------------------------------------------------------------------------

PROGRAM Speed_for_dummies;

GLOBAL
    coche_1[]=8,-12,-11,10,11,12,13,14,-13; // Graficos para xgraph
    coche_2[]=8,-22,-21,20,21,22,23,24,-23; // Son rotatorios en  ngulos de 45 grados
    coche_3[]=8,-32,-31,30,31,32,33,34,-33;
    coche_4[]=8,-42,-41,40,41,42,43,44,-43;

    numero;             // Numero de puntos de control
    primer_punto=44;    // Punto de control de la meta

    sluces;             // Identificadores de sonido
    smotor;
    scrash;
    scoli;

    canal[4];           // Identificadores de canal de sonido

    opcion;             // Opcion del menu

    primero;            // Graficos de los coches del podium
    segundo;
    tercero;
    cuarto;
    nivel=0;            // Nivel de dificultad con textos
    tnivel[]="FACIL","MEDIO","DIFICIL";

    recorrido=0;        // Numero de recorrido y textos
    trecorrido[]="BOSQUE","DESIERTO";

    vueltas=0;          // Numero de vueltas y textos
    tvueltas[]="3 VUELTAS","6 VUELTAS","9 VUELTAS";
    tvueltas2[]="/3","/6","/9";

    velocidad_coches_ordenador; // En centesimas de pixel

LOCAL
    velocidad;          // Velocidad coche
    max_velocidad;      // Maxima velocidad
    pos;                // Posicion del coche (1..4)
    vuelta;             // Numero de vuelta
    contador_vuelta;    // Vuelta (solo 1..n)
    donde;              // Donde esta el coche coche
    punto;              // Cercania al punto de control

PRIVATE
    coche1;             // Identificador coche uno
    coche2;             // Identificador coche dos
    coche3;             // Identificador coche tres
    coche4;             // Identificador coche cuatro
    idobjetos;          // Identificador de objetos
    fin_usuario=FALSE;  // Controla la pulsacion de la tecla de escape

BEGIN

    // Carga sonidos
    sluces=load_pcm("speed\barco.pcm",0);
    smotor=load_pcm("speed\tractor.pcm",1);
    scrash=load_pcm("speed\choque.pcm",0);
    scoli=load_pcm("speed\metal7.pcm",0);

    set_fps(32,0);  // Selecciona velocidad de muestreo

    // Carga graficos
    load_fpg("speed\speed.fpg");
    load_fpg("speed\pantalla.fpg");

    // Carga graficos
    load_fnt("speed\menu.fnt");
    load_fnt("speed\g_numero.fnt");
    load_fnt("speed\g_mayusc.fnt");

    LOOP                        // Bucle principal

        load_pal("speed\pantalla.fpg"); // Carga la paleta correcta
        fade_on();                      // Enciende la pantalla
        put_screen(1,1);                // Pone el fondo de pantalla

        // Presentacion
        file=1;
        graph=5;                        // Selecciona el grafico de presentacion
        flags=0;
        x=160;
        y=100;

        FROM size=0 TO 95 STEP 5;       // Acerca el titulo (mayor tama¤o)
            FRAME;
        END

        menu();                         // Crea el menu principal


        FROM size=100 TO 62 STEP -2;    // Rota el titulo y colocalo en su sitio
            angle-=pi/10;
            x+=5;
            y+=3;
            FRAME;
        END

        graph=6;                            // Selecciona otro titulo de presentacion
        size=100;                           // Pone valores correctos
        angle=0;

        minicoche(26,128);                  // Crea el cursor en forma de minicoche

        opcion=0;                           // Pone a 0 la variable de opciones

        REPEAT                              // Bucle del menu
            IF (key(_enter) OR key (_space) OR key (_control))
                opcion=(son.y-110)/18;      // Toma la posicion del minicoche
            END
            IF (key(_esc))                  // Si se pulsa la tecla escape, sale del programa
                opcion=4;
            END
            IF (opcion==3)
                menu_opciones();            // Va al menu de opciones
                opcion=0;
            END
            FRAME;
        UNTIL (opcion<>0)

        signal(TYPE minicoche,s_kill);      // Elimina los procesos del menu
        signal(TYPE menu,s_kill_tree);

        fade_off();                         // Apaga la pantalla
        timer=0;
        IF (opcion==4)                      // Sale al dos
            put_screen(1,2);                // Imprime creditos
            graph=0;                        // Borra el grafico de presentacion

            // Pone los textos de creditos
            write (1,120,50,4,"- CREDITOS -");
            write (1,120,75,4,"PROGRAMADOR:");
            write (1,120,90,4,"DANIEL NAVARRO");
            write (1,120,110,4,"GRAFICOS:");
            write (1,120,125,4,"RAFAEL BARRASO");
            write (1,120,145,4,"SONIDOS:");
            write (1,120,160,4,"LUIS SUREDA");
            write (1,120,180,4,"DIV GAMES STUDIO");
            fade_on();

            // Repite hasta que se pulse una tecla
            REPEAT
                FRAME;
            UNTIL (scan_code<>0)
            // Apaga la pantalla, borra los textos y sale del programa
            fade_off();
            delete_text(all_text);
            exit("Gracias por jugar!",0);
            FRAME;
        END

        // Pone valores iniciales
        velocidad_coches_ordenador=500+nivel*100;

        IF (recorrido==0)                   // Carga recorrido elegido
            load_fpg("speed\bosque.fpg");
            load_pal("speed\bosque.fpg");
            numero=194;
        ELSE
            load_fpg("speed\desierto.fpg");
            load_pal("speed\desierto.fpg");
            numero=192;
        END
        canal[0]=sound(smotor,100,256);     // Inicia el sonido de motores
        canal[1]=sound(smotor,100,256);

        clear_screen();                     // Borra el fondo de la pantalla

        IF (opcion==1)                      // Un jugador
            canal[2]=sound(smotor,100,256);
            canal[3]=sound(smotor,100,150);
            define_region(1,0,0,320,64);    // Crea una region para el horizonte (monta¤as)
            start_scroll(0,2,101,0,1,15);   // Crea el horizonte del fondo (scroll)
            start_mode7(0,2,100,0,0,64);    // Crea el suelo
            m7.color=203;                   // Pone diversos parametros del modo 7
            m7.height=128;
            m7.focus=224;

            // Crea el coche del jugador
            coche1=jugador(&coche_1,0,primer_punto,_left,_right,_up,_down);

            m7.camera=coche1;               // La camara sigue al coche del jugador

            // Crea los coches manejador por el ordenador
            coche2=coche(&coche_2,primer_punto+1);
            coche3=coche(&coche_3,primer_punto+2);
            coche4=coche(&coche_4,primer_punto+3);

            // Pone los mensajes de posicion y numero de vueltas
            write(3,160,0,1,"POS");
            write_int(2,160,14,1,&coche1.pos);
            write(3,278,0,1,"VUELTA");
            write(2,290,14,1,tvueltas2[vueltas]);
            write_int(2,254,14,1,&coche1.contador_vuelta);

        ELSE                                // Dos jugadores
            canal[2]=sound(smotor,100,150);
            canal[3]=sound(smotor,100,150);

            define_region(1,0,0,320,16);    // Crea la region para el horizonte
            define_region(2,0,0,320,99);    // Crea la region para carretera
            define_region(3,0,100,320,16);  // Crea otra region para el horizonte
            define_region(4,0,100,320,99);  // Crea una region para la otra carretera
            start_scroll(0,2,101,0,1,15);   // Crea dos horizontes uno para cada jugador (scroll)
            start_scroll(1,2,101,0,3,15);
            start_mode7(0,2,100,0,2,16);    // Crea dos suelos con la carreteras (modo 7)
            start_mode7(1,2,100,0,4,16);
            scroll.y0=48;                   // Inicializa parametros del horizontes
            scroll[1].y0=48;
            m7.color=203;                   // Inicializa valores para los modos 7
            m7.height=78;
            m7.focus=224;
            m7[1].color=203;
            m7[1].height=78;
            m7[1].focus=224;

            // Crea los coches de los jugadores
            coche1=jugador(&coche_1,0,primer_punto,_left,_right,_up,_down);
            coche2=jugador(&coche_2,1,primer_punto+1,_r,_t,_q,_a);
            m7[0].camera=coche1;
            m7[1].camera=coche2;             // Posiciona las camaras

            // Crea los coches manejados por el ordenador
            coche3=coche(&coche_3,primer_punto+2);
            coche4=coche(&coche_4,primer_punto+3);

            // Imprime los mensajes de posicion y numero de vueltas
            write(3,140,-4,1,"POS");
            write_int(3,170,-4,1,&coche1.pos);
            write(3,140,96,1,"POS");
            write_int(3,170,96,1,&coche2.pos);
            write(3,260,-4,1,"VTA");
            write(3,300,-4,1,tvueltas2[vueltas]);
            write_int(3,285,-4,1,&coche1.contador_vuelta);
            write(3,260,96,1,"VTA");
            write(3,300,96,1,tvueltas2[vueltas]);
            write_int(3,285,96,1,&coche2.contador_vuelta);

        END

        // Crea  los objetos del mapa
        FROM z=0 TO 9;
            get_point(2,100,z,&x,&y);   // Coge el punto de control del mapa
            objeto(200,x,y);            // Y crea el objeto
        END

        FROM z=10 TO 14;                // Crea m s objetos con otros gr ficos
            get_point(2,100,z,&x,&y);
            objeto(201,x,y);
        END
        FROM z=15 TO 19;
            get_point(2,100,z,&x,&y);
            objeto(202,x,y);
        END

        minimapa();      // Muestra el minimapa del recorrido
        fade_on();
        priority=-1;
        file=0;
        graph=200;      // Pone el grafico para el semaforo
        x=160;          // Y sus coordenadas
        y=100;
        z=-512;

        REPEAT

            // Pone las posiciones (orden de llegada) en los coches todos a uno
            coche1.pos=1;
            coche2.pos=1;
            coche3.pos=1;
            coche4.pos=1;
            // Y los coloca segun orden de salida
            IF (coche1.donde>coche2.donde)
                coche2.pos++;
            ELSE
                coche1.pos++;
            END
            IF (coche1.donde>coche3.donde)
                coche3.pos++;
            ELSE
                coche1.pos++;
            END
            IF (coche1.donde>coche4.donde)
                coche4.pos++;
            ELSE
                coche1.pos++;
            END

            IF (coche2.donde>coche3.donde)
                coche3.pos++;
            ELSE
                coche2.pos++;
            END
            IF (coche2.donde>coche4.donde)
                coche4.pos++;
            ELSE
                coche2.pos++;
            END
            IF (coche3.donde>coche4.donde)
                coche4.pos++;
            ELSE
                coche3.pos++;
            END

            // Sube o baja la velocidad de los coches controlados por el ordenador
            // Dependiendo de si van detras o delante del coche de los jugadores
            IF (opcion==1)
                IF (coche1.pos==1)
                    IF (coche4.max_velocidad<velocidad_coches_ordenador+64)
                        coche2.max_velocidad++;
                        coche3.max_velocidad++;
                        coche4.max_velocidad++;
                    END
                END
                IF (coche1.pos==4)
                    IF (coche4.max_velocidad>velocidad_coches_ordenador-64)
                        coche2.max_velocidad--;
                        coche3.max_velocidad--;
                        coche4.max_velocidad--;
                    END
                END
            ELSE
                IF (coche1.pos+coche2.pos==3)
                    IF (coche4.max_velocidad<velocidad_coches_ordenador+64)
                        coche3.max_velocidad++;
                        coche4.max_velocidad++;
                    END
                END
                IF (coche1.pos+coche2.pos==7)
                    IF (coche4.max_velocidad>velocidad_coches_ordenador-64)
                        coche3.max_velocidad--;
                        coche4.max_velocidad--;
                    END
                END
                IF (coche1.pos<coche2.pos)
                    coche1.max_velocidad=700;
                    coche2.max_velocidad=900;
                ELSE
                    coche1.max_velocidad=900;
                    coche2.max_velocidad=700;
                END
            END

            // Controla el semaforo inicial
            IF (timer>800 AND timer<900)
                graph=0;
                signal(idobjetos,s_kill);
                timer=900;
            ELSE
                IF (timer>600 AND timer<700)
                    sound(sluces,1000,500);
                    sound(sluces,1000,256);
                    idobjetos.x+=56;

                    idobjetos.graph++;
                    flags=4;
                    timer=700;
                ELSE
                    IF (timer>400 AND timer<500)
                        sound(sluces,200,256);
                        idobjetos.x+=56;
                        timer=500;
                    ELSE
                        IF (timer>200 AND timer<300)
                            sound(sluces,200,256);
                            idobjetos=objeto2(201,103,100,-600); // Luz verde
                            timer=300;
                        END
                    END
                END
            END

            // Lee la tecla escape y pone la variable de salida a True(Verdadero)
            IF (key(_esc)) fin_usuario=TRUE; END

            FRAME;
        // Repite hasta que el o los coches protagonistas acaben o se pulse la tecla escape y el usuario acabe
        UNTIL ((coche1.vuelta==vueltas*3+4 AND opcion==1) OR (coche1.vuelta>=vueltas*3+4 AND coche2.vuelta>=vueltas*3+4) OR fin_usuario)

        IF (fin_usuario)    // Si se ha acabado por la tecla escape apaga la pantalla (a negro)
            fade_off();
        ELSE                // Si es porque se acabo la carrera apaga la pantalla de otra manera
            fade(0,0,0,1);
            WHILE (fading)  // Espera hasta que se apage la pantalla
                FRAME;
            END
        END

        // Elimina todos los procesos, horizontes(scroll) o modos 7 del juego
        unload_fpg(2);
        graph=0;
        signal(id,s_kill_tree);
        signal(id,s_wakeup);
        delete_text(all_text);
        stop_sound(all_sound);
        stop_scroll(0);
        stop_scroll(1);
        stop_mode7(0);
        stop_mode7(1);

        // Podium
        IF (NOT fin_usuario)
            put_screen(0,400);                  // Pone la pantalla de fondo
            objeto2(401,119,163,0);              // Podium

            // Ense¤a los mensaje de Ganador o Perdedor
            IF (opcion==1)
                IF (coche1.pos==1)
                    objeto2(402,160,16,0);       // Ha ganado
                ELSE
                    objeto2(403,160,16,0);       // Ha perdido
                END
            ELSE
                IF (coche1.pos==1)
                    objeto2(404,168,34,0);       // Jugador 1 ha ganado
                ELSE
                    IF (coche2.pos==1)
                        objeto2(405,168,34,0);   // Jugador 2 ha ganado
                    ELSE
                        objeto2(406,160,16,0);   // Han perdido
                    END
                END
            END

            // Informacion del recorrido
            write(3,260,76,4,trecorrido[recorrido]);
            write(3,260,92,4,tnivel[nivel]);
            write(3,260,108,4,tvueltas[vueltas]);

            // Muestra los coches en el podium
            selecciona_posicion(coche1.pos,coche1.xgraph);
            selecciona_posicion(coche2.pos,coche2.xgraph);
            selecciona_posicion(coche3.pos,coche3.xgraph);
            selecciona_posicion(coche4.pos,coche4.xgraph);
            podium_coches(primero,120,118);
            podium_coches(segundo,120-80,118+16);
            podium_coches(tercero,120+80,118+32);
            objeto2([cuarto+3],280,220,1);     // Coche destruido (el ultimo)
            fade_on();

            // Espera a pulsar la tecla enter
            WHILE (NOT key(_esc) AND NOT key(_enter))
                humo(280+rand(-8,8),210+rand(-8,8));
                FRAME;
            END

            // Borra y elimina todos los procesos en pantalla
            fade_off();
            delete_text(all_text);
            signal(id,s_kill_tree);
            signal(id,s_wakeup);
        ELSE
            fin_usuario=FALSE;
        END
    END
END

//------------------------------------------------------------------------------
// Proceso menu_opciones
// Controla el menu de opciones
//------------------------------------------------------------------------------

PROCESS menu_opciones();

PRIVATE
    texto[3];               // Identificadores para texto
    enter_presionado=FALSE; // Controla la pulsacion de la tecla -enter-

BEGIN

    // Escribe los textos de las opciones
    texto[0]=write(1,0,0,4,tnivel[nivel]);
    texto[1]=write(1,0,0,4,trecorrido[recorrido]);
    texto[2]=write(1,0,0,4,tvueltas[vueltas]);
    texto[3]=write(1,0,0,4,"SALIR");

    // Para otros procesos
    signal(father,s_freeze);
    signal(TYPE minicoche,s_sleep);

    file=1;
    graph=7;
    x=104;
    y=-40;              // Selecciona grafico y coordenadas
    sombra_menu();      // Crea una sombra al menu
    WHILE (y<45)        // Baja el menu con los textos
        y+=4;
        move_text(texto[0],104,y-28);
        move_text(texto[1],104,y-28+18);
        move_text(texto[2],104,y-28+18*2);
        move_text(texto[3],104,y-28+18*3);
        FRAME;
    END
    minicoche(26,21);     // Pone dos cursores uno a cada lado
    minicoche(182,21);
    LOOP
        IF (key(_enter))                // Comprueba si se pulsa la tecla enter (intro)
            IF (NOT enter_presionado)   // Hace que se ejecute solo una vez
                SWITCH ((son.y-3)/18);  // Coge la posicion del cursor
                    CASE 1:
                        delete_text(texto[0]);  // Borra texto antiguo
                        nivel=(nivel+1)%3;      // Cambia el nivel de dificultad
                        texto[0]=write(1,104,y-28,4,tnivel[nivel]);
                    END
                    CASE 2:
                        delete_text(texto[1]);
                        recorrido=++recorrido MOD 2;    // Cambia el recorrido
                        texto[1]=write(1,104,y-28+18,4,trecorrido[recorrido]);
                    END
                    CASE 3:
                        delete_text(texto[2]);
                        vueltas=++vueltas MOD 3;      // Cambia el numero de vueltas
                        texto[2]=write(1,104,y-28+18*2,4,tvueltas[vueltas]);
                    END
                    DEFAULT:
                        BREAK;                  // Sale del men£ de opciones
                    END
                END
                enter_presionado=TRUE;          // Hace que se ejecute solo una vez
            END
        ELSE
            enter_presionado=FALSE;
        END
        IF (key(_esc))                          // Si se pulsa escape se sale del menu
            BREAK;
        END
        FRAME;
    END
    signal(TYPE minicoche,s_sleep);             // Duerme a los procesos minicoches
    FROM y=45 TO -40 STEP -4;        // Sube el menu
        move_text(texto[0],104,y-28);
        move_text(texto[1],104,y-28+18);
        move_text(texto[2],104,y-28+18*2);
        move_text(texto[3],104,y-28+18*3);
        FRAME;
    END

    // Borra todos los procesos del menu de opciones
    delete_text(all_text);
    signal(father,s_wakeup);
    signal(TYPE minicoche,s_wakeup);
    signal(id,s_kill_tree);
END

//------------------------------------------------------------------------------
// Proceso objeto
// Muestra un grafico cogiendo sus datos como parametros
//------------------------------------------------------------------------------

PROCESS objeto2(graph,x,y,z);

BEGIN
    LOOP
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso podium_coches
// Muestra los graficos del coche en el podium
//------------------------------------------------------------------------------

PROCESS podium_coches(xgraph,x,y);

BEGIN
    size=50;            // A la mitad de tama¤o
    LOOP
        angle+=pi/12;   // Rota el grafico
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso selecciona_posicion
// Selecciona la posicion de los coches en el podium
//------------------------------------------------------------------------------

PROCESS selecciona_posicion(pos,xgraph);

BEGIN
    SWITCH (pos);           // Asigna las tablas de gr ficos seg£n la posici¢n
        CASE 1:
            primero=xgraph;
        END
        CASE 2:
            segundo=xgraph;
        END
        CASE 3:
            tercero=xgraph;
        END
        CASE 4:
            cuarto=xgraph;
        END
    END
END

//------------------------------------------------------------------------------
// Proceso menu
// Muestra los graficos del menu
//------------------------------------------------------------------------------

PROCESS menu();

BEGIN
    file=1;
    graph=3;
    x=104;
    y=155;                      // Selecciona grafico y coordenadas
    sombra_menu();              // Crea una sombra al menu

    FROM size=0 TO 95 STEP 5;
        FRAME;                  // Haz el menu cada vez mas grande hasta su tama¤o
    END
    LOOP
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso sombra_menu
// Muestra una sombra en los menus
//------------------------------------------------------------------------------

PROCESS sombra_menu();

BEGIN
    FRAME(0);   // Espera al proceso de menu que lo ha llamado
    file=1;
    graph=4;
    flags=4;    // Pone transparencia
    z=1;        // Lo pone al fondo
    priority=-1;
    LOOP
        x=father.x;
        y=father.y;
        size=father.size;
        FRAME;             // Selecciona respecto al menu que lo llamo
    END
END

//------------------------------------------------------------------------------
// Proceso minicoche
// Muestra y controla el cursor ( coche peque¤o)
//------------------------------------------------------------------------------

PROCESS minicoche(x,arriba);

PRIVATE
    cont_graficos=0;    // Contador de gr ficos

BEGIN
    file=1;
    y=arriba;   // Coordenada vertical inicial
    LOOP     // Lee teclas
        IF (key(_down)AND y<arriba+18*3)
            y+=18;  // Baja
        END
        IF (key(_up)AND y>arriba)
            y-=18;  // SUbe
        END
        cont_graficos++;
        cont_graficos=cont_graficos MOD 8; // Hace que vaya de 8 en 8
        graph=10+cont_graficos;     // Anima el grafico
        FRAME(200);
    END
END

//------------------------------------------------------------------------------
// Proceso jugador
// Maneja el coche del jugador
// Entradas:
//      pant             = numero de movimiento horizontal
//      punto            = Punto de control de comienzo
//      izquierda..abajo = teclas de control
//------------------------------------------------------------------------------

PROCESS jugador(xgraph,pant,punto,izquierda,derecha,arriba,abajo);

PRIVATE
    hvelocidad; // Altura para los saltos de los coches
    color;      // Mapa de textura
    id2;        // Identificador de coches
    contador0;  // Contador de uso general y distancia entre coches
    s_volumen;  // Volumen de los sonidos
    cont_canal; // Contador de canal

BEGIN
    // Inicia las variables
    max_velocidad=800;
    velocidad=0;
    vuelta=1;

    // Coge la posici¢n de salida de un punto de control
    get_point(2,100,punto,&x,&y);

    // Pone las coordenadas con resolucion (*100)
    resolution=100;
    x=x*resolution;
    y=y*resolution;
    ctype=c_m7;                 // Coge la posicion de inicio

    sombra();                   // Crea la sombra del coche
    bola([xgraph+7]+1,id);      // Crea los graficos del mapa peque¤o

    donde=busca_donde();        // Toma la posicion en carrera inicial
    WHILE (timer<700)           // Espera a acabar el semaforo
        FRAME;
    END

    LOOP
        // Cambia el sonido del motor dependiendo de la velocidad
        change_sound(canal[3-pant],200,abs(velocidad)+150);

        // Mira en que posici¢n de carrera esta
        donde=busca_donde();

        IF (key(izquierda))                     // Comprueba la tecla usada para la izquierda
            scroll[pant].x0-=max_velocidad/64;  // Mueve el horizonte
            angle+=pi/(51200/max_velocidad);    // Cambia la direccion del coche
        END

        IF (key(derecha))                       // Comprueba la tecla usada para la derecha
            scroll[pant].x0+=max_velocidad/64;
            angle-=pi/(51200/max_velocidad);
        END

        // Comprueba la tecla usada para arriba (acelerar)
        IF (key(arriba)AND velocidad<max_velocidad)
            velocidad+=max_velocidad/50;    // Acelera
        END

        // Comprueba la tecla usada para arriba (frenar)
        IF (key(abajo)AND velocidad>-max_velocidad/2)
            velocidad-=max_velocidad/50;    // Frena
        END

        advance(velocidad);                 // Hace avanzar al coche

        // Limites de velocidad
        IF (velocidad>max_velocidad/100)
            velocidad-=max_velocidad/100;
        ELSE
            IF (velocidad<-max_velocidad/100)
                velocidad+=max_velocidad/100;
            ELSE
                velocidad=0;
            END
        END

        // Colision entre los coches manejados por el jugador
        IF (id2=get_id(TYPE jugador))   // Coge el identificador del jugador
            contador0=get_dist(id2);    // Mira si esta a menos de 16 puntos
            IF (contador0<1600)         // Han chocado
                IF(timer[2]>60)         // Pone el sonido de colision
                    timer[2]=0;
                    sound(scoli,500,256);
                END

                IF (donde<id2.donde)    // El coche esta por detras
                    IF (velocidad>0)
                        velocidad=velocidad*9/10;   // Frena un poco
                    END
                ELSE
                    IF (id2.velocidad>0)
                        id2.velocidad=id2.velocidad*9/10;
                    END
                END
                IF (rand(0,10)==0)                  // De vez en cuando echa humo
                    humo((x+id2.x)/200,(y+id2.y)/200);
                END
            END
        END

        // Colisiona con los coches manejados por el ordenador
        cont_canal=0;
        WHILE (id2=get_id(TYPE coche))
            contador0=get_dist(id2);    // Coge la distancia hasta el coche del ordenador
            s_volumen=contador0/16;     // Sonido de motor
            IF (s_volumen>600) s_volumen=600; END
            change_sound(canal[cont_canal],600-s_volumen,700);
            cont_canal++;
            IF (contador0<1600)         // Hay colision
                IF(timer[1]>60)         // Para el sonido de colision
                    timer[1]=0;
                    sound(scoli,500,256);
                END
                IF (donde<id2.donde)
                    IF (velocidad>0)
                        velocidad=velocidad*9/10;   // Frena al coche
                    END
                ELSE
                    IF (id2.velocidad>0)
                        id2.velocidad=id2.velocidad*9/10;
                    END
                END
                // Hecha un poco de humo aleatoriamente
                IF (rand(0,10)==0)
                    humo((x+id2.x)/200,(y+id2.y)/200);
                END
            END
        END

        // Coge el color del mapa de durezas de donde esta el coche
        color=map_get_pixel(2,100,x/100,y/100);

        IF (color<208 AND color>191)    // Comprueba si se ha salido de la carretera
            sound(scrash,1000,256);
            change_sound(canal[3-pant],100,150); // Cambia el sonido de motor
            // Hace un poco de humo
            FROM contador0=0 TO 9;
                humo(x/100+rand(-8,8),y/100+rand(-8,8));
            END

            velocidad=-velocidad/6;     // Quita velocidad
            hvelocidad=24;
            REPEAT                      // Hace que haga un salto
                FRAME;
                advance(velocidad);
                height+=hvelocidad;
                hvelocidad-=4;
            UNTIL (height<0)

            height=12;
            hvelocidad=12;
            REPEAT                      // Hace otros saltos m s peque¤os

                FRAME;
                advance(velocidad);
                height+=hvelocidad;
                hvelocidad-=4;
            UNTIL (height<0)

            height=0;
        END
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso sombra
// Crea la sombra de los coches
//------------------------------------------------------------------------------

PROCESS sombra();

BEGIN
    ctype=c_m7;
    graph=100;
    flags=4;        // Selecciona grafico
    resolution=100; // Pone la resoluci¢n de coordenadad igual que el proceso padre
    priority=-1;    // Hace que el proceso despues del coche
    z=1;            // Pinta por debajo del coche
    LOOP
        x=father.x; // Sigue al coche
        y=father.y;
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso humo
// Crea el humo del coche
//------------------------------------------------------------------------------

PROCESS humo(x,y);

BEGIN
    ctype=father.ctype; // Dentro del modo 7, como el padre
    graph=101;
    flags=4;            // Seleccion grafico
    REPEAT
        IF (ctype==c_m7)
            height+=4;  // Altura del modo 7
        ELSE
            y-=2;       // Altura en la pantalla
        END
        size-=5;        // Cada vez mas peque¤o

        FRAME;
    UNTIL (size<=0)
END

//------------------------------------------------------------------------------
// Proceso coche
// Controla coches manejados por el ordenador
// Entradas: punto = Punto de control de inicio
//------------------------------------------------------------------------------

PROCESS coche(xgraph,punto);

PRIVATE
    siguiente_p;    // Siguiente punto de control
    siguiente_p_x;  // Coordenadas de siguiente punto de control
    siguiente_p_y;

BEGIN
    // Reincia variables
    max_velocidad=velocidad_coches_ordenador;
    velocidad=0;
    vuelta=1;

    get_point(2,100,punto,&x,&y);   // Coge la posicion inicial
    siguiente_p=punto+2;            // Coge el siguiente punto de control

    resolution=100;
    x*=resolution;
    y*=resolution;
    ctype=c_m7;                     // Pone coordenadas

    sombra();               // Crea la sombra del coche
    bola([xgraph+7]+1,id);  // Crea la bola del minimapa del coche

    // Coge el siguiente punto de control en el mapa de modo 7
    get_point(2,100,siguiente_p,&siguiente_p_x,&siguiente_p_y);
    siguiente_p_x*=100;
    siguiente_p_y*=100;

    donde=busca_donde();    // Toma la posicion en la carrera inicial
    WHILE (timer<700)       // Espera a la se¤al del semaforo
        FRAME;
    END

    LOOP

        // Mira en que posici¢n se encuentra el coche en la carrera
        donde=busca_donde();

        // Halla y guarda el  ngulo del siguiente punto de control dentro del mapa de modo 7
        angle=fget_angle(x,y,siguiente_p_x,siguiente_p_y);

        advance(velocidad);             // Mueve el coche en el  ngulo elegido

        IF (velocidad<max_velocidad)    // Limites de velocidad
            velocidad+=10;
        END
        IF (velocidad>max_velocidad)
            velocidad=max_velocidad;
        END

        // Cuando el coche esta cerca del punto de control
        IF (fget_dist(x,y,siguiente_p_x,siguiente_p_y)<1600)
            siguiente_p+=2;             // Incrementa el contador de puntos de control

            IF (rand(0,8)==0)           // Cambia la trayectoria
                siguiente_p=(siguiente_p&-2)+rand(0,1);
            END

            IF (siguiente_p>=primer_punto+numero) // Avanza una vuelta
                siguiente_p-=numero;
            END

            // Coge el siguiente punto de control
            get_point(2,100,siguiente_p,&siguiente_p_x,&siguiente_p_y);
            siguiente_p_x*=100;
            siguiente_p_y*=100;
        END
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso objeto
// Pone los objetos dentro del recorrido
//------------------------------------------------------------------------------

PROCESS objeto(graph,x,y);

BEGIN
    file=2;
    ctype=c_m7;    // Selecciona fichero y tipo de scroll
    LOOP
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso minimapa
// Muestra un peque¤o mapa
//------------------------------------------------------------------------------

PROCESS minimapa();

BEGIN
    x=36;
    y=36;
    file=2;
    z=1;
    flags=4;
    graph=104;          // Selecciona grafico y coordenadas

    IF (opcion==2)      // Para dos jugadores
        CLONE
            y+=100;     // Crea otro minimapa m s abajo
        END
    END

    LOOP
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso bola
// Muestra los coches que son bolas en el minimapa
//------------------------------------------------------------------------------

PROCESS bola(graph,idcoche);

PRIVATE
    coord_y=0;      // Posicion y

BEGIN
    IF (opcion==2)          // Para dos jugadores
        CLONE
            coord_y=100;    // Crea otra bola m s abajo
        END
    END

    LOOP
        x=idcoche.x/1600+4;
        y=idcoche.y/1600+4+coord_y;     // Actualiza coordenadas respecto al coche
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso busca_donde
// Retorna la posicion dentro de la carrera
//------------------------------------------------------------------------------

PROCESS busca_donde();

PRIVATE
    distancia=0;    // Distancia al punto de control

    min_punto=0;    // Posiciones de los puntos dentro de la carrera
    min_donde1=0;
    min_donde2=0;
    min_donde3=0;

    min_distancia1=-1; // Distancia a min_donde1
    min_distancia2=-1; // Distancia a min_donde2
    min_distancia3=-1; // Distancia a min_donde3


BEGIN
    vuelta=father.vuelta;
    punto=(father.punto&-2)-10; // Coge 5 puntos de control por detras y por delante

    IF (punto<primer_punto)     // Mueve una vuelta para atras
        punto+=numero;
        vuelta--;
    END

    FROM z=0 TO 10;             // Coge los tres puntos de control mas cercanos
        get_point(2,100,punto,&x,&y);

        distancia=fget_dist(father.x/100,father.y/100,x,y);

        IF (distancia>512)      // Si el punto esta muy lejos
            distancia=0;        // No lo cojas
        ELSE
            distancia=512-distancia;
        END

        // Ordena las distancias segun cercania
        IF (distancia>min_distancia1)
            min_donde3=min_donde2;
            min_distancia3=min_distancia2;
            min_donde2=min_donde1;
            min_distancia2=min_distancia1;
            min_punto=punto;
            min_donde1=punto+vuelta*200;
            min_distancia1=distancia;
        ELSE
            IF (distancia>min_distancia2)
                min_donde3=min_donde2;
                min_distancia3=min_distancia2;
                min_donde2=punto+vuelta*200;
                min_distancia2=distancia;
            ELSE
                IF (distancia>min_distancia3)
                    min_donde3=punto+vuelta*200;
                    min_distancia3=distancia;
                END

            END
        END
        IF ((punto+=2)>=primer_punto+numero)    // Avanza una vuelta
            punto-=numero;
            vuelta++;
        END
    END

    // Controla el paso por meta
    IF (min_punto+numero/2<father.punto)        // Hacia delante
        father.vuelta++;
    END

    IF (min_punto>father.punto+numero/2)        // Hacia atras
        father.vuelta--;
    END

    // Normaliza las vueltas
    IF (father.vuelta<1)
        father.contador_vuelta=1;
    ELSE
        IF (father.vuelta>vueltas*3+3)
            father.contador_vuelta=vueltas*3+3;
        ELSE
            father.contador_vuelta=father.vuelta;
        END
    END

    // Actualiza el punto del proceso que le ha llamado
    father.punto=min_punto;

    distancia=min_distancia1+min_distancia2+min_distancia3;

    IF (distancia<>0)    // Interpola los puntos (hace media ponderada)
        donde=(min_donde1*100*min_distancia1+min_donde2*100*min_distancia2+min_donde3*100*min_distancia3)/distancia;
    ELSE
        donde=min_donde3;
    END

    RETURN(donde);      // Retorna el punto donde se encuentra

END

