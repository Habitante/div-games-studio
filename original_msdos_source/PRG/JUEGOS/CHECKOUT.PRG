
//-----------------------------------------------------------------------------
// TITULO: CHECK OUT
// AUTOR:  DANIEL NAVARRO MEDRANO
// FECHA:  01/09/96
//-----------------------------------------------------------------------------

PROGRAM Checkout;

CONST
    pieza_jugador=42;   // Valor de las piezas del jugador
    pieza_ordenador=50; // Valor de las piezas del ordenador
    no_pieza=36;        // Indica cuando no hay ninguna pieza
    prof_min=3;         // Profundidad m¡nima de b£squeda

GLOBAL

    fichero1,fichero2;  // Identificadores de los ficheros de gráficos
    letras1;            // Letras para los mensajes durante el juego
    letras2,letras3,letras4,letras5,letras6;
    finaljuego=0;       // Controla cuando se acaba el juego
    quiengana=0;        // Sirve para determinar quien gana el juego
    seleccionada;       // Indica que pieza se selecciona con el rat¢n
    que_pieza;          // Indica que pieza se va a mover
    mover;              // Indica hacia donde se va a mover la pieza
    nivel_actual;       // Nivel de dificultad 1 (f cil), 2, 3, ...
    color;              // Qui‚n juega abajo 0-Blancas / 3-Negras
    mueve;              // Indica qui‚n mueve 1-Jugador / 2-Ordenador
    quienhaganado;
    tablero[35];        // Tablero de juego 6x6 posiciones (0..35)

    estrategia1[35];    // Estrategia que sigue el jugador superior
    estrategia2[35];    // Estrategia que sigue el jugador inferior

    niveles[]=          // Colocacion de las piezas en los diferentes niveles

          2,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0,
          0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,1,  // Nivel 0

          0,2,2,0,0,0, 2,0,2,0,0,0, 2,2,0,0,0,0,
          0,0,0,0,1,1, 0,0,0,1,0,1, 0,0,0,1,1,0,  // Nivel 1

          2,2,2,0,0,0, 2,0,2,0,0,0, 2,2,0,0,0,0,
          0,0,0,0,1,1, 0,0,0,1,0,1, 0,0,0,1,1,1,  // Nivel 2

          2,2,2,0,0,0, 2,2,2,0,0,0, 2,2,0,0,0,0,
          0,0,0,0,1,1, 0,0,0,1,1,1, 0,0,0,1,1,1,  // Nivel 3

          2,2,2,2,0,0, 2,0,2,0,0,0, 2,2,0,0,0,1,
          2,0,0,0,1,1, 0,0,0,1,0,1, 0,0,1,1,1,1,  // Nivel 4

          2,2,2,2,0,0, 2,2,2,0,0,0, 2,2,0,0,0,1,
          2,0,0,0,1,1, 0,0,0,1,1,1, 0,0,1,1,1,1;  // Nivel 5

    estrategia[]=   // Estrategias de juego para los diferentes niveles

       3072,133,33,12,08,04, // Nivel 1
         133,33,10,06,02,00,
          33,10,04,01,01,02,
          12,06,01,00,05,08,
          08,02,01,05,05,20,
          04,00,02,08,20,30,

       4096,256,40,20,08,04, // Nivel 2
         256,40,20,06,02,08,
          40,20,04,10,04,10,
          20,06,10,06,08,20,
          08,02,04,08,30,40,
          04,08,10,20,40,75,

        4096,80,40,20,10,10, // Nivel 3
          80,40,20,10,05,05,
          40,20,10,05,10,10,
          20,10,05,10,20,20,
          10,05,10,20,40,40,
          10,05,10,20,40,75,

        5096,60,20,10,08,08, // Nivel 4
          60,20,10,08,05,05,
          20,10,08,05,10,10,
          10,08,05,10,20,20,
          08,05,10,20,40,40,
          08,05,10,20,40,75,

        6096,40,20,10,08,08, // Nivel 5
          40,20,10,08,05,05,
          20,10,08,05,10,10,
          10,08,05,10,20,20,
          08,05,10,20,40,40,
          08,05,10,20,40,75;

    id_piezas=0;        // Identificador del proceso que crea las piezas
    puntuaci¢n;         // Puntuaci¢n obtenida por una jugada

    parpadeo;           // Controla el parpadeo de la pieza seleccionada
    opci¢n;             // opci¢n seleccionada en el men£ (0 Jugar, 1 Salir)

LOCAL
    // Contadores de uso general
    contador_mov;       // Sirve de contador de movimientos
    tipo_movimiento;    // Se¤ala el tipo de movimiento a realizar
    inc_desplazamiento; // Incremento parcial en los desplazamientos de piezas
    mejor_movimiento;   // Guarda el mejor movimiento posible
    columna_tablero;    // Se¤ala la columna del tablero
    tipo_pieza;         // Indica el tipo de pieza
    fila_pieza;         // Indica la fila de la pieza
    columna_pieza;      // Indica la columna de la pieza
    cuentapausa;        // Sirve para contar la pausa del final
    textos1;            // Identificador de textos

    num_mov;            // Numero de movimientos encontrados
    tmovimientos[3*20]= 60 dup (0); // Por cada movimiento se indica (pieza,lado,puntuaci¢n)
    lado;               // Lado que se esta  evaluando
    num_nivel;          // Indica el n£mero de nivel
    contador_filas;     // Contador de filas
    contador_columna;   // Contador de columnas
    profundidad;        // Profundidad a explorar en cada proceso piensa()
    nivel;              // Nivel de profundidad en el que est  cada proceso piensa()
    num_casilla;        // Indica el n£mero de casilla en bucles
    num_piezas;         // Indica el n£mero de piezas

