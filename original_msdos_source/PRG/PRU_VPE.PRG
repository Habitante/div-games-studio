program ejemplo;

global
  ang,vel,velang;

begin
  load_fpg("rotika\fpg\texturas.fpg");
  load_wld("rotika\wld\prueba.wld",0);

  start_mode8(id,0,0);
  m8.height=32;  // Altura de los ojos
  m8.angle=0;    // Mirar hacia arriba/abajo

  ctype=c_m8;       // Es un proceso de modo 8
  height=64;        // Altura del pibe
  radius=48;        // Distancia hasta las parades

  write_int(0,0,0,0,&x);
  write_int(0,0,10,0,&y);
  write_int(0,0,20,0,&z);

  write_int(0,0,40,0,&fps);
  resolution=10;

  go_to_flag(0);           // Coordenadas de inicio

  loop
    if (key(_alt))
      if (key(_right))
        x_advance(150,angle+90000);
      end
      if (key(_left))
        x_advance(150,angle-90000);
      end
    else              //girar izq y der con aceleracion.....
      if (key(_right) and velang>-4000) velang-=300; end
      if (key(_left) and velang<4000) velang+=300; end
    end

   if (key(_space))
     debug;
     kk();
   end

//  abre_puerta(23,125,16,1256,1000); //puerta,zona1,zona2,altura y suel
//    ctype=c_m8;       // Es un proceso de modo 8

                      // desaceleracion..........
    if (!key(_right) and !key(_left))
      if (velang>300) velang-=300;
      else
        if (velang<-300) velang+=300;
        else velang=0;
        end
      end
    end

    angle+=velang;
                      //adelante y atras con aceleracion.....
    if (key(_up) and vel<480) vel+=60; end
    if (m8_wall!=-1 and key(_down)) vel=0; end
    if (key(_down) and vel>-320) vel-=40; end
                       // desaceleracion....................
    if (!key(_up) and !key(_down))
      if (vel>20) vel-=20;
      else
        if (vel<-20) vel+=20;
        else vel=0;
        end
      end
    end

    advance(vel);
                     // subir y bajar...........
    if (key(_q)) z+=160; end
    if (key(_a)) z-=160; end

                     // mirar arriba y abajo.......
    if (key(_w)) m8.angle+=4; end
    if (key(_s)) m8.angle-=4; end
                    // cabeceo al avanzar y retroceder.......
    m8.angle=-vel/10;

    if (key(_r))     // strafe...................
        x=x+get_distx(angle-90000,300);
        y=y+get_disty(angle-90000,300);
    end
    if (key(_e))
        x=x+get_distx(angle+90000,300);
        y=y+get_disty(angle+90000,300);
    end

   frame;

   z+=get_distx(ang,50);
   ang+=5000;
   z-=get_distx(ang,50);

  end
end

process kk()
begin
end
/*
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
*/
