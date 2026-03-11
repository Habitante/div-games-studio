
//--------------------------------------------------------------------
// TITULO: PUZZLE 'O' MATIC
// AUTOR:  DANIEL NAVARRO
// FECHA   25/08/97
//--------------------------------------------------------------------

PROGRAM puzzle;

GLOBAL

    cogida;             // Bandera-> 1.- Con una pieza cogida, 0.- Sin pieza cogida
    rotando;            // Bandera-> 1.- Se esta rotando una pieza, 0.- No se esta rotando nada
    £ltima_z;           // Ultima coordenada z de las piezas, para ir poniendo las piezas por delante
    piezas_colocadas;   // Numero de piezas colocadas

    nivel;              // Nivel actual
    nivel2;             // Guarda temporalmente el nivel al que se va a pasar

    fuente;             // Fuente actual que se esta usando

    // Nombres de las fuentes que se tienen que cargar
    fuentes[]=
        "puzzle\puzzle1.fnt","puzzle\puzzle2.fnt","puzzle\puzzle3.fnt",
        "puzzle\puzzle4.fnt","puzzle\puzzle5.fnt";

    fichero;            // Fichero de gr ficos que se esta usando

    // Nombres de los ficheros gr ficos que se tienen que usar
    ficheros[]=
        "puzzle\puzzle.fpg","puzzle\puzzle2.fpg","puzzle\puzzle3.fpg",
        "puzzle\puzzle4.fpg","puzzle\puzzle5.fpg";

    tiempo;             // Contador del tiempo restante
    tiempo_anterior;    // Variable temporal donde se guarda el tiempo restante
    tiempo_restante;    // Tiempo que va sobrando de un nivel a otro

    // Tiempos totales de cada fase
    tiempos[]=
        250,200,150,100,50;

    r‚cord;             // Contador del tiempo que se tiene para superar el r‚cord
    mensaje_r‚cord;     // Bandera-> 1.-Se ha hecho r‚cord

    // Tabla que guarda los r‚cords de cada nivel
    r‚cords[]=
        250,250,250,250,250;

    // Identificadores de los textos que aparecen en el juego
    texto1,texto2,texto3,texto4,texto5;

    // Identificadores de sonidos
    idsonido1,idsonido2,idsonido3,idsonido4,idsonido5,idsonido6,idsonido7;