BEGIN

    max_process_time=3000;          // Se pone el tiempo de proceso a 30 segundos
                                    // para que el programa tenga tiempo de pensar
    inicializaci¢n();               // Inicializa el juego

    REPEAT
        menu_opciones();            // Pone el menu de opciones
        WHILE (son) FRAME; END      // Espera a que se acabe con el menu de opciones
        SWITCH (opci¢n)             // Mira que eleccion se ha hecho
            CASE 0:                 // Jugar
                control_juego();    // Empieza la partida
                WHILE (son)         // Repite mientras se este jugando
                    IF (key(_esc))  // Si se pulsa la tecla escape sale del juego
                        let_me_alone();     // Elimina cualquier proceso
                        fade_off();         // Apaga la pantalla
                        clear_screen();     // Borra la pantalla y cualquier texto
                        delete_text(all_text); // Borra los texto que hubiera
                    ELSE
                        FRAME;
                    END
                END
            END
            CASE 2:     // Instruccions
                instrucciones();
                WHILE (son) FRAME; END
            END
        END
    UNTIL (opci¢n==1)   // Salir del programa
    creditos();
END

//-----------------------------------------------------------------------------
// Proceso inicializaci¢n
// Inicializa el modo de v¡deo y carga los ficheros de gr ficos y fuentes
//-----------------------------------------------------------------------------

PROCESS inicializaci¢n()

BEGIN
    // Selecciona el modo de video y los frames por segundo
    set_mode(m640x480);
    set_fps(32,4);

    // Carga los ficheros de graficos
    fichero1=load_fpg("checkout\checkout.fpg");
    fichero2=load_fpg("checkout\checkmnu.fpg");

    // Carga los ficheros de letras
    letras1=load_fnt("checkout\check.fnt");
    letras2=load_fnt("checkout\checkmn2.fnt");
    letras3=load_fnt("checkout\checkmnu.fnt");
    letras4=load_fnt("checkout\checkmn3.fnt");
    letras5=load_fnt("checkout\checkgam.fnt");
    letras6=load_fnt("checkout\checkgm2.fnt");
END

//-----------------------------------------------------------------------------
// Proceso menu_opciones
// Maneja todo el men£ de opciones
//-----------------------------------------------------------------------------

PROCESS menu_opciones()

BEGIN
    // Carga la paleta apropiada y pone el fondo de pantalla
    load_pal("checkout\checkmnu.fpg"):
    put_screen(1,1);

    fade_on();

    // Escribe los mensajes
    write(letras2,320,-16,1,"CHECKOUT");
    write(letras3,320,64,1,"El juego de ordenador");
    write(letras3,320,390,1,"Empezar  Instrucciones  Salir");

    // Pone la barra negra de debajo de las opciones
    xput(1,2,320,420,0,100,4,0);

    // Elige el grafico del rat¢n
    mouse.file=fichero2;
    mouse.graph=4;

    // Repite hasta que se coga una opci¢n
    opci¢n=-1;
    WHILE (opci¢n<0)

        // Si se pulsa la tecla escape se sale del programa
        IF (key(_esc)) opci¢n=1; END

        // Si se pulsa espacio o enter, empieza el juego
        IF (key(_space) OR key(_enter)) opci¢n=0; END

        // Mira si se pulsado el raton por la parte baja de la pantalla
        IF (mouse.left AND mouse.y>370)
            // Mira en que parte y pon la opci¢n elegida
            IF (mouse.x<214)
                opci¢n=0;
            ELSE
                IF (mouse.x>426)
                    opci¢n=1;
                ELSE
                    opci¢n=2;
                END
            END
        END
        FRAME;
    END

    // Se apaga la pantalla, se borra todo y se sale del proceso
    fade_off();
    clear_screen();
    delete_text(all_text);
    nivel_actual=1;
END

//-----------------------------------------------------------------------------
// Proceso creditos
// Muestra los cr‚ditos del programa
//-----------------------------------------------------------------------------

PROCESS creditos()

BEGIN

    // Pone la pantalla de fondo
    put_screen(1,3);
    fade_on();

    // Pone los textos de los creditos
    write(letras4,320,0,1,"Programa por:");
    write(letras3,320,20,1,"Daniel Navarro");
    write(letras4,320,80,1,"Idea original:");
    write(letras3,320,100,1,"Luis F. Fern ndez");
    write(letras4,320,160,1,"Gr ficos:");
    write(letras3,320,180,1,"M. Jes£s Recio");
    write(letras3,320,220,1,"Pablo de la Sierra");
    write(letras3,320,260,1,"J. Ricardo Avella");

    // Pone el grafico del raton
    mouse.file=fichero2;
    mouse.graph=4;

    // Repite hasta que se pulse escape o un boton del rat¢n
    scan_code=0;
    REPEAT
        FRAME;
    UNTIL (mouse.right OR mouse.left OR scan_code<>0)
    let_me_alone();

END

//-----------------------------------------------------------------------------
// Proceso instrucciones
// Muestra las instrucciones del juego
//-----------------------------------------------------------------------------

PROCESS instrucciones()

BEGIN
    // Pone el grafico en pantalla
    put_screen(fichero2,5);
    fade_on();

    // Repite hasta que se pulse escape o un boton del rat¢n
    REPEAT
        FRAME;
    UNTIL (mouse.right OR key(_esc) OR mouse.left)

    // Apaga y borra la pantalla, borra cualquier texto, y devuelve el grafico al rat¢n
    fade_off();
    clear_screen();
    delete_text(all_text);
