
//-------------------------------------------------------------------
//  TITULO: SOCCER (F£tbol 97')
//  AUTOR:  Antonio Marchal
//  FECHA:  10-08-97
//-------------------------------------------------------------------

PROGRAM soccer;

CONST

    pase=0;         // Distintos estados de los jugadores y porteros
    parado=1;
    normal=2;
    conbalon=3;
    atontado=4;
    desmarcado=5;
    esperandopase=6;
    irasaque=7;
    saquedebanda=8;
    porterotirandose=10;
    iracorner=11;
    tirandocorner=12;
    irarecibecorner=13;
    recibecorner=14;

    xicampo=176;            // Coordenadas x e y de la esquina superior izquierda
    yicampo=182;
    xfcampo=754;            // Coordenadas x e y de la esquina inferior derecha
    yfcampo=1149;

    campoagobio=4000;       // Distancia m xima a la que tiene que estar
                            // el contrario para tirar o pasar el bal¢n
    campoagobiojug=2000;    // Distancia m xima a la que tiene que estar
                            // el contrario para tirar o pasar el bal¢n para el jugador

    zonaquitada=5000;       // Zona donde empieza el jugador a tirarse para quitar la pelota

    zonarecogida=700;       // Zona de recogida de balones debe ser m¡nimo la mitad de velocidad m xima
    totaltestigo=5;         // Par metro de cada cuanto se pasa el testigo a cada jugador
    zonatiro1=20000;        // Zona para que tire sin estar agobiado.
    zonatiro2=35000;        // Zona de tiro estando agobiado
    vmaxima=300;            // Velocidad m xima de los jugadores en estado normal
    vmaximacg=200;          // Velocidad m xima con posesi¢n de bal¢n

    vmaxima=300;            // Velocidad m xima de los jugadores en estado normal
    vmaximacg=200;          // Velocidad m xima con posesi¢n de bal¢n

    vmaximajug=450;         // Velocidad m xima de los jugadores en estado normal
    vmaximacgjug=300;       // Velocidad m xima con posesi¢n de bal¢n


    azar=10;                // Par metro de desv¡o al azar
    retardoatontado=3;      // Contador de los pasos que un jugador esta atontado
    conbote=2500;           // Velocidad a partir de la cual el bal¢n se eleva
    distanciacamara=180;    // Distancia de la c mara al jugador
    alturacamara=224;       // Altura de la c mara
    cambiodesmarcado=100;   // Tiempo que pasa para que se eliga otro desmarcado
    incrvelocidadpelota=50; // Incremento/decremento de la velocidad de la pelota


GLOBAL

    idinicio;               // Identificador del programa principal
    elegidosaque=0;         // Bandera.-> Que equipo saca.
    conmarcadores=1;        // Bandera.-> 1.-Con marcadores 0.- Sin marcadores
    conintro=0;             // Hace que se ejecute o no la intro
    idnumerojugador=0;      // Identificador para poner el n£mero del jugador
    // Letras para poner los nombres de jugadores y equipo
    letras[]="Q","W","E","R","T","Y","U","I","O","P",
           "A","S","D","F","G","H","J","K","L","¥",
           "Z","X","C","V","B","N","M"," ",".";
    inicioparte=0;          // Controla el estado de las partes de partido 1¦ y 2¦ parte
    tipotiempo=0;           // Tipo de tiempo de juego 1->5:00 2->10:00 3->15:00 4->20:00
    totaltiempo=0;          // Tiempo total de cada parte del partido
    tiempojuego;            // Tiempo transcurrido en cada tiempo
    idtextomensaje;         // Identificador de texto central en juego
    controljugador=2;       // Control del jugador 0->joystick 1->teclado 2->ordenador
    editando=0;             // Bandera.-> 1 si se esta editando un nombre
    // Identificadores de los ficheros de gr ficos
    fichero1; fichero2; fichero3; fichero4; fichero5; fichero6; fichero7; fichero8; fichero9;
    // Identificadores de los ficheros de letras
    fuente1; fuente2; fuente3;
    // Identificadores de los ficheros de sonido
    idsonido0,idsonido1,idsonido2,idsonido3,idsonido4,idsonido5,idsonido6,idsonido7,idsonido8;
    idcanal,idcanal2;       // Identificador del canal de sonido
    colorini1[3]=3,6,1,0;   // Posici¢n inicial (dividida entre cinco) de los
    colorini2[3]=1,6,3,1;   // colores de las camisetas de los jugadores
    STRUCT nombres[23];     // Nombres de los jugadores con 10 letras
        letras[9];
    END
    STRUCT idnombres[23];   // Identificadores a las letras de los nombres
        idletras[9];
    END
    realopcionmenu1=-1;     // Opci¢n elegida en los men£s
    paljugador1[255];       // Paleta del jugador 1 (Ordenador)
    paljugador2[255];       // Paleta del jugador 2
    goles[1]=0,0;           // Goles marcados por cada equipo
    modovideo=0;            // Modo de v¡deo 1.-640x480 :: 2.-320x200
    quienmetegol=0;         // Quien mete gol  1.- equipo 2 :: 2.- equipo 1
    quienempieza=0;         // Quien saca 0.- equipo 1 :: 1.- equipo 2
    quiensacadebanda=0;     // Quien saca de banda 0.- equipo 1 :: 1.- equipo 2
    quiensacacorner=0;      // Quien saca de c¢rner 0.- equipo 1 || 1.- equipo 2
    porterosacando=0;       // Flags.-> El portero esta sacando
    cuenporterosacando=0;   // Contador de la posici¢n de portero sacando
    esquinacorner=0;        // Esquina del c¢rner 0->Superior-der. 1->Superior-Izq. 2->Inferior-der. 3->Inferior-Izq.
    sacando=0;              // Evita que se repita el saque de banda
    sacandoporteria=0;
    sacandocorner=0;        // Evita que se repita el saque de c¢rner
    estadosaque=0;          // Estado del saque de banda
    ultimotiro=0;           // Quien ha tocado la ultima vez el bal¢n
    incactual=0;            // Incremento en altura de la pelota
    incbote=0;              // Incremento en altura m ximo
    testigo1=0;             // Testigo equipo 1
    testigo2=0;             // Testigo equipo 2
    testigo3=0;             // Testigo de control de equipos
    pesopelota=3;           // Peso de la pelota
    estadoataque=50;        // Estado del ataque. 50->Centro del campo
    idjugadores1[9];        // Identificadores jugadores equipo 1
    idjugadores2[9];        // Identificadores jugadores equipo 2
    idporteros[1];          // Identificadores de los porteros
    idpelota;               // Identificador del bal¢n
    idcamara;
    estadoequipo=0;         // Estado equipo 0.-parado 1.-Normal
    estadojuego=0;          // 0.-Parado 1..3.- Procesos intermedios de saque
                            // 4.-Normal 5.- Inicio de saque despu‚s del gol
                            // 10.- Inicio saque banda  12.- Final saque banda
                            // 15.- Inicio saque puerta 16.- Final saque puerta
                            // 20.- Inicio c¢rner
    STRUCT tactica0[9]      // 5-3-2 No usada (para usar cambiar el nombre con la siguiente)
        xi0,yi0;            // Coordenadas iniciales con saque propio
        xi1,yi1;            // Coordenadas iniciales con saque contrario
        xp0,yp0;            // Posici¢n de defensa
        xp1,yp1;            // Posici¢n de ataque
        p0,p1;              // Peso de las posiciones de defensa y ataque
        xc0,yc0;            // Posiciones de ataque en un c¢rner
        xc1,yc1;            // Posiciones de defensa en un c¢rner
    END =
          70,130, 70,120, 70,130, 40,460,15, 5, 75,465,150, 50,
         140,200,140,190,140,200,150,390,15, 5,225,465,240, 70,
         290,220,290,210,290,220,290,350,15, 5,320,410,320,100,
         450,200,450,190,450,200,460,390,15, 5,440,465,400, 70,
         510,130,510,120,510,130,540,460,15, 5,560,465,490, 50,
         170,340,170,330, 40,420, 50,660,10,10,160,190,170,170,
         290,350,290,340,290,420,290,660,10,10,320,190,320,170,
         450,340,450,330,520,420,550,660,10,10,480,190,460,170,
         290,480,320,410,130,550,150,790, 5,15,240,100,210,420,
         320,480,350,410,450,550,440,790, 5,15,400,100,410,420;

    STRUCT t ctica[9]   // 4-3-3 (T ctica usada)
        xi0,yi0;        // Coordenadas iniciales con saque propio
        xi1,yi1;        // Coordenadas iniciales con saque contrario
        xp0,yp0;        // Posici¢n de defensa
        xp1,yp1;        // Posici¢n de ataque
        p0,p1;          // Peso de las posiciones de defensa y ataque
        xc0,yc0;        // Posiciones de ataque en un c¢rner
        xc1,yc1;        // Posiciones de defensa en un c¢rner
    END =
         510,130, 510,120, 530,210, 540,520, 15, 5, 75,465,150, 50,
         290,200, 290,190, 410,120, 420,340, 15, 5,225,465,240, 70,
         140,220, 140,210, 170,120, 120,330, 15, 5,320,410,320,100,
          70,200,  70,190,  80,210,  60,480, 15, 5,440,465,400, 70,
         450,130, 450,120, 470,320, 370,590, 10,10,560,465,490, 50,
         290,340, 290,330, 300,320, 160,590, 10,10,160,190,170,170,
         170,350, 170,340, 100,340,  90,700, 10,10,320,190,320,170,
         450,340, 450,330, 520,460, 460,770,  5,15,480,190,460,170,
         290,485, 320,410, 300,440, 290,820,  5,15,240,100,210,420,
         320,450, 350,410,  40,440, 130,800,  5,15,400,100,410,420;
    STRUCT equipototal[1]
        ataque;             // Fuerza de ataque del equipo
        defensa;            // Fuerza de defensa del equipo
        conbalon;           // 0 si no tiene el bal¢n o identificador del poseedor
        desmarcado;         // 0 si no hay desmarcado o identificador del desmarcado
        esperandopase;      // 0 si no espera pase o identificador del que espera el pase
        saquedebanda;       // 0 si no se saca de banda o identificador del que saca de banda
        sacacorner;         // 0 si no se saca c¢rner o identificador del que saca del c¢rner
        distanciaminima;    // Distancia al bal¢n del jugador que esta mas cerca de la pelota
    END

LOCAL
        xi,yi;              // Posiciones de inicio del jugador
        xp0,yp0;            // Posici¢n de defensa
        xp1,yp1;            // Posici¢n de ataque
        p0,p1;              // Peso de las posiciones de defensa y ataque
        estado;             // Estado del jugador
        equipo;             // Equipo al que pertenece
        pasa_a;             // N£mero del jugador al que se pasa
        tirando=0;          // Indicador del que el jugador ha realizado un tiro
        contador;           // Contador de uso general
        contador2;          // Contador de uso general
        cuentaatontado;     // Contador del estado atontado
        angulopelota=0;     // Angulo de la direcci¢n de la pelota
        velocidadpelota=0;  // Velocidad de la pelota
        velocidad;          // Velocidad de los jugadores
        n£mero;             // N£mero de jugador de 0 a 9
        anguloportero;      // Angulo de la direcci¢n del portero
        x_vieja;            // Coordenadas x e y antiguas del jugador
        y_vieja;
        tirandose=0;        // Indicador de que el jugador esta tir ndose
        cuentaanima;        // Contador de la animaci¢n de los jugadores

PRIVATE
    idfondo;                // Identificador del gr fico de fondo
    colorini[3]=3,6,1,0;    // Lugar donde comienza el color de cada parte del vestuario
    idtextosvarios[3];      // Identificadores a textos varios
    salirjuego;             // Bandera.-> 1.-Se sale del juego
    continuajuego=0;        // Hace que se continue el juego
    iniciomensaje=0;        // Sirve para tabular los textos

