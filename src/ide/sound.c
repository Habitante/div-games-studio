
#include "global.h"
#include "sound.h"
#include "mixer.h"

int frame_clock = 0;
extern int safe;

int sound_active = 0;

void sound_init(void) {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
#ifdef MIXER

  int audio_rate = 44100;
  Uint16 audio_format = AUDIO_S16SYS;
  int audio_channels = 2;
  int audio_buffers = 4096;


  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
    fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
  } else {
    sound_active = 1;
  }

#endif
}

void sound_end(void) {
#ifdef MIXER
  Mix_CloseAudio();
#endif

  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