BEGIN
    // Selecciona del modo de v¡deo y los frames por segundo
    set_mode(m640x480);
    set_fps(32,0);      // Selecciona el n£mero de imagenes por segundo

    // Carga los sonidos
    idsonido1=load_pcm("puzzle\cogida.pcm",0);
    idsonido2=load_pcm("puzzle\dejada.pcm",0);
    idsonido3=load_pcm("puzzle\girando.pcm",0);
    idsonido4=load_pcm("puzzle\colocada.pcm",0);
    idsonido5=load_pcm("puzzle\alarma.pcm",0);
    idsonido6=load_pcm("puzzle\acaba.pcm",0);
    idsonido7=load_pcm("puzzle\consigue.pcm",0);

    // Carga los r‚cords guardados
    load("puzzle\puzzle.dat",offset r‚cords);

                            // Carga las 35 piezas de ficheros map
    FROM x=0 TO 34;         // Usa la variable predefinida como contador
        load_map("puzzle\puzzle.map");
    END

                            // Bucle principal
    LOOP

        fade_on();          // Enciende la pantalla

        // Carga la pantalla del men£ principal y la pone en pantalla
        load_pal("puzzle\puzmenu.map");
        graph=load_map("puzzle\puzmenu.map");   // Usa la variable graph para cargarlo
        put_screen(0,graph);
        unload_map(graph);                      // Descarga el gr fico para recuperar memoria
        graph=0;                                // Quita el gr fico para que no se imprima

        // Repite hasta que se pulse F1 para empezar a jugar
        REPEAT

            FRAME;

            // Si se pulsa la tecla escape se graba los r‚cords y se sale del programa
            IF (key(_esc))
                fade_off();
                save("puzzle\puzzle.dat",offset r‚cords,sizeof(r‚cords));
                exit("",0);
            END

        UNTIL (key(_f1));

        fade_off();             // Apaga la pantalla y la borra
        clear_screen();

                                // Reinicializa el juego
        nivel=0 ;
        tiempo_restante=0;

                                // Bucle principal del juego
        LOOP

            // Carga el puzzle del nivel y la fuente, todo ello con su paleta
            fichero=load_fpg(ficheros[nivel]);
            fuente=load_fnt(fuentes[nivel]);
            load_pal(ficheros[nivel]);
            put_screen(0,2);
            mouse.graph=200;    // Selecciona el cursor del rat¢n


            // Crea las piezas del puzzle
            FROM y=0 TO 6;
                FROM x=0 TO 4;

                   // Copia el bloque gr fico a cada pieza que es un c¢digo del fichero de gr ficos
                   map_block_copy(0,1000+y*5+x,0,0,1,x*64,y*64,64,64);

                   // Mueve y gira las piezas en posiciones aleatorias cuando las crea
                   pieza(1000+y*5+x,rand(32,608),rand(200,448),rand(0,3)*90000,192+x*64,48+y*64);
                END
            END
            fade_on();          // Enciende la pantalla

            // Reinicializa las variables del nivel
            cogida=0;                               // Pieza cogida
            rotando=0;                              // Pieza rotando
            £ltima_z=-1;                            // Profundidad de la £ltima pieza puesta
            piezas_colocadas=0;                     // Piezas puestas
            nivel2=nivel+1;                         // Variable temporal que guarda el nivel
            tiempo=tiempos[nivel]+tiempo_restante;  // Tiempo restate
            r‚cord=r‚cords[nivel];                  // Tiempo record

            // Pone los textos necesarios
            write(fuente,72,0,1,"TIEMPO");
            write_int(fuente,72,24,1,&tiempo);
            write(fuente,72,80,1,"RECORD");
            write_int(fuente,72,104,1,&r‚cord);
            texto1=write(fuente,320,240,4,"PULSE EL RATON");
            texto2=write(fuente,320,264,4,"PARA JUGAR EL NIVEL");
            texto3=write_int(fuente,320,290,4,&nivel2);
            IF (nivel>0)
                texto4=write(fuente,320,120,4,"TIEMPO SOBRANTE");
                texto5=write_int(fuente,320,150,4,&tiempo_restante);
            END

            // Espera hasta que se pulse el bot¢n del rat¢n
            REPEAT FRAME; UNTIL (mouse.left);

            // Borra los textos que ya no sirven
            delete_text(texto1);
            delete_text(texto2);
            delete_text(texto3);
            IF (nivel>0)
                delete_text(texto4);
                delete_text(texto5);
            END

            // Inicializa otras variables
            mensaje_r‚cord=FALSE;   // Puesto mensaje de record
            timer=0;                // Contador de tiempo

            // Bucle principal de cada nivel de juego
            REPEAT

                // Calcula los tiempos de juego y r‚cord
                tiempo_anterior=tiempo;
                tiempo=tiempos[nivel]+tiempo_restante-timer/100;
                r‚cord=r‚cords[nivel]-timer/100;

                // Crea los procesos tipo mensaje con una frase sobre el tiempo restante
                IF (tiempo_anterior==121 AND tiempo==120) mensaje("QUEDAN 2 MINUTOS",220); END
                IF (tiempo_anterior==61 AND tiempo==60) mensaje("1 MINUTO, DATE PRISA!",220); END
                IF (tiempo_anterior==11 AND tiempo==10) mensaje("10 SEGUNDOS SOLO !!!",220); END
                IF (tiempo_anterior==6 AND tiempo==5) mensaje("CINCO !!!",220); END
                IF (tiempo<0) tiempo=0; END

                // Comprueba si ya no se puede hacer r‚cord
                IF (r‚cord<0)
                    r‚cord=0;
                    // Pone el mensaje de r‚cord perdido
                    IF (NOT mensaje_r‚cord)
                        mensaje_r‚cord=TRUE;
                        mensaje("RECORD PERDIDO!",260);
                    END
                END

                // Si se pulsa la tecla escape sale al menu
                IF (key(_esc)) tiempo=-1; BREAK; END
                FRAME;

            // Repite hasta que se acabe el tiempo o coloque todas las piezas
            UNTIL (piezas_colocadas==35 OR tiempo==0);

            // Comprueba si se ha pasado de nivel o se ha acabado el tiempo
            IF (piezas_colocadas==35)   // Nivel conseguido
                sound(idsonido7,128,256);

                // Pone el mensaje de felicitaci¢n
                write(fuente,320,220,4,"LO CONSEGUISTES");

                // Comprueba si se ha conseguido un r‚cord y lo guarda
                IF (r‚cords[nivel]>tiempos[nivel]+tiempo_restante-tiempo)
                    write(fuente,320,245,4,"NUEVO RECORD!!!");
                    r‚cords[nivel]=tiempos[nivel]+tiempo_restante-tiempo;
                END

                // Guarda el tiempo sobrante para el siguiente nivel
                write(fuente,320,290,4,"TIEMPO SOBRANTE");
                write_int(fuente,320,320,4,&tiempo);
                tiempo_restante=tiempo;

            ELSE    // Si no, se le ha acabado el tiempo

                sound(idsonido6,128,256);   // Hace un sonido

                // Pone el mensaje de final de juego y lo reinicia
                IF (tiempo>=0)
                    write(fuente,320,220,4,"OHHH, FUERA DE TIEMPO!");
                ELSE
                    tiempo=0;
                END
                write(fuente,320,260,4,"FIN DE JUEGO");
            END

            // Pone el mensaje de pulsar el rat¢n para continuar
            write(fuente,320,440,4,"PULSE EL RATON PARA CONTINUAR");
            WHILE (mouse.left) FRAME; END

            // Espera a que se pulse el rat¢n
            REPEAT FRAME; UNTIL (mouse.left);
            fade_off();             // Apaga la pantalla
            delete_text(all_text);  // Borra cualquier texto

            // Elimina los procesos que hubiera y descarga los ficheros de memoria
            signal(TYPE pieza,s_kill);
            unload_fnt(fuente);
            unload_fpg(fichero);


            nivel++;            // Incrementa el nivel de juego

            // Juego finalizado, todos los puzzles conseguidos
            IF (nivel>4) nivel=4; pantalla_final(); FRAME; END

            // Mira si se ha acabado por no poner todas las piezas
            IF (piezas_colocadas<35) BREAK; END
        END

        // Apaga la pantalla y borra cualquier proceso que hubiera abierto
        fade_off();
        let_me_alone();
    END
