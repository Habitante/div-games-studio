
// ***************************************************************************
//  OJO cuando se compile la version para el interprete // versus la del DIV
#define INTERPRETE
// ***************************************************************************

#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <stdlib.h>
#include "gamevtal.h"
#include "sonido.h"
#include "tarjetas.h"
#include "mixer.hpp"

#define CHANNELS 16
int BSONIDO=0;

GAME_TSetup     ssetup;
GAME_PRec       gamev;
PLAY_TInitData  sid;
GAME_PEffect    eff[128];
char            *Sonido[128];
int             channel[CHANNELS];
int             SLooping[128];
int             CLooping[CHANNELS];
SINS_TLoadRec   lrec;
int             canalact=0;

int InitSound(void)
{
int x;
int Tarj,Port,Irq,Dma2,Dma,master,sound_fx,cd_audio;
FILE *File_Cfg;

char *Deviceptr[8]={sb_1_5,sb_1_5,sb_2_0,sb_pro,sb_16,sb_awe,gus,gusmax};
int DeviceLen[8]={0,SB_1_5LEN,SB_2_0LEN,SB_PROLEN,SB_16LEN,SB_AWELEN,GUSLEN,GUSMAXLEN};

unsigned short Puertos[6]={0x210,0x220,0x230,0x240,0x250,0x260};
unsigned short IRQS[10]  ={2,3,5,7,10,11,12,13,14,15};
unsigned short DMAS[10]  ={0,1,2,3,4,5,6,7,8,9};
unsigned short DMAS2[10] ={0,1,2,3,4,5,6,7,8,9};

        remove("DIV32RUN.SWP");
        BSONIDO=0;
        File_Cfg=fopen("sound.cfg","rb");
        if(File_Cfg!=NULL) {
          fread(&Tarj,1,4,File_Cfg);
          fread(&Port,1,4,File_Cfg);
          fread(&Irq,1,4,File_Cfg);
          fread(&Dma,1,4,File_Cfg);
          fread(&Dma2,1,4,File_Cfg);
          fread(&master,1,4,File_Cfg);
          fread(&sound_fx,1,4,File_Cfg);
          fread(&cd_audio,1,4,File_Cfg);
          fclose(File_Cfg);
      	} else {
          DetectBlaster(&Tarj,&Port,&Irq,&Dma,&Dma2);
          DetectGUS(&Tarj,&Port,&Irq,&Dma,&Dma2);
          master=15; sound_fx=15; cd_audio=15;
        }

        if(Tarj==0) { InitHandler(0); return -1; }

        File_Cfg=fopen("DIV32RUN.SWP","wb");
        fwrite(Deviceptr[Tarj],1,DeviceLen[Tarj],File_Cfg);
        fclose(File_Cfg);

        if (master>15) master=15;
        if (sound_fx>15) sound_fx=15;
        if (cd_audio>15) cd_audio=15;
        InitMixer(Tarj,Puertos[Port],master,sound_fx,cd_audio);

        for(x=0;x<CHANNELS;x++)
                CLooping[x]=0;
        for(x=0;x<128;x++)
        {
                Sonido[x]=NULL;
                channel[x]=-1;
                SLooping[x]=0;
        }
        ssetup.MusicDevice[0]               =     0;
        ssetup.EffectDevice[0]              =     0;
        ssetup.MusicConfig.port1            = Puertos[Port];
        ssetup.MusicConfig.irq1             = IRQS[Irq];
        ssetup.MusicConfig.dma1             = DMAS[Dma];
        ssetup.MusicConfig.dma2             = DMAS2[Dma2];
        ssetup.MusicConfig.tickrate         = 256*70;
        ssetup.MusicConfig.rate             = 11270;
        ssetup.MusicConfig.maxchans         =     32-CHANNELS;
        ssetup.MusicConfig.stereo           = FALSE;
        ssetup.MusicConfig.bits16           = FALSE;
        ssetup.MusicConfig.panning          = FALSE;
        ssetup.MusicConfig.surround         = FALSE;
        ssetup.EffectConfig.maxchans        =    CHANNELS;
        ssetup.MusicParams.Period           = 65536;
        ssetup.EffectParams.Period          = 65536;
        ssetup.MusicParams.Volume           =     0;
        ssetup.EffectParams.Volume          =   512;
        ssetup.MusicParams.PanWidth         =    60;
        ssetup.EffectParams.PanWidth        =   256;
        ssetup.MusicParams.PanOffs          =     0;
        ssetup.EffectParams.PanOffs         =     0;
        strcpy(ssetup.MusicDevice,"DIV32RUN.SWP");
        gamev = GAME_Init(&ssetup);

        if (gamev == NULL)
        {
                InitHandler(0);
                return -1;
        }
        GAME_Poll(gamev, 20);
        GAME_SetMode(gamev, GAME_TIMER);
        GAME_Poll(gamev, 20);
        canalact=0;
        BSONIDO=1;
return(1);
}

