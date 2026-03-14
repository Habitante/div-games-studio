#include <stdio.h>
#include "recorder.h"
#include "global.h"

byte *frame_copy;
byte dac_copy[768];
FILE *grb_file;
char CHUNK;
int init_recorder = 0;

int check_differences(byte *new_screen) {
  int x, dif = 0;
  for (x = 0; x < 64000; x++)
    if ((frame_copy[x] ^= new_screen[x]))
      dif = 1;
  return (dif);
}

char *compress_off(char *buffer, int *length_off) {
  int x, rep, last_off, cnt = 0;
  byte *cbuffer = (byte *)malloc(*length_off * 2);

  for (x = 0; x < *length_off; x++)
    if (buffer[x]) {
      last_off = x;
      cbuffer[cnt++] = last_off % 256;
      cbuffer[cnt++] = last_off / 256;
      rep = 0;
      while (buffer[x] || buffer[x + 1] || buffer[x + 2]) {
        rep++;
        if (rep == 255)
          break;
        x++;
      }
      cbuffer[cnt++] = rep;
      memcpy(&cbuffer[cnt], buffer + last_off, rep);
      cnt += rep;
    }
  *length_off = cnt;
  return ((char *)cbuffer);
}

char *compress_rle(char *video_mem, int *length_off) {
  char act_pixel;
  char cntPixel = 0;
  int ptr = 0, cptr = 0;
  byte *cbuffer = (byte *)malloc(*length_off * 2);

  act_pixel = video_mem[ptr];
  while (ptr < *length_off) {
    while ((video_mem[ptr] == act_pixel) && (ptr < *length_off)) {
      cntPixel++;
      ptr++;
      if (cntPixel == 63)
        break;
    }
    if (cntPixel == 1) {
      if (act_pixel > 63)
        cbuffer[cptr++] = 193;
      cbuffer[cptr++] = act_pixel;
    } else {
      cbuffer[cptr++] = 192 + cntPixel;
      cbuffer[cptr++] = act_pixel;
    }
    act_pixel = video_mem[ptr];
    cntPixel = 0;
  }
  *length_off = cptr;
  return ((char *)cbuffer);
}

void compress_and_save_frame() {
  byte *buff_off;
  byte *buff_rle;
  int length_off = 64000, length_rle = 64000;

  buff_off = (byte *)compress_off((char *)frame_copy, &length_off);
  buff_rle = (byte *)compress_rle((char *)frame_copy, &length_rle);

  if (length_off < length_rle) {
    CHUNK = INIT_FRAME;
    fwrite(&CHUNK, 1, 1, grb_file);
    CHUNK = CMP_OFF;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&length_off, 1, 4, grb_file);
    _ffwrite(buff_off, length_off, grb_file);
  } else {
    CHUNK = INIT_FRAME;
    fwrite(&CHUNK, 1, 1, grb_file);
    CHUNK = CMP_RLE;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&length_rle, 1, 4, grb_file);
    _ffwrite(buff_rle, length_rle, grb_file);
  }
  free(buff_off);
  free(buff_rle);
}

void record_screen(unsigned char *new_screen) {
  if (!init_recorder) {
    init_recorder = 1;
    frame_copy = (byte *)malloc(64004);
    memset(frame_copy, 0, 64004);
    grb_file = fopen("GRABADOR.SSN", "wb");
    write_dac(dac);
    memcpy(frame_copy, new_screen, 64000);
    compress_and_save_frame();
  } else {
    if (check_differences(new_screen))
      compress_and_save_frame();
    memcpy(frame_copy, new_screen, 64000);
  }
}

void end_recorder() {
  CHUNK = END_ANIM;
  fwrite(&CHUNK, 1, 1, grb_file);
  fclose(grb_file);
  free(frame_copy);
}

void write_dac(byte *dac) {
  int x;
  if (init_recorder)
    for (x = 0; x < 768; x++)
      if (dac_copy[x] != dac[x]) {
        CHUNK = SET_PALET;
        fwrite(&CHUNK, 1, 1, grb_file);
        fwrite(dac, 768, 1, grb_file);
        memcpy(dac_copy, dac, 768);
        return;
      }
}

void write_mouse_key(char mouse_key) {
  if (init_recorder) {
    CHUNK = MOUSE_KEY;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&mouse_key, 1, 1, grb_file);
  }
}
