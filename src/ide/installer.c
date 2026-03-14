
//-----------------------------------------------------------------------------
//  Installation creator module
//-----------------------------------------------------------------------------

#include "global.h"
#include "div_string.h"
#ifdef ZLIB
#include <zlib.h>
#endif
#include <time.h>


//-----------------------------------------------------------------------------
//  Module prototypes and variables
//-----------------------------------------------------------------------------

void install_setup0();
void install_setupm0();
void install_setupe0();
void create_install_image(char *file, int errores);
int is_point(int *ptr, int n);
int compress_file(FILE *file_in, FILE *fout, unsigned long len);
int copy_file(FILE *file_in, FILE *fout, unsigned long len, int patch);
int file_copy_ice(char *org, char *dest, int vols, int _texto);

// for packfiles

int pack(char *runtime, char *exefile, char *datafile, char *outfile);

#include "shared/lib/zip/src/zzip.h"

char app_name[128];
char copyright_text[128];
char pack_name[128] = "";
char drive_letter[128];
char default_dir[128];
char msg_exe[128];
char help_text1[128];
char help_text2[128];
char disk_text1[128];
char disk_text2[128];

char install_err0[128];
char install_err1[128];
char install_err2[128];
char install_err3[128];
char install_err4[128];
char install_err5[128];
char install_err6[128];
char install_err7[128];
char install_err8[128];

char ifile1[256] = "install/INSTALL.FPG", ifile1name[16] = "INSTALL.FPG";
char ifile2[256] = "install/INST_SMA.FNT", ifile2name[16] = "INST_SMA.FNT";
char ifile3[256] = "install/INST_BIG.FNT", ifile3name[16] = "INST_BIG.FNT";

//-----------------------------------------------------------------------------

int include_setup = 0;
int create_dir = 0;
int empaquetar = 1;
int pentium = 1;
int segundo_font = 1;

//-----------------------------------------------------------------------------

typedef struct _HeaderSetup {
  char name[16];       // Filename (ASCIIZ, no path)
  unsigned int offset; // Offset from the start of the pack
  unsigned int len1;   // File length in the packfile (compressed)
  unsigned int len2;   // Actual file length (uncompressed)
} header_setup_t;

header_setup_t *header_setup;

int nfiles, is_disk = 0;

char exe_gen[_MAX_PATH]; // PROGRAM.EXE

char *__ins, *_ins, *ins; // Pointers into EXEC.INS

extern char user1[];
extern char user2[];

//-----------------------------------------------------------------------------

struct _dirhead {
  char pack[16];        // Packfile name (not stored in the file itself)
  char head[8];         // Packfile header signature (dat<-\n00)
  int crc1, crc2, crc3; // CRCs for up to three programs that use it
  int nfiles;           // Number of contained files
} dirhead;

struct tdir {
  char name[16];       // Filename (ASCIIZ, no path)
  unsigned int offset; // Offset from the start of the pack
  unsigned int len1;   // File length in the packfile (compressed)
  unsigned int len2;   // Actual file length (uncompressed)
};

struct tdir *hdir; // File directory

int memcrc[9]; // For computing program CRCs

byte *install_image = NULL;

//-----------------------------------------------------------------------------
//  Custom setup window
//-----------------------------------------------------------------------------

void install_setup1() {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  int x, y;

  if (segundo_font) {
    v.item[4].type = ITEM_BUTTON;
  } else {
    v.item[4].type = -1;
    if (v.selected_item == 4) {
      _select_new_item(v.selected_item + 1);
    }
  }

  _show_items();

  wwrite(ptr, w, h, 4, 32, 0, texts[521], c3);
  wwrite(ptr, w, h, 4, 32 + 19, 0, texts[522], c3);
  wwrite(ptr, w, h, 4, 32 + 48, 0, texts[523], c3);

  wbox(ptr, w, h, c12, 4, 40, 100, 8);
  wbox(ptr, w, h, c12, 4, 40 + 19, 100, 8);
  wbox(ptr, w, h, c12, 4, 40 + 48, 100, 8);

  wwrite(ptr, w, h, 5, 41, 0, (byte *)ifile1name, c3);
  wwrite(ptr, w, h, 5, 41 + 19, 0, (byte *)ifile2name, c3);
  wwrite(ptr, w, h, 5, 41 + 48, 0, (byte *)ifile3name, c3);

  if (!segundo_font) {
    for (y = (32 + 48) * big2; y < (32 + 48 + 19) * big2; y++) {
      for (x = 4 * big2; x < (120) * big2; x++) {
        ptr[y * v.w + x] = *(ghost + (int)c2 * 256 + ptr[y * v.w + x]);
      }
    }
  }

  wbox(ptr, w, h, c0, 2, 29, w - 4, 1);
  wbox(ptr, w, h, c0, 2, 60 + 19 * 3, w - 4, 1);
  wbox(ptr, w, h, c0, 2, h - 20, w - 4, 1);

  wrectangle(ptr, w, h, c0, 140, 29, 144, 89);

  if (install_image != NULL) {
    for (y = 0; y < 87 * big2; y++)
      for (x = 0; x < 142 * big2; x++) {
        ptr[141 * big2 + x + (30 * big2 + y) * v.w] = install_image[x + y * 142 * big2];
      }
  } else {
    wbox(ptr, w, h, c1, 141, 30, 142, 87);
  }
}

