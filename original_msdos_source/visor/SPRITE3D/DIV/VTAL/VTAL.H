// ----------------------- VTAL.H --------------------------
// VT Audio Library definitions.
// WATCOM 10.0 + DOS4GW/PMODEW version.
// (C) Copyright 1994-95 by Juan Carlos Ar‚valo Baeza

#ifndef _VTAL_H_
#define _VTAL_H_

#include "base.h"           // Basic types and defionitions.

#pragma library (vtal);     // #pragma for automatic use of VTAL.LIB
#pragma pack(4)             // Structure packing for VTAL variables.
#ifdef __cplusplus
extern "C" {
#endif




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// General types and definitions.                             |######
// VTAL API definitions.                                      |######


// **********************************************
// Pointer macro definitions.

#define LP             FAR *        // Pointer-to macro for defining types.
#define LPLP           LP LP        // Pointer-to pointer macro.


// **********************************************
// VTAL Function attributes, provided for safety.
// This way, it doesn't matter if the program is compiled
// with stack or register calling conventions.

#pragma aux __VTAL "vtal_*" modify [] parm [EAX EBX ECX EDX ESI EDI];


// **********************************************
// Generic types with a definite meaning.

typedef void  LP       LPmem;       // Pointer to a memory buffer.
typedef LPmem LP       LPLPmem;     // Pointer to a pointer to a memory buffer.
typedef char  LP       LPstring;    // Ascii string buffer.
typedef const char LP  LPconststr;  // Constant ascii string.


// **********************************************
// VTAL handle definitions. VTAL handles
// are VTAL-specific values that identify
// a given resource.

typedef uint32      VTAL_handle;     // Generic handle definition.

#define NULL_handle ((VTAL_handle)0) // Invalid handle used for error conditions.

typedef VTAL_handle FILE_handle;     // Handle that identifies disk file.
typedef VTAL_handle SINS_handle;     // Handle that identifies a given sound or instrument.
typedef VTAL_handle SDEV_handle;     // Handle that identifies a sound device.
typedef VTAL_handle SONG_handle;     // Handle that identifies a song loaded in memory.
typedef VTAL_handle PLAY_handle;     // Handle that identifies a song player.


// **********************************************
// Debugging stuff.

PUBLIC uint32 PUBLICDATA DBUG_Flags;

enum {
    DBUG_PARTITURE  = 0x00000001,
    DBUG_MIX        = 0x00000002,
    DBUG_DMAMIX     = 0x00000004,
    DBUG_DMADUMP    = 0x00000008
};


// **********************************************
// Functions for init-deinitialization of the whole library.

#pragma aux (__VTAL)         VTAL_Init
PUBLIC bool       PUBLICFUNC VTAL_Init        (void);
//                           =========
//  Initializes the tables used for DMA sound output.
// Tables occupy a maximum of 70 Kb of memory.
// Returns FALSE if there was an error.

#pragma aux (__VTAL)         VTAL_Done
PUBLIC void       PUBLICFUNC VTAL_Done        (void);
//                           =========
//  Frees the tables used for DMA sound output.

#pragma aux (__VTAL)         VTAL_GetIdString
PUBLIC LPconststr PUBLICFUNC VTAL_GetIdString (void);
//                           ================
//  Returns an ASCII string identifying the library license.

#pragma aux (__VTAL)         VTAL_GetVerString
PUBLIC LPconststr PUBLICFUNC VTAL_GetVerString(void);
//                           =================
//  Returns an ASCII string identifying library version.


// **********************************************


// General types and definitions.                             |######
// VTAL API definitions.                                      |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Functions defined in XTRN.C. Those are system-dependent    |######
// functions that VTAL needs for its operation. They should   |######
// be modified in XTRN.C if needed.                           |######


// **********************************************
// Memory heap functions.

#pragma aux (__VTAL)          XTRN_GetMem
PUBLIC void        PUBLICFUNC XTRN_GetMem    (LPLPmem ppointer, ulong size);
//                            ===========
//  Function for allocating memory.
//
//      ppointer = pointer to a pointer that will be filled with the address.
//      size     = amount of memory in bytes that must be allocated.
//
//  '*ppointer' must be returned NULL if no memory is available.

#pragma aux (__VTAL)          XTRN_FreeMem
PUBLIC void        PUBLICFUNC XTRN_FreeMem   (LPLPmem ppointer, ulong size);
//                            ============
//  Function for freeing allocated memory.
//
//      ppointer = pointer to a pointer that contains the memory address.
//      size     = amount of memory in bytes that must be freed. This parameter
//                 can be ignored if the memory manager doesn't need it.
//
//  '*ppointer' must be set to NULL.

#pragma aux (__VTAL)          XTRN_ReallocMem
PUBLIC void        PUBLICFUNC XTRN_ReallocMem(LPLPmem ppointer, ulong oldsize, ulong newsize);
//                            ===============
//  Function for resizing already allocated memory.
//
//      ppointer = pointer to a pointer that contains the memory address.
//      oldsize  = amount of memory in bytes that had been allocated. This
//                 parameter can be ignored if the memory manager doesn't
//                 need it.
//      newsize  = new amount of memory in bytes desired for the memory block.
//
//  '*ppointer' must be set to NULL and the block freed if no memory is
//  available for resizing.
//  Otherwise, '*ppointer' will be set to the new address of the memory block.


// **********************************************
// File handling functions.

#pragma aux (__VTAL)          XTRN_FileSize
PUBLIC ulong       PUBLICFUNC XTRN_FileSize  (LPconststr fname);
//                            =============
//  Function that returns the size of a given file.
//
//      fname = file name.
//
//  Returns the size of the gile identified by 'filename'.
//  If the file doesn't exist, it must return a value of 0.

#pragma aux (__VTAL)          XTRN_LoadFile
PUBLIC ulong       PUBLICFUNC XTRN_LoadFile  (LPconststr fname, LPmem buf, ulong size);
//                            =============
//  Function that loads a given file into memory.
//
//      fname = file name.
//      buf   = pointer to the buffer where the file must be loaded.
//      size  = maximum number of bytes that must be loaded. This should
//              be the size of the buffer.
//
//  Returns the number of bytes loaded.
//  If the file doesn't exist or can't be read, it must return a value of 0.

#pragma aux (__VTAL)          XTRN_OpenFile
PUBLIC FILE_handle PUBLICFUNC XTRN_OpenFile  (LPconststr fname, LPconststr mode);
//                            =============
//  Function that opens a given file.
//
//      fname = file name.
//      mode  = stdio-compatible mode string.
//
//  Returns the handle to the file.
//  If the file doesn't exist or can't be opened, it must return 'NULL_handle'.

#pragma aux (__VTAL)          XTRN_CloseFile
PUBLIC void        PUBLICFUNC XTRN_CloseFile (FILE_handle f);
//                            ==============
//  Function that closes a given file opened by 'XTRN_OpenFile'.
//
//      f    = file handle as returned by 'XTRN_OpenFile'.

#pragma aux (__VTAL)          XTRN_ReadFile
PUBLIC ulong       PUBLICFUNC XTRN_ReadFile  (FILE_handle f, LPmem buf, ulong size);
//                            =============
//  Function that reads from a given file into memory.
//
//      f    = file handle as returned by 'XTRN_OpenFile'.
//      buf  = pointer to the buffer where the file must be loaded.
//      size = maximum number of bytes that must be loaded. This should
//             be the size of the buffer.
//
//  Returns the number of bytes loaded.
//  If the file doesn't exist or can't be read, it must return a value = 0.

#pragma aux (__VTAL)          XTRN_FilePos
PUBLIC ulong       PUBLICFUNC XTRN_FilePos  (FILE_handle f);
//                            =============
//  Function that returns the current read/write position of a file.
//
//      f    = file handle as returned by 'XTRN_OpenFile'.
//
//  Returns the current position.
//  If the file doesn't exist or can't be read, it must return a value = 0.

#pragma aux (__VTAL)          XTRN_SeekFile
PUBLIC ulong       PUBLICFUNC XTRN_SeekFile  (FILE_handle f, ulong newpos);
//                            =============
//  Function that sets the current read/write position of a file.
//
//      f      = file handle as returned by 'XTRN_OpenFile'.
//      newpos = new current position.
//
//  Returns the previous current position.
//  If the file doesn't exist or can't be read, it must return a value = 0.


// **********************************************
// CPU stack handling functions.

typedef struct XTRN_SStack {
    uint  Semaphore;
    uint  Size;
    LPmem Buf;
    uint  SaveSS;
    uint  SaveSP;
} XTRN_TStack, LP XTRN_PStack;

typedef void (*XTRN_TStkFunc)(void);

bool XTRN_StackExec(XTRN_PStack stk, XTRN_TStkFunc func);
#pragma aux XTRN_StackExec modify [EAX EDX] parm [ESI] [EBX] value [AL] = \
"               MOV     EAX,[ESI]       "\
"               AND     EAX,EAX         "\
"               JNZ     @@c1            "\
"                XOR    EAX,EAX         "\
"                JMP    @@e             "\
"       @@c1:   XOR     EAX,EAX         "\
"               MOV     [ESI],EAX       "\
"                                       "\
"               MOV     [ESI+12],ESP    "\
"               MOV     [ESI+16],SS     "\
"               MOV     EAX,[ESI+4]     "\
"               ADD     EAX,[ESI+8]     "\
"               MOV     DX,DS           "\
"               MOV     SS,DX           "\
"               MOV     ESP,EAX         "\
"                                       "\
"               PUSH    ES              "\
"               MOV     ES,DX           "\
"               CLD                     "\
"               PUSH    ESI             "\
"               CALL    EBX             "\
"               POP     ESI             "\
"               POP     ES              "\
"                                       "\
"               MOV     SS,[ESI+16]     "\
"               MOV     ESP,[ESI+12]    "\
"               MOV     EAX,1           "\
"               MOV     [ESI],EAX       "\
"       @@e:                            "

#pragma aux (__VTAL)   XTRN_InitStack
PUBLIC bool PUBLICFUNC XTRN_InitStack(XTRN_PStack stk, uint size);
//                     ==============
//  Function that initializes a stack for use by XTRN_StackExec.
// This is intended to provide a nice stack to IRQ handlers.
//
//      stk  = pointer to the stack data structure. The structure must be
//             in global memory (so it can be accessed in a IRQ handler).
//      size = stack size desired. It's allocated through XTRN_GetMem.
//
//  Returns TRUE if the stack was correctly initialized.

#pragma aux (__VTAL)   XTRN_DoneStack
PUBLIC void PUBLICFUNC XTRN_DoneStack(XTRN_PStack stk);
//                     ==============
//  Function that uninitializes and frees a stack initialized
// by XTRN_InitStack.
//
//      stk  = pointer to the initialized stack data structure.


// **********************************************


// Functions defined in XTRN.C. Those are system-dependent    |######
// functions that VTAL needs for its operation. They should   |######
// be modified in XTRN.C if needed.                           |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Definitions global to the VT Audio Library (GSND API).     |######


// **********************************************
// Definition of the values that change for a sound.

typedef struct GSND_SData {

    uint32 Period;      // Period value.
                        // 6B0 hex is standard middle-C note.
                        // Higher values lower frequency.
                        // 0 means no change.

    sint32 Volume;      // 0-511, with 256 as the standard max. value.
                        // 128 is the non-distortion value for 4 channels.
                        // 64  is the non-distortion value for 8 channels.
                        // ... etc.
                        // 256 is choosed as the standard value because it
                        // 'usually' gives a better signal/noise ratio, even
                        // with the saturation distortion.
                        // -1 means no change.

    sint32 Panning;     // Full 8-bit circular range.
                        //      0 = front
                        //     64 = right
                        // +/-128 = rear
                        //    -64 = left
                        // If not using surround, rear = front.
                        // if not using fine panning, values in between
                        // are approximated by those above.
                        // 0x8000 means no change.

    SINS_handle Instrument;  // Handle of the instrument being played.
    uint32      SOffset;     // Offset in the instrument when triggering.
    bool        Trigger;     // TRUE if the instrument must be triggered.

} GSND_TData, LP GSND_PData;

typedef struct GSND_SSndChg {

    uint32 Flags;       // Each bit indicates a different type of operation.

    uint32 Volume;      // Global volume destination.
    uint32 VolTime;     // Number of ticks for volume change.
    uint32 Period;      // Global period destination.
    uint32 PerTime;     // Number of ticks for period change.
    sint32 Panning;     // Panning destination.
    uint32 PanTime;     // Number of ticks for panning change.

} GSND_TSndChg, LP GSND_PSndChg;

enum {      // Values fot GSND_TSndChg.Flags

    GSND_SC_TOVOLUME  = 0x00000001,
    GSND_SC_TOPERIOD  = 0x00000002,
    GSND_SC_TOPANNING = 0x00000004
};


// **********************************************
// Definition of the general parameters for sound change.

typedef struct GSND_SGenData {

    uint32 Volume;      // Global volume. 0-511, with 256 as the standard max. value.
    uint32 Period;      // Global period. > 65536 means larger period (lower frequency).
    sint32 PanWidth;    // Panning width. 256 means full width. Negative inverts.
    sint32 PanOffs;     // Panning offset. This is the panning value for the front.

} GSND_TGenData, LP GSND_PGenData;

typedef struct GSND_SGenChg {

    uint32 Flags;       // Each bit indicates a different type of operation.

    uint32 Volume;      // Global volume destination.
    uint32 VolTime;     // Number of ticks for volume change.
    uint32 Period;      // Global period destination.
    uint32 PerTime;     // Number of ticks for period change.
    sint32 Panning;     // Panning offset destination.
    uint32 PanTime;     // Number of ticks for panning change.

} GSND_TGenChg, LP GSND_PGenChg;

enum {      // Values fot GSND_TGenChg.Flags

    GSND_GC_TOVOLUME  = 0x00000001,
    GSND_GC_TOPERIOD  = 0x00000002,
    GSND_GC_TOPANNING = 0x00000004
};


// **********************************************
// Dynamic sound changing functions.

#pragma aux (__VTAL)   GSND_GenPlay
PUBLIC void PUBLICFUNC GSND_GenPlay(GSND_PGenData gdata, GSND_PGenChg gchg);
//                     ============
//  Updates the global playing raw data according to the info.
//
//      gdata = global data for sounds (master volume, panning offset, etc.).
//      gchg  = changes to global data.

#pragma aux (__VTAL)   GSND_SndPlay
PUBLIC void PUBLICFUNC GSND_SndPlay(GSND_PData raw, GSND_PSndChg schg);
//                     ============
//  Updates the playing raw data according to the info.
//
//      raw   = playing info to be updated.
//      schg  = changes to the sound (ramps and stuff).

#pragma aux (__VTAL)   GSND_SndGlob
PUBLIC void PUBLICFUNC GSND_SndGlob(GSND_PData raw, GSND_PGenData gdata);
//                     ============
//  Updates the playing raw data according to the global data.
// (master volume, panning offset, etc.)
//
//      raw   = playing info to be updated.
//      gdata = global changes to the sound.


// **********************************************


// Definitions global to the VT Audio Library (GSND API).     |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Definitions for the sound effect/Instrument (SINS) API.    |######


// **********************************************
// Sound type identifiers.

enum {

    // Constants for flags.

    SINS_TYP_DIGITAL  = 0x0001,   // Digital sound.
    SINS_TYP_MIDI     = 0x0002    // Midi sound.
};                                                  


// **********************************************
// Sound initialization structure.
                                                    
typedef struct SINS_SLoadRec {

    uint32    Types;

    struct {    // If Types & SINS_TYP_DIGITAL

        uint32      digiAdjNum;     // Period adjustment numerator.
        uint32      digiAdjDen;     // Period adjustment denominator.
        bool        digiStereo;     // TRUE if interleaved stereo samples.
                                    // NOTE!! Interleaved stereo doesn't work yet!
        bool        digiSigned;     // TRUE if signed samples.
        bool        digiBits16;     // TRUE if 16 bit samples.
        bool        digiLEndian;    // TRUE if 16 bit sample is in higher byte first (Motorola).
        LPconststr  digiFileName;   // Load from a named file.
        FILE_handle digiFile;       // Load from an open file.
        LPmem       digiPtr;        // Load from a memory position.
                                    // One of the previous 3 must not be NULL.
                                    // If they are all NULL, continuous play
                                    // is selected and buffers must be provided
                                    // periodically.
        uint32      digiLoopStart;  // Start  of the loop (in samples).
        uint32      digiLoopLen;    // Length of the loop (in samples). 0 = no loop.
        uint32      digiSize;       // Size of the sound in samples.
    };
    struct {    // If Types & SINS_TYP_MIDI

        sint8       midiPatchNum;   // Patch number. -1 if GMIDI-percussion-like.
        sint8       midiNoteNum;    // Note number. != -1 if GMIDI-percussion-like.
        sint8       midiChannelNum; // Channel number. -1 if no special channel.
    };

} SINS_TLoadRec, LP SINS_PLoadRec;
                        

// **********************************************
// Sound handling functions.

#pragma aux (__VTAL)          SINS_InitSound
PUBLIC SINS_handle PUBLICFUNC SINS_InitSound(SINS_PLoadRec lrec);
//                            ==============
//  Loads and initializes a given sound.
//
//      lrec = structure that keeps the various initialization values.
//
//  Returns the pointer to the sound.
//  If the sound couldn't be initialized, returns NULL_handle.

#pragma aux (__VTAL)          SINS_DoneSound
PUBLIC void        PUBLICFUNC SINS_DoneSound(SINS_handle   sound);
//                            ==============
//  Uninitializes and unloads a given sound.
//
//      sound = initialized sound handle, as returned by SINS_InitSound.

#pragma aux (__VTAL)          SINS_Play
PUBLIC void        PUBLICFUNC SINS_Play     (SINS_handle   sound, GSND_PData raw);
//                            =========
//  Updates the playing raw data according to the sound information.
//
//      sound = initialized sound handle, as returned by SINS_InitSound.
//      raw   = playing info to be updated.

#pragma aux (__VTAL)          SINS_AdjPeriod
PUBLIC uint32 PUBLICFUNC      SINS_AdjPeriod(SINS_handle   sound, uint32 period);
//                            ==============
//  Does the necessary adjustments to the period value according to the
// instrument's data.
//
//      sound  = initialized sound handle, as returned by SINS_InitSound.
//      period = a period as used in the SDEV API.
//
// Returns the adjusted period.


// **********************************************


// Definitions for the sound effect/Instrument (SINS) API.    |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Definitions for the sound device (SDEV) API.               |######


// **********************************************
// Sound device configuration.

typedef union {

    struct {
        sint16 port1;       // Hardware base I/O port.
        sint8  irq1;        // Main IRQ channel.
        sint8  dma1;        // Main DMA channel.

        sint16 port2;       // Additional hardware values.
        sint8  irq2;
        sint8  dma2;

        sint16 port3;
        sint8  irq3;
        sint8  dma3;

        sint16 port4;
        sint8  irq4;
        sint8  dma4;

        uint16 tickrate;    // Time quantums per second * 256.
        sint16 rate;        // Desired samplerate in Hertz (if applicable).
        sint16 maxchans;    // Maximum number of channels to be used.
        bool   stereo;      // Stereo output is wanted if available.
        bool   bits16;      // 16 bit output is wanted if available.
        bool   panning;     // Fine panning  is wanted if available.
        bool   surround;    // Surround      is wanted if available.
    };
    uint8 fill[32]; // Structure must occupy 32 bytes.

} SDEV_TConfig, LP SDEV_PConfig;


// **********************************************
// Device type identifiers.

enum {
    DEV_TYP_NONE       = 0, // Not a device.
    DEV_TYP_DMA        = 1, // DMA-type sound device, like Sound Blaster.
    DEV_TYP_WAVETABLE  = 2, // Wavetable-type sound device like Gravis Ultrasound.
    DEV_TYP_MIDI       = 3  // MIDI-type sound device like Roland-MPU.
};


// **********************************************
// Sound device info structure.

typedef struct {

    LPstring FName;            // File name loaded.
    LPstring IdString;         // ID string of the device.
    LPstring Name;             // Full name of the device.
    LPstring TypeId;           // Type of device.
    LPstring VersionType;      // Kind of version (debug, normal, etc.)
    LPstring Version;          // Version: X.YZ
    LPstring Copyright;        // Copyright string.

    uint     Type;             // Device type number.
    uint     SoundTypes;       // SINS types allowed for the device.
    uint     RealTickRate;     // Real (used) ticks per second * 256.
    uint     NumChannels;      // Number of channels that will be used.
    LPmem    DumpBuffer;       // Data buffer pointer.
    uint     DumpBufferSize;   // Data buffer size.
    uint     DumpBufferItem;   // Size of a data buffer for one tick.
    uint     DumpBufferLength; // Number of ticks in the data buffer.
    bool     Stereo;           // TRUE if output is stereo.
    bool     Bits16;           // TRUE if output is 16 bits.
    bool     Panning;          // TRUE if output uses fine panning.
    bool     Surround;         // TRUE if output uses surround.

    GSND_PData     SoundData;  // Array of sound data for all channels.
    GSND_PSndChg   SoundChg;   // Array of sound changing values.

} SDEV_TInfo, LP SDEV_PInfo;


// **********************************************
// Buffer for buffered sound.

typedef struct SDEV_SBuffer SDEV_TBuffer, LP SDEV_PBuffer;
typedef struct SDEV_SBuffer {

    LPmem    BufData;   // Pointer to the sample data.
    uint     BufSize;   // Number of samples in the data.

};


// **********************************************
// Sound device handling functions.

#pragma aux (__VTAL)          SDEV_GetInfo
PUBLIC SDEV_PInfo  PUBLICFUNC SDEV_GetInfo         (SDEV_handle rec);
//                            ============
//  Obtains information about a loaded device.
//
//      rec = sound device handle.
//
//  Returns the pointer to the info.
//  If the device isn't opened, returns NULL.

#pragma aux (__VTAL)          SDEV_Load
PUBLIC SDEV_handle PUBLICFUNC SDEV_Load            (LPconststr fname, SDEV_PConfig config);
//                            =========
//  Loads and initializes a given sound device.
//
//      fname  = sound device filename.
//      config = desired configuration for the device.
//
//  Returns the handle of the device.
//  If the device can't be loaded or initialized, returns NULL_handle.

#pragma aux (__VTAL)          SDEV_Unload
PUBLIC void        PUBLICFUNC SDEV_Unload          (SDEV_handle handle);
//                            ===========
//  Uninitializes and unloads a loaded sound device.
//
//      handle = sound device handle.

#pragma aux (__VTAL)          SDEV_UnloadAll
PUBLIC void        PUBLICFUNC SDEV_UnloadAll       (void);
//                            ==============
//  Uninitializes and unloads all loaded sound devices.
// Useful for clean-exiting in critical error conditions.

#pragma aux (__VTAL)          SDEV_InitSound
PUBLIC bool        PUBLICFUNC SDEV_InitSound       (SDEV_handle handle, SINS_handle sound);
//                            ==============
//  Initializes a sound or instrument for a loaded sound device.
//
//      handle = sound device handle.
//      sound  = sound handle.
//
//  Returns TRUE if the sound initialized correctly.

#pragma aux (__VTAL)          SDEV_DoneSound
PUBLIC void        PUBLICFUNC SDEV_DoneSound       (SDEV_handle handle, SINS_handle sound);
//                            ==============
//  Uninitializes and unloads a sound loaded by 'SDEV_LoadInstrument'.
//
//      handle = sound device handle.
//      sound  = sound handle.

#pragma aux (__VTAL)          SDEV_DoRawChannel
PUBLIC void        PUBLICFUNC SDEV_DoRawChannel    (SDEV_handle _rec, uint ch, GSND_PData raw);
//                            ==============
//  Outputs a channel of the device with the specified values.
//
//      handle = sound device handle.
//      ch     = channel (0-based).
//      raw    = new sound data.

#pragma aux (__VTAL)          SDEV_DoChannel
PUBLIC void        PUBLICFUNC SDEV_DoChannel       (SDEV_handle _rec, uint ch, GSND_PData raw, GSND_PGenData gdata);
//                            ==============
//  Changes and outputs a channel of the device with new values.
//
//      handle = sound device handle.
//      ch     = channel (0-based).
//      raw    = new sound data.
//      gdata  = dynamic change info.

#pragma aux (__VTAL)          SDEV_ChannelFree
PUBLIC bool        PUBLICFUNC SDEV_ChannelFree     (SDEV_handle handle, uint ch);
//                            ================
//  Returns the active state of a given channel.
//
//      handle = sound device handle.
//      ch     = channel (0-based).
//
//  Returns TRUE if the channel is not in use (playing).

#pragma aux (__VTAL)          SDEV_GetSetMixMargin
PUBLIC uint        PUBLICFUNC SDEV_GetSetMixMargin (SDEV_handle _rec, uint margin);
//                            ==========
//  Sets the minimum number of ticks available to be mixed, necessary
// before actual mixing is done by SED_DoMix. This is used to render
// smoother mixing (see GAMEVTAL.C for an example of usage).
//
//      handle = sound device handle.
//      margin = number of ticks.
//
//  Returns the previous margin.
                                              
#pragma aux (__VTAL)          SDEV_DoMix
PUBLIC bool        PUBLICFUNC SDEV_DoMix           (SDEV_handle handle);
//                            ==========
//  Advances the sound device to the next tick.
//
//      handle = sound device handle.
//
//  Returns TRUE if the tick has been advanced.
//  Returns FALSE if the device is not yet ready to advance a tick.
                                              
#pragma aux (__VTAL)          SDEV_BufferAvailable
PUBLIC bool        PUBLICFUNC SDEV_BufferAvailable (SDEV_handle handle, uint ch);
//                            ====================
//  Checks the specified channel (which must be initialized as buffered play)
// to see if a new buffer will be accepted.
//
//      handle = sound device handle.
//      ch     = channel.
//
//  Returns TRUE  if a new buffer will be accepted.
//  Returns FALSE if no buffer will be accepted yet.
                                              
#pragma aux (__VTAL)          SDEV_NewBuffer
PUBLIC bool        PUBLICFUNC SDEV_NewBuffer       (SDEV_handle handle, uint ch, SDEV_PBuffer buf);
//                            ==============
//  Gives the specified channel (which must be initialized as buffered play)
// a new buffer of samples.
//
//      handle = sound device handle.
//      ch     = channel.
//      buf    = buffer definition.
//
//  Returns TRUE  if the buffer was accepted.
//  Returns FALSE if no buffer will be accepted yet.
                                              
                              
// **********************************************


// Definitions for the sound device (SDEV) API.               |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Definitions for the song (SONG) API.                       |######


// **********************************************
// Song data types.

typedef struct SONG_SMiscInfo {

    char    Name[80];      // Name of the song.
    char    FileName[80];  // Filename the song was loaded from.
    char LP Comments;      // Freestyle comments to the song.

} SONG_TMiscInfo, LP SONG_PMiscInfo;

typedef struct {

    sint32 Types;       // Types of instruments allowed in the channel.
    sint32 Panning;     // Initial panning value.

} SONG_TChannelInfo, LP SONG_PChannelInfo;

typedef struct {

    uint16            SongStart;        // Song's first position (pattern).
    uint16            SequenceRepStart; // Song's restart position when looping.
    uint16            SequenceLength;   // Number of patterns in the song.
    uint16            NumChannels;      // Total number of channels (maximum).
    uint16            NumInstruments;   // Total number of instruments (counting empty ones).
    uint16            NumPatterns;      // Total number of patterns (including unused ones).
    uint8             FirstTick;        // TRUE if the song plays effects in every tick.
    uint8             InitialTempo;     // Initial value for the tempo effect.
    uint16            InitialBPM;       // Initial value for the BPM effect.
    SONG_PChannelInfo ChannelInfo;      // Array of info values for all the channels.
    SONG_PMiscInfo    Misc;             // Info not necessary for playing.

} SONG_TInfo, LP SONG_PInfo;

enum {
    SNDP_SFTK_VOLSLIDE    = 0x01,
    SNDP_SFTK_TONESLIDE   = 0x02,
    SNDP_SFTK_SLIDETONOTE = 0x04
};


// **********************************************
// Song loading functions.

#pragma aux (__VTAL)          SONG_Load
PUBLIC SONG_handle PUBLICFUNC SONG_Load          (LPconststr fname, int instrtypes);
//                            =========
//  Loads a song prepared to play in the specified device.
//
//      fname      = Song's file name.
//      instrtypes = Set of flags for each type of instrument/sound..
//
//  Returns the handle of the loaded song. NULL_handle if error.

#pragma aux (__VTAL)          SONG_Unload
PUBLIC void        PUBLICFUNC SONG_Unload        (SONG_handle song);
//                            ===========
//  Unloads a previously loaded song.
//
//      song = Song handle.

#pragma aux (__VTAL)          SONG_GetInfo
PUBLIC SONG_PInfo  PUBLICFUNC SONG_GetInfo       (SONG_handle song);
//                            ============
//  Returns a pointer to a SONG_Info of a previously loaded song.
//
//      song = Song handle.

#pragma aux (__VTAL)          SONG_New
PUBLIC SONG_handle PUBLICFUNC SONG_New           (void);
//                            ========
//  Initializes a new song in memory. It's previously empty:
// 0 channels, 0 instruments, 0 patterns.
//
//  Returns the handle to the song.

#pragma aux (__VTAL)          SONG_ChangeNumChannels
PUBLIC void        PUBLICFUNC SONG_ChangeNumChannels(SONG_handle song, uint c);
//                            ========
//  Changes the number of channels of the song. When decreasing,
// channel info is lost. When increasing, new channel info is set
// to defaults.
//
//      song = Song handle.
//      c    = New number of channels.

#pragma aux (__VTAL)          SONG_BindToDevice
PUBLIC void        PUBLICFUNC SONG_BindToDevice  (SONG_handle song, SDEV_handle dev);
//                            =================
//  Initializes the instruments of the song for playing through the given
// sound device.
//
//      song = Song handle.
//      dev  = Sound device.
                            
#pragma aux (__VTAL)          SONG_UnbindToDevice
PUBLIC void        PUBLICFUNC SONG_UnbindToDevice(SONG_handle song, SDEV_handle dev);
//                            ===================
//  Frees the instruments of the song for playing through the given
// sound device.
//
//      song = Song handle.
//      dev  = Sound device. If NULL, use all unbind all sound devices.


// **********************************************


// Definitions for the song (SONG) API.                       |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




// ##############################################################
// ################################################################
// #################################################################
// #################################################################
// -----------------------------------------------------------+######
// Definitions for the song interpreter (PLAY) API.           |######


// **********************************************
// Player init structure.

typedef struct {

    bool   LoopMod;           // If TRUE, then the MOD will loop if it was so defined.
    bool   ForceLoopMod;      // TRUE if music must be played forever.
    bool   PermitFilterChange;// TRUE if the partiture is allowed to change the filter. (unused)
    uint16 FirstPattern;      // Position, etc. where the user wants the music to start.
    uint16 RepStart;          //     (Non-zero if the value must be used)
    uint16 SongLen;           //        "
    uint16 RealTimerVal;      // Timer rate in Hz*256, must be as accurate as possible.

} PLAY_TInitData, LP PLAY_PInitData;


// **********************************************

typedef struct {

    // Values initialized from a PLAY_TInitData structure.

    bool   LoopMod;            // If TRUE, then the MOD will loop if it was so defined.
    bool   PermitFilterChange; // TRUE if the partiture is allowed to change the filter. (unused)
    uint16 FirstPattern;       // Initial song position.
    uint16 RepStart;           // Repeating song position.
    uint16 SongLen;            // Total number of patterns in the song.
    uint16 RealTimerVal;       // Timer rate in Hz*256, must be as accurate as possible.
    
    // Temporary (changing) values.

    uint16 TickCount;       // Ticks counter. Increments each tick.
    uint16 BPMDivider;      // Fine speed adjust values.
    uint16 BPMCount;        //             "
    uint16 NextNote;        // Next note in the pattern.
    uint16 NextSeq;         // Next pattern index (for the next note).
                            // They both must have been set BEFORE calling this UNIT.
    uint8  NotePlaying;     // Index of the note inside the pattern.
    uint8  SeqPlaying;      // Sequence number of the pattern.
    uint8  TempoCt;         // Number of the actual tick. Not changed in this UNIT.
    uint8  PDelay;          // Times to repeat this note.
    uint8  FirstPDelay;     // Initial value of the above.
    bool   InPDelay;        // True if not in the first pass of a PDelay thing.
       
    // Effect (changing) values common to all the channels.

    bool   Playing;         // (Read only) TRUE if the music is sounding right now.
    bool   FilterIsOn;      // Position of the filter (FALSE = OFF).
    uint8  Tempo;           // Number of ticks in the current note.
    uint16 BPMIncrement;    // BPM value defined in the song.

    // Other info.

    GSND_PData RawData;  // Pointer to the playing data
    SONG_PInfo    Song;     // Copy of the song definition values.

} PLAY_TInfo, LP PLAY_PInfo;


// **********************************************
// Song playing functions.

#pragma aux (__VTAL)          PLAY_InitSong
PUBLIC PLAY_handle PUBLICFUNC PLAY_InitSong   (SONG_handle sh, PLAY_PInitData id);
//                            =============
//  Initializes a previously loaded song for playing.
//
//      sh = Song handle.
//      id = Initialization info.
//
// Returns the handle of the player. NULL_handle if error.

#pragma aux (__VTAL)          PLAY_DoneSong
PUBLIC void        PUBLICFUNC PLAY_DoneSong   (PLAY_handle ph);
//                            =============
//  Uninitializes a previously initialized song for playing.
//
//      ph = Player handle.

#pragma aux (__VTAL)          PLAY_GetInfo
PUBLIC PLAY_PInfo  PUBLICFUNC PLAY_GetInfo    (PLAY_handle ph);
//                            ============
//  Returns a pointer to a PLAY_TInfo of a previously initialized song.
//
//      ph = Song handle.

#pragma aux (__VTAL)          PLAY_DoPartiture
PUBLIC bool        PUBLICFUNC PLAY_DoPartiture(PLAY_handle ph);
//                            ================
//  Runs one tick of a song.
//
//      ph = Song handle.
//
// Returns TRUE if a new note has been processed.
// FALSE if it was only a tick.


// **********************************************
                             

// Definitions for the song interpreter (PLAY) API.           |######
// -----------------------------------------------------------+######
// #################################################################
// #################################################################
// ################################################################
// ##############################################################




#ifdef __cplusplus
}
#endif
#pragma pack()      // Restore default structure packing value.

#endif       
