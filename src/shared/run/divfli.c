#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "topflc.h"
#include "inter.h"
#include "../../div_string.h"

static void Error_Reporter(char *msg);
static void Palette_Update(TFUByte (*palette)[256][3]);

TFAnimation *animation = NULL;
char *CBuffer;
int CBuff_alt, TFOffset;
int CBuff_anc;
TFUByte *TFframe, (*TFpalette)[256][3];
TFAnimationInfo info;
char TFNombre[256];

int fli_palette_update = 0;

struct {
  FILE *file;
  Uint8 *pMembuf;
  Uint32 membufSize;
  Uint8 *pFrame;
  Uint8 *pChunk;
  Uint16 FrameCount; /* Frame Counter */
  /*
  */
  Uint32 HeaderSize;   /* Fli file size */
  Uint16 HeaderCheck;  /* Fli header check */
  Uint16 HeaderFrames; /* Number of frames in flic */
  Uint16 HeaderWidth;  /* Fli width */
  Uint16 HeaderHeight; /* Fli heigth */
  Uint16 HeaderDepth;  /* Color depth */
  Uint16 HeaderSpeed;  /* Number of video ticks between frame */
  Uint32 FrameSize;    /* Frame size in bytes */
  Uint16 FrameCheck;   /* Frame check */
  Uint16 FrameChunks;  /* Number of chunks in frame */
  Uint32 ChunkSize;    /* Size of chunk */
  Uint16 ChunkType;    /* Type of chunk */
  /*
  */
  OSDEP_Surface *mainscreen;
  OSDEP_Color colors[256];
  uint8_t *buffer;
  int offx;
  int offy;
  int screen_w;
  int screen_h;
  int screen_depth;
  int loop;
  byte ok;
} flc;

void fli_read_u16(uint16_t *tmp1, uint8_t *tmp2) {
  *tmp1 = ((Uint8) * (tmp2 + 1) << 8) + (Uint8) * (tmp2);
}

void fli_read_u32(uint32_t *tmp1, uint8_t *tmp2) {
  *tmp1 = (((((((Uint8) * (tmp2 + 3) << 8) + ((Uint8) * (tmp2 + 2))) << 8) + ((Uint8) * (tmp2 + 1)))
            << 8) +
           (Uint8) * (tmp2));
}

void fli_read_file(Uint32 size) {
  if (size > flc.membufSize) {
    if (!(flc.pMembuf = (byte *)realloc(flc.pMembuf, size + 1))) {
      printf("Realloc failed: %d\n", size);
      exit(1);
    }
  }

  if (fread(flc.pMembuf, sizeof(Uint8), size, flc.file) == 0) {
    printf("Can't read flx file");
    exit(1);
  }
} /* fli_read_file */

#ifdef DIV1
extern FILE *div_open_file(byte *file);
#else
extern FILE *div_open_file(char *file);
#endif

int fli_check_header(char *filename) {
#ifdef DIV1
  if ((flc.file = div_open_file((byte *)filename)) == NULL)
#else
  if ((flc.file = div_open_file((char *)filename)) == NULL)
#endif

    return (1);


  fli_read_file(128);

  fli_read_u32(&flc.HeaderSize, flc.pMembuf);
  fli_read_u16(&flc.HeaderCheck, flc.pMembuf + 4);
  fli_read_u16(&flc.HeaderFrames, flc.pMembuf + 6);
  fli_read_u16(&flc.HeaderWidth, flc.pMembuf + 8);
  fli_read_u16(&flc.HeaderHeight, flc.pMembuf + 10);
  fli_read_u16(&flc.HeaderDepth, flc.pMembuf + 12);
  fli_read_u16(&flc.HeaderSpeed, flc.pMembuf + 16);


  if ((flc.HeaderCheck == 0x0AF12) || (flc.HeaderCheck == 0x0AF11)) {
    flc.screen_w = flc.HeaderWidth;
    flc.screen_h = flc.HeaderHeight;
    flc.screen_depth = 8;
    if (flc.HeaderCheck == 0x0AF11) {
      flc.HeaderSpeed *= 1000 / 70;
    }
    return (0);
  }
  return (1);
} /* fli_check_header */

void fli_init_first_frame() {
  flc.FrameSize = 16;
  flc.FrameCount = 0;
  if (fseek(flc.file, 128, SEEK_SET)) {
    printf("Fseek read failed\n");
    exit(1);
  };
  fli_read_file(flc.FrameSize);
} /* fli_init_first_frame */

extern OSDEP_Surface *vga;

int fli_start(char *flinombre, char *Buffer, int Buff_anc, int Buff_alt, int cx, int cy) {
  char nombre[strlen(flinombre) + 10];
  flc.pMembuf = NULL;
  flc.membufSize = 0;
  flc.mainscreen = vga; //Buffer;
  flc.buffer = (byte *)Buffer;

  div_strcpy(nombre, sizeof(nombre), flinombre);

  if (fli_check_header(nombre)) {
    printf("Wrong header\n");
    return 0;
  }
  printf("Loaded %s\n", nombre);
  fli_init_first_frame();
  printf("Frames: %d\n", flc.HeaderFrames);
  return flc.HeaderFrames;
}

