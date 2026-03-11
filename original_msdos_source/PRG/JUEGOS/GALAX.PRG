
//------------------------------------------------------------------------------
// TITULO:      GALAX
// AUTOR:       LUIS SUREDA
// FECHA:       2/15/97
//------------------------------------------------------------------------------

PROGRAM galax;

CONST
    ancho_escuadron=12;  // Tama¤o del escuadr¢n
    alto_escuadron=4;
    // Total de enemigos en el escuadr¢n
    n_enemigos= ancho_escuadron*alto_escuadron;

    ancho_nave=20;      // Distancia horizontal entre enemigos
    alto_nave=20;       // Distancia vertical entre enemigos
    movi_escuadron=1;   // Numero de pixel que se mover  el escuadr¢n

GLOBAL
    x_escuadron=60;     // Esquina superior-izquierda del escuadr¢n
    y_escuadron=20;

    opcion=0;           // Opci¢n del men£

    laser=0;            // Identificadores de sonido
    laser2=0;
    ataque3=0;
    ataque1=0;
    explo=0;

    puntuacion;         // Puntuaci¢n del jugador
    max_puntuacion=0;   // Maxima puntuaci¢n obtenida
    nivel;              // Nivel actual (1..n)
    siguiente_nivel;    // =1 Cuando el nivel esta completado
    vidas;              // Vidas restantes
    otra_vida;          // =1 Cuando la nave ha chocado con el enemigo o el fuego
    enemigos;           // Numero activo de enemigos
    cambia_dir;         // 1=Cambia la direcci¢n de movimiento del escuadr¢n
    dir;                // Direcci¢n de movimiento del escuadr¢n 0=izq.,1=der.
    bandera_misil;      // Bandera del misil . 0=no preparado 1=preparado
    idnave;             // Identificador de la nave

    // Formaci¢n inicial: 0 vac¡o, 1 tipo_nave_1, 2 tipo_nave_2, 3 tipo_nave_3

    formacion[n_enemigos]=3,3,3,3,3,0,0,3,3,3,3,3,
                          0,2,2,2,0,2,2,0,2,2,2,0,
                          2,1,2,1,0,2,2,0,1,2,1,2,
                          1,1,0,1,1,0,0,1,1,0,1,1;

    tipo_enemigo[n_enemigos];  // Formaci¢n en juego

LOCAL
    numero=0;          // Numero de enemigos

PRIVATE
    contador0=0;       // Contador

