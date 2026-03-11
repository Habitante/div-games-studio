COMPILER_OPTIONS _extended_conditions;

//------------------------------------------------------------------------------
//TITULO:      TOTAL BILLIARDS
//AUTOR:       DANIEL NAVARRO
//FECHA:       1/15/97
//------------------------------------------------------------------------------

PROGRAM spanish_billar;

CONST
    radio_bola=14;                      // Radio de la bola
    x_reg_mesa=62-radio_bola;           // Region de la mesa
    y_reg_mesa=104-radio_bola;
    ancho_reg_mesa=580-62+radio_bola*2; // Region de la ventana
    alto_reg_mesa=375-104+radio_bola*2;

    pos_toque_x=604;                    // Posicion de toque en efecto
    pos_toque_y=452;

GLOBAL
    puntuacion1;    // Numero de carambolas con la bola blanca
    puntuacion2;    // Numero de carambolas con la bola amarilla

    blanco;         // Identificadores de las bolas
    amarillo;
    rojo;

    ultimo;         // Controla la ultima colision
    quien;          // Identificador de la bola que colisiona
    turno;          // Controla turno de los jugadores

    sonido0;        // Identificadores de sonido
    sonido1;

    vtotal0;        // Velocidad total de la bola blanca
    vtotal1;        // Velocidad total de la bola amarilla
    modos[]="Apuntar","Efecto","Tiro";
    txt_puntuacion[]="7 Puntos","21 Puntos","40 Puntos";
    puntuacion=0;   // modos[] y txt_puntuacion[] texto que cambian
    puntuaciones[]=7,21,40;
    txt_modo;       // Identificador para txt_puntuacion[]
    modo;           // Identificador de modo
    id_efectos;     // Identificador de efectos
    efecto_vert;    // Efecto vertical
    efecto_horiz;   // Efecto horizontal
    choque_contraria=0; // Comprueba el choque con la bola contraria
    choque_roja=0;  // Comprueba el choque con la bola roja
LOCAL
    velocidad=0;    // Velocidad de bola
    avelocidad=0;   // Velocidad acumulada
    ang=0;          // Direccion

    x_resol;        // Coordenadas * 100
    y_resol;

    incr_x;         // X incremento
    incr_y;         // Y incremento

PRIVATE
     ngulo2=0;      // Movimiento de la bola
    inc_ ngulo2=0;

    opci¢n=0;       // Opci¢n del menu
    opciones[2];    // Identificador de texto
    fin_juego=0;    // Bandera. 1=Fin ded juego
    ultima_x;       // Ultimas posiciones del rat¢n
    ultima_y;
