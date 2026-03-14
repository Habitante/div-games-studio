
#define INIT_FRAME 0
#define CMP_RLE    1
#define CMP_OFF    2
#define END_ANIM   3
#define SET_PALET  4
#define MOUSE_KEY  5

void record_screen(unsigned char *new_screen);
void end_recorder();
void write_dac(unsigned char *dac);
void write_mouse_key(char mouse_key);
