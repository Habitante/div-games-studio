#include "osdep.h"
#include <ctype.h>

#include <stdio.h>

#include "global.h"
#include "../div_string.h"

char *tmpnames[255];

int tmpcount = 0;

FILE *fmemopen(void *buf, size_t size, const char *opentype) {
  FILE *f;
#ifdef WIN32
  char *tmp_fname = _tempnam("%TMP%", "fmemopen");
  if (tmp_fname == NULL) {
    fprintf(stderr, "fmemopen: _tempnam failed\n");
    return NULL;
  }
  if (tmpcount < 255) {
    tmpnames[tmpcount] = tmp_fname;
    tmpcount++;
  }

  f = fopen(tmp_fname, "wb");
  if (f == NULL) {
    fprintf(stderr, "fmemopen: failed to create temp file: %s\n", tmp_fname);
    free(tmp_fname);
    return NULL;
  }
  fwrite(buf, 1, size, f);
  fclose(f);
  f = fopen(tmp_fname, "rb");
#else
  f = tmpfile();
  if (f == NULL)
    return NULL;
  fwrite(buf, 1, size, f);
  rewind(f);
#endif
  return f;
}

void closefiles(void) {
  while (tmpcount != 0) {
    tmpcount--;
    remove(tmpnames[tmpcount]);
  }
}

#ifdef __llvm___
#include <string.h>

#else

char *strupr(char *s) {
  char *ucs = (char *)s;
  for (; *ucs != '\0'; ucs++) {
    *ucs = toupper(*s++);
  }
  return ucs;
}

char *strlwr(char *s) {
  char *ucs = (char *)s;
  for (; *ucs != '\0'; ucs++) {
    *ucs = tolower(*s++);
  }
  return ucs;
}


#endif


void _dos_setdrive(unsigned __drivenum, unsigned *__drives) {
  char c[3];
  c[0] = __drivenum + 'A' - 1;
  c[1] = ':';
  c[2] = 0;
  chdir(c);
}


#ifdef NOYET
char *itoa(long n, char *buf, int len) {
  if (n < 0)
    len++; // room for negative sign '-'
  snprintf(buf, len + 1, "%ld", n);
  return buf;
}
#endif
void call(const void_return_type_t func) {
  func();
}


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if !defined(__MINGW32__)
// Abstract:   split a path into its parts
// Parameters: Path: Object to split
//             Drive: Logical drive , only for compatibility , not considered
//             Directory: Directory part of path
//             Filename: File part of path
//             Extension: Extension part of path (includes the leading point)
// Returns:    Directory Filename and Extension are changed
// Comment:    Note that the concept of an extension is not available in Linux,
//             nevertheless it is considered

void _splitpath(const char *path, char *drive, char *directory, char *fname, char *extension) {
  char *copy_of_path = (char *)path;
  int counter = 0;
  int last = 0;
  int rest = 0;

  // no drives available in linux .
  // extensions are not common in linux
  // but considered anyway
  drive = NULL;

  while (*copy_of_path != '\0') {
    // search for the last slash
    while (*copy_of_path != '/' && *copy_of_path != '\0') {
      copy_of_path++;
      counter++;
    }
    if (*copy_of_path == '/') {
      copy_of_path++;
      counter++;
      last = counter;
    } else
      rest = counter - last;
  }
  // directory is the first part of the path until the
  // last slash appears
  strncpy(directory, path, last);
  // strncpy doesnt add a '\0'
  directory[last] = '\0';
  // fname is the part behind the last slahs
  div_strcpy(fname, _MAX_FNAME + 1, copy_of_path -= rest);
  // get extension if there is any
  char *ext = extension;
  while (*fname != '\0') {
    // the part behind the point is called extension in windows systems
    // at least that is what i thought apperantly the '.' is used as part
    // of the extension too .
    if (*fname == '.') {
      *fname = '\0';
      *fname++;
      *extension = '.';
      *extension++;
      while (*fname != '\0') {
        *extension = *fname;
        extension++;
        fname++;
      }
    }
    if (*fname != '\0') {
      fname++;
    }
  }
  *extension = '\0';
  extension = ext;

  return;
}

