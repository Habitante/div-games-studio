#include "global.h"
#include "../div_string.h"

#include "osdep.h"
#include <ctype.h>

#include <stdio.h>

#include <fnmatch.h>


#ifdef __EMSCRIPTEN__
#include <string.h>

#else


char *strupr(char *s) {
  char *ucs = (char *)s;
  for (; *ucs != '\0'; ucs++) {
    *ucs = toupper(*s++);
  }
  return ucs;
}

char *_strupr(char *string) {
  int x = 0;
  char *st = string;
  if (string > 0 && strlen(string) > 0) {
    st = (char *)malloc(strlen(string));

    for (x = 0; x < strlen(string); x++)
      st[x] = toupper((unsigned char)string[x]);

    st[x] = 0;
    return st;

  } else
    return " ";
}

char *strlwr(char *s) {
  char *ucs = (char *)s;
  for (; *ucs != '\0'; ucs++) {
    *ucs = tolower(*s++);
  }
  return ucs;
}


char *_strlwr(char *string) {
  int x = 0;
  char *st = string;
  if (strlen(string) > 0) {
    st = (char *)malloc(strlen(string));

    for (x = 0; x < strlen(string); x++)
      st[x] = tolower((unsigned char)string[x]);

    st[x] = 0;
  }
  return st;
}

#endif


void _dos_setdrive(unsigned __drivenum, unsigned *__drives) {
  char c[3];
  c[0] = __drivenum + 'A' - 1;
  c[1] = ':';
  c[2] = 0;
}

char *itoa(long n, char *buf, int len) {
  if (n < 0)
    len++; // room for negative sign '-'
  snprintf(buf, len + 1, "%ld", n);
  return buf;
}

void call(const void_return_type_t func) {
  func();
}


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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


int nummatch = 0;
int np = 0;
int type = 0;

char findmask[255];
char findname[2048];

struct dirent **namelist = NULL;

unsigned int _dos_findfirst(char *name, unsigned int attr, struct find_t *result) {
  unsigned int ret = 0;

  div_strcpy(findmask, sizeof(findmask), name);
  strlwr(findmask);

  if (namelist != NULL) {
    while (++np < nummatch) {
      free(namelist[np]);
    }
    free(namelist);
    namelist = NULL;
  }

  nummatch = scandir(".", &namelist, 0, alphasort);
  np = -1;
  type = attr;

  ret = _dos_findnext(result);

  return (ret);
}
unsigned int _dos_findnext(struct find_t *result) {
  while (++np < nummatch) {
    div_strcpy(result->name, sizeof(result->name), namelist[np]->d_name);
    result->attrib = 0;
    if (result->name[0] != '.' || (result->name[0] == '.' && result->name[1] == '.')) {
      if (namelist[np]->d_type == DT_DIR && type == _A_SUBDIR) {
        // only if searching via wildcard - fixes "new"
        if (strchr(findmask, '*')) {
          free(namelist[np]);
          result->attrib = 16;
          return 0;
        }
      }
      div_strcpy(findname, sizeof(findname), result->name);

      if (fnmatch(findmask, findname, FNM_PATHNAME | FNM_CASEFOLD) == 0) {
        if (namelist[np]->d_type != DT_DIR && type == _A_NORMAL) {
          free(namelist[np]);
          result->attrib = 0;
          return 0;
        }
      }
    }
    if (np < nummatch)
      free(namelist[np]);
  }
  return 1;
}

unsigned int _dos_setfileattr(const char *filename, unsigned int attr) {
  return 1;
}

void __mkdir(char *dir) {
  printf("mkdir %s\n", dir);
}


void set_text_color(int attr, int fg, int bg) {
  char command[13];

  /* Command is the control command to the terminal */
  div_snprintf(command, sizeof(command), "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
  printf("%s", command);
}
