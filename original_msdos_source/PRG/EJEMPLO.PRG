PROGRAM ejemplo_collision;

PRIVATE
    id2;

BEGIN
    load_fpg("help\help.fpg");
    put_screen(0, 1);
    graph=100;
    x=160;
    y=100;
    proceso_m¢vil(80, 50, 101);
    LOOP
        delete_text(all_text);
        write(0, 160, 0, 1, "Use los cursores para mover el tri ngulo");

        id2 = collision(TYPE proceso_m¢vil); // Se detecta la colisi¢n

        IF (id2<>0)
            write(0, 160, 200, 7, "­ DETECTADA COLISION !");
        END
        FRAME;
    END
END

PROCESS proceso_m¢vil(x, y, graph);

BEGIN
    LOOP
        IF (key(_right)) angle-=10000; END
        IF (key(_left)) angle+=10000; END
        IF (key(_up)) advance(4); END
        FRAME;
    END
END