void install_setup2() {
  FILE *f;
  byte *ptrimg;
  char cwork[256];

  _process_items();

  switch (v.active_item) {
  case 0:
    v_accept = 1;
    end_dialog = 1;
    break;
  case 1:
    end_dialog = 1;
    break;
  case 2:
    div_strcpy(cwork, sizeof(cwork), file_types[FT_FPG].path);
    div_strcpy(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), file_types[1].path);
    if (file_types[FT_FPG].path[strlen(file_types[FT_FPG].path) - 1] != '/')
      div_strcat(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), "/");
    div_strcat(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), "INSTALL");
    v_mode = 1;
    v_type = FT_FPG;
    v_text = (char *)texts[524];
    show_dialog(browser0);

    div_strcpy(full, sizeof(full), file_types[FT_FPG].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    div_strcpy(file_types[FT_FPG].path, sizeof(file_types[FT_FPG].path), cwork);

    if (v_finished)
      if (v_exists) {
        if ((f = fopen(full, "rb")) != NULL) {
          fread(cwork, 1, 8, f);
          fclose(f);
          if (strcmp(cwork, "fpg\x1a\x0d\x0a")) {
            v_text = (char *)texts[46];
            show_dialog(err0);
          } else {
            ptrimg = install_image;
            create_install_image(full, 1);
            if (install_image != NULL) {
              free(ptrimg);
              DIV_STRCPY(ifile1, full);
              div_strcpy(ifile1name, sizeof(ifile1name), input);
            } else
              install_image = ptrimg;
            call(v.paint_handler);
            v.redraw = 1;
          }
        }
      }
    break;
  case 3:
    div_strcpy(cwork, sizeof(cwork), file_types[FT_FNT].path);
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), file_types[1].path);
    if (file_types[FT_FNT].path[strlen(file_types[FT_FNT].path) - 1] != '/')
      div_strcat(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), "/");
    div_strcat(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), "INSTALL");
    v_mode = 1;
    v_type = FT_FNT;
    v_text = (char *)texts[525];
    show_dialog(browser0);

    div_strcpy(full, sizeof(full), file_types[FT_FNT].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), cwork);

    if (v_finished)
      if (v_exists) {
        if ((f = fopen(full, "rb")) != NULL) {
          fread(cwork, 1, 8, f);
          fclose(f);
          if (strcmp(cwork, "fnt\x1a\x0d\x0a")) {
            v_text = (char *)texts[46];
            show_dialog(err0);
          } else {
            DIV_STRCPY(ifile2, full);
            div_strcpy(ifile2name, sizeof(ifile2name), input);
            call(v.paint_handler);
            v.redraw = 1;
          }
        }
      }
    break;
  case 4:
    div_strcpy(cwork, sizeof(cwork), file_types[FT_FNT].path);
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), file_types[1].path);
    if (file_types[FT_FNT].path[strlen(file_types[FT_FNT].path) - 1] != '/')
      div_strcat(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), "/");
    div_strcat(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), "INSTALL");
    v_mode = 1;
    v_type = FT_FNT;
    v_text = (char *)texts[525];
    show_dialog(browser0);

    div_strcpy(full, sizeof(full), file_types[v_type].path);
    if (full[strlen(full) - 1] != '/')
      div_strcat(full, sizeof(full), "/");
    div_strcat(full, sizeof(full), input);
    div_strcpy(file_types[FT_FNT].path, sizeof(file_types[FT_FNT].path), cwork);

    if (v_finished)
      if (v_exists) {
        if ((f = fopen(full, "rb")) != NULL) {
          fread(cwork, 1, 8, f);
          fclose(f);
          if (strcmp(cwork, "fnt\x1a\x0d\x0a")) {
            v_text = (char *)texts[46];
            show_dialog(err0);
          } else {
            DIV_STRCPY(ifile3, full);
            div_strcpy(ifile3name, sizeof(ifile3name), input);
            call(v.paint_handler);
            v.redraw = 1;
          }
        }
      }
    break;
  case 5:
    show_dialog(install_setupm0);
    break;
  case 6:
    show_dialog(install_setupe0);
    break;
  case 7:
    call(v.paint_handler);
    break;
  }
}

void install_setup3() {
  FILE *f;

  if ((f = fopen("install/INS_TEXT.ASC", "wb")) != NULL) {
    fwrite(msg_exe, 1, 128, f);
    fwrite(help_text1, 1, 128, f);
    fwrite(help_text2, 1, 128, f);
    fwrite(disk_text1, 1, 128, f);
    fwrite(disk_text2, 1, 128, f);
    fwrite(install_err0, 1, 128, f);
    fwrite(install_err1, 1, 128, f);
    fwrite(install_err2, 1, 128, f);
    fwrite(install_err3, 1, 128, f);
    fwrite(install_err4, 1, 128, f);
    fwrite(install_err5, 1, 128, f);
    fwrite(install_err6, 1, 128, f);
    fwrite(install_err7, 1, 128, f);
    fwrite(install_err8, 1, 128, f);
    fclose(f);
  }
}

void install_setup0() {
  char *chr;
  time_t t;
  char tbuf[26];
  FILE *f;
  int y;

  v_accept = 0;
  v.type = WIN_DIALOG;
  v.w = 310;
  v.h = 125 + 16 + 19 + 20;
  v.title = texts[236];

  v.paint_handler = install_setup1;
  v.click_handler = install_setup2;
  v.close_handler = install_setup3;

  div_strcpy(exe_gen, sizeof(exe_gen), (char *)window[v_window + 1].title);
  if ((chr = strchr(exe_gen, '.')) != NULL)
    *chr = 0;
  strupr(exe_gen);

  if (strcmp(pack_name, exe_gen)) {
    div_strcpy(pack_name, sizeof(pack_name), exe_gen);
    div_strcpy(copyright_text, sizeof(copyright_text), (char *)texts[352]);
    {
      time_t t = time(NULL);
      struct tm *tm = localtime(&t);
      snprintf(tbuf, sizeof(tbuf), "%04d ", tm->tm_year + 1900);
    }
    div_strcat(copyright_text, sizeof(copyright_text), tbuf);
    if (strlen(user2) + strlen(copyright_text) <= 127)
      div_strcat(copyright_text, sizeof(copyright_text), user2);
    div_strcpy(drive_letter, sizeof(drive_letter), (char *)&file_types[8]); //"/tmp/");// :/TMP");
    //drive_letter[0]=toupper(file_types[1].path[0]);
    div_strcpy(default_dir, sizeof(default_dir), (char *)texts[353]);
    div_strcat(default_dir, sizeof(default_dir), exe_gen);
    div_strcpy(app_name, sizeof(app_name), exe_gen);
    div_strcpy(dirhead.pack, sizeof(dirhead.pack), exe_gen);
    div_strcat(dirhead.pack, sizeof(dirhead.pack), ".PAK");
  }

#ifdef WIN32
  div_strcat(exe_gen, sizeof(exe_gen), ".EXE");
#endif

  if ((f = fopen("install/INS_TEXT.ASC", "rb")) != NULL) {
    fread(msg_exe, 1, 128, f);
    fread(help_text1, 1, 128, f);
    fread(help_text2, 1, 128, f);
    fread(disk_text1, 1, 128, f);
    fread(disk_text2, 1, 128, f);
    fread(install_err0, 1, 128, f);
    fread(install_err1, 1, 128, f);
    fread(install_err2, 1, 128, f);
    fread(install_err3, 1, 128, f);
    fread(install_err4, 1, 128, f);
    fread(install_err5, 1, 128, f);
    fread(install_err6, 1, 128, f);
    fread(install_err7, 1, 128, f);
    fread(install_err8, 1, 128, f);
    fclose(f);
  } else {
    div_strcpy(msg_exe, sizeof(msg_exe), (char *)texts[354]);
    div_strcpy(help_text1, sizeof(help_text1), (char *)texts[351]);
    div_strcpy(help_text2, sizeof(help_text2), (char *)texts[355]);
    div_strcpy(disk_text1, sizeof(disk_text1), (char *)texts[515]);
    div_strcpy(disk_text2, sizeof(disk_text2), (char *)texts[516]);
    div_strcpy(install_err0, sizeof(install_err0), (char *)texts[528]);
    div_strcpy(install_err1, sizeof(install_err1), (char *)texts[529]);
    div_strcpy(install_err2, sizeof(install_err2), (char *)texts[530]);
    div_strcpy(install_err3, sizeof(install_err3), (char *)texts[531]);
    div_strcpy(install_err4, sizeof(install_err4), (char *)texts[532]);
    div_strcpy(install_err5, sizeof(install_err5), (char *)texts[533]);
    div_strcpy(install_err6, sizeof(install_err6), (char *)texts[534]);
    div_strcpy(install_err7, sizeof(install_err7), (char *)texts[535]);
    div_strcpy(install_err8, sizeof(install_err8), (char *)texts[536]);
  }

  y = 64 + 19 * 3;

  _button(100, 7, v.h - 14, 0);
  _button(101, v.w - 8, v.h - 14, 2);

  _button(121, 109, 38, 0);
  _button(121, 109, 38 + 19, 0);
  _button(121, 109, 38 + 48, 0);

  _button(527, 7, 48 + 19 * 3, 0);
  _button(519, 113, 48 + 19 * 3, 2);

  _flag(539, 4, 32 + 39, &segundo_font);

  _flag(517, 4, 12, &empaquetar);
  _flag(518, v.w / 2, 12, &pentium);
  _flag(239, 4, 20, &include_setup);
  _flag(240, v.w / 2, 20, &create_dir);

  _get(224, 4, y + 19 * 0, 6 * 16, (byte *)app_name, 127, 0, 0);
  _get(226, 6 * 16 + 8, y + 19 * 0, (v.w - 4) - (6 * 16 + 8), (byte *)copyright_text, 127, 0, 0);

  _get(227, 4, y + 19 * 1, 6 * 16, (byte *)drive_letter, 127, 0, 0);
  _get(228, 6 * 16 + 8, y + 19 * 1, (v.w - 4) - (6 * 16 + 8), (byte *)default_dir, 127, 0, 0);
}

