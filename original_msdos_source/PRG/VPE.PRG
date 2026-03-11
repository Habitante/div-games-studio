program ejemplo;
global
    file1,file2,file3,file4;


begin

//  set_mode(m640x480);

  set_fps(25,0);

  file1=load_fpg("texturas.fpg");

  load_wld("zeen.wld",file1);
//  load_wld("Fernandi.wld",file1);

  file2=load_fpg("sprites.fpg");
  file3=load_fpg("enemigos.fpg");
  file4=load_fpg("marcador.fpg");

   restore_type=no_restore;

  jugador(0);
//  pinta_marcador();

    loop
//    if (key(_1))
//      set_fog(1,0,0,0);

//    end
//    if (key(_2))
//      set_fog(0,0,0,0);
//    end
    frame;
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
  x=14879;
  y=15218;
  angle=152000;

//  cell();
//  cell2();
//  cell3();
//  robotor();

  loop
    if (key(_alt))
      if (key(_right))
//        x_advance(15,angle+90000);
      end
      if (key(_left))
//        x_advance(15,angle-90000);
      end
    else              //girar izq y der con aceleracion.....
      if (key(_right) and velang>-8000) velang-=500; end
      if (key(_left) and velang<8000) velang+=500; end
    end
                      // desaceleracion..........
    if (!key(_right) and !key(_left))
      if (velang>500) velang-=500;
      else
        if (velang<-500) velang+=500;
        else velang=0;
        end
      end
    end

    angle+=velang;
                      //adelante y atras con aceleracion.....
    if (key(_up) and vel<48) vel+=6; end
    if (key(_down) and vel>-32) vel-=2; end
                       // desaceleracion....................
    if (!key(_up) and !key(_down))
      if (vel>2) vel-=2;
      else
        if (vel<-2) vel+=2;
        else vel=0;
        end
      end
    end

    advance(vel);
                     // subir y bajar...........
    if (key(_q)) z+=16; end
    if (key(_a)) z-=16; end
                     // mirar arriba y abajo.......
    if (key(_w)) m8[i].angle+=4; end
    if (key(_s)) m8[i].angle-=4; end

    m8[i].angle=-vel;

    if (key(_r))     // strafe...................
        x=x+get_distx(angle-90000,16);
        y=y+get_disty(angle-90000,16);
    end
    if (key(_e))
        x=x+get_distx(angle+90000,16);
        y=y+get_disty(angle+90000,16);
    end
                      //movimiento de flotacion.........
    m8[i].height=32+get_distx(ang,12);
    frame;
    ang+=5000;

  end
end
PROCESS cell()

private
    tablagraph[]=20,
                     1, 2, 3, 4, 5, 6, 7, 8, 9,10,
                    11,12,13,14,15,16,17,18,19,20;

begin
    file=file2;
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
    file=file2;
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
    file=file2;
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
    tablagraph[]=20,
                     1, 2, 3, 4, 5, 6, 7, 8, 9,10,
                    11,12,13,14,15,16,17,18,19,20;


begin
    file=file3;
      ctype=c_m8;       // Es un proceso de modo 8
    xgraph=&tablagraph;
      go_to_flag(4);           // Coordenadas de inicio

   LOOP
        angle+=9000;
       FRame;
    end
end

PROCESS marcador(x,y,graph)
private ang;

begin
   file=file4;
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
a=0;
b=0;
  //marcadr de abajo
  marcador(a+45,b+436,6);
  marcador(a+291,b+454,7);
  marcador(a+572,b+429,8);
  //tubo de goma
  marcador(a+632,b+301,5);
  //barra izq
  marcador(a+45,b+27,1);
  marcador(a+18,b+141,2);
  //barra der
  marcador(a+597,b+26,3);
  marcador(a+624,b+138,4);

end