// Abstract:   Make a path out of its parts
// Parameters: Path: Object to be made
//             Drive: Logical drive , only for compatibility , not considered
//             Directory: Directory part of path
//             Filename: File part of path
//             Extension: Extension part of path (includes the leading point)
// Returns:    Path is changed
// Comment:    Note that the concept of an extension is not available in Linux,
//             nevertheless it is considered

void _makepath(char *path, const char *drive, const char *directory, const char *file,
               const char *extension) {
  while (*drive != '\0' && drive != NULL) {
    *path = *drive;
    path++;
    drive++;
  }
  while (*directory != '\0' && directory != NULL) {
    *path = *directory;
    path++;
    directory++;
  }
  while (*file != '\0' && file != NULL) {
    *path = *file;
    path++;
    file++;
  }
  while (*extension != '\0' && extension != NULL) {
    *path = *extension;
    path++;
    extension++;
  }
  *path = '\0';
  return;
}

// Abstract:   Change the current working directory
// Parameters: Directory: The Directory which should be the working directory.
// Returns:    0 for success , other for error
// Comment:    The command doesnt fork() , thus the directory is changed for
//             The actual process and not for a forked one which would be true
//             for system("cd DIR");

int _chdir(const char *directory) {
  if (directory != NULL && strlen(directory) > 0)
    chdir(directory);

  return 0;
}

char *_fullpath(char *_FullPath, const char *_Path, size_t _SizeInBytes) {
  if (realpath(_Path, _FullPath) == NULL)
    div_strcpy(_FullPath, _SizeInBytes, _Path);
  return _FullPath;
}
#endif


int nummatch = 0;
int np = 0;
int type = 0;

char findmask[255];
char findname[2048];

struct dirent **namelist = NULL;

long hFile;
unsigned int _dos_findfirst(char *name, unsigned int attr, struct find_t *result) {
  int ret = 0;

  div_strcpy(findname, sizeof(findname), name);
  type = attr;

  div_strcpy(findmask, sizeof(findmask), name);
  strlwr(findmask);

#if defined(__MINGW64__)
  {
    struct _finddata64i32_t first_result;
    hFile = _findfirst("*.*", &first_result);
  }
#else
  {
    struct _finddata32_t first_result;
    hFile = _findfirst("*.*", &first_result);
  }
#endif

  ret = _dos_findnext(result);

  return (ret);
}
unsigned int _dos_findnext(struct find_t *result) {
  char fname[255];
  int i = 0;
  int j = 0;
  int match = 0;

#if defined(__MINGW64__)
  struct _finddata64i32_t result2;
#else
  struct _finddata32_t result2;
#endif

  int n = 0;

  while ((n = _findnext(hFile, &result2)) == 0) {
    div_strcpy(result->name, sizeof(result->name), result2.name);

    result->attrib = result2.attrib;
    result->size = result2.size;

    if ((type == _A_NORMAL && result2.attrib != _A_SUBDIR) ||
        (type == _A_SUBDIR && (result2.attrib & _A_SUBDIR))) {
      if (type == _A_SUBDIR) {
        if (!strcmp(findmask, "*.*")) // match everything
          return 0;

        if (!strcmp(findmask, result->name)) // match specific
          return 0;
      }
      j = 0;
      div_strcpy(findname, sizeof(findname), result->name);
      strlwr(findname);

      match = 0;

      if (!strcmp(findmask, "*"))
        return 0;

      if (!strcmp(findmask, "*.*")) {
        for (i = 1; i < strlen(findname); i++) {
          if (findname[i] == '.') {
            match = 1;
          }
        }

        if (match == 1)
          return 0;
      }

      if (strchr(findmask, '*') != NULL) {
        if (!strcmp(&findname[strlen(findname) - 3], &findmask[2])) {
          return 0;
        }

      } else {
        if (!strcmp(findname, findmask))
          return 0;
      }
    }
  }
  return 1;
}

unsigned int _dos_setfileattr(const char *filename, unsigned int attr) {
  //		printf("TODO - setfileattr\n");

  return 1;
}

void __mkdir(char *dir) {
  printf("mkdir %s\n", dir);
}


void text_color(int attr, int fg, int bg) {
  char command[40];

  /* Command is the control command to the terminal */
  snprintf(command, sizeof(command), "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
  printf("%s", command);
}
