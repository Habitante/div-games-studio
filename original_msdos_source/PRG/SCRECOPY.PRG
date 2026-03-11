program ejemplo_screen_copy;
global
    ang,ang2;
begin
    load_fpg("screcopy.fpg");
    set_mode(m640x480);
    define_region(1,80,0,480,480);
    graph=1;x=320;y=240;
    put(0,graph,x,y);
    loop
        screen_copy(1,0,1,0,0,480,480);
        ang+=1000; ang2+=1971;
        angle=get_distx(ang,25000);
        size=90+get_disty(ang2,30);
        frame;
    end
end
