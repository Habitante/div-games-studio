#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <i86.h>
#include "MIXER.HPP"

UWORD SoundCard=NOSOUND;
UWORD BaseAddress=0x220;
UWORD CD=10;
UWORD MASTER=10;
UWORD VOC=10;

void SetMasterVolume (UWORD volumen)
{
    if (SoundCard==NOSOUND) return;

    volumen &= 15;
    MASTER=volumen;
    switch (SoundCard)
    {
        case SB15:
        case SB20:  volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBMASTER);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPMASTER);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case GUS:   volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSMASTERL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSMASTERR);
                    outp(BaseAddress+GUSMIXDATA,volumen);

        case GUSMAX:
                    volumen = 15-volumen;
                    volumen <<= 2;
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXMASTERL);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXMASTERR);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    break;

    }
}

void SetVocVolume (UWORD volumen)
{
    if (SoundCard==NOSOUND) return;

    volumen &= 15;
    VOC=volumen;
    switch (SoundCard)
    {
        case SB15:
        case SB20:  volumen >>= 1;
                    volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBVOC);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPVOC);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case GUS:   volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSVOCL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSVOCR);
                    outp(BaseAddress+GUSMIXDATA,volumen);
        case GUSMAX:
                    volumen=15-volumen;
                    volumen <<= 1;
                    volumen++;
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXVOCL);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXVOCR);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    break;
    }
}

void SetCDVolume (UWORD volumen)
{
    if (SoundCard==NOSOUND) return;

    volumen &= 15;
    CD=volumen;
    switch (SoundCard)
    {
        case SB15:
        case SB20:  volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBCD);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPCD);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case GUS:   volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSCDL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSCDR);
                    outp(BaseAddress+GUSMIXDATA,volumen);
        case GUSMAX:
                    volumen=15-volumen;
                    volumen <<= 1;
                    volumen++;
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXCDL);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    outp(BaseAddress+MAXMIXADDRESS,GUSMAXCDR);
                    outp(BaseAddress+MAXMIXDATA,volumen);
                    break;
    }
}

UWORD GetMasterVolume()
{
    return MASTER;
/*
    UWORD vol=0;

    switch (SoundCard)
    {
        case SB15:
        case SB20:  outp (BaseAddress+SBMIXADDRESS, SBMASTER);
                    vol=inp (BaseAddress+SBMIXDATA);
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: outp (BaseAddress+SBMIXADDRESS, SBPMASTER);
                    vol=inp(BaseAddress+SBMIXDATA) >> 4;
                    break;
    }
    return (vol > 15 ? 0 : vol);
*/
}

UWORD GetVocVolume()
{
    return VOC;
/*
    UWORD vol=0;

    switch (SoundCard)
    {
        case SB15:
        case SB20:  outp (BaseAddress+SBMIXADDRESS, SBVOC);
                    vol=inp (BaseAddress+SBMIXDATA);
                    vol <<= 1;
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: outp (BaseAddress+SBMIXADDRESS, SBPVOC);
                    vol=inp(BaseAddress+SBMIXDATA) >> 4;
                    break;
    }
    return (vol > 15 ? 0 : vol);
*/
}

UWORD GetCDVolume()
{
    return CD;
/*
    UWORD vol=0;

    switch (SoundCard)
    {
        case SB15:
        case SB20:  outp (BaseAddress+SBMIXADDRESS, SBCD);
                    vol=inp (BaseAddress+SBMIXDATA);
                    break;
        case SBPRO:
        case SB16:
        case SBAWE: outp (BaseAddress+SBMIXADDRESS, SBPCD);
                    vol=inp(BaseAddress+SBMIXDATA) >> 4;
                    break;
    }
    return (vol > 15 ? 0 : vol);
*/
}

UWORD MusicFadeout()
{
    UWORD i,v;

    if (SoundCard==NOSOUND) return 0;
    v=GetCDVolume();
    i=v;
    while(i) {
        SetCDVolume(--i);
        delay(200);
    }
    return(v);
}

void InitMixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd)
{
    if (card > NOSOUND && card <= GUSMAX)
    {
        SoundCard=card;
        BaseAddress=address;
        CD=cd & 15;
        MASTER=master & 15;
        VOC=voc & 15;
        SetMasterVolume(MASTER);
        SetVocVolume(VOC);
        SetCDVolume(CD);
    }
}


