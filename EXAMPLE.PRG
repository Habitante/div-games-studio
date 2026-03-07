PROGRAM example_collision;

PRIVATE
    id2;

BEGIN
    load_fpg("help/help.fpg");
    put_screen(0, 1);
    graph=100;
    x=160;
    y=100;
    mobile_process(80, 50, 101);
    LOOP
        delete_text(all_text);
        write(0, 160, 0, 1, "Use the cursors to move the triangle");

        id2 = collision(TYPE mobile_process); // Collision detected

        IF (id2<>0)
            write(0, 160, 200, 7, "­ COLLISION DETECTED!");
        END
        FRAME;
    END
END

PROCESS mobile_process(x, y, graph);

BEGIN
    LOOP
        IF (key(_right)) angle-=10000; END
        IF (key(_left)) angle+=10000; END
        IF (key(_up)) advance(4); END
        FRAME;
    END
END