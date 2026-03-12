#include "../inter.h"
#include "div_string.h"


#ifndef __EMSCRIPTEN__

// Helper to safely sprintf into a buffer using a temp to avoid overlap warnings
#define SAFE_SPRINTF(dst, fmt, ...)                             \
  do {                                                          \
    char _tmp_buf[512];                                         \
    div_snprintf(_tmp_buf, sizeof(_tmp_buf), fmt, __VA_ARGS__); \
    div_strcpy(dst, sizeof(dst), _tmp_buf);                     \
  } while (0)
// Copy between different elements of the same array (compiler can't prove non-overlap)
static inline void array_strcpy(char *dst, size_t dst_size, const char *src) {
  char tmp[512];
  div_strcpy(tmp, sizeof(tmp), src);
  div_strcpy(dst, dst_size, tmp);
}

#define STACK_SIZE 128

int dstack[STACK_SIZE];
byte stp[STACK_SIZE];
char cstack[STACK_SIZE][512];

#define getvarref(X) cstack[X]

FILE *prg = NULL;

byte locvar[STACK_SIZE];
byte localvar;

void getvarname(int i, char *name) {
  char mousestruct[12][10] = {"x",    "y",     "z",      "file", "graph",  "angle",
                              "size", "flags", "region", "left", "middle", "right"};

  char m7struct[8][10] = {"z", "camera", "height", "distance", "horizon", "focus", "color"};
  switch (i) {
  case 16:
    div_strcpy(name, 255, "father");
    break;
  case 21:
    div_strcpy(name, 255, "ctype");
    break;
  case 22:
    div_strcpy(name, 255, "x");
    break;
  case 23:
    div_strcpy(name, 255, "y");
    break;
  case 24:
    div_strcpy(name, 255, "z");
    break;
  case 25:
    div_strcpy(name, 255, "graph");
    break;
  case 26:
    div_strcpy(name, 255, "flags");
    break;
  case 27:
    div_strcpy(name, 255, "size");
    break;
  case 28:
    div_strcpy(name, 255, "angle");
    break;
  case 29:
    div_strcpy(name, 255, "region");
    break;
  case 30:
    div_strcpy(name, 255, "file");
    break;
  case 34:
    div_strcpy(name, 255, "resolution");
    break;
  default:
    div_snprintf(name, 255, "var%d", i);
    break;
  }
  if (i >= mem[6] && i < 255) {
    if (localvar == 0)
      i += mem[5];
    div_snprintf(name, 255, "var%d", i);

    if (i >= iloc_len && i <= iloc_len + 12) {
      div_snprintf(name, 255, "mouse.%s", mousestruct[i - iloc_len]);
    }
    if (i >= iloc_len + 112 && i <= iloc_len + 120) {
      div_snprintf(name, 255, "m7.%s", m7struct[i - iloc_len - 112]);
    }
  }
}


void printglobals(void) {
  int i = 255; //mem[1];
  fprintf(prg, "GLOBAL // %u %u\n", i, mem[1] - 1);
  while (i <= mem[1] - 1) {
    fprintf(prg, "var%u=%d;\n", i, mem[i]);
    i++;
  }
}
void printlocal(void) {
  int i = mem[2] + 35;

  fprintf(prg, "LOCAL\n");

  while (i < mem[7]) {
    fprintf(prg, "var%u=%d;\n", i, mem[i]);
    i++;
  }
}