BEGIN
    max_process_time=2000;
    idinicio=id;            // Se coge el identificador del proceso

    // Se carga el sonido con la musica de introduccion y se ejecuta
    idsonido8=load_pcm("futbol\musica1.pcm",1);
    idcanal2=sound(idsonido8,100,256);
    scan_code=0;

    // Se cargan y se ejecutan las distintas animaciones iniciales
    start_fli("fli\futbol\futbol1.fli",0,0);
    WHILE (frame_fli()<>0 AND scan_code==0 AND NOT mouse.left) FRAME; END
    end_fli();
    IF (scan_code==0 AND NOT mouse.left)
        start_fli("fli\futbol\futbol2.fli",0,0);
        WHILE (frame_fli()<>0 AND scan_code==0 AND NOT mouse.left) FRAME; END
        end_fli();
    END
    IF (scan_code==0 AND NOT mouse.left)
        start_fli("fli\futbol\futbol3.fli",0,0);
        WHILE (frame_fli()<>0 AND scan_code==0 AND NOT mouse.left) FRAME; END
        end_fli();
    END
    WHILE (scan_code<>0 OR mouse.left) FRAME; END
    LOOP
        set_mode(m640x480); // Selecciona el modo de v¡deo para men£s

        // Carga los ficheros de sonido del juego
        idsonido0=load_pcm("futbol\silbato.pcm",0);
        idsonido1=load_pcm("futbol\aplauso.pcm",0);
        idsonido2=load_pcm("futbol\aplauso2.pcm",0);
        idsonido3=load_pcm("futbol\golpe.pcm",0);
        idsonido4=load_pcm("futbol\fondo.pcm",1);
        idsonido5=load_pcm("futbol\bote.pcm",0);
        idsonido6=load_pcm("futbol\barrida.pcm",0);
        idsonido7=load_pcm("futbol\saqueban.pcm",0);

        // Carga los ficheros de letras del men£
        fuente1=load_fnt("futbol\futbol0.fnt");
        fuente2=load_fnt("futbol\futbol1.fnt");
        fuente3=load_fnt("futbol\futbol3.fnt");

        // Carga los ficheros de gr ficos del men£
        fichero1=load_fpg("futbol\menus.fpg");
        fichero2=load_fpg("futbol\traje.fpg");
        fichero3=load_fpg("futbol\traje.fpg");
        fichero4=load_fpg("futbol\intro.fpg");

        // Carga los nombres de los jugadores
        load ("dat\futbol\nombres.dat",&nombres);

        // Carga los datos de los colores de los equipos
        load ("dat\futbol\traje0.dat",&colorini1);
        load ("dat\futbol\traje1.dat",&colorini2);

        // Prepara las variables donde se guardan las paletas
        FOR (contador=0;contador<256;contador++)
            paljugador1[contador]=contador;
            paljugador2[contador]=contador;
        END

        // Cambia los colores a su posici¢n correcta
        FOR (contador2=0;contador2<4;contador2++)
            FOR (contador=1;contador<6;contador++)
                paljugador1[(colorini[contador2]*5)+contador]=(colorini2[contador2]*5)+contador;
                paljugador2[(colorini[contador2]*5)+contador]=(colorini1[contador2]*5)+contador;
            END
        END

        // Convierte los gr ficos a la paleta correcta
        FOR (contador=1;contador<5;contador++)
            convert_palette(fichero2,contador,&paljugador2);
            convert_palette(fichero3,contador,&paljugador1);
        END
        mouse.x=320; mouse.y=240; mouse.left=0;
        IF (conintro==0)

            // Carga la pantalla de fondo de la intro con su paleta correcta
            idfondo=load_map("futbol\intro.map");
            load_pal("futbol\intro.map");

            // Prepara el gr fico del rat¢n
            mouse.file=fichero4; mouse.graph=908;

            // Pone la pantalla de fondo
            put_screen(0,idfondo);

            // Pone los t¡tulos de la pantalla inicial
            titulosoccer();
            titulofutbol97();
            fade_on();          // Enciende la pantalla

            // Espera hasta que se pulse una tecla
            WHILE (scan_code==0 AND NOT mouse.left AND NOT mouse.right)
                scan_code=0;
                FRAME;
            END

            // Si se ha pulsado la tecla escape o el bot¢n derecho
            IF (key(_esc) OR mouse.right)
                cr‚ditos();     // Pone los cr‚ditos y sale del programa
                FRAME;
            END
            fade_off();             // Apaga la pantalla

            // Elimina los t¡tulos de introducci¢n
            signal(TYPE titulosoccer,s_kill_tree);
            signal(TYPE titulofutbol97,s_kill_tree);
            signal(TYPE textopulsaunatecla,s_kill);
            delete_text(all_text);  // Borra todos los textos
            clear_screen();         // Borra la pantalla
            unload_map(idfondo);    // Descarga la pantalla de fondo
        END

        mouse.graph=909; mouse.file=fichero1; // Pone el gr fico del rat¢n
        // Carga la pantalla de fondo del men£ con su paleta correcta
        idfondo=load_map("futbol\menu.map");
        load_pal("futbol\menu.map");

        // Pone la pantalla de fondo
        put_screen(0,idfondo);

        // Pone todos los botones y textos de las opciones
        pintaopciones1();
        eligeopciones1();
        fade_on();              // Enciende la pantalla

        // Imprime todos los botones y textos hasta que se elija una opci¢n
        WHILE (realopcionmenu1==-1)
            FRAME;
        END

        // Borra pintaopciones1 y eligeopciones1
        signal(TYPE pintaopciones1,s_kill_tree);
        signal(TYPE eligeopciones1,s_kill_tree);
        conintro=0;
        fade_off();             // Apaga la pantalla

        // Descarga de memoria el gr fico de fondo
        unload_map(idfondo);

        // Borra la pantalla y cualquier texto
        clear_screen();
        delete_text(all_text);
        salirjuego=0;             // Reinicia la variable de salir del juego

        // Si se ha elegido cambiar los nombres
        IF (realopcionmenu1==1)

            // Graba los colores actuales de las camisetas para luego actualizarlos
            save ("dat\futbol\traje0.dat",&colorini1,sizeof(colorini1));
            save ("dat\futbol\traje1.dat",&colorini2,sizeof(colorini2));

            // Carga la pantalla de fondo con su paleta y la pone en pantalla
            idfondo=load_map("futbol\nombres.map");
            load_pal("futbol\nombres.map");
            put_screen(0,idfondo);
            realopcionmenu1=-1;
            fade_on();

            // Imprime todos los botones hasta que se elija una opci¢n
            WHILE (realopcionmenu1==-1)
                opciones2();
                FRAME;
            END

            // Graba los datos de los nombres de los jugadores
            save ("dat\futbol\nombres.dat",&nombres,sizeof(nombres));
            fade_off();

            // Si se hab¡a quedado editando nombres, elimina el proceso
            IF (editando==1)
                editando=0; signal (TYPE introduce_nombre,s_kill_tree);
            END
            realopcionmenu1=-1;
            conintro=1;// Hace que se salte la intro
            mouse.left=0;
            WHILE (mouse.left) FRAME; END
        END

        // Se ha elegido la opci¢n de jugar
        IF (realopcionmenu1==3)
            unload_pcm(idsonido8);

            // Reinicia los goles de cada equipo y el tiempo de cada parte
            conmarcadores=1;
            goles[0]=0; goles[1]=0;
            SWITCH (tipotiempo)
                CASE 0: totaltiempo=60;END
                CASE 1: totaltiempo=120;END
                CASE 2: totaltiempo=240;END
                CASE 3: totaltiempo=480;END
            END

            // Descarga los ficheros gr ficos y de letras que hubiera
            unload_fpg(fichero1);
            unload_fpg(fichero2);
            unload_fpg(fichero3);
            unload_fpg(fichero4);
            unload_fnt(fuente1);
            unload_fnt(fuente2);
            unload_fnt(fuente3);

            // Graba los colores de las camisetas
            save ("dat\futbol\traje0.dat",&colorini1,sizeof(colorini1));
            save ("dat\futbol\traje1.dat",&colorini2,sizeof(colorini2));

            // Borra todos los textos y procesos
            delete_text(all_text);
            let_me_alone();

            // Pone a cero las variables que controla las partes del juego
            salirjuego=0;
            inicioparte=0;

            // Repite hasta que se quiera salir o se haya acabado el partido
            WHILE (NOT key(_esc) AND inicioparte<>5 AND salirjuego==0)

                // Reinicia las variables de equipo
                equipototal[0].ataque=0;
                equipototal[0].defensa=0;
                equipototal[0].conbalon=0;
                equipototal[0].desmarcado=0;
                equipototal[0].esperandopase=0;
                equipototal[0].saquedebanda=0;
                equipototal[0].distanciaminima=0;
                equipototal[1].ataque=0;
                equipototal[1].defensa=0;
                equipototal[1].conbalon=0;
                equipototal[1].desmarcado=0;
                equipototal[1].esperandopase=0;
                equipototal[1].saquedebanda=0;
                equipototal[1].distanciaminima=0;
                quienmetegol=0;

                // Decide quien empieza dependiendo de la parte del juego
                IF (inicioparte==0)
                    quienempieza=0;
                ELSE
                    quienempieza=1;
                END

                // Reinicia las variables que controlan el juego
                quiensacadebanda=0;
                quiensacacorner=0;
                porterosacando=0;
                cuenporterosacando=0;
                esquinacorner=0;
                sacando=0;
                sacandoporteria=0;
                sacandocorner=0;
                estadosaque=0;
                ultimotiro=0;
                incactual=0;
                incbote=0;
                testigo1=0;
                testigo2=0;
                testigo3=0;
                pesopelota=3;
                estadoataque=50;
                estadoequipo=0;
                estadojuego=0;
                let_me_alone();     // Elimina otros procesos
                FOR (contador=0;contador<10;contador++)
                    IF (idjugadores1[contador])
                        idjugadores1[contador].estado=normal;
                    END
                    IF (idjugadores2[contador])
                        idjugadores2[contador].estado=normal;
                    END
                END

                // Selecciona el modo 640x480 para pantalla introductoria al partido
                set_mode(m640x480);

                // Carga los ficheros de los gr ficos de la pantalla introductoria
                fichero1=load_fpg("futbol\traje.fpg");
                fichero2=load_fpg("futbol\traje.fpg");
                fichero3=load_fpg("futbol\menus.fpg");

                // Carga los colores de los trajes
                load ("dat\futbol\traje0.dat",&colorini1);
                load ("dat\futbol\traje1.dat",&colorini2);

                // Prepara las variables donde se guardan las paletas
                FOR (contador=0;contador<256;contador++)
                    paljugador1[contador]=contador;
                    paljugador2[contador]=contador;
                END

                // Cambia los colores a su posici¢n correcta
                FOR (contador2=0;contador2<4;contador2++)
                    FOR (contador=1;contador<6;contador++)
                        paljugador1[(colorini[contador2]*5)+contador]=(colorini2[contador2]*5)+contador;
                        paljugador2[(colorini[contador2]*5)+contador]=(colorini1[contador2]*5)+contador;
                    END
                END
                FOR (contador=1;contador<5;contador++)
                    convert_palette(fichero1,contador,&paljugador2);
                    convert_palette(fichero2,contador,&paljugador1);
                END

                // Carga los ficheros de letras
                fuente2=load_fnt("futbol\futbol3.fnt");
                fuente3=load_fnt("futbol\futbol1.fnt");

                // Carga la pantalla de fondo y la pone en pantalla
                idfondo=load_map("futbol\presenta.map");
                put_screen(0,idfondo);

                // Pone el mensaje de 1¦ o 2¦ parte
                IF (inicioparte==0)
                    write(fuente2,320,10,1,"- 1er TIEMPO -");
                ELSE
                    write(fuente2,320,10,1,"- 2do TIEMPO -");
                END

                // Pone el mensaje de tiempo elegido
                SWITCH (tipotiempo)
                    CASE 0: write(fuente2,320,430,4,"TIEMPO 2:00"); END
                    CASE 1: write(fuente2,320,430,4,"TIEMPO 4:00"); END
                    CASE 2: write(fuente2,320,430,4,"TIEMPO 8:00"); END
                    CASE 3: write(fuente2,320,430,4,"TIEMPO 16:00"); END
                END

                // Pone los nombres de los equipos
                iniciomensaje=50;
                FROM contador=9 TO 0;
                    IF (nombres[0].letras[contador]==letras[27])
                        iniciomensaje+=9;
                    ELSE
                        BREAK;
                    END
                END
                escribe_nombre(iniciomensaje,100,0,fuente3,18);
                iniciomensaje=410;
                FROM contador=9 TO 0;
                    IF (nombres[12].letras[contador]==letras[27])
                        iniciomensaje+=9;
                    ELSE
                        BREAK;
                    END
                END
                escribe_nombre(iniciomensaje,100,12,fuente3,18);

                // Y los goles marcados por cada uno
                write_int(fuente2,140,340,1,&goles[0]);
                write(fuente2,320,340,1,"-");
                write_int(fuente2,500,340,1,&goles[1]);

                // Pone los trajes de con los colores elegidos
                traje2(141,210,1,fichero1);
                traje2(140,210,2,fichero1);
                traje2(140,270,3,fichero1);
                traje2(140,320,4,fichero1);
                traje2(501,210,1,fichero2);
                traje2(500,210,2,fichero2);
                traje2(500,270,3,fichero2);
                traje2(500,320,4,fichero2);

                // Pone gr fico al rat¢n
                mouse.graph=909;
                mouse.file=fichero3;

                // Espera a que no este pulsado nada
                WHILE(key(_space)) FRAME;  END
                WHILE(mouse.left) FRAME;  END

                // Reinicia variables para pantalla presentacion
                idtextosvarios[3]=0;
                continuajuego=0;

                // Repite hasta que se pulse espacio o el bot¢n de pantalla
                WHILE (NOT key(_space) AND continuajuego==0)

                    // Pone el texto si se posiciona el cursor del rat¢n encima
                    IF (comprueba_raton(550,395,615,455))
                        idtextosvarios[3]=write(0,580,390,4,"CONTINUA");
                    END

                    // Pone el bot¢n de la derecha (continue), con el gr fico correcto
                    bot¢n(580,422,0,comprueba_raton(550,395,615,455),fichero3);

                    // Mira si se ha pulsado el rat¢n y la opci¢n de empezar
                    IF (comprueba_raton(550,395,615,455)==3)
                        continuajuego=1;
                    END

                    FRAME;
                    IF (idtextosvarios[3]<>0)
                        delete_text(idtextosvarios[3]);
                        idtextosvarios[3]=0;
                    END
                END

                // Apaga la pantalla
                fade_off();

                // Quita el gr fico del rat¢n y dem s gr ficos
                mouse.graph=0;
                signal(TYPE traje2,s_kill);

                // Descarga los ficheros de gr ficos de memoria
                unload_fpg(fichero1);
                unload_fpg(fichero2);
                unload_fpg(fichero3);

                // Borra la pantalla y los textos que hubiera
                clear_screen();
                delete_text(all_text);
                fade_on();

                // Pone el modo de pantalla y los frames por segundo
                // dependiendo de modo de v¡deo elegido
                IF (modovideo==0)
                    set_fps(16,2);
                    set_mode(m640x480);
                ELSE
                    set_fps(24,4);
                    set_mode(m320x240);
                END

                // Carga los ficheros de los gr ficos necesarios
                fichero1=load_fpg("futbol\futbol.fpg");
                fichero2=load_fpg("futbol\jugador0.fpg");
                fichero3=load_fpg("futbol\jugador0.fpg");
                fichero4=load_fpg("futbol\portero0.fpg");
                fichero5=load_fpg("futbol\portero1.fpg");
                fichero6=load_fpg("futbol\balon.fpg");
                fichero7=load_fpg("futbol\sombras2.fpg");
                fichero8=load_fpg("futbol\sombrap0.fpg");
                fichero9=load_fpg("futbol\sombrap1.fpg");

                // Carga los ficheros de las letras
                fuente1=load_fnt("futbol\futbol3.fnt");
                fuente2=load_fnt("futbol\futbol1.fnt");
                fuente3=load_fnt("futbol\futbol2.fnt");

                // Prepara las variables donde se guardan las paletas
                FOR (contador=0;contador<256;contador++)
                    paljugador1[contador]=contador;
                    paljugador2[contador]=contador;
                END

                // Cambia los colores a su posici¢n correcta
                FOR (contador2=0;contador2<4;contador2++)
                    FOR (contador=1;contador<6;contador++)
                        paljugador1[(colorini[contador2]*5)+contador]=(colorini2[contador2]*5)+contador;
                        paljugador2[(colorini[contador2]*5)+contador]=(colorini1[contador2]*5)+contador;
                    END
                END

                // Convierta los gr ficos a la paleta elegida
                FOR (contador=1;contador<97; contador++)
                    convert_palette(fichero3,contador,&paljugador2);
                    convert_palette(fichero2,contador,&paljugador1);
                END

                // Pone el modo 7
                start_mode7(0,fichero1,100,0,0,0);

                // Pone los par metros del modo 7
                m7.height=alturacamara;     // Altura de la c mara
                m7.focus=512;               // Foco de la c mara
                m7.color=158;               // Color de fuera de la pantalla

                valla();                    // Pone el publico del fondo
                bander¡n (xicampo,yicampo); // Pone los banderines de los c¢rners
                bander¡n (xicampo,yfcampo);
                bander¡n (xfcampo,yicampo);
                bander¡n (xfcampo,yfcampo);
                tu_porteria();              // Pone las porter¡as
                su_porteria();
                idpelota=balon();           // Pone el bal¢n

                // Pone los porteros
                idporteros[0]=porteros(0,(yfcampo-50)*100,fichero4);
                idporteros[1]=porteros(1,(yicampo+50)*100,fichero5);

                // Pone la c mara
                idcamara=camara();
                m7.camera=idcamara;

                // Pone los marcadores de defensa y ataque
                equipototal[0].defensa=50;
                equipototal[0].ataque=50;
                equipototal[1].defensa=50;
                equipototal[1].ataque=50;

                // Pone los jugadores de los dos equipos
                FOR (contador=0;contador<10;contador++)
                    idjugadores1[contador]=ponjugador(contador,0);
                    idjugadores2[contador]=ponjugador(contador,1);
                END

                // Inicia el proceso que controla al equipo
                controlaequipo(0);
                controlaequipo(1);

                // Inicia los procesos que marcan a los jugadores desmarcados
                flecha(0);
                flecha(1);

                // Inicia el proceso que controla el juego
                controlajuego ();

                // Calcula el tiempo para imprimirlo
                tiempojuego=45-(((totaltiempo-(timer[0]/100))*45)/totaltiempo);

                // Pone los textos de los marcadores dependiendo del modo gr fico
                // y de si se quieren o no marcadores
                IF (conmarcadores==1)
                    IF (modovideo==0)
                        minicampo();
                        idtextosvarios[0]=write_int(fuente1,44,480,6,&goles[0]);
                        idtextosvarios[1]=write_int(fuente1,600,480,8,&goles[1]);
                        idtextosvarios[2]=write_int(fuente1,320,0,0,&tiempojuego);
                        ponminicamisas(20,456,65,fichero3,50);
                        ponminicamisas(620,456,65,fichero2,50);
                    ELSE
                        minicampo();
                        idtextosvarios[0]=write_int(fuente3,22,240,6,&goles[0]);
                        idtextosvarios[1]=write_int(fuente3,300,240,8,&goles[1]);
                        idtextosvarios[2]=write_int(fuente3,160,0,0,&tiempojuego);
                        ponminicamisas(10,228,65,fichero3,25);
                        ponminicamisas(310,228,65,fichero2,25);
                    END
                END

                // Hace que no se restaure la pantalla para ganar tiempo
                restore_type=no_restore;
                fade_on();

                // Realiza el sonido del publico cuando se inicia el partido
                sound(idsonido2,50,256);

                // Realiza el sonido que estar  durante todo el partido
                idcanal=sound(idsonido4,50,256);

                // Bucle del partido
                WHILE (NOT key(_esc) AND inicioparte<>2 AND inicioparte<>5)

                    salirjuego=0;     // Mientras se este jugando valdr  0

                    // Hasta que empiece el partido realmente pone el tiempo a 0
                    IF (inicioparte==0 OR inicioparte==3) timer[0]=0; END

                    // Calcula el tiempo de juego para luego imprimirlo
                    tiempojuego=45-(((totaltiempo-(timer[0]/100))*45)/totaltiempo);

                    // No permite que el tiempo pase de 45
                    IF (tiempojuego>45) tiempojuego=45; END

                    // Comprueba si se ha acabado el partido y esta en estado normal
                    IF (tiempojuego==45 AND estadojuego==4 AND
                        idpelota.velocidadpelota==0 AND
                        equipototal[0].conbalon==0 AND
                        equipototal[1].conbalon==0 AND incbote==0)

                        // Si se ha acabado pone el mensaje apropiado
                        IF (inicioparte==1)
                            inicioparte=2;
                            ponmensajes (6,20);
                        ELSE
                            inicioparte=5;
                            ponmensajes (7,20);
                        END

                        // Deja los dem s proceso parados
                        signal(id,s_freeze_tree);
                        signal(id,s_wakeup);
                        contador=0;

                        // Espera hasta que se pulse espacio o pase un tiempo
                        WHILE (NOT key (_space) AND contador<50) FRAME; contador++; END

                        // Vuelve a ejecutar los dem s procesos
                        signal(id,s_wakeup_tree);
                    END

                    // Borra los textos para despu‚s actualizarlos
                    delete_text(idtextosvarios[0]);
                    delete_text(idtextosvarios[1]);
                    delete_text(idtextosvarios[2]);

                    // Pone los textos de los marcadores seg£n el modo de v¡deo
                    // y de si se quieren o no marcadores
                    IF (conmarcadores==1)
                        IF (modovideo==0)
                            idtextosvarios[0]=write_int(fuente1,44,480,6,&goles[0]);
                            idtextosvarios[1]=write_int(fuente1,600,480,8,&goles[1]);
                            idtextosvarios[2]=write_int(fuente1,320,0,0,&tiempojuego);
                        ELSE
                            idtextosvarios[0]=write_int(fuente3,22,240,6,&goles[0]);
                            idtextosvarios[1]=write_int(fuente3,300,240,8,&goles[1]);
                            idtextosvarios[2]=write_int(fuente3,160,0,0,&tiempojuego);
                        END
                    END

                    // Actualiza el estado de ataque del juego
                    estadoataque=(idpelota.y-((yicampo*100)+20))/(yfcampo-(yicampo+20));
                    IF (estadoataque<0) estadoataque=0; END
                    IF (estadoataque>100) estadoataque=100; END

                    // Cambia el sonido dependiendo de la posici¢n del equipo en el campo
                    change_sound(idcanal,(30*(100-estadoataque))/100,rand(230,280));

                    FRAME;
                    IF (key(_m))
                        IF (conmarcadores==1)
                            conmarcadores=0;
                        ELSE
                            conmarcadores=1;
                        END
                        WHILE (key(_m)) FRAME; END
                    END

                    // Actualiza los testigos que dan el control a los jugadores y equipos
                    testigo1++;IF (testigo1>(totaltestigo-1)) testigo1=0; END
                    testigo2++;IF (testigo2>(totaltestigo-1)) testigo2=0; END
                    testigo3++;IF (testigo3>1) testigo3=0; END
                END

                // Comprueba si se ha pulsado escape para salir del juego
                IF (key(_esc)) salirjuego=1; END

                // Actualiza la variable que controla las partes del juego
                IF (inicioparte==2)inicioparte=3;END
                fade_off();

                // Mira si se ha acabado el partido
                IF (inicioparte==5)
                    // Elimina a los dem s procesos
                    let_me_alone();

                    // Quita el sonido de ambiente
                    change_sound(idcanal,0,256);

                    // Quita el modo 7
                    stop_mode7(0);

                    // Hace que se vuelva a restaurar la pantalla
                    restore_type=complete_restore;

                    // Selecciona el modo de v¡deo para pantalla de resultados
                    set_mode(m640x480);

                    // Descarga los ficheros de gr ficos de memoria
                    unload_fpg(fichero1);
                    unload_fpg(fichero2);
                    unload_fpg(fichero3);

                    // Descarga los ficheros de letras de pantalla
                    unload_fnt(fuente1);
                    unload_fnt(fuente2);

                    // Carga los gr ficos necesarios para la pantalla de resultados
                    fichero1=load_fpg("futbol\traje.fpg");
                    fichero2=load_fpg("futbol\traje.fpg");
                    fichero3=load_fpg("futbol\menus.fpg");

                    // Carga los ficheros de letras
                    fuente1=load_fnt("futbol\futbol3.fnt");
                    fuente2=load_fnt("futbol\futbol1.fnt");

                    // Prepara las variables donde se guardan las paletas
                    FOR (contador=0;contador<256;contador++)
                        paljugador1[contador]=contador;
                        paljugador2[contador]=contador;
                    END

                    // Cambia los colores a su posici¢n correcta
                    FOR (contador2=0;contador2<4;contador2++)
                        FOR (contador=1;contador<6;contador++)
                            paljugador1[(colorini[contador2]*5)+contador]=(colorini2[contador2]*5)+contador;
                            paljugador2[(colorini[contador2]*5)+contador]=(colorini1[contador2]*5)+contador;
                        END
                    END

                    // Convierte los trajes a los colores elegidos
                    FOR (contador=1;contador<5;contador++)
                        convert_palette(fichero1,contador,&paljugador2);
                        convert_palette(fichero2,contador,&paljugador1);
                    END

                    // Borra los textos que hubiera
                    delete_text(all_text);

                    // Descarga la pantalla de fondo si la hubiera
                    unload_map(idfondo);

                    // Carga la pantalla de fondo y la pone en pantalla
                    idfondo=load_map("futbol\presenta.map");
                    put_screen(0,idfondo);

                    // Pone los nombres de los equipos
                    iniciomensaje=50;
                    FROM contador=9 TO 0;
                        IF (nombres[0].letras[contador]==letras[27])
                            iniciomensaje+=9;
                        ELSE
                            BREAK;
                        END
                    END
                    escribe_nombre(iniciomensaje,100,0,fuente3,18);
                    iniciomensaje=410;
                    FROM contador=9 TO 0;
                        IF (nombres[12].letras[contador]==letras[27])
                            iniciomensaje+=9;
                        ELSE
                            BREAK;
                        END
                    END
                    escribe_nombre(iniciomensaje,100,12,fuente3,18);

                    // Pone los trajes elegidos por los equipos
                    traje2(141,210,1,fichero1);
                    traje2(140,210,2,fichero1);
                    traje2(140,270,3,fichero1);
                    traje2(140,320,4,fichero1);
                    traje2(501,210,1,fichero2);
                    traje2(500,210,2,fichero2);
                    traje2(500,270,3,fichero2);
                    traje2(500,320,4,fichero2);

                    // Pone el mensaje de final de partido
                    write(fuente1,320,10,1,"FINAL PARTIDO");

                    // Pone los textos de los goles marcados por cada equipo
                    write_int(fuente1,140,340,1,&goles[0]);
                    write(fuente1,320,340,1,"-");
                    write_int(fuente1,500,340,1,&goles[1]);

                    // Selecciona gr fico para el rat¢n
                    mouse.graph=909;mouse.file=fichero3;
                    fade_on();

                    // Comprueba que no este nada pulsado
                    mouse.left=0; WHILE(key(_space)) FRAME;  END

                    // Espera hasta que se pulse el espacio o el rat¢n
                    WHILE (NOT key(_space) AND NOT mouse.left)
                        FRAME;
                    END
                    fade_off();
                    unload_map(idfondo);
                END

                // Elige que se vuelva a restaurar la pantalla
                restore_type=complete_restore;

                // Borra los textos que hubiera
                delete_text(all_text);

                // Descarga los ficheros de gr ficos de memoria
                unload_fpg(fichero1);
                unload_fpg(fichero2);
                unload_fpg(fichero3);
                unload_fpg(fichero4);
                unload_fpg(fichero5);
                unload_fpg(fichero6);
                unload_fpg(fichero7);
                unload_fpg(fichero8);
                unload_fpg(fichero9);

                // Descarga los ficheros de letras de memoria
                unload_fnt(fuente1);
                unload_fnt(fuente2);
                unload_fnt(fuente3);

                // Quita mode 7
                stop_mode7(0);
                fade_on();

                // Quita el sonido de ambiente (pone el volumen a 0)
                change_sound(idcanal,0,256);
            END
            idsonido8=load_pcm("futbol\musica1.pcm",1);
            idcanal2=sound(idsonido8,100,256);
            conintro=0;     // Hace que se ejecute
        END

        // Quita los procesos que hubiera abiertos y borra los textos
        let_me_alone();
        delete_text(all_text);
        realopcionmenu1=-1;
        fade_on();
    END
END

//-------------------------------------------------------------------
// Proceso pintaopciones1
// Pone los texto y gr ficos de men£ de opciones principal
//-------------------------------------------------------------------
PROCESS pintaopciones1()
BEGIN

    // Pone los trajes de los jugadores para elegir el color de los mismos
    traje(121,210,1,fichero2);
    traje(120,210,2,fichero2);
    traje(120,270,3,fichero2);
    traje(120,320,4,fichero2);
    traje(521,210,1,fichero3);
    traje(520,210,2,fichero3);
    traje(520,270,3,fichero3);
    traje(520,320,4,fichero3);

    LOOP
        // Borra los texto que hubiera para despu‚s actualizarlos
        delete_text(all_text);

        // Pone los nombres de los equipos
        escribe_nombre(90,20,0,fuente1,16);
        escribe_nombre(390,20,12,fuente1,16);

        // Comprueba si el rat¢n esta encima del mensaje y pone el apropiado
        IF (comprueba_raton(170,420,415,455))
            write(fuente2,325,430,4,"CAMBIA NOMBRES EQUIPO");
        ELSE
            write(fuente1,325,430,4,"CAMBIA NOMBRES EQUIPO");
        END

        // Comprueba si el rat¢n esta encima del mensaje y pone el texto apropiado
        IF (comprueba_raton(170,360,415,400))
            IF (modovideo==0)
                write(fuente2,320,380,4,"ALTA RESOLUCION");
            ELSE
                write(fuente2,320,380,4,"BAJA RESOLUCION");
            END
        ELSE
            IF (modovideo==0)
                write(fuente1,320,380,4,"ALTA RESOLUCION");
            ELSE
                write(fuente1,320,380,4,"BAJA RESOLUCION");
            END
        END

        // Pone el bot¢n de la izquierda (salir), con el gr fico correcto
        bot¢n(52,422,1,comprueba_raton(20,395,85,455),fichero1);

        // Pone el texto si se posiciona el cursor del rat¢n delante
        IF (comprueba_raton(20,395,85,455))
            write(0,50,390,4,"VOLVER");
        END

        // Pone el bot¢n de la derecha (jugar), con el gr fico correcto
        bot¢n(580,422,0,comprueba_raton(550,395,615,455),fichero1);

        // Pone el texto si se posiciona el cursor del rat¢n encima
        IF (comprueba_raton(550,395,615,455))
            write(0,580,390,4,"EMPEZAR");
        END

        // Pone el texto del tiempo elegido
        pontiempo(319,197,tipotiempo+1);

        // Pone el tipo de control elegido
        poncontrol(40,40,20+(controljugador*10));
        poncontrol(600,40,40);
        FRAME;
    END