//-----------------------------------------------------------------------------
// Create the installer thumbnail of 142x87 (*big2)
//-----------------------------------------------------------------------------

void create_install_image(char *file, int errores) {
  FILE *es;
  byte *p, *fpg;
  int *ptr;
  int file_len;
  int lst[1000];
  byte pal[768];
  byte xlat[256];
  char cwork[256];

  float coefredy, coefredx, a, b;
  byte *temp2;
  int x, y, n, m, w, h;
  int fl = 0;

  install_image = NULL;

  if ((es = fopen(file, "rb")) == NULL)
    return;
  else {
    fseek(es, 0, SEEK_END);
    file_len = ftell(es);
    if ((p = (byte *)malloc(file_len)) != NULL) {
      fpg = p;
      fseek(es, 0, SEEK_SET);
      fread(fpg, 1, file_len, es);
      fclose(es);

      if (strcmp((char *)fpg, "fpg\x1a\x0d\x0a")) {
        free(fpg);
        return;
      }

      memcpy(pal, fpg + 8, 768);

      p += 1352; // FPG Header len

      memset(lst, 0, 1000 * sizeof(int));

      while (p < fpg + file_len && *(int32_t *)p < 1000 && *(int32_t *)p > 0) {
        lst[*(int32_t *)p] = (intptr_t)p;
        p += *(memptrsize *)(p + 4); //sizeof(int*));
      }

      cwork[0] = '\0';

      if (!lst[1])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 1);
      if ((ptr = (int32_t *)lst[2])) {
        if (ptr[13] != 640 || ptr[14] != 480)
          div_snprintf(cwork, sizeof(cwork), (char *)texts[541], 2);
      }
      if (!(ptr = (int32_t *)lst[3]))
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 3);
      if (ptr[13] != 640 || ptr[14] != 480)
        div_snprintf(cwork, sizeof(cwork), (char *)texts[541], 3);
      if (!is_point(ptr, 1))
        div_snprintf(cwork, sizeof(cwork), (char *)texts[542], 1, 3);
      if (!lst[4] && !lst[5])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 4);
      if (!lst[13])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 13);
      if (!lst[14])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 14);
      if (!lst[15])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 15);
      if (!lst[16])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 16);
      if (!lst[17])
        div_snprintf(cwork, sizeof(cwork), (char *)texts[540], 17);

      if (strlen(cwork)) {
        if (errores) {
          v_text = cwork;
          show_dialog(err0);
        }
        return;
      }
      //printf("next bit commented\n");
      /*
      if (ptr=(int*)lst[3]) {
        if (ptr[13]==640 && ptr[14]==480) {
          p=(byte*)ptr+64+ptr[15]*4; // Start of the screen

          // Create the thumbnail ...

          create_dac4();
          for (n=0;n<256;n++) xlat[n]=fast_find_color(pal[n*3],pal[n*3+1],pal[n*3+2]);

          an=142*2*big2; al=87*2*big2;

          coefredx=640/((float)an);
          coefredy=480/((float)al);

          if ((temp2=(char *)malloc(an*al))!=NULL) {

            memset(temp2,0,an*al);
            a=(float)0.0;
            for(y=0;y<al;y++) {
              b=(float)0.0;
              for(x=0;x<an;x++) {
                temp2[y*an+x]=p[((int)a)*640+(int)b];
                b+=coefredx;
              } a+=coefredy;
            }

            // Apply the xlat[] table to the thumbnail

            for (n=an*al-1;n>=0;n--) {
              temp2[n]=xlat[temp2[n]];
            }

            if ((install_image=(char *)malloc(an*al/4))!=NULL) {

              for (y=0;y<al;y+=2) {
                for (x=0;x<an;x+=2) {
                  n=*(ghost+temp2[x+y*an]*256+temp2[x+1+y*an]);
                  m=*(ghost+temp2[x+(y+1)*an]*256+temp2[x+1+(y+1)*an]);
                  install_image[x/2+(y/2)*(an/2)]=*(ghost+n*256+m);
                }
              }

            } free(temp2);
          }
        }
      }
*/
      free(fpg);

    } else
      fclose(es);
  }
}

int px(int *ptr, int n) {
  return (*((word *)ptr + 32 + n * 2));
}

int is_point(int *ptr, int n) {
  if (n < ptr[15]) {
    if (px(ptr, n) >= 0 && px(ptr, n) < ptr[13])
      return (1);
    else
      return (0);
  } else
    return (0);
}

//-----------------------------------------------------------------------------
//  Installer information messages
//-----------------------------------------------------------------------------

void install_setupm1() {
  byte *ptr = v.ptr;
  int w = v.w / big2, h = v.h / big2;
  _show_items();
  wwrite(ptr, w, h, v.w / 2 - 16, 12 + 11 + 19 + 9, 0, (byte *)"#", c3);
}

void install_setupm2() {
  _process_items();
  switch (v.active_item) {
  case 0:
    end_dialog = 1;
    break;
  case 6:
    div_strcpy(msg_exe, sizeof(msg_exe), (char *)texts[354]);
    div_strcpy(help_text1, sizeof(help_text1), (char *)texts[351]);
    div_strcpy(help_text2, sizeof(help_text2), (char *)texts[355]);
    div_strcpy(disk_text1, sizeof(disk_text1), (char *)texts[515]);
    div_strcpy(disk_text2, sizeof(disk_text2), (char *)texts[516]);
    call(v.paint_handler);
    v.redraw = 1;
    break;
  }
}

