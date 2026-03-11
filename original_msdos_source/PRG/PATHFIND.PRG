PROGRAM ejemplo_collision;
GLOBAL
    n;
    STRUCT tabla[100]
      x,y;
    END

    i;

BEGIN
    load_fpg("help\help.fpg");
    put_screen(0, 7);
    graph=100;
    LOOP
        x=mouse.x;
        y=mouse.y;
        n=path_find(0,0,201,2,160,100,&tabla,100);
        FOR (i=0;i<n;i++)
          proceso(tabla[i].x,tabla[i].y,200);
        END
        FRAME;
    END
END

PROCESS proceso(x, y, graph);

BEGIN
    FRAME;
END