END

//-----------------------------------------------------------------------------
// Proceso control_juego
// Bucle principal de control del juego
//-----------------------------------------------------------------------------

PROCESS control_juego()

PRIVATE
    idmov;           // Identificador para obtener los movimientos
    mov_posibles;    // N£mero de movimientos que puede hacer una pieza

BEGIN
    // Carga la paleta del juego y pone la pantalla
    load_pal("checkout\checkout.fpg");
    put_screen(0,100);
    fade_on();

    // Elige el grafico del raton
    mouse.file=fichero1;
    mouse.graph=4;
    finaljuego=0;

    REPEAT  // Repite hasta que se acabe el juego

        // Pone la pantalla de fondo
        put_screen(0,100);

        //Pone la variable de quien ha ganado a 0
        quiengana=0;

        REPEAT  // Repite hasta que se acabe la partida

            // Pon el tablero de seleccion de pieza
            pon_nivel(0);
            color=0;
            id_piezas=piezas();

            // Escribe los mensajes explicativos
            write(letras1,320,28,4,"Use el raton para elegir bando");
            write(letras1,290,48,4,"Nivel :");
            write_int(letras1,366,48,4,&nivel_actual);
            num_mov=2*3; // Determina como posibles elecciones las piezas blanca y negra
            tmovimientos[0]=0;
            tmovimientos[3]=35;
            que_pieza=no_pieza;

            REPEAT      // Repite hasta que se elija un color
                signal(id_piezas,s_kill_tree);  // Borra las piezas antiguas
                id_piezas=piezas();             // Crea piezas nuevas
                FRAME;
            UNTIL (mouse.left!=0 AND seleccionada!=no_pieza)

            // Mira que seleccion se ha elegido
            IF (seleccionada==0)
                color=3; mueve=2; // Se eligieron negras, comienza el ordenador
            ELSE
                color=0; mueve=1; // Se eligieron blancas, comienza el jugador
            END
            fade_off();

            // Inicio de la partida
            pon_nivel(nivel_actual);        // Carga el nivel actual
            signal(id_piezas,s_kill_tree);  // Borra las piezas viejas
            id_piezas=piezas();             // Crea las nuevas piezas
            delete_text(all_text);          // Borra cualquier texto en pantalla

            // Carga la estragia del nivel (pesos de cada casilla)
            FROM num_casilla=0 TO 35;
                estrategia1[num_casilla]=estrategia[num_casilla]+rand(0,5)-2;
                estrategia2[35-num_casilla]=estrategia[num_casilla]+rand(0,5)-2; // Espeja la estrategia
            END
            fade_on();

            REPEAT  // Repite hasta que gane alguien
                SWITCH (mueve)
                    CASE 1: // Cuando mueve el jugador ...
                        que_pieza=no_pieza;
                        idmov=piensa(0,mueve,1); // Se mira que posibilidades tiene el jugador
                        num_mov=idmov.num_mov;
                        FROM contador_mov=0 TO 59;
                            tmovimientos[contador_mov]=idmov.tmovimientos[contador_mov];
                        END
                        IF (que_pieza!=no_pieza) // Si tiene al menos un movimiento ...
                            REPEAT  // Repite hasta que gane la partida
                                delete_text(all_text);          // Borra cualquier texto

                                // Pone los texto explicativos y el grafico del raton
                                write(letras1,320,28,4,"Por favor, elija su pieza");
                                mouse.file=fichero1;
                                mouse.graph=4;

                                // En principio no hay seleccionada ninguna ficha
                                que_pieza=no_pieza;
                                signal(id_piezas,s_kill_tree);      // Borra piezas antiguos
                                id_piezas=piezas();                 // Crea fichas nuevas

                                // El jugador selecciona la pieza a mover (aunque solo haya una)
                                REPEAT
                                    signal(id_piezas,s_kill_tree);  // Borra piezas antiguos
                                    id_piezas=piezas();             // Crea fichas nuevas
                                    FRAME;
                                // Repite hasta que se selecciona una ficha
                                UNTIL (mouse.left!=0 AND seleccionada!=no_pieza)
                                que_pieza=seleccionada;

                                // Ahora el jugador debe seleccionar que movimiento quiere hacer
                                delete_text(all_text);          // Borra cualquier texto que hubiera

                                // Escribe textos explicativos
                                write(letras1,320,28,4,"Por favor, elija su movimiento");
                                parpadeo=0;
                                REPEAT
                                    // Hace parpadear a la ficha
                                    IF (parpadeo==1) parpadeo=0; ELSE parpadeo=1; END
                                    signal(id_piezas,s_kill_tree);  // Borra piezas viejas
                                    id_piezas=piezas();             // Crea piezas nuevas
                                    FRAME;
                                // Espera hasta elegir un movimiento o deseleccionar la pieza
                                UNTIL ((mouse.left!=0 AND seleccionada!=no_pieza) || mouse.right==1)

                            UNTIL (mouse.right!=1)
                            mover=seleccionada;

                            // Realiza el movimiento seleccionado
                            tipo_pieza=tablero[que_pieza];
                            movimiento();
                            WHILE (tablero[mover]!=tipo_pieza) FRAME; END  // Espera a acabar el movimiento
                        END
                    END

                    CASE 2: // Cuando mueve el ordenador ...
                        delete_text(all_text);  // Borra cualquier texto

                        // Pone los textos explicativos y el grafico del raton
                        write(letras1,320,28,4,"Por favor espere... Estoy pensando");
                        mouse.graph=0;
                        que_pieza=no_pieza;
                        signal(id_piezas,s_kill_tree);  // Borra piezas viejas
                        id_piezas=piezas();             // Crea piezas nuevas

                        FRAME;                          // Para que se vea el texto mientras piensa
                        piensa(prof_min,mueve,1);       // Piensa que jugada es menos

                        // Si se encontr¢ un movimiento lo realiza
                        IF (que_pieza!=no_pieza)
                            tipo_pieza=tablero[que_pieza];
                            movimiento();
                            WHILE (tablero[mover]!=tipo_pieza) FRAME; END  // Espera ha realizar movimiento
                        END
                    END
                END

            // Se juega hasta que un oponente coloque una pieza en
            // la diagonal contraria, o bien no pueda mover
            UNTIL (tablero[0]==1 OR tablero[35]==2 OR que_pieza==no_pieza)

            // Decide quien de los dos ha ganado
            IF (tablero[0]==1) quiengana=1;END
            IF (tablero[35]==2) quiengana=2; END
            IF (que_pieza==no_pieza)
                IF (mueve==1) quiengana=2; END
                IF (mueve==2) quiengana=1; END
            END
        UNTIL (quiengana<>0)
        // Final de la partida

        // Borra textos explicativos anteriores
        delete_text(all_text);

        // Pone los textos explicativos y el grafico del raton
        write(letras1,320,28,4,"Fin del juego");
        write(letras1,320,48,4,"Presione el boton del raton");
        mouse.file=fichero1;
        mouse.graph=4;
        que_pieza=no_pieza;
        num_mov=0;

        // Espera a que se pulse el raton o una tecla
        scan_code=0; cuentapausa=0;
        REPEAT
            signal(id_piezas,s_kill_tree);  // Borra piezas viejas
            id_piezas=piezas();             // Crea piezas nuevas
            ++cuentapausa;
            FRAME;
        UNTIL (mouse.left OR mouse.right OR scan_code<>0 OR cuentapausa>100)


        // Borra la pantalla, el cualquier texto o pieza que hubiera
        fade_off();
        clear_screen();
        delete_text(all_text);
        signal(id_piezas,s_kill_tree);
        fade_on();

        // Comprueba quien ha ganado
        // Si ha ganado el jugador
        IF (quiengana==1)
            IF (nivel_actual+1<6)   // Se sube un nivel si se puede
                nivel_actual++;
            ELSE
                finaljuego=2;       // Y si no prepara para el mensaje de felicitacion
            END
        END

        // Si ha ganado el ordenador
        IF (quiengana==2)
            IF (nivel_actual-1>0)   // Se baja un nivel si se puede
                nivel_actual--;
            ELSE
                finaljuego=1;       // Y si no se prepara el final del juego
            END
        END
    UNTIL (finaljuego>0)

    // Pone la pantalla de fondo
    put_screen(0,100);

    // Comprueba si se ha hecho todo o ha acabado el juego
    IF (finaljuego==1)  // Ha finalizado el juego por haber perdido
        textos1=write(letras5,320,500,4,"FIN DEL JUEGO");
    ELSE                // Ha finalizado el juego por hacer todos los niveles
        textos1=write(letras5,320,500,4,"GANASTES !");
    END

    // Espera hasta que se pulse el raton o alguna tecla
    scan_code=0; cuentapausa=500;
    REPEAT
        cuentapausa-=2;
        if (cuentapausa>239) move_text(textos1,320,cuentapausa); END
        FRAME;
    UNTIL (mouse.left OR mouse.right OR scan_code<>0 OR cuentapausa<201)
    fade_off();

    // Borra la pantalla y cualquier texto que hubiera
    clear_screen();
    delete_text(all_text);

