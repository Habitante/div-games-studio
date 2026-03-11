
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Pruebas con la interrupciขn de teclado
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#define uchar unsigned char
#define byte unsigned char
#define ushort unsigned short
#define word unsigned short
#define ulong unsigned int
#define dword unsigned int

#define swap(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}

// System Keys
#define _ESC            0x01
#define _BACKSPACE      0x0E
#define _TAB            0x0F

#define _ENTER          0x1C
#define _L_CTRL         0x1D
#define _R_CTRL         0x5A
#define _L_SHIFT        0x2A
#define _R_SHIFT        0x36
#define _L_ALT          0x38
#define _R_ALT          0x5C

#define _SPC            0x39
#define _CAPS_LOCK      0x3A
#define _NUM_LOCK       0x45
#define _PRNT_SCRN      0x47
#define _SCROLL_LOCK    0x57

// Function Keys
#define _F1             0x3b
#define _F2             0x3c
#define _F3             0x3d
#define _F4             0x3e
#define _F5             0x3f
#define _F6             0x40
#define _F7             0x41
#define _F8             0x42
#define _F9             0x43
#define _F10            0x44
#define _F11            0x57
#define _F12            0x58

// Directional Control Keys
#define _HOME           0x47
#define _UP             0x48
#define _PGUP           0x49
#define _LEFT           0x4b
#define _CENTER         0x4c
#define _RIGHT          0x4d
#define _END            0x4f
#define _DOWN           0x50
#define _PGDN           0x51
#define _INS            0x52
#define _DEL            0x53

// Cluster Directional Control Keys
#define _C_ENTER        0x59
#define _C_HOME         0x5d
#define _C_UP           0x5e
#define _C_PGUP         0x5f
#define _C_LEFT         0x60
#define _C_RIGHT        0x61
#define _C_END          0x62
#define _C_DOWN         0x63
#define _C_PGDN         0x64
#define _C_INS          0x65
#define _C_DEL          0x66

// Alphanumerics
#define _1              0x02
#define _2              0x03
#define _3              0x04
#define _4              0x05
#define _5              0x06
#define _6              0x07
#define _7              0x08
#define _8              0x09
#define _9              0x0A
#define _0              0x0B

#define _Q              0x10
#define _W              0x11
#define _O              0x18
#define _P              0x19
#define _A              0x1e
#define _S              0x1f
#define _D              0x20
#define _H              0x23
#define _Z              0x2c
#define _X              0x2d
#define _C              0x2e
#define _B              0x30
#define _M              0x32

//ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
//      Tabla de scan codes pulsados
//ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ

char shift_status,ascii,scan_code;
byte kbdFLAGS[128];

#define key(x) kbdFLAGS[x]

void kbdInit(void);
void kbdReset(void);
void tecla(void);
void vacia_buffer(void);

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// IRQ Data
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

typedef void (__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;

byte * shift = (byte *) 0x417; // Shift status

byte buf[64*3]; // {ascii,scan_code,shift_status}
int ibuf=0; // Puntero al buffer, inicio de la cola
int fbuf=0; // Puntero al buffer, fin de la cola

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// Interrupt handler related functions.
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

int ctrl_c=0;
word * kb_start = (void*) 0x41a;
word * kb_end = (void*) 0x41c;

void __far __interrupt __loadds IrqHandler(void)
{
  int n;
  int scancode;

  scancode=inp(0x60);

  if (((*shift)&4) && scancode==46) {
    kbdFLAGS[46]=1; ctrl_c=1;
    outp(0x20,0x20);
  } else {

    if (scancode<128) {
      kbdFLAGS[scancode]=1;
    } else if (scancode>129) {
      kbdFLAGS[scancode-128]=0;
    } else {
      n=128; do { kbdFLAGS[--n]=0; } while (n);
    }

//    if (kbhit()) getch();

    OldIrqHandler();

    if ((*kb_end-28)%32+30==*kb_start) *kb_start=(*kb_start-28)%32+30;

  }
}

void __far __interrupt __loadds Irq23(void){}
void __far __interrupt __loadds Irq1b(void){}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// Obtains the address of an IRQ handler.
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

TIRQHandler GetIRQVector(int n)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x3500 + n;   // DOS4GW redefines the DOS get vector call.
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (TIRQHandler)(MK_FP((word)sregs.es, outregs.x.ebx));
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// Sets the address of an IRQ handler.
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void SetIRQVector(int n, TIRQHandler vec)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x2500 + n;   // DOS set vector call.
    inregs.x.edx = FP_OFF (vec);
    sregs.ds     = FP_SEG (vec);     // Handler pointer.
    sregs.es     = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// Function to initialize the handler.
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void kbdInit(void)
{
    if (GetIRQVector(9) != IrqHandler) {   // If not already installed.
      OldIrqHandler=GetIRQVector(9);       // Get old handler.
      SetIRQVector(9,IrqHandler);          // Set our handler.
      OldIrq1b=GetIRQVector(0x1b);         // Get old handler.
      SetIRQVector(0x1b,Irq1b);            // Set our handler.
      OldIrq23=GetIRQVector(0x23);         // Get old handler.
      SetIRQVector(0x23,Irq23);            // Set our handler.
    }

    signal(SIGBREAK,SIG_IGN);
    signal(SIGINT,SIG_IGN);

}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// Function to uninitialize the handler.
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void kbdReset(void)
{
    if (GetIRQVector(9) == IrqHandler) {   // If it was installed.
      SetIRQVector(9,OldIrqHandler);       // Uninstall it.
      SetIRQVector(0x1b,OldIrq1b);         // Uninstall it.
      SetIRQVector(0x23,OldIrq23);         // Uninstall it.
    }
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Espera una tecla, actualiza variables (ascii,scan_code y shift_status)
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void tecla_bios(void) {
  int as,sc,ss;
  union REGS r;
  struct SREGS s;

  if (_bios_keybrd(_KEYBRD_READY)) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    do {
      r.h.ah=0; int386x(0x16,&r,&r,&s);
      as=r.h.al; sc=r.h.ah;
      r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
      if (as>='0' && as<='9') {
        kbdFLAGS[0x4B]=0;
        kbdFLAGS[0x4D]=0;
        kbdFLAGS[0x48]=0;
        kbdFLAGS[0x50]=0;
        sc=0;
      }
      buf[fbuf]=as; buf[fbuf+1]=sc; buf[fbuf+2]=ss;
      if ((fbuf+=3)==64*3) fbuf=0;

    } while (_bios_keybrd(_KEYBRD_READY));
  }

  if (ctrl_c==1) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
    buf[fbuf]=0; buf[fbuf+1]=46; buf[fbuf+2]=ss;
    if ((fbuf+=3)==64*3) fbuf=0; ctrl_c=0;
  }
}

void tecla(void) {
  union REGS r;
  struct SREGS s;

  tecla_bios();

  if (ibuf!=fbuf) {
    ascii=buf[ibuf]; scan_code=buf[ibuf+1]; shift_status=buf[ibuf+2];
    if ((ibuf+=3)==64*3) ibuf=0;
  } else {
    ascii=0; scan_code=0;
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;
  }
}

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//      Vacia el buffer de teclado (real e interno)
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

void vacia_buffer(void) {
  union REGS r;
  struct SREGS s;

  s.ds=s.es=s.fs=s.gs=FP_SEG(&s);

  while(_bios_keybrd(_KEYBRD_READY)) { r.h.ah=0; int386x(0x16,&r,&r,&s); }
  ascii=0; scan_code=0;
  r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;

  ibuf=fbuf=0; // Vacia el buffer interno
}

