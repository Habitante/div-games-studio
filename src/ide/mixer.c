#include "global.h"
#include "mixer.h"

void set_master_volume(UWORD volumen) {}

void set_voc_volume(UWORD volumen) {}

void set_cd_volume(UWORD volumen) {}

void init_mixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd) {
  voc = (master < voc) ? master : voc;
  cd = (master < cd) ? master : cd;
  cd = Mix_VolumeMusic(cd * 8);
  voc = Mix_Volume(-1, voc * 8);
}

void mix_reset(void) {}

void mix_set_input(byte opt) {}

void mix_get_volume(byte reg, byte *left, byte *right) {}

void mix_set_volume(byte reg, byte left, byte right) {}

void set_mixer(void) {
  int fx, cd, ma;

  fx = setup_file.vol_fx;
  if (setup_file.mut_fx)
    fx = 0;
  cd = setup_file.vol_cd;
  if (setup_file.mut_cd)
    cd = 0;
  ma = setup_file.vol_ma;
  if (setup_file.mut_ma)
    ma = 0;
  init_mixer(0, 0, ma, fx, cd);
}

void set_init_mixer(void) {
  int fx, cd, ma;

  fx = setup_file.vol_fx;
  if (setup_file.mut_fx)
    fx = 0;
  cd = setup_file.vol_cd;
  if (setup_file.mut_cd)
    cd = 0;
  ma = setup_file.vol_ma;
  if (setup_file.mut_ma)
    ma = 0;
  init_mixer(0, 0, ma, fx, cd);
}