END

//--------------------------------------------------------------------
// Proceso pieza
// Maneja las piezas del puzzle
// Entradas: Variable predefinidas
//           mi_x, mi_y = coordenadas dentro del puzzle
//--------------------------------------------------------------------

PROCESS pieza(graph,x,y,angle,mi_x,mi_y);

PRIVATE
    contador;       // Contador de uso general
    incr_x,incr_y;  // Distancia entre el punto medio de la pieza y el rat¢n

BEGIN
    LOOP
        // Si no hay ninguna cogida y esta pulsado el rat¢n
        IF (NOT cogida AND mouse.left)

            // Si esta tocando la pieza
            IF (collision(TYPE mouse))
                sound(idsonido1,128,256);
                cogida=1;       // Pon la variable como que se esta cogiendo una pieza
                z=£ltima_z;     // La pone por delante de todas
                £ltima_z--;

                // Coge la distancia al rat¢n para luego usarla
                incr_x=x-mouse.x;
                incr_y=y-mouse.y;

                // Repite hasta que se suelte el rat¢n
                REPEAT
                    // Hace que siga al rat¢n desde la distancia a la que esta
                    x=mouse.x+incr_x;
                    y=mouse.y+incr_y;

                    // Si tiene el  ngulo correcto
                    IF (angle MOD 360000==0)

                        // Y hay una distancia menor de 6 pixels a su posici¢n correcta
                        IF (fget_dist(x,y,mi_x,mi_y)<6)
                            sound(idsonido4,128,256);

                            // Coloca la pieza (pieza conseguida)
                            x=mi_x; y=mi_y; z=1;
                            piezas_colocadas++;
                            cogida=0;

                            // Y no hace mas cosas con la pieza, se mete en un bucle enemigo
                            LOOP FRAME; END
                        END
                    END
                    FRAME;
                UNTIL (mouse.left==0);

                cogida=0;   // Ya no se tiene ninguna pieza cogida
                // Hace el sonido de dejada
                sound(idsonido2,128,256);
            END
        END

        // Si no se esta rotando ninguna pieza y se pulsa el bot¢n derecho
        IF (NOT rotando AND mouse.right)

            // Comprueba si se esta tocando alguna pieza
            IF (collision(TYPE mouse))
                sound(idsonido3,64,96);

                // Pone la variable como que se esta rotando
                rotando=1;
                z=£ltima_z;
                £ltima_z--;

                // Gira la pieza de 10 en 10 grados, en un total de 90
                FROM contador=0 TO 8; angle+=10000; FRAME; END

                // Hace rotativo el giro por si se pasa de 360000
                IF (angle==360000) angle=0; END
                rotando=0;
            END
        END
        FRAME;
    END
