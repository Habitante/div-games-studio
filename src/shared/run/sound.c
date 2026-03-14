#include "inter.h"
#include "divsound.h"

sound_t sounds[128];
song_t songs[128];
tChannels channels[64];

int song_type = 0;
int music_channels = 0;
int next_channel = 0;
int *new_sound;
int channel_con = 0;

int song_inst[128];

int freq_original[CHANNELS];

int sound_active = 1;
void print_init_flags(int flags) {
#ifdef MIXER

#define PFLAG(a)              \
  if (flags & MIX_INIT_##a) { \
    printf(#a " ");           \
  }
  PFLAG(FLAC);
  PFLAG(MOD);
  PFLAG(MP3);
  PFLAG(OGG);

  if (!flags) {
    printf("None");
  }

  printf("\n");
#endif
}

static int initted = 0;
void sound_init(void) {
#ifdef MIXER
  if (initted)
    return;

  int audio_rate = 44800; //44100;
  Uint16 audio_format = AUDIO_S16SYS;
  int audio_channels = 2;
  int audio_buffers = 512;
  int flags = MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG;

  initted = Mix_Init(flags);

  if ((initted & flags) != flags) {
    printf("Mix_Init error: %s\n", Mix_GetError());
  }

  SDL_Init(SDL_INIT_AUDIO);

  fprintf(stdout, "Opening Audio Device \n");
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
    fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
  }


  print_init_flags(initted);
  SDL_version compile_version;
  const SDL_version *link_version = Mix_Linked_Version();
  SDL_MIXER_VERSION(&compile_version);
  fprintf(stdout, "compiled with SDL_mixer version: %d.%d.%d\n", compile_version.major,
          compile_version.minor, compile_version.patch);
  fprintf(stdout, "running with SDL_mixer version: %d.%d.%d\n", link_version->major,
          link_version->minor, link_version->patch);

  initted = 1;
#endif

  music_channels = 0;
}

void sound_reset(void) {}

int sound_load(char *ptr, long len, int loop_flag) {
  HeadDC wav_header;

#ifdef MIXER
  int channel;
  Mix_Chunk *sound = NULL;
  SDL_RWops *rw;
  byte *dst;
  byte *dst2;
  FILE *mem, *fdst;
  char *riff = "RIFF";
  char *wavefmt = "WAVEfmt ";
  char *data = "data";
  int con = 0;
  byte res = 0;
  int32_t ilen = (int32_t)len + 36;

  while (con < 128 && sounds[con].smp != 0)
    con++;

  if (con == 128)
    return (-1);

  // check if wav or OGG
  if ((ptr[0] == 'R' && ptr[1] == 'I') || (ptr[0] == 'O' && ptr[1] == 'g')) {
    rw = SDL_RWFromMem(ptr, len);
    sound = Mix_LoadWAV_RW(rw, 1);
  }

  // If sound cant be loaded, must be a pcm
  if (!sound) {
    dst = (byte *)malloc((int)ilen + 50);

    if (dst == NULL)
      return (-1);

    memset(dst, 0, (int)len + 40);

    wav_header.dwUnknow = 16;
    wav_header.wFormatTag = 1;
    wav_header.wChannels = 1;
    wav_header.dwSamplePerSec = 11025;
    wav_header.dwAvgBytesPerSec = 11025;
    wav_header.wBlockAlign = 1;
    wav_header.wBits = 8;

    memcpy(dst, riff, 4);
    memcpy(dst + 4, &ilen, 4);
    memcpy(dst + 8, wavefmt, 8);
    memcpy(dst + 16, &wav_header, sizeof(HeadDC));
    memcpy(dst + 16 + sizeof(HeadDC), data, 4);
    memcpy(dst + 20 + sizeof(HeadDC), &len, 4);
    memcpy(dst + 24 + sizeof(HeadDC), ptr, len);

    rw = SDL_RWFromMem((void *)dst, (int)(len + 24 + sizeof(HeadDC)));
    sound = Mix_LoadWAV_RW(rw, 1);

    if (!sound) {
      printf("Mix_LoadWAV: %s\n", Mix_GetError());
      return (-1);
    }
  }
  // all ok. save our data to free() later
  sounds[con].smp = 1;
  sounds[con].sound = sound;
  sounds[con].loop = loop_flag;

  return con;
#endif

  return -1;
}

int sound_unload(int sound_num) {
#ifdef MIXER
  if (sounds[sound_num].sound) {
    Mix_FreeChunk(sounds[sound_num].sound);
    sounds[sound_num].smp = 0;
    sounds[sound_num].sound = NULL;
  }
#endif
  return (1);
}
#ifdef MIXER

void done_effect(int chan, void *data) {
  return;
}

