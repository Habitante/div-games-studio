program ejemplo;
global
    n=100;
    texanim;
     angmio;
     fluor;
     misuelo,mitecho,estado_puerta,contador_puerta;

begin

  set_mode(m640x480);

  //set_fps(100,0);

  load_fpg("texturas.fpg");

  load_wld("zeen.wld",0);

  load_fpg("sprites.fpg");
  load_fpg("robotnew.fpg");
  //load_fpg("enemigos.fpg");
  load_fpg("marcador.fpg");
  load_fpg("explosio.fpg");
  load_fpg("disparos.fpg");


   restore_type=no_restore;

  jugador(0);
  pinta_marcador();

    loop
      if (key(_1))
        set_fog(20,40);

      end
      if (key(_2))
        set_fog(0,0);
      end
                          //    LAVA ииииииииииииииииииии
      from texanim= 84 to 93;
        set_sector_texture(210,texanim+10,9,-1);
        frame(n);
        set_sector_texture(212,texanim+10,9,-1);
        frame(n);
        set_sector_height(212,440+get_distx(angmio,12),725);
        angmio+=3000;

                      // fluorescente jodido ииииииииииииии

        fluor=rand(1,100);
        if  (fluor>20)
        set_wall_texture(1331,69,-1);
        set_wall_texture(1927,69,-1);
        set_sector_texture(231,69,5,1);
        else set_wall_texture(1331,70,-1);
        set_wall_texture(1927,70,-1);
        set_sector_texture(231,43,5,1);

        end

        frame;
      end
   end
end

process jugador(i)
private ang;
        vel=0;
        velang=0;

 begin
  start_mode8(id,i,i);
  m8[i].height=32;  // Altura de los ojos
  m8[i].angle=0;    // Mirar hacia arriba/abajo

  ctype=c_m8;       // Es un proceso de modo 8
  height=64;        // Altura del pibe
  radius=48;        // Distancia hasta las parades

  write_int(0,0,0,0,&x);
  write_int(0,0,10,0,&y);
  write_int(0,0,20,0,&z);

  write_int(0,0,40,0,&fps);

 // x=5500;
 // y=5500;
 // frame;

  go_to_flag(0);           // Coordenadas de inicio


  cell();
  cell2();
  cell3();
  robotor();
  explosion();


  loop
    if (key(_alt))
      if (key(_right))
//        x_advance(15,angle+90000);
      end
      if (key(_left))
//        x_advance(15,angle-90000);
      end
    else              //girar izq y der con aceleracion.....
      if (key(_right) and velang>-8000) velang-=300; end
      if (key(_left) and velang<8000) velang+=300; end
    end

  abre_puerta(23,125,16,1256,1000); //puerta,zona1,zona2,altura y suel
    ctype=c_m8;       // Es un proceso de modo 8

                      // desaceleracion..........
    if (!key(_right) and !key(_left))
      if (velang>1000) velang-=2000;
      else
        if (velang<-1000) velang+=2000;
        else velang=0;
        end
      end
    end

    angle+=velang;
                      //adelante y atras con aceleracion.....
    if (key(_up) and vel<48) vel+=6; end
    if (m8_wall!=-1 and key(_down)) vel=0; end
    if (key(_down) and vel>-32) vel-=4; end
                       // desaceleracion....................
    if (!key(_up) and !key(_down))
    //if (m8_wall!=-1)
     // vel=0;
      //else
      if (vel>2) vel-=4;
      else
        if (vel<-4) vel+=4;
        else vel=0;
        end
      end
    //end
    end

    advance(vel);
                     // subir y bajar...........
    if (key(_q)) z+=16; end
    if (key(_a)) z-=16; end

                     // mirar arriba y abajo.......
    if (key(_w)) m8[i].angle+=4; end
    if (key(_s)) m8[i].angle-=4; end
                    // cabeceo al avanzar y retroceder.......
  m8[i].angle=-vel;

    if (key(_r))     // strafe...................
        x=x+get_distx(angle-90000,30);
        y=y+get_disty(angle-90000,30);
    end
    if (key(_e))
        x=x+get_distx(angle+90000,30);
        y=y+get_disty(angle+90000,30);
    end
                      //movimiento de flotacion.........
   m8[i].height=32+get_distx(ang,7);
   IF (key(_space))
        disparo(x,y,angle,z);
   END
   frame;
    ang+=5000;


  end
end

process kk()
begin
end

PROCESS cell()

private
    tablagraph[]=20,
                     1, 2, 3, 4, 5, 6, 7, 8, 9,10,
                    11,12,13,14,15,16,17,18,19,20;