void install_setupm0() {
  int y;

  v.type = WIN_DIALOG;
  v.w = 310;
  v.h = 12 + 9 + 19 * 4;
  v.title = texts[526];
  v.paint_handler = install_setupm1;
  v.click_handler = install_setupm2;

  y = 12;
  _button(100, v.w - 8, v.h - 14, 2);
  _get(225, 4, y + 19 * 0, v.w - 8, (byte *)help_text1, 127, 0, 0);
  y += 11;
  _get(414, 4, y + 19 * 0, v.w - 8, (byte *)help_text2, 127, 0, 0);
  _get(520, 4, y + 19 * 1, (v.w - 16) / 2 - 16, (byte *)disk_text1, 127, 0, 0);
  _get(414, v.w / 2 - 8, y + 19 * 1, v.w / 2 + 4, (byte *)disk_text2, 127, 0, 0);
  _get(229, 4, y + 19 * 2, v.w - 8, (byte *)msg_exe, 127, 0, 0);
  _button(538, 7, v.h - 14, 0);
}

//-----------------------------------------------------------------------------
//  Installer error messages
//-----------------------------------------------------------------------------

void install_setupe1() {
  _show_items();
}

void install_setupe2() {
  _process_items();
  switch (v.active_item) {
  case 0:
    end_dialog = 1;
    break;
  case 10:
    div_strcpy(install_err0, sizeof(install_err0), (char *)texts[528]);
    div_strcpy(install_err1, sizeof(install_err1), (char *)texts[529]);
    div_strcpy(install_err2, sizeof(install_err2), (char *)texts[530]);
    div_strcpy(install_err3, sizeof(install_err3), (char *)texts[531]);
    div_strcpy(install_err4, sizeof(install_err4), (char *)texts[532]);
    div_strcpy(install_err5, sizeof(install_err5), (char *)texts[533]);
    div_strcpy(install_err6, sizeof(install_err6), (char *)texts[534]);
    div_strcpy(install_err7, sizeof(install_err7), (char *)texts[535]);
    div_strcpy(install_err8, sizeof(install_err8), (char *)texts[536]);
    call(v.paint_handler);
    v.redraw = 1;
    break;
  }
}

void install_setupe0() {
  int y;

  v.type = WIN_DIALOG;
  v.w = 310;
  v.h = 12 + 9 + 99 + 8;
  v.title = texts[537];
  v.paint_handler = install_setupe1;
  v.click_handler = install_setupe2;

  y = 4;
  _button(100, v.w - 8, v.h - 14, 2);
  _get(414, 4, y + 11 * 0, v.w - 8, (byte *)install_err0, 127, 0, 0);
  _get(414, 4, y + 11 * 1, v.w - 8, (byte *)install_err1, 127, 0, 0);
  _get(414, 4, y + 11 * 2, v.w - 8, (byte *)install_err2, 127, 0, 0);
  _get(414, 4, y + 11 * 3, v.w - 8, (byte *)install_err3, 127, 0, 0);
  _get(414, 4, y + 11 * 4, v.w - 8, (byte *)install_err4, 127, 0, 0);
  _get(414, 4, y + 11 * 5, v.w - 8, (byte *)install_err5, 127, 0, 0);
  _get(414, 4, y + 11 * 6, v.w - 8, (byte *)install_err6, 127, 0, 0);
  _get(414, 4, y + 11 * 7, v.w - 8, (byte *)install_err7, 127, 0, 0);
  _get(414, 4, y + 11 * 8, v.w - 8, (byte *)install_err8, 127, 0, 0);
  _button(538, 7, v.h - 14, 0);
}


//-----------------------------------------------------------------------------
//  Function called to create an installation from the programs menu
//-----------------------------------------------------------------------------

int get_file_len(FILE *file) {
  int d;

  fseek(file, 0, SEEK_END);
  d = ftell(file);
  fseek(file, 0, SEEK_SET);
  return (d);
}

void create_zip(char *dWork) {
  char out[1024];
  char zipout[1024];

  char readme[] = "Made with DIV GAMES STUDIO\n\nhttp://div-arena.co.uk/\n\n";

  div_strcpy(out, sizeof(out), dWork);
  div_strcat(out, sizeof(out), "/");
  div_strcat(out, sizeof(out), exe_gen);

  struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 0);

  if (dirhead.nfiles > 0) {
    zip_entry_open(zip, "packfile.pak");
    zip_entry_fwrite(zip, "install/PACKFILE.DAT");
    zip_entry_close(zip);
  }

  zip_close(zip);

  fprintf(stdout, "Writing to: %s\n", out);

  pack("system/" RUNTIME, "system/EXEC.EXE", "foo.zip", out);

  delete_file("foo.zip");

#ifdef WIN32

  div_strcpy(zipout, sizeof(zipout), out);
  zipout[strlen(zipout) - 3] = 0;
  div_strcat(zipout, sizeof(zipout), "zip");
  zip = zip_open(zipout, ZIP_DEFAULT_COMPRESSION_LEVEL, 0);

  zip_entry_open(zip, exe_gen);
  zip_entry_fwrite(zip, out);
  zip_entry_close(zip);

  zip_entry_open(zip, "SDL.dll");
  zip_entry_fwrite(zip, "system/SDL.dll");
  zip_entry_close(zip);

  zip_entry_open(zip, "SDL_mixer.dll");
  zip_entry_fwrite(zip, "system/SDL_mixer.dll");
  zip_entry_close(zip);

  zip_entry_open(zip, "libmikmod-2.dll");
  zip_entry_fwrite(zip, "system/libmikmod-2.dll");
  zip_entry_close(zip);

  zip_entry_open(zip, "readme.txt");
  zip_entry_write(zip, readme, strlen(readme));
  zip_entry_close(zip);

  zip_close(zip);

#endif
}


