#include "inter.h"
#include "divmixer.hpp"

UWORD SoundCard;
UWORD BaseAddress;

void SetMasterVolume (UWORD volumen)
{
#ifdef DOS
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
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
#ifdef DOS
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
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
#ifdef DOS
    if (SoundCard==DEV_NOSOUND) return;

    volumen &= 15;
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
    if (card > DEV_NOSOUND && card <= DEV_GUS)
    {
        SoundCard=card;
        BaseAddress=address;
        setup->cd_audio=cd & 15;
        setup->master=master & 15;
        setup->sound_fx=voc & 15;
        SetMasterVolume(setup->master);
        SetVocVolume(setup->sound_fx);
        SetCDVolume(setup->cd_audio);
    }
}

void MIX_Reset(void)
{
#ifdef DOS
  outp(judascfg_port+MIX_ADR_OFF,   0);
  outp(judascfg_port+MIX_ADR_OFF+1, 0);
#endif
}

void MIX_SetInput(byte opt)
{
#ifdef DOS
  outp(judascfg_port+MIX_ADR_OFF,   MIX_INPUT);
  outp(judascfg_port+MIX_ADR_OFF+1, opt);
#endif
}

void MIX_GetVolume(byte reg, byte *left, byte *right)
{
#ifdef DOS
  byte al;

  outp(judascfg_port+MIX_ADR_OFF, reg);
  al = inp(judascfg_port+MIX_ADR_OFF+1);
  *right = al  & 0x0f;
  *left  = al >> 4;

#endif
}

void MIX_SetVolume(byte reg, byte left, byte right)
{
#ifdef DOS
  byte al;

  outp(judascfg_port+MIX_ADR_OFF,   reg);
  al  = left << 4;
  al |= right;
  outp(judascfg_port+MIX_ADR_OFF+1, al);
#endif
}

void set_mixer(void) {
#ifdef DOS
  if(judascfg_device==DEV_NOSOUND) return;

  InitMixer(judascfg_device, judascfg_port, setup->master, setup->sound_fx, setup->cd_audio);
#endif
}

void set_init_mixer(void) {
#ifdef DOS
  byte fx_l, fx_r;
  byte cd_l, cd_r;
  byte ma_l, ma_r;

  if(judascfg_device==DEV_NOSOUND) return;

  setup->sound_fx = 8;
  setup->cd_audio = 8;
  setup->master   = 8;

  if( judascfg_device == DEV_SB    ||
      judascfg_device == DEV_SBPRO ||
      judascfg_device == DEV_SB16  )
  {
    MIX_GetVolume(MIX_VOICE_VOL,  &fx_l, &fx_r);
    MIX_GetVolume(MIX_CD_VOL,     &cd_l, &cd_r);
    MIX_GetVolume(MIX_MASTER_VOL, &ma_l, &ma_r);
    setup->sound_fx = ((fx_l>fx_r) ? fx_l : fx_r);
    setup->cd_audio = ((cd_l>cd_r) ? cd_l : cd_r);
    setup->master   = ((ma_l>ma_r) ? ma_l : ma_r);
  }
  InitMixer(judascfg_device, judascfg_port, setup->master, setup->sound_fx, setup->cd_audio);

#endif
}

