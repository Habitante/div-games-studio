#include "global.h"
#include "divmixer.hpp"

UWORD SoundCard;
UWORD BaseAddress;
UWORD CD;
UWORD MASTER;
UWORD VOC;

void SetMasterVolume (UWORD volumen)
{
#ifdef NOTYET
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
    MASTER=volumen;
    switch (SoundCard)
    {
        case DEV_SB:
                    volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBMASTER);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_SBPRO:
        case DEV_SB16 :
                    volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPMASTER);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_GUS:
                    volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSMASTERL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSMASTERR);
                    outp(BaseAddress+GUSMIXDATA,volumen);
    }
#endif
}

void SetVocVolume (UWORD volumen)
{
#ifdef NOTYET
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
    VOC=volumen;
    switch (SoundCard)
    {
        case DEV_SB:
                    volumen >>= 1;
                    volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBVOC);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_SBPRO:
        case DEV_SB16:
                    volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPVOC);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_GUS:
                    volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSVOCL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSVOCR);
                    outp(BaseAddress+GUSMIXDATA,volumen);
    }
    
#endif
}

void SetCDVolume (UWORD volumen)
{
#ifdef NOTYET
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
    CD=volumen;
    switch (SoundCard)
    {
        case DEV_SB:
                    volumen &= ~1;
                    outp (BaseAddress+SBMIXADDRESS, SBCD);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_SBPRO:
        case DEV_SB16:
                    volumen += volumen << 4;
                    outp (BaseAddress+SBMIXADDRESS, SBPCD);
                    outp (BaseAddress+SBMIXDATA, volumen);
                    break;
        case DEV_GUS:
                    volumen <<=3;
                    outp(BaseAddress+GUSMIXADDRESS,GUSCDL);
                    outp(BaseAddress+GUSMIXDATA,volumen);
                    outp(BaseAddress+GUSMIXADDRESS,GUSCDR);
                    outp(BaseAddress+GUSMIXDATA,volumen);
    }
#endif
}

void InitMixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd)
{
  voc=(master<voc)?master:voc;
  cd=(master<cd)?master:cd;
  cd=Mix_VolumeMusic(cd*8);
  voc=Mix_Volume(-1, voc*8);

#ifdef NOTYET
    if (card > DEV_NOSOUND && card <= DEV_GUS)
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
#endif
}

void MIX_Reset(void)
{
#ifdef NOTYET
  outp(judascfg_port+MIX_ADR_OFF,   0);
  outp(judascfg_port+MIX_ADR_OFF+1, 0);
#endif
}

void MIX_SetInput(byte opt)
{
#ifdef NOTYET
  outp(judascfg_port+MIX_ADR_OFF,   MIX_INPUT);
  outp(judascfg_port+MIX_ADR_OFF+1, opt);
#endif
}

void MIX_GetVolume(byte reg, byte *left, byte *right)
{
#ifdef NOTYET
  byte al;

  outp(judascfg_port+MIX_ADR_OFF, reg);
  al = inp(judascfg_port+MIX_ADR_OFF+1);
  *right = al  & 0x0f;
  *left  = al >> 4;
#endif
}

void MIX_SetVolume(byte reg, byte left, byte right)
{
#ifdef NOTYET
  byte al;

  outp(judascfg_port+MIX_ADR_OFF,   reg);
  al  = left << 4;
  al |= right;
  outp(judascfg_port+MIX_ADR_OFF+1, al);
#endif
}

void set_mixer(void) {
  int  fx, cd, ma;

//  if(judascfg_device==DEV_NOSOUND) return;

  fx=Setupfile.vol_fx; if(Setupfile.mut_fx) fx=0;
  cd=Setupfile.vol_cd; if(Setupfile.mut_cd) cd=0;
  ma=Setupfile.vol_ma; if(Setupfile.mut_ma) ma=0;
  InitMixer(0, 0, ma, fx, cd);
}

void set_init_mixer(void) {
  int  fx, cd, ma;
  byte fx_l, fx_r;
  byte cd_l, cd_r;
  byte ma_l, ma_r;

//  if(judascfg_device==DEV_NOSOUND) return;

/*  if( judascfg_device == DEV_SBPRO ||
      judascfg_device == DEV_SB16  )
  {
    MIX_GetVolume(MIX_VOICE_VOL,  &fx_l, &fx_r);
    MIX_GetVolume(MIX_CD_VOL,     &cd_l, &cd_r);
    MIX_GetVolume(MIX_MASTER_VOL, &ma_l, &ma_r);
    fx=Setupfile.vol_fx=((fx_l>fx_r) ? fx_l : fx_r); if (Setupfile.mut_fx) fx=0;
    cd=Setupfile.vol_cd=((cd_l>cd_r) ? cd_l : cd_r); if (Setupfile.mut_cd) cd=0;
    ma=Setupfile.vol_ma=((ma_l>ma_r) ? ma_l : ma_r); if (Setupfile.mut_ma) ma=0;
    InitMixer(judascfg_device, judascfg_port, ma, fx, cd);
  }
  else
  */
  {
    fx=Setupfile.vol_fx; if(Setupfile.mut_fx) fx=0;
    cd=Setupfile.vol_cd; if(Setupfile.mut_cd) cd=0;
    ma=Setupfile.vol_ma; if(Setupfile.mut_ma) ma=0;
    InitMixer(0, 0, ma, fx, cd);
  }
}

