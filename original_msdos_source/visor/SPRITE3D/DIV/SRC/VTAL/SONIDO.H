/*************************************************************/
/* Inicializa el sonido                                      */
/*                                                           */
/*************************************************************/
/* Retorna: 1 si todo va bien                                */
/*         -1 si vamos jodido                                */
/*************************************************************/
int InitSound(void);

/*************************************************************/
/* Carga un sonido                                           */
/*                                                           */
/*************************************************************/
/* Retorna: n numero de sonido                               */
/*         -1 si vamos jodido                                */
/*************************************************************/
int LoadSound(char *SndName,int Loop);

/*************************************************************/
/* Descarga un sonido                                        */
/*                                                           */
/*************************************************************/
/* Entrega: Sonido a descargar                               */
/* Retorna:-1 si vamos jodido                                */
/*                                                           */
/*************************************************************/
int UnloadSound(int NumSonido);

/*************************************************************/
/* Toca un sonido                                            */
/*                                                           */
/*************************************************************/
/* Entrega: Sonido a tocar                                   */
/*          Volumen de sonido                                */
/*          Si quiero loop                                   */
/* Retorna: n de canal por el que toca                       */
/*         -1 si vamos jodido                                */
/*************************************************************/
int PlaySound(int NumSonido,int Volumen,int Frec);

/*************************************************************/
/* Para un sonido                                            */
/*                                                           */
/*************************************************************/
/* Entrega: Sonido a parar                                   */
/* Retorna:-1 si vamos jodido                                */
/*                                                           */
/*************************************************************/
int StopSound(int NumChannel);

/*************************************************************/
/* Cambia la frecuencia y el volumen del sonido              */
/*                                                           */
/*************************************************************/
/* Entrega: Sonido a cambiar                                 */
/*          Nueva frecuencia                                 */
/* Retorna:-1 si vamos jodido                                */
/*                                                           */
/*************************************************************/
int ChangeSound(int NumChannel,int Volumen,int Frec);

/*************************************************************/
/* Termina el motor de sonido                                */
/*                                                           */
/*************************************************************/
/* Retorna: Nada                                             */
/*                                                           */
/*************************************************************/
void EndSound(void);

/*************************************************************/
/* Necesario para que suene algo, porner en el loop del      */
/* programa                                                  */
/*************************************************************/
/* Retorna: Nada                                             */
/*                                                           */
/*************************************************************/
void LoopSound(void);

void WaitUntilFree(int NumChannel);
void SetTimer();
void SetPoll();
int ChannelFree(int NumChannel);