END
//-------------------------------------------------------------------
// Proceso eligeopciones1
// Comprueba si se ha cambiado alguna opci¢n
//-------------------------------------------------------------------

PROCESS eligeopciones1()

PRIVATE
    idcollisionmouse=0;

BEGIN
    LOOP
        // Comprueba si se ha pulsado el rat¢n y pone la variable de la opci¢n elegida
        IF (comprueba_raton(170,420,415,455)==3)
            realopcionmenu1=1;
        END

        // Mira si se ha pulsado el rat¢n y cambia el modo de v¡deo
        IF (mouse.x>170 AND mouse.x<415 AND mouse.y>360 AND mouse.y<400 AND mouse.left)
            IF (modovideo==0)
                modovideo=1;
            ELSE
                modovideo=0;
            END
            WHILE (mouse.left) FRAME; END
        END

        // Comprueba si se ha pulsado el rat¢n y elige la opci¢n de salir
        IF (comprueba_raton(20,395,85,455)==3 OR key (_esc))
            realopcionmenu1=2;
        END

        // Mira si se ha pulsado el rat¢n y la opci¢n de empezar
        IF (comprueba_raton(550,395,615,455)==3)
            realopcionmenu1=3;
        END

        // Mira si se ha pulsado el rat¢n y cambia el tiempo elegido
        IF (mouse.x>249 AND mouse.x<391 AND mouse.y>159 AND mouse.y<301 AND mouse.left)
            tipotiempo++;IF (tipotiempo>3) tipotiempo=0; END
            WHILE (mouse.left) FRAME; END
        END

        // Mira si se ha pulsado el rat¢n y cambia el modo de control
        IF (mouse.x>0 AND mouse.x<291 AND mouse.y>0 AND mouse.y<91 AND mouse.left)
            controljugador++;IF (controljugador>2) controljugador=0; END
            WHILE (mouse.left) FRAME; END
        END
        FRAME;
    END

END

//-------------------------------------------------------------------
// Proceso opciones2
// Pone los textos y gr ficos de la pantalla de seleccionar nombres
//-------------------------------------------------------------------

PROCESS opciones2 ()

BEGIN
    // Borra los textos para despu‚s actualizarlos
    delete_text(all_text);

    // Escribe los nombres de los equipos
    escribe_nombre(78,17,0,0,10);
    escribe_nombre(458,17,12,0,10);

    // Escribe los nombres de los porteros
    escribe_nombre(78,80,1,0,10);
    escribe_nombre(458,80,13,0,10);

    // Escribe los nombres de los jugadores
    FOR (contador=2;contador<12;contador++)
        escribe_nombre(78,72+(21*contador),contador,0,10);
        escribe_nombre(458,72+(21*contador),contador+12,0,10);
    END

    // Pone el bot¢n de la izquierda inferior con el gr fico apropiado
    bot¢n(52,422,1,comprueba_raton(20,400,80,450),fichero1);

    // Pone el texto si pasas el cursor del rat¢n por encima
    IF (comprueba_raton(20,400,80,450))
        write(0,50,390,4,"VOLVER");
    END

    // Mira si se ha pulsado el rat¢n y sale de la pantalla
    IF (comprueba_raton(20,400,80,450)==3)
        realopcionmenu1=2;
    END

    // Mira si se ha pulsado el rat¢n encima del nombre de los equipos
    IF (comprueba_raton(0,12,247,29)==3)
        if (editando==0)
            introduce_nombre(0);
        ELSE
            signal (TYPE introduce_nombre,s_kill);
            introduce_nombre(0);
        END
    END
    IF (comprueba_raton(392,13,639,30)==3)
        IF (editando==0)
            introduce_nombre(12);
        ELSE
            signal (TYPE introduce_nombre,s_kill);
            introduce_nombre(12);
        END
    END

    // Mira si se ha pulsado el rat¢n encima del nombre de los porteros
    IF (comprueba_raton(1,73,264,92)==3)
        IF (editando==0)
            introduce_nombre(1);
        ELSE
            signal (TYPE introduce_nombre,s_kill);
            introduce_nombre(1);
        END
    END
    IF (comprueba_raton(380,73,639,92)==3)
        IF (editando==0)
            introduce_nombre(13);
        ELSE
            signal (TYPE introduce_nombre,s_kill);
            introduce_nombre(13);
        END
    END

    // Mira si se ha pulsado el rat¢n encima del nombre de los jugadores
    FOR (contador=2;contador<12;contador++)
        IF (comprueba_raton(1,66+(contador*21),263,68+(contador*21)+19)==3)
            IF (editando==0)
                introduce_nombre(contador);
            ELSE
                signal (TYPE introduce_nombre,s_kill);
                introduce_nombre(contador);
            END
        END
        IF (comprueba_raton(381,66+(contador*21),639,68+(contador*21)+19)==3)
            IF (editando==0)
                introduce_nombre(contador+12);
            ELSE
                signal (TYPE introduce_nombre,s_kill);
                introduce_nombre(contador+12);
            END
        END
    END

    // Pone el gr fico de las letras del teclado
    ponletras (358,416,-10,6);
    FRAME;
END

//-------------------------------------------------------------------
// Proceso ponletras
// Pone el gr fico de las letras del teclado en la pantalla de
// cambiar nombres
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS ponletras (x,y,z,graph)

BEGIN
    FRAME;
END

//-------------------------------------------------------------------
// Proceso pontiempo
// Pone el gr fico de selecci¢n de tiempo en la pantalla de opciones
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS pontiempo (x,y,graph)

BEGIN
    FRAME;
END

//-------------------------------------------------------------------
// Proceso bot¢n
// Pone el gr fico de los botones de las pantallas de opciones
// Entradas: Variables predefinidas y...
//           cual   = Elige hacia donde mira
//           estado = Elige si esta iluminado o no
//-------------------------------------------------------------------

PROCESS bot¢n(x,y,cual,estado,file)

BEGIN
    // Pone el gr fico mirando a la derecha o a la izquierda
    IF (cual==0)
        flags=1;
    ELSE
        flags=0;
    END

    // Pone el gr fico dependiendo si esta o no el cursor del rat¢n encima
    IF (estado==0)
        graph=51;
    ELSE
        graph=50;
    END
    FRAME;
END

//-------------------------------------------------------------------
// Proceso comprueba_raton
// Mira si se ha pulsado el rat¢n o si esta en una zona concreta
// Entradas: x1,y1 = Coordenadas superior izquierda de la zona a mirar
//           x2,y2 = Coordenadas inferior derecha de la zona a mirar
// Salidas:  0 = El rat¢n no esta encima de la zona ni tiene el bot¢n
//               pulsado
//           1 = El rat¢n esta encima de la zona
//           3 = El rat¢n esta encima de la zona y tiene el bot¢n
//               pulsado
//-------------------------------------------------------------------

PROCESS comprueba_raton(x1,y1,x2,y2)

BEGIN

    // Mira si esta en la zona correcta
    IF (mouse.x>x1 AND mouse.x<x2 AND
        mouse.y>y1 AND mouse.y<y2)

        // Mira si se ha pulsado el rat¢n
        IF (mouse.left==TRUE)
            RETURN (3);         // Devuelve 3 si se ha pulsado el rat¢n
        ELSE
            RETURN (1);         // Devuelve 1 si esta en la zona correcta
        END
    END
    RETURN(0);                  // Devuelve 0 si no se pulsa el rat¢n y
                                // no esta en la zona correcta
END

//-------------------------------------------------------------------
// Proceso escribe_nombre
// Escribe los nombres que est n dentro del equipo que est n en
// estructura
// Entradas: Variables predefinidas y...
//           numeror     = N£mero de nombre
//           tipoletras  = Tipo de letras a usar
//           ancholetras = Separaci¢n entre las letras
//-------------------------------------------------------------------

PROCESS escribe_nombre(x,y,numeror,tipoletras,ancholetras)

BEGIN

    // Va recorriendo las 10 letras del nombre
    FOR (contador=0;contador<10;contador++)
        // Y las va imprimiendo qued ndose con su identificador
        idnombres[numeror].idletras[contador]=write(tipoletras,x+(contador*ancholetras),y,0,nombres[numeror].letras[contador]);
    END

    FRAME;
END

//-------------------------------------------------------------------
// Proceso titulofutbol97
// Pone y mueve el titulo de la intro que pone FUTBOL 97'
//-------------------------------------------------------------------

PROCESS titulofutbol97()

PRIVATE
    solouno=0;

BEGIN

    // Elige las coordenadas iniciales y el gr fico
    y=414;
    x=-140;
    graph=48;
    file=fichero4;
    LOOP

        // Mueve el gr fico hasta su posici¢n final
        IF (x<155)
            x+=15;
        ELSE
            // Si ha llegado al final pone el bal¢n
            IF (solouno==0)
                textopulsaunatecla();
                solouno=1;      // Se usa esto para sacar solo un bal¢n
            END
        END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso titulosoccer
// Pone el gr fico de titulo Soccer de la pantalla introductoria
//-------------------------------------------------------------------

PROCESS titulosoccer()

BEGIN
    // Elige las coordenadas iniciales y el gr fico
    y=356;
    x=-140;
    graph=10;
    file=fichero4;

    // Pone la sombra de detraes de soccer
    sombrasoccer();
    LOOP
        // Lo mueve hasta su posici¢n final
        IF (x<160) x+=30; END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso sombrasoccer
// Pone la sombra de detraes de soccer
//-------------------------------------------------------------------

PROCESS sombrasoccer()

BEGIN
    // Elige el gr fico y las coordenadas iniciales
    graph=11;
    x=father.x+5;
    y=father.y+5;
    z=father.z+1;

    // Lo hace transparente
    flags=4;
    file=fichero4;
    LOOP
        // Actualiza las coordenadas dependiendo de donde este el gr fico que lo llama
        x=father.x+5;
        y=father.y+5;
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso traje
// Pone los gr ficos de selecci¢n de color del equipo y comprueba si
// se ha cambiado por el jugador
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS traje(x,y,graph,file)

PRIVATE
    oldcolorini;

BEGIN
    LOOP
        // Comprueba si se ha pulsado el rat¢n encima del gr fico
        // para cambiar el color
        IF ((collision(TYPE mouse)) AND mouse.left)

            // Mira cual de los trajes hay que modificar y lo prepara
            IF (file==fichero2)
                oldcolorini=colorini1[graph-1];
                colorini1[graph-1]++;
                IF (colorini1[graph-1]>9) colorini1[graph-1]=0; END
            ELSE
                oldcolorini=colorini2[graph-1];
                colorini2[graph-1]++;
                IF (colorini2[graph-1]>9) colorini2[graph-1]=0; END
            END

            // Prepara la paleta para cambiar el color
            FOR (contador=1;contador<6;contador++)
                IF (file==fichero2)
                    paljugador2[(oldcolorini*5)+contador]=(colorini1[graph-1]*5)+contador;
                ELSE
                    paljugador1[(oldcolorini*5)+contador]=(colorini2[graph-1]*5)+contador;
                END
            END

            // Convierte la paleta a los colores apropiados
            IF (file==fichero2)
                convert_palette(file,graph,&paljugador2);
            ELSE
                convert_palette(file,graph,&paljugador1);
            END
            WHILE (mouse.left) FRAME; END
        END
    FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso poncontrol
// Pone el gr fico de los controles en la pantalla de selecci¢n
//-------------------------------------------------------------------

PROCESS poncontrol(x,y,graph)

BEGIN
    FRAME;
END

//-------------------------------------------------------------------
// Proceso textopulsaunatecla
// Pone el gr fico 'press any key' de la pantalla introductoria
//-------------------------------------------------------------------

PROCESS textopulsaunatecla()

PRIVATE
    pausatexto=0;       // Contador para que el texto parpadee

BEGIN
    // Elige las coordenadas iniciales y el gr fico
    x=452;
    y=414;
    graph=49;
    file=fichero4;
    LOOP

        // Va conmutando el contador de 0 9
        pausatexto++;IF (pausatexto>9) pausatexto=0; END

        // De cada 10 veces, 7 veces pone el gr fico en pantalla
        IF (pausatexto<7) graph=49; ELSE graph=0; END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso introduce_nombre
// Coge los nombres que edita el jugador
// Entradas: cualnombre = N£mero del nombre a editar
//-------------------------------------------------------------------

PROCESS introduce_nombre(cualnombre)

PRIVATE

    posicicursor=10;        // Posici¢n del cursor
    inixcursor,iniycursor;  // Coordenadas x,y iniciales del cursor
    parpadeocursor=0;       // Controla el parpadeo del cursor
    tecladeraton;           // Valor de la tecla cuando se usa el rat¢n

    // Valores de las teclas del teclado para luego convertirlas
    tablascan[29]=16,17,18,19,20, 21,22,23,24,25,
                  30,31,32,33,34, 35,36,37,38,39,
                  44,45,46,47,48, 49,50,57,52,14;

    contadorespacios;        // Cuenta los espacios de la derecha
BEGIN

    editando=1; // Pone la variable a 1 porque se esta editando
    // Pone el cursor al final del nombre
    contadorespacios=9;
    WHILE (contadorespacios>-1)
        IF (nombres[cualnombre].letras[contadorespacios]==letras[27])
            contadorespacios--;
        ELSE
            BREAK;
        END
    END
    posicicursor=contadorespacios+1;

    // Selecciona las coordenadas iniciales del cursor
    SWITCH (cualnombre)
        CASE 0: inixcursor=80; iniycursor=20; END
        CASE 1: inixcursor=80; iniycursor=83; END
        CASE 2..11:inixcursor=80; iniycursor=75+(cualnombre*21); END
        CASE 12: inixcursor=460; iniycursor=20; END
        CASE 13: inixcursor=460; iniycursor=83; END
        CASE 14..23:inixcursor=460; iniycursor=75+((cualnombre-12)*21); END
    END

    // Repite hasta que se pulse enter o escape, o el bot¢n derecho del rat¢n
    WHILE (NOT mouse.right AND NOT key(_enter) AND NOT key(_esc))
        // Hace que el cursor parpadee
        IF (parpadeocursor<3)
            poncursor(inixcursor+(posicicursor*10),iniycursor,9);
        ELSE
            poncursor(inixcursor+(posicicursor*10),iniycursor,0);
        END
        parpadeocursor++;IF (parpadeocursor>5) parpadeocursor=0;END
        FRAME;

        // Comprueba si se ha pulsado el rat¢n en la zona del teclado
        IF (mouse.left AND
            mouse.x>152 AND mouse.x<565 AND
            mouse.y>354 AND mouse.y<477)

            // Pone la tecla del teclado que se haya pulsado en una variable
            tecladeraton=((mouse.x-153)/42)+(((mouse.y-355)/42)*10);

            // Mira si es la tecla de borrado
            IF (tecladeraton==29)
                // Borra una letra poniendo un espacio
                posicicursor--; IF (posicicursor<0)posicicursor=0; END
                nombres[cualnombre].letras[posicicursor]=letras[27];
            ELSE
                // O pone la letra que se haya pulsado si esta dentro del limite
                IF (posicicursor<10)
                    nombres[cualnombre].letras[posicicursor]=letras[tecladeraton];
                END
                posicicursor++; IF (posicicursor>10)posicicursor=10; END
            END

            // Pone el gr fico de la tecla pulsada
            pontecla(tecladeraton);
            WHILE (mouse.left) FRAME; END
        END

        // Comprueba si se ha pulsado una tecla
        IF (scan_code<>0)
            // Comprueba si esta dentro de las teclas admisibles
            FOR (contador=0;contador<30;contador++)
                IF (scan_code==tablascan[contador])
                    // Mira si la tecla es la de borrado
                    IF (contador==29)
                        // Borra una tecla sustituy‚ndola por el espacio
                        posicicursor--; IF (posicicursor<0)posicicursor=0; END
                        nombres[cualnombre].letras[posicicursor]=letras[27];
                    ELSE
                        // Pone la letra que se haya pulsado
                        IF (posicicursor<10)
                            nombres[cualnombre].letras[posicicursor]=letras[contador];
                        END
                        posicicursor++; IF (posicicursor>10)posicicursor=10; END
                    END
                    // Pone el gr fico de la tecla pulsada
                    pontecla(contador);
                    // Hace que no se miren mas teclas en esta vez
                    WHILE (key(tablascan[contador])) FRAME; END
                    BREAK;
                END
            END
        END

    END
    editando=0;     // Como se ha acabado de editar, actualiza la variable
END

//-------------------------------------------------------------------
// Proceso pontecla
// Pone el gr fico de la tecla pulsada
// Entradas: cualtecla = N£mero de la tecla a imprimir
//-------------------------------------------------------------------

PROCESS pontecla (cualtecla)

BEGIN
    // Elige las coordenadas iniciales y el gr fico apropiado
    x=175+((cualtecla MOD 10)*41);
    y=375+((cualtecla/10)*41);
    graph=7;
    z=-5;
    FRAME(300);
END

//-------------------------------------------------------------------
// Proceso poncursor
// Pone el gr fico del cursor en la pantalla de editar nombres
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS poncursor (x,y,graph)

BEGIN
    FRAME;
END

//-------------------------------------------------------------------
// Proceso c mara
// Controla la c mara durante el partido
//-------------------------------------------------------------------

PROCESS c mara()

PRIVATE

    xdestc;     // Coordenadas x e y de donde tendr  que estar la c mara
    ydestc;
    cincx;      // Incrementos x e y que tiene la c mara
    cincy;
    cangle;     // Angulo hacia donde se dirige la c mara
    cdist;      // Distancia que le queda hasta la posici¢n ideal
    cvelo;      // Velocidad del movimiento de la c mara

BEGIN

    ctype=2;        // Lo pone dentro de modo 7
    x=32000;        // Selecciona las coordenadas iniciales
    y=100000;
    graph=0;        // No pone gr fico porque no tiene
    angle=90000;    // Selecciona el  ngulo hacia donde mira la c mara
    resolution=100; // Las coordenadas ir n con dos decimales
    LOOP
        // Comprueba si el jugador juega
        IF (controljugador==2)
            // Si el jugador no juega, sigue al bal¢n
            x=idpelota.x;
            y=idpelota.y+(distanciacamara*100);
            xdestc=x;
            ydestc=y;
        ELSE
            // Si el jugador tiene el bal¢n, le sigue la c mara
            IF (equipototal[1].conbalon<>0)
                xdestc=equipototal[1].conbalon.x;
                ydestc=equipototal[1].conbalon.y+(distanciacamara*100);
            ELSE
                // Si no, si el jugador esta esperando un pase, le sigue la c mara
                IF (equipototal[1].esperandopase<>0)
                    xdestc=equipototal[1].esperandopase.x;
                    ydestc=equipototal[1].esperandopase.y+(distanciacamara*100);
                    xdestc=(xdestc+idpelota.x*2)/3;
                ELSE
                    // Si no, si el jugador esta desmarcado, le sigue la c mara
                    IF (equipototal[1].desmarcado<>0)
                        xdestc=equipototal[1].desmarcado.x;
                        ydestc=equipototal[1].desmarcado.y+(distanciacamara*100);
                        xdestc=(xdestc+idpelota.x*2)/3;
                    ELSE
                        // Si no, la c mara sigue a la pelota
                        xdestc=idpelota.x;
                        ydestc=idpelota.y+(distanciacamara*100);
                    END
                END
            END


            // Coge el  ngulo hacia la posici¢n adecuada de la c mara
            cangle=fget_angle(x,y,xdestc,ydestc);

            // Coge la distancia
            cdist=fget_dist(x,y,xdestc,ydestc);

            // Haya la velocidad
            cvelo=cdist/totaltestigo;

            // Si la velocidad esta por encima del valor m ximo, la trunca
            IF (cvelo>vmaxima*6) cvelo=vmaxima*6; END

            // Coge los incrementos x e y, de acuerdo a la velocidad y el  ngulo
            cincx=get_distx(cangle,cvelo);
            cincy=get_disty(cangle,cvelo);

            // Mueve la c mara
            x+=cincx;
            y+=cincy;
        END

        // Si la pelota esta por detras de la camara por la c mara para que se pueda ver
        IF (y<idpelota.y+distanciacamara*100)
            y=idpelota.y+distanciacamara*100;
        END

        // No deja que se pase de los limites del campo
        IF (x<20900) x=20900; END
        IF (x>72100) x=72100; END
        IF (y<51300) y=51300; END
        IF (y>124900) y=124900; END

        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso valla
// Pone el gr fico del publico
//-------------------------------------------------------------------

PROCESS valla()

BEGIN
    // Pone los par metros correctos
    file=fichero1;  // Fichero gr fico
    graph=101;      // Gr fico
    ctype=2;        // Dentro de modo 7

    // Coordenadas
    x=((xfcampo-xicampo)/2)+xicampo; y=0;

    // Tama¤o
    size=444;
    LOOP FRAME; END

END

//-------------------------------------------------------------------
// Proceso tu_porteria
// Pone el gr fico de tu porter¡a
//-------------------------------------------------------------------