#ifdef INTERPRETE
int InitSound2(int Tarj,int Port,int Irq,int Dma,int Dma2,int master,int sound_fx,int cd_audio)
{
int x;
FILE *File_Cfg;

char *Deviceptr[8]={sb_1_5,sb_1_5,sb_2_0,sb_pro,sb_16,sb_awe,gus,gusmax};
int DeviceLen[8]={0,SB_1_5LEN,SB_2_0LEN,SB_PROLEN,SB_16LEN,SB_AWELEN,GUSLEN,GUSMAXLEN};

unsigned short Puertos[6]={0x210,0x220,0x230,0x240,0x250,0x260};
unsigned short IRQS[10]  ={2,3,5,7,10,11,12,13,14,15};
unsigned short DMAS[10]  ={0,1,2,3,4,5,6,7,8,9};
unsigned short DMAS2[10] ={0,1,2,3,4,5,6,7,8,9};

        remove("DIV32RUN.SWP");
        BSONIDO=0;
        File_Cfg=fopen("DIV32RUN.SWP","wb");
        fwrite(Deviceptr[Tarj],1,DeviceLen[Tarj],File_Cfg);
        fclose(File_Cfg);
        if (master>15) master=15;
        if (sound_fx>15) sound_fx=15;
        if (cd_audio>15) cd_audio=15;
        InitMixer(Tarj,Puertos[Port],master,sound_fx,cd_audio);
        if(Tarj==0)
        {
                InitHandler(0);
                return -1;
        }
        for(x=0;x<CHANNELS;x++)
                CLooping[x]=0;
        for(x=0;x<128;x++)
        {
                Sonido[x]=NULL;
                channel[x]=-1;
                SLooping[x]=0;
        }
        ssetup.MusicDevice[0]               =     0;
        ssetup.EffectDevice[0]              =     0;
        ssetup.MusicConfig.port1            = Puertos[Port];
        ssetup.MusicConfig.irq1             = IRQS[Irq];
        ssetup.MusicConfig.dma1             = DMAS[Dma];
        ssetup.MusicConfig.dma2             = DMAS2[Dma2];
        ssetup.MusicConfig.tickrate         = 256*70;
        ssetup.MusicConfig.rate             = 11270;
        ssetup.MusicConfig.maxchans         =     32-CHANNELS;
        ssetup.MusicConfig.stereo           = FALSE;
        ssetup.MusicConfig.bits16           = FALSE;
        ssetup.MusicConfig.panning          = FALSE;
        ssetup.MusicConfig.surround         = FALSE;
        ssetup.EffectConfig.maxchans        =    CHANNELS;
        ssetup.MusicParams.Period           = 65536;
        ssetup.EffectParams.Period          = 65536;
        ssetup.MusicParams.Volume           =     0;
        ssetup.EffectParams.Volume          =   512;
        ssetup.MusicParams.PanWidth         =    60;
        ssetup.EffectParams.PanWidth        =   256;
        ssetup.MusicParams.PanOffs          =     0;
        ssetup.EffectParams.PanOffs         =     0;
        strcpy(ssetup.MusicDevice,"DIV32RUN.SWP");
        gamev = GAME_Init(&ssetup);

        if (gamev == NULL)
        {
                InitHandler(0);
                return -1;
        }
        GAME_Poll(gamev, 20);
        GAME_SetMode(gamev, GAME_TIMER);
        GAME_Poll(gamev, 20);
        canalact=0;
        BSONIDO=1;
return(1);
}
#endif

