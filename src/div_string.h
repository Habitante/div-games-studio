/**
 * div_string.h - Safe string helpers for DIV Games Studio
 *
 * Drop-in replacements for strcpy/strcat/sprintf that prevent buffer overflows
 * by always NUL-terminating and truncating when needed.
 *
 * Usage:
 *   For local arrays:  DIV_STRCPY(buf, src);  DIV_STRCAT(buf, src);
 *   For pointers:      div_strcpy(ptr, size, src);
 *   For sprintf:       DIV_SPRINTF(buf, fmt, ...);
 */

#ifndef DIV_STRING_H
#define DIV_STRING_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Path separator check - works on both Unix and Windows */
#define IS_PATH_SEP(c) ((c) == '/' || (c) == '\\')

/* Safe string copy - always NUL-terminates, never overflows dest.
 * Returns dest for chaining. */
static inline char *div_strcpy(char *dest, size_t dest_size, const char *src) {
    if (dest_size == 0) return dest;
    if (!src) { dest[0] = '\0'; return dest; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(dest, src, dest_size - 1);
#pragma GCC diagnostic pop
    dest[dest_size - 1] = '\0';
    return dest;
}

/* Safe string concatenation - always NUL-terminates. */
static inline char *div_strcat(char *dest, size_t dest_size, const char *src) {
    size_t len;
    if (dest_size == 0 || !src) return dest;
    len = strlen(dest);
    if (len < dest_size - 1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(dest + len, src, dest_size - len - 1);
#pragma GCC diagnostic pop
        dest[dest_size - 1] = '\0';
    }
    return dest;
}

/* Safe snprintf wrapper - always NUL-terminates.
 * Returns number of characters that would have been written (like snprintf). */
static inline int div_snprintf(char *dest, size_t dest_size, const char *fmt, ...) {
    int r;
    va_list ap;
    if (!dest || dest_size == 0) return -1;
    va_start(ap, fmt);
    r = vsnprintf(dest, dest_size, fmt, ap);
    va_end(ap);
    dest[dest_size - 1] = '\0';
    return r;
}

/* Macro helpers for local arrays - sizeof gives the correct buffer size */
#define DIV_STRCPY(dst, src)   div_strcpy((dst), sizeof(dst), (src))
#define DIV_STRCAT(dst, src)   div_strcat((dst), sizeof(dst), (src))
#define DIV_SPRINTF(dst, ...)  div_snprintf((dst), sizeof(dst), __VA_ARGS__)

#endif /* DIV_STRING_H */