void create_installation(void) {
  FILE *file_in, *fout;
  int x, n, m, topack;
  char cWork[256];
  char dWork[256];
  unsigned _drive = 0, my_drive;
  byte *chr;
  int TotLen = 0, PackSize;
  int size[6];

  char full[_MAX_PATH + 1];
  char drive[_MAX_DRIVE + 1];
  char dir[_MAX_DIR + 1];
  char fname[_MAX_FNAME + 1];
  char ext[_MAX_EXT + 1];

  if (install_image == NULL) {
    //    create_install_image(ifile1,0);
  }

  show_dialog(install_setup0);
  if (!v_accept)
    return;

  // *** Handle destination disk drive (accepts "a:","dir","\dir\new","d:\tmp",...)

  if (strlen(drive_letter) == 1) { // Assume a letter 'a'-'z' is a drive, not a directory
    strupr(drive_letter);
    if (drive_letter[0] >= 'A' && drive_letter[0] <= 'Z')
      div_strcat(drive_letter, sizeof(drive_letter), ":");
  }

  _fullpath(full, drive_letter, _MAX_PATH);
  _splitpath(full, drive, dir, fname, ext);
  if (strlen(dir) == 0 || dir[strlen(dir) - 1] != '/')
    div_strcat(dir, sizeof(dir), "/");
  if (strlen(fname) || strlen(ext)) {
    div_strcat(dir, sizeof(dir), fname);
    div_strcat(dir, sizeof(dir), ext);
    div_strcat(dir, sizeof(dir), "/");
  }

  strupr(drive);

  if (_drive <= 2) {
    div_strcpy(dir, sizeof(dir), "/");
    is_disk = _drive;
  } // On a floppy, don't create directories

  for (x = 1; x < strlen(dir); x++)
    if (IS_PATH_SEP(dir[x])) { // Create directories ...
      div_strcpy(cWork, sizeof(cWork), full);
    }

  // *** Default directory

  if (strlen(default_dir) == 0) {
    div_strcpy(default_dir, sizeof(default_dir), (char *)window[v_window].title);
    if ((chr = (byte *)strchr(default_dir, '.')) != NULL)
      *chr = 0;
  }

  strupr(default_dir);

  // *** Installation files (exec and setup files)

  file_in = fopen("system/exec.ins", "rb");
  fseek(file_in, 0, SEEK_END);
  n = ftell(file_in);
  fclose(file_in);

  file_in = fopen("install/setup.ins", "rb");
  if (file_in) {
    fseek(file_in, 0, SEEK_END);
    x = ftell(file_in);
  }

  if (!include_setup)
    x = 0;
  if ((__ins = _ins = ins = (char *)malloc(n + x + 32)) == NULL) {
    v_text = (char *)texts[357];
    show_dialog(err0);
    fclose(file_in);
    return;
  }

  if (file_in) {
    fseek(file_in, 0, SEEK_SET);
    x = fread(_ins, 1, x, file_in);
    fclose(file_in);
  }
  file_in = fopen("system/exec.ins", "rb");
  n = fread(_ins + x, 1, n, file_in) + x;
  fclose(file_in);

  nfiles = 2;
  if (include_setup)
    nfiles++;
  while (ins < _ins + n) {
    nfiles++;
    ins += strlen(ins) + 1;
  }
  ins = _ins;

  // *** Eliminate duplicate files

  if ((header_setup = (header_setup_t *)malloc(nfiles * sizeof(header_setup_t))) == NULL) {
    v_text = (char *)texts[357];
    show_dialog(err0);
    free(_ins);
    return;
  }

  dirhead.nfiles = 0; // Files that go into the PACKFILE

  for (x = 0; x < nfiles; x++) {
    if (x == 0)
      div_strcpy(header_setup[x].name, sizeof(header_setup[x].name), exe_gen);
    else if (x == 1)
      div_strcpy(header_setup[x].name, sizeof(header_setup[x].name), "DIV32RUN.DLL");
    else if (x == 2 && include_setup)
      div_strcpy(header_setup[x].name, sizeof(header_setup[x].name), "SETUP.EXE");
    else {
      chr = (byte *)ins;
      if (*ins == '+') { // When the file can not be included in the PACKFILE
        ins++;
        topack = 0;
      } else
        topack = 1;
      _splitpath(ins, drive, dir, fname, ext);
      div_strcpy(header_setup[x].name, sizeof(header_setup[x].name), fname);
      div_strcat(header_setup[x].name, sizeof(header_setup[x].name), ext);
      for (n = 0; n < x; n++) {
        if (!strcmp(header_setup[n].name, header_setup[x].name))
          break;
      }
      if (n < x || !strcmp(header_setup[n].name, "SOUND.CFG")) {
        // If the file had '+', add '+' to its earlier occurrence

        ins += strlen(ins) + 1;

        if (n < x && topack == 0) {
          chr = (byte *)_ins;

          if (include_setup)
            n -= 3;
          else
            n -= 2;

          while (n) {
            chr += strlen((char *)chr) + 1;
            n--;
          }
          if (*chr != '+') {
            memmove(chr + 1, chr, __ins - (char *)chr);
            __ins++;
            *chr = '+';
            dirhead.nfiles--;
          }
        }

        x--;
        nfiles--;

        continue;
      } else {
        ins += strlen(ins) + 1;
        strcpy((char *)__ins, (char *)chr); // TODO(Sprint E): buffer size unknown
        __ins += strlen(__ins) + 1;
        if (topack)
          dirhead.nfiles++;
      }
    }
  }

  free(header_setup);
  ins = _ins;

  // ALWAYS pack files

#if 0
  if (!empaquetar) dirhead.nfiles=0; // When the PACKFILE should not be generated
#endif


  //-----------------------------------------------------------------------------

  // NOTE: Protected installation mode was planned but never implemented
  // (not needed for modern distribution).
  // nfiles = total files (EXE + OVL [+ SETUP] + char * ins)
  // Directory entries stored in hdir[].name/offset/len1/len2
  // Header stored in dirhead.packname/head/crc1/crc2/crc3/nfiles

  if (dirhead.nfiles) {
    // 0. Define the basic packfile header values

    div_strcpy(cWork, sizeof(cWork), (char *)texts[498]);
    div_strcat(cWork, sizeof(cWork), dirhead.pack);

    memcpy(dirhead.head, "dat\x1a\x0d\x0a\x00\x00", 8);

    // dirhead.nfiles was already pre-calculated above

    // 1. Get CRCs of system\exec.exe and install\setup.ovl (if include_setup)

    dirhead.crc1 = 0;
    dirhead.crc2 = 0;
    dirhead.crc3 = 0;

    if ((file_in = fopen("system/EXEC.EXE", "rb")) != NULL) {
      fseek(file_in, 602, SEEK_SET);
      fread(memcrc, 4, 9, file_in);
      memcrc[0] = (memcrc[0] & 1);
      for (n = 0; n < 9; n++) {
        dirhead.crc1 <<= 1;
        dirhead.crc1 ^= memcrc[n];
      }
      fclose(file_in);
    }

    if ((file_in = fopen("install/setup.ovl", "rb")) != NULL) {
      fseek(file_in, 602, SEEK_SET);
      fread(memcrc, 4, 9, file_in);
      memcrc[0] = (memcrc[0] & 1);
      for (n = 0; n < 9; n++) {
        dirhead.crc2 <<= 1;
        dirhead.crc2 ^= memcrc[n];
      }
      fclose(file_in);
    }

    // 2. Open the file ("INSTALL/PACKFILE.DAT","wb")

    if ((fout = fopen("install/PACKFILE.DAT", "wb")) == NULL) {
      v_text = (char *)texts[358];
      show_dialog(err0);
      free(_ins);
      return;
    }

    // 3. Write the header (&dirhead.head)

    fwrite(&dirhead.head, 1, 8 + 3 * 4 + 4, fout);

    // 4. Allocate memory for the directory (hdir[])

    if ((hdir = (struct tdir *)malloc(dirhead.nfiles * sizeof(struct tdir))) == NULL) {
      v_text = (char *)texts[357];
      show_dialog(err0);
      fclose(fout);
      free(_ins);
      return;
    }

    // 5. Write the directory (garbage initially, updated later)

    fwrite(hdir, sizeof(struct tdir), dirhead.nfiles, fout);

    // 6. Write all files sequentially (filling in hdir[])

    ins = __ins = _ins;

    for (n = 0; n < dirhead.nfiles; n++) {
      show_progress(cWork, n, dirhead.nfiles);

      while (*ins == '+') {
        chr = (byte *)ins;
        ins += strlen(ins) + 1;
        strcpy(__ins, (char *)chr);
        __ins += strlen(__ins) + 1; // TODO(Sprint E): buffer size unknown
      }

      fprintf(stdout, "PACKING FILE: %s\n", ins);
      if ((file_in = fopen(ins, "rb")) == NULL) {
        v_text = (char *)texts[231];
        show_dialog(err0);
        free(hdir);
        fclose(fout);
        free(_ins);
        return;
      }

      _splitpath(ins, drive, dir, fname, ext);
      div_strcpy(hdir[n].name, sizeof(hdir[n].name), fname);
      div_strcat(hdir[n].name, sizeof(hdir[n].name), ext);
      strupr(hdir[n].name);
      hdir[n].offset = ftell(fout);

      fseek(file_in, 0, SEEK_END);
      hdir[n].len2 = ftell(file_in);
      fseek(file_in, 0, SEEK_SET);

      // Read file_in, compress it, and write it to fout (-1 on error)

      hdir[n].len1 = compress_file(file_in, fout, (unsigned long)hdir[n].len2);

      fclose(file_in);

      if (hdir[n].len1 == -1) {
        show_progress(cWork, dirhead.nfiles, dirhead.nfiles);
        v_text = (char *)texts[357];
        show_dialog(err0);
        free(hdir);
        fclose(fout);
        free(_ins);
        return;
      }

      ins += strlen(ins) + 1;
    }

    // 7. Update the file list (_ins) with the PACKFILE

    nfiles = nfiles - dirhead.nfiles + 1;

    while (*ins == '+') {
      chr = (byte *)ins;
      ins += strlen(ins) + 1;
      strcpy(__ins, (char *)chr);
      __ins += strlen(__ins) + 1; // TODO(Sprint E): buffer size unknown
    }

    strcpy(__ins, "install/PACKFILE.DAT"); // TODO(Sprint E): buffer size unknown

    // 8. Rewrite hdir[] and close the file

    fseek(fout, 8 + 3 * 4 + 4, SEEK_SET);
    fwrite(hdir, sizeof(struct tdir), dirhead.nfiles, fout);
    fseek(fout, 0, SEEK_END);
    fclose(fout);

    free(hdir);

    show_progress(cWork, dirhead.nfiles, dirhead.nfiles);
  }

  create_zip(full);


  // The rest is old code (disabled)

#if 0
//-----------------------------------------------------------------------------

   // Create INSTALL\DIV32RUN.DLL from (INSTALL\DIV32RUN.INS/386 + SYSTEM\LENGUAJE.INT)

  if ((fout=fopen("install/DIV32RUN.DLL","wb"))==NULL) {
    v_text=(char *)texts[358]; show_dialog(err0);
    free(_ins); return;
  }

  if (pentium) file_in=fopen("install/div32run.ins","rb");
  else         file_in=fopen("install/div32run.386","rb");

  if (file_in==NULL) {
    v_text=(char *)texts[231]; show_dialog(err0);
    fclose(fout); free(_ins); return;
  }

  fseek(file_in,0,SEEK_END);
  x=ftell(file_in);
  fseek(file_in,0,SEEK_SET);

  if (copy_file(file_in,fout,(unsigned long)x,0)==-1) {
    v_text=(char *)texts[358]; show_dialog(err0);
    fclose(fout); fclose(file_in); free(_ins); return;
  }

  fclose(file_in);

  if ((file_in=fopen("system/lenguaje.int","rb"))==NULL) {
    v_text=(char *)texts[231]; show_dialog(err0);
    fclose(fout); free(_ins); return;
  }

  fseek(file_in,0,SEEK_END);
  x=ftell(file_in);
  fseek(file_in,0,SEEK_SET);

  if (copy_file(file_in,fout,(unsigned long)x,0)==-1) {
    v_text=(char *)texts[358]; show_dialog(err0);
    fclose(fout); fclose(file_in); free(_ins); return;
  }

  fclose(file_in);

  if (fwrite(&x,4,1,fout)!=1) {
    v_text=(char *)texts[358]; show_dialog(err0);
    fclose(fout); free(_ins); return;
  }

  fclose(fout);

//-----------------------------------------------------------------------------

  // *** Create install.div (original packed file)

  if ((fout=fopen("install/INSTALL.DIV","wb"))==NULL) {
	v_text=(char *)texts[358]; show_dialog(err0);
    free(_ins); return;
  }

  fwrite(&nfiles,sizeof(int),1,fout);

  if ((header_setup=(header_setup_t *)malloc(nfiles*sizeof(header_setup_t)))==NULL) {
    v_text=(char *)texts[357]; show_dialog(err0);
    fclose(fout); free(_ins); return;
  } fwrite(header_setup,sizeof(header_setup_t),nfiles,fout);

  // Copy all installation files into INSTALL.DIV

  ins=_ins;

  for(x=0;x<nfiles;x++) {
    show_progress((char *)texts[219],x*100,nfiles*100);
    if (x==0) {
      file_in=fopen("system/EXEC.EXE","rb");
      div_strcpy(header_setup[x].name,sizeof(header_setup[x].name),exe_gen);
      topack=0;
    } else if (x==1) {

      file_in=fopen("install/div32run.ins","rb");

      div_strcpy(header_setup[x].name,sizeof(header_setup[x].name),"DIV32RUN.DLL");
      topack=0;
    } else if (x==2 && include_setup) {
      file_in=fopen("install/setup.ovl","rb");
      div_strcpy(header_setup[x].name,sizeof(header_setup[x].name),"SETUP.EXE");
      topack=0;
    } else {
      if (*ins=='+') ins++;
      if (!strcmp(ins,"install/PACKFILE.DAT")) topack=0; else topack=1;
      file_in=fopen(ins,"rb");
      _splitpath(ins,drive,dir,fname,ext);
      div_strcpy(header_setup[x].name,sizeof(header_setup[x].name),fname);
      div_strcat(header_setup[x].name,sizeof(header_setup[x].name),ext);
      if (!topack) div_strcpy(header_setup[x].name,sizeof(header_setup[x].name),dirhead.pack);
      ins+=strlen(ins)+1;
    }

    if (file_in==NULL) {
      show_progress((char *)texts[219],nfiles*100,nfiles*100);
      v_text=(char *)texts[231]; show_dialog(err0);
      fclose(fout); free(_ins); free(header_setup);
      return;
    }

    header_setup[x].offset=ftell(fout);

    header_setup[x].len2=get_file_len(file_in);

    if (topack) {
      header_setup[x].len1=compress_file(file_in,fout,(unsigned long)header_setup[x].len2);
    } else {
      header_setup[x].len1=copy_file(file_in,fout,(unsigned long)header_setup[x].len2,((!x)&&(!pentium)));
    }

    if (header_setup[x].len1==-1) {
      show_progress((char *)texts[219],nfiles*100,nfiles*100);
      v_text=(char *)texts[357]; show_dialog(err0);
      fclose(file_in); fclose(fout); free(_ins); free(header_setup);
      return;
    }

    TotLen+=((get_file_len(file_in)/32768)+1)*32768;
    fclose(file_in);
  }

  free(_ins);
  fseek(fout,sizeof(int),SEEK_SET);
  fwrite(header_setup,sizeof(header_setup_t),nfiles,fout);
  PackSize=((get_file_len(fout)/32768)+1)*32768;
  TotLen+=PackSize;
  fseek(fout,0,SEEK_END);
  fclose(fout);


  free(header_setup);

  show_progress((char *)texts[219],nfiles*100,nfiles*100); // INSTALL.DIV already created

  div_strcpy(cWork,sizeof(cWork),file_types[1].path); // Delete the PACKFILE.DAT
  div_strcat(cWork,sizeof(cWork),"/");
  div_strcat(cWork,sizeof(cWork),"install/PACKFILE.DAT");
  delete_file(cWork);

  // *** Write INSTALL.EXE with the informational trailer

  div_strcpy(cWork,sizeof(cWork),file_types[1].path);
  div_strcat(cWork,sizeof(cWork),"/");
  div_strcat(cWork,sizeof(cWork),"install/install.ovl");

  div_strcpy(dWork,sizeof(dWork),full);
  div_strcat(dWork,sizeof(dWork),"INSTALL.EXE");

  if(!file_copy_ice(cWork,dWork,0,237)) { v_text=(char *)texts[231]; show_dialog(err0); return; }

  show_progress((char *)texts[543],0,100);

  if((fout=fopen(cWork,"ab"))==NULL) {
    show_progress((char *)texts[543],100,100); v_text=(char *)texts[231]; show_dialog(err0); return;
  }

  fwrite(app_name,1,strlen(app_name)+1,fout); n=strlen(app_name)+1;
  fwrite(copyright_text,1,strlen(copyright_text)+1,fout); n+=strlen(copyright_text)+1;
  fwrite(pack_name,1,strlen(pack_name)+1,fout); n+=strlen(pack_name)+1;
  fwrite(default_dir,1,strlen(default_dir)+1,fout); n+=strlen(default_dir)+1;
  fwrite(msg_exe,1,strlen(msg_exe)+1,fout); n+=strlen(msg_exe)+1;
  div_snprintf(cWork,sizeof(cWork),"%s %s",help_text1,help_text2);
  fwrite(cWork,1,strlen(cWork)+1,fout); n+=strlen(cWork)+1;
  fwrite(disk_text1,1,strlen(disk_text1)+1,fout); n+=strlen(disk_text1)+1;
  fwrite(disk_text2,1,strlen(disk_text2)+1,fout); n+=strlen(disk_text2)+1;
  fwrite(install_err0,1,strlen(install_err0)+1,fout); n+=strlen(install_err0)+1;
  fwrite(install_err1,1,strlen(install_err1)+1,fout); n+=strlen(install_err1)+1;
  fwrite(install_err2,1,strlen(install_err2)+1,fout); n+=strlen(install_err2)+1;
  fwrite(install_err3,1,strlen(install_err3)+1,fout); n+=strlen(install_err3)+1;
  fwrite(install_err4,1,strlen(install_err4)+1,fout); n+=strlen(install_err4)+1;
  fwrite(install_err5,1,strlen(install_err5)+1,fout); n+=strlen(install_err5)+1;
  fwrite(install_err6,1,strlen(install_err6)+1,fout); n+=strlen(install_err6)+1;
  fwrite(install_err7,1,strlen(install_err7)+1,fout); n+=strlen(install_err7)+1;
  fwrite(install_err8,1,strlen(install_err8)+1,fout); n+=strlen(install_err8)+1;

  fwrite(&TotLen,1,4,fout); n+=4;
  fwrite(&PackSize,1,4,fout); n+=4;
  fwrite(&create_dir,1,4,fout); n+=4;
  fwrite(&include_setup,1,4,fout); n+=4;
  fwrite(&segundo_font,1,4,fout); n+=4;

  show_progress((char *)texts[543],25,100);

  if ((file_in=fopen(ifile1,"rb"))==NULL) {
    fclose(fout);
    show_progress((char *)texts[543],100,100);
    v_text=(char *)texts[231]; show_dialog(err0); return;
  }

  fseek(file_in,0,SEEK_END); x=ftell(file_in); fseek(file_in,0,SEEK_SET);
  m=compress_file(file_in,fout,(unsigned long)x);
  fclose(file_in);

  if (m==-1) {
    fclose(fout);
    show_progress((char *)texts[543],100,100);
    v_text=(char *)texts[231]; show_dialog(err0); return;
  } else n+=m;

  size[0]=m; size[1]=x;
  show_progress((char *)texts[543],75,100);

  if ((file_in=fopen(ifile2,"rb"))==NULL) {
    fclose(fout);
    show_progress((char *)texts[543],100,100);
    v_text=(char *)texts[231]; show_dialog(err0); return;
  }

  fseek(file_in,0,SEEK_END); x=ftell(file_in); fseek(file_in,0,SEEK_SET);
  m=compress_file(file_in,fout,(unsigned long)x);
  fclose(file_in);

  if (m==-1) {
    fclose(fout);
    show_progress((char *)texts[543],100,100);
    v_text=(char *)texts[231]; show_dialog(err0); return;
  } else n+=m;

  size[2]=m; size[3]=x;
  show_progress((char *)texts[543],85,100);

  if (segundo_font) {
    if ((file_in=fopen(ifile3,"rb"))==NULL) {
      fclose(fout);
      show_progress((char *)texts[543],100,100);
      v_text=(char *)texts[231]; show_dialog(err0); return;
    }

    fseek(file_in,0,SEEK_END); x=ftell(file_in); fseek(file_in,0,SEEK_SET);
    m=compress_file(file_in,fout,(unsigned long)x);
    fclose(file_in);

    if (m==-1) {
      fclose(fout);
      show_progress((char *)texts[543],100,100);
      v_text=(char *)texts[231]; show_dialog(err0); return;
    } else n+=m;

    size[4]=m; size[5]=x;

  } else {

    size[4]=0; size[5]=0;

  }

  fwrite(size,6,4,fout); n+=6*4;

  n+=4; fwrite(&n,1,4,fout);
  fclose(fout);

  show_progress((char *)texts[543],100,100);

  // *** Write pack_name.001, .002, ... from INSTALL.DIV

  div_strcpy(cWork,sizeof(cWork),file_types[1].path);
  div_strcat(cWork,sizeof(cWork),"/");
  div_strcat(cWork,sizeof(cWork),"install/INSTALL.DIV");

  div_strcpy(dWork,sizeof(dWork),full);
  div_strcat(dWork,sizeof(dWork),pack_name);

  if(!file_copy_ice(cWork,dWork,1,234)) { v_text=(char *)texts[231]; show_dialog(err0); return; }

  delete_file(cWork);              // Delete the INSTALL.DIV

#endif

  v_title = (char *)texts[359]; // Dialog indicating installation is complete
  div_strcpy(cWork, sizeof(cWork), (char *)texts[360]);
  strupr(full);
  div_strcat(cWork, sizeof(cWork), full);
  cWork[strlen(cWork) - 1] = 0;
  div_strcat(cWork, sizeof(cWork), (char *)texts[361]);
  v_text = cWork;
  show_dialog(info0);
}

