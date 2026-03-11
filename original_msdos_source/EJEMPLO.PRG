PROGRAM ejemplo_start_mode7;

PRIVATE
    fichero1;

BEGIN
    fichero1=load_fpg("test.fpg");
    set_fps(60,0);

    start_mode7(0, fichero1, 7, 7, 0, 64);

    m7.height = 64;
    m7.distance = 32;
    m7.color = 162;
    m7.camera = id;

    write(0, 160, 0, 1, "Utilice los cursores para moverse");
    LOOP
        IF (key(_right)) angle-=8000; END
        IF (key(_left)) angle+=8000; END
        IF (key(_up)) advance(6); END
        IF (key(_down)) advance(-6); END
        FRAME;
    END
END