
//------------------------------------------------------------------------------
//TITTLE:      NOID SCREEN EDITOR
//AUTHOR:      DANIEL NAVARRO
//DATE:        2/15/97
//------------------------------------------------------------------------------

PROGRAM mapa_noid;

GLOBAL
    // Tabla con la posicion de los graficos dentro del fichero
    ladrillo_g[]=10,11,12,13,14,15,20,30;

    ladrillo_x;     // Coordenadas del cursor
    ladrillo_y;
    s_ladrillo;     // Tipo de ladrillo elegido

    mapa[16*14];    // Mapa que se graba en disco
    mapaid[16*14];  // Mapa de identificadores
    fondopantalla;  // Fondo de pantalla elegido
    sonido0;        // Identificador de sonido

BEGIN

    load_pal("noid\noid.fpg");  // Carga paleta de colores
    load_fpg("noid\noid.fpg");  // Carga fichero de graficos
    sonido0=load_pcm("noid\billar0.pcm",0);  // Carga sonido
    fade(100,100,100,8);        // Enciende la pantalla
    put(0,100,0,0);             // Pone el fondo de pantalla

    // Pone unos textos con las instrucciones
    write(0,272,0,0,"O,P,Q,A");
    write(0,272,10,0,"Mover");
    write(0,272,20,0,"cursor");
    write(0,272,30,0,"W,S");
    write(0,272,40,0,"Cambiar");
    write(0,272,50,0,"ladrillo");
    write(0,272,60,0,"L,¥");
    write(0,272,70,0,"Cambiar");
    write(0,272,80,0,"fondo");
    write(0,272,90,0,"Espacio");
    write(0,272,100,0,"Poner");
    write(0,272,110,0,"ladrillo");
    write(0,272,120,0,"F1");
    write(0,272,130,0,"Cargar");
    write(0,272,140,0,"mapa");
    write(0,272,150,0,"F2");
    write(0,272,160,0,"Grabar");
    write(0,272,170,0,"mapa");

    // Inicializa las coordenadas
    ladrillo_x=0;
    ladrillo_y=0;

    WHILE (NOT key (_esc))

        // Mueve de posicion el cursor
        IF (key(_q) AND ladrillo_y>0) ladrillo_y--; END
        IF (key(_a) AND ladrillo_y<13) ladrillo_y++; END
        IF (key(_o) AND ladrillo_x>0) ladrillo_x--; END
        IF (key(_p) AND ladrillo_x<15) ladrillo_x++; END

        // Cambia el ladrillo seleccionado
        IF (key(_w) AND s_ladrillo>0) s_ladrillo--; END
        IF (key(_s) AND s_ladrillo<7) s_ladrillo++; END

        // Cambia el fondo de pantalla
        IF (key(_l) AND fondopantalla>0)
            fondopantalla--;
            clear_screen();                 // Borra el fondo
            put(0,100+fondopantalla,0,0);   // Y pone el nuevo elegido
        END
        IF (key(_semicolon) AND fondopantalla<3)
            fondopantalla++;
            clear_screen();
            put(0,100+fondopantalla,0,0);
        END

        // Pone o borra un ladrillo
        IF (key(_space))
            sound(sonido0,256,256);          // Realiza sonido

            // Borra ladrillo
            IF (mapa[ladrillo_y*16+ladrillo_x]==ladrillo_g[s_ladrillo])
                mapa[ladrillo_y*16+ladrillo_x]=0;
                signal(mapaid[ladrillo_y*16+ladrillo_x],s_kill);
                mapaid[ladrillo_y*16+ladrillo_x]=0;
            ELSE

                // Pone ladrillo
                IF (mapaid[ladrillo_y*16+ladrillo_x])
                    signal(mapaid[ladrillo_y*16+ladrillo_x],s_kill);
                END
                mapaid[ladrillo_y*16+ladrillo_x]=ladrillo(16+ladrillo_x*16,12+ladrillo_y*8,ladrillo_g[s_ladrillo]);
                mapa[ladrillo_y*16+ladrillo_x]=ladrillo_g[s_ladrillo];
            END
            WHILE (key(_space)) END
        END

        // Graba mapa
        IF (key(_f1))
            save("dat\noid\screen.blk",&mapa,16*14);
        END

        // Carga mapa
        IF (key(_f2))
            FROM x=0 TO 223;            // Primero borra el mapa antiguo
                IF (mapaid[x])
                    signal(mapaid[x],s_kill);
                    mapaid[x]=0;
                END
            END
            load("dat\noid\screen.blk",&mapa);
            FROM y=0 TO 13;             // Crea el mapa nuevo
                FROM x=0 TO 15;
                    IF (mapa[y*16+x]<>0)
                        mapaid[y*16+x]=ladrillo(16+x*16,12+y*8,mapa[y*16+x]);
                    END
                END
            END
        END

        // Pinta cursor
        x=16+ladrillo_x*16;
        y=12+ladrillo_y*8;
        graph=ladrillo_g[s_ladrillo];
        flags=flags XOR 1;
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Process ladrillo
// Pone un objeto en el mapa
// Entradas: Variables predefinidas
//------------------------------------------------------------------------------

PROCESS ladrillo(x,y,graph)

BEGIN
    LOOP        // Crea un bucle infinito
        FRAME;
    END
END