BEGIN

    load_fpg("galax\galax.fpg");  // Carga gr ficos
    load_fpg("galax\galax2.fpg"); // Carga pantalla de presentaci¢n
    load_fnt("galax\galax.fnt");  // Carga fuentes


    laser=load_pcm("galax\laser.pcm",0);   // Carga sonidos
    laser2=load_pcm("galax\laser2.pcm",0);
    explo=load_pcm("galax\laser3.pcm",0);
    ataque3=load_pcm("galax\buiu.pcm",0);
    ataque3=load_pcm("galax\fx1.pcm",0);

    priority=-1;

    LOOP        // Bucle de men£

        load_pal("galax\galax2.fpg");
        put_screen(1,1);    // Muestra pantalla de presentaci¢n

        fade_on();

        opcion=0;           // Valores iniciales
        vidas=3;
        nivel=1;
        scan_code=0;
        // Repite hasta que se pulse una tecla
        REPEAT              // Bucle de men£
            FRAME;
        UNTIL (scan_code<>0)

        // Si se pulsa escape se sale del juego
        IF (key (_esc)) BREAK; END

        // Si no, comienza a jugar y reinicia la variable de muerto
        otra_vida=0;

        // Comprueba si se ha hecho record la £ltima vez y pone a cero la puntuacion
        IF (puntuacion>max_puntuacion) max_puntuacion=puntuacion; END
        puntuacion=0;

        // Borra todos los textos que hubiera
        delete_text(all_text);

        fade_off();
        load_pal("galax\galax.fpg");
        put_screen(0,1);    // Ense¤a pantalla de fondo

        // Escribe los textos necesarios
        write(1,0,0,0,"PUNTUACION");
        write(1,320,0,2,"MAX-PUNTUACION");
        write(1,110,0,0,"VIDAS");
        write(1,160,0,0,"NIVEL");
        write_int(1,0,8,0,&puntuacion);
        write_int(1,120,10,0,&vidas);
        write_int(1,175,10,0,&nivel);
        write_int(1,320,10,2,&max_puntuacion);

        // Selecciona el n£mero de imagenes por segundo
        set_fps(34,0);
        fade_on();

        idnave=nave();      // Identificador para nave

        LOOP                // Bucle general
            siguiente_nivel=0;

            // Prepara la formaci¢n
            x_escuadron=60;
            enemigos=0;

            // Crea los enemigos mediante un bucle
            FOR (contador0=0;contador0<n_enemigos;contador0++)
                tipo_enemigo[contador0]=formacion[contador0];
                IF (tipo_enemigo[contador0]<>0)
                    enemigos++;     // Incrementa el n£mero de enemigos vivos
                    enemigo(contador0,tipo_enemigo[contador0]);
                END
            END

            dir=1;         // Inicialmente el escuadr¢n se mueve a la derecha
            cambia_dir=0;

            REPEAT         // Bucle del juego
                REPEAT     // Bucle de nivel
                    FRAME;
                    IF (dir==0)         // Mueve el escuadr¢n seg£n la direcci¢n
                        x_escuadron-=movi_escuadron;
                        IF (x_escuadron<0) cambia_dir=1; END
                    ELSE
                        x_escuadron+=movi_escuadron;
                        IF (x_escuadron>320) cambia_dir=1;  END
                    END

                    IF (cambia_dir==1)  // Cambia la direcci¢n
                        dir=dir XOR 1;
                        cambia_dir=0;
                    END

                    IF (key(_esc))      // Aborta el juego
                        otra_vida=1;
                        vidas=0;
                    END

                UNTIL (siguiente_nivel OR otra_vida)

                // Espera explosi¢n
                WHILE (get_id(TYPE explosion))
                    FRAME;
                END

                IF (otra_vida==1)       // El jugador pierde una nave
                    vidas--;
                    otra_vida=0;
                    fade_off();
                    //Termina procesos
                    signal(TYPE enemigo,s_kill);
                    signal(TYPE fuego,s_kill);
                    // Crea nuevos procesos para siguiente vida
                    idnave=nave();
                    FOR (contador0=0;contador0<n_enemigos;contador0++)   // Crea los enemigos
                        IF (tipo_enemigo[contador0]<>0)
                            enemigo(contador0,tipo_enemigo[contador0]);
                        END
                    END
                    fade_on();
                END
                IF (siguiente_nivel==1) BREAK; END
            UNTIL (vidas<0)

            IF (vidas<0) BREAK; END

            fade_off();
            fade_on();

            nivel++;    // Cambia el nivel
        END
        // Borra procesos y textos
        let_me_alone();
        delete_text(all_text);
    END     // Fin del juego
    creditos();
END

//------------------------------------------------------------------------------
// Proceso nave
// Crea y controla la nave
//------------------------------------------------------------------------------

PROCESS nave();

PRIVATE
    velocidad=0;    // Velocidad de la nave

BEGIN
    graph=13;           // Selecciona gr fico y coordenadas
    x=160;
    y=188;
    bandera_misil=0;    // Sirve para disparar solo un disparo cada vez
    LOOP

        // Lee las teclas y mueve la nave
        IF (key(_right) AND velocidad<6)
            velocidad+=2;
        ELSE
            IF (velocidad>0)  velocidad--; END
        END
        IF (key(_left) AND velocidad>-6)
            velocidad-=2;
        ELSE
            IF (velocidad<0)  velocidad++; END
        END
        x+=velocidad;

        IF (x>310)  // Limites de la pantalla
            x=310;
            velocidad=0;
        END
        IF (x<10)
            x=10;
            velocidad=0;
        END

        IF (key(_space) OR key (_control))      // Dispara misil
            IF (bandera_misil==0)               // Mira si hay otro disparado
                sound(laser,40,50);
                bandera_misil=1;
                misil(x,y-10);
            END
        END

        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso misil
// La nave lanza un misil.
// Simplemente va hacia arriba (y-=8).
//------------------------------------------------------------------------------

PROCESS misil(x,y);

BEGIN
    graph=14;           // Selecciona gr fico
    REPEAT
        y-=8;
        IF (graph==14)  // Cambia el gr fico (entre 14 y 15)
            graph=15;
        ELSE
            graph=14;
        END

        FRAME;
    UNTIL (out_region(id,0))    // Repite mientras el objeto este en pantalla
    bandera_misil=0;
END

