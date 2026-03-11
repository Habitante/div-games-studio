PROGRAM alfawater2;
GLOBAL
    file1;
    idlancha;
BEGIN
    set_fps(35,4);
    file1=load_fpg("fpg\alfawtex.fpg");
    load_wld("alfaw.wld",file1);
//    restore_type=no_restore;
    idlancha=lancha();

    LOOP
        FRAME;
    END

END
PROCESS lancha()

PRIVATE
    tablagraph[]=64,
                    648,649,650,651,652,653,654,655,
                    656,657,658,659,660,661,662,663,
                    600,601,602,603,604,605,606,607,
                    608,609,610,611,612,613,614,615,
                    616,617,618,619,620,621,622,623,
                    624,625,626,627,628,629,630,631,
                    632,633,634,635,636,637,638,639,
                    640,641,642,643,644,645,646,647,
                    648,649,650,651,652,653,654,655,
                    656,657,658,659,660,661,662,663;


BEGIN
    ctype=c_m8;
    xgraph=&tablagraph;
//    graph=660;
//    go_to_flag(0);           // Coordenadas de inicio
    file=file1;
    start_mode8(id,0,0);
    x=5000;
    y=5000;
    LOOP
        FRAME;
    END

END
