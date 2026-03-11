
#include "global.h"
#include "divsound.h"
#include "divsb.h"
#include "divmixer.hpp"

int reloj = 0;
extern int safe;

int SoundActive = 0;

void InitSound(void) {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
#ifdef MIXER

  int audio_rate = 44100;
  Uint16 audio_format = AUDIO_S16SYS;
  int audio_channels = 2;
  int audio_buffers = 4096;


  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
    fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
  } else {
    SoundActive = 1;
  }

#endif
}

void EndSound(void) {
#ifdef MIXER
  Mix_CloseAudio();
#endif

  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
