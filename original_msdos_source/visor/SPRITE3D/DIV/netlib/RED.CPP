#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#define GLOBALS

#include "div.h"
#include "net.h"

int net_game_id;

// Importante: Para cada funci¢n se debe indicar el retval(int), y hacer
// siempre un getparm() por cada par metro de llamada (el retval() es
// imprescindible incluso si la funci¢n no necesita devolver un valor).

//-----------------------------------------------------------------------------
//  Inicializacion de la red (Luego no vale, es temporal)
//-----------------------------------------------------------------------------
void net_inicio()
{
  int net_init=getparm();
  net=(net_struct *)&mem[net_init];

  retval(0);
}

//-----------------------------------------------------------------------------
//  Inicializacion del identificador de juego
//-----------------------------------------------------------------------------
void net_create_game_id()
{
  net_game_id=0;

  for (int i=0;i<9;i++) {
    net_game_id+=mem[i];
  }
  net_game_id=abs(net_game_id);
  if (net_game_id<10500)
    net_game_id+=10500;

}

//-----------------------------------------------------------------------------
//  Entra en una de las partidas activas
//-----------------------------------------------------------------------------
void net_join_game()
{
  int game_id;

  int longitud=getparm();
  int datos=getparm();
  int nombre=getparm();

  game_id=net_game_id+nombre+mem[text_offset+nombre];

  int c=_net_join_game(game_id,(char *)&mem[text_offset+nombre],(void *)&mem[datos],longitud);

  retval(c);
}

//-----------------------------------------------------------------------------
//  Entra en una de las partidas activas
//-----------------------------------------------------------------------------
void net_get_games()
{
  int c=_net_get_games(net_game_id);

  retval(c);
}

//-----------------------------------------------------------------------------
//  Bucle central de la red
//-----------------------------------------------------------------------------
void post_process_buffer(void)
{
  _net_loop();
}

//-----------------------------------------------------------------------------
//  Se indica el nombre que tendr  la funci¢n en el lenguaje, el puntero a la
//  misma y el n£mero de par metros que tiene.
//-----------------------------------------------------------------------------
void __export divlibrary(LIBRARY_PARAMS) {
  COM_export("NET_INICIO",net_inicio,1);
  COM_export("NET_JOIN_GAME",net_join_game,3);
  COM_export("NET_GET_GAMES",net_get_games,0);
}

void __export divmain(COMMON_PARAMS) {
  GLOBAL_IMPORT();
  net_create_game_id();
  DIV_export("post_process_buffer",post_process_buffer);
}

void __export divend(COMMON_PARAMS) {
  net_end();
}
