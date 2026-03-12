
#include "global.h"
#include "div_string.h"
#include "sound.h"
#include "mixer.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


int file_pos = 0;

void free_mod(void);
int sound_get_song_pos(void);
int sound_get_song_line(void);
void mostrar_mod_meters(void);
void open_sound(void);
void open_sound_file(void);
void open_desktop_sound(FILE *f);
void save_sound(pcminfo *mypcminfo, char *dst);
void save_desktop_sound(pcminfo *mypcminfo, FILE *f);
void close_sound(char *snd_path);
void open_song(void);
void open_desktop_song(void);
void sound_play_song(char *pathname);
void edit_sound0(void);
void edit_sound1(void);
void edit_sound2(void);
void EditSound3(void);
void rec_sound0(void);
void RecSound1(void);
void RecSound2(void);
void RecSound3(void);
void modify_sound(int option);
void change_sound_freq(int freq);
void record_sound(void);
void poll_record(void);
int judas_progress_read(int handle, void *buffer, int size);
void copy_new_sound(pcminfo *mypcminfo, int ini, int fin);
void paste_new_sounds(void);
byte *save_sound_mem(pcminfo *mypcminfo);
int is_wav(char *filename);
int new_sample(pcminfo *mypcminfo); // 1-OK, 0-ERROR
// Allocate (malloc+lock) in mypcminfo->sound_data for mypcminfo->sound_size shorts
void wline(char *ptr, int realan, int w, int h, int x0, int y0, int x1, int y1, char color);
void linea_pixel(char *ptr, int w, int realan, int h, int x, int y, char color);

void set_mixer(void);
int find_pcm_window(void);

char sound_name[255];
char sound_path_name[256];
char song_name[255];
char song_path_name[256];
byte *pcminfo_aux;
byte *modinfo_aux;
int SongType = 0;
int song_code = 10;
int SongChannels = 0;
int last_mod_clean = 1;
int wav_handle;
char sound_file[256];
int rec_device[2] = {1, 0};
int ModButton = 0;
int ModWindow = -1;
extern int frame_clock;

void errhlp0(void);

//-----------------------------------------------------------------------------
//  Sounds Window
//-----------------------------------------------------------------------------

void pcm1(void) {
  int length;
  float step, position = 0;
  int x, y, y0, y1, p0, p1;
  byte *ptr = v.ptr;
  int w = v.w, h = v.h;
  int width, height, first = 1, lx, ly;
  pcminfo *mypcminfo;
  short *buffer;
  short sample;

  if (big) {
    w /= 2;
    h /= 2;
  }
  mypcminfo = (pcminfo *)v.aux;

  wbox(ptr, w, h, c1, 2, 10, w - 4, h - 12);
  width = v.w - 4 * big2;
  height = v.h - 12 * big2;
  ptr += (2 + (10 * v.w)) * big2 + (height / 2) * v.w;
  buffer = mypcminfo->sound_data;
  length = mypcminfo->sound_size;

  if (length > 1) {
    if (length < 3 * width) {
      step = (float)width / (float)(length - 1);
      for (x = 0; x < length; x++) {
        sample = buffer[x], y = (sample * height / 65536);

        if (first) {
          first = 0;
          lx = (int)position;
          ly = y;
        } else {
          wline((char *)ptr, v.w, width, height, lx, ly, (int)position, y, c_g_low);
          lx = (int)position;
          ly = y;
        }
        position += step;
      }
    } else {
      step = (float)length / (float)width;
      for (x = 0; x < width; x++) {
        p0 = (memptrsize)position;
        position += step;
        p1 = (memptrsize)position;

        sample = buffer[p0], y0 = y1 = (sample * height / 65536);

        do {
          sample = buffer[p0], y = (sample * height / 65536);
          if (y < y0)
            y0 = y;
          else if (y > y1)
            y1 = y;
          p0 += 2;
        } while (p0 < p1);

        y = y0;
        do {
          ptr[x + y * v.w] = c_g_low;
        } while (y++ < y1);
      }
    }
  } // length>1
}

extern int ns, chn;
extern int bload;
int sound_window;

void pcm2(void) {
  pcminfo *mypcminfo = (pcminfo *)v.aux;

  if (mouse_b & 1) {
    if (!sound_active) {
      if (sound_error) {
        v_text = (char *)texts[549];
        show_dialog(errhlp0);
        if (v_accept)
          help(2008);
      } else {
        v_text = (char *)texts[548];
        show_dialog(errhlp0);
        if (v_accept)
          help(2009);
      }
      return;
    } else {
      if (mypcminfo->sound_data) {
#ifdef MIXER
        Mix_PlayChannel(0, mypcminfo->si, 0);
#endif
      }
      while (mouse_b & 1)
        read_mouse();
    }
  }
}

void PCM3(void) {
  pcminfo *mypcminfo = (pcminfo *)v.aux;

  if (mypcminfo->sound_data) {
#ifdef MIXER
    Mix_FreeChunk(mypcminfo->si);
    mypcminfo->si = NULL;
#endif
    mypcminfo->sound_data = NULL;
    mypcminfo->sound_size = 0;
  }
  free(v.aux);
}

void pcm0(void) {
  pcminfo *mypcminfo;

  v.type = 105;
  v.w = 80;
  v.h = 50;

  v.paint_handler = pcm1;
  v.click_handler = pcm2;
  v.close_handler = PCM3;

  v.aux = pcminfo_aux;
  pcminfo_aux = NULL;
  mypcminfo = (pcminfo *)v.aux;

  v.title = (byte *)mypcminfo->name;
  v.name = (byte *)mypcminfo->name;
}

//-----------------------------------------------------------------------------
//  Song (mod/s3m/xm) window
//-----------------------------------------------------------------------------

void MOD1(void) {
  modinfo *mymodinfo = (modinfo *)v.aux;
  int w = v.w / big2, h = v.h / big2;

  _show_items();

  wbox(v.ptr, w, h, c1, 2, 10, w - 4, h - 12 - 10);
  wbox(v.ptr, w, h, c2, 2, 39, w - 4, 9);
  wrectangle(v.ptr, w, h, c0, 1, 38, w - 2, 11);
  wrectangle(v.ptr, w, h, c0, 1, 38, 21, 11);

  if (Mix_PlayingMusic() && mymodinfo->song_code == song_code) {
    if (ModButton && ModWindow == v.order)
      wput(v.ptr, w, h, 2, 39, -214);
    else
      wput(v.ptr, w, h, 2, 39, -234);
  } else {
    if (ModButton && ModWindow == v.order)
      wput(v.ptr, w, h, 2, 39, -215);
    else
      wput(v.ptr, w, h, 2, 39, -235);
  }
}

void MOD2(void) {
  modinfo *mymodinfo = (modinfo *)v.aux;
  int need_refresh = 0;

  _process_items();

  ModButton = 0;
  if (wmouse_in(2, 39, 19, 9)) {
    if (mouse_b & 1) {
      ModButton = 1;
      ModWindow = v.order;
      need_refresh = 1;
    } else if (prev_mouse_buttons & 1) {
      ModWindow = v.order;
      need_refresh = 1;
#ifdef MIXER
      if (Mix_PlayingMusic() && mymodinfo->song_code == song_code) {
        free_mod();
        Mix_FreeMusic(mymodinfo->music);
      } else {
        free_mod();
        sound_play_song(mymodinfo->pathname);
      }
#endif
    }
  } else if (ModWindow != -1)
    ModWindow = -1, need_refresh = 1;

  if (need_refresh) {
    call(v.paint_handler);
    v.redraw = 1;
  }
}

void MOD3(void) {
  modinfo *mymodinfo = (modinfo *)v.aux;
#ifdef MIXER
  if (mymodinfo->song_code == song_code) {
    free_mod();
    Mix_FreeMusic(mymodinfo->music);
  }
#endif
  free(v.aux);
}

