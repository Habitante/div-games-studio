#include "global.h"
#include "divmixer.hpp"

void SetMasterVolume(UWORD volumen) {}

void SetVocVolume(UWORD volumen) {}

void SetCDVolume(UWORD volumen) {}

void InitMixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd) {
  voc = (master < voc) ? master : voc;
  cd = (master < cd) ? master : cd;
  cd = Mix_VolumeMusic(cd * 8);
  voc = Mix_Volume(-1, voc * 8);
}

void MIX_Reset(void) {}

void MIX_SetInput(byte opt) {}

void MIX_GetVolume(byte reg, byte *left, byte *right) {}

void MIX_SetVolume(byte reg, byte left, byte right) {}

void set_mixer(void) {
  int fx, cd, ma;

  fx = Setupfile.vol_fx;
  if (Setupfile.mut_fx)
    fx = 0;
  cd = Setupfile.vol_cd;
  if (Setupfile.mut_cd)
    cd = 0;
  ma = Setupfile.vol_ma;
  if (Setupfile.mut_ma)
    ma = 0;
  InitMixer(0, 0, ma, fx, cd);
}

void set_init_mixer(void) {
  int fx, cd, ma;

  fx = Setupfile.vol_fx;
  if (Setupfile.mut_fx)
    fx = 0;
  cd = Setupfile.vol_cd;
  if (Setupfile.mut_cd)
    cd = 0;
  ma = Setupfile.vol_ma;
  if (Setupfile.mut_ma)
    ma = 0;
  InitMixer(0, 0, ma, fx, cd);
}
