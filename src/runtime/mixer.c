#include "inter.h"
#include "mixer.h"

void set_master_volume(UWORD volumen) {}

void set_voc_volume(UWORD volumen) {}

void set_cd_volume(UWORD volumen) {}

void init_mixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd) {}

void mix_reset(void) {}

void mix_set_input(byte opt) {}

void mix_get_volume(byte reg, byte *left, byte *right) {}

void mix_set_volume(byte reg, byte left, byte right) {}

void set_mixer(void) {}

void set_init_mixer(void) {}