BEGIN
    set_mode(m640x480);
    set_fps(30,0);

    load_fpg("billar\billar.fpg");   // Carga graficos
    load_fpg("billar\b_menu.fpg");

    load_fnt("billar\billar.fnt");   // Carga letras
    load_fnt("billar\numeros.fnt");
    load_fnt("billar\titulo.fnt");
    load_fnt("billar\menu.fnt");

    sonido0=load_pcm("billar\billar0.pcm",0); // Carga sonidos
    sonido1=load_pcm("billar\banda.pcm",0);

    // Crea una ventana
    define_region(1,x_reg_mesa,y_reg_mesa,ancho_reg_mesa,alto_reg_mesa);

    LOOP
        // Carga la paleta para que los gr ficos salgan bien
        load_pal("billar\b_menu.fpg");
        put_screen(1,1);    // Pone la pantalla de fondo

        // Imprime mensajes
        write(3,320,4,1,"TOTAL BILLIARD");
        opciones[0]=write(4,400,320,0,"Empezar");
        opciones[1]=write(4,400,354,0,txt_puntuacion[puntuacion]);
        opciones[2]=write(4,400,388,0,"Salir al DOS");
        write(4,320,480,7,"Daniel Navarro  DIV Games Studio");

        // Selecciona el gr fico del taco para el cursor
        file=1;
        graph=2;

        // Renicia las variables del cursor
        y=342; incr_y=0;  ngulo2=0;
        opci¢n=0;

        fade_on();  // Enciende la pantalla

        REPEAT  // Menu
            ultima_y=mouse.y;   // Guarda la posici¢n del rat¢n para mirar si hay cambio
            x=17+get_distx(ang,16);
            IF ((ang+=pi/8)>pi)
                ang-=2*pi;
            END

            // Comprueba si se ha elegido una opci¢n
            IF (opci¢n<>0 AND ang<pi/15 AND ang>-pi/15)
                incr_x=400;
                // Repite hasta que desaparezca el texto
                REPEAT
                    move_text(opciones[opci¢n-1],incr_x+=16,incr_y);
                    FRAME;
                UNTIL (incr_x>640)  // Cambia texto
                // Se ha elegido la opcion de cambiar tipo de juego
                IF (opci¢n==2)
                    delete_text(opciones[1]);
                    puntuacion=++puntuacion%3;
                    opciones[1]=write(4,incr_x=640,incr_y,0,txt_puntuacion[puntuacion]);
                    REPEAT
                        move_text(opciones[1],incr_x-=16,incr_y);
                        FRAME;
                    UNTIL (incr_x==400)
                    opci¢n=0;
                    incr_y=0;
                ELSE
                    opci¢n-=4;
                END
            END
            IF (incr_y==0)          // Cuando el palo este sobre una opci¢n
                IF (key(_enter) OR mouse.left OR
                    key(_control) OR key(_space))    // Selecciona esa opci¢n
                    incr_y=y-22;
                    opci¢n=(y-342)/34+1;
                ELSE
                    IF ((key(_down) OR mouse.y>ultima_y) AND y<410)   // Cambia a otra opci¢n
                        incr_y=y+17;
                         ngulo2=pi/2;
                        inc_ ngulo2=-pi/8;
                    END
                    IF ((key(_up) OR mouse.y<ultima_y)  AND y>342)     // Cambia a otra opci¢n
                        incr_y=y-17;
                         ngulo2=-pi/2;
                        inc_ ngulo2=pi/8;
                    END
                END
            END

            IF (inc_ ngulo2<>0)       // Detiene el incremento vertical
                 ngulo2+=inc_ ngulo2;
                y=incr_y+get_disty( ngulo2,17);
                IF (inc_ ngulo2<0)
                    IF ( ngulo2<-pi/2)
                        y=incr_y+17;
                        incr_y=0;
                        inc_ ngulo2=0;
                    END
                ELSE
                    IF ( ngulo2>pi/2)
                        y=incr_y-17;
                        incr_y=0;
                        inc_ ngulo2=0;
                    END
                END
            END
            // comprueba la pulsacion de la tecla escape
            IF (key(_esc))
                opci¢n=-1;
            END
            FRAME;
        UNTIL (opci¢n<0)
        // Borra todo y apaga la pantalla
        fade_off();
        clear_screen();
        delete_text(all_text);
        graph=0;
        FRAME;

        IF (opci¢n==-1)     // Sale del programa
            exit("Gracias por jugar!!!",0);
            FRAME;
        END

        load_pal("billar\billar.fpg"); // Carga la paleta de colores

        // Imprime los textos necesarios
        write(1,12,6,0,"Jugador 1");
        write(1,628,6,2,"Jugador 2");
        write_int(2,115,0,0,&puntuacion1);
        write_int(2,525,0,2,&puntuacion2);

        // Pone los gr ficos de la mesa y la bola de efectos
        put(0,1,320,240);
        put(0,8,pos_toque_x,pos_toque_y);

        // Crea el proceso que maneja los efectos
        id_efectos=efecto();

        // Escribe el texto del turno
        write(1,12,428,0,"Jugador");
        write_int(1,98,428,0,&turno);
        fade_on();

        // Crea las bolas
        blanco=bola(3,320,240);
        amarillo=bola(4,128,240-48);
        rojo=bola(5,128,240+48);


        // Valores iniciales
        fin_juego=0;
        choque_contraria=0;
        choque_roja=0;
        puntuacion1=0;
        puntuacion2=0;
        ultimo=0;
        quien=0;
        turno=2;

        REPEAT  // Bucle principal
            // Comprueba que este parado todo y que no haya palo, es decir, el fin de la tirada
            IF (blanco.velocidad==0 AND amarillo.velocidad==0 AND rojo.velocidad==0 AND NOT get_id(TYPE palo))
                // Si no ha chocado con las dos bolas a la vez,
                // es que no ha sido una carambola
                IF (NOT (choque_contraria AND choque_roja))
                    // Cambia de turno
                    IF (turno==1)
                        turno=2;
                    ELSE
                        turno=1;
                    END
                ELSE
                    // Carambola conseguida
                    IF (turno)  // Mira quien jugaba en ese turno
                        // Mira los puntos obtenidos hasta ahora
                        IF (++puntuacion1==puntuaciones[puntuacion])
                            x=800;
                            quien=write(1,x,240,4,"Jugador uno gana");
                            FROM x=800 TO 319 STEP -8;
                                move_text(quien,x,240);
                                FRAME;
                            END
                            FROM x=0 TO 29;
                                FRAME;
                            END
                            fin_juego=1;
                        END
                    ELSE
                        // Comprueba y hace lo mismo que antes pero para el otro jugador
                        IF (++puntuacion2==puntuaciones[puntuacion])
                            x=800;
                            quien=write(1,x,240,4,"Jugador dos gana");
                            FROM x=800 TO 320 STEP -8;
                                move_text(quien,x,240);
                                FRAME;
                            END
                            FROM x=0 TO 29;
                                FRAME;
                            END
                            fin_juego=1;
                        END
                    END
                END

                // Comprueba si no ha acabado
                IF (NOT fin_juego)
                    // Pone el palo dependiendo del turno al lado de la bola apropiada
                    IF (turno)
                        palo(blanco);
                    ELSE
                        palo(amarillo);
                    END
                    choque_contraria=0;
                    choque_roja=0;
                END
            END

            // Si se pulsa la tecla escape sale del juego
            IF (key(_esc))
                fin_juego++;
            END
            FRAME;
        UNTIL (fin_juego)

        fade_off();
        signal(id,s_kill_tree);
        signal(id,s_wakeup);
        clear_screen();
        delete_text(all_text);
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso bola
// Maneja la bola
//------------------------------------------------------------------------------