void dump(int size) {
  int i = mem[1] - 1, itmp = 0;
  int sp = 0, spp = 0;

  int *jmp;
  int *jpf;
  int *jpb;
  int *end;
  int *els;

  int j = 0;
  int args = 0;
  int ifloop = 0;
  char condstack[255][100];
  int con = 0;
  int f = 0;
  //					0    1     2    3    4    5    6    7    8    9
  char keys[128][15] = {
      "", "_esc", "_1", "_2", "_3", "_4", "_5", "_6", "_7", "_8",
      //				   10    11     12       13         14       15
      "_9", "_0", "_minus", "_plus", "_backspace", "_tab",

      //				   16    17   18   19   20   21   22   23   24   25   26           27          28
      "_q", "_w", "_e", "_r", "_t", "_y", "_u", "_i", "_o", "_p", "_l_bracket", "_r_bracket",
      "_enter",
      //				   29          30   31   32   33   34   35   36   37   38    39          40            41
      "_control", "_a", "_s", "_d", "_f", "_g", "_h", "_j", "_k", "_l", "_semicolon", "_apostrophe",
      "_wave"
      //				   42          43           44   45   46   47   48   49   50    51           52
      "_l_shift",
      "_backslash", "_z", "_x", "_c", "_v", "_b", "_n", "_m", "_comma", "", "_c_backslash",
      //					53
      "_c_backslash", "_r_shift", "_c_asterisk", "_alt", "_space", "_caps_lock",
      //					59		60    61    62   63    64     65    66    67    68
      "_f1", "_f2", "_f3", "_f4", "_f5", "_f6", "_f7", "_f8", "_f9", "_f10",

      //
      "_num_lock", "_scroll_lock", "_home", "_up", "_pgup", "_c_minus", "_left", "_c_center",

      "_right", "_plus", "_end", "_down", "_c_pgdn", "_c_ins", "_c_del"
      //						"_f11","_f12"
  };
  jmp = (int *)malloc(size * 5);
  jpf = (int *)malloc(size * 5);
  jpb = (int *)malloc(size * 5);
  end = (int *)malloc(size * 5);
  els = (int *)malloc(size * 5);

  memset(jmp, 0, size * 4);
  memset(jpf, 0, size * 4);
  memset(jpb, 0, size * 4);
  memset(end, 0, size * 4);
  memset(els, 0, size * 4);

  char name[255];

  char cmd[512];
  memset(cmd, 0, 255);
  printf("pos is %d %d\n", i, size);

  FILE *sta = fopen("dump.txt", "w+");
  prg = fopen("prg.prg", "w+");


  // find the jmps

  while (i++ < mem[7] - 36) {
    switch ((byte)mem[i]) {
    case lnop:
      fprintf(sta, "%5u\tnop", i);
      break;
    case lcar:
      fprintf(sta, "%5u\tcar %u", i, mem[i + 1]);
      i++;
      break;
    case lasi:
      fprintf(sta, "%5u\tasi", i);
      break;
    case lori:
      fprintf(sta, "%5u\tori", i);
      break;
    case lxor:
      fprintf(sta, "%5u\txor", i);
      break;
    case land:
      fprintf(sta, "%5u\tand", i);
      break;
    case ligu:
      fprintf(sta, "%5u\tigu", i);
      break;
    case ldis:
      fprintf(sta, "%5u\tdis", i);
      break;
    case lmay:
      fprintf(sta, "%5u\tmay", i);
      break;
    case lmen:
      fprintf(sta, "%5u\tmen", i);
      break;
    case lmei:
      fprintf(sta, "%5u\tmei", i);
      break;
    case lmai:
      fprintf(sta, "%5u\tmai", i);
      break;
    case ladd:
      fprintf(sta, "%5u\tadd", i);
      break;
    case lsub:
      fprintf(sta, "%5u\tsub", i);
      break;
    case lmul:
      fprintf(sta, "%5u\tmul", i);
      break;
    case ldiv:
      fprintf(sta, "%5u\tdiv", i);
      break;
    case lmod:
      fprintf(sta, "%5u\tmod", i);
      break;
    case lneg:
      fprintf(sta, "%5u\tneg", i);
      break;
    case lptr:
      fprintf(sta, "%5u\tptr", i);
      break;
    case lnot:
      fprintf(sta, "%5u\tnot", i);
      break;
    case laid:
      fprintf(sta, "%5u\taid", i);
      break;
    case lcid:
      fprintf(sta, "%5u\tcid", i);
      break;
    case lrng:
      fprintf(sta, "%5u\trng %u", i, mem[i + 1]);
      i++;
      break;
    case ljmp:
      fprintf(sta, "%5u\tjmp %u", i, mem[i + 1]);
      jmp[mem[i + 1]]++;
      i++;
      break;
    case ljpf:
      fprintf(sta, "%5u\tjpf %u", i, mem[i + 1]);
      if (mem[i + 1] < i) {
        printf("jump backward %d %d\n", i, mem[i + 1]);
        jpb[mem[i + 1]]++;
      } else {
        printf("jump forward %d %d\n", i, mem[i + 1]);
        jpf[mem[i + 1]]++;
      }
      // check if there is a back jump before
      if (mem[mem[i + 1] - 2] == ljmp) {
        printf("jpf followed by jump %d %d %d\n", i, mem[i + 1], mem[mem[i + 1] - 1]);
        jmp[mem[mem[i + 1] - 1]]--;
        printf("jmp[%d]=%d\n", mem[mem[i + 1] - 1], jmp[mem[mem[i + 1] - 1]]);
      }
      i++;
      break;
    case lfun:
      fprintf(sta, "%5u\tfun %u stack: %d", i, mem[i + 1], sp);
      switch (mem[i + 1]) {
      case 2:  // load_pal
      case 3:  // load_fpg
      case 15: // load_fnt
      case 16: // write
               // check if ref is ptr or static

        // ptr
      case 34: // save
      case 35: // load
      case 37: // load_pcm
      case 47: // system
      case 67: // exit
        printf("text used at %u\n", i);

        if (mem[i + 1] == 16) {   // write
          if (mem[i - 2] == lcar) // static
            printf("static text val\n");
          else
            printf("by ref\n");
        }

        break;
      }
      i++;
      break;
    case lcal:
      fprintf(sta, "%5u\tcal %u", i, mem[i + 1]);
      i++;
      break;
    case lret:
      fprintf(sta, "%5u\tret", i);
      break;
    case lasp:
      fprintf(sta, "%5u\tasp", i);
      break;
    case lfrm:
      fprintf(sta, "%5u\tfrm", i);
      break;
    case lcbp:
      fprintf(sta, "%5u\tcbp %u", i, mem[i + 1]);
      i++;
      break;
    case lcpa:
      fprintf(sta, "%5u\tcpa", i);
      break;
    case ltyp:
      fprintf(sta, "\n%5u\ttyp %u", i, mem[i + 1]);
      i++;
      break;
    case lpri:
      fprintf(sta, "%5u\tpri %u", i, mem[i + 1]);
      i++;
      break;
    case lcse:
      fprintf(sta, "%5u\tcse %u", i, mem[i + 1]);
      i++;
      break;
    case lcsr:
      fprintf(sta, "%5u\tcsr %u", i, mem[i + 1]);
      i++;
      break;
    case lshr:
      fprintf(sta, "%5u\tshr", i);
      break;
    case lshl:
      fprintf(sta, "%5u\tshl", i);
      break;
    case lipt:
      fprintf(sta, "%5u\tipt", i);
      break;
    case lpti:
      fprintf(sta, "%5u\tpti", i);
      break;
    case ldpt:
      fprintf(sta, "%5u\tdpt", i);
      break;
    case lptd:
      fprintf(sta, "%5u\tptd", i);
      break;
    case lada:
      fprintf(sta, "%5u\tada", i);
      break;
    case lsua:
      fprintf(sta, "%5u\tsua", i);
      break;
    case lmua:
      fprintf(sta, "%5u\tmua", i);
      break;
    case ldia:
      fprintf(sta, "%5u\tdia", i);
      break;
    case lmoa:
      fprintf(sta, "%5u\tmoa", i);
      break;
    case lana:
      fprintf(sta, "%5u\tana", i);
      break;
    case lora:
      fprintf(sta, "%5u\tora", i);
      break;
    case lxoa:
      fprintf(sta, "%5u\txoa", i);
      break;
    case lsra:
      fprintf(sta, "%5u\tsra", i);
      break;
    case lsla:
      fprintf(sta, "%5u\tsla", i);
      break;
    case lpar:
      fprintf(sta, "%5u\tpar %u", i, mem[i + 1]);
      i++;
      break;
    case lrtf:
      fprintf(sta, "%5u\trtf", i);
      break;
    case lclo:
      fprintf(sta, "%5u\tclo %u", i, mem[i + 1]);
      i++;
      break;
    case lfrf:
      fprintf(sta, "%5u\tfrf", i);
      break;
    case limp:
      fprintf(sta, "%5u\timp %u", i, mem[i + 1]);
      i++;
      break;
    case lext:
      fprintf(sta, "%5u\text %u", i, mem[i + 1]);
      i++;
      break;
    case lchk:
      fprintf(sta, "%5u\tchk", i);
      break;
    case ldbg:
      fprintf(sta, "%5u\tdbg", i);
      break;
    }
  }


  rewind(sta);
  j = 0;
  f = 0;

  // GLOBAL vars;


  i = mem[1] - 1;
  while (i++ < mem[2] - 1) {
    fprintf(sta, "\n");

    if (end[i] > 0) {
      while (end[i] > 0) {
        fprintf(prg, "END // endstop %d %d\n\n", i, end[i]);
        end[i]--;
      }
    }
    if (els[i] > 0) {
      while (els[i] > 0) {
        fprintf(prg, "ELSE // else %d %d\n\n", i, els[i]);
        els[i]--;
      }
    }

    if (jmp[i] || jpb[i] || jpf[i]) {
      if (jmp[i] > 0) {
        fprintf(prg, "\nLOOP // %d %d\n\n", i, jmp[i]);
      }

      if (jpb[i] > 0) {
        fprintf(prg, "\nREPEAT // %d %d\n", i, jpb[i]);
      } else {
      }
    }

    switch ((byte)mem[i]) {
      // no op
    case lnop:
      fprintf(sta, "%5u\tnop", i);
      fprintf(prg, "// NOP\n");
      break;

      // load val to stack
    case lcar:
      fprintf(sta, "%5u\tcar %d", i, mem[i + 1]);
      dstack[sp] = mem[i + 1];
      div_snprintf(cstack[sp], sizeof(cstack[sp]), "%d%c", mem[i + 1], 0);
      div_strcpy(cmd, sizeof(cmd), cstack[sp]);
      stp[sp] = 0;    // not a pointer
      locvar[sp] = 0; // not a local
      sp++;
      i++;
      break;
    case lasi:
      fprintf(sta, "%5u\tasi", i);
      if (strlen(cmd) > 0 && sp > 1) {
        fprintf(prg, "// asi stack %s %s %d %d\n", cstack[sp - 2], cstack[sp - 1], sp, i);
        fprintf(prg, "%s=%s", cstack[sp - 2], cmd);
      }
      memset(cmd, 0, 255);
      con = 1;
      sp--;
      break;
    case lori:
      fprintf(sta, "%5u\tori", i);
      fprintf(prg, "// stack: || %d %d\n", sp, i);
      SAFE_SPRINTF(condstack[con], "%s OR %s", condstack[con - 2], condstack[con - 1]);
      array_strcpy(condstack[con - 2], sizeof(condstack[con - 2]), condstack[con]);
      div_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), condstack[con]);

      con--;
      div_strcpy(cmd, sizeof(cmd), condstack[con - 1]);

      break;
    case lxor:
      fprintf(sta, "%5u\txor", i);
      fprintf(prg, "// UNIMP! XOR\n");
      fprintf(prg, "// stack: %d\n", sp);
      break;
    case land:
      fprintf(sta, "%5u\tand", i);
      fprintf(prg, "// stack: && %d\n", sp);
      SAFE_SPRINTF(condstack[con], "(%s && %s)", condstack[con - 2], condstack[con - 1]);
      array_strcpy(condstack[con - 2], sizeof(condstack[con - 2]), condstack[con]);
      con--;
      div_strcpy(cmd, sizeof(cmd), condstack[con - 1]);
      break;
    case ligu:
      fprintf(sta, "%5u\tigu", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "%s == %s", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;
      break;
    case ldis:
      fprintf(sta, "%5u\tdis", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "%s != %s", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;

      break;
    case lmay:
      fprintf(sta, "%5u\tmay", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "%s > %s", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;
      break;
    case lmen:
      fprintf(sta, "%5u\tmen", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "%s < %s", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;

      break;
    case lmei:
      fprintf(sta, "%5u\tmei", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "(%s <= %s)", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;

      break;
    case lmai:
      fprintf(sta, "%5u\tmai", i);
      div_snprintf(condstack[con], sizeof(condstack[con]), "(%s >= %s)", cstack[sp - 2],
                   cstack[sp - 1]);
      div_strcpy(cmd, sizeof(cmd), condstack[con]);
      con++;
      break;
    case ladd:
      fprintf(sta, "%5u\tadd", i);
      fprintf(prg, "// ladd %d[%d]+%d[%d] %d %d\n", dstack[sp - 2], stp[sp - 2], dstack[sp - 1],
              stp[sp - 1], sp, i);
      SAFE_SPRINTF(cstack[sp], "(%s+%s)", cstack[sp - 2], cstack[sp - 1]);
      dstack[sp - 2] += dstack[sp - 1];
      div_strcpy(cmd, sizeof(cmd), cstack[sp]);
      array_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), cstack[sp]);
      sp--;
      break;
    case lsub:
      fprintf(sta, "%5u\tsub", i);
      //dstack[sp-2]-=dstack[sp-1];
      SAFE_SPRINTF(cstack[sp], "(%s-%s)", cstack[sp - 2], cstack[sp - 1]);
      dstack[sp - 2] -= dstack[sp - 1];
      array_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), cstack[sp]);
      div_strcpy(cmd, sizeof(cmd), cstack[sp - 2]);

      sp--;
      break;
    case lmul:
      fprintf(sta, "%5u\tmul", i);
      SAFE_SPRINTF(cstack[sp], "(%s*%s)", cstack[sp - 2], cstack[sp - 1]);
      array_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), cstack[sp]);
      sp -= 1;

      break;
    case ldiv:
      fprintf(sta, "%5u\tdiv", i);
      SAFE_SPRINTF(cstack[sp], "(%s/%s)", cstack[sp - 2], cstack[sp - 1]);
      array_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), cstack[sp]);
      sp -= 1;

      break;
    case lmod:
      fprintf(sta, "%5u\tmod", i);
      SAFE_SPRINTF(cstack[sp], "(%s MOD %s)", cstack[sp - 2], cstack[sp - 1]);
      array_strcpy(cstack[sp - 2], sizeof(cstack[sp - 2]), cstack[sp]);
      sp -= 1;
      break;

    case lneg:
      fprintf(sta, "%5u\tneg", i);
      SAFE_SPRINTF(cstack[sp], "-%s", cstack[sp - 1]);
      dstack[sp - 1] = -dstack[sp - 1];
      div_strcpy(cmd, sizeof(cmd), cstack[sp]);
      array_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cstack[sp]);

      break;

    case lptr:
      fprintf(sta, "%5u\tptr", i);
      //
      fprintf(prg, "// lptr %s %d %d\n", cstack[sp - 1], dstack[sp - 1], i);
      // get var names
      localvar = locvar[sp - 1];
      getvarname(dstack[sp - 1], name);
      div_snprintf(cstack[sp - 1], sizeof(cstack[sp - 1]), "%s", name);
      div_strcpy(cmd, sizeof(cmd), cstack[sp - 1]);
      stp[sp - 1] = 1;
      break;
    case lnot:
      fprintf(sta, "%5u\tnot", i);
      break;
    case laid:
      fprintf(sta, "%5u\taid", i);
      localvar = locvar[sp - 1];
      getvarname(dstack[sp - 1], cstack[sp - 1]);
      fprintf(prg, "// %s %d %d %d\n", cstack[sp - 1], dstack[sp - 1], i, localvar);

      break;
    case lcid:
      fprintf(sta, "%5u\tcid", i);
      dstack[sp] = 0;
      div_strcpy(cstack[sp], sizeof(cstack[sp]), "id");
      div_strcpy(cmd, sizeof(cmd), cstack[sp]);
      stp[sp] = 1;
      sp++;
      break;
    case lrng:
      fprintf(sta, "%5u\trng %u", i, mem[i + 1]);
      fprintf(prg, "// UNIMP range %d %d\n", i, mem[i + 1]);
      i++;
      break;
    case ljmp:
      fprintf(sta, "%5u\tjmp %u", i, mem[i + 1]);
      if (mem[i + 1] < i) {
        fprintf(prg, "// jmp[%d]=%d JMP BACK TO %d %d\nEND\n\n", i, jmp[i], i, mem[i + 1]);
      }
      i++;
      break;
    case ljpf:
      fprintf(sta, "%5u\tjpf %u", i, mem[i + 1]);

      if (mem[i + 1] < i)
        fprintf(prg, "// %d\nUNTIL", i);
      else {
        if (strlen(cmd) == 0)
          div_strcpy(cmd, sizeof(cmd), cstack[sp - 1]);
        // check if the jpf goes past a jmp

        if (mem[mem[i + 1] - 2] == ljmp) {
          if (mem[mem[i + 1] - 1] < mem[i + 1] - 2)
            fprintf(prg, "WHILE");
          else {
            fprintf(prg, "// else at %d %d\n", mem[mem[i + 1]], mem[i + 1]);
            fprintf(prg, "IF");
            els[mem[i + 1]]++;
            // end after jpf/jmp
            end[mem[mem[i + 1] - 1]]++;
          }
        } else {
          fprintf(prg, "IF");
          // ifs need an end, set an endstop
          end[mem[i + 1]]++;
        }
      }

      fprintf(prg, "(%s) // if val %d %d %s\n", cmd, i, sp, cstack[sp - 1]);

      memset(cmd, 0, 255);
      i++;
      con = 0;
      sp = 0;
      break;
    case lfun:
      fprintf(sta, "%5u\tfun %u", i, mem[i + 1]);
      // calling function


      switch (mem[i + 1]) {
      case 0: // signal

        div_snprintf(cmd, sizeof(cmd), "signal(%s,", cstack[sp - 2]);

        switch (dstack[sp - 1]) {
        case 0:
          div_strcat(cmd, sizeof(cmd), "s_kill");
          break;
        case 1:
          div_strcat(cmd, sizeof(cmd), "s_wakeup");
          break;
        case 2:
          div_strcat(cmd, sizeof(cmd), "s_sleep");
          break;
        case 3:
          div_strcat(cmd, sizeof(cmd), "s_freeze");
          break;
        case 100:
          div_strcat(cmd, sizeof(cmd), "s_kill_tree");
          break;
        case 101:
          div_strcat(cmd, sizeof(cmd), "s_wakeup_tree");
          break;
        case 102:
          div_strcat(cmd, sizeof(cmd), "s_sleet_tree");
          break;
        case 103:
          div_strcat(cmd, sizeof(cmd), "s_freeze_tree");
          break;
        default:
          div_strcat(cmd, sizeof(cmd), "0");
          break;
        }
        div_strcat(cmd, sizeof(cmd), ")");
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 1:
        div_snprintf(cmd, sizeof(cmd), "key(%s)", keys[dstack[sp - 1]]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 2: // load_pal
        div_snprintf(cmd, sizeof(cmd), "load_pal(\"%s\")", (byte *)&mem[mem[7] + dstack[sp - 1]]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);

        break;

      case 3:
        if (stp[sp - 1]) {
          div_snprintf(cmd, sizeof(cmd), "load_fpg(%s)", cstack[sp - 1]);
          div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        } else {
          div_snprintf(cmd, sizeof(cmd), "load_fpg(\"%s\")", (byte *)&mem[mem[7] + dstack[sp - 1]]);
          div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        }
        break;

      case 4: // start_scroll (6)
        div_snprintf(cmd, sizeof(cmd), "start_scroll(%s, %s, %s, %s, %s, %s)", cstack[sp - 6],
                     cstack[sp - 5], cstack[sp - 4], cstack[sp - 3], cstack[sp - 2],
                     cstack[sp - 1]);
        sp -= 5;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 5: // stop_scroll
        div_snprintf(cmd, sizeof(cmd), "stop_scroll(%s)", cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 6: // out_region (2)
        div_snprintf(cmd, sizeof(cmd), "out_region(%s,%s)", cstack[sp - 2], cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        sp--;
        break;
      case 8:
        div_snprintf(cmd, sizeof(cmd), "collision(type proc%s)", cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 9: // get_id (1)
        fprintf(prg, "// GET_ID\n");
        div_snprintf(cmd, sizeof(cmd), "get_id(type proc%s)", cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 12:
        div_snprintf(cmd, sizeof(cmd), "get_angle(%s)", cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 15:
        div_snprintf(cmd, sizeof(cmd), "load_fnt(\"%s\")", (byte *)&mem[mem[7] + dstack[sp - 1]]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 16: // write (5)
        div_snprintf(cmd, sizeof(cmd), "write(%d,%d,%d,%d,\"%s\")", dstack[sp - 5], dstack[sp - 4],
                     dstack[sp - 3], dstack[sp - 2], (byte *)&mem[mem[7] + dstack[sp - 1]]);
        sp -= 4;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 17:
        div_snprintf(cmd, sizeof(cmd), "write_int(%d,%d,%d,%d,&var%d)", dstack[sp - 5],
                     dstack[sp - 4], dstack[sp - 3], dstack[sp - 2], dstack[sp - 1]);
        sp -= 4;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 18:
        if (dstack[sp - 1] == 0)
          div_snprintf(cmd, sizeof(cmd), "delete_text(all_text)");
        else
          div_snprintf(cmd, sizeof(cmd), "delete_text(%d)", dstack[sp - 1]);

        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);

        break;
      case 20: // unload_fpg (1)
        div_snprintf(cmd, sizeof(cmd), "unload_fpg(%s)", cstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 21: // rand (2)
        div_snprintf(cmd, sizeof(cmd), "rand(%d,%d)", dstack[sp - 2], dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 22: // define region (5)
        div_snprintf(cmd, sizeof(cmd), "define_region(%d,%d,%d,%d,%d)", dstack[sp - 5],
                     dstack[sp - 4], dstack[sp - 3], dstack[sp - 2], dstack[sp - 1]);
        sp -= 4;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 24: // put (4)
        div_snprintf(cmd, sizeof(cmd), "put(%d,%d,%d,%d)", dstack[sp - 4], dstack[sp - 3],
                     dstack[sp - 2], dstack[sp - 1]);
        sp -= 3;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;


      case 25:
        div_snprintf(cmd, sizeof(cmd), "put_screen(%d,%d)", dstack[sp - 2], dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 27: // map_put(); 5
        div_snprintf(cmd, sizeof(cmd), "map_put(%s,%s,%s,%s,%s)", cstack[sp - 5], cstack[sp - 4],
                     cstack[sp - 3], cstack[sp - 2], cstack[sp - 1]);
        sp -= 4;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 29:
        div_snprintf(cmd, sizeof(cmd), "get_pixel(%s,%s)", cstack[sp - 2], cstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 33: // clear_screen
        div_snprintf(cmd, sizeof(cmd), "clear_screen()");
        sp++;
        div_strcpy(cstack[sp], sizeof(cstack[sp]), cmd);
        break;

      case 35: //load
        div_snprintf(cmd, sizeof(cmd), "load(\"%s\",offset var%d)",
                     (byte *)&mem[mem[7] + dstack[sp - 2]], dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 36:
        div_snprintf(cmd, sizeof(cmd), "set_mode(%d)", dstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 37: // load_pcm (2)
        div_snprintf(cmd, sizeof(cmd), "load_pcm(\"%s\",%d)", (byte *)&mem[mem[7] + dstack[sp - 2]],
                     dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 39: // sound (3)
        div_snprintf(cmd, sizeof(cmd), "sound(%s,%s,%s)", getvarref(sp - 3), getvarref(sp - 2),
                     getvarref(sp - 1));
        sp -= 2;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);

        break;

      case 40: // stop_sound (1)
        div_snprintf(cmd, sizeof(cmd), "stop_sound(%d)", dstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 42: // set_fps (2)
        div_snprintf(cmd, sizeof(cmd), "set_fps(%d,%d)", dstack[sp - 2], dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 54: // start_mode7 (6)
        div_snprintf(cmd, sizeof(cmd), "start_mode7(%s,%s,%s,%s,%s,%s)", cstack[sp - 6],
                     cstack[sp - 5], cstack[sp - 4], cstack[sp - 3], cstack[sp - 2],
                     cstack[sp - 1]);
        sp -= 5;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 56: // advance (1)
        div_snprintf(cmd, sizeof(cmd), "advance(%d)", dstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        //sp;
        break;

      case 57: // abs(); (1)
        div_snprintf(cmd, sizeof(cmd), "abs(%d)", dstack[sp - 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 58: // fade_on (0)
        div_snprintf(cmd, sizeof(cmd), "fade_on()");
        sp++;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 66: // let_me_alone (0)
        div_snprintf(cmd, sizeof(cmd), "let_me_alone()");
        sp++;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;

      case 67: // exit (2)
        div_snprintf(cmd, sizeof(cmd), "exit(\"%s\",%d)", (byte *)&mem[mem[7] + dstack[sp - 2]],
                     dstack[sp - 1]);
        sp--;
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;


      default:
        div_snprintf(cmd, sizeof(cmd), "func(%d)", mem[i + 1]);
        div_strcpy(cstack[sp - 1], sizeof(cstack[sp - 1]), cmd);
        break;
      }
      div_strcpy(condstack[con], sizeof(condstack[con]), cmd);
      con++;
      i++;
      break;
    case lcal:
      fprintf(sta, "%5u\tcal %u", i, mem[i + 1]);
      div_snprintf(cmd, sizeof(cmd), "proc%u(", mem[mem[i + 1] + 1]);

      fprintf(prg, "// assigned: %s\n", mem[i + 2] == lasi ? "yes" : "no");

      spp = sp;
      while (sp > (mem[i + 2] == lasi ? 1 : 0)) {
        div_strcat(cmd, sizeof(cmd), cstack[spp - sp]);
        sp--;
        if (sp > (mem[i + 2] == lasi ? 1 : 0))
          div_strcat(cmd, sizeof(cmd), ",");
      }
      div_strcat(cmd, sizeof(cmd), ")");
      div_strcpy(cstack[sp], sizeof(cstack[sp]), cmd);
      sp++;
      i++;
      break;

    case lret:
      fprintf(sta, "%5u\tret", i);
      fprintf(prg, "\nEND // lret %d\n\n", i);
      if (i && 1)
        i++;
      break;
    case lasp:
      fprintf(sta, "%5u\tasp", i);
      fprintf(prg, "%s;\n", cmd);
      memset(cmd, 0, 255);
      con = 0;
      sp--;

      break;
    case lfrm:
      fprintf(sta, "%5u\tfrm", i);
      fprintf(prg, "FRAME;\n");
      break;

    case lcbp:
      fprintf(sta, "%5u\tcbp %u", i, mem[i + 1]);
      //	fprintf(prg,"// %d args\n",mem[i+1]);
      args = mem[i + 1];

      if (args == 0)
        fprintf(prg, ");\n");

      i++;
      break;
    case lcpa:
      fprintf(sta, "%5u\tcpa", i);
      if (args > 0) {
        args--;
        localvar = locvar[0];
        getvarname(dstack[0], name);
        fprintf(prg, "%s ", name);
        if (args == 0)
          fprintf(prg, ");\n\n");
        //BEGIN /// lcpa\n\n");
        else
          fprintf(prg, ",");
      }
      sp = 0;
      break;
    case ltyp:
      fprintf(sta, "\n%5u\ttyp %u", i, mem[i + 1]);
      if (mem[i + 2] == lfrm) {
        fprintf(prg, "PROGRAM myprg;\n");
        i += 2;
        printglobals();
        printlocal();
      } else {
        fprintf(prg, "\n\nPROCESS proc%u(", mem[i + 1]);
      }

      i++;
      break;
    case lpri:
      fprintf(sta, "%5u\tpri %u", i, mem[i + 1]);
      fprintf(prg, "PRIVATE\n");
      itmp = i;
      i++;
      while (i < (mem[itmp + 1] - 1)) {
        fprintf(prg, "var%u=%u; // %d\n", mem[6] - 1 + (i - itmp), mem[i + 1], i);
        i++;
      }
      fprintf(prg, "\nBEGIN // lpri\n");
      sp = 0;
      break;
    case lcse:
      fprintf(sta, "%5u\tcse %u", i, mem[i + 1]);
      i++;
      break;
    case lcsr:
      fprintf(sta, "%5u\tcsr %u", i, mem[i + 1]);
      i++;
      break;
    case lshr:
      fprintf(sta, "%5u\tshr", i);
      break;
    case lshl:
      fprintf(sta, "%5u\tshl", i);
      break;
    case lipt:
      fprintf(sta, "%5u\tipt", i);
      break;
    case lpti:
      fprintf(sta, "%5u\tpti", i);
      localvar = locvar[0];
      getvarname(dstack[0], cmd);
      div_strcat(cmd, sizeof(cmd), "++");
      break;
    case ldpt:
      fprintf(sta, "%5u\tdpt", i);
      break;

    case lptd:
      fprintf(sta, "%5u\tptd", i);
      localvar = locvar[0];
      getvarname(dstack[0], cmd);
      div_strcat(cmd, sizeof(cmd), "++");
      break;

    case lada:
      fprintf(sta, "%5u\tada", i);
      localvar = locvar[sp - 2];
      getvarname(dstack[sp - 2], cstack[sp - 2]);
      div_snprintf(cmd, sizeof(cmd), "%s+=%s", cstack[sp - 2], cstack[sp - 1]);
      fprintf(prg, "// %s %i\n", cmd, i);
      sp--;
      break;
    case lsua:
      fprintf(sta, "%5u\tsua", i);
      localvar = locvar[sp - 2];
      getvarname(dstack[sp - 2], name);
      div_snprintf(cmd, sizeof(cmd), "%s-=%s", name, cstack[sp - 1]);
      break;
    case lmua:
      fprintf(sta, "%5u\tmua", i);
      break;
    case ldia:
      fprintf(sta, "%5u\tdia", i);
      break;
    case lmoa:
      fprintf(sta, "%5u\tmoa", i);
      break;
    case lana:
      fprintf(sta, "%5u\tana", i);
      break;
    case lora:
      fprintf(sta, "%5u\tora", i);
      break;
    case lxoa:
      fprintf(sta, "%5u\txoa", i);
      break;
    case lsra:
      fprintf(sta, "%5u\tsra", i);
      break;
    case lsla:
      fprintf(sta, "%5u\tsla", i);
      break;
    case lpar:
      fprintf(sta, "%5u\tpar %u", i, mem[i + 1]);
      fprintf(prg, "// lpar: %d\n", mem[i + 1]);
      i++;
      break;
    case lrtf:
      fprintf(sta, "%5u\trtf", i);
      break;
    case lclo:
      fprintf(sta, "%5u\tclo %u", i, mem[i + 1]);
      i++;
      break;
    case lfrf:
      fprintf(sta, "%5u\tfrf", i);
      break;
    case limp:
      fprintf(sta, "%5u\timp %u", i, mem[i + 1]);
      i++;
      break;
    case lext:
      fprintf(sta, "%5u\text %u", i, mem[i + 1]);
      i++;
      break;
    case lchk:
      fprintf(sta, "%5u\tchk", i);
      fprintf(prg, "// offset %d %s %d %s %s\n", mem[i + 2], cstack[sp - 1], dstack[sp - 1],
              condstack[con - 1], cmd);
      getvarname(mem[i + 2], name);
      div_strcat(cstack[sp - 1], sizeof(cstack[sp - 1]), ".");
      div_strcat(cstack[sp - 1], sizeof(cstack[sp - 1]), name);
      i += 2;
      fprintf(prg, "// lchk next %d %d\n", i, mem[i + 1]);
      if (mem[i + 1] == lptr || mem[i + 1] == lchk)
        i++;

      if (mem[i + 1] == ladd)
        i++;

      if (mem[i + 1] == lptr || mem[i + 1] == lchk)
        i++;

      div_strcpy(cmd, sizeof(cmd), cstack[sp - 1]);
      fprintf(prg, "// cmd is %s\n", cmd);
      fprintf(prg, "// lchk next %d %d\n", i, mem[i + 1]);

      break;

    case ldbg:
      fprintf(sta, "%5u\tdbg", i);
      break;
    default:
      fprintf(sta, "***");
      break;
    }
  }
  fflush(sta);
  fflush(prg);
  fclose(sta);
  fclose(prg);
  printf("stack: %d\n", sp);
}


#endif