#ifdef INTERPRETE
extern void (*process_sound)(char * sound, int sound_lenght);
#endif

int LoadSound(char *SndName,int Loop)
{
FILE *SoundFile;
int x=0;
long Len;
        if(!BSONIDO) return -1;
        SoundFile=fopen(SndName,"rb");
        if(SoundFile==NULL)
        {
                return -1;
        }

        while(x<128 && Sonido[x]!=NULL) x++;

        fseek(SoundFile,0,SEEK_END);
        Len=ftell(SoundFile);
        fseek(SoundFile,0,SEEK_SET);
        Sonido[x]=(char *)malloc(Len);
        if(Sonido[x]==NULL)
        {
                return -1;
        }

        fread(Sonido[x],1,Len,SoundFile);
        fclose(SoundFile);

        #ifdef INTERPRETE
        if (process_sound!=NULL) process_sound(Sonido[x],Len);
        #endif

        lrec.Types              = SINS_TYP_DIGITAL;
        lrec.digiFile           = NULL;
        lrec.digiFileName       = NULL;
        lrec.digiLoopStart      = 0;
        lrec.digiAdjNum         = 0x2000;
        lrec.digiAdjDen         = 0x2000;
        lrec.digiSigned         = FALSE;
        lrec.digiBits16         = FALSE;
        lrec.digiLEndian        = FALSE;
        if(Loop)
                lrec.digiLoopLen        = Len;
        else
                lrec.digiLoopLen        = 0;
        lrec.digiPtr            = Sonido[x];
        lrec.digiSize           = Len;
        if(Loop)
                SLooping[x]=1;
        else
                SLooping[x]=0;
        eff[x] = GAME_EFF_Load(gamev, &lrec, 11000);
        if(eff[x]==NULL)
        {
                free(Sonido[x]);
                Sonido[x]=NULL;
                return -1;
        }

// ojo!!

        else {
                free(Sonido[x]);
        }

return x;
}

int UnloadSound(int NumSonido)
{
int x;
        if(!BSONIDO) return -1;

        if(Sonido[NumSonido]==NULL)
                return -1;

        for(x=0;x<CHANNELS;x++)
                if(channel[x]==NumSonido)
                {
                        GAME_EFF_Stop(gamev,x);
                        channel[x]=-1;
                }

        GAME_EFF_Unload(gamev, eff[NumSonido]);

        Sonido[NumSonido]=NULL;

return 1;
}

int PlaySound(int NumSonido,int Volumen,int Frec)
{
int cnt=0;
        if(!BSONIDO) {
           return -1;
        }

        if(Sonido[NumSonido]==NULL)
        {
                return -1;
        }

        canalact++;
        if(canalact>=CHANNELS)
                canalact=0;
        while(CLooping[canalact]==1)
        {
                if(++cnt==CHANNELS) break;

                canalact++;
                if(canalact>=CHANNELS)
                        canalact=0;
        }
        if(canalact>=CHANNELS)
                canalact=0;

        GAME_EFF_Start(gamev, canalact, Frec, Volumen, -32, eff[NumSonido]);
        channel[canalact]=NumSonido;
        if(SLooping[NumSonido]==1)
                CLooping[canalact]=1;
return(canalact);
}

int StopSound(int NumChannel)
{
int x;
        if(!BSONIDO) return -1;
        if(NumChannel==-1)
        {
                for(x=0;x<CHANNELS;x++)
                        StopSound(x);
                return(-1);
        }
        GAME_EFF_Stop(gamev,NumChannel);
        if(CLooping[NumChannel]==1)
                CLooping[NumChannel]=0;
        channel[NumChannel]=-1;
return(1);
}
int ChangeSound(int NumChannel,int Volumen,int Frec)
{
        GAME_EFF_SetChannelVolume(gamev,NumChannel,Volumen,20);
        GAME_EFF_SetChannelFreq(gamev,NumChannel,Frec,20);
return(1);
}

void LoopSound(void)
{
        if(!BSONIDO) return;
        GAME_Poll(gamev, 5);
}

void EndSound(void)
{
int x;
        if(!BSONIDO)
        {
                DoneHandler(0);
                return;
        }
        for(x=0;x<128;x++)
                UnloadSound(x);
        GAME_SetMode(gamev, GAME_POLL);
        SDEV_UnloadAll();
        VTAL_Done();
        remove("DIV32RUN.SWP");
}