void MOD0(void) {
  modinfo *mymodinfo;

  v.type = 107;
  v.w = 68;
  v.h = 50;

  v.paint_handler = MOD1;
  v.click_handler = MOD2;
  v.close_handler = MOD3;

  v.aux = modinfo_aux;
  modinfo_aux = NULL;
  mymodinfo = (modinfo *)v.aux;

  mymodinfo->song_code = 0;

  v.title = (byte *)mymodinfo->name;
  v.name = (byte *)mymodinfo->name;
}

void free_mod(void) {
  Mix_HaltMusic();
  Mix_SetPostMix(NULL, NULL);
  SongType = 0;
}
int songpos;
int songline;
int sound_get_song_pos(void) {
  return songpos;
}

int sound_get_song_line(void) {
  return songline;
}

void mostrar_mod_meters(void) {
  modinfo *mymodinfo = (modinfo *)v.aux;
  int w = v.w / big2, h = v.h / big2;
  int x, y, con, canal, ancho_barra;
  int ini = v.w * v.h - v.w * ((2 + 10) * big2 + 1) + 2 * big2;
  char cwork[10];

  if (mymodinfo->song_code == song_code) {
    if (Mix_PlayingMusic()) {
      wbox(v.ptr, w, h, c1, 2, 10, w - 4, h - 12 - 10);
      wbox(v.ptr, w, h, c2, 22, 39, w - 4 - 20, 9);

      wwrite(v.ptr, w, h, 29, 40, 2, (byte *)"L ", c1);
      wwrite(v.ptr, w, h, 28, 40, 2, (byte *)"L ", c4);
      div_snprintf(cwork, sizeof(cwork), "%03d", sound_get_song_line());
      wwrite(v.ptr, w, h, 42, 40, 2, (byte *)cwork, c3);

      wwrite(v.ptr, w, h, 52, 40, 2, (byte *)"P ", c1);
      wwrite(v.ptr, w, h, 51, 40, 2, (byte *)"P ", c4);
      div_snprintf(cwork, sizeof(cwork), "%03d", sound_get_song_pos());
      wwrite(v.ptr, w, h, 64, 40, 2, (byte *)cwork, c3);

      ancho_barra = (v.w - 4 * big2) / SongChannels;

      for (canal = 0; canal < SongChannels; canal++) {
        x = ancho_barra * canal;
        y = (int)rand() % 25 * big2; //(judas_getvumeter(canal)*(float)25)*big2;

        for (con = 0; con < y; con++) {
          if (canal & 1)
            memset(v.ptr + ini + x - con * v.w, c_g_low, ancho_barra);
          else
            memset(v.ptr + ini + x - con * v.w, c_g, ancho_barra);
        }
      }
    } else {
      song_code++;
      last_mod_clean = 0;
    }
    v.redraw = 1;
  } else if (mymodinfo->song_code == song_code - 1) {
    if (!last_mod_clean) {
      call(v.paint_handler);
      v.redraw = 1;
      last_mod_clean = 1;
    }
  }
}

//-----------------------------------------------------------------------------
//  Funciones de sonidos
//-----------------------------------------------------------------------------

#define MAX_FILES 512 // ------------------------------- Listbox de archivos
extern struct t_listboxbr file_list_br;
extern t_thumb thumb[MAX_FILES];
extern int num_taggeds;

typedef struct _HeadDC {
  unsigned int dwUnknow;
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned int dwSamplePerSec;
  unsigned int dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBits;
} HeadDC;

#ifdef MIXER
Mix_Chunk *DIVMIX_LoadPCM(char *path) {
  FILE *f;
  HeadDC dc_header;
  char *riff = "RIFF";
  char *wavefmt = "WAVEfmt ";
  char *data = "data";
  byte *dst;
  byte *ptr;
  int32_t iLen, Len;
  SDL_RWops *rw;
  Mix_Chunk *smp = NULL;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  _splitpath(path, (char *)drive, (char *)dir, (char *)fname, (char *)ext);
  strupr((char *)ext);

  // if file isnt a pcm, reject
  if (strcmp(ext, ".PCM")) {
    return NULL;
  }

  f = fopen(path, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    Len = ftell(f);
    fseek(f, 0, SEEK_SET);

    dst = (byte *)malloc((int)Len + 50);
    ptr = (byte *)malloc((int)Len + 50);

    iLen = Len + 50;

    if (dst == NULL || ptr == NULL) {
      if (dst)
        free(dst);
      if (ptr)
        free(ptr);
      fclose(f);
      return NULL;
    }

    memset(dst, 0, (int)Len + 40);
    fread(ptr, 1, Len, f);
    fclose(f);

    dc_header.dwUnknow = 16;
    dc_header.wFormatTag = 1;
    dc_header.wChannels = 1;
    dc_header.dwSamplePerSec = 11025;
    dc_header.dwAvgBytesPerSec = 11025;
    dc_header.wBlockAlign = 1;
    dc_header.wBits = 8;

    memcpy(dst, riff, 4);
    memcpy(dst + 4, &iLen, 4);
    memcpy(dst + 8, wavefmt, 8);
    memcpy(dst + 16, &dc_header, sizeof(HeadDC));
    memcpy(dst + 16 + sizeof(HeadDC), data, 4);
    memcpy(dst + 20 + sizeof(HeadDC), &Len, 4);
    memcpy(dst + 24 + sizeof(HeadDC), ptr, Len);

    rw = SDL_RWFromMem((void *)dst, (int)(Len + 24 + sizeof(HeadDC)));
    smp = Mix_LoadWAV_RW(rw, 1);
    free(dst);
    free(ptr);
  }
  return smp;
}
#endif

void open_sound(void) {
  pcminfo *mypcminfo;
  Uint32 wav_length;
  Uint8 *wav_buffer;
  FILE *f;

#ifdef MIXER

  Mix_Chunk *si;

#endif

  int num;

  v_mode = 0;
  v_type = 7;
  v_text = (char *)texts[341];
  show_dialog(browser0);
  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    if ((f = fopen(full, "rb")) != NULL) {
      fclose(f);
      v_exists = 1;
    } else
      v_exists = 0;
    div_strcpy(file_list_br.list, file_list_br.item_width, input);
    file_list_br.total_items = 1;
    thumb[0].tagged = 1;
    num_taggeds = 1;
  }

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);
      DIV_STRCPY(sound_name, input);
      DIV_STRCPY(sound_path_name, full);

      if (!v_exists) {
        v_text = (char *)texts[43];
        show_dialog(err0);
        continue;
      }

      if ((pcminfo_aux = (byte *)malloc(sizeof(pcminfo))) == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        continue;
      }

      memset(pcminfo_aux, 0, sizeof(pcminfo));

      mypcminfo = (pcminfo *)pcminfo_aux;

#ifdef MIXER

      si = Mix_LoadWAV(sound_path_name);

      if (si == NULL)
        si = DIVMIX_LoadPCM(sound_path_name);

      if (si == NULL) {
        free(pcminfo_aux);
        v_text = (char *)texts[46];
        show_dialog(err0);
        continue;
      }

#endif

      memcpy(mypcminfo->name, sound_name, 14);
      memcpy(mypcminfo->pathname, sound_path_name, 256);
      mypcminfo->sound_freq = 44100;
      mypcminfo->sound_bits = 16;
#ifdef MIXER
      mypcminfo->sound_size = si->alen / 2;
      mypcminfo->sound_data = (short *)malloc(si->alen);
      if (mypcminfo->sound_data != NULL)
        memcpy(mypcminfo->sound_data, si->abuf, si->alen);

      mypcminfo->si = si;
#endif
      new_window(pcm0);
    }
  }
}

