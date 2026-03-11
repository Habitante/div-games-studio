program test;
global
    fuente;
    s1;
    numzana;
begin
    load_fpg("test.fpg");
    s1=load_pcm("pcm/libreria/animales/varios/comiendo.wav",0);
    set_mode(m1024x768);
    set_fps(60,0);
    put(0,3,512,384);
    blob(2);
    z=zana(4);
    fuente=load_fnt("nuevofnt.fnt");
    write_int(fuente,512,0,1,offset numzana);
    loop
        if (rand(0,100)==0)
            plane();
        end
        frame;
    end
end

process plane()
private
    ix;
begin
    ix=rand(0,1);
    if (ix==0) ix=-1; x=1024; flags=1; else x=0; end
    graph=6;
    size=25;
    y = rand(0,300);
    repeat
        if (collision(type blob))
            break;
        end
        x+=ix*8;
        frame;
    until (out_region(id,0));
    repeat
        angle = angle +1000;
        y=y+10;
        frame;
    until (out_region(id,0));
end


process blob(graph)
private
    i,inc=-100;
begin
    resolution=10;
    loop
        //angle=angle+1000;
        x=mouse.x*10;
        y=mouse.y*10+i;
        i+=inc;
        inc+=5;
        if (inc==105) inc=-100; end
        if (key(_esc)) exit("bye!",0); end
        if (mouse.left or mouse.right) size=300; end
        if (size>200) size-=5; end
        frame;
    end
end

process zana(graph);
private
    xx=50;
    yy=50;
    an=0;
begin
    loop
        angle = angle+500;
        an = fget_angle(x,y,xx,yy);
        if (fget_dist(x,y,xx,yy)<10)
            xx=rand(20,1004);
            yy=rand(20,748);
        else
            xadvance(an,2);
        end
        if (collision(TYPE blob))
            sound(s1,256,256);
            size=size-10;
            par(x,y);
            par(x,y);
            par(x,y);
            par(x,y);
            if (size<=25)
                size=100;
                x=rand(20,1004);
                y=rand(20,748);
                numzana=numzana+1;
            end
        end
        frame;
    end;
end

process par(x, y)
private
    ang;
begin
    size = rand(25,100);
    ang = rand(0,360000);
    graph = 5;
    repeat
        xadvance(ang,4);
        size-=5;
        frame;
    until (size<=0);
end
