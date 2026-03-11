// ----------------------- DPMI.C --------------------------
// VTAL DPMI routines.
// (C) Copyright 1994-95 by Jare & JCAB of Iguana-VangeliSTeam.

#include "dpmi.h"

#include <i86.h>
#include <string.h>





// **********************************************
// Variable that contains real mode registers.

DPMI_TRealModeInfo PUBLICDATA DPMI_rmi = {0};


// --------------------------
// Call the DPMI interface.

static void DPMI_RealModeCall(union REGS *rmregs, struct SREGS *rmsregs)
{
    DPMI_rmi.x.SS = DPMI_rmi.x.SP = 0; // DPMI host will provide its own stack.
    rmregs->h.bh  = 0;                 // Do not reset PIC or A20 gate.
    rmregs->w.cx  = 0;                 // 0 words of parameters on the stack.
    rmregs->x.edi = FP_OFF(&DPMI_rmi); // Real mode info.
    rmsregs->es   = FP_SEG(&DPMI_rmi);
    int386x(0x31, rmregs, rmregs, rmsregs);
}


// --------------------------
// Call a real mode interrupt vector.

PUBLIC void PUBLICFUNC DPMI_RealModeInt(uint i)
{
    union  REGS  rmregs;
    struct SREGS rmsregs;

    memset(&rmregs,  0, sizeof(rmregs));
    memset(&rmsregs, 0, sizeof(rmsregs));

    rmregs.w.ax = 0x300;
    rmregs.h.bl = i;        // Vector.
    DPMI_RealModeCall(&rmregs, &rmsregs);
}

PUBLIC void PUBLICFUNC DPMI_RealModeProc(void *proc, uint16 seg) {
/*
    union  REGS  rmregs;
    struct SREGS rmsregs;

    memset(&rmregs,  0, sizeof(rmregs));
    memset(&rmsregs, 0, sizeof(rmsregs));

    rmregs.w.ax = 0x301;
    DPMI_rmi.x.CS = seg;
    DPMI_rmi.x.IP = off;
    DPMI_RealModeCall(&rmregs, &rmsregs);

    ********************************************************
    WARNING: DOS4GW v1.8 bundled with WATCOM C++ 9.5 doesn't
    seem to suuport the above call (at least I didn't get it
    to work), so we used a wierd scheme to emulate it.
    JCAB - 2/94
    DOS4GW 1.95 doesn't work either. What's going on?
    Jare - 3/94
    DOS4GW 1.97 doesn't, but PMODE/W does. :-/ In fact, the
    documentation that comes with DOS4GW doesn't list the
    above function as supported. It'll work if there's a
    DPMI server different from DOS4GW, though.
    JCAB - 2/95
    ********************************************************
*/

#define DPMI_REALVECTORS ((uint32 *)0)   // Real mode vector table.

    uint32 oldvec4 = DPMI_REALVECTORS[4];
    uint32 oldvec6 = DPMI_REALVECTORS[6];
    uint32 oldvec7 = DPMI_REALVECTORS[7];
    uint16  ofs;

    if (seg == 0) {
        seg = DPMI_PTR_SEG(proc);
        ofs = DPMI_PTR_OFS(proc);
    } else
        ofs = DPMI_PTR_OFS_SEG(proc, seg);

       // None of these interrupts should occur during normal operation.
       // but the way to do would be to allocate Real Mode Memory and place
       // the call there. INT 4 still should be used. I hate this!
    DPMI_REALVECTORS[4] = 6*4;  // Address of INT6 vector.
    DPMI_REALVECTORS[6] =  (long)0x9AFB + ((long)ofs<<16);  // STI + CALL FAR
    DPMI_REALVECTORS[7] =  (long)seg +                      // [seg:off]
                          ((long)0xCF<<16);                 // + IRET   CF
    DPMI_RealModeInt(4);

    DPMI_REALVECTORS[4] = oldvec4;
    DPMI_REALVECTORS[6] = oldvec6;
    DPMI_REALVECTORS[7] = oldvec7;
}