END

//-----------------------------------------------------------------------------
// Proceso piensa
// Inteligencia del juego, decide que piezas mover en cada jugada
// Entradas: profundidad  = Numero de profundidades a mirar
//           lado  = Que jugador es el que se mueve
//           nivel = Nivel de profundidad donde se esta en este momento
//-----------------------------------------------------------------------------

PROCESS piensa(profundidad,lado,nivel)

BEGIN
    // Reinicializa las variable de n£mero de casillas y la de n£mero movimientos
    num_casilla=0; num_mov=0;

    // Mira que lado es el que se mueve
    IF (lado==1)    // Mueve el lado del jugador
        // Comprueba una a una todas las casillas
        FROM num_casilla=0 TO 35;
            // Comprueba las columnas 0 a 3 de cada fila
            IF (num_casilla MOD 6<4)
                // Si se tiene en la casilla un pieza a la derecha una contraria
                // y detras de ella un espacio (OX.) (O->propia X->contraria .-> vacia)
                IF (tablero[num_casilla+1]==2 AND tablero[num_casilla]==1 AND tablero[num_casilla+2]==0)
                    // Guarda movimiento de comer
                    tmovimientos[num_mov]=num_casilla;          // Guarda posicion donde esta la pieza
                    tmovimientos[num_mov+1]=num_casilla+2;      // Guarda posicion de donde va a ir

                    // Y guarda la puntuacion que es igual a la puntuacion de la
                    // casilla donde esta menos la puntuacion de la casilla donde va
                    // mas la puntuacion de la casilla de la pieza que se va a comer
                    // mas cincuenta puntos porque se esta comiendo
                    tmovimientos[num_mov+2]=estrategia1[num_casilla+2]-estrategia1[num_casilla]+estrategia2[num_casilla+1]+50;

                    // E incrementa el numero de movimientos en 1 (por 3 datos)
                    num_mov=num_mov+3;
                END

                // Comprueba si esta casilla esta vacia la de la derecha tiene
                // una pieza contraria y detras de ella una pieza propia (.XO)
                IF (tablero[num_casilla+1]==2 AND tablero[num_casilla+2]==1 AND tablero[num_casilla]==0)

                    // Si se cumple guarda el movimiento de igual manera que antes
                    tmovimientos[num_mov]=num_casilla+2;
                    tmovimientos[num_mov+1]=num_casilla;
                    tmovimientos[num_mov+2]=estrategia1[num_casilla]-estrategia1[num_casilla+2]+estrategia2[num_casilla+1]+50;
                    num_mov=num_mov+3;
                END
            END

            // Mira las filas 0 a 3 de cada columna
            IF (num_casilla<24)

                // Comprueba si esta casilla se tiene una pieza, si en la de abajo
                // hay una pieza contraria y debajo esta vacio
                IF (tablero[num_casilla+6]==2 AND tablero[num_casilla]==1 AND tablero[num_casilla+12]==0)

                    // Y guarda la puntuacion de movimiento como se guardaba arriba
                    tmovimientos[num_mov]=num_casilla;
                    tmovimientos[num_mov+1]=num_casilla+12;
                    tmovimientos[num_mov+2]=estrategia1[num_casilla+12]-estrategia1[num_casilla]+estrategia2[num_casilla+6]+50;
                    num_mov=num_mov+3;
                END

                // Comprueba si esta casilla esta vacia, si la de encima esta ocupada
                // por una ficha contraria, y encima de ella hay una pieza propia
                IF (tablero[num_casilla+6]==2 AND tablero[num_casilla+12]==1 AND tablero[num_casilla]==0)

                    // Guarda la puntuacion del movimiento como en los casos anteriores
                    tmovimientos[num_mov]=num_casilla+12;
                    tmovimientos[num_mov+1]=num_casilla;
                    tmovimientos[num_mov+2]=estrategia1[num_casilla]-estrategia1[num_casilla+12]+estrategia2[num_casilla+6]+50;
                    num_mov=num_mov+3;
                END
            END
        END
    ELSE
        // Hace la misma comprobacion de comer fichas pero con el otro jugador
        FROM num_casilla=0 TO 35;

            // Mira las columnas
            IF (num_casilla MOD 6<4)
                IF (tablero[num_casilla+1]==1 AND tablero[num_casilla]==2 AND tablero[num_casilla+2]==0)

                    // Si come pone el movimiento con su puntuacion
                    tmovimientos[num_mov]=num_casilla;
                    tmovimientos[num_mov+1]=num_casilla+2;
                    tmovimientos[num_mov+2]=estrategia2[num_casilla+2]-estrategia2[num_casilla]+estrategia1[num_casilla+1]+42;
                    num_mov=num_mov+3;
                END
                IF (tablero[num_casilla+1]==1 AND tablero[num_casilla+2]==2 AND tablero[num_casilla]==0)

                    // Si come pone el movimiento con su puntuacion
                    tmovimientos[num_mov]=num_casilla+2;
                    tmovimientos[num_mov+1]=num_casilla;
                    tmovimientos[num_mov+2]=estrategia2[num_casilla]-estrategia2[num_casilla+2]+estrategia1[num_casilla+1]+42;
                    num_mov=num_mov+3;
                END
            END
            // Mira las filas
            IF (num_casilla<24)
                IF (tablero[num_casilla+6]==1 AND tablero[num_casilla]==2 AND tablero[num_casilla+12]==0)

                    // Si come pone el movimiento con su puntuacion
                    tmovimientos[num_mov]=num_casilla;
                    tmovimientos[num_mov+1]=num_casilla+12;
                    tmovimientos[num_mov+2]=estrategia2[num_casilla+12]-estrategia2[num_casilla]+estrategia1[num_casilla+6]+42;
                    num_mov=num_mov+3;
                END
                IF (tablero[num_casilla+6]==1 AND tablero[num_casilla+12]==2 AND tablero[num_casilla]==0)

                    // Si come pone el movimiento con su puntuacion
                    tmovimientos[num_mov]=num_casilla+12;
                    tmovimientos[num_mov+1]=num_casilla;
                    tmovimientos[num_mov+2]=estrategia2[num_casilla]-estrategia2[num_casilla+12]+estrategia1[num_casilla+6]+42;
                    num_mov=num_mov+3;
                END
            END
        END
    END
    // Mira si hay que comer en este turno
    IF (num_mov>0)

        // Si hay que comer reinicia contadores
        mejor_movimiento=-1; contador_mov=2;

        // Busca si con algun movimiento llega al final y gana
        REPEAT
            IF (tmovimientos[contador_mov]>1000) mejor_movimiento=contador_mov-2; END
        UNTIL ((contador_mov+=3)>num_mov)

        // Si ha encontrado algun movimiento
        IF (mejor_movimiento>=0)

            // Guarda la puntuacion del movimiento
            puntuaci¢n=tmovimientos[mejor_movimiento+2];

            // Y si el nivel de pensamiento es el primero (siguiente jugada)
            IF (nivel==1)

                // Guarda el movimiento para que lo de salida
                que_pieza=tmovimientos[mejor_movimiento];
                mover=tmovimientos[mejor_movimiento+1];
                mueve=mueve XOR 3;
            END
        ELSE    // Si no hay ningun movimiento con que ganar en la siguiente jugada

            // Si hay mas de un movimiento o no esta en el primer nivel de pensamiento
            IF (num_mov>3 OR nivel>1)
                contador_mov=0;
                REPEAT  // Repite hasta que no haya movimientos

                    // Come la ficha para poder pensar otro nivel
                    tablero[tmovimientos[contador_mov]]=0;
                    tablero[tmovimientos[contador_mov+1]]=lado;
                    tablero[(tmovimientos[contador_mov]+tmovimientos[contador_mov+1])/2]=0;

                    // Piensa al movimiento del otro lado (XOR 3)
                    piensa(profundidad,lado XOR 3,nivel+1);

                    // Resta la puntuacion obtenida a la actual
                    tmovimientos[contador_mov+2]=tmovimientos[contador_mov+2]-puntuaci¢n;

                    // Devuelve la ficha a su posicion antigua
                    tablero[tmovimientos[contador_mov]]=lado;
                    tablero[tmovimientos[contador_mov+1]]=0;
                    tablero[(tmovimientos[contador_mov]+tmovimientos[contador_mov+1])/2]=lado XOR 3;
                UNTIL ((contador_mov+=3)==num_mov)
            END

            // Reinicia las variables para busca la mayor puntuacion
            mejor_movimiento=0;
            puntuaci¢n=tmovimientos[2];
            contador_mov=2;

            // Busca la mayor puntuacion de los movimientos de comer
            WHILE ((contador_mov+=3)<num_mov)
                IF (tmovimientos[contador_mov]>puntuaci¢n)
                    mejor_movimiento=contador_mov-2;
                    puntuaci¢n=tmovimientos[contador_mov];
                END
            END

            // Si es el primer nivel de pensamiento guarda los valores
            // para poder comer cuando salga de la rutina
            IF (nivel==1)
                que_pieza=tmovimientos[mejor_movimiento];
                mover=tmovimientos[mejor_movimiento+1];
                mueve=mueve XOR 3;
            END
        END
    ELSE // No hay que comer en este turno, rellena movimientos con su puntuacion

        // Reinicializa las variables para mirar las piezas
        num_piezas=0; //N§ de piezas
        num_mov=0;

        // Mira que lado de las piezas es el que mueve
        IF (lado==1)

            // Va mirando una por una todas las casillas
            FROM num_casilla=0 TO 35;
                columna_tablero=num_casilla MOD 6;   // Coge la columna

                // Si hay una pieza en esa casilla
                IF (tablero[num_casilla]==1)

                    // Incrementa el num_casilla de piezas
                    num_piezas=num_piezas+1;

                    // Si esta entre las columnas 1 a 5 y la casilla
                    // de la derecha esta vacia guarda el movimiento
                    IF (columna_tablero>0 AND tablero[num_casilla-1]==0)

                        // Guarda el movimiento en la tabla
                        tmovimientos[num_mov]=num_casilla;      // Posicion en la que esta
                        tmovimientos[num_mov+1]=num_casilla-1;  // Posicion donde se va a mover

                        // Guarda la puntuacion que es igual a la puntuacion de
                        // casilla a donde va menos la puntuacion de donde esta
                        tmovimientos[num_mov+2]=estrategia1[num_casilla-1]-estrategia1[num_casilla];

                        // Incrementa el numero de movimientos
                        num_mov=num_mov+3;
                    END
                    // Mira si la ficha encontrada esta entre las filas de 1 a 5
                    IF (num_casilla>=6)

                        // Y si la casilla de arriba esta vacia
                        IF(tablero[num_casilla-6]==0)

                            // Guarda el movimiento igual que lo ha guardado arriba
                            tmovimientos[num_mov]=num_casilla;
                            tmovimientos[num_mov+1]=num_casilla-6;
                            tmovimientos[num_mov+2]=estrategia1[num_casilla-6]-estrategia1[num_casilla];
                            num_mov=num_mov+3;
                        END
                    END
                ELSE

                    // Si hay una pieza contraria, suma el numero de piezas
                    IF(tablero[num_casilla]==2)
                        num_piezas=num_piezas+1;
                    END
                END
            END
        ELSE    // Si no esta moviendose las otras piezas

            // Hace el mismo proceso de arriba pero con las otras piezas
            FROM num_casilla=0 TO 35;
                columna_tablero=num_casilla MOD 6;
                IF (tablero[num_casilla]==2)
                    num_piezas=num_piezas+1;
                    IF (num_casilla<35)
                        IF (columna_tablero<5 AND tablero[num_casilla+1]==0)
                            tmovimientos[num_mov]=num_casilla;
                            tmovimientos[num_mov+1]=num_casilla+1;
                            tmovimientos[num_mov+2]=estrategia2[num_casilla+1]-estrategia2[num_casilla];
                            num_mov=num_mov+3;
                        END
                    END
                    IF (num_casilla<30)
                        IF(tablero[num_casilla+6]==0)
                            tmovimientos[num_mov]=num_casilla;
                            tmovimientos[num_mov+1]=num_casilla+6;
                            tmovimientos[num_mov+2]=estrategia2[num_casilla+6]-estrategia2[num_casilla];
                            num_mov=num_mov+3;
                        END
                    END
                ELSE
                    IF(tablero[num_casilla]==1)
                        num_piezas=num_piezas+1;
                    END
                END
            END
        END

        // Si no se ha encontrado ningun movimiento
        IF (num_mov==0)

            // Si es el primer nivel de pensar
            IF (nivel==1)
                que_pieza=36;       // La rutina devuelve ningun movimiento permitido
            ELSE                    // Si esta en otro nivel de pensamiento
                puntuaci¢n=-2048;   // Devuelve una puntuacion muy mala
            END
        ELSE                        // Si hay movimientos posibles

            // Reinicia las variables para empezar a buscar
            mejor_movimiento=-1;
            contador_mov=2;

            // Comprueba si con algun movimiento se llega al final del juego
            REPEAT
                IF (tmovimientos[contador_mov]>1000)
                    mejor_movimiento=contador_mov-2;
                END
            UNTIL ((contador_mov+=3)>num_mov)

            // Si hay algun movimiento con el que ganar
            IF (mejor_movimiento>=0)
                // Guarda la puntuacion
                puntuaci¢n=tmovimientos[mejor_movimiento+2];
                // Y si esta en el primer nivel de pensamiento
                IF (nivel==1)
                    // Guarda los valoresde salida de la rutina para asi
                    // hacer ese movimiento en el proximo turno
                    que_pieza=tmovimientos[mejor_movimiento];
                    mover=tmovimientos[mejor_movimiento+1];
                    mueve=mueve XOR 3;
                END
            ELSE    // Si no tiene ningun moviento con el que ganar

                // Si esta en el primer nivel de pensar
                IF (nivel==1)
                    // Si quedan menos de 6 piezas en el tablero
                    IF (num_piezas<6)
                        profundidad=profundidad+2;          // Incrementa en dos el nivel de
                                                            // profundidad de pensamiendo en 2
                    ELSE
                        // Si quedan menos de 8 piezas incrementa
                        // el nivel de pensamiento en 1
                        IF (num_piezas<8)
                            profundidad=profundidad+1;
                        END
                    END
                END

                // Mientras se tenga nivel de profundidad
                IF (profundidad>0)
                    contador_mov=0;
                    // Mira todos los movimientos posibles
                    REPEAT
                        // Realiza el movimiento
                        tablero[tmovimientos[contador_mov]]=0;
                        tablero[tmovimientos[contador_mov+1]]=lado;
                        // Piensa la estrategia del oponente
                        piensa(profundidad-1,lado XOR 3,nivel+1);
                        // Resta la puntuacion del oponente
                        tmovimientos[contador_mov+2]=tmovimientos[contador_mov+2]-puntuaci¢n;
                        // Restablece el tablero a la posicion de antes de pensar
                        tablero[tmovimientos[contador_mov]]=lado;
                        tablero[tmovimientos[contador_mov+1]]=0;
                    UNTIL ((contador_mov=contador_mov+3)==num_mov)
                END

                // Pone las variables a cero para buscar el mejor movimiento
                mejor_movimiento=0;
                puntuaci¢n=tmovimientos[2];
                contador_mov=2;

                // Repite mientras haya movimientos
                WHILE ((contador_mov+=3)<num_mov)
                    // Si este movimiento es mejor que el ultimo, guardalo
                    IF (tmovimientos[contador_mov]>puntuaci¢n)
                        mejor_movimiento=contador_mov-2;
                        puntuaci¢n=tmovimientos[contador_mov];
                    END
                END

                // Si es el primer nivel de pensamiento
                IF (nivel==1)
                    // Guarda los datos de salida de la rutina
                    que_pieza=tmovimientos[mejor_movimiento];
                    mover=tmovimientos[mejor_movimiento+1];
                    mueve=mueve XOR 3;
                END
            END
        END
    END
