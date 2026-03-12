#include <stdio.h>
#include "recorder.h"
#include "global.h"

byte *frame_copy;
byte dac_copy[768];
FILE *grb_file;
char CHUNK;
int init_recorder = 0;

int check_differences(byte *NewScreen) {
  int x, Dif = 0;
  for (x = 0; x < 64000; x++)
    if ((frame_copy[x] ^= NewScreen[x]))
      Dif = 1;
  return (Dif);
}

char *compress_off(char *Buffer, int *LengtOff) {
  int x, rep, LastOff, cnt = 0;
  byte *cbuffer = (byte *)malloc(*LengtOff * 2);

  for (x = 0; x < *LengtOff; x++)
    if (Buffer[x]) {
      LastOff = x;
      cbuffer[cnt++] = LastOff % 256;
      cbuffer[cnt++] = LastOff / 256;
      rep = 0;
      while (Buffer[x] || Buffer[x + 1] || Buffer[x + 2]) {
        rep++;
        if (rep == 255)
          break;
        x++;
      }
      cbuffer[cnt++] = rep;
      memcpy(&cbuffer[cnt], Buffer + LastOff, rep);
      cnt += rep;
    }
  *LengtOff = cnt;
  return ((char *)cbuffer);
}

char *compress_rle(char *pVideoMem, int *LengtOff) {
  char ActPixel;
  char cntPixel = 0;
  int ptr = 0, cptr = 0;
  byte *cbuffer = (byte *)malloc(*LengtOff * 2);

  ActPixel = pVideoMem[ptr];
  while (ptr < *LengtOff) {
    while ((pVideoMem[ptr] == ActPixel) && (ptr < *LengtOff)) {
      cntPixel++;
      ptr++;
      if (cntPixel == 63)
        break;
    }
    if (cntPixel == 1) {
      if (ActPixel > 63)
        cbuffer[cptr++] = 193;
      cbuffer[cptr++] = ActPixel;
    } else {
      cbuffer[cptr++] = 192 + cntPixel;
      cbuffer[cptr++] = ActPixel;
    }
    ActPixel = pVideoMem[ptr];
    cntPixel = 0;
  }
  *LengtOff = cptr;
  return ((char *)cbuffer);
}

void compress_and_save_frame() {
  byte *BuffOff;
  byte *BuffRLE;
  int LengtOff = 64000, LengtRLE = 64000;

  BuffOff = (byte *)compress_off((char *)frame_copy, &LengtOff);
  BuffRLE = (byte *)compress_rle((char *)frame_copy, &LengtRLE);

  if (LengtOff < LengtRLE) {
    CHUNK = INIT_FRAME;
    fwrite(&CHUNK, 1, 1, grb_file);
    CHUNK = CMP_OFF;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&LengtOff, 1, 4, grb_file);
    _ffwrite(BuffOff, LengtOff, grb_file);
  } else {
    CHUNK = INIT_FRAME;
    fwrite(&CHUNK, 1, 1, grb_file);
    CHUNK = CMP_RLE;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&LengtRLE, 1, 4, grb_file);
    _ffwrite(BuffRLE, LengtRLE, grb_file);
  }
  free(BuffOff);
  free(BuffRLE);
}

void record_screen(unsigned char *NewScreen) {
  if (!init_recorder) {
    init_recorder = 1;
    frame_copy = (byte *)malloc(64004);
    memset(frame_copy, 0, 64004);
    grb_file = fopen("GRABADOR.SSN", "wb");
    write_dac(dac);
    memcpy(frame_copy, NewScreen, 64000);
    compress_and_save_frame();
  } else {
    if (check_differences(NewScreen))
      compress_and_save_frame();
    memcpy(frame_copy, NewScreen, 64000);
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

void write_mouse_key(char bMouseKey) {
  if (init_recorder) {
    CHUNK = MOUSE_KEY;
    fwrite(&CHUNK, 1, 1, grb_file);
    fwrite(&bMouseKey, 1, 1, grb_file);
  }
}