//-----------------------------------------------------------------------------
//  Compresses a file to another , using zlib (returns -1 if error)
//-----------------------------------------------------------------------------

int compress_file(FILE *file_in, FILE *fout, unsigned long len) {
  unsigned char *pin, *pout;
  unsigned long final_len;

  final_len = len + 1024;

  if ((pin = (unsigned char *)malloc(len)) == NULL)
    return (-1);
  if ((pout = (unsigned char *)malloc(final_len)) == NULL) {
    free(pin);
    return (-1);
  }

  if (fread(pin, 1, len, file_in) != len) {
    free(pout);
    free(pin);
    return (-1);
  }

#ifndef ZLIB
  if (false)
#else
  if (compress(pout, &final_len, pin, len))
#endif
  {
    free(pout);
    free(pin);
    return (-1);
  }

  if (fwrite(pout, 1, final_len, fout) != final_len) {
    free(pout);
    free(pin);
    return (-1);
  }

  free(pout);
  free(pin);

  return (final_len);
}

int copy_file(FILE *file_in, FILE *fout, unsigned long len, int patch) {
  unsigned char *pin;

  if ((pin = (unsigned char *)malloc(len)) == NULL)
    return (-1);
  if (fread(pin, 1, len, file_in) != len) {
    free(pin);
    return (-1);
  }

  if (patch) {        // 386+ version (patches the div_stub)
    pin[0x4F] = 0x03; // Compare CPU with 80386 or higher
    pin[0x51] = 0x16; // Otherwise, show "Intel386 not found" message
  }

  if (fwrite(pin, 1, len, fout) != len) {
    free(pin);
    return (-1);
  }
  free(pin);
  return (len);
}