// make a passthru processor function that alters the stream size
void freq_effect(int chan, void *stream, int len, void *udata) {
  float x = 0;
  sound_t *s = &sounds[channels[chan].num];
  int pos = channels[chan].pos;

  if (!Mix_Playing(chan))
    return;

  if (channels[chan].freq > 1024)
    channels[chan].freq = 1024;
  if (channels[chan].freq < 0)
    channels[chan].freq = 0;

  float ratio = channels[chan].freq / 256.0f; //(22050 +10000) / 22050.0f;

  short *samples = (short *)stream;
  memset(stream, 0, len);
  uint16_t *input = (uint16_t *)(s->sound->abuf) + pos;
  int i = 0;
  float j = 0;
  for (x = 0; i < len / 2 && pos + x < s->sound->alen / 2; x += ratio) {
    if (pos + (int)x >= s->sound->alen / 2) {
      if (s->loop == 1) {
        x = 0;
        j = 0;
        pos = 0;
        input = (uint16_t *)(s->sound->abuf);
      } else {
        fprintf(stdout, "Sound ended\n");
        while (i < len / 2 - 1) {
          samples[i++] = 0;
          x += ratio;
          j += ratio;
        }
        i = len / 2;
        pos = len / 2 + s->sound->alen / 2;
        break;
        //        sound_stop(chan);
      }
    }
    samples[i++] = input[(int)x];
    j += ratio;
  }
  pos += (int)j;

  if (pos >= s->sound->alen / 2) {
    if (s->loop == 1)
      pos = 0;
    else {
      for (; i < len / 2; i++) {
        samples[i] = 0;
      }
      sound_stop(chan);
      return;
    }
  }
  channels[chan].pos = pos;

  // fill rest with empty :(
  // this needs to be moved into the loop ideally
  for (; i < len / 2; i++) {
    samples[i] = 0;
  }
}
#endif
void channel_done(int channel) {
  // Done!
}

int sound_play(int sound_num, int volume, int frequency) // Vol y frequency (0..256)
{
  int con = 0;
  int loop = -1;
#ifdef MIXER

  // always play as loop, let the freq_effect manage stop_sound when loop is zero
  // this permits slow playing sound to run for the correct length.
  con = Mix_PlayChannel(-1, sounds[sound_num].sound, loop);


  // if unable to play, return
  if (con == -1)
    return (0);

  // Make sure all old callbacks are cleared
  Mix_UnregisterAllEffects(con);

  channels[con].freq = frequency;
  channels[con].vol = volume;
  channels[con].pos = 0;

  // Setup our callback to change frequency
  Mix_RegisterEffect(con, freq_effect, done_effect, NULL);
  Mix_ChannelFinished(channel_done);

  Mix_Volume(con, volume / 2);
  channels[con].num = sound_num;
  channels[con].con = con;

  freq_original[con] = channels[con].freq;

#endif
  return (con);
}

int sound_stop(int num_channel) {
  if (num_channel == -2)
    num_channel = -1;

#ifdef MIXER
  int x = 99;
  if (Mix_Playing(num_channel)) {
    Mix_HaltChannel(num_channel);
  }
  while (x-- > 0 && Mix_Playing(num_channel))
    Mix_HaltChannel(num_channel);

#endif // MIXER
  return (1);
}

int sound_change(int num_channel, int volume, int frequency) {
#ifdef MIXER
  channels[num_channel].freq = frequency;
  Mix_Volume(num_channel, volume / 2);
#endif

  return (1);
}

int sound_change_channel(int num_channel, int volume, int panning) {
  // Set the volume
#ifdef MIXER
  Mix_Volume(num_channel, volume / 2);

  // set the balance
  Mix_SetPanning(num_channel, 255 - panning, panning);
#endif
  return (1);
}

int sound_is_playing(int num_channel) {
#ifdef MIXER
  return Mix_Playing(num_channel);
#endif

  return (1);
}

#ifdef DIV2
int sound_load_song(char *ptr, int len, int loop_flag) {
#ifdef MIXER
  int con = 0;

  // Check we can load the file
  SDL_RWops *rw = SDL_RWFromMem(ptr, len);

  Mix_Music *music = Mix_LoadMUS_RW(rw, 0);
  if (!music)
    return (-1);

  Mix_FreeMusic(music);
  SDL_FreeRW(rw);

  while (con < 128 && songs[con].ptr != NULL)
    con++;
  if (con == 128)
    return (-1);

  if ((songs[con].ptr = (char *)malloc(len)) == NULL)
    return (-1);

  memcpy(songs[con].ptr, ptr, len);
  songs[con].loop = loop_flag;

  rw = SDL_RWFromMem(songs[con].ptr, len);
  music = Mix_LoadMUS_RW(rw, 0);

  songs[con].music = music;
  songs[con].rw = rw;

  return (con);
#endif
  return -1;
}

int sound_play_song(int num_song) {
  if (num_song > 127 || !songs[num_song].ptr)
    return (-1);

  sound_stop_song();

#ifdef MIXER
  if (songs[num_song].music) {
    Mix_PlayMusic(songs[num_song].music, songs[num_song].loop ? -1 : 0);
  }
#endif

  return (1);
}

void sound_stop_song(void) {
#ifdef MIXER
  Mix_HaltMusic();
#endif

  music_channels = 0;
}

void sound_unload_song(int num_song) {
  if (num_song > 127 || !songs[num_song].ptr)
    return;
#ifdef MIXER
  Mix_FreeMusic(songs[num_song].music);
  free(songs[num_song].ptr);
  SDL_FreeRW(songs[num_song].rw);

  songs[num_song].music = NULL;
  songs[num_song].rw = NULL;
#endif
  songs[num_song].ptr = NULL;
  songs[num_song].loop = 0;
}

void sound_set_song_pos(int song_pos) {
#ifdef MIXER
  Mix_SetMusicPosition((double)song_pos);
#endif
}

int sound_get_song_pos(void) {
  return -1;
}

int sound_get_song_line(void) {
  return -1;
}

int sound_is_playing_song(void) {
#ifdef MIXER
  return (Mix_PlayingMusic());
#endif
}

#endif

void sound_end(void) {}