//------------------------------------------------------------------------------
// Proceso fuego
// El fuego enemigo va hacia abajo(sentencia y+=300) en cada impresi¢n, y
// va hacia a la nave en direcci¢n x (sentencia x+=x_avance).
//------------------------------------------------------------------------------

PROCESS fuego(x,y,tipo_fuego);

PRIVATE
    id_proceso; // Identificador de la nave
    x_avance;   //  Porcentaje horizontal para usar con coordenada y

BEGIN
    graph=tipo_fuego;       // Selecciona gr fico
    sound(laser2,256,300);  // Realiza sonido
    resolution=100;         // Usa dos decimales

    IF (idnave.y==y)        // Si la nave esta en la vertical el incremento horizontal es 0
        x_avance=0;
    ELSE
        // Calcula el x_avance
        // *100 hace dos decimales
        // *3 porque el fuego se mueve 3 puntos cada vez

        x_avance=(((idnave.x-x)*resolution)*3)/(idnave.y-y);

        // M xima velocidad. Esta puede ser 0 si el disparo es vertical
        IF (x_avance>100) x_avance=100; END
        IF (x_avance<-100) x_avance=-100; END
    END

    y=y*resolution; // Convierte coordenadas a la resoluci¢n 100
    x=x*resolution;

    REPEAT
        IF ((graph==16) OR (graph==17)) // Cambia el gr fico
            IF (graph==16)
                graph=17;
            ELSE
                graph=16;
            END
        END
        // Mueve al proceso
        y+=300;
        x+=x_avance;

        // Comprueba si ha tocado la nave
        id_proceso=collision(TYPE nave);
        IF (id_proceso)
            // Crea un proceso tipo explosi¢n
            explosion(id_proceso.x,id_proceso.y,100);
            sound(explo,80,100);
            // Elimina los procesos de tiro y de la nave
            signal(id_proceso,s_kill);
            signal(id,s_kill);
            otra_vida=1;       // Prepara para luego quitar una vida
        END
        FRAME;
    UNTIL (out_region(id,0))

END

//------------------------------------------------------------------------------
// Proceso enemigo
// Proceso para las naves enemigas
// 1-crea el enemigo y lo pone en la posici¢n dentro del escuadr¢n.
// 2-controla la acci¢n del enemigo cuando la variable indique que baje.
// Empieza con abajo=0 (Quedarse en la posici¢n de escuadr¢n).
//------------------------------------------------------------------------------

PROCESS enemigo(numero_enemigo,enemigo_tipo);

PRIVATE
    fila;               // Filas y columnas del escuadr¢n
    col;
    num_aleatorio;      // Para n£meros aleatorios
    id_proceso;         // Identificador de uso general.
    ahora=0;            // Numero de animaci¢n
    animacion[]=        // Configuraci¢n de la animaci¢n
        8,9,10,11,10,9;
    abajo=0;            // 0=Posicion de escuadr¢n, 1=ataque,
                        // 3=retorna a la posici¢n de escuadr¢n
    velocidad;          // Velocidad horizontal cuando atacan

