program doom;
global
  yo,myfont;
  world;
  fichero_grafico1;
  fichero_grafico2;
  i,tecla;
  coche_1[]=8,-12,-11,10,11,12,13,14,-13; // Graficos para xgraph
  coche_2[]=8,-22,-21,20,21,22,23,24,-23; // Son rotatorios en  ngulos de 45 grados
  coche_3[]=8,-32,-31,30,31,32,33,34,-33;
  coche_4[]=8,-42,-41,40,41,42,43,44,-43;

local tiempo;

begin
  set_mode(m320x200);
  set_fps(35,4);
  fichero_grafico2=load_fpg("vpe\zeen.fpg");
  fichero_grafico1=load_fpg("vpe\speed.fpg");

//  define_region(1,0,0,320,100);
//  define_region(2,0,100,320,200);

  load_wld("wld\zeen.wld",fichero_grafico2);

  jugador(0);
//  coche();

  i=0;
  tecla=0;

  loop
    if (key(_0))
      let_me_alone();
      load_wld("wld\x.wld",fichero_grafico2);
      jugador(0);
    end
    if (key(_1))
      let_me_alone();
      load_wld("wld\new.wld",fichero_grafico2);
      jugador(0);
    end
//    if (key(_1))
//      stop_mode8(2);
//    end
    if (key(_3))
      set_fog(1,100,100,100);
    end
    if (key(_4))
      set_fog(0,0,19,92);
    end
    if (key(_9))
      stop_mode8(2);
    end
    frame;
  end
end

process coche()
private
  old;
begin

  start_mode8(id,2,2);
  m8[i].height=24;
  m8[i].angle=0;
  z=0;
  angle=0;
  ctype=c_m8;
  height=16;
  radius=2;
  file=fichero_grafico1;
  xgraph=&coche_1;
  flags=0;

  go_to_flag(0);

  loop
    if (key(_o)) angle+=15000; end
    if (key(_p)) angle-=15000; end

    if (!key(_control)) old=1; end
    if (key(_control) && old)
      disparo(x,y,angle);
      old=0;
    end
//    advance(160);
//    angle+=4000;
    frame;
  end
end

process jugador(i)
private
  old;
  a,b;
begin
  start_mode8(id,i,i);
  m8[i].height=24;
  m8[i].angle=0;
  graph=10;
  file=fichero_grafico1;
  ctype=c_m8;
  height=16;
  radius=16;
  m8_step=0;
  resolution=1;

  go_to_flag(0);

  angle=210000;
//  x=14864;
//  y=15360;
  z=88;

  write_int(0,10,10,0,&m8_wall);
  write_int(0,10,20,0,&m8_sector);
  write_int(0,10,30,0,&m8_nextsector);

  loop
    if (key(_right)) angle-=5000; end
    if (key(_left)) angle+=5000; end
    if (key(_n)) x_advance(angle+90000,15); end
    if (key(_m)) x_advance(angle-90000,15); end
    if (key(_up)) advance(15); end
    if (key(_down)) advance(-15); end
    if (key(_q)) z+=16; end
    if (key(_a)) z-=16; end
    if (key(_w)) m8[i].angle+=4; end
    if (key(_s)) m8[i].angle-=4; end

    if (!key(_control)) old=1; end
    if (key(_control) && old)
      disparo(x,y,angle);
      old=0;
    end

    if (key(_u)) go_to_flag(0); end

    if (key(_0))
      go_to_flag(0);
    end
    if (key(_1))
      go_to_flag(1);
    end
    frame;
  end
end

process disparo(x,y,angle)
private
  id_hijo;
begin
  height=father.height;
//  x+=get_distx(angle,64);
//  y+=get_disty(angle,64);
  z=father.z;
  ctype=c_m8;
  radius=16;
  graph=10;
  file=fichero_grafico1;
  tiempo=0;
  id_hijo=sombra_disparo(x,y,angle);
  repeat
    advance(30);
    frame;
  until (tiempo++>100);
  signal(id_hijo,S_KILL);
end

process sombra_disparo(x,y,angle)
begin
  height=father.height;
  ctype=c_m8;
  radius=16;
  graph=10;
  file=fichero_grafico1;
  loop
    z=0;
    x=father.x;
    y=father.y;
    frame;
  end
end