#ifdef INTERPRETE
void EndSound2(void)
{
        if(!BSONIDO) { DoneHandler(0); return; }
        GAME_SetMode(gamev, GAME_POLL);
        SDEV_UnloadAll();
        VTAL_Done();
}
#endif

void WaitUntilFree(int NumChannel)
{
        if(!BSONIDO) return;
        do
        {
                GAME_Poll(gamev, 1);
        }while(!SDEV_ChannelFree(gamev->EffectDev, NumChannel));
}

int ChannelFree(int NumChannel)
{
        if(!BSONIDO) return -1;
return(SDEV_ChannelFree(gamev->EffectDev, NumChannel));
}

void SetTimer()
{
        GAME_SetMode(gamev, GAME_TIMER);
}

void SetPoll()
{
        GAME_SetMode(gamev, GAME_POLL);
}

//ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//  Funciones de detecciขn de la tarjeta de sonido
//ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

int ports[]={0x210, 0x220, 0x230, 0x240, 0x250, 0x260};
int tablairqs[]={2,3,5,7,10,11,12,12,14,15};

int DetectBlaster(int *tarj, int *port, int *irq, int *dma, int *hdma) {
    const char *p;
    char i,enc;

    *tarj=0; *port=0; *irq=0; *dma=0; *hdma=0;

    if ( (p = (const char *) getenv("BLASTER")) == NULL)
        return(0);

    while (*p != '\0') {
        if (isalpha(*p)) {
            switch(toupper(*p)) {
                case 'A': if (port != NULL) sscanf(p+1, "%hx", port); break;
                case 'I': if (irq != NULL)  sscanf(p+1, "%hd", irq); break;
                case 'D': if (dma != NULL)  sscanf(p+1, "%hd", dma); break;
                case 'H': if (hdma != NULL)  sscanf(p+1, "%hd", hdma); break;
                case 'T': if (*(p+1)<'3') *tarj=1;
                          else if (*(p+1)=='3') *tarj=2;
                          else if (*(p+1)=='4') *tarj=3;
                          else if (*(p+1)=='6') *tarj=4;
                          else *tarj=5;
                          break;
            }
            while (*p != '\0' && *p != ' ')
                p++;
        } else
            p++;
    }
    for (i=enc=0; !enc && i<sizeof(ports)/sizeof(ports[0]); i++)
        if (*port==ports[i])
        {
            *port=i;
            enc=1;
        }
    if (!enc)         // port desconocido
        *port=1;
    for (i=enc=0; !enc && i<sizeof(tablairqs)/sizeof(tablairqs[0]); i++)
        if (*irq==tablairqs[i])
        {
            *irq=i;
            enc=1;
        }
    if (!enc)         // irq desconocida
        *irq=3;
    return(1);
}


int DetectGUS(int *tarj, int *port, int *irq, int *dma, int *hdma) {
    const char *p;
    char i,enc;

    if ( (p = (const char *) getenv("ULTRASND")) == NULL)
        return(0);

    if (port != NULL)
        sscanf(p, "%hx", port);
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return(0);
    p++;    // Skip comma.
    if (dma!= NULL)
        sscanf(p, "%hd", dma);
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return(0);
    p++; // skip comma
    if (hdma!= NULL)
        sscanf(p, "%hd", hdma);
    while (*p != '\0' && *p != ',') p++;
    if (*p == '\0') return(0);
    p++;    // Skip comma.
    if (irq != NULL)
        sscanf(p, "%hd", irq);

    *tarj=6;
    if ( (p = (const char *) getenv("ULTRA16")) != NULL)
        *tarj=7;
    else
        *tarj=6;

    for (i=enc=0; !enc && i<sizeof(ports)/sizeof(ports[0]); i++)
        if (*port==ports[i])
        {
            *port=i;
            enc=1;
        }
    if (!enc)         // port desconocido
        *port=1;
    for (i=enc=0; !enc && i<sizeof(tablairqs)/sizeof(tablairqs[0]); i++)
        if (*irq==tablairqs[i])
        {
            *irq=i;
            enc=1;
        }
    if (!enc)         // irq desconocida
        *irq=3;
    return(1);
}

