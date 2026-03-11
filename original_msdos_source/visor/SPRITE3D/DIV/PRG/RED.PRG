program redwar;
global
  struct j[1]
    x,y,graph;
    size,angle;
    dis;
  end
  yo,myfont;

begin
  load_fpg("steroid.fpg");
  myfont=0;
  set_mode(m640x480);
  set_fps(35,4);
  // Inicializo la red
  net.dispositivo=1;
  net.com=1;
  net.velocidad=33600;
  net.num_players=2;
//  yo=net_get_games();

  yo=net_join_game("MiPartida",&j,sizeof(j));

  if (yo<0) exit("Error...",yo); end
  nave(yo);
  enemigo(1-yo);
end

process nave(i)
begin
  x=640*i;
  angle=i*pi;
  y=240;
  graph=1;
  write(myfont,0,0,0,"Game ID:");
  write_int(myfont,50,0,0,&net.tonos);
  write_int(myfont,0,20,0,&net.servidor);
  write_int(myfont,0,40,0,&yo);
  loop
    if (key(_right)) angle-=15000; end
    if (key(_left)) angle+=15000; end
    if (key(_up)) advance(8); end
    if (key(_space))
      dis(x,y,angle);
      j[i].dis=1;
    else
      j[i].dis=0;
    end
    j[i].x=x;
    j[i].y=y;
    j[i].angle=angle;
    j[i].size=size;
    j[i].graph=graph;
    frame;
  end
end

process dis(x,y,angle)
begin
  graph=2;
  advance(24);
  repeat
    advance(16);
    frame;
  until (out_region(id,0));
end

process enemigo(i)
begin
  loop
    x=j[i].x;
    y=j[i].y;
    graph=j[i].graph;
    size=j[i].size;
    angle=j[i].angle;
    if (j[i].dis)
      dis(x,y,angle);
      j[i].dis=0;
    end
    frame;
  end
end