// ----------------------- DPMI.H --------------------------
// VTAL DPMI routines.
// (C) Copyright 1994-95 by Jare & JCAB of Iguana-VangeliSTeam.

#ifndef _DPMI_H_
#define _DPMI_H_

#include "vtal.h"

#pragma pack(4)
#ifdef __cplusplus
extern "C" {
#endif




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// DPMI interface definitions for DPMI.C.                     |######


// **********************************************
// Useful macros for DPMI pointer conversions.

#define DPMI_PTR_SEG(p)       ((uint16)(((uint32)(p)) >> 4))
#define DPMI_MK_PTR(s,o)      ((void *)(((uint32)(s) << 4) + (uint16)(o)))
#define DPMI_PTR_OFS(p)       (((uint16)(p)) & 0x000F)
#define DPMI_PTR_OFS_SEG(p,s) (((uint32)(p))-((uint32)(s)<<4))


// **********************************************
// Register passing structures.
                                         
typedef struct DPMI_SXrminfo {      // Real mode registers for the DPMI
    uint32 EDI;                       // translation API.
    uint32 ESI;                       // 32 bit registers.
    uint32 EBP;
    uint32 _reserved;
    uint32 EBX;
    uint32 EDX;
    uint32 ECX;
    uint32 EAX;
    uint16  flags;
    uint16  ES, DS, FS, GS;
    uint16  IP, CS, SP, SS;
} DPMI_TXRealModeInfo;

typedef struct DPMI_SWrminfo {      // Real mode registers for the DPMI
    uint16 DI, HDI;                   // translation API.
    uint16 SI, HSI;                   // 16 bit registers.
    uint16 BP, HBP;
    uint32 _reserved;
    uint16 BX, HBX;
    uint16 DX, HDX;
    uint16 CX, HCX;
    uint16 AX, HAX;
    uint16 flags;
    uint16 ES, DS, FS, GS;
    uint16 IP, CS, SP, SS;
} DPMI_TWRealModeInfo;

typedef struct DPMI_SBrminfo {      // Real mode registers for the DPMI
    uint16 DI, HDI;                   // translation API.
    uint16 SI, HSI;                   // 8 bit registers
    uint16 BP, HBP;
    uint32 _reserved;
    uint8 BL, BH, HBL, HBH;
    uint8 DL, DH, HDL, HDH;
    uint8 CL, CH, HCL, HCH;
    uint8 AL, AH, HAL, HAH;
    uint8 flagsl, flagsh;
    uint16 ES, DS, FS, GS;
    uint16 IP, CS, SP, SS;
} DPMI_TBRealModeInfo;

typedef union DPMI_Urminfo {       // Real mode registers for the DPMI
    // translation API.
    DPMI_TBRealModeInfo b;
    DPMI_TWRealModeInfo w;
    DPMI_TXRealModeInfo x;
} DPMI_TRealModeInfo;

PUBLIC DPMI_TRealModeInfo PUBLICDATA DPMI_rmi; // Place real mode register values
                                               // here when calling the next functions.


// **********************************************
// DPMI interface functions.

PUBLIC void       PUBLICFUNC DPMI_RealModeProc(LPmem proc, uint16 seg);
//                           =================
// Calls a real mode procedure.

PUBLIC void       PUBLICFUNC DPMI_RealModeInt(uint intn);
//                           =================
// Calls a real mode interrupt.

PUBLIC void far * PUBLICFUNC DPMI_InterruptFix(void);
//                           =================
// Does some fixups with stack and segments, at the beginning of an
// interrupt handler routine. Returns a far pointer that must be saved.

PUBLIC void       PUBLICFUNC DPMI_InterruptRestore(void far *);
//                           =====================
// Restores the state that was present when DPMI_FixStack was called.

#pragma aux DPMI_InterruptRestore parm [DX EAX]


// **********************************************


// DPMI interface definitions for DPMI.C.                     |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




#ifdef __cplusplus
}
#endif
#pragma pack(4)

#endif