PROCESS tu_porteria()

BEGIN
    // Selecciona el gr fico de la red y que vaya en modo 7
    file=fichero1;
    ctype=2;
    graph=203;

    // Selecciona las coordenadas
    x=((xfcampo-xicampo)/2)+xicampo;
    y=yfcampo+2;
    flags=4;        // Pone al gr fico transparente
    palo1();        // Pone el palo de la porter¡a
    LOOP FRAME; END

END

//-------------------------------------------------------------------
// Proceso palo1
// Pone el palo de tu porter¡a
//-------------------------------------------------------------------

PROCESS palo1()

BEGIN
    // Selecciona el gr fico
    file=fichero1;
    graph=204;
    ctype=2;    // Lo pone en modo 7

    // Pone las coordenadas del gr fico
    x=((xfcampo-xicampo)/2)+xicampo; y=yfcampo+1;
    LOOP FRAME; END
END

//-------------------------------------------------------------------
// Proceso su_porteria
// Pone la porter¡a del ordenador
//-------------------------------------------------------------------

PROCESS su_porteria()

BEGIN
    // Selecciona el gr fico dependiendo del modo de v¡deo
    file=fichero1;
    graph=102+(100*modovideo);
    ctype=2;        // Lo pone en modo 7

    // Pone las coordenadas
    x=((xfcampo-xicampo)/2)+xicampo;
    y=yicampo-2;

    // Como es la red, la pone transparente
    flags=4;

    // Pone el palo de la porter¡a
    palo2();
    LOOP FRAME; END
END

//-------------------------------------------------------------------
// Proceso palo2
// Pone el gr fico del palo de la porter¡a
//-------------------------------------------------------------------

PROCESS palo2()
BEGIN
    // Selecciona el gr fico
    file=fichero1;
    graph=201;
    ctype=2;    // Lo pone dentro de modo 7

    // Selecciona la s coordenadas
    x=((xfcampo-xicampo)/2)+xicampo; y=yicampo-1;
    LOOP FRAME; END
END

//-------------------------------------------------------------------
// Proceso bal¢n
// Maneja el bal¢n durante el juego
//-------------------------------------------------------------------

PROCESS bal¢n()

PRIVATE

    incrx,incry; // Incrementos x e y del bal¢n

BEGIN
    // Pone las coordenadas iniciales y la altura
    x=(((xfcampo-xicampo)/2)+xicampo)*100;
    y=(((yfcampo-yicampo)/2)+yicampo)*100;
    height=0;

    // Selecciona el gr fico y el tama¤o
    file=fichero6;
    graph=1;
    size=20;
    ctype=2;        // Lo pone dentro del modo 7
    resolution=100; // Usa dos decimales
    sombrabalon();  // Pone la sombra del bal¢n

    LOOP
        // Mira la velocidad del bal¢n
        IF (velocidadpelota>0)
            velocidadpelota-=incrvelocidadpelota;   // Lo va frenando
            // Mueve el bal¢n
            x+=get_distx(angulopelota,velocidadpelota);
            y+=get_disty(angulopelota,velocidadpelota);
        END

        // Comprueba si el bal¢n esta botando
        IF (incbote<>0)
            height+=incactual;                  // Mueve el bal¢n en altura
            incactual-=2;
            IF (abs(incactual)>abs(incbote))    // Hasta que se llegue al limite
                sound(idsonido5,50,256);
                // Dependiendo de la velocidad, as¡ bota el bal¢n
                incbote=(((velocidadpelota/100)-15)/10)*2;
                // Siempre dentro de unos limites
                IF (incbote>20) incbote=20; END
                IF (incbote<0) incbote=0; END
                incactual=incbote;
            END
        END

        // Si el bal¢n se mueve o bota, realiza la animaci¢n
        IF (velocidadpelota>0 OR incbote<>0 OR
            equipototal[0].conbalon<>0 OR equipototal[1].conbalon<>0)
            IF (graph<>0)
                graph++;IF (graph>6) graph=1; END
            END
        END

        // Comprueba si se ha salido el bal¢n por la banda derecha
        IF (x<xicampo*100 AND sacando==0 AND estadojuego==4)
            bal¢nfantasma(x,y,size,height,graph,file,velocidadpelota,angulopelota);
            graph=0;
            ponmensajes (4,20);
            velocidadpelota=0;
            estadojuego=10;
            quiensacadebanda=1-ultimotiro;
            x=(xicampo-5)*100;
        END

        // Comprueba si se ha salido el bal¢n por la banda izquierda
        IF (x>xfcampo*100 AND sacando==0 AND estadojuego==4)
            bal¢nfantasma(x,y,size,height,graph,file,velocidadpelota,angulopelota);
            graph=0;
            ponmensajes (4,20);
            estadojuego=10;
            quiensacadebanda=1-ultimotiro;
            x=(xfcampo+5)*100;
            velocidadpelota=0;
        END

        // Comprueba si ha metido gol el jugador
        IF (y<yicampo*100 AND x>((((xfcampo-xicampo)/2)+xicampo-40)*100) AND x<((((xfcampo-xicampo)/2)+xicampo+40)*100))
            ponmensajes (3,25);
            quienmetegol=2;
            quienempieza=0;
            estadojuego=5;
            goles[0]++;
            sound(idsonido1,50,256);
        END

        // Comprueba si ha metido gol el ordenador
        IF (y>yfcampo*100 AND x>((((xfcampo-xicampo)/2)+xicampo-40)*100) AND x<((((xfcampo-xicampo)/2)+xicampo+40)*100))
            ponmensajes (3,25);
            quienmetegol=1;
            quienempieza=1;
            estadojuego=5;
            goles[1]++;
            sound(idsonido1,50,256);
        END

        // Comprueba los rebotes con los postes de la porter¡a
        IF ((y<yicampo*100 OR y>yfcampo*100) AND ((x>((((xfcampo-xicampo)/2)+xicampo-48)*100) AND x<((((xfcampo-xicampo)/2)+xicampo-40)*100)) OR
            (x>((((xfcampo-xicampo)/2)+xicampo+40)*100) AND x<((((xfcampo-xicampo)/2)+xicampo+48)*100))))
            incrx=get_distx(angulopelota,velocidadpelota);
            incry=get_disty(angulopelota,velocidadpelota);
            incry=-incry;
            angulopelota=fget_angle(0,0,incrx,incry);
        END
        IF ((y<yicampo*100 OR y>yfcampo*100) AND x>((((xfcampo-xicampo)/2)+xicampo-48)*100) AND x<((((xfcampo-xicampo)/2)+xicampo-40)*100) AND
            x>((((xfcampo-xicampo)/2)+xicampo+40)*100) AND x<((((xfcampo-xicampo)/2)+xicampo+48)*100))
            incrx=get_distx(angulopelota,velocidadpelota);
            incry=get_disty(angulopelota,velocidadpelota);
            incry=-incry;
            angulopelota=fget_angle(0,0,incrx,incry);
        END

        // Comprueba si se ha salido el bal¢n por el borde superior
        IF (y<=yicampo*100  AND (x<((((xfcampo-xicampo)/2)+xicampo-48)*100) OR x>((((xfcampo-xicampo)/2)+xicampo+48)*100)) AND sacandocorner==0 AND sacandoporteria==0)
            // Saca el portero
            bal¢nfantasma(x,y,size,height,graph,file,velocidadpelota,angulopelota);
            graph=0;
            IF (ultimotiro==1)
                idporteros[1].velocidad=0;
                idporteros[1].estado=conbalon;
                velocidadpelota=0;
                estadojuego=15;
                porterosacando=1;
                sacandoporteria=1;
            ELSE
                // Se ha producido un c¢rner
                ponmensajes (5,20);
                sacandocorner=0;
                estadojuego=20;
                quiensacacorner=1;
                // Comprueba en que esquina se ha producido
                IF (x<((((xfcampo-xicampo)/2)+xicampo-48)*100))
                    esquinacorner=0;
                    idpelota.x=xicampo*100;
                ELSE
                    esquinacorner=1;
                    idpelota.x=xfcampo*100;
                END
                idpelota.y=yicampo*100;
                idpelota.height=0;
            END
        END

        // Comprueba si se ha salido el bal¢n por el borde inferior
        IF (y=>yfcampo*100 AND (x<((((xfcampo-xicampo)/2)+xicampo-48)*100) OR x>((((xfcampo-xicampo)/2)+xicampo+48)*100)) AND sacandocorner==0 AND sacandoporteria==0)
            bal¢nfantasma(x,y,size,height,graph,file,velocidadpelota,angulopelota);
            graph=0;
            IF (ultimotiro==0)
                // Saca el portero
                idporteros[0].velocidad=0;
                idporteros[0].estado=conbalon;
                velocidadpelota=0;
                estadojuego=15;
                porterosacando=0;
                sacandoporteria=1;
            ELSE
                // O se ha producido un c¢rner
                ponmensajes (5,20);
                sacandocorner=0;
                estadojuego=20;
                quiensacacorner=0;
                // Comprueba en que esquina se ha producido
                IF (x<((((xfcampo-xicampo)/2)+xicampo-48)*100))
                    esquinacorner=2;
                    idpelota.x=xicampo*100;
                ELSE
                    esquinacorner=3;
                    idpelota.x=xfcampo*100;
                END
                idpelota.y=yfcampo*100;
                idpelota.height=0;
            END
        END

        // Evita que el bal¢n este por debajo del terreno
        IF (height<0) height=0; END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso sombrabalon
// Pone la sombra del bal¢n
//-------------------------------------------------------------------

PROCESS sombrabalon()

BEGIN
    // Elige el gr fico y el tama¤o del mismo
    file=fichero1;
    graph=11;
    size=50;

    ctype=2;        // Lo pone en modo 7
    resolution=100; // Usa dos decimales como el proceso que lo llama

    flags=4;        // Lo pone transparente
    z=(father.z)+10;// Y por detraes del bal¢n

    LOOP
        // Hace que siga al bal¢n
        x=father.x;
        y=father.y-2;
        FRAME;
    END

END

//-------------------------------------------------------------------
// Proceso ponjugador
// Maneja a los jugadores durante el juego
// Entradas: n£mero = N£mero del jugador
//           equipo = Equipo del jugador
//-------------------------------------------------------------------

PROCESS ponjugador(n£mero,equipo)

PRIVATE

    estado_anterior;    // Estado anterior del jugador
    xdest; ydest;       // Coordenadas de destino
    pospelox,pospeloy;  // Pr¢xima posici¢n del bal¢n
    pasex,pasey;        // Posicion donde caera el balon al hacer un pase
    fuerzabalon;        // Fuerza con la que se tirara el bal¢n
    iniciosaque;        // Sirve para seleccionar el que saca

    // Tabla con las animaciones de todas las posiciones

    tablag1[]=8,67,68,69,70,71,72,65,66;    // Posici¢n parado

    tablag10[]=8,17,25,33,-25,-17, -9,1, 9; // Posici¢n corriendo
    tablag11[]=8,18,26,34,-26,-18,-10,2,10;
    tablag12[]=8,19,27,35,-27,-19,-11,3,11;
    tablag13[]=8,20,28,36,-28,-20,-12,4,12;
    tablag14[]=8,21,29,37,-29,-21,-13,5,13;
    tablag15[]=8,22,30,38,-30,-22,-14,6,14;
    tablag16[]=8,23,31,39,-31,-23,-15,7,15;
    tablag17[]=8,24,32,40,-32,-24,-16,8,16;

    tablag20[]=8,57,58,59,60,61,62,63,64;   // Posici¢n tir ndose

    tablag30[]=8,51,52,53,54,55,56,49,50;   // Posici¢n sacando de banda
    tablag31[]=8,43,44,45,46,47,48,41,42;
    tablag32[]=8,75,76,77,78,79,80,73,74;

    tablag40[]=8,83,84,85,86,87,88,81,82;   // Posici¢n tirando
    tablag41[]=8,91,92,93,94,95,96,89,90;
    dirconcontrol;

