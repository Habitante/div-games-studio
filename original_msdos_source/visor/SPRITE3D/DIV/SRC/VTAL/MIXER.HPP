typedef unsigned short  UWORD;

#define NOSOUND         0
#define SB15            1
#define SB20            2
#define SBPRO           3
#define SB16            4
#define SBAWE           5
#define GUS             6
#define GUSMAX          7

#define SBMIXADDRESS    4
#define SBMIXDATA       5
#define SBMIXRESET      0
#define SBMASTER        2
#define SBVOC           0x0A
#define SBCD            8
#define SBPMASTER       0x22
#define SBPVOC          0x4
#define SBPCD           0x28

#define MAXMIXADDRESS   0x10C
#define MAXMIXDATA      0x10D
#define GUSMAXVOCL      2
#define GUSMAXVOCR      3
#define GUSMAXCDL       4
#define GUSMAXCDR       5
#define GUSMAXMASTERL   6
#define GUSMAXMASTERR   7

#define GUSMIXADDRESS   0x506
#define GUSMIXDATA      0x106
#define GUSVOCL         (3<<3) | 2
#define GUSVOCR         (3<<3) | 3
#define GUSCDL          (2<<3) | 2
#define GUSCDR          (2<<3) | 3
#define GUSMASTERL      (5<<3) | 2
#define GUSMASTERR      (5<<3) | 3

void InitMixer(UWORD card, UWORD address, UWORD master, UWORD voc, UWORD cd);
void SetMasterVolume(UWORD volumen);     // de 0 a 15
void SetVocVolume(UWORD volumen);        // de 0 a 15
void SetCDVolume(UWORD volumen);         // de 0 a 15
UWORD GetMasterVolume();
UWORD GetVocVolume();
UWORD GetCDVolume();
UWORD MusicFadeout();    // devuelve el volumen actual del CD
                  // para restaurarlo despu‚s del fadeout.