//-----------------------------------------------------------------------------
// Copy the installation destination disk
//-----------------------------------------------------------------------------

unsigned int get_free_drive(char unidad) {
  return 65535 * 64;
}

int file_copy_ice(char *org, char *dest, int vols, int _texto) { // Returns 0 -Error , 1- Success
  FILE *file_in = NULL, *fout = NULL;
  unsigned int tlen, len, NewVolume = 1, curvol = 0, retval = 1;
  unsigned int totfree;
  char *buffer;
  char cWork[256];

  buffer = (char *)malloc(16384);
  if (buffer == NULL)
    return 0;

  file_in = fopen(org, "rb");
  if (file_in == NULL) {
    free(buffer);
    return 0;
  }

  fseek(file_in, 0, SEEK_END);
  tlen = len = ftell(file_in);
  fseek(file_in, 0, SEEK_SET);

  while (len) {
    show_progress((char *)texts[_texto], tlen - len, tlen);

    if (NewVolume) {
      if (curvol != 0 && NewVolume < 2) {
        if (!is_disk) {
          retval = 0;
          break;
        }
        v_title = (char *)texts[233]; // Disk full.
        v_text = (char *)texts[232];  // Please, insert a new disk.
        show_dialog(accept0);
        if (!v_accept) {
          retval = 0;
          break;
        }
      }

      NewVolume = 0;

      totfree = get_free_drive((dest[0] - 'A') + 1);

      if (totfree <= 1024) { // Disk Full
        if (vols) {
          v_title = (char *)texts[362];
          v_text = (char *)texts[232];
          show_dialog(accept0);
          if (v_accept) {
            NewVolume = 2;
            continue;
          }
        }
        retval = 0;
        break;
      }

      if (vols) {
        div_snprintf(cWork, sizeof(cWork), "%s.%03d", dest, curvol + 1);
        debugprintf("cwork: %s\n", cWork);
        fout = fopen(cWork, "wb");
      } else
        fout = fopen(dest, "wb");

      if (fout == NULL) { // Protected floppy(!?)
        v_title = (char *)texts[363];
        v_text = (char *)texts[364];
        show_dialog(accept0);
        if (v_accept) {
          NewVolume = 2;
          continue;
        }
        retval = 0;
        break;
      }

      if (vols) {
        if (fwrite("stp\x1a\x0d\x0a\x00", 1, 8, fout) != 8) { // Any other error
          retval = 0;
          break;
        }
        totfree -= 8;
        curvol++;
      }
    }

    if (totfree > 16384) { // Enough disk space to record 16384 bytes
      if (len > 16384) {   // Enough to save complete 16384 bytes block
        fread(buffer, 1, 16384, file_in);
        if (fwrite(buffer, 1, 16384, fout) != 16384) {
          retval = 0;
          break;
        }
        len -= 16384;
        totfree -= 16384;
      } else {
        fread(buffer, 1, len, file_in);
        if (fwrite(buffer, 1, len, fout) != len) {
          retval = 0;
          break;
        }
        totfree -= len;
        len = 0;
      }

    } else {                // Less than 16384 bytes of free space
      if (len <= totfree) { // Still fits, copy it anyway
        fread(buffer, 1, len, file_in);
        if (fwrite(buffer, 1, len, fout) != len) {
          retval = 0;
          break;
        }
        totfree -= len;
        len = 0;
      } else { // Doesn't fit (only totfree space left)

        if (!vols) {
          retval = 0;
          break;
        }

        fread(buffer, 1, totfree, file_in);
        if (fwrite(buffer, 1, totfree, fout) != totfree) {
          retval = 0;
          break;
        }
        len -= totfree;
        totfree = 0;
        fseek(fout, 7, SEEK_SET);
        fwrite("\x01", 1, 1, fout);
        fclose(fout);
        fout = NULL;
        NewVolume = 1;
      }
    }
  }

  if (file_in)
    fclose(file_in);
  if (fout)
    fclose(fout);
  show_progress((char *)texts[_texto], tlen, tlen);
  free(buffer);
  return (retval);
}