BEGIN
    // Corrige la altura del gr fico del jugador
    height=-5;

    // Pone la sombra del jugador
    sombrajugador();

    IF (equipo==0)
        // Selecciona el fichero gr fico y lo reinicializa
        file=fichero2;
        xgraph=&tablag1;

        // Pasa la t ctica a las variables locales del jugador
        IF (quienempieza==0)
            xi=(t ctica[n£mero].xi0+xicampo)*100;
            yi=(t ctica[n£mero].yi0+yicampo)*100;
        ELSE
            xi=(t ctica[n£mero].xi1+xicampo)*100;
            yi=(t ctica[n£mero].yi1+yicampo)*100;
        END
        xp0=(t ctica[n£mero].xp0+rand(-20,20)+xicampo)*100;
        yp0=(t ctica[n£mero].yp0+rand(-20,20)+yicampo)*100;
        xp1=(t ctica[n£mero].xp1+rand(-20,20)+xicampo)*100;
        yp1=(t ctica[n£mero].yp1+rand(-20,20)+yicampo)*100;
    ELSE
        // Selecciona el fichero gr fico y lo reinicializa
        file=fichero3;
        xgraph=&tablag1;

        // Pasa la t ctica a las variables locales del jugador pero espejada
        IF (quienempieza==0)
            xi=(xfcampo-tactica[n£mero].xi1)*100;
            yi=(yfcampo-tactica[n£mero].yi1)*100;
        ELSE
            xi=(xfcampo-tactica[n£mero].xi0)*100;
            yi=(yfcampo-tactica[n£mero].yi0)*100;
        END
        xp0=(xfcampo-tactica[n£mero].xp0+rand(-20,20))*100;
        yp0=(yfcampo-tactica[n£mero].yp0+rand(-20,20))*100;
        xp1=(xfcampo-tactica[n£mero].xp1+rand(-20,20))*100;
        yp1=(yfcampo-tactica[n£mero].yp1+rand(-20,20))*100;
    END

    // Pasa lo que queda de t ctica a los jugadores
    p0=tactica[n£mero].p0;
    p1=tactica[n£mero].p1;

    // Pone el estado inicial
    estado=parado;

    // Pone las coordenadas iniciales al azar
    x=rand(500,1500);
    y=rand(50500,52500);

    // Pone el tama¤o de los gr ficos
    size=150;
    ctype=2;                // Lo mete dentro de modo 7
    resolution=100;         // Usa dos decimales en las coordenadas
    cuentaanima=rand(0,7);  // Pone una posici¢n de animaci¢n al azar
    velocidad=1;            // Necesario para que ponga la animaci¢n a 0
    LOOP
        // Si toca a un jugador a cierta altura rebota al azar
        IF (fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida AND idpelota.height>0 AND idpelota.height<100)
            idpelota.angulopelota=rand(0,360)*100;
            idpelota.x+=get_distx(idpelota.angulopelota,idpelota.velocidadpelota);
            idpelota.y+=get_disty(idpelota.angulopelota,idpelota.velocidadpelota);
            ultimotiro=equipo;  // Necesario para los saques de banda y los c¢rners
        END

        // Comprueba que se le ha pasado el testigo
        IF ((testigo1==(n£mero/(10/totaltestigo)) AND equipo==0) OR
            (testigo2==(n£mero/(10/totaltestigo)) AND equipo==1) )
            REPEAT
                // Guarda el estado anterior
                estado_anterior=estado;

                // Comprueba en que estado esta el bal¢n
                SWITCH (estado)
                    // Si esta parado, va hacia la posici¢n inicial de partido
                    CASE parado:
                        velocidad=fget_dist(x,y,xi,yi)/totaltestigo;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        angle=fget_angle(x,y,xi,yi);
                        IF (fget_dist(x,y,xi,yi)<=(vmaxima+200)/2) velocidad=0; END
                        IF (velocidad==0) angle=fget_angle(x,y,idpelota.x,idpelota.y); END
                    END

                    // Este es el estado normal de juego
                    CASE normal:

                        // Calcula las coordenadas ponderando las posiciones de ataque y defensa
                        xdest=((xp0*p0*equipototal[equipo].defensa)+
                            (xp1*p1*equipototal[equipo].ataque)+
                            (idpelota.x*pesopelota*100))/
                            (p0*equipototal[equipo].defensa+
                             p1*equipototal[equipo].ataque+
                             pesopelota*100);

                        ydest=((yp0*p0*equipototal[equipo].defensa)+
                            (yp1*p1*equipototal[equipo].ataque)+
                            (idpelota.y*pesopelota*100))/
                            (p0*equipototal[equipo].defensa+
                             p1*equipototal[equipo].ataque+
                             pesopelota*100);

                        // Va hacia la posici¢n correcta
                        IF (fget_dist(x,y,xdest,ydest)>3000)
                            velocidad=fget_dist(x,y,xdest,ydest)/totaltestigo;
                            IF (velocidad>vmaxima) velocidad=vmaxima; END
                            IF (velocidad<100) velocidad=100; END
                            angle=fget_angle(x,y,xdest,ydest);
                            IF (fget_dist(x,y,xdest,ydest)<=(vmaxima+200)/2) velocidad=0; END
                        ELSE
                            velocidad=0;
                        END
                    END

                    // Estado cuando lleva la pelota
                    CASE conbalon:
                        ultimotiro=equipo; // Necesario para el saque

                        // Si hubiera un desmarcado lo quita
                        IF (equipototal[equipo].desmarcado<>0)
                            equipototal[equipo].desmarcado.estado=normal;
                        END
                        equipototal[equipo].desmarcado=0;

                        // Si hubiera alguien esperando un pase lo quita
                        IF (equipototal[equipo].esperandopase<>0)
                            equipototal[equipo].esperandopase.estado=normal;
                        END
                        equipototal[equipo].esperandopase=0;

                        IF (equipo==0)      // Si el equipo es el del ordenador
                            // Mira si esta cerca de la porter¡a
                            IF (y>(yfcampo*100)-zonatiro1)
                                sound(idsonido3,50,256);
                                // Pone al jugador en estado atontado
                                estado=atontado;
                                cuentaatontado=retardoatontado;
                                // Y tira el bal¢n
                                idpelota.angulopelota=fget_angle(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yfcampo*100);
                                idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yfcampo*100));
                                IF (idpelota.velocidadpelota>conbote)
                                     incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                END
                                (equipototal[0].conbalon).estado=normal;
                                equipototal[0].conbalon=0;
                                tirando=2;
                                ultimotiro=0;
                            ELSE
                                // Sino esta dentro de la zona de tiro mira si tiene a alg£n contrario cerca
                                FOR (contador=0;contador<10;++contador)
                                    IF (fget_dist(x,y,idjugadores2[contador].x,idjugadores2[contador].y)<campoagobio
                                        AND idjugadores2[contador].tirandose==0)
                                        // Si hay alg£n contrario cerca
                                        IF (y>(yfcampo*100)-zonatiro2)
                                            // Y esta dentro de la zona de tiro 2
                                            sound(idsonido3,50,256);

                                            // Tira la pelota
                                            idpelota.angulopelota=fget_angle(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yfcampo*100);
                                            idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yfcampo*100));
                                            IF (idpelota.velocidadpelota>conbote)
                                                incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                            END
                                            equipototal[0].conbalon=0;

                                            // Pone al jugador en estado atontado
                                            estado=atontado;
                                            cuentaatontado=retardoatontado;
                                            tirando=2;
                                            ultimotiro=0;
                                        ELSE

                                            // Si no esta dentro de la zona de tiro 2
                                            sound(idsonido3,50,256);

                                            // Busca un desmarcado
                                            pasa_a=daquienpasa(equipo);

                                            // Y le pasa la pelota
                                            idpelota.angulopelota=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                            angle=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                            idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y))+(rand(-azar,azar)*100);
                                            IF (idpelota.velocidadpelota>conbote)
                                                incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                            END
                                            ultimotiro=0;

                                            // Pone al jugador que pasa en estado esperandopase
                                            idjugadores1[pasa_a].estado=esperandopase;
                                            idjugadores1[pasa_a].velocidad=0;
                                            equipototal[0].esperandopase=idjugadores1[pasa_a];
                                            equipototal[0].conbalon=0;

                                            // Pone al jugador en estado atontado
                                            estado=atontado;
                                            cuentaatontado=retardoatontado;
                                            tirando=2;
                                        END
                                    END
                                END
                                // Si el estado es con bal¢n mueve al jugador hacia la porter¡a
                                IF (estado==conbalon)
                                    velocidad=fget_dist(x,y,(((xfcampo-xicampo)/2)+xicampo)*100,yfcampo*100)/totaltestigo;
                                    IF (velocidad>vmaximacg) velocidad=vmaximacg; END
                                    IF (velocidad<100) velocidad=100; END
                                    angle=fget_angle(x,y,(((xfcampo-xicampo)/2)+xicampo)*100,yfcampo*100);
                                END
                            END
                        ELSE
                            // Si el jugador no es el ordenador
                            // Comprueba el tipo de control que usa
                            IF (controljugador==2)
                                // El jugador lo lleva el ordenador
                                IF (y<(yicampo*100)+zonatiro1)
                                    // Si esta dentro de la zona de tiro 1
                                    sound(idsonido3,50,256);
                                    // Pone al jugador en el estado atontado
                                    estado=atontado;
                                    cuentaatontado=retardoatontado;
                                    // Y tira el bal¢n
                                    idpelota.angulopelota=fget_angle(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yicampo*100);
                                    idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yicampo*100));
                                    // Comprobando si se le tiene que aplicar un bote
                                    IF (idpelota.velocidadpelota>conbote)
                                        incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                    END
                                    (equipototal[1].conbalon).estado=normal;
                                    equipototal[1].conbalon=0;
                                    tirando=2;
                                    ultimotiro=1;
                                ELSE
                                    // Sino esta dentro de la zona de tiro 1
                                    // Comprueba si tiene un jugador contrario cerca
                                    FOR (contador=0;contador<10;++contador)
                                        IF (fget_dist(x,y,idjugadores1[contador].x,idjugadores1[contador].y)<campoagobio AND idjugadores1[contador].estado<>atontado)
                                            // Si tiene un contrario cerca
                                            IF (y<(yicampo*100)+zonatiro2)

                                                // Y esta dentro de la zona de tiro 2
                                                sound(idsonido3,50,256);

                                                // Dispara el bal¢n
                                                idpelota.angulopelota=fget_angle(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yicampo*100);
                                                idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,((((xfcampo-xicampo)/2)+xicampo)*100)+((rand(-azar*10,azar*10))*100),yicampo*100));
                                                IF (idpelota.velocidadpelota>conbote)
                                                    incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                                END
                                                equipototal[1].conbalon=0;
                                                contador=10;

                                                // Y pone al jugador en estado atontado
                                                estado=atontado;
                                                cuentaatontado=retardoatontado;
                                                tirando=2;
                                                ultimotiro=1;
                                            ELSE
                                                // Sino esta dentro de ninguna zona de tiro y esta agobiado
                                                sound(idsonido3,50,256);
                                                ultimotiro=1;

                                                // Busca un desmarcado
                                                pasa_a=daquienpasa(equipo);

                                                // Y le pasa el bal¢n
                                                idpelota.angulopelota=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                                angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                                idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y))+(rand(-azar,azar)*100);;

                                                // Mira si el bal¢n tiene que botar (salir con altura)
                                                IF (idpelota.velocidadpelota>conbote)
                                                    incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                                END

                                                // Pone al jugador que recibe el pase en estado esperandopase
                                                idjugadores2[pasa_a].estado=esperandopase;
                                                idjugadores2[pasa_a].velocidad=0;
                                                equipototal[1].esperandopase=idjugadores2[pasa_a];
                                                equipototal[1].conbalon=0;
                                                contador=10;

                                                // Y a este jugador lo pone en estado atontado
                                                estado=atontado;
                                                cuentaatontado=retardoatontado;
                                                tirando=2;
                                            END
                                        END
                                    END
                                    // Si el jugador sigue teniendo el bal¢n entonces se dirige hacia la porter¡a
                                    IF (estado==conbalon)
                                        velocidad=fget_dist(x,y,(((xfcampo-xicampo)/2)+xicampo)*100,yicampo*100)/totaltestigo;
                                        IF (velocidad>vmaximacg) velocidad=vmaximacg; END
                                        IF (velocidad<100) velocidad=100; END
                                        angle=fget_angle(x,y,(((xfcampo-xicampo)/2)+xicampo)*100,yicampo*100);
                                    END
                                END
                            ELSE    // Cuando se controla con teclado o joystick
                                    // Comprueba si tiene un jugador contrario cerca
                                FOR (contador=0;contador<10;++contador)
                                    IF (fget_dist(x,y,idjugadores1[contador].x,idjugadores1[contador].y)<campoagobiojug)

                                        // Sino esta dentro de ninguna zona de tiro y esta agobiado
                                        sound(idsonido3,50,256);
                                        ultimotiro=1;

                                        // Busca un desmarcado
                                        pasa_a=daquienpasa(equipo);

                                        // Y le pasa el bal¢n
                                        idpelota.angulopelota=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y));

                                        // Mira si el bal¢n tiene que botar (salir con altura)
                                        IF (idpelota.velocidadpelota>conbote)
                                            incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                                        END

                                        // Pone al jugador que recibe el pase en estado esperandopase
                                        idjugadores2[pasa_a].estado=esperandopase;
                                        idjugadores2[pasa_a].velocidad=0;
                                        equipototal[1].esperandopase=idjugadores2[pasa_a];
                                        equipototal[1].conbalon=0;
                                        contador=10;

                                        // Y a este jugador lo pone en estado atontado
                                        estado=atontado;
                                        cuentaatontado=retardoatontado;
                                        tirando=2;
                                    END
                                END
                                // Si el ordenador no controla al equipo
                                dirconcontrol=0;
                                IF (controljugador==0)  // Si el control es el joystick

                                    // Lee el joystick
                                    IF (joy.right) dirconcontrol+=1; END
                                    IF (joy.left) dirconcontrol-=1; END
                                    IF (joy.up) dirconcontrol-=10; END
                                    IF (joy.down) dirconcontrol+=10; END
                                ELSE

                                    // Y si no lee el teclado
                                    IF (key(_right)) dirconcontrol+=1; END
                                    IF (key(_left)) dirconcontrol-=1; END
                                    IF (key(_up)) dirconcontrol-=10; END
                                    IF (key(_down)) dirconcontrol+=10; END
                                END

                                // Le pone velocidad o no, seg£n si se esta moviendo
                                IF (dirconcontrol<>0) velocidad=400; ELSE velocidad-=300; END
                                IF (velocidad>vmaximacgjug) velocidad=vmaximacgjug; END
                                IF (velocidad<100) velocidad=0; END

                                // Pone el  ngulo dependiendo de las teclas pulsadas
                                SWITCH(dirconcontrol)
                                    CASE -11: angle=135000; END
                                    CASE -10: angle=90000; END
                                    CASE -9: angle=45000; END
                                    CASE -1: angle=180000; END
                                    CASE 1: angle=0; END
                                    CASE 9: angle=225000; END
                                    CASE 10: angle=270000; END
                                    CASE 11: angle=315000; END
                                END
                                IF (controljugador==0) // Comprueba si el control es el joystick
                                    IF (joy.button1 AND tirandose==0)   // Si se pulsa el bot¢n 1 del joystick
                                        sound(idsonido3,50,256);

                                        // Dispara el bal¢n a distancia corta
                                        idpelota.angulopelota=angle;
                                        idpelota.velocidadpelota=2000;
                                        fuerza_tiro();
                                        incactual=16; incbote=16; idpelota.height=0;

                                        // Pone al jugador en estado atontado
                                        estado=atontado;
                                        cuentaatontado=retardoatontado;
                                        equipototal[equipo].conbalon=0;
                                        tirando=2;
                                        ultimotiro=1;
                                    END

                                    IF (joy.button2)    // Si se pulsa el bot¢n 2
                                        sound(idsonido3,50,256);
                                        ultimotiro=1;

                                        // Busca un desmarcado
                                        pasex=x+get_distx(angle,35000);
                                        pasey=y+get_disty(angle,35000);
                                        pasa_a=0;
                                        FROM contador=0 TO 9;
                                            IF (numero<>contador)
                                                IF (fget_dist(pasex,pasey,idjugadores2[contador].x,idjugadores2[contador].y)<
                                                    fget_dist(pasex,pasey,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y))
                                                    pasa_a=contador;
                                                END
                                            END
                                        END

                                        // Y le pasa el bal¢n
                                        idpelota.angulopelota=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y));
                                        estado=atontado;
                                        cuentaatontado=retardoatontado;
                                        tirando=2;
                                        equipototal[equipo].conbalon=0;
                                    END
                                ELSE
                                    // Si no, es que esta usando el teclado
                                    IF (key(_control) AND tirandose==0)

                                        // Si se pulsa la tecla control hace el disparo corto
                                        sound(idsonido3,50,256);

                                        // Dispara el bal¢n
                                        idpelota.angulopelota=angle;
                                        idpelota.velocidadpelota=2000;
                                        fuerza_tiro();
                                        incactual=16; incbote=16; idpelota.height=0;

                                        // Pone el estado del jugador en atontado
                                        estado=atontado;
                                        cuentaatontado=retardoatontado;
                                        equipototal[equipo].conbalon=0;
                                        tirando=2;
                                        ultimotiro=1;
                                    END
                                    IF (key(_alt))

                                        // Si se pulsa la tecla alt hace disparo largo
                                        sound(idsonido3,50,256);
                                        ultimotiro=1;

                                        // Dispara el bal¢n
                                        pasex=x+get_distx(angle,35000);
                                        pasey=y+get_disty(angle,35000);
                                        pasa_a=0;
                                        FROM contador=0 TO 9;
                                            IF (numero<>contador)
                                                IF (fget_dist(pasex,pasey,idjugadores2[contador].x,idjugadores2[contador].y)<
                                                    fget_dist(pasex,pasey,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y))
                                                    pasa_a=contador;
                                                END
                                            END
                                        END
                                        idpelota.angulopelota=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                        idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y));

                                        // Y pone al jugador en estado atontado
                                        estado=atontado;
                                        cuentaatontado=retardoatontado;
                                        tirando=2;
                                        equipototal[equipo].conbalon=0;
                                    END
                                END
                            END
                        END
                    END

                    CASE atontado:  // Estado del jugador atontado

                            // Espera hasta que el contador llegue a 0
                            cuentaatontado--;
                            IF (cuentaatontado==0)
                                // Y pone al jugador en estado normal
                                estado=normal;
                            END
                    END
                    // Estado del jugador cuando esta esperando un pase
                    CASE esperandopase:
                        // Si el bal¢n esta dentro de la zona de recogida lo coge
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida AND idpelota.height==0)

                            // Quita a cualquier otro compa¤ero que estuviera desmarcado
                            IF (equipototal[equipo].desmarcado<>0)
                                (equipototal[equipo].desmarcado).estado=normal;
                            END
                            equipototal[equipo].desmarcado=0;

                            // Quita a cualquier otro que tuviera el bal¢n y lo pone en atontado
                            IF (equipototal[0].conbalon<>0)
                                (equipototal[0].conbalon).estado=atontado;
                                (equipototal[0].conbalon).cuentaatontado=retardoatontado;
                                equipototal[0].conbalon=0;
                            END
                            IF (equipototal[1].conbalon<>0)
                                (equipototal[1].conbalon).estado=atontado;
                                (equipototal[1].conbalon).cuentaatontado=retardoatontado;
                                equipototal[1].conbalon=0;
                            END

                            // Y se le pone en el estado conbalon
                            estado=conbalon;
                            equipototal[equipo].conbalon=id;
                            equipototal[equipo].esperandopase=0;
                        END

                        // Si la pelota esta casi parada, el jugador va a por ella
                        IF (idpelota.velocidadpelota<500)
                            idpelota.velocidadpelota=0;
                            velocidad=fget_dist(x,y,idpelota.x,idpelota.y)/totaltestigo;
                            IF (velocidad>vmaxima) velocidad=vmaxima; END
                            IF (velocidad<100) velocidad=100; END
                            angle=fget_angle(x,y,idpelota.x,idpelota.y);
                        ELSE
                            velocidad=0;
                        END
                    END
                    // Estado cuando el jugador esta desmarcado, el mas cerca de la pelota
                    CASE desmarcado:

                        // Si el jugador esta cerca de la pelota se tira a por ella
                        IF (equipo==0)  // Si el equipo es el del ordenador

                            // Dirige al jugador en la direcci¢n donde vaya el bal¢n
                            pospelox=da_pos_balon(0,equipo);
                            pospeloy=da_pos_balon(1,equipo);
                            velocidad=fget_dist(x,y,pospelox,pospeloy)/totaltestigo;
                            IF (velocidad>vmaxima) velocidad=vmaxima; END
                            IF (velocidad<100) velocidad=100; END
                            angle=fget_angle(x,y,pospelox,pospeloy);
                        ELSE

                            // Si es el equipo del jugador
                            IF (controljugador==2)  // Pero lo controla el ordenador
                                // Dirige al jugador en la direcci¢n donde vaya el bal¢n
                                pospelox=da_pos_balon(0,equipo);
                                pospeloy=da_pos_balon(1,equipo);
                                velocidad=fget_dist(x,y,pospelox,pospeloy)/totaltestigo;
                                IF (velocidad>vmaxima) velocidad=vmaxima; END
                                IF (velocidad<100) velocidad=100; END
                                angle=fget_angle(x,y,pospelox,pospeloy);
                            ELSE
                                // Si no lo dirige el ordenador
                                dirconcontrol=0;
                                IF (controljugador==0)
                                    // Lee el joystick
                                    IF (joy.right) dirconcontrol+=1; END
                                    IF (joy.left) dirconcontrol-=1; END
                                    IF (joy.up) dirconcontrol-=10; END
                                    IF (joy.down) dirconcontrol+=10; END
                                    IF (joy.button1 AND fget_dist(x,y,idpelota.x,idpelota.y)<zonaquitada)
                                        sound(idsonido5,50,256);
                                        tirandose=1;
                                        angle=fget_angle(x,y,idpelota.x,idpelota.y);
                                    END
                                ELSE
                                    // O lee el teclado
                                    IF (key(_right)) dirconcontrol+=1; END
                                    IF (key(_left)) dirconcontrol-=1; END
                                    IF (key(_up)) dirconcontrol-=10; END
                                    IF (key(_down)) dirconcontrol+=10; END
                                    IF (key(_control) AND
                                        fget_dist(x,y,idpelota.x,idpelota.y)<zonaquitada)
                                        sound(idsonido5,50,256);
                                        tirandose=1;
                                        angle=fget_angle(x,y,idpelota.x,idpelota.y);
                                    END
                                END

                                // Mueve o no, dependiendo si se ha elegido una direcci¢n
                                IF (dirconcontrol<>0) velocidad+=400; ELSE velocidad-=300; END
                                IF (velocidad>vmaximajug) velocidad=vmaximajug; END
                                IF (velocidad<100) velocidad=0; END

                                // Elige el  ngulo dependiendo de las teclas pulsadas o la lectura del joystick
                                SWITCH(dirconcontrol)
                                    CASE -11: angle=135000; END
                                    CASE -10: angle=90000; END
                                    CASE -9: angle=45000; END
                                    CASE -1: angle=180000; END
                                    CASE 1: angle=0; END
                                    CASE 9: angle=225000; END
                                    CASE 10: angle=270000; END
                                    CASE 11: angle=315000; END
                                END
                            END
                        END

                        // Si el bal¢n esta dentro de la zona de recogida
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida*2 AND idpelota.height==0)
                            equipototal[equipo].desmarcado=0;
                            // Quita a cualquiera que hubiera con bal¢n y lo pone en estado atontado
                            IF (equipototal[0].conbalon<>0)
                                (equipototal[0].conbalon).estado=atontado;
                                (equipototal[0].conbalon).cuentaatontado=retardoatontado;
                                equipototal[0].conbalon=0;
                            END
                            IF (equipototal[1].conbalon<>0)
                                (equipototal[1].conbalon).estado=atontado;
                                (equipototal[1].conbalon).cuentaatontado=retardoatontado;
                                equipototal[1].conbalon=0;
                            END

                            // Pone al jugador en estado conbalon
                            estado=conbalon;
                            equipototal[equipo].conbalon=id;

                            // Quita a cualquiera que estuviera esperando un pase
                            IF (equipototal[0].esperandopase<>0)
                                (equipototal[0].esperandopase).estado=normal;
                            END
                            IF (equipototal[1].esperandopase<>0)
                                (equipototal[1].esperandopase).estado=normal;
                            END
                            equipototal[0].esperandopase=0;
                            equipototal[1].esperandopase=0;
                        END
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida*3)
                            angle=fget_angle(x,y,idpelota.x,idpelota.y);
                            IF (equipo==0)
                                IF (equipototal[1].conbalon<>0)
                                    sound(idsonido6,50,256);
                                    tirandose=1;
                                END
                            ELSE
                                IF (equipototal[0].conbalon<>0)
                                    sound(idsonido5,50,256);
                                    tirandose=1;
                                END
                            END
                        END
                    END

                    // Estado del que va a hacer un saque de banda
                    CASE irasaque:
                        // Va hacia donde esta el bal¢n, que esta fuera de banda
                        velocidad=fget_dist(x,y,idpelota.x,idpelota.y)/totaltestigo;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        angle=fget_angle(x,y,idpelota.x,idpelota.y);
                        // Cuando llega hasta el bal¢n se le pone en estado saquedebanda
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<=(vmaxima+200)/2)
                            velocidad=0;
                            estado=saquedebanda;
                            idpelota.graph=1;
                        END
                    END

                    // Jugador que esta sacando de banda
                    CASE saquedebanda:
                        // Mira en que estado esta el saque de banda
                        SWITCH (estadosaque)
                            CASE 0:
                                // Pone el gr fico 1 de la animaci¢n de saque de banda
                                xgraph=&tablag30;
                                angle=fget_angle(x,y,(((xfcampo-xicampo)/2)+xicampo)*100,(((yfcampo-yicampo)/2)+yicampo)*100);
                                IF (equipo==1 AND controljugador<2)
                                    IF (numero==0) elegidosaque=1; iniciosaque=2; ELSE elegidosaque=0; iniciosaque=1; END
                                    FOR (contador=iniciosaque;contador<10;contador++)
                                        IF (contador<>numero)
                                            IF (fget_dist(idpelota.x,idpelota.y,idjugadores2[contador].x,idjugadores2[contador].y)<
                                                fget_dist(idpelota.x,idpelota.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y))
                                                elegidosaque=contador;
                                            END
                                        END
                                    END
                                    elige_saque(numero);
                                    FRAME;
                                    pasa_a=elegidosaque;
                                ELSE
                                    // Busca un desmarcado y pone al jugador mirando hacia el elegido
                                    pasa_a=daquienpasa(equipo);
                                END
                                sound(idsonido7,50,256);
                                // Sube el peso de la pelota para que los jugadores se acerquen
                                pesopelota=6;
                                // Quita el gr fico de la pelota, porque el jugador ya lo tiene
                                idpelota.graph=0;
                                IF (equipo==0)
                                    angle=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                ELSE
                                    angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                END
                            END
                            CASE 1:
                                // Pone al jugador mirando hacia el desmarcado
                                IF (equipo==0)
                                    angle=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                ELSE
                                    angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                END
                                // Pone el gr fico 2 de la animaci¢n de saque de banda
                                xgraph=&tablag31;
                            END
                            CASE 2:
                                // Tira el bal¢n hacia el desmarcado
                                IF (equipo==0)
                                    idpelota.angulopelota=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                    idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y));
                                ELSE
                                    idpelota.angulopelota=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                    idpelota.velocidadpelota=coge_veloc(fget_dist(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y));
                                END
                                // Pone al gr fico mirando hacia el desmarcado
                                IF (equipo==0)
                                    angle=fget_angle(x,y,idjugadores1[pasa_a].x,idjugadores1[pasa_a].y);
                                ELSE
                                    angle=fget_angle(x,y,idjugadores2[pasa_a].x,idjugadores2[pasa_a].y);
                                END
                                // Pone el gr fico 2 de la animaci¢n de saque de banda
                                xgraph=&tablag32;

                                // Devuelve el gr fico al bal¢n
                                idpelota.graph=10;

                                // A la altura y con el bote apropiados
                                idpelota.height=90;
                                incactual=0; incbote=18;
                                IF (equipo==0)
                                    idjugadores1[pasa_a].estado=esperandopase;
                                    equipototal[0].esperandopase=idjugadores1[pasa_a];
                                ELSE
                                    idjugadores2[pasa_a].estado=esperandopase;
                                    equipototal[1].esperandopase=idjugadores2[pasa_a];

                                END

                            END
                        END
                    ++estadosaque; // Cambia el estado del saque
                    END
                    // Jugador que va a sacar el c¢rner
                    CASE iracorner:

                        // Dirige al jugador hacia la pelota que ya esta en el c¢rner
                        velocidad=fget_dist(x,y,idpelota.x,idpelota.y)/totaltestigo;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        angle=fget_angle(x,y,idpelota.x,idpelota.y);

                        // Cuando llegue a la pelota se cambia el estado a tirandocorner
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<=(vmaxima+200)/2)
                            velocidad=0;
                            estado=tirandocorner;
                            idpelota.graph=1;
                        END
                    END
                    // Estado del jugador que tira el c¢rner
                    CASE tirandocorner:

                        // Comprueba si se ha llegado a tirar el c¢rner si
                        // esta el estado del juego en normal(=4) o este
                        // en estado de haber acabado de tirarlo(>21)
                        IF (estadojuego==4 OR estadojuego>21)
                            // Pone animaci¢n de tiro
                            tirando=2;
                            // Pon al jugador en estado atontado
                            estado=atontado;
                            cuentaatontado=retardoatontado;
                        END

                        // Si sigue tirando el c¢rner
                        IF (estadojuego<>4)
                            // Pone al jugador mirando a la pelota dependiendo de la esquina
                            SWITCH(esquinacorner)
                                CASE 0: x=idpelota.x-400; y=idpelota.y-400; END
                                CASE 1: x=idpelota.x+400; y=idpelota.y-400; END
                                CASE 2: x=idpelota.x-400; y=idpelota.y+400; END
                                CASE 3: x=idpelota.x+400; y=idpelota.y+400; END
                            END
                        END
                        angle=get_angle(idpelota);
                    END

                    // Jugadores cuando van a la posici¢n de recibir el c¢rner
                    CASE irarecibecorner:

                        // Dependiendo de quien saque el c¢rner, el campo donde se saque
                        IF (quiensacacorner==1)     // Campo de arriba
                            // Y del equipo que sea
                            IF (equipo==0)
                                // Pone al jugador en su posici¢n correcta
                                xdest=(t ctica[n£mero].xc0+xicampo)*100;
                                ydest=(t ctica[n£mero].yc0+yicampo)*100;
                            ELSE
                                xdest=(t ctica[n£mero].xc1+xicampo)*100;
                                ydest=(t ctica[n£mero].yc1+yicampo)*100;
                            END
                        ELSE
                            // Si es el campo de abajo hay que espejar la t ctica
                            IF (equipo==0)
                                xdest=(xfcampo-tactica[n£mero].xc1)*100;
                                ydest=(yfcampo-tactica[n£mero].yc1)*100;
                            ELSE
                                xdest=(xfcampo-tactica[n£mero].xc0)*100;
                                ydest=(yfcampo-tactica[n£mero].yc0)*100;
                            END
                        END

                        // Se hace que vaya hacia su posici¢n
                        velocidad=fget_dist(x,y,xdest,ydest)/totaltestigo;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        angle=fget_angle(x,y,xdest,ydest);

                        // Si ha llegado a su posici¢n se cambia el estado a recibecorner
                        IF (fget_dist(x,y,xdest,ydest)<=(vmaxima+200)/2)
                            velocidad=0;
                            estado=recibecorner;
                        END
                    END
                END

            // Repite el proceso si se ha cambiado de estado
            until (estado_anterior==estado)
        END

        // Si esta esperan el bal¢n y pasa cerca lo coge
        IF (estado==esperandopase && fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida AND idpelota.height==0)

            // Si hab¡a alguien desmarcado se quita
            IF (equipototal[equipo].desmarcado<>0)
                (equipototal[equipo].desmarcado).estado=normal;
            END
            equipototal[equipo].desmarcado=0;
            IF (equipototal[0].conbalon<>0)
                (equipototal[0].conbalon).estado=atontado;
                (equipototal[0].conbalon).cuentaatontado=retardoatontado;
                equipototal[0].conbalon=0;
            END

            // Si hab¡a alguien con el bal¢n se le quita
            IF (equipototal[1].conbalon<>0)
                (equipototal[1].conbalon).estado=atontado;
                (equipototal[1].conbalon).cuentaatontado=retardoatontado;
                equipototal[1].conbalon=0;
            END

            // Pone al jugador en estado conbalon y quita esperandopase
            equipototal[equipo].conbalon=id;
            equipototal[equipo].conbalon.estado=conbalon;
            equipototal[equipo].esperandopase=0;
        END

        // Hace que avance el jugador en el  ngulo en que este
        advance(velocidad);

        // Si el estado del jugador no es de fuera de banda
        IF (estado<>saquedebanda)

            // Si a cogido el bal¢n o esta muy lejos la pelota quita el indicador de tir ndose
            IF (estado==conbalon OR fget_dist(x,y,idpelota.x,idpelota.y)>zonaquitada)
                tirandose=0;
            END

            // Si no se mueve y no se disparando el bal¢n
            IF (velocidad==0 AND tirando==0)
                // Pon el gr fico de parado
                xgraph=&tablag1;
            ELSE
                // Si esta tir ndose pon el gr fico de tir ndose
                IF (tirandose==1)
                    xgraph=&tablag20;
                ELSE
                    // Si esta tirando el bal¢n pon la animaci¢n
                    IF (tirando>0)
                        IF (tirando==2)
                            xgraph=&tablag40;
                        ELSE
                            xgraph=&tablag41;
                        END
                        --tirando;
                    ELSE
                        // Y si no pon la animaci¢n de corriendo
                        SWITCH(cuentaanima)
                            CASE 0: xgraph=&tablag10;END
                            CASE 1: xgraph=&tablag11;END
                            CASE 2: xgraph=&tablag12;END
                            CASE 3: xgraph=&tablag13;END
                            CASE 4: xgraph=&tablag14;END
                            CASE 5: xgraph=&tablag15;END
                            CASE 6: xgraph=&tablag16;END
                            CASE 7: xgraph=&tablag17;END
                        END
                        ++cuentaanima;IF (cuentaanima>7) cuentaanima=0; END
                    END
                END
            END

            // Si tiene el bal¢n hace que este le siga
            IF (estado==conbalon)
                idpelota.x=x+get_distx(angle,300);
                idpelota.y=y+get_disty(angle,300);
            END
        END
        FRAME;
   END