begin
    file=1;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(1);           // Coordenadas de inicio

    LOOP
        angle+=9000;
        FRame;
    end
end
PROCESS cell3()

private
    tablagraph[]=20,
                     41, 42, 43, 44, 45, 46, 47, 48, 49,50,
                    51,52,53,54,55,56,57,58,59,60;


begin
    file=1;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(3);           // Coordenadas de inicio

    LOOP
        angle+=9000;
        FRame;
    end
end



PROCESS cell2()

private
    tablagraph[]=20,
                     21, 22, 23, 24, 25, 26, 27, 28, 29,30,
                    31,32,33,34,35,36,37,38,39,40;


begin
    file=1;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(2);           // Coordenadas de inicio

    LOOP
        angle+=9000;
        FRame;
    end
end


PROCESS robotor()

private
    tablagraph[]=40,
                     1, 2, 3, 4, 5, 6, 7, 8, 9,10,
                    11,12,13,14,15,16,17,18,19,20,
                    21,22,23,24,25,26,27,28,29,30,
                    31,32,33,34,35,36,37,38,39,40;


begin
    file=2;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(4);

                 // Coordenadas de inicio

      radius=80;


   LOOP


      if (key(_i)) x++; advance(8); end
      if (key(_k)) x--; advance(-8);end
      if (key(_j)) angle+=9000; end
      if (key(_l)) angle-=9000; end

        //angle+=9000;
       FRame;
    end
end


PROCESS explosion()

private
    tablagraph[]=40,
                    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
                    21, 22, 23, 24, 25, 26, 27, 28, 29,30,
                    31,32,33,34,35,36,37,38,39,40;


begin
    file=4;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(7);           // Coordenadas de inicio

    LOOP
       angle+=9000;
        FRame;
    end
end



PROCESS marcador(x,y,graph)
private ang;

begin
   file=3;
   LOOP
      y-=get_distx(ang,7);
      FRame;
      y+=get_distx(ang,7);
      ang+=5000;
   end
end

PROCESS pinta_marcador()
  PRIVATE a,b;
begin
a=45-68;
b=436-54;
  //marcadr de abajo
  marcador(45,436,6);
  marcador(291,454,7);
  marcador(572,429,8);
  //tubo de goma
  marcador(632,301,5);
  //barra izq
  marcador(45,27,1);
  marcador(18,141,2);
  //barra der
  marcador(597,26,3);
  marcador(624,138,4);

   // luz_encendido
       marcador(a+58,b+59,9);
   // barra amarilla
       marcador(a+190,b+63,10);
   // barra roja
       marcador(a+182,b+80,11);
   // barra azul
      marcador(a+173,b+97,12);
   // ammo1
      marcador(a+511,b+77,13);
   // ammo2
      marcador(a+529,b+77,14);
   // panel luces
      marcador(a+566,b+29,15);
      marcador(a+590,b+29,16);
      marcador(a+575,b+44,17);
      marcador(a+605,b+43,18);
      marcador(a+587,b+55,19);
      marcador(a+602,b+48,20);

end


process abre_puerta(puerta,zona1,zona2,altura,suelo)

begin

    if (father.m8_sector==zona1 or father.m8_sector==zona2 or father.m8_sector==puerta)
         get_sector_height(puerta,&misuelo,&mitecho);
         IF (estado_puerta==0 and mitecho<altura-128)   // si esta parada.....
            estado_puerta=1;
            contador_puerta=mitecho;
         end
    else
         get_sector_height(puerta,&misuelo,&mitecho);
         if (estado_puerta==0 and mitecho>=altura)
            estado_puerta=-1;
            contador_puerta=mitecho;
         END
    END

    IF (estado_puerta)
       contador_puerta+=estado_puerta*4; //multiplicador es la velocidad
       set_sector_height(puerta,suelo,contador_puerta);
       IF (estado_puerta==1)
           set_wall_texture(147,18,-1);
           set_sector_texture(16,12,7,-1);
       end
       IF (contador_puerta>=altura)
          estado_puerta=0;

        ELSE
       IF (contador_puerta<=suelo)
           estado_puerta=0;
           set_wall_texture(147,18,7);
           set_sector_texture(16,33,7,7);
       END
    END
    end
 END



PROCESS disparo(x,y,angle,z)

 begin
   z-=100;
   file=5;
      ctype=c_m8;
    graph=9;

    WHILE (m8_wall==-1)
        advance(120);
        FRAME;
    END
    file=4;
    FROM graph=1 TO 40;
        FRAME;
    END
END