BEGIN
    numero=numero_enemigo;

    fila=numero/ancho_escuadron;    // Fila y columna de la posici¢n en escuadr¢n
    col=numero MOD ancho_escuadron;

    x=col*ancho_nave+x_escuadron;   // Selecciona posici¢n inicial
    y=fila*alto_nave+y_escuadron;

    SWITCH (enemigo_tipo)           // Selecciona el gr fico del enemigo
        case 1: graph=2; END
        case 2: graph=animacion[0]; END
        case 3: graph=12; END
    END

    LOOP
        IF (enemigo_tipo==1)        // Animaci¢n de enemigos tipo 1
            graph++;
            IF (graph==7) graph=2; END
        END

        IF (enemigo_tipo==2)        // Animaci¢n de enemigos tipo 2
            ahora++;
            IF (ahora>5) ahora=0; END
            graph=animacion[ahora];
        END

        IF (y<130)                  // Puede disparar si vuela bajo
            num_aleatorio=rand(0,1500);     // Porcentaje de disparo
            IF (enemigo_tipo==3 AND num_aleatorio<=nivel*2)
                fuego(x,y+4,18);
            END
            IF (enemigo_tipo==2 AND num_aleatorio<=nivel)
                fuego(x,y+4,16);
            END
        END

        IF (abajo==1)           // Ataca la nave
            y+=2;
            // Comprueba si ha salido de pantalla
            IF (out_region(id,0))
                // Cambia las coordenadas a arriba
                y=0;
                x=col*ancho_nave+x_escuadron;
                abajo=3;
            END
            IF (x>idnave.x)     // Va hacia la nave del jugador
                IF (velocidad>-enemigo_tipo-2) velocidad--; END
            ELSE
                IF (velocidad<enemigo_tipo+2) velocidad++; END
            END
            x+=velocidad;
        END

        IF (abajo==3)           // Retorna a la posici¢n de escuadr¢n
            y++;
            IF (y==fila*alto_nave+y_escuadron) abajo=0; END
            // Si hay menos enemigos que el nivel por 3 vuelve a bajar
            IF (enemigos<nivel*3) abajo=1; END
        END

        IF ((abajo==0) OR (abajo==3)) // Mueve al escuadr¢n
            num_aleatorio=rand(1,4000);
            // Aleatoriamente y seg£n el nivel hace que el proceso baje
            IF (num_aleatorio<nivel*2)
                abajo=1;
                IF (enemigo_tipo==3)
                    SOUND(ataque3,200,300);
                END
                IF (enemigo_tipo==1)
                    SOUND(ataque1,200,300);
                END
            END

            IF (dir==0)         // Si la direcci¢n es hacia la izquierda
                x-=movi_escuadron;

                // Cuando no se pueda mover a la izquierda cambia de direcci¢n
                IF (x<20) cambia_dir=1; END

            ELSE                // Si la direcci¢n es a la derecha
                x+=movi_escuadron;
                IF (x>300) cambia_dir=1; END // Cambia la direcci¢n
            END
        END

        // Mata la nave del jugador cuando colisione con ella
        id_proceso=collision(TYPE nave);
        IF (id_proceso)
            otra_vida=1;
            signal(id_proceso,s_kill);
        ELSE
            id_proceso=collision(TYPE misil);
            IF (id_proceso)  // Elimina la nave
                signal(id_proceso,s_kill);
                puntuacion+=10;                    // Y suma puntos
                // Comprueba lass vidas extras
                IF (puntuacion MOD 1000==0) vidas++; END
                bandera_misil=0;        // Deja misiles en preparado
            END
        END

        IF (id_proceso)                 // El enemigo muere
            enemigos--;
            tipo_enemigo[numero]=0;     // Quita al enemigo de la formaci¢n
            IF (enemigos==0) siguiente_nivel=1; END
            explosion(x,y,100);         // y elimina el proceso haciendo un sonido y una explosion
            sound(explo,80,100);
            signal(id,s_kill);
        END
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso explosion
// Genera una explosi¢n de forma aleatoria
//------------------------------------------------------------------------------

PROCESS explosion(x,y,size);

BEGIN
    graph=19;               // Selecciona gr fico
    REPEAT
        IF (size>50 AND rand(0,100)<size/8) // Crea otras explosiones
            CLONE
                x+=rand(-8,8);
                y+=rand(-8,8);
            END
        END
        size-=4;            // Cambia de tama¤o reducciendose
        FRAME;
    UNTIL (size<25)         // Continua hasta que sea demasiado peque¤o
END
//------------------------------------------------------------------------------
// Proceso creditos
// Imprime los mensajes de creditos
//------------------------------------------------------------------------------
PROCESS creditos()

PRIVATE
    fuentec;            // Fuente para los cr‚ditos

BEGIN
    // Apaga la pantalla
    fade_off();
    delete_text(all_text);  // Borra otros textos
    put_screen(1,2);        // Muestra la pantalla de presentacion

    // Carga la fuente de letras
    fuentec=load_fnt("galax\galax2.fnt");

    // Imprime textos
    write(fuentec,160,30,4,"- CREDITOS -");
    write(fuentec,160,60,4,"PROGRAMADOR: LUIS SUREDA");
    write(fuentec,160,80,4,"GRAFICOS: JOSE FERNANDEZ");
    write(fuentec,160,100,4,"SONIDOS: CARLOS ILLANA");
    write(fuentec,160,120,4,"COPYRIGHT 1997");
    write(fuentec,160,140,4,"DIV GAMES STUDIO");
    fade_on();  // Enciende la pantalla

    // Espera hasta que se pulse la barra espaciadora
    scan_code=0;
    WHILE (scan_code==0)
        FRAME;
    END
    fade_off();
    exit("Gracias por jugar!",0);   // Sale del juego
END