END

//-----------------------------------------------------------------------------
// Proceso piezas
// Proceso que crea las piezas
//-----------------------------------------------------------------------------

PROCESS piezas()

PRIVATE
    x_seleccionada,y_seleccionada;     // Para determinar que pieza se selecciona con el rat¢n
    seleccion_raton;

BEGIN

    // Se calcula que pieza est  seleccionada por el rat¢n
    // Calculas la columna y fila del tablero
    y_seleccionada=-(mouse.x*265/10-46*(mouse.y+17)-2438)/2438;
    x_seleccionada=(mouse.x*265/10+46*(mouse.y-300))/2438;

    // Calcula el numero de casilla
    seleccion_raton=y_seleccionada*6+x_seleccionada;

    // Pone a cero las variables para hallar
    seleccionada=36;
    contador_mov=0;

    // Si no hay pieza seleccionada
    IF (que_pieza==36)
        // Busca en todos los movimientos posibles
        WHILE (contador_mov<father.num_mov)
            // Si hay alguno en la posicion que se¤ala el raton
            IF (father.tmovimientos[contador_mov]==seleccion_raton)
                // Y si hay, la coge como seleccionada
                seleccionada=seleccion_raton;
            END
            contador_mov+=3;
        END
    ELSE    // Si hay alguna pieza seleccionada
            // Mira en todos los movimientos posibles
        WHILE (contador_mov<father.num_mov)

            // Si el raton esta se¤alando a donde se pueda mover
            IF (father.tmovimientos[contador_mov]==que_pieza AND
                father.tmovimientos[contador_mov+1]==seleccion_raton)
                // Y elige esa pieza como seleccion
                seleccionada=seleccion_raton;
            END
            contador_mov+=3;
        END
    END

    FROM contador_filas=0 TO 5;         // Recorre todas las filas
        FROM contador_columna=0 to 5;   // Recorre todas las columnas
            // Si esa casilla esta seleccionada
            IF (contador_filas*6+contador_columna==seleccionada)
                // Si en esa casilla hay una pieza
                IF (tablero[contador_filas*6+contador_columna]!=0)
                    // Pone el grafico trasparente de esa pieza
                    pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,4);
                ELSE    // Si no hay pieza
                        // Si no, es un movimiento y pone una piza trasparente
                    IF (que_pieza<36)
                        // Pone el grafico transparente en el lugar donde va a mover
                        pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,mueve XOR color XOR 3,4);
                    END
                END
            ELSE        // Si no hay piezas seleccionada en esa casilla

                // Si es la pieza que se esta moviendo y toca imprimirla por el parpadeo
                IF (contador_filas*6+contador_columna==que_pieza AND parpadeo)
                    // Pone la pieza transparente
                    pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,4);
                ELSE
                    // Si no, si hay pieza
                    IF (tablero[contador_filas*6+contador_columna]!=0)
                        // Imprime una pieza normal
                        pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,0);
                    END
                END
            END
        END
    END
    LOOP FRAME; END