void open_sound_file(void) // Open the file sound_path_name
{
  fprintf(stdout, "TODO - divpcm.cpp open_sound_file\n");

  pcminfo *mypcminfo;
#ifdef MIXER
  Mix_Chunk *si = NULL;
#endif
  debugprintf("sound_name %s\n", input);
  debugprintf("SOundPath %s\n", full);


  DIV_STRCPY(sound_name, input);
  DIV_STRCPY(sound_path_name, full);

  if ((pcminfo_aux = (byte *)malloc(sizeof(pcminfo))) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  mypcminfo = (pcminfo *)pcminfo_aux;

#ifdef MIXER

  si = Mix_LoadWAV(sound_path_name);

  if (si == NULL)
    si = DIVMIX_LoadPCM(sound_path_name);

  if (si == NULL) {
    //     free(pcminfo_aux);
    //if(si) free(si);
    v_text = (char *)texts[46];
    show_dialog(err0);
    free(mypcminfo);
    return;
  }

#endif
  memcpy(mypcminfo->name, sound_name, 14);
  memcpy(mypcminfo->pathname, sound_path_name, 256);
  mypcminfo->sound_freq = 44100;
  mypcminfo->sound_bits = 16;
#ifdef MIXER
  mypcminfo->sound_size = si->alen / 2;
  mypcminfo->sound_data = (short *)malloc(si->alen);
  if (mypcminfo->sound_data != NULL)
    memcpy(mypcminfo->sound_data, si->abuf, si->alen);

  mypcminfo->si = si;

#endif
  new_window(pcm0);
}
int create_saved_window(void_return_type_t init_handler, int nx, int ny);
//void create_saved_window(int init_handler,int nx,int ny);
extern struct twindow window_aux;

void open_desktop_sound(FILE *f) {
  pcminfo *mypcminfo;

  pcminfo_aux = (byte *)malloc(sizeof(pcminfo));
  if (pcminfo_aux == NULL)
    return;
  mypcminfo = (pcminfo *)pcminfo_aux;

  fread(mypcminfo->name, 1, 14, f);
  fread(mypcminfo->pathname, 1, 256, f);
  fread(&mypcminfo->sound_freq, 1, 4, f);
  fread(&mypcminfo->sound_bits, 1, 4, f);
  fread(&mypcminfo->sound_size, 1, 4, f);

  if (!new_sample(mypcminfo)) {
    free(pcminfo_aux);
    fseek(f, mypcminfo->sound_size * 2, SEEK_CUR);
    return;
  }

  fread(mypcminfo->sound_data, 2, mypcminfo->sound_size, f);

#ifdef MIXER
  {
    byte *file_buffer = save_sound_mem(mypcminfo);
    if (file_buffer != NULL) {
      int wavSize = mypcminfo->sound_size * 2 + 44;
      SDL_RWops *rw = SDL_RWFromMem(file_buffer, wavSize);
      mypcminfo->si = Mix_LoadWAV_RW(rw, 1);
      free(file_buffer);
    }
    if (mypcminfo->si == NULL) {
      free(mypcminfo->sound_data);
      free(pcminfo_aux);
      return;
    }
  }
#endif

  create_saved_window(pcm0, window_aux.x, window_aux.y);
}

void save_sound(pcminfo *mypcminfo, char *dst) {
  printf("TODO - divpcm.cpp save_sound\n");
  FILE *dstfile;
  HeadDC dc_header;
  int length;
  int con;
  byte *byte_ptr = (byte *)mypcminfo->sound_data;
  float paso, pos;
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  _splitpath(dst, (char *)drive, (char *)dir, (char *)fname, (char *)ext);
  strupr((char *)ext);
  length = mypcminfo->sound_size;

  if ((dstfile = fopen(dst, "wb")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }

  if (!strcmp(ext, ".WAV")) {
    if (mypcminfo->sound_bits == 16)
      length *= 2;
    fputc('R', dstfile);
    fputc('I', dstfile);
    fputc('F', dstfile);
    fputc('F', dstfile);

    length += 36;
    if (fwrite(&length, 1, 4, dstfile) != 4) {
      fclose(dstfile);
      delete_file(dst);
      v_text = (char *)texts[47];
      show_dialog(err0);
      return;
    }
    length -= 36;

    fputc('W', dstfile);
    fputc('A', dstfile);
    fputc('V', dstfile);
    fputc('E', dstfile);
    fputc('f', dstfile);
    fputc('m', dstfile);
    fputc('t', dstfile);
    fputc(' ', dstfile);

    dc_header.dwUnknow = 16;
    dc_header.wFormatTag = 1;
    dc_header.wChannels = 1;
    dc_header.dwSamplePerSec = mypcminfo->sound_freq;
    dc_header.dwAvgBytesPerSec = mypcminfo->sound_freq * (mypcminfo->sound_bits / 8);
    dc_header.wBlockAlign = 1;
    dc_header.wBits = mypcminfo->sound_bits;

    if (fwrite(&dc_header, 1, sizeof(HeadDC), dstfile) != sizeof(HeadDC)) {
      fclose(dstfile);
      delete_file(dst);
      v_text = (char *)texts[47];
      show_dialog(err0);
      return;
    }

    fputc('d', dstfile);
    fputc('a', dstfile);
    fputc('t', dstfile);
    fputc('a', dstfile);

    if (fwrite(&length, 1, 4, dstfile) != 4) {
      fclose(dstfile);
      delete_file(dst);
      v_text = (char *)texts[47];
      show_dialog(err0);
      return;
    }
    if (mypcminfo->sound_bits == 8) {
      if ((byte_ptr = (byte *)malloc(length)) == NULL) {
        fclose(dstfile);
        delete_file(dst);
        v_text = (char *)texts[45];
        show_dialog(err0);
        return;
      }
      for (con = 0; con < length; con++)
        byte_ptr[con] = mypcminfo->sound_data[con] / 0xFF + 0x80;
    }
  } else {
    paso = (float)mypcminfo->sound_freq / (float)11025;
    pos = (float)length / paso;
    length = (int)(pos + 0.5);
    if ((byte_ptr = (byte *)malloc(length)) == NULL) {
      fclose(dstfile);
      delete_file(dst);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    pos = 0;
    for (con = 0; con < length; con++) {
      byte_ptr[con] = mypcminfo->sound_data[(int)pos] / 0xFF + 0x80;
      pos += paso;
    }
  }

  if (fwrite(byte_ptr, 1, length, dstfile) != length) {
    fclose(dstfile);
    if (byte_ptr != (byte *)mypcminfo->sound_data)
      free(byte_ptr);
    delete_file(dst);
    v_text = (char *)texts[47];
    show_dialog(err0);
    return;
  }

  if (byte_ptr != (byte *)mypcminfo->sound_data)
    free(byte_ptr);
  fclose(dstfile);
}

void save_desktop_sound(pcminfo *mypcminfo, FILE *f) {
  fwrite(mypcminfo->name, 1, 14, f);
  fwrite(mypcminfo->pathname, 1, 256, f);
  fwrite(&mypcminfo->sound_freq, 1, 4, f);
  fwrite(&mypcminfo->sound_bits, 1, 4, f);
  fwrite(&mypcminfo->sound_size, 1, 4, f);
  fwrite(mypcminfo->sound_data, 2, mypcminfo->sound_size, f);
}

void close_sound(char *snd_path) {
  pcminfo *mypcminfo;
  int m;

  for (m = 0; m < max_windows; m++) {
    if (window[m].type == 105) {
      mypcminfo = (pcminfo *)window[m].aux;
      if (!strcmp(snd_path, mypcminfo->pathname)) {
        move(0, m);
        close_window();
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//  Funciones de canciones
//-----------------------------------------------------------------------------

void open_song(void) {
  int num;
  modinfo *mymodinfo;
  FILE *f;

  v_mode = 0;
  v_type = 16;
  v_text = (char *)texts[558];
  show_dialog(browser0);
  if (!v_finished)
    return;

  if (!num_taggeds) {
    div_strcpy(full, sizeof(full), file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    if ((f = fopen(full, "rb")) != NULL) {
      fclose(f);
      v_exists = 1;
    } else
      v_exists = 0;
    div_strcpy(file_list_br.list, file_list_br.item_width, input);
    file_list_br.total_items = 1;
    thumb[0].tagged = 1;
    num_taggeds = 1;
  }

  for (num = 0; num < file_list_br.total_items; num++) {
    if (thumb[num].tagged) {
      div_strcpy(input, sizeof(input), file_list_br.list + file_list_br.item_width * num);
      div_strcpy(full, sizeof(full), file_types[v_type].path);
      if (full[strlen(full) - 1] != '/')
        div_strcat(full, sizeof(full), "/");
      div_strcat(full, sizeof(full), input);
      DIV_STRCPY(song_name, input);
      DIV_STRCPY(song_path_name, full);

      if (!v_exists) {
        v_text = (char *)texts[43];
        show_dialog(err0);
        continue;
      }

      if ((modinfo_aux = (byte *)malloc(sizeof(modinfo))) == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        continue;
      }
      mymodinfo = (modinfo *)modinfo_aux;

      memcpy(mymodinfo->name, song_name, 14);
      memcpy(mymodinfo->pathname, song_path_name, 256);

      new_window(MOD0);
    }
  }
}

void open_desktop_song(void) {
  modinfo *mymodinfo;

  if ((modinfo_aux = (byte *)malloc(sizeof(modinfo))) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  mymodinfo = (modinfo *)modinfo_aux;

  memcpy(mymodinfo->name, song_name, 14);
  memcpy(mymodinfo->pathname, song_path_name, 256);

  create_saved_window(MOD0, window_aux.x, window_aux.y);
}

int songposcount = 0;
// make a passthru processor function that does nothing...
void noEffect(void *udata, Uint8 *stream, int len) {
  songposcount++;
  if (songposcount == 32) {
    songpos++;
    songposcount = 0;
    if (songpos == 64) {
      songpos = 1;
      songline++;
      // stop overflow
      if (songline == 1000)
        songline = 0;
    }
  }
}

void sound_play_song(char *pathname) {
  printf("TODO - divpcm.cpp sound_play_song\n");
#ifdef MIXER
  Mix_Music *music;
  modinfo *mymodinfo = (modinfo *)v.aux;
  song_code++;
  mymodinfo->song_code = song_code;
  last_mod_clean = 0;

  printf("loadsong [%s]\n", pathname);

  music = Mix_LoadMUS(pathname);

  mymodinfo->music = music;

  if (!music) {
    v_text = strdup((char *)Mix_GetError()); //texts[46];
    if (strlen(v_text) == 0) {
      free(v_text);
      v_text = strdup((const char *)texts[46]);
    }
    show_dialog(err0);
    free(v_text);
    return;
  }
  songposcount = 0;
  songpos = 1;
  songline = 1;
  Mix_SetPostMix(noEffect, NULL);
  if (Mix_PlayMusic(music, 1) == -1) {
    debugprintf("Mix_PlayMusic: %s\n", Mix_GetError());
    // well, there's no music, but most games don't break without music...
  }

  SongChannels = 8;

#endif
}

//-----------------------------------------------------------------------------
//  Editor de sonidos
//-----------------------------------------------------------------------------

int window_width, window_height;
int sel_1, sel_2;

typedef struct _CLP {
  int sound_freq;
  int sound_size;
  short *sound_data;
} CLP;

typedef struct _SND {
  char name[14];
  int sound_freq;
  int sound_bits;
  int sound_size;
  short *sound_data;
} SND;

SND DesktopSND[100];
int NumSND = 0;    // Contador de sonidos pegados durante una edicion
int snd_count = 1; // Contador de sonidos pegados durante toda la sesion
//SAMPLE  sample;
CLP sound_clipboard = {0, 0, NULL};
int PosY = 31;

void edit_sound0(void) {
  int pos;

  v.type = 1;
  v.title = texts[349];
  v.name = texts[349];
  v.w = 308;
  v.h = 151; // 116

  v.paint_handler = edit_sound1;
  v.click_handler = edit_sound2;
  v.close_handler = EditSound3;

  window_width = (v.w - 8) * big2;
  window_height = 64 * big2;
  sel_1 = 0;
  sel_2 = 0;

  for (NumSND = 0; NumSND < 100; NumSND++) {
    DesktopSND[NumSND].name[0] = 0;
    DesktopSND[NumSND].sound_freq = 0;
    DesktopSND[NumSND].sound_bits = 0;
    DesktopSND[NumSND].sound_size = 0;
    DesktopSND[NumSND].sound_data = NULL;
  }
  NumSND = 0;

  pos = 7;
  _button(501, pos, v.h - 34, 0);
  pos += text_len(texts[501]) + 3;
  _button(502, pos, v.h - 34, 0);
  pos += text_len(texts[502]) + 3;
  _button(503, pos, v.h - 34, 0);
  pos += text_len(texts[503]) + 3;
  _button(504, pos, v.h - 34, 0);

  pos += text_len(texts[504]) + 3;
  pos += v.w - 1 - text_len(texts[506]);
  pos -= text_len(texts[507]) + 3;
  pos -= text_len(texts[508]) + 3;
  pos -= text_len(texts[505]) + 3;
  pos /= 2;
  _button(505, pos, v.h - 34, 0);

  pos = v.w - 1 - text_len(texts[506]);
  _button(506, pos, v.h - 34, 0);
  pos -= text_len(texts[507]) + 3;
  _button(507, pos, v.h - 34, 0);
  pos -= text_len(texts[508]) + 3;
  _button(508, pos, v.h - 34, 0);

  _button(509, 7, v.h - 14, 0);       // Prueba
  _button(512, v.w - 8, v.h - 14, 2); // Copiar en window

  pos = 193 + text_len(texts[555]) + 3; // 241;

  _button(553, pos, 15, 0);
  pos += text_len(texts[553]) + 3;
  _button(554, pos, 15, 0);

  pos = 193;

  _button(555, pos, 30, 0);
  pos += text_len(texts[555]) + 3;
  _button(556, pos, 30, 0);
  pos += text_len(texts[556]) + 3;
  _button(557, pos, 30, 0);
  pos += text_len(texts[557]) + 3;

  v_finished = 0;
}

void edit_sound1(void) {
  pcminfo *mypcminfo = (pcminfo *)pcminfo_aux;
  short *buffer;
  byte *ptr = v.ptr;
  float step, position = 0;
  int w = v.w / big2, h = v.h / big2;
  int w_v = window_width / big2, h_v = window_height / big2;
  int length;
  int x, y, y0, y1, p0, p1;
  int width, height, first = 1, lx, ly;
  short sample;
  char cwork[16];

  _show_items();

  // Info del sounds
  wbox(v.ptr, w, h, c1, 4, 12, 69, 28);
  wrectangle(v.ptr, w, h, c0, 3, 11, 71, 30);
  wrectangle(v.ptr, w, h, c0, 3, 21, 71, 11);
  wwrite(v.ptr, w, h, 39, 13, 1, (byte *)mypcminfo->name, c3);
  itoa(mypcminfo->sound_freq, cwork, 10);
  wwrite(v.ptr, w, h, 39, 23, 1, (byte *)cwork, c3);
  div_snprintf(cwork, sizeof(cwork), "%02d bit", mypcminfo->sound_bits);
  wwrite(v.ptr, w, h, 39, 33, 1, (byte *)cwork, c3);

  // Opciones de conversion

  // Zona de edicion
  wrectangle(v.ptr, w, h, c0, 3, 11 + PosY, w_v + 2, h_v + 2);
  wbox(v.ptr, w, h, c1, 4, 12 + PosY, w_v, h_v);

  wrectangle(v.ptr, w, h, c0, 1, 11 + PosY + h_v + 5, w - 2, 19);

  if (sel_1 <= sel_2) {
    //  No pone cajas de 1 de width
    //  wbox(v.ptr, an, al, c3, (sel_1/big2)+4, 12+PosY, (sel_2/big2)-(sel_1/big2)+1, h_v);

    p0 = 4 * big2;
    y0 = (12 + PosY) * big2;
    for (x = sel_1 + p0; x < sel_2 + p0 + 1; x++)
      for (y = 0; y < window_height; y++)
        ptr[x + (y0 + y) * v.w] = c3;
  } else {
    //  No pone cajas de 1 de width
    //  wbox(v.ptr, an, al, c3, (sel_2/big2)+4, 12+PosY, (sel_1/big2)-(sel_2/big2)+1, h_v);

    p0 = 4 * big2;
    y0 = (12 + PosY) * big2;
    for (x = sel_2 + p0; x < sel_1 + p0 + 1; x++)
      for (y = 0; y < window_height; y++)
        ptr[x + (y0 + y) * v.w] = c3;
  }

  width = window_width;
  height = window_height;
  ptr += (4 + ((12 + PosY) * v.w)) * big2 + (height / 2) * v.w;
#ifdef MIXER
  buffer = (short *)mypcminfo->si->abuf;
  length = mypcminfo->si->alen / 2;
#endif
  if (length > 1) {
    if (length < 3 * width) {
      step = (float)width / (float)(length - 1);
      for (x = 0; x < length; x++) {
        sample = buffer[x], y = (sample * height / 65536);

        if (first) {
          first = 0;
          lx = (int)position;
          ly = y;
        } else {
          wline((char *)ptr, v.w, width, height, lx, ly, (int)position, y, c_g_low);
          lx = (int)position;
          ly = y;
        }
        position += step;
      }
    } else {
      step = (float)length / (float)width;
      for (x = 0; x < width; x++) {
        p0 = (memptrsize)position;
        position += step;
        p1 = (memptrsize)position;

        sample = buffer[p0];
        y0 = y1 = (sample * height / 65536);

        do {
          sample = buffer[p0], y = (sample * height / 65536);
          if (y < y0)
            y0 = y;
          else if (y > y1)
            y1 = y;
          p0 += 2;
        } while (p0 < p1);

        y = y0;
        do {
          ptr[x + y * v.w] = c_g_low;
        } while (y++ < y1);
      }
    }
  } // length>1
}

void edit_sound2(void) {
  int w_v = window_width / big2, h_v = window_height / big2;
  int need_refresh = 0;

  _process_items();

  if (v.active_item != -1) {
    modify_sound(v.active_item);
    need_refresh = 1;
  }

  if (wmouse_in(4, 12 + PosY, w_v, h_v)) {
    if (mouse_b & 1) {
      if (!(prev_mouse_buttons & 1))
        sel_1 = mouse_x - v.x - (4 * big2), sel_2 = mouse_x - v.x - (4 * big2);
      else
        sel_2 = mouse_x - v.x - (4 * big2);
      need_refresh = 1;
    }
    if (mouse_b & 2) {
      sel_1 = 0, sel_2 = window_width - 1;
      need_refresh = 1;
    }
  }

  if (need_refresh) {
    call(v.paint_handler);
    v.redraw = 1;
  }
}

void EditSound3(void) {
  Mix_HaltChannel(-1);
}

//-----------------------------------------------------------------------------
//  Grabador de sonidos
//-----------------------------------------------------------------------------

void rec_sound0(void) {
  v.type = 1;
  v.title = texts[560];
  v.name = texts[560];
  v.w = 200;
  v.h = 68;

  v.paint_handler = RecSound1;
  v.click_handler = RecSound2;
  v.close_handler = RecSound3;

  DIV_STRCPY(sound_file, file_types[7].path);
  if (sound_file[strlen(sound_file) - 1] != '/')
    DIV_STRCAT(sound_file, "/");
  DIV_STRCAT(sound_file, "SAMPLE.WAV");

  // Botones Aceptar/Cancelar
  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);

  // Flags MIC/CD
  _flag(561, 3, 20, &rec_device[0]);
  _flag(562, 3 + 11 + text_len(texts[561]), 20, &rec_device[1]);

  // Boton de seleccion de filename
  _button(121, v.w - 12, 17, 0);

  v_accept = 0;
}

void RecSound1(void) {
  printf("TODO - divpcm.cpp RecSound1\n");

  int w = v.w / big2, h = v.h / big2;

  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];
  char cwork[13];

  _show_items();

  // Fuente de grabacion
  wwrite(v.ptr, w, h, 3, 11, 0, texts[565], c3);

  // Fichero WAV a grabar
  wwrite(v.ptr, w, h, w - 86, 11, 0, texts[566], c3);
  _splitpath(sound_file, drive, dir, fname, ext);
  DIV_STRCPY(cwork, fname);
  DIV_STRCAT(cwork, ext);
  wbox(v.ptr, w, h, c12, w - 86, 19, 69, 8);
  wwrite_in_box(v.ptr, w, w - 12, h, w - 85, 20, 0, (byte *)cwork, c4);

  // Mensaje de ayuda
  wbox(v.ptr, w, h, c12, 3, 29, w - 6, 20);
  wrectangle(v.ptr, w, h, c0, 3, 29, w - 6, 20);
  wwrite_in_box(v.ptr, w, w - 4, h, w / 2, 31, 1, texts[563], c3);
  wwrite_in_box(v.ptr, w, w - 4, h, w / 2, 40, 1, texts[564], c3);
}

void RecSound2(void) {
  int need_refresh = 0;

  _process_items();

  switch (v.active_item) {
  case 0: // Aceptar
    end_dialog = 1;
    v_accept = 1;
    break;
  case 1: // Cancelar
    end_dialog = 1;
    break;
  case 2: // MIC
    rec_device[1] = !rec_device[0];
    need_refresh = 1;
    break;
  case 3: // CD
    rec_device[0] = !rec_device[1];
    need_refresh = 1;
    break;
  case 4: // Seleccion de filename
    v_mode = 1;
    v_type = 7;
    v_text = (char *)texts[339];
    show_dialog(browser0);

    div_strcpy(full, sizeof(full), file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);

    if (v_finished) {
      if (v_exists) {
        v_title = (char *)texts[340];
        v_text = input;
        show_dialog(accept0);
      } else
        v_accept = 1;
      if (v_accept) {
        DIV_STRCPY(sound_file, full);
        need_refresh = 1;
      }
      v_accept = 0;
    }
    break;
  }
  if (need_refresh) {
    call(v.paint_handler);
    v.redraw = 1;
  }
}

void RecSound3(void) {
  if (v_accept) {
    record_sound();
  }
}

void modify_sound(int option) {
  printf("TODO - divpcm.cpp modify_sound\n");
#ifdef MIXER
  SDL_RWops *rw;
  pcminfo *mypcminfo = (pcminfo *)pcminfo_aux;
  pcminfo pcminfo_bak;
  Mix_Chunk *si = NULL;
  byte *file_buffer;
  float inicio, final, tam_rel, fade;
  int pos, value, n;
  int ini, fin;
  short *buffer = (short *)mypcminfo->si->abuf;
  short *short_ptr = sound_clipboard.sound_data;
  int length;
  float paso, pos_f;

  if (sel_1 <= sel_2)
    inicio = sel_1, final = sel_2 + 1;
  else
    inicio = sel_2, final = sel_1 + 1;

  tam_rel = (float)mypcminfo->sound_size / (float)window_width;

  ini = (float)inicio * tam_rel;
  fin = (float) final * tam_rel;
  if (final == window_width)
    fin = mypcminfo->sound_size;

  Mix_HaltChannel(-1);

  switch (option) {
  case 5: // Copy
    if (sound_clipboard.sound_data != NULL) {
      free(sound_clipboard.sound_data);
      sound_clipboard.sound_data = NULL;
    }
    if (mypcminfo->sound_data == NULL)
      return;
    sound_clipboard.sound_freq = mypcminfo->sound_freq;
    sound_clipboard.sound_size = fin - ini;
    if (sound_clipboard.sound_size <= 0)
      return;
    sound_clipboard.sound_data = (short *)malloc(sound_clipboard.sound_size * 2);
    if (sound_clipboard.sound_data == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    memcpy(sound_clipboard.sound_data, buffer + ini, sound_clipboard.sound_size * 2);
    return;
  case 6: // Cut
    if (sound_clipboard.sound_data != NULL) {
      free(sound_clipboard.sound_data);
      sound_clipboard.sound_data = NULL;
    }
    if (mypcminfo->sound_data == NULL)
      return;
    sound_clipboard.sound_freq = mypcminfo->sound_freq;
    sound_clipboard.sound_size = fin - ini;
    if (sound_clipboard.sound_size <= 0)
      return;
    sound_clipboard.sound_data = (short *)malloc(sound_clipboard.sound_size * 2);
    if (sound_clipboard.sound_data == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    memcpy(sound_clipboard.sound_data, buffer + ini, sound_clipboard.sound_size * 2);
    if (mypcminfo->sound_size - sound_clipboard.sound_size <= 0) {
      if (mypcminfo->sound_data) {
        mypcminfo->sound_data = NULL;
        mypcminfo->sound_size = 0;
      }
    } else {
      if ((pcminfo_bak.sound_data =
               (short *)malloc((mypcminfo->sound_size - sound_clipboard.sound_size) * 2)) == NULL) {
        free(sound_clipboard.sound_data);
        sound_clipboard.sound_data = NULL;
        v_text = (char *)texts[45];
        show_dialog(err0);
        return;
      }
      pcminfo_bak.sound_bits = mypcminfo->sound_bits;
      pcminfo_bak.sound_freq = mypcminfo->sound_freq;
      pcminfo_bak.sound_size = mypcminfo->sound_size - sound_clipboard.sound_size;

      memcpy(pcminfo_bak.sound_data, buffer, ini * 2);
      memcpy(pcminfo_bak.sound_data + ini, buffer + fin, (mypcminfo->sound_size - fin) * 2);

      if ((file_buffer = save_sound_mem(&pcminfo_bak)) == NULL) {
        free(pcminfo_bak.sound_data);
        free(sound_clipboard.sound_data);
        sound_clipboard.sound_data = NULL;
        v_text = (char *)texts[45];
        show_dialog(err0);
        return;
      }
      si = Mix_QuickLoad_RAW(file_buffer, file_pos);
      pcminfo_bak.si = si;

      if (si == NULL) {
        if (file_buffer)
          free(file_buffer);
        free(pcminfo_bak.sound_data);
        free(sound_clipboard.sound_data);
        sound_clipboard.sound_data = NULL;
        v_text = (char *)texts[45];
        show_dialog(err0);
        return;
      }

      if (mypcminfo->sound_data) {
        mypcminfo->sound_data = NULL;
        mypcminfo->sound_size = 0;
      }

      mypcminfo->sound_size = si->alen / 2;

      mypcminfo->sound_data = (short *)malloc(si->alen);

      if (mypcminfo->sound_data != NULL)
        memcpy(mypcminfo->sound_data, si->abuf, si->alen);

      mypcminfo->sample = NULL;
      mypcminfo->si = si;

      free(file_buffer);
      free(pcminfo_bak.sound_data);
    }
    return;
  case 7: // Paste
    if (sound_clipboard.sound_data == NULL)
      return;
    if (sound_clipboard.sound_size <= 0)
      return;
    length = sound_clipboard.sound_size;
    if (sound_clipboard.sound_freq != mypcminfo->sound_freq) {
      paso = (float)sound_clipboard.sound_freq / (float)mypcminfo->sound_freq;
      pos_f = (float)length / paso;
      length = (int)(pos_f + 0.5);
      pos_f = 0;
      if ((short_ptr = (short *)malloc(length * 2)) == NULL) {
        v_text = (char *)texts[45];
        show_dialog(err0);
        return;
      }
      for (pos = 0; pos < length; pos++) {
        short_ptr[pos] = sound_clipboard.sound_data[(int)pos_f];
        pos_f += paso;
      }
    }

    if ((pcminfo_bak.sound_data = (short *)malloc((mypcminfo->sound_size + length) * 2)) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    pcminfo_bak.sound_bits = mypcminfo->sound_bits;
    pcminfo_bak.sound_freq = mypcminfo->sound_freq;
    pcminfo_bak.sound_size = mypcminfo->sound_size + length;

    memcpy(pcminfo_bak.sound_data, buffer, ini * 2);
    memcpy(pcminfo_bak.sound_data + ini, short_ptr, length * 2);
    memcpy(pcminfo_bak.sound_data + ini + length, buffer + ini, (mypcminfo->sound_size - ini) * 2);
    if (sound_clipboard.sound_freq != mypcminfo->sound_freq)
      free(short_ptr);

    if ((file_buffer = save_sound_mem(&pcminfo_bak)) == NULL) {
      free(pcminfo_bak.sound_data);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }
    free(pcminfo_bak.sound_data);
    si = Mix_QuickLoad_RAW(file_buffer, file_pos);
    pcminfo_bak.si = si;

    if (si == NULL) {
      if (file_buffer)
        free(file_buffer);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    if (mypcminfo->sound_data) {
      mypcminfo->sound_data = NULL;
      mypcminfo->sound_size = 0;
    }

    mypcminfo->sound_size = si->alen / 2;

    mypcminfo->sound_data = (short *)malloc(si->alen);
    if (mypcminfo->sound_data != NULL)
      memcpy(mypcminfo->sound_data, si->abuf, si->alen);

    mypcminfo->sample = NULL;
    mypcminfo->si = si;

    free(file_buffer);
    return;
  case 8: // Play
    if (mypcminfo->sound_data == NULL)
      return;

    Mix_PlayChannel(-1, mypcminfo->si, 0);

    return;
  case 9: // Pega el sounds en el escritorio
    copy_new_sound(mypcminfo, ini, fin);
    return;
  case 10: // Convertir a 8 bit
    mypcminfo->sound_bits = 8;
    return;
  case 11: // Convertir a 16 bit
    mypcminfo->sound_bits = 16;
    return;
  case 12: // Convertir a 11025 Hz
    change_sound_freq(11025);
    return;
  case 13: // Convertir a 22050 Hz
    change_sound_freq(22050);
    return;
  case 14: // Convertir a 44100 Hz
    change_sound_freq(44100);
    return;
  }

  for (pos = ini; pos < fin; pos++) {
    value = buffer[pos];
    switch (option) {
    case 0: // Volume down
      if (value > 0) {
        n = (float)value * 0.1;
        if (n)
          value -= n;
        else
          value--;
      } else if (value < 0) {
        n = (float)value * 0.1;
        if (n)
          value -= n;
        else
          value++;
      }
      break;
    case 1: // Volume up
      if (value > 0) {
        n = (float)value * 0.1;
        if (n)
          value += n;
        else
          value++;
      } else if (value < 0) {
        n = (float)value * 0.1;
        if (n)
          value += n;
        else
          value--;
      }
      break;
    case 2: // Fade in
      fade = (float)(pos - ini) / (float)(fin - ini);
      value = (float)value * fade;
      break;
    case 3: // Fade out
      fade = (float)(pos - ini) / (float)(fin - ini);
      value = (float)value * (1.0 - fade);
      break;
    case 4: // Silence
      value = 0;
      break;
    }
    if (value < -32768)
      value = -32768;
    if (value > 32767)
      value = 32767;
    buffer[pos] = (short)value;
  }
#endif
}

void change_sound_freq(int freq) {
  pcminfo *mypcminfo = (pcminfo *)pcminfo_aux;
  pcminfo pcminfo_bak;
  byte *file_buffer;
  short *short_ptr;
  float paso, pos_f;
  int length = mypcminfo->sound_size;
  int pos;

  if (mypcminfo->sound_data == NULL || mypcminfo->sound_freq == freq)
    return;

  // Resample from current frequency to target frequency (nearest-neighbor)
  paso = (float)mypcminfo->sound_freq / (float)freq;
  pos_f = (float)length / paso;
  length = (int)(pos_f + 0.5);
  pos_f = 0;
  if ((short_ptr = (short *)malloc(length * 2)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  for (pos = 0; pos < length; pos++) {
    short_ptr[pos] = mypcminfo->sound_data[(int)pos_f];
    pos_f += paso;
  }

  // Build a WAV file in memory from the resampled data
  pcminfo_bak.sound_freq = freq;
  pcminfo_bak.sound_size = length;
  pcminfo_bak.sound_data = short_ptr;

  if ((file_buffer = save_sound_mem(&pcminfo_bak)) == NULL) {
    free(short_ptr);
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }

#ifdef MIXER
  {
    int wavSize = length * 2 + 44;
    SDL_RWops *rw = SDL_RWFromMem(file_buffer, wavSize);
    Mix_Chunk *new_si = Mix_LoadWAV_RW(rw, 1);

    if (new_si == NULL) {
      free(file_buffer);
      free(short_ptr);
      v_text = (char *)texts[45];
      show_dialog(err0);
      return;
    }

    // Free old sound data
    if (mypcminfo->sound_data) {
      Mix_FreeChunk(mypcminfo->si);
      mypcminfo->si = NULL;
      mypcminfo->sound_data = NULL;
      mypcminfo->sound_size = 0;
    }

    // Store new resampled sound
    mypcminfo->sound_freq = freq;
    mypcminfo->sound_size = new_si->alen / 2;
    mypcminfo->sound_data = (short *)malloc(new_si->alen);
    if (mypcminfo->sound_data != NULL)
      memcpy(mypcminfo->sound_data, new_si->abuf, new_si->alen);
    mypcminfo->si = new_si;
  }
#endif

  free(file_buffer);
  free(short_ptr);
}


void record_sound(void) {
  printf("TODO - divpcm.cpp record_sound\n");

  HeadDC dc_header;
  FILE *f;
  unsigned short ra;
  unsigned char ca;
  int length;
  debugprintf("sound_file to record: %s\n", sound_file);

  if ((f = fopen(sound_file, "wb")) == NULL) {
    v_text = (char *)texts[43];
    show_dialog(err0);
    return;
  }

  fputc('R', f);
  fputc('I', f);
  fputc('F', f);
  fputc('F', f);

  length += 36;
  if (fwrite(&length, 1, 4, f) != 4) {
    fclose(f);
    delete_file(sound_file);
    v_text = (char *)texts[47];
    show_dialog(err0);
    return;
  }
  length -= 36;

  fputc('W', f);
  fputc('A', f);
  fputc('V', f);
  fputc('E', f);
  fputc('f', f);
  fputc('m', f);
  fputc('t', f);
  fputc(' ', f);

  dc_header.dwUnknow = 16;
  dc_header.wFormatTag = 1;
  dc_header.wChannels = 1;
  dc_header.wBlockAlign = 1;

  if (true /*judascfg_device == DEV_SB16 */) {
    dc_header.dwSamplePerSec = 44100;
    dc_header.dwAvgBytesPerSec = 44100;
    dc_header.wBits = 8;
  } else {
    dc_header.dwSamplePerSec = 22050;
    dc_header.dwAvgBytesPerSec = 22050;
    dc_header.wBits = 8;
  }

  if (fwrite(&dc_header, 1, sizeof(HeadDC), f) != sizeof(HeadDC)) {
    fclose(f);
    delete_file(sound_file);
    v_text = (char *)texts[47];
    show_dialog(err0);
    return;
  }

  fputc('d', f);
  fputc('a', f);
  fputc('t', f);
  fputc('a', f);

  if (fwrite(&length, 1, 4, f) != 4) {
    fclose(f);
    delete_file(sound_file);
    v_text = (char *)texts[47];
    show_dialog(err0);
    return;
  }

  fclose(f);


  if (rec_device[0]) {
    mix_set_input(MIX_IN_MICRO | MIX_NO_FILT);
    mix_set_volume(MIX_MICRO_VOL, 5, 5);
    set_cd_volume(0);
  } else {
    mix_set_input(MIX_IN_CD | MIX_NO_FILT);
  }

  poll_record();
}

void poll_record(void) {
  printf("TODO - divpcm.cpp poll_record\n");

  FILE *f;
  int length;

  if ((f = fopen(sound_file, "ab")) == NULL) {
    v_text =
        (char *)texts[47]; // TODO: Use a more specific error message for sound recording failure
    show_dialog(err0);
    return;
  }

  kbdFLAGS[_ESC] = 0;

  length = ftell(f) - 44;
  fclose(f);
  if ((f = fopen(sound_file, "rb+")) == NULL) {
    v_text =
        (char *)texts[47]; // TODO: Use a more specific error message for sound file reopen failure
    show_dialog(err0);
    return;
  }
  fseek(f, 40, SEEK_SET);
  fwrite(&length, 1, 4, f);
  fclose(f);
}

int judas_progress_read(int handle, void *buffer, int size) {
  int con, pasos, resto;
  byte *byte_ptr = (byte *)buffer;

  pasos = size / 4096;
  resto = size - (pasos * 4096);
  if (pasos < 1)
    pasos = 1, resto = 0;
  for (con = 0; con < pasos; con++) {
    if (read(handle, &byte_ptr[con * 4096], 4096) != 4096) {
      show_progress((char *)texts[559], pasos, pasos);
      return (0);
    }
    show_progress((char *)texts[559], con, pasos);
  }
  if (resto) {
    if (read(handle, &byte_ptr[con * 4096], resto) != resto) {
      show_progress((char *)texts[559], pasos, pasos);
      return (0);
    }
    show_progress((char *)texts[559], pasos, pasos);
  }
  return (1);
}

void copy_new_sound(pcminfo *mypcminfo, int ini, int fin) {
  char aux[5];
  short *sound_data;
  int sound_bytes = (fin - ini) * 2;

  if (fin == -1)
    fin = mypcminfo->sound_size;

  if (mypcminfo->sound_data == NULL || mypcminfo->sound_size == 0 || ini > mypcminfo->sound_size ||
      fin > mypcminfo->sound_size || ini >= fin)
    return;

  if (NumSND > 99) {
    v_text = (char *)texts[574];
    show_dialog(err0);
    return;
  }

  if ((sound_data = (short *)malloc(sound_bytes)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return;
  }
  memcpy(sound_data, mypcminfo->sound_data + ini, sound_bytes);

  div_strcpy(DesktopSND[NumSND].name, sizeof(DesktopSND[NumSND].name), (char *)texts[137]);
  div_strcat(DesktopSND[NumSND].name, sizeof(DesktopSND[NumSND].name), itoa((snd_count), aux, 10));
  DesktopSND[NumSND].sound_freq = mypcminfo->sound_freq;
  DesktopSND[NumSND].sound_bits = mypcminfo->sound_bits;
  DesktopSND[NumSND].sound_size = fin - ini;
  DesktopSND[NumSND].sound_data = sound_data;
  snd_count++;
  if (snd_count > 99)
    snd_count = 0;
  NumSND++;
}

void paste_new_sounds(void) {
  pcminfo *mypcminfo;
  byte *file_buffer;
  int con;

  for (con = 0; con < NumSND; con++) {
    if ((pcminfo_aux = (byte *)malloc(sizeof(pcminfo))) == NULL) {
      v_text = (char *)texts[45];
      show_dialog(err0);
      continue;
    }
    memset(pcminfo_aux, 0, sizeof(pcminfo));
    mypcminfo = (pcminfo *)pcminfo_aux;

    mypcminfo->sound_freq = DesktopSND[con].sound_freq;
    mypcminfo->sound_size = DesktopSND[con].sound_size;
    mypcminfo->sound_data = DesktopSND[con].sound_data;

    if ((file_buffer = save_sound_mem(mypcminfo)) == NULL) {
      free(DesktopSND[con].sound_data);
      free(pcminfo_aux);
      v_text = (char *)texts[46];
      show_dialog(err0);
      continue;
    }

#ifdef MIXER
    {
      int wavSize = mypcminfo->sound_size * 2 + 44;
      SDL_RWops *rw = SDL_RWFromMem(file_buffer, wavSize);
      Mix_Chunk *new_si = Mix_LoadWAV_RW(rw, 1);

      if (new_si == NULL) {
        free(file_buffer);
        free(DesktopSND[con].sound_data);
        free(pcminfo_aux);
        v_text = (char *)texts[46];
        show_dialog(err0);
        continue;
      }

      memcpy(mypcminfo->name, DesktopSND[con].name, 14);
      mypcminfo->pathname[0] = 0;
      mypcminfo->sound_bits = DesktopSND[con].sound_bits;
      mypcminfo->sound_size = new_si->alen / 2;
      mypcminfo->sound_data = (short *)malloc(new_si->alen);
      if (mypcminfo->sound_data != NULL)
        memcpy(mypcminfo->sound_data, new_si->abuf, new_si->alen);
      mypcminfo->si = new_si;
    }
#endif

    free(file_buffer);
    free(DesktopSND[con].sound_data);

    new_window(pcm0);
  }
}

byte *save_sound_mem(pcminfo *mypcminfo) {
  HeadDC dc_header;
  byte *byte_ptr = (byte *)mypcminfo->sound_data;
  byte *file_buffer;
  int length;

  file_pos = 0;
  length = mypcminfo->sound_size * 2;

  if ((file_buffer = (byte *)malloc(length + 44)) == NULL) {
    v_text = (char *)texts[45];
    show_dialog(err0);
    return (NULL);
  }

  file_buffer[file_pos] = 'R';
  file_pos++;
  file_buffer[file_pos] = 'I';
  file_pos++;
  file_buffer[file_pos] = 'F';
  file_pos++;
  file_buffer[file_pos] = 'F';
  file_pos++;

  length += 36;
  memcpy(file_buffer + file_pos, &length, 4);
  file_pos += 4;
  length -= 36;

  file_buffer[file_pos] = 'W';
  file_pos++;
  file_buffer[file_pos] = 'A';
  file_pos++;
  file_buffer[file_pos] = 'V';
  file_pos++;
  file_buffer[file_pos] = 'E';
  file_pos++;
  file_buffer[file_pos] = 'f';
  file_pos++;
  file_buffer[file_pos] = 'm';
  file_pos++;
  file_buffer[file_pos] = 't';
  file_pos++;
  file_buffer[file_pos] = ' ';
  file_pos++;

  dc_header.dwUnknow = 16;
  dc_header.wFormatTag = 1;
  dc_header.wChannels = 2;
  dc_header.dwSamplePerSec = mypcminfo->sound_freq;
  dc_header.dwAvgBytesPerSec = mypcminfo->sound_freq * (mypcminfo->sound_bits / 8);
  dc_header.wBlockAlign = 1;
  dc_header.wBits = 16;

  memcpy(file_buffer + file_pos, &dc_header, sizeof(HeadDC));
  file_pos += sizeof(HeadDC);

  file_buffer[file_pos] = 'd';
  file_pos++;
  file_buffer[file_pos] = 'a';
  file_pos++;
  file_buffer[file_pos] = 't';
  file_pos++;
  file_buffer[file_pos] = 'a';
  file_pos++;

  memcpy(file_buffer + file_pos, &length, 4);
  file_pos += 4;
  memcpy(file_buffer + file_pos, byte_ptr, length);
  file_pos += length;

  return (file_buffer);
}

int is_wav(char *filename) {
  FILE *f;
  int ok = 1;

  if ((f = fopen(filename, "rb")) == NULL)
    return (0);

  if (fgetc(f) != 'R')
    ok = 0;
  if (fgetc(f) != 'I')
    ok = 0;
  if (fgetc(f) != 'F')
    ok = 0;
  if (fgetc(f) != 'F')
    ok = 0;
  fseek(f, 4, SEEK_CUR);
  if (fgetc(f) != 'W')
    ok = 0;
  if (fgetc(f) != 'A')
    ok = 0;
  if (fgetc(f) != 'V')
    ok = 0;
  if (fgetc(f) != 'E')
    ok = 0;

  fclose(f);

  return (ok);
}

int new_sample(pcminfo *mypcminfo) {
  mypcminfo->sound_data = (short *)malloc(mypcminfo->sound_size * 2);
  if (mypcminfo->sound_data == NULL)
    return (0);
  mypcminfo->sample = NULL;
  mypcminfo->si = NULL;
  return (1);
}

void wline(char *ptr, int realan, int w, int h, int x0, int y0, int x1, int y1, char color) {
  int dx, dy, a, b, d, x, y;

  if (x0 > x1) {
    x = x1;
    dx = x0 - x1;
  } else {
    x = x0;
    dx = x1 - x0;
  }
  if (y0 > y1) {
    y = y1;
    dy = y0 - y1;
  } else {
    y = y0;
    dy = y1 - y0;
  }

  if (!dx && !dy)
    linea_pixel(ptr, w, realan, h, x0, y0, color);
  else {
    linea_pixel(ptr, w, realan, h, x0, y0, color);
    if (dy <= dx) {
      if (x0 > x1) {
        linea_pixel(ptr, w, realan, h, x1, y1, color);
        x0--;
        swap(x0, x1);
        swap(y0, y1);
      }
      d = 2 * dy - dx;
      a = 2 * dy;
      b = 2 * (dy - dx);
      x = x0;
      y = y0;
      if (y0 <= y1)
        while (x < x1) {
          if (d <= 0) {
            d += a;
            x++;
          } else {
            d += b;
            x++;
            y++;
          }
          linea_pixel(ptr, w, realan, h, x, y, color);
        }
      else
        while (x < x1) {
          if (d <= 0) {
            d += a;
            x++;
          } else {
            d += b;
            x++;
            y--;
          }
          linea_pixel(ptr, w, realan, h, x, y, color);
        }
    } else {
      if (y0 > y1) {
        linea_pixel(ptr, w, realan, h, x1, y1, color);
        y0--;
        swap(x0, x1);
        swap(y0, y1);
      }
      d = 2 * dx - dy;
      a = 2 * dx;
      b = 2 * (dx - dy);
      x = x0;
      y = y0;
      if (x0 <= x1)
        while (y < y1) {
          if (d <= 0) {
            d += a;
            y++;
          } else {
            d += b;
            y++;
            x++;
          }
          linea_pixel(ptr, w, realan, h, x, y, color);
        }
      else
        while (y < y1) {
          if (d <= 0) {
            d += a;
            y++;
          } else {
            d += b;
            y++;
            x--;
          }
          linea_pixel(ptr, w, realan, h, x, y, color);
        }
    }
  }
}

void linea_pixel(char *ptr, int w, int realan, int h, int x, int y, char color) {
  if (x >= 0 && y >= 0 && x < w && y < h) {
    ptr[x + y * realan] = color;
  }
}