PROCESS bola(graph,x,y);

PRIVATE
    id_colisiones;  // Identificador de colisiones
    dir_final;      // Direccion despues de la colision

    veloc_final_x;  // Velocidad final despues de la colision
    veloc_final_y;

    longitud;       // Longitud a un punto

    ultima_pos_x;   // Ultima posicion
    ultima_pos_y;

BEGIN

    sombra(6);      // Pone la sombra de la bola

    x_resol=x*100;  // Coge las coordenadas multiplicadas por 100
    y_resol=y*100;

    LOOP

        avelocidad+=velocidad;

        // Hasta que pare
        WHILE (avelocidad>100)

            avelocidad-=100;        // A cada paso decrementa la velocidad

            ultima_pos_x=x_resol;   // Guarda posicion
            ultima_pos_y=y_resol;

            // Mueve la bola
            incr_x=get_distx(ang,100);
            x_resol+=incr_x;
            incr_y=get_disty(ang,100);
            y_resol+=incr_y;

            // Comprueba el rebote con las bandas de manera vertical
            IF ((y_resol<=10400 AND incr_y<0) OR (y_resol>=37400 AND incr_y>0))
                velocidad=velocidad*7/10;               // Reduce la velocidad
                sound(sonido1,80*velocidad/4000,100);   // Hace un sonido de rebote
                // Cambia el  ngulo para que rebote
                ang=fget_angle(x,y,x+incr_x,y-incr_y)+efecto_horiz*(pi/96);
                // Quita cualquier efecto que existiera
                efecto_vert=80;
                efecto_horiz=0;
            END

            // Comprueba el rebote con las bandas de manera horizontal
            IF ((x_resol<=6200 AND incr_x<0) OR (x_resol>=57900 AND incr_x>0))
                velocidad=velocidad*7/10;
                sound(sonido1,30+50*velocidad/5000,100);
                ang=fget_angle(x,y,x-incr_x,y+incr_y)+efecto_horiz*(pi/96);
                efecto_vert=80;
                efecto_horiz=0;
            END

            // Carga las coordenadas reales del gr fico con las temporales
            x=x_resol/100;
            y=y_resol/100;

            // Comprueba si ha chocado con una bola
            IF (id_colisiones=collision(TYPE bola))
                // Mira si esta colision no esta mirada y no es con esta bola
                IF (ultimo<>id_colisiones+id OR quien<>id)
                    // Guarda los identificadores para proximas comprobaciones
                    quien=id;
                    ultimo=id_colisiones+id;

                    // Comprueba si ha chocado la bola blanca con la amarilla
                    IF (ultimo==blanco+amarillo)
                        choque_contraria=TRUE;
                    END

                    // Mira que bola tiene el turno
                    IF (turno)
                        // Y comprueba si choca con la roja
                        IF (id_colisiones+id==blanco+rojo)
                            choque_roja=TRUE;
                        END
                    ELSE
                        IF (id_colisiones+id==amarillo+rojo)
                            choque_roja=TRUE;
                        END
                    END

                    sound(sonido0,512,256); // Sonido de colision

                    avelocidad+=100;
                    // Las bolas rebotan
                    // Primero coge los incrementos horizontal y vertical de una bola
                    incr_x=get_distx(ang,velocidad);
                    incr_y=get_disty(ang,velocidad);

                    // Y los mismo incrementos de la otra
                    id_colisiones.incr_x=get_distx(id_colisiones.ang,id_colisiones.velocidad);
                    id_colisiones.incr_y=get_disty(id_colisiones.ang,id_colisiones.velocidad);

                    // Suma la velocidades, que es lo mismo que la fuerza total
                    vtotal0=velocidad+id_colisiones.velocidad;

                    // Halla el angulo que hay entre las dos bolas que chocan
                    dir_final=get_angle(id_colisiones)+efecto_horiz*(pi/96);

                    // Suma los incrementos sacando dos que es igual a la velocidades
                    veloc_final_x=incr_x+id_colisiones.incr_x;
                    veloc_final_y=incr_y+id_colisiones.incr_y;

                    // Halla la longitud del vector entre las dos bolas,
                    // que es proporcional a la suma de las velocidades
                    longitud=fget_dist(x,y,x+veloc_final_x,y+veloc_final_y)*efecto_vert/100;

                    // Borra los efectos una vez que han colisionado
                    efecto_vert=80;
                    efecto_horiz=0;

                    // A la bola que choca se le resta el vector
                    // total del choque
                    incr_x-=get_distx(dir_final,longitud);
                    incr_y-=get_disty(dir_final,longitud);

                    // Y se hallan los nuevos valores para dicha bola
                    ang=fget_angle(x,y,x+incr_x,y+incr_y);
                    velocidad=fget_dist(x,y,x+incr_x,y+incr_y);

                    // A la bola que recibe el choque se le suma
                    // el vector total que ha salido del choque
                    id_colisiones.incr_x+=get_distx(dir_final,longitud);
                    id_colisiones.incr_y+=get_disty(dir_final,longitud);

                    // Y se actualizan sus valores de  ngulo y velocidad
                    id_colisiones.ang=fget_angle(x,y,x+id_colisiones.incr_x,y+id_colisiones.incr_y);
                    id_colisiones.velocidad=fget_dist(x,y,x+id_colisiones.incr_x,y+id_colisiones.incr_y);

                    // Por ultimo se hace una media con la fuerza inicial
                    // para que en vez de ser proporcional, sea identica
                    vtotal1=velocidad+id_colisiones.velocidad;
                    velocidad=vtotal0*velocidad/vtotal1;
                    id_colisiones.velocidad=vtotal0*id_colisiones.velocidad/vtotal1;

                    avelocidad=velocidad;
                    id_colisiones.avelocidad=id_colisiones.velocidad;

                END

                x_resol=ultima_pos_x;     // Restaura posicion
                y_resol=ultima_pos_y;

            ELSE
                // Si el identificador es el de la propia bola
                // borra cualquier rese¤a del choque
                IF (quien==id)
                    ultimo=0;
                END

            END

            FRAME(0);       // Refresca pantalla

        END

        x=x_resol/100;
        y=y_resol/100;

        FRAME;
        // Parando las bolas
        IF (velocidad>10)
            velocidad-=10;
            IF (efecto_vert>80)
                efecto_vert-=2;
                velocidad+=(efecto_vert-80)/10;
            END
            IF (efecto_vert<80)
                efecto_vert++;
                velocidad-=(80-efecto_vert)/2;
            END
        ELSE
            // Se ha parado completamente
            velocidad=0;
        END

    END
