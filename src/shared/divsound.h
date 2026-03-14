#ifndef __DIV_SOUND_
#define __DIV_SOUND_

#define CHANNELS    128
#define DEV_NOSOUND 0
#define DEV_SB      1
#define DEV_SBPRO   2
#define DEV_SB16    3
#define DEV_GUS     4
#define FASTMIXER   0
#define SIXTEENBIT  0
#define STEREO      0
#define EIGHTBIT    0


#define MOD 1
#define S3M 2
#define XM  3

#define SAMPLE char

typedef struct _channels {
  int freq;
  int vol;
  int con;
  int num;
  int pos;
} tChannels;

typedef struct _sonido {
  SAMPLE smp;
#ifdef MIXER
  Mix_Chunk *sound;
  char loop;
#endif
} sound_t;

typedef struct _cancion {
  char *ptr;
  int loop;
  int song_type;
#ifdef MIXER
  Mix_Music *music;
  SDL_RWops *rw;
#endif
} song_t;

extern sound_t sounds[128];
extern song_t songs[128];
extern int *new_sound;
extern int channel_con;

void sound_init(void);
void sound_reset(void);
int sound_load(char *ptr, long len, int loop_flag);
int sound_unload(int sound_num);
int sound_play(int sound_num, int volume, int frequency);
int sound_stop(int num_channel);
int sound_change(int num_channel, int volume, int frequency);
int sound_change_channel(int num_channel, int volume, int panning);
int sound_is_playing(int num_channel);
int sound_load_song(char *ptr, int len, int loop_flag);
int sound_play_song(int num_song);
void sound_stop_song(void);
void sound_unload_song(int num_song);
void sound_set_song_pos(int song_pos);
int sound_get_song_pos(void);
int sound_get_song_line(void);
int sound_is_playing_song(void);
void sound_end(void);

// For in-memory pcm to wav conversion
typedef struct _HeadDC {
  uint32_t dwUnknow;
  uint16_t wFormatTag;
  uint16_t wChannels;
  uint32_t dwSamplePerSec;
  uint32_t dwAvgBytesPerSec;
  uint16_t wBlockAlign;
  uint16_t wBits;
} HeadDC;

#endif