END

//-------------------------------------------------------------------
// Proceso controlaequipo
// Maneja los desmarcado, los que est n mas cerca de la pelota de los equipos
// Entradas: cual = N£mero de equipo
//-------------------------------------------------------------------

PROCESS controlaequipo(cual)

PRIVATE

    mascerca;   // N£mero del que esta mas cerca
    pospelox;   // Coordenadas x e y de donde estar  la pelota
    pospeloy;

BEGIN
    graph=0;    // Este proceso no tiene gr fico
    timer[1]=0;
    LOOP
        // Si el juego y el equipo est n en estado normal
        IF (estadojuego==4 AND estadoequipo==1)

            // Y le toca el turno de moverse
            IF (testigo3==cual)

                // Si el equipo no tiene el bal¢n
                IF (equipototal[cual].conbalon==0 )
                    // Si hab¡a alg£n desmarcado lo quita y lo pone en estado normal
                    IF (cual==0)
                        IF (equipototal[cual].desmarcado<>0)
                            (equipototal[cual].desmarcado).estado=normal;
                            equipototal[cual].desmarcado=0;
                        END
                    ELSE
                        IF (timer[1]>cambiodesmarcado)
                            IF (equipototal[cual].desmarcado<>0)
                                (equipototal[cual].desmarcado).estado=normal;
                                equipototal[cual].desmarcado=0;
                            END
                        END
                    END

                    // Mira cual de los equipos es
                    IF (cual==0)

                        // Halla los coordenadas de la pelota
                        pospelox=da_pos_balon(0,cual);
                        pospeloy=da_pos_balon(1,cual);

                        // Presupone que el que esta mas cerca es el n£mero 0
                        mascerca=0;

                        // Va mirando los dem s n£meros de 1 al 9
                        FOR (contador=1;contador<10;++contador)
                            // Si no esta en el modo normal se salta a ese jugador
                            IF (idjugadores1[contador].estado<>normal)
                                contador++;
                            ELSE
                                // Comprueba si supera al ultimo que estaba mas cerca
                                IF(fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,pospelox,pospeloy)<
                                   fget_dist(idjugadores1[mascerca].x,idjugadores1[mascerca].y,pospelox,pospeloy))
                                    // Y si esta se pone el
                                    mascerca=contador;
                                END
                            END
                        END

                        // Pone al jugador que este mas cerca del bal¢n en desmarcado
                        idjugadores1[mascerca].estado=desmarcado;
                        equipototal[0].desmarcado=idjugadores1[mascerca];
                        equipototal[0].distanciaminima=fget_dist(idjugadores1[mascerca].x,idjugadores1[mascerca].y,pospelox,pospeloy);
                    ELSE

                        // Realiza el mismo proceso anterior con el otro equipo
                        pospelox=da_pos_balon(0,cual);
                        pospeloy=da_pos_balon(1,cual);
                        mascerca=0;
                        FOR (contador=1;contador<10;++contador)
                            IF (idjugadores2[contador].estado<>normal)
                                contador++;
                            ELSE
                                IF(fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,pospelox,pospeloy)<
                                   fget_dist(idjugadores2[mascerca].x,idjugadores2[mascerca].y,pospelox,pospeloy))
                                    mascerca=contador;
                                END
                            END
                        END
                        IF (controljugador==2)
                            idjugadores2[mascerca].estado=desmarcado;
                            equipototal[1].desmarcado=idjugadores2[mascerca];
                            equipototal[1].distanciaminima=fget_dist(idjugadores2[mascerca].x,idjugadores2[mascerca].y,pospelox,pospeloy);
                        ELSE
                            IF (timer[1]>cambiodesmarcado)
                                idjugadores2[mascerca].estado=desmarcado;
                                equipototal[1].desmarcado=idjugadores2[mascerca];
                                equipototal[1].distanciaminima=fget_dist(idjugadores2[mascerca].x,idjugadores2[mascerca].y,pospelox,pospeloy);
                                timer[1]=0;
                            END
                        END
                    END
                END
            END

            // Actualiza la variable de equipo que designa al desmarcado
            IF (equipototal[cual].desmarcado<>0)
                (equipototal[cual].desmarcado).estado=desmarcado;
            END
            IF (equipototal[cual].conbalon<>0)
                (equipototal[cual].conbalon).estado=conbalon;
            END

        END

        // Actualiza el estado de ataque y defensa de los dos equipos
        equipototal[0].defensa=100-estadoataque;
        equipototal[1].defensa=estadoataque;
        equipototal[0].ataque=estadoataque;
        equipototal[1].ataque=100-estadoataque;

        // Si alg£n equipo tiene el bal¢n se incrementa la fuerza de ataque
        IF (equipototal[0].conbalon<>0)
            (equipototal[0].ataque)+=10;
        END
        IF (equipototal[1].conbalon<>0)
            (equipototal[1].ataque)+=10;
        END

        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso flecha
// Pone el gr fico que se¤ala al desmarcado en el juego
// Entradas: tipo = N£mero de equipo
//-------------------------------------------------------------------

PROCESS flecha(tipo)

BEGIN
    // Pone el gr fico necesario
    file=fichero1;
    graph=4;

    // Usa dos decimales como los jugadores
    resolution=100;

    // Lo pone dentro de modo 7 y por detraes del jugador
    ctype=2;
    z=20;

    // Hace que sea transparente
    flags=4;
    LOOP
        // Si el equipo tiene el bal¢n hace que le siga la flecha
        IF (equipototal[tipo].conbalon<>0)
            x=equipototal[tipo].conbalon.x;
            y=equipototal[tipo].conbalon.y-300;
            graph=4;
        ELSE
            // Si no mira si hay un desmarcado y le sigue la flecha
            IF (equipototal[tipo].desmarcado<>0)
                x=equipototal[tipo].desmarcado.x;
                y=equipototal[tipo].desmarcado.y-300;
                graph=4;
            ELSE
                // Sino quita el gr fico
                graph=0;
            END
        END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso coge_veloc
// Transforma la distancia en una velocidad
// Entradas: dist = Distancia
// Salidas: n = Velocidad
//-------------------------------------------------------------------

PROCESS coge_veloc(dist)

PRIVATE
    n=100;      // Guarda la velocidad necesaria para llegar a un distancia
    incdist=0;  // Distancia que lleva en cada momento

BEGIN

    // Repite hasta que no se supere la distancia pedida
    WHILE (incdist<dist)
        incdist+=n;     // Se suma la velocidad a la distancia relativa
        n+=incrvelocidadpelota;         // Y se incrementa
    END
    RETURN (n);         // Retorna la velocidad obtenida
END

//-------------------------------------------------------------------
// Proceso da_pos_balon
// Da la posici¢n futura del bal¢n cuando este se esta moviendo
// Entradas: cual       = Cual de las coordenadas de pide, x o y
//           cualequipo = N£mero del equipo que quiere las coordenadas
// Salidas: posx = Posici¢n x donde estar  la pelota
//          posy = Posici¢n y donde estar  la pelota
//-------------------------------------------------------------------

PROCESS da_pos_balon(cual,cualequipo)

PRIVATE
    posx;   // Coordenadas de la posici¢n de donde quedara el bal¢n
    posy;
    velo;   // Guarda la velocidad del bal¢n en cada momento

BEGIN
    // Comprueba si el equipo contrario tiene el bal¢n y mira la direcci¢n
    // y el  ngulo de quien la lleva para hallar as¡ la posici¢n
    IF (cualequipo==0 AND equipototal[1].conbalon<>0)
        posx=(equipototal[1].conbalon).x+get_distx((equipototal[1].conbalon).angle,(equipototal[1].conbalon).velocidad*totaltestigo);
        posy=(equipototal[1].conbalon).y+get_disty((equipototal[1].conbalon).angle,(equipototal[1].conbalon).velocidad*totaltestigo);
    ELSE
        IF (cualequipo==1 AND equipototal[0].conbalon<>0)
            posx=(equipototal[0].conbalon).x+get_distx((equipototal[0].conbalon).angle,(equipototal[0].conbalon).velocidad*totaltestigo);
            posy=(equipototal[0].conbalon).y+get_disty((equipototal[0].conbalon).angle,(equipototal[0].conbalon).velocidad*totaltestigo);
        ELSE
            // Si no, coge las variables de la pelota
            posx=idpelota.x;
            posy=idpelota.y;

            // Y la velocidad
            velo=idpelota.velocidadpelota;

            // Y repite hasta que se quede si velocidad
            WHILE (velo==0)
                posx+=get_distx(idpelota.angulopelota,velo);
                posy+=get_disty(idpelota.angulopelota,velo);
                velo-=100;
            END
        END
    END

    // Retorna la posici¢n que se haya pedido
    IF (cual==0)
        RETURN(posx);
    ELSE
        RETURN(posy);
    END

END

//-------------------------------------------------------------------
// Proceso minicampo
// Imprime el minicampo que aparece de marcador en el juego
//-------------------------------------------------------------------

PROCESS minicampo()

BEGIN

    // Selecciona el gr fico dependiendo del modo del v¡deo
    file=fichero1;
    IF (modovideo==0)
        graph=104;
    ELSE
        graph=108;
    END

    // Pone las coordenadas dependiendo del modo de v¡deo
    x=572-(286*modovideo);
    y=108-(54*modovideo);
    z=-10;

    // Pinta los puntos de los jugadores y de la pelota
    FOR (contador=0;contador<23;contador++)
        pintapunto(contador);
    END
    LOOP
        IF(conmarcadores==0)
            graph=0;
        ELSE
            IF (modovideo==0)
                graph=104;
            ELSE
                graph=108;
            END
        END
        FRAME;
    END

END

//-------------------------------------------------------------------
// Proceso pintapunto
// Pinta los puntos de los jugadores y el bal¢n en el minimapa
// Entradas: cual = n£mero del punto que se quiere pintar
//-------------------------------------------------------------------

PROCESS pintapunto(cual)

BEGIN
    // Elige el gr fico adecuado
    file=fichero1;

    // Lo pone por encima del minicampo
    z=-12;

    // El tama¤o seg£n el modo de v¡deo
    LOOP
        IF(conmarcadores==0)
            graph=0;
        ELSE
            SWITCH (cual)
                CASE 0..9,21:
                    IF (modovideo==0)
                        graph=106;
                    ELSE
                        graph=109;
                    END
                END
                CASE 0..19,22:
                    IF (modovideo==0)
                        graph=107;
                    ELSE
                        graph=110;
                    END
                END
                CASE 20:graph=105: END
            END
        END

        // Mira de donde tiene que coger las coordenadas
        SWITCH (cual)
            CASE 0..9:      // Jugador del equipo 1
                x=((idjugadores1[cual].x-(xicampo*100))/(500+(500*modovideo)))+517-(261*modovideo);
                y=(idjugadores1[cual].y-(yicampo*100))/(500+(500*modovideo))+(10-(4*modovideo));
            END
            CASE 10..19:    // Jugador del equipo 2
                x=((idjugadores2[cual-10].x-(xicampo*100))/(500+(500*modovideo)))+517-(261*modovideo);
                y=(idjugadores2[cual-10].y-(yicampo*100))/(500+(500*modovideo))+(10-(4*modovideo));
            END
            CASE 20:        // Pelota
                x=((idpelota.x-(xicampo*100))/(500+(500*modovideo)))+517-(261*modovideo);
                y=(idpelota.y-(yicampo*100))/(500+(500*modovideo))+(10-(4*modovideo));
            END
            CASE 21:        // Portero del equipo 1
                    x=((idporteros[0].x-(xicampo*100))/(500+(500*modovideo)))+517-(261*modovideo);
                    y=(idporteros[0].y-(yicampo*100))/(500+(500*modovideo))+(10-(4*modovideo));
            END
            CASE 22:        // Portero del equipo 2
                    x=((idporteros[1].x-(xicampo*100))/(500+(500*modovideo)))+517-(261*modovideo);
                    y=(idporteros[1].y-(yicampo*100))/(500+(500*modovideo))+(10-(4*modovideo));
            END
        END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso sombrajugador
// Pone el gr fico de la sombra del jugador
//-------------------------------------------------------------------

PROCESS sombrajugador()

PRIVATE

    // Tabla con las gr ficos de las sombras

    tablag1[]=8,67,68,69,70,71,72,65,66;    // Posici¢n parado

    tablag10[]=8,17,25,33,113,105, 97,1, 9; // Posici¢n corriendo
    tablag11[]=8,18,26,34,114,106, 98,2,10;
    tablag12[]=8,19,27,35,115,107, 99,3,11;
    tablag13[]=8,20,28,36,116,108,100,4,12;
    tablag14[]=8,21,29,37,117,109,101,5,13;
    tablag15[]=8,22,30,38,118,110,102,6,14;
    tablag16[]=8,23,31,39,119,111,103,7,15;
    tablag17[]=8,24,32,40,120,112,105,8,16;

    tablag20[]=8,57,58,59,60,61,62,63,64;   // Posici¢n tir ndose

    tablag30[]=8,51,52,53,54,55,56,49,50;   // Posici¢n sacando de banda
    tablag31[]=8,43,44,45,46,47,48,41,42;
    tablag32[]=8,75,76,77,78,79,80,73,74;

    tablag40[]=8,83,84,85,86,87,88,81,82;   // Posici¢n tirando
    tablag41[]=8,91,92,93,94,95,96,89,90;


BEGIN
    // Pone en el modo 7 y elige el fichero de gr ficos
    ctype=2;
    file=fichero7;

    // Hace que sea transparente
    flags=4;

    // Usa dos decimales como los jugadores
    resolution=100;

    // Pone por detraes de los jugadores
    z=(father.z)+10;
    LOOP

        // Coge el  ngulo y las coordenadas del jugador
        angle=father.angle;
        x=father.x;
        y=father.y-2;   // Adelanta la coordenada y para que se vea por debajo
        // Si no esta sacando de banda
        IF (father.estado<>saquedebanda)
            // Si la velocidad es cero y el jugador no esta disparando
            IF (father.velocidad==0 && father.tirando==0)
                xgraph=&tablag1;        // Pone el gr fico de parado
            ELSE
                // Si esta tir ndose a por la pelota
                IF (father.tirandose==1)
                    xgraph=&tablag20;   // Pone el gr fico de tir ndose
                ELSE
                    // Si esta disparando el bal¢n realiza la animaci¢n
                    IF (father.tirando>0)
                        IF (father.tirando==2)
                            xgraph=&tablag40;
                        ELSE
                            xgraph=&tablag41;
                        END
                    ELSE
                        // Si no es que esta corriendo y realiza la animaci¢n
                        SWITCH(father.cuentaanima)
                            CASE 0: xgraph=&tablag10;END
                            CASE 1: xgraph=&tablag11;END
                            CASE 2: xgraph=&tablag12;END
                            CASE 3: xgraph=&tablag13;END
                            CASE 4: xgraph=&tablag14;END
                            CASE 5: xgraph=&tablag15;END
                            CASE 6: xgraph=&tablag16;END
                            CASE 7: xgraph=&tablag17;END
                        END
                    END
                END
            END
        ELSE    // Entonces es que esta sacando de banda y realiza la animaci¢n
            SWITCH (estadosaque)
            CASE 0: xgraph=&tablag30; END
            CASE 1: xgraph=&tablag31; END
            CASE 2: xgraph=&tablag32; END
            END
        END
        FRAME;
    END

END

//-------------------------------------------------------------------
// Proceso controlajuego
// Maneja los estados de juego, como c¢rners saque de banda, saque
// de gol, saque de porter¡a, al inicio del juego ...
//-------------------------------------------------------------------

PROCESS controlajuego ()

PRIVATE
    cuentaparados;  // Cuenta los que est n en un determinado proceso
    pasaa;          // Guarda el n£mero al que se le hacen los pases
    mascerca;       // Guarda el n£mero del que esta mas cerca del bal¢n
    xtemp,ytemp;    // Guarda temporalmente los puntos del control del portero
    iniciosaque;    // Sirve para pasar del primero si es este el que saca