END

//------------------------------------------------------------------------------
// Proceso sombra
// Muestra las sombras de la bola y el palo
//------------------------------------------------------------------------------

PROCESS sombra(graph);

BEGIN
    region=1;               // Elige la regi¢n
    z=1;                    // Lo pone por debajo de las bolas
    flags=4;                // Lo pone transparente
    priority=-1;            // Hace que se ejecute despues de la bola
    LOOP
        angle=father.angle; // Coge el angulo del padre
        x=father.x+8;       // Sigue al proceso que lo llamo
        y=father.y+8;       // el palo o la bola
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso palo
// Controla el palo
//------------------------------------------------------------------------------
PROCESS palo(idbola);

PRIVATE
    ultima_pos_rat¢n;   // Ultima posicion del raton
    real_x_rat¢n;       // Coordenadas reales del raton
    real_y_rat¢n;
    fuerza;             // Fuerza de lanzamiento
    distancia;          // Para coger la distancia
     ngulo_res;         // Para coger el angulo

BEGIN
    sombra(7);      // Crea la sombra del palo

    // Imprime el modo de juego
    txt_modo=write(1,114,428,0,modos[modo=0]);

    mouse.x=320;    // Centra el raton
    mouse.y=240;
    graph=2;        // Elige el gr fico del palo
    ultima_pos_rat¢n=mouse.x;
    // Coge las coordenadas respecto a la bola que se¤ala
    x=idbola.x;
    y=idbola.y;
    angle=-pi/2;    // Angulo inicial
    // Posicion inicial de los efectos
    id_efectos.x=pos_toque_x;
    id_efectos.y=pos_toque_y;

    LOOP
        // Esta en el modo de apuntar con el taco
        IF (modo==0)
            // Actualiza la posici¢n del taco con la del rat¢n
            angle-=(ultima_pos_rat¢n-mouse.x)*(pi/256);
            ultima_pos_rat¢n=mouse.x;
            // Hace que el raton este entre 512 y 512
            IF (mouse.x<128)
                mouse.x+=320;
                ultima_pos_rat¢n=mouse.x;
            END
            IF (mouse.x>512)
                mouse.x-=320;
                ultima_pos_rat¢n=mouse.x;
            END
            // Comprueba si se ha pulsado el bot¢n del rat¢n
            IF (mouse.left)
                // Borra el texto antiguo y pone el nuevo
                delete_text(txt_modo);
                txt_modo=write(1,114,428,0,modos[1]);
                // Centra el rat¢n
                mouse.x=320;
                mouse.y=240;
                // Espera a que se suelte el bot¢n del rat¢n
                REPEAT
                    FRAME;
                UNTIL (NOT mouse.left)
                // Cambia el modo de tratar el taco
                modo=1;
                // Coge las coordenadas del rat¢n y las guarda en variables propias
                real_x_rat¢n=mouse.x;
                real_y_rat¢n=mouse.y;
            ELSE
                // Si no se pulsa el rat¢n crea el visor
                visor(idbola,angle+pi);
            END
        END
        // Modo en el que se selecciona efecto
        IF (modo==1)

            // Actualiza el afecto dependiendo de las coordenadas del rat¢n
            id_efectos.x=pos_toque_x+(mouse.x-real_x_rat¢n)/3;
            id_efectos.y=pos_toque_y+(mouse.y-real_y_rat¢n)/3;

            // Coge la distancia del centro de la bola a la posici¢n del efecto
            distancia=fget_dist(pos_toque_x,pos_toque_y,id_efectos.x,id_efectos.y);

            // Si la longitud es mayor que 22 la trunca para hacerlo redondo
            IF (distancia>22)
                 ngulo_res=fget_angle(pos_toque_x,pos_toque_y,id_efectos.x,id_efectos.y);
                id_efectos.x=pos_toque_x+get_distx( ngulo_res,22);
                id_efectos.y=pos_toque_y+get_disty( ngulo_res,22);
            END

            // Mira si se pulsa el bot¢n del rat¢n
            IF (mouse.left)

                // Borra el texto antiguo
                delete_text(txt_modo);

                // Pone el texto nuevo
                txt_modo=write(1,114,428,0,modos[2]);

                // Centra el rat¢n
                mouse.x=320;
                mouse.y=240;

                // Espera a que se suelte el bot¢n del rat¢n
                REPEAT
                    FRAME;
                UNTIL (NOT mouse.left)
                // Actualiza las variables del proceso
                ultima_pos_rat¢n=mouse.y;
                real_x_rat¢n=x;
                real_y_rat¢n=y;
                // Cambia el modo de acci¢n del taco
                modo=2;
                // Guarda los efectos elegidos en sus variables
                efecto_vert=id_efectos.y-pos_toque_y;
                IF (efecto_vert<0)
                    efecto_vert=80-28*-efecto_vert/22;
                ELSE
                    efecto_vert=80+efecto_vert*120/22;
                END
                efecto_horiz=id_efectos.x-pos_toque_x;
            END
        END
        // Modo de taco de seleccion de fuerza
        IF (modo==2)
            // Coloca el taco en relaci¢n a las coordenadas del rat¢n
            x=real_x_rat¢n+get_distx(angle,mouse.y-240);
            y=real_y_rat¢n+get_disty(angle,mouse.y-240);
            // Si se pasa de un limite significa que se ha hecho el tiro
            IF (mouse.y<232)
                // Borra el texto de modo
                delete_text(txt_modo);
                // Hace un sonido
                sound(sonido1,100,128);
                // Halla la fuerza, dependiendo de la £ltima posici¢n del rat¢n
                fuerza=(ultima_pos_rat¢n-mouse.y)*100;
                // Comprueba si la fuerza se pasa de los limites permitidos
                IF (fuerza<200)
                    fuerza=200;
                END
                IF (fuerza>8000)
                    fuerza=8000;
                END
                // Guarda la fuerza como velocidad de la bola
                idbola.velocidad=fuerza;
                // Y pone el  ngulo apropiado a la bola
                idbola.ang=angle+pi;
                // Borra todo y se prepara para salir del proceso
                ultimo=0;
                signal(id,s_kill);
                signal(son,s_kill);
            END
            // Comprueba si se pulsa el bot¢n del rat¢n
            IF (mouse.left)
                // Borra el texto antiguo y pone el nuevo
                delete_text(txt_modo);
                txt_modo=write(1,112,428,0,modos[0]);
                // Actualiza la posici¢n del taco al lado de la bola
                x=idbola.x;
                y=idbola.y;
                // Centra el rat¢n
                mouse.x=320;
                mouse.y=240;
                // Espera hasta que se suelte el bot¢n del rat¢n
                REPEAT
                    FRAME;
                UNTIL (NOT mouse.left)
                // Cambia de modo
                ultima_pos_rat¢n=mouse.x;
                modo=0;
            ELSE
                // Actualiza la £ltima posici¢n del rat¢n
                ultima_pos_rat¢n=mouse.y;
            END
        END
        FRAME;
    END