void fli_colors_256() {
  Uint8 *pSrc;
  Uint16 NumColorPackets;
  Uint16 NumColors;
  Uint8 NumColorsSkip;
  int i;

  pSrc = flc.pChunk + 6;
  fli_read_u16(&NumColorPackets, pSrc);
  pSrc += 2;
  while (NumColorPackets--) {
    NumColorsSkip = *(pSrc++);
    if (!(NumColors = *(pSrc++))) {
      NumColors = 256;
    }
    i = 0;
    while (NumColors--) {
      flc.colors[i].r = *(pSrc++);
      flc.colors[i].g = *(pSrc++);
      flc.colors[i].b = *(pSrc++);
      i++;
    }
    OSDEP_SetPalette(flc.mainscreen, flc.colors, NumColorsSkip, i);
  }
} /* fli_colors_256 */

void fli_ss2() {
  Uint8 *pSrc, *pDst, *pTmpDst;
  Sint8 CountData;
  Uint8 ColumSkip, Fill1, Fill2;
  Uint16 Lines, Count;

  pSrc = flc.pChunk + 6;
  pDst = flc.buffer; //mainscreen;//->pixels;
  fli_read_u16(&Lines, pSrc);
  pSrc += 2;
  while (Lines--) {
    fli_read_u16(&Count, pSrc);
    pSrc += 2;

    while (Count & 0xc000) {
      /* Upper bits 11 - Lines skip 
*/
      if ((Count & 0xc000) == 0xc000) {        // 0xc000h = 1100000000000000
        pDst += (0x10000 - Count) * vga_width; //flc.mainscreen->pitch;
      }

      if ((Count & 0xc000) == 0x4000) { // 0x4000h = 0100000000000000
/* Upper bits 01 - Last pixel
*/
#ifdef DEBUG
        printf("Last pixel not implemented");
#endif
      }
      fli_read_u16(&Count, pSrc);
      pSrc += 2;
    }

    if ((Count & 0xc000) == 0x0000) { // 0xc000h = 1100000000000000
      pTmpDst = pDst;
      while (Count--) {
        ColumSkip = *(pSrc++);
        pTmpDst += ColumSkip;
        CountData = *(pSrc++);
        if (CountData > 0) {
          while (CountData--) {
            *(pTmpDst++) = *(pSrc++);
            *(pTmpDst++) = *(pSrc++);
          }
        } else {
          if (CountData < 0) {
            CountData = (0x100 - CountData);
            Fill1 = *(pSrc++);
            Fill2 = *(pSrc++);
            while (CountData--) {
              *(pTmpDst++) = Fill1;
              *(pTmpDst++) = Fill2;
            }
          }
        }
      }
      pDst += vga_width; //=flc.mainscreen->pitch;
    }
  }
} /* fli_ss2 */

void fli_decode_color() {
  Uint8 *pSrc;
  Uint16 NumColors, NumColorPackets;
  Uint8 NumColorsSkip;
  int i;

  pSrc = flc.pChunk + 6;
  fli_read_u16(&NumColorPackets, pSrc);
  pSrc += 2;
  while (NumColorPackets--) {
    NumColorsSkip = *(pSrc++);
    if (!(NumColors = *(pSrc++))) {
      NumColors = 256;
    }
    i = 0;
    while (NumColors--) {
      flc.colors[i].r = *(pSrc++) << 2;
      flc.colors[i].g = *(pSrc++) << 2;
      flc.colors[i].b = *(pSrc++) << 2;
      i++;
    }
    OSDEP_SetPalette(flc.mainscreen, flc.colors, NumColorsSkip, i);
  }
} /* fli_decode_color  */


void fli_decode_copy() {
  Uint8 *pSrc, *pDst;
  int Lines = flc.screen_h;
  pSrc = flc.pChunk + 6;
  pDst = flc.buffer; //mainscreen;/->pixels;
  while (Lines-- > 0) {
    memcpy(pDst, pSrc, flc.screen_w);
    pSrc += flc.screen_w;
    pDst += vga_width; //flc.mainscreen->pitch;
  }
} /* fli_decode_copy */

void fli_black() {
  Uint8 *pDst;
  int Lines = flc.screen_h;
  pDst = flc.buffer; //mainscreen->pixels;
  while (Lines-- > 0) {
    memset(pDst, 0, flc.screen_w);
    pDst += vga_width; //flc.mainscreen->pitch;
  }
} /* BLACK */