END

//--------------------------------------------------------------------
// Proceso mensaje
// Pone los mensajes que aparecen durante el juego
// Entradas: texto = Texto que se quiere imprimir
//           y     = Coordenada y del texto
//--------------------------------------------------------------------

PROCESS mensaje(texto,y);

PRIVATE
    idtexto;    // Identificador del texto a imprimir

BEGIN
    sound(idsonido5,64,256);    // Hace sonido de alarma

    // Pone el texto
    idtexto=write(fuente,320,y,4,texto);

    // Hace frame pero con pausa
    FRAME(600);

    // Quita el texto
    delete_text(idtexto);

    // Hace que se espera si imprimir nada
    FRAME(600);

    sound(idsonido5,64,256);    // Hace otra vez el sonido de alarma

    // Lo repite 3 veces mas el mismo proceso de arriba
    idtexto=write(fuente,320,y,4,texto);
    FRAME(600);
    delete_text(idtexto);
    FRAME(600);
    sound(idsonido5,64,256);
    idtexto=write(fuente,320,y,4,texto);
    FRAME(600);
    delete_text(idtexto);
    FRAME(600);
    sound(idsonido5,64,256);
    idtexto=write(fuente,320,y,4,texto);
    FRAME(600);
    delete_text(idtexto);   // Se acaba borrando el texto
END

//--------------------------------------------------------------------
// Proceso pantalla_final
// Pone los mensajes que aparecen al acabar todos los puzzle
//--------------------------------------------------------------------

PROCESS pantalla_final()
BEGIN

    // Apaga y borra la pantalla y pone la del final del juego
    fade_off();
    let_me_alone();                         // Borra todos los procesos
    load_pal("puzzle\puzfinal.map");
    graph=load_map("puzzle\puzfinal.map");  // Usa la variable graph para cargar el mapa gr fico
    put_screen(0,graph);
    unload_map(graph);                      // Descarga el mapa gr fico de memoria
    fade_on();                              // Enciende la pantalla

    // Espera a que se pulse una tecla y sale del juego
    WHILE (scan_code==0)
        FRAME;
    END
    exit("Gracias por jugar!",0);           // Sale del programa

END