END

//-----------------------------------------------------------------------------
// Proceso pieza
// Pone el grafico de las piezas
//-----------------------------------------------------------------------------

PROCESS pieza(x,y,graph,flags)

BEGIN
  LOOP FRAME; END
END

//-----------------------------------------------------------------------------
// Proceso pon_nivel
// Carga la posicion de las piezas de cada nivel
//-----------------------------------------------------------------------------

PROCESS pon_nivel(n_nivel)

BEGIN
    FROM num_casilla=0 TO 35;
        tablero[num_casilla]=niveles[num_casilla+(n_nivel*36)];
    END
END

//-----------------------------------------------------------------------------
// Proceso movimiento
// Hace el movimiento de la pieza
//-----------------------------------------------------------------------------

PROCESS movimiento()

PRIVATE
    x1,y1;          // Desde donde se realiza el movimiento
    x2,y2;          // Hasta donde se realiza el movimiento

BEGIN
    mouse.graph=0;  // Quita el grafico del raton

    // Destruye las piezas viejas
    signal(id_piezas,s_kill_tree);

    // Inicializa los contadores
    tipo_movimiento=mover-que_pieza;   // Tipo de movimiento
    tipo_pieza=tablero[que_pieza];     // Tipo de pieza

    // Comprueba si es un movimiento de comer
    IF (tipo_movimiento==2 OR tipo_movimiento==-2 OR tipo_movimiento==12 OR tipo_movimiento==-12)
        // Y quita la pieza que se ha comido del tablero
        tablero[(que_pieza+mover)/2]=0;
    END
    tablero[mover]=3;
    tablero[que_pieza]=0;

    // Coge las coordenadas de tablero de donde esta la pieza
    fila_pieza=que_pieza/6;      // Fila
    columna_pieza=que_pieza MOD 6;  // Columna

    // Halla las coordenadas en pantalla
    x1=320+(columna_pieza-fila_pieza)*46;
    y1=124+(columna_pieza+fila_pieza)*265/10;

    // Coge las coordenadas de tablero de la posicion a donde se mueve
    fila_pieza=mover/6;
    columna_pieza=mover MOD 6;

    // Y hallas las coordenadas en pantalla
    x2=320+(columna_pieza-fila_pieza)*46;
    y2=124+(columna_pieza+fila_pieza)*265/10;

    // Comprueba si el movimiento es horizontal
    IF (tipo_movimiento==2 OR tipo_movimiento==-2 OR tipo_movimiento==1 OR tipo_movimiento==-1)

        // Hace el movimiento en 12 partes
        FROM inc_desplazamiento=0 TO 11;

            // Recorre las filas del tablero
            FROM contador_filas=0 TO 5;

                // Recorre las columnas del tablero
                FROM contador_columna=0 TO 5;

                    // Comprueba que la casilla no este vacia
                    IF (tablero[contador_filas*6+contador_columna]!=0)

                        // Mira si es una pieza normal
                        IF (tablero[contador_filas*6+contador_columna]!=3)
                            // Y la imprime
                            pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,0);
                        ELSE    // O es la pieza que se esta movimiento
                            // Tambien se imprime pero con el incremento del movimiento
                            pieza((x1*(11-inc_desplazamiento)+x2*inc_desplazamiento)/11,(y1*(11-inc_desplazamiento)+y2*inc_desplazamiento)/11,tipo_pieza XOR color,0);
                        END
                    END
                END
            END
            FRAME;

            // Despues de imprimirlas, borra las piezas
            signal(id,s_kill_tree);
            signal(id,s_wakeup);
        END
    ELSE    // Si no, es un movimiento vertical
            // Hace el mismo proceso que el indicado arriba
        FROM inc_desplazamiento=0 TO 11;
            FROM contador_columna=0 TO 5;
                FROM contador_filas=0 TO 5;
                    IF (tablero[contador_filas*6+contador_columna]!=0)
                        IF (tablero[contador_filas*6+contador_columna]!=3)
                            pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,0);
                        ELSE
                            pieza((x1*(11-inc_desplazamiento)+x2*inc_desplazamiento)/11,(y1*(11-inc_desplazamiento)+y2*inc_desplazamiento)/11,tipo_pieza XOR color,0);
                        END
                    END
                END
            END
            FRAME;
            signal(id,s_kill_tree);
            signal(id,s_wakeup);
        END
    END

    // Pone en la nueva posicion la pieza
    tablero[mover]=tipo_pieza;

    // Guarda el identificador para luego eliminar el proceso
    id_piezas=id;

    // Imprime el tablero como haya quedado
    FROM contador_filas=0 TO 5;
        FROM contador_columna=0 TO 5;
            IF (tablero[contador_filas*6+contador_columna]!=0)
                pieza(320+(contador_columna-contador_filas)*46,124+(contador_columna+contador_filas)*265/10,tablero[contador_filas*6+contador_columna] XOR color,0);
            END
        END
    END
    LOOP FRAME; END
END