END

//------------------------------------------------------------------------------
// Proceso visor
// Muestra donde ira la bola
//------------------------------------------------------------------------------

PROCESS visor(idbola,ang);

PRIVATE
    impacto=0;        // Bandera. 1=impacto 0=no impacto
    id_bola2;         // Identificador de la bola

BEGIN
    z=2;
    // Inicialmente coge el gr fico de la bola para comprobar colisiones
    graph=idbola.graph;
    // Tambi‚n coge las coordenadas de la bola que es lanzada
    x_resol=idbola.x_resol;
    y_resol=idbola.y_resol;
    REPEAT

        // Mueve el visor un punto en el  ngulo deseado
        incr_x=get_distx(ang,100);
        x_resol+=incr_x;
        incr_y=get_disty(ang,100);
        y_resol+=incr_y;

        // La coloca en la nueva posici¢n
        x=x_resol/100;
        y=y_resol/100;

        // Testea limites de la mesa
        IF ((y_resol<=10400 AND incr_y<0) OR (y_resol>=37400 AND incr_y>0) OR (x_resol<=6200 AND incr_x<0) OR (x_resol>=57900 AND incr_x>0))
            impacto=1;
        END

        // Va comprobando si colisiona con cualquiera otra bola
        WHILE (id_bola2=collision(TYPE bola))
            IF (id_bola2<>idbola)
                impacto=1;
            END
        END
        FRAME(0);   // Hace que no aparezca en pantalla
    UNTIL (impacto) // Busca la posicion del visor hasta que colisiones
    graph=9;        // Selecciona ya el gr fico propio del visor
    FRAME;          // Ahora ya puede mostrar el gr fico del visor
END

//------------------------------------------------------------------------------
// Proceso efecto
// Muestra la posicion del toque
//------------------------------------------------------------------------------

PROCESS efecto();

BEGIN
    // Coge las coordenadas de la posici¢n de toque
    x=pos_toque_x;
    y=pos_toque_y;
    graph=6;        // Selecciona el gr fico
    size=29;        // Y cambia el tama¤o porque es muy grande
    LOOP
        FRAME;
    END
END