void fli_decode_brun() {
  Uint8 *pSrc, *pDst, *pTmpDst, Fill;
  Sint8 CountData;
  int HeightCount, PacketsCount;

  HeightCount = flc.HeaderHeight;
  pSrc = flc.pChunk + 6;
  pDst = flc.buffer; //mainscreen->pixels;
  while (HeightCount--) {
    pTmpDst = pDst;
    PacketsCount = *(pSrc++);
    while (PacketsCount--) {
      CountData = *(pSrc++);
      if (CountData > 0) {
        Fill = *(pSrc++);
        while (CountData--) {
          *(pTmpDst++) = Fill;
        }
      } else {
        if (CountData < 0) {
          CountData = (0x100 - CountData);
          while (CountData--) {
            *(pTmpDst++) = *(pSrc++);
          }
        }
      }
    }
    pDst += vga_width; //flc.mainscreen->pitch;
  }
} /* fli_decode_brun */


void fli_decode_lc() {
  Uint8 *pSrc, *pDst, *pTmpDst;
  Sint8 CountData;
  Uint8 CountSkip;
  Uint8 Fill;
  Uint16 Lines, tmp;
  int PacketsCount;

  pSrc = flc.pChunk + 6;
  pDst = flc.buffer; //mainscreen->pixels;

  fli_read_u16(&tmp, pSrc);
  pSrc += 2;
  pDst += vga_width; //tmp*flc.mainscreen->pitch;
  fli_read_u16(&Lines, pSrc);
  pSrc += 2;
  while (Lines--) {
    pTmpDst = pDst;
    PacketsCount = *(pSrc++);
    while (PacketsCount--) {
      CountSkip = *(pSrc++);
      pTmpDst += CountSkip;
      CountData = *(pSrc++);
      if (CountData > 0) {
        while (CountData--) {
          *(pTmpDst++) = *(pSrc++);
        }
      } else {
        if (CountData < 0) {
          CountData = (0x100 - CountData);
          Fill = *(pSrc++);
          while (CountData--) {
            *(pTmpDst++) = Fill;
          }
        }
      }
    }
    pDst += vga_width; //flc.mainscreen->pitch;
  }
} /* fli_decode_lc */


int fli_check_frame() {
  flc.pFrame = flc.pMembuf + flc.FrameSize - 16;
  fli_read_u32(&flc.FrameSize, flc.pFrame + 0);
  fli_read_u16(&flc.FrameCheck, flc.pFrame + 4);
  fli_read_u16(&flc.FrameChunks, flc.pFrame + 6);


  flc.pFrame += 16;
  if (flc.FrameCheck == 0x0f1fa) {
    return (0);
  }

  if (flc.FrameCheck == 0x0f100) {
#ifdef DEBUG
    printf("Ani info!!!\n");
#endif
    return (0);
  }

  return (1);
} /* fli_check_frame */


extern SDL_Surface *vga;

void fli_do_one_frame() {
  int ChunkCount;
  ChunkCount = flc.FrameChunks;
  flc.pChunk = flc.pMembuf;
  while (ChunkCount--) {
    fli_read_u32(&flc.ChunkSize, flc.pChunk + 0);
    fli_read_u16(&flc.ChunkType, flc.pChunk + 4);


    switch (flc.ChunkType) {
    case 4:
      fli_colors_256();
      break;
    case 7:
      fli_ss2();
      break;
    case 11:
      fli_decode_color();
      break;
    case 12:
      fli_decode_lc();
      break;
    case 13:
      fli_black();
      break;
    case 15:
      fli_decode_brun();
      break;
    case 16:
      fli_decode_copy();
      break;
    case 18:
#ifdef DEBUG
      printf("Chunk 18 not yet done.\n");
#endif
      break;
    default:
      printf("Ieek an non implemented chunk type: %d\n", flc.ChunkType);
    }

    flc.pChunk += flc.ChunkSize;
  }
} /* fli_do_one_frame */
int fli_next_frame() {
  flc.FrameCount++;

  if (flc.FrameCount > flc.HeaderFrames)
    return 0;

  if (fli_check_frame()) {
    if (flc.FrameCount <= flc.HeaderFrames) {
      printf("Frame failure -- corrupt file?\n");
      return 0;
    } else {
      return 0;
    }
  }
  fli_read_file(flc.FrameSize);

  if (flc.FrameCheck != 0x0f100) {
    fli_do_one_frame();
    /* TODO: Track which rectangles have really changed */
    OSDEP_UpdateRect(flc.mainscreen, 0, 0, 0, 0);
  }

  return (flc.FrameCount);
}

void fli_end() {
  if (flc.file != NULL)
    fclose(flc.file);
  if (flc.pMembuf != NULL) {
    free(flc.pMembuf);
    flc.pMembuf = NULL;
  }

  flc.pMembuf = NULL;
  flc.file = NULL;
}

void fli_reset() {}

int quit_warning;

static void Error_Reporter(char *msg) {}

static void Palette_Update(TFUByte (*TFpalette)[256][3]) {}