BEGIN
    graph=0;        // Este proceso no tiene gr fico
    LOOP

        SWITCH (estadojuego)
            // Estados 1-3 -> Cuando se saca del centro
            CASE 1:
                IF (quienempieza==1 AND controljugador<>2)
                    elegidosaque=8;
                    elige_saque(9);
                    FRAME;
                    pasaa=elegidosaque;
                ELSE
                    IF (quienempieza==1)
                        pasaa=daquienpasa(1);
                    END
                END
                ponmensajes (0,20); // Pone el mensaje de comienzo

                // Pone el tiempo a cero dependiendo de la parte en que este
                IF (inicioparte==0)
                    inicioparte=1;
                    timer[0]=0;
                END
                IF (inicioparte==3)
                    inicioparte=4;
                    timer[0]=0;
                END
                sound(idsonido3,50,256); // Realiza sonido de publico al principio

                // Mira quien empieza el partido y pasa a su jugador n£mero 9
                IF (quienempieza==0)
                    idpelota.angulopelota=fget_angle(idpelota.x,idpelota.y,idjugadores1[9].x,idjugadores1[9].y);
                    idpelota.velocidadpelota=coge_veloc(fget_dist(idpelota.x,idpelota.y,idjugadores1[9].x,idjugadores1[9].y));
                    idjugadores1[8].tirando=2;
                ELSE
                    idpelota.angulopelota=fget_angle(idpelota.x,idpelota.y,idjugadores2[9].x,idjugadores2[9].y);
                    idpelota.velocidadpelota=coge_veloc(fget_dist(idpelota.x,idpelota.y,idjugadores2[9].x,idjugadores2[9].y));
                    idjugadores2[8].tirando=2;
                END

                estadojuego=2;
            END
            CASE 2:
                // Mira quien empieza
                IF (quienempieza==0)
                    // Y si el jugador n£mero 9 le ha llegado la pelota
                    IF (fget_dist(idjugadores1[9].x,idjugadores1[9].y,idpelota.x,idpelota.y)<zonarecogida)
                        sound(idsonido3,50,256);    // Hace sonido de chutar
                        // Pone a todos los jugadores en estado normal
                        FOR (contador=0;contador<10;++contador)
                            idjugadores1[contador].estado=normal;
                            idjugadores2[contador].estado=normal;
                        END
                        pasaa=daquienpasa(0);

                        // Y le pasa la pelota
                        idpelota.angulopelota=fget_angle(idjugadores1[9].x,idjugadores1[9].y,idjugadores1[pasaa].x,idjugadores1[pasaa].y);
                        idjugadores1[9].angle=fget_angle(idjugadores1[9].x,idjugadores1[9].y,idjugadores1[pasaa].x,idjugadores1[pasaa].y);
                        idpelota.velocidadpelota=coge_veloc(fget_dist(idjugadores1[9].x,idjugadores1[9].y,idjugadores1[pasaa].x,idjugadores1[pasaa].y));

                        // Comprueba si la lanza con bote
                        IF (idpelota.velocidadpelota>conbote)
                            incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                        END

                        // Pone al jugador al que pasa en estado esperando pase
                        idjugadores1[pasaa].estado=esperandopase;
                        idjugadores1[pasaa].velocidad=0;
                        equipototal[0].esperandopase=idjugadores1[pasaa];
                        estadojuego=3;
                        idjugadores1[9].tirando=2;
                    END
                ELSE
                    // Hace lo mismo pero con el otro equipo
                    IF (fget_dist(idjugadores2[9].x,idjugadores2[9].y,idpelota.x,idpelota.y)<zonarecogida)
                        sound(idsonido3,50,256);
                        FOR (contador=0;contador<10;++contador)
                            idjugadores1[contador].estado=normal;
                            idjugadores2[contador].estado=normal;
                        END
                        idpelota.angulopelota=fget_angle(idjugadores2[9].x,idjugadores2[9].y,idjugadores2[pasaa].x,idjugadores2[pasaa].y);
                        idpelota.velocidadpelota=coge_veloc(fget_dist(idjugadores2[9].x,idjugadores2[9].y,idjugadores2[pasaa].x,idjugadores2[pasaa].y));
                        idjugadores2[9].angle=fget_angle(idjugadores2[9].x,idjugadores2[9].y,idjugadores2[pasaa].x,idjugadores2[pasaa].y);
                        IF (idpelota.velocidadpelota>conbote)
                            incactual=(idpelota.velocidadpelota-conbote)/50; incbote=(idpelota.velocidadpelota-conbote)/50; idpelota.height=0;
                        END

                        idjugadores2[pasaa].estado=esperandopase;
                        idjugadores2[pasaa].velocidad=0;
                        equipototal[1].esperandopase=idjugadores2[pasaa];
                        estadojuego=3;
                        idjugadores2[9].tirando=2;
                    END
                END
            END
            CASE 3:
                estadoequipo=1; // Pone a los dos equipos movi‚ndose
                estadojuego=4;  // Y el estado de juego en normal
            END
            // Estado 5-> Cuando se mete gol, luego va a los estados de 1 a 3
            CASE 5:
                // Reinicia la pelota
                idpelota.x=(((xfcampo-xicampo)/2)+xicampo)*100;
                idpelota.y=(((yfcampo-yicampo)/2)+yicampo)*100;
                idpelota.velocidadpelota=0; idpelota.angulopelota=0;
                idpelota.height=0; incactual=0; incbote=0;

                // Pone a los jugadores en sus posiciones iniciales, seg£n quien empiece
                FOR (contador=0;contador<10;contador++)
                    (idjugadores1[contador]).estado=parado;
                    (idjugadores2[contador]).estado=parado;
                     IF (quienempieza==0)
                        (idjugadores1[contador]).xi=(t ctica[contador].xi0+xicampo)*100;
                        (idjugadores1[contador]).yi=(t ctica[contador].yi0+yicampo)*100;
                        (idjugadores2[contador]).xi=(xfcampo-tactica[contador].xi1)*100;
                        (idjugadores2[contador]).yi=(yfcampo-tactica[contador].yi1)*100;
                     ELSE
                        (idjugadores1[contador]).xi=(t ctica[contador].xi1+xicampo)*100;
                        (idjugadores1[contador]).yi=(t ctica[contador].yi1+yicampo)*100;
                        (idjugadores2[contador]).xi=(xfcampo-tactica[contador].xi0)*100;
                        (idjugadores2[contador]).yi=(yfcampo-tactica[contador].yi0)*100;
                     END
                END

                // Pone a 0 las variables de equipo
                equipototal[0].conbalon=0;
                equipototal[1].conbalon=0;
                equipototal[0].desmarcado=0;
                equipototal[1].desmarcado=0;
                equipototal[0].esperandopase=0;
                equipototal[1].esperandopase=0;

                // Hace que se saque del centro, manda a estados 1-3
                estadojuego=0;
                estadoequipo=0;
            END

            CASE 10:    // Estados 10-11 -> Cuando se saca de banda
                IF (sacando==0) // Hace que el proceso se ejecute solo una vez
                    // Si hubiera alg£n desmarcado, lo quita
                    IF (equipototal[0].desmarcado<>0 )
                        (equipototal[0].desmarcado).estado=normal;
                    END
                    IF (equipototal[1].desmarcado<>0 )
                        (equipototal[1].desmarcado).estado=normal;
                    END
                    // Si hubiera alguien con bal¢n, lo quita
                    IF (equipototal[0].conbalon<>0 )
                        (equipototal[0].conbalon).estado=normal;
                    END
                    IF (equipototal[1].conbalon<>0 )
                        (equipototal[1].conbalon).estado=normal;
                    END
                    // Si hubiera alguien esperando pase. lo quita
                    IF (equipototal[0].esperandopase<>0 )
                        (equipototal[0].esperandopase).estado=normal;
                    END
                    IF (equipototal[1].esperandopase<>0 )
                        (equipototal[1].esperandopase).estado=normal;
                    END

                    //Pone las variables del equipo a 0
                    equipototal[0].desmarcado=0;
                    equipototal[1].desmarcado=0;
                    equipototal[0].conbalon=0;
                    equipototal[1].conbalon=0;
                    equipototal[0].esperandopase=0;
                    equipototal[1].esperandopase=0;

                    // Pone a todos los jugadores en estado normal
                    FOR (contador=1;contador<10;++contador)
                        idjugadores1[mascerca].estado=normal;
                        idjugadores2[mascerca].estado=normal;
                    END

                    // Busca el que este mas cerca de la pelota y hace que vaya a sacar
                    IF (quiensacadebanda==0)
                        mascerca=0;
                        FOR (contador=1;contador<10;++contador)
                            IF (fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,idpelota.x,idpelota.y)<
                                fget_dist(idjugadores1[mascerca].x,idjugadores1[mascerca].y,idpelota.x,idpelota.y))
                                mascerca=contador;
                            END
                        END
                        equipototal[quiensacadebanda].saquedebanda=idjugadores1[mascerca];
                        idjugadores1[mascerca].estado=irasaque;
                    ELSE
                        mascerca=0;
                        FOR (contador=1;contador<10;++contador)
                            IF (fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,idpelota.x,idpelota.y)<
                                fget_dist(idjugadores2[mascerca].x,idjugadores2[mascerca].y,idpelota.x,idpelota.y))
                                mascerca=contador;
                            END
                        END
                        equipototal[quiensacadebanda].saquedebanda=idjugadores2[mascerca];
                        idjugadores2[mascerca].estado=irasaque;
                    END
                    // Para la pelota
                    idpelota.velocidadpelota=0;
                    idpelota.angulopelota=0;
                    idpelota.height=0;
                    // Pasa al siguiente estado de saque de banda
                    estadojuego=11;
                    estadosaque=0;
                    sacando=1;
                END
            END
            CASE 11:
                IF (estadosaque>2)  // Espera a que acabe de sacar
                    pesopelota=3;   // Pon la pelota con su peso original (peso=acercamiento a la pelota)

                    // Pone al jugador que sacaba en atontado
                    IF (equipototal[0].saquedebanda<>0)
                        (equipototal[0].saquedebanda).estado=atontado;
                        (equipototal[0].saquedebanda).cuentaatontado=retardoatontado;
                         equipototal[0].saquedebanda=0;
                    END
                    IF (equipototal[1].saquedebanda<>0)
                        (equipototal[1].saquedebanda).estado=atontado;
                        (equipototal[1].saquedebanda).cuentaatontado=retardoatontado;
                         equipototal[1].saquedebanda=0;
                    END
                    // Pone el estado de juego en normal
                    estadojuego=4;
                    sacando=0;
                END
            END

            // Estados 15 y 18 -> Sacando de porter¡a
            CASE 15:
                IF (sacandoporteria==1)
                    // Quita a los desmarcados, con bal¢n y esperando pase que hubiera
                    equipototal[0].desmarcado=0;
                    equipototal[1].desmarcado=0;
                    equipototal[0].conbalon=0;
                    equipototal[1].conbalon=0;
                    equipototal[0].esperandopase=0;
                    equipototal[1].esperandopase=0;
                    FROM contador=0 TO 9;
                        idjugadores1[contador].estado=parado;
                        idjugadores2[contador].estado=parado;
                    END
                    estadoequipo=0;
                    estadojuego=16;
                    sacandoporteria=2;
                    // Coge los puntos de control( en manos) del portero y le pone la pelota
                    IF (porterosacando==0)
                        get_point(fichero4,idporteros[porterosacando].graph,1,&xtemp,&ytemp);
                        idpelota.y=idporteros[porterosacando].y-100;
                    ELSE
                        get_point(fichero5,idporteros[porterosacando].graph,1,&xtemp,&ytemp);
                        idpelota.y=idporteros[porterosacando].y+100;
                    END
                    idpelota.x=(idporteros[porterosacando].x+((xtemp-50)*150));
                    idpelota.height=(105-ytemp);
                END
            END
            CASE 17:
                IF (porterosacando<>1 AND controljugador<>2 AND sacandoporteria==3)
                    elegidosaque=0;
                    FROM contador=0 TO 9;
                        IF (fget_dist(idpelota.x,idpelota.y,idjugadores2[contador].x,idjugadores2[contador].y)<
                            fget_dist(idpelota.x,idpelota.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y))
                            elegidosaque=contador;
                        END
                    END
                    elige_saque(10);
                    FRAME;
                    pasaa=elegidosaque;
                END
                sacandoporteria=4;
            END
            CASE 18:
                IF (sacandoporteria==4)
                    estadojuego=19;
                    sacandoporteria=5;
                END
            END

            CASE 19:
                IF (sacandoporteria==5);
                    // Pone a los jugadores en estado normal
                    FROM contador=0 TO 9;
                        idjugadores1[contador].estado=normal;
                        idjugadores2[contador].estado=normal;
                    END

                    IF (porterosacando==1)      // Mira que portero saca
                        // Busca un jugador para hacerle un pase
                        pasaa=daquienpasa(0);
                        // Le pasa la pelota y lo pone en estado esperando pase
                        idpelota.angulopelota=fget_angle(idpelota.x,idpelota.y,idjugadores1[pasaa].x,idjugadores1[pasaa].y);
                        idpelota.velocidadpelota=coge_veloc(fget_dist(idpelota.x,idpelota.y,idjugadores1[pasaa].x,idjugadores1[pasaa].y));
                        idjugadores1[pasaa].estado=esperandopase;
                        equipototal[0].esperandopase=idjugadores1[pasaa];
                    ELSE    // Igual proceso para el otro portero
                        IF (controljugador==2)
                            pasaa=daquienpasa(1);
                        END
                        idpelota.angulopelota=fget_angle(idpelota.x,idpelota.y,idjugadores2[pasaa].x,idjugadores2[pasaa].y);
                        idpelota.velocidadpelota=coge_veloc(fget_dist(idpelota.x,idpelota.y,idjugadores2[pasaa].x,idjugadores2[pasaa].y));
                        idjugadores2[pasaa].estado=esperandopase;
                        equipototal[1].esperandopase=idjugadores2[pasaa];
                    END
                    // Pone el estado de juego y el del portero en normal
                    estadojuego=4;
                    estadoequipo=1;
                    idporteros[porterosacando].estado=normal;
                    // Pone bote a la pelota
                    idpelota.height=60;incactual=-12; incbote=18;
                    cuenporterosacando=0;
                    sacandoporteria=0;
                    idpelota.x+=get_distx(idpelota.angulopelota,idpelota.velocidadpelota);
                    idpelota.y+=get_disty(idpelota.angulopelota,idpelota.velocidadpelota);

                END
            END

            // Estados 20 a 23 -> C¢rners
            CASE 20:
                IF (sacandocorner==0)   // Hace que se ejecute solo una vez
                    SWITCH (esquinacorner)
                        CASE 0: idpelota.x=xicampo*100;idpelota.y=yicampo*100; END
                        CASE 1: idpelota.x=xfcampo*100;idpelota.y=yicampo*100; END
                        CASE 2: idpelota.x=xicampo*100;idpelota.y=yfcampo*100; END
                        CASE 3: idpelota.x=xfcampo*100;idpelota.y=yfcampo*100; END
                    END

                    // Si hay alg£n desmarcado, con bal¢n o esperando pase lo quita
                    IF (equipototal[0].desmarcado<>0 )
                        (equipototal[0].desmarcado).estado=normal;
                    END
                    IF (equipototal[1].desmarcado<>0 )
                        (equipototal[1].desmarcado).estado=normal;
                    END
                    IF (equipototal[0].conbalon<>0 )
                        (equipototal[0].conbalon).estado=normal;
                    END
                    IF (equipototal[1].conbalon<>0 )
                        (equipototal[1].conbalon).estado=normal;
                    END
                    IF (equipototal[0].esperandopase<>0 )
                        (equipototal[0].esperandopase).estado=normal;
                    END
                    IF (equipototal[1].esperandopase<>0 )
                        (equipototal[1].esperandopase).estado=normal;
                    END

                    // Pone a 0 la variables de equipo
                    equipototal[0].desmarcado=0;
                    equipototal[1].desmarcado=0;
                    equipototal[0].conbalon=0;
                    equipototal[1].conbalon=0;
                    equipototal[0].esperandopase=0;
                    equipototal[1].esperandopase=0;

                    // Pone a los porteros en estado recibecorner
                    idporteros[0].estado=recibecorner;
                    idporteros[1].estado=recibecorner;

                    // Para la pelota
                    idpelota.velocidadpelota=0;
                    idpelota.angulopelota=0;
                    idpelota.height=0;

                    // Dependiendo de que quien saque el c¢rner
                    IF (quiensacacorner==0)
                        // Busca al jugador que este mas cerca de la pelota saque el c¢rner
                        mascerca=0;
                        FOR (contador=1;contador<10;++contador)
                            IF (fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,idpelota.x,idpelota.y)<
                                fget_dist(idjugadores1[mascerca].x,idjugadores1[mascerca].y,idpelota.x,idpelota.y))
                                mascerca=contador;
                            END
                        END
                        idjugadores1[mascerca].estado=iracorner;
                        equipototal[0].sacacorner=idjugadores1[mascerca];
                        // Y a los dem s jugadores a recibirlo
                        FOR (contador=0;contador<10;++contador)
                            IF (idjugadores1[contador].estado<>iracorner)
                                idjugadores1[contador].estado=irarecibecorner;
                            END
                            idjugadores2[contador].estado=irarecibecorner;
                        END
                    ELSE        // Hace lo mismo pero con el otro equipo
                        mascerca=0;
                        FOR (contador=1;contador<10;++contador)
                            IF (fget_dist(idjugadores1[contador].x,idjugadores2[contador].y,idpelota.x,idpelota.y)<
                                fget_dist(idjugadores1[mascerca].x,idjugadores2[mascerca].y,idpelota.x,idpelota.y))
                                mascerca=contador;
                            END
                        END
                        idjugadores2[mascerca].estado=iracorner;
                        equipototal[1].sacacorner=idjugadores2[mascerca];
                        FOR (contador=0;contador<10;++contador)
                            IF (idjugadores2[contador].estado<>iracorner)
                                idjugadores2[contador].estado=irarecibecorner;
                            END
                            idjugadores1[contador].estado=irarecibecorner;
                       END
                    END
                    // Pasa al siguiente estado de juego en c¢rner
                    estadojuego=21;
                    sacandocorner=1;
                END
            END
            CASE 21:
                IF (sacandocorner==1) // Hace que se espere a que acaben
                    // Comprueba si est n todos en su posici¢n
                    cuentaparados=0;
                    FOR (contador=0;contador<10;contador++)
                        IF (idjugadores1[contador].estado==recibecorner)
                            cuentaparados++;
                        END
                        IF (idjugadores1[contador].estado==tirandocorner)
                            cuentaparados++;
                        END

                        IF (idjugadores2[contador].estado==recibecorner)
                            cuentaparados++;
                        END
                        IF (idjugadores2[contador].estado==tirandocorner)
                            cuentaparados++;
                        END
                    END
                    // Cuando est n todos en su posici¢n pasa al siguiente estado de c¢rner
                    IF (cuentaparados==20) sacandocorner=2; estadojuego=22; END
                END
            END
            CASE 22:
                IF (sacandocorner==2)
                    sound(idsonido3,50,256);
                    IF (quiensacacorner==1 AND controljugador<2)
                        IF (equipototal[1].sacacorner.numero==0)
                            iniciosaque=2;
                            elegidosaque=1;
                        ELSE
                            iniciosaque=1;
                            elegidosaque=0;
                        END
                        FOR (contador=iniciosaque;contador<10;contador++)
                            IF (contador<>equipototal[1].sacacorner.numero)
                                IF (fget_dist(idpelota.x,idpelota.y,idjugadores2[contador].x,idjugadores2[contador].y)<
                                    fget_dist(idpelota.x,idpelota.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y))
                                    elegidosaque=contador;
                                END
                            END
                        END
                        elige_saque((equipototal[1].sacacorner).numero);
                        FRAME;
                        idpelota.angulopelota=fget_angle(idpelota.x,idpelota.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y);
                        idpelota.velocidadpelota=coge_veloc(fget_dist(idpelota.x,idpelota.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y));
                    ELSE
                        // Busca un  ngulo para la pelota seg£n la esquina
                        SWITCH (esquinacorner)
                            CASE 0: idpelota.angulopelota=rand (280000,350000); END
                            CASE 1: idpelota.angulopelota=rand (190000,260000); END
                            CASE 2: idpelota.angulopelota=rand ( 10000, 80000); END
                            CASE 3: idpelota.angulopelota=rand (100000,170000); END
                        END
                        // Y tira la pelota
                        idpelota.velocidadpelota= coge_veloc(rand(12000,44000));
                        // Pone a todos los jugadores que recib¡an la pelota en normal
                    END
                    FOR (contador=0;contador<10;contador++)
                        IF (idjugadores1[contador].estado==recibecorner)
                            idjugadores1[contador].estado=normal;
                        END
                        IF (idjugadores2[contador].estado==recibecorner)
                            idjugadores2[contador].estado=normal;
                        END
                    END
                    idporteros[0].estado=normal;
                    idporteros[1].estado=normal;
                    // Pasa al siguiente estado de juego en c¢rner
                    estadojuego=23;
                    sacandocorner=3;
                END
            END
            CASE 23:
                IF (sacandocorner==3)
                    // Quita a los que estaban sacando el c¢rner
                    IF (equipototal[0].sacacorner<>0)
                        equipototal[0].sacacorner=0;
                    END
                    IF (equipototal[1].sacacorner<>0)
                        equipototal[1].sacacorner=0;
                    END
                    // Y pasa al modo de juego normal
                    sacandocorner=0;
                    estadojuego=4;
                END
            END
        END

        // Comprueba si al hacer saque de centro est n todos en su posici¢n
        cuentaparados=0;
        IF (estadoequipo==0 AND estadojuego==0)
            FROM contador=0 TO 9;
                IF (idjugadores1[contador].estado==parado AND
                    idjugadores1[contador].velocidad==0   AND
                    idjugadores2[contador].estado==parado AND
                    idjugadores2[contador].velocidad==0   )
                    cuentaparados++;
                END
            END
            IF (cuentaparados==10)
                estadojuego=1;
            END
        END
        cuentaparados=0;
        IF (estadojuego==16 AND estadoequipo==0 AND sacandoporteria==2)
            FROM contador=0 TO 9;
                IF (idjugadores1[contador].estado==parado AND
                    idjugadores1[contador].velocidad==0   AND
                    idjugadores2[contador].estado==parado AND
                    idjugadores2[contador].velocidad==0   )
                    cuentaparados++;
                END
            END
            IF (cuentaparados==10)
                estadojuego=17;
                cuentaparados=0;
                sacandoporteria=3;
            END
        END

        FRAME;

    END
END

