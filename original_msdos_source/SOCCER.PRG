
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
END

//-------------------------------------------------------------------
// Proceso pintaopciones1
// Pone los texto y gr ficos de men£ de opciones principal
//-------------------------------------------------------------------
PROCESS pintaopciones1()
BEGIN
END
//-------------------------------------------------------------------
// Proceso eligeopciones1
// Comprueba si se ha cambiado alguna opci¢n
//-------------------------------------------------------------------

PROCESS eligeopciones1()

PRIVATE
    idcollisionmouse=0;

BEGIN
END

//-------------------------------------------------------------------
// Proceso opciones2
// Pone los textos y gr ficos de la pantalla de seleccionar nombres
//-------------------------------------------------------------------

PROCESS opciones2 ()

BEGIN
END

//-------------------------------------------------------------------
// Proceso ponletras
// Pone el gr fico de las letras del teclado en la pantalla de
// cambiar nombres
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS ponletras (x,y,z,graph)

BEGIN
END

//-------------------------------------------------------------------
// Proceso pontiempo
// Pone el gr fico de selecci¢n de tiempo en la pantalla de opciones
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS pontiempo (x,y,graph)

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso titulofutbol97
// Pone y mueve el titulo de la intro que pone FUTBOL 97'
//-------------------------------------------------------------------

PROCESS titulofutbol97()

PRIVATE
    solouno=0;

BEGIN
END

//-------------------------------------------------------------------
// Proceso titulosoccer
// Pone el gr fico de titulo Soccer de la pantalla introductoria
//-------------------------------------------------------------------

PROCESS titulosoccer()

BEGIN
END

//-------------------------------------------------------------------
// Proceso sombrasoccer
// Pone la sombra de detraes de soccer
//-------------------------------------------------------------------

PROCESS sombrasoccer()

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso poncontrol
// Pone el gr fico de los controles en la pantalla de selecci¢n
//-------------------------------------------------------------------

PROCESS poncontrol(x,y,graph)

BEGIN
END

//-------------------------------------------------------------------
// Proceso textopulsaunatecla
// Pone el gr fico 'press any key' de la pantalla introductoria
//-------------------------------------------------------------------

PROCESS textopulsaunatecla()

PRIVATE
    pausatexto=0;       // Contador para que el texto parpadee

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso pontecla
// Pone el gr fico de la tecla pulsada
// Entradas: cualtecla = N£mero de la tecla a imprimir
//-------------------------------------------------------------------

PROCESS pontecla (cualtecla)

BEGIN
END

//-------------------------------------------------------------------
// Proceso poncursor
// Pone el gr fico del cursor en la pantalla de editar nombres
// Entradas: Variables predefinidas
//-------------------------------------------------------------------

PROCESS poncursor (x,y,graph)

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso valla
// Pone el gr fico del publico
//-------------------------------------------------------------------

PROCESS valla()

BEGIN
END

//-------------------------------------------------------------------
// Proceso tu_porteria
// Pone el gr fico de tu porter¡a
//-------------------------------------------------------------------

PROCESS tu_porteria()

BEGIN
END

//-------------------------------------------------------------------
// Proceso palo1
// Pone el palo de tu porter¡a
//-------------------------------------------------------------------

PROCESS palo1()

BEGIN
END

//-------------------------------------------------------------------
// Proceso su_porteria
// Pone la porter¡a del ordenador
//-------------------------------------------------------------------

PROCESS su_porteria()

BEGIN
END

//-------------------------------------------------------------------
// Proceso palo2
// Pone el gr fico del palo de la porter¡a
//-------------------------------------------------------------------

PROCESS palo2()
BEGIN
END

//-------------------------------------------------------------------
// Proceso bal¢n
// Maneja el bal¢n durante el juego
//-------------------------------------------------------------------

PROCESS bal¢n()

PRIVATE

    incrx,incry; // Incrementos x e y del bal¢n

BEGIN
END

//-------------------------------------------------------------------
// Proceso sombrabalon
// Pone la sombra del bal¢n
//-------------------------------------------------------------------

PROCESS sombrabalon()

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso flecha
// Pone el gr fico que se¤ala al desmarcado en el juego
// Entradas: tipo = N£mero de equipo
//-------------------------------------------------------------------

PROCESS flecha(tipo)

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso minicampo
// Imprime el minicampo que aparece de marcador en el juego
//-------------------------------------------------------------------

PROCESS minicampo()

BEGIN
END

//-------------------------------------------------------------------
// Proceso pintapunto
// Pinta los puntos de los jugadores y el bal¢n en el minimapa
// Entradas: cual = n£mero del punto que se quiere pintar
//-------------------------------------------------------------------

PROCESS pintapunto(cual)

BEGIN
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
END

//-------------------------------------------------------------------
// Proceso daquienpasa
// Da el n£mero de jugador que esta mas solo para pasarle la pelota
// Entradas: equipo = N£mero del equipo
// Salidas: mascerca= N£mero del jugador a quien pasar
//-------------------------------------------------------------------

PROCESS daquienpasa(equipo)

PRIVATE

    distanciasmin[10]=11 dup (0);              // Guarda la distancias de los jugadores
                                    // a los contrarios que tengan mas cerca
    mascerca;                       // Guarda los que van teniendo mas que los otros
    masadelantado;

BEGIN
END

