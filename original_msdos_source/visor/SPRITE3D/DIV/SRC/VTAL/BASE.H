// ----------------------- BASE.H --------------------------
// Basic data types and macros. WATCOM 10.0 + DOS4GW/PMODEW version.
// (C) Copyright 1994-95 by Juan Carlos Ar‚valo Baeza

#ifndef _BASE_H_
#define _BASE_H_

#pragma pack(4)
#ifdef __cplusplus
extern "C" {
#endif




// ********************************************************
// --------------------------------------------------------
// Basic macro definitions. Those can be system-dependent.

#define PUBLIC     extern       // Defines an extern variable or function.
#define PUBLICFUNC __stdcall    // Defines a library function.
#define PUBLICDATA __stdcall    // Defines a library variable.
#define FAR                     // Extent of pointers used in the library.




// ********************************************************
// --------------------------------------------------------
// Basic type definitions. Those can be system-dependent.

// --------------------------
// Machine-dependent types.

typedef unsigned char  uint8;   // Unsigned  8-bit value.
typedef   signed char  sint8;   //   Signed  8-bit value.
typedef unsigned short uint16;  // Unsigned 16-bit value.
typedef   signed short sint16;  //   Signed 16-bit value.
typedef unsigned long  uint32;  // Unsigned 32-bit value.
typedef   signed long  sint32;  //   Signed 32-bit value.

// --------------------------
// Boolean values.

enum {
    TRUE  = 1,
    FALSE = 0
};

// --------------------------
// Machine-easytouse types.

typedef   signed int  sint;     // Signed integer.
typedef unsigned int  uint;     // Unsigned integer.
typedef   signed long slong;    // Signed long integer.
typedef unsigned long ulong;    // Unsigned long integer.

typedef uint8         bool;     // Boolean type is defined as 8-bits.




#ifdef __cplusplus
}
#endif
#pragma pack()

#endif       