//-------------------------------------------------------------------
// Proceso daquienpasa
// Da el n£mero de jugador que esta mas solo para pasarle la pelota
// Entradas: equipo = N£mero del equipo
// Salidas: mascerca= N£mero del jugador a quien pasar
//-------------------------------------------------------------------

PROCESS daquienpasa(equipo)

PRIVATE

    distanciasmin[10];              // Guarda la distancias de los jugadores
                                    // a los contrarios que tengan mas cerca
    mascerca;                       // Guarda los que van teniendo mas que los otros
    masadelantado;

BEGIN

    // Va mirando uno a uno todos los jugadores
    FOR (contador=0;contador<10;contador++)
        // Dependiendo del equipo que se quiera calcular
        IF (equipo==0)
            // Si el jugador no esta en estado normal, pasa al siguiente
            IF (idjugadores1[contador].estado<>normal)
                // Y pone como si tuviera un contrario pegado
                distanciasmin[contador]=0;
            ELSE
                // Si no, el contrario que tiene mas cerca por defecto es el 0
                mascerca=0;
                // Va mirando por los dem s contrarios
                FOR (contador2=1;contador2<10;contador2++)
                    // Si este esta mas cerca que el ultimo, lo pone en su posici¢n
                    IF (fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,idjugadores2[contador2].x,idjugadores2[contador2].y)<
                        fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,idjugadores2[mascerca].x,idjugadores2[mascerca].y))
                        mascerca=contador2;
                    END
                END
                masadelantado=fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,(((xfcampo*100)-(xicampo*100))/2)+(xicampo*100),yfcampo*100)/((xfcampo-xicampo)*100);
                // Guarda la distancia del contrario que tenga mas cerca en la tabla
                distanciasmin[contador]=fget_dist(idjugadores1[contador].x,idjugadores1[contador].y,idjugadores2[mascerca].x,idjugadores2[mascerca].y)-(masadelantado*5000);
           END
        ELSE
            // Igual pero para el otro equipo
            IF (idjugadores2[contador].estado<>normal)
                distanciasmin[contador]=0;
            ELSE
                mascerca=0;
                FOR (contador2=1;contador2<10;contador2++)
                    IF (fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,idjugadores1[contador2].x,idjugadores1[contador2].y)<
                        fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,idjugadores1[mascerca].x,idjugadores1[mascerca].y))
                        mascerca=contador2;
                    END
                END
                masadelantado=fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,(((xfcampo*100)-(xicampo*100))/2)+(xicampo*100),yicampo*100)/((xfcampo-xicampo)*100);
                distanciasmin[contador]=fget_dist(idjugadores2[contador].x,idjugadores2[contador].y,idjugadores1[mascerca].x,idjugadores1[mascerca].y)-(masadelantado*5000);
            END
        END
    END

    // Se toma por defecto el 0, como el que tiene a los jugador que esta mas solo
    mascerca=0;
    FOR (contador=1;contador<10;contador++)
        // Mira si la distancia del contrario mas cerca, es mayor que la del ultimo
        IF (distanciasmin[contador]>distanciasmin[mascerca])
            // Si es mayor lo pone como el que esta mas solo
            mascerca=contador;
        END
    END
    // Devuelve el valor del jugador mas solo
    RETURN(mascerca);
END

//-------------------------------------------------------------------
// Proceso porteros
// Controla los porteros
// Entradas: cual      = N£mero de equipo
//           yiportero = Coordenada y inicial del portero
//           file      = Variable predefinida
//-------------------------------------------------------------------

PROCESS porteros(cual, yiportero, file)

PRIVATE

    pesopelota2;    // Peso de la pelota respecto al portero
    distbalon;      // Distancia a la pelota
    destx,desty;    // Posici¢n donde deber¡a estar el portero
    xtemp2,ytemp2;  // Guarda los puntos de control del portero
    cuentatirada=0; // Contador del estado cuando el portero se tira

BEGIN
    height=-5;      // Corrige la altura del gr fico (hueco debajo de los pies)
    ctype=2;        // Lo pone en modo 7
    resolution=100; // Usa dos decimales
    size=150;       // Coge el tama¤o apropiado
    graph=17;       // Elige el gr fico inicial

    // Coordenadas iniciales del portero
    x=(((xfcampo-xicampo)/2)+xicampo)*100;
    y=yiportero;

    // Estado inicial a normal
    estado=normal;
    LOOP
        // Mira si tiene el testigo
        IF (cual==testigo3)
            // Comprueba el estado del portero
            SWITCH (estado)
                // Estado normal
                CASE normal:
                    // Halla la coordenada x destino del portero respecto a la pelota
                    destx=((381+((((idpelota.x/100))*165)/926))*100);
                    // Coge la distancia a la pelota
                    distbalon=fget_dist(x,y,idpelota.x,idpelota.y);
                    // Calcula el peso a la pelota dependiendo de la distancia
                    IF (distbalon>30000)
                        pesopelota2=0;
                    ELSE
                        pesopelota2=(30000-distbalon)/600;
                    END
                    // Calcula la coordenada y respecto al peso de la pelota
                    desty=(((100*yiportero)+(pesopelota2*idpelota.y))/(100+pesopelota2));
                    // Mueve al portero hacia su posici¢n
                    velocidad=fget_dist(x,y,destx,desty)/4;
                    IF (velocidad>vmaxima) velocidad=vmaxima; END
                    IF (velocidad<100) velocidad=100; END
                    anguloportero=fget_angle(x,y,destx,desty);
                    // Si ha llegado hasta la pelota se para
                    IF (fget_dist(x,y,idpelota.x,idpelota.y)<=(vmaxima)/2) velocidad=0; END

                    // Si la pelota esta cerca y no hay ning£n jugador cerca, sale a por ella
                    IF (fget_dist(x,y,idpelota.x,idpelota.y)<equipototal[0].distanciaminima AND
                        fget_dist(x,y,idpelota.x,idpelota.y)<equipototal[1].distanciaminima AND
                        fget_dist(x,y,idpelota.x,idpelota.y)<22000)
                        velocidad=fget_dist(x,y,idpelota.x,idpelota.y)/4;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        anguloportero=fget_angle(x,y,idpelota.x,idpelota.y);
                        IF (fget_dist(x,y,idpelota.x,idpelota.y)<=(vmaxima+200)/2) velocidad=0; END
                    ELSE
                        velocidad=fget_dist(x,y,destx,desty)/4;
                        IF (velocidad>vmaxima) velocidad=vmaxima; END
                        IF (velocidad<100) velocidad=100; END
                        anguloportero=fget_angle(x,y,destx,desty);
                        IF (fget_dist(x,y,destx,desty)<=(vmaxima+200)/2) velocidad=0; END
                    END

                    // Mueve al portero
                    IF (velocidad>0)
                        x+=get_distx(anguloportero,velocidad);
                        y+=get_disty(anguloportero,velocidad);
                    END

                    // Comprueba si coge la pelota
                    IF (fget_dist(x,y,idpelota.x,idpelota.y)<zonarecogida*3 AND height<180 AND sacandoporteria==0)
                        velocidad=0;
                        estado=conbalon;
                        idpelota.velocidadpelota=0;
                        estadojuego=15;
                        porterosacando=cual;
                        sacandoporteria=1;

                    END

                    // Mira si el portero se tira
                    IF (idpelota.y-y<500 AND abs(idpelota.x-x)<zonarecogida*5 AND
                        fget_dist(x,y,idpelota.x,idpelota.y)>zonarecogida*3 AND idpelota.velocidadpelota>0)
                        estado=porterotirandose;
                        cuentatirada=0;
                        pesopelota=1;
                    END

                    // Si el muñeco se ha movido de posici¢n se hace la animaci¢n
                    IF (x_vieja<>x)
                        IF (x_vieja<x)
                            graph++;IF (graph>23)graph=17; END
                        ELSE
                            graph--;IF (graph<17)graph=23; END
                        END
                    ELSE
                        IF (y_vieja<>y)
                            IF (y_vieja<y)
                                graph++;IF (graph>23)graph=17; END
                            ELSE
                                graph--;IF (graph<17)graph=23; END
                            END
                        END
                    END
                END

                // Estado del portero cuando tiene la pelota y saca
                CASE conbalon:
                    graph=9;
                    // Coge los puntos de control( en manos) del portero y le pone la pelota
                    IF (porterosacando==0)
                        get_point(fichero4,graph,1,&xtemp2,&ytemp2);
                        idpelota.y=idporteros[porterosacando].y-100;
                    ELSE
                        get_point(fichero5,graph,1,&xtemp2,&ytemp2);
                        idpelota.y=idporteros[porterosacando].y+100;
                    END
                    idpelota.x=(idporteros[porterosacando].x+((xtemp2-50)*150));
                    idpelota.height=(105-ytemp2);

                    idpelota.graph=1;
                    IF (estadojuego>16)
                        // Mira en que estado de saque esta en este momento
                        SWITCH(cuenporterosacando)
                            // Va cambiando el gr fico
                            CASE 0:
                                graph=9;
                            END
                            CASE 1..7:
                                graph++;
                            END
                            // Acaba el saque
                            CASE 8:
                                estadojuego=18;
                                pesopelota=3;
                                graph=17;
                            END
                        END
                        // Coge el punto de control de las manos
                        IF (porterosacando==0)
                            get_point(fichero4,graph,1,&xtemp2,&ytemp2);
                            idpelota.y=y-100;
                        ELSE
                            get_point(fichero5,graph,1,&xtemp2,&ytemp2);
                            idpelota.y=y+100;
                        END
                        // Y le pone la pelota en las manos
                        idpelota.x=x+((xtemp2-50)*150);
                        idpelota.height=(105-ytemp2);
                        ++cuenporterosacando;
                    END
                END

                // Estado del portero tir ndose
                CASE porterotirandose:
                    // Comprueba en que estado de la animaci¢n esta
                    SWITCH(cuentatirada)
                        CASE 0:
                            // Mira hacia donde es ...
                            IF (x>idpelota.x)
                                IF (cual)
                                    flags=1;
                                ELSE
                                    flags=0;
                                END
                            ELSE
                                IF (cual)
                                    flags=0;
                                ELSE
                                    flags=1;
                                END
                            END
                            graph=1;
                        END
                        // Va cambiando el gr fico
                        CASE 1..7:
                            graph++;
                        END
                        // Acaba la estirada
                        CASE 8:
                            estado=normal;
                            graph=17;
                            flags=0;
                        END
                    END
                    // Comprueba si el bal¢n choca con los puntos de control del portero
                    FOR (contador=1;contador<6;contador++)
                        IF (porterosacando==0)
                            get_point(fichero4,graph,contador,&xtemp2,&ytemp2);
                        ELSE
                            get_point(fichero5,graph,contador,&xtemp2,&ytemp2);
                        END
                        IF ((abs(idpelota.x-(x+(xtemp2*150)-5000))<2000 AND abs(idpelota.height-ytemp2)<2000 AND abs(idpelota.y-y)<200))
                            estado=normal;
                            graph=17;
                            flags=0;
                            angulopelota=fget_angle(0,0,get_distx(angulopelota,velocidadpelota),-get_disty(angulopelota,velocidadpelota));
                            velocidadpelota+=4;
                        END
                    END
                    cuentatirada++;
                END
            END
        END

        // Pone la sombra del portero
        ponsombraportero(cual);
        FRAME;

        // Guarda las coordenadas para la pr¢xima vez
        x_vieja=x;y_vieja=y;
    END

END

//-------------------------------------------------------------------
// Proceso ponsombraportero
// Dibuja las sombras de los porteros
// Entradas: cualportero = N£mero de equipo del portero
//-------------------------------------------------------------------

PROCESS ponsombraportero(cualportero)

BEGIN
    // Dependiendo del portero elige el fichero gr fico
    IF (cualportero==0)
        file=fichero8;
    ELSE
        file=fichero9;
    END

    // Si esta tir ndose
    if (father.estado==tirandose)
        // Mira hacia que lado se tira
        IF (father.x>idpelota.x)
            IF (cualportero)
                flags=1;
            ELSE
                flags=0;
            END
        ELSE
            IF (cualportero)
                flags=0;
            ELSE
                flags=1;
            END
        END
    END
    height=15;      // Corrige la altura por los gr ficos
    resolution=100; // Pone dos decimales como los porteros
    ctype=2;        // Lo pone dentro del modo 7
    flags=4;        // Transparente

    // Pone las coordenadas y el gr fico id‚ntico al del portero que lo usa
    x=father.x;
    y=father.y-2;   // Se incrementa la y un poco para que aparezca delante
    graph=father.graph;
    FRAME;
END

//-------------------------------------------------------------------
// Proceso bander¡n
// Pone los banderines de los c¢rners
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS bander¡n (x,y)

BEGIN

    // Elige el gr fico y lo pone en el modo 7
    file=fichero1;
    graph=1;
    ctype=2;
    LOOP
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso ponmensajes
// Pone el mensajes durante el juego un tiempo determinado
// Entradas: cualmensaje  = N£mero del mensaje a poner
//           tiempopuesto = Tiempo que va a estar puesto
//-------------------------------------------------------------------

PROCESS ponmensajes (cualmensaje, tiempopuesto)

PRIVATE

    // Tabla con los mensaje a poner
    mensaje[]="SAQUE INICIO","1er TIEMPO","2do TIEMPO","GOL!!!","FUERA","CORNER","DESCANSO","FIN DEL PARTIDO";
    contatexto; // Contador del tiempo que lleva puesto

BEGIN

    // Hace sonido de silbato
    sound(idsonido0,50,256);

    // Guarda el total de tiempo
    contatexto=tiempopuesto;

    // Pone mensaje dependiendo de modo de v¡deo
    IF (modovideo==0)
        idtextomensaje=write(fuente1,320-(160*modovideo),240-(140*modovideo),4,mensaje[cualmensaje]);
    ELSE
        idtextomensaje=write(fuente2,320-(160*modovideo),240-(140*modovideo),4,mensaje[cualmensaje]);
    END

    // Y Imprime hasta que se acabe el tiempo
    WHILE (contatexto>0)
        contatexto--;
        FRAME;
    END

    // Borra el texto de pantalla
    IF (idtextomensaje<>0)
        delete_text(idtextomensaje);
        idtextomensaje=0;
    END

END

//-------------------------------------------------------------------
// Proceso traje2
// Pone los trajes de las pantallas de informaci¢n sobre el partido
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS traje2(x,y,graph,file)

BEGIN
    LOOP FRAME; END
END

//-------------------------------------------------------------------
// Proceso ponminicamisas
// Pone las minicamisas de los marcadores
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS ponminicamisas(x,y,graph,file,size)

PRIVATE
    guardagrafico;

BEGIN
    guardagrafico=graph;
    LOOP
        IF (conmarcadores==0) graph=0; ELSE graph=guardagrafico; END
        FRAME;
    END
END

//-------------------------------------------------------------------
// Proceso cr‚ditos
// Pone los cr‚ditos y finaliza el juego
//-------------------------------------------------------------------

PROCESS cr‚ditos()

PRIVATE
    idfondopant;    // Identificador del fondo de pantalla

BEGIN

    // Borra los textos y cualquier otro proceso
    fade_off();
    let_me_alone();
    delete_text(all_text);

    // Carga la pantalla de fondo con su paleta y el grafico del rat¢n
    mouse.file=fichero1; mouse.graph=909;
    idfondopant=load_map("futbol\presenta.map");
    load_pal("futbol\presenta.map");

    // Y la pone en pantalla
    put_screen(0,idfondopant);

    // Descarga otra letra que hubiera antes
    unload_fnt(fuente1);

    // Y carga la adecuada
    fuente1=load_fnt("futbol\futbol4.fnt");

    // Escribe los textos adecuados
    write(fuente1,320,0,1,"- CREDITOS -");
    write(fuente1,320,50,1,"PROGRAMADO POR:");
    write(fuente1,320,80,1,"ANTONIO MARCHAL.");
    write(fuente1,320,130,1,"GRAFICOS:");
    write(fuente1,320,160,1,"RAFAEL BARRASO.");
    write(fuente1,320,190,1,"JOSE FERNANDEZ.");
    write(fuente1,320,320,1,"MUSICA:");
    write(fuente1,320,350,1,"MOISES DIAZ TOLEDANO");

    write(fuente1,320,400,1,"-COPYRIGHT 1997-");

    fade_on();

    // Espera hasta que pase un tiempo o se pulse la tecla espacio
    scan_code=0;
    WHILE (scan_code==0 AND NOT mouse.left AND NOT mouse.right)
        FRAME;
    END

    // Graba los colores de las camisetas para otras partidas
    save ("dat\futbol\traje0.dat",&colorini1,sizeof(colorini1));
    save ("dat\futbol\traje1.dat",&colorini2,sizeof(colorini2));

    // Apaga la pantalla y sale del programa
    fade_off();
    exit("Thanks for playing!",0);
END

//-------------------------------------------------------------------
// Proceso bal¢nfantasma
// Crea un bal¢n para cuando se sale del campo
// Entradas: Variables predefinidas
//           velocidadpelota = velocidad de la pelota
//           angulopelota    = angulo que lleva
//-------------------------------------------------------------------

PROCESS bal¢nfantasma(x,y,size,height,graph,file,velocidadpelota,angulopelota)

BEGIN
    resolution=100;
    ctype=2;
    REPEAT

        // Mira la velocidad del bal¢n
        IF (velocidadpelota>0)
            velocidadpelota-=incrvelocidadpelota;   // Lo va frenando
            // Mueve el bal¢n
            x+=get_distx(angulopelota,velocidadpelota);
            y+=get_disty(angulopelota,velocidadpelota);
        END

        // Si el bal¢n se mueve o bota, realiza la animaci¢n
        IF (velocidadpelota>0)
            IF (graph<>0)
                graph++;IF (graph>6) graph=1; END
            END
        END
        FRAME;
    UNTIL (x<(xicampo-180)*100 OR x>(xfcampo+180)*100 OR
           y<(yicampo-180)*100 OR y>(yfcampo+180)*100 OR
           velocidadpelota<incrvelocidadpelota)
END

//-------------------------------------------------------------------
// Proceso elige_saque
// Elige al jugador que se va a pasar en saques de banda y de centro
// Entradas: numerojugador = numero del jugador que esta sacando
//-------------------------------------------------------------------

PROCESS elige_saque(numerojugador)

PRIVATE
    paratimer;              // Sirve para parar el tiempo
    canglec,cveloc,cdistc;  // Variables para mover la camara
    cincrx,cincry;
BEGIN

    // Se elige el gr fico y se reinicializa
    ctype=2;
    file=fichero1;
    graph=4;
    flags=4;
    resolution=100;

    // Para el reloj y todos los procesos
    paratimer=timer[0];
    signal(idinicio,s_freeze_tree);
    signal(id,s_wakeup);

    // Repite hasta que se pulse una tecla o el joystick
    WHILE (NOT key(_control) AND NOT key(_alt) AND NOT joy.button1 AND NOT joy.button2)

        // Si se pulsan los cursores o el joystick va cambiando de elegido
        IF (key(_right) OR key (_up) OR joy.right OR joy.up)
            elegidosaque++;
            IF (elegidosaque>9) elegidosaque=0; END
            IF (elegidosaque==numerojugador) elegidosaque++; END
            IF (elegidosaque>9) elegidosaque=0; END
            WHILE (key(_right) OR key (_up) OR joy.right OR joy.up) FRAME; END
        END
        IF (key(_left) OR key (_down) OR joy.left OR joy.down)
            elegidosaque--;
            IF (elegidosaque<0) elegidosaque=9; END
            IF (elegidosaque==numerojugador) elegidosaque--; END
            IF (elegidosaque<0) elegidosaque=9; END
            WHILE (key(_left) OR key (_down) OR joy.left OR joy.down) FRAME; END
        END
        x=idjugadores2[elegidosaque].x;
        y=idjugadores2[elegidosaque].y;

        // Coge el  ngulo hacia la posici¢n adecuada de la c mara
        canglec=fget_angle(idcamara.x,idcamara.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y+(distanciacamara*100));

        // Coge la distancia
        cdistc=fget_dist(idcamara.x,idcamara.y,idjugadores2[elegidosaque].x,idjugadores2[elegidosaque].y+(distanciacamara*100));

        // Haya la velocidad
        cveloc=cdistc/totaltestigo;

        // Si la velocidad esta por encima del valor m ximo, la trunca
        IF (cveloc>vmaxima*6) cveloc=vmaxima*6; END

        // Coge los incrementos x e y, de acuerdo a la velocidad y el  ngulo
        cincrx=get_distx(canglec,cveloc);
        cincry=get_disty(canglec,cveloc);

        // Mueve la c mara
        idcamara.x+=cincrx;
        idcamara.y+=cincry;
        FRAME;
        timer[0]=paratimer;
    END

    // Devuelve el control a los procesos
    signal(idinicio,s_wakeup_tree);
END
//-------------------------------------------------------------------
// Proceo fuerza_tiro()
// Hace que se mantenga la fuerza del tiro cuando se sigue pulsando el boton
//-------------------------------------------------------------------

PROCESS fuerza_tiro()

PRIVATE
    tiempomaximo;

BEGIN
    // Repite mientras se tenga pulsado el boton de tiro o el joystick
    WHILE ((key (_control) OR joy.button1) AND tiempomaximo<10)
        // Mantiene la velocidad de la pelota
        idpelota.velocidadpelota=2000;
        tiempomaximo++;
        FRAME;
    END

END
