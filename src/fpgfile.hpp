typedef struct _HeadFPG {
  int code;
  int length;
  uint8_t description[32];
  uint8_t Filename[12];
  int width;
  int height;
  int num_points;
} HeadFPG;

#define FPG_HEAD 64

typedef struct _FPG {
  FILE *fpg;
  FILE *Oldfpg;
  HeadFPG MiHeadFPG;
  uint8_t current_file[_MAX_PATH + 14];
  int nIndex;
  int last_used;
  int desc_index[1000];
  uint8_t code_desc[1000][40];
  int grf_offsets[1000];
  byte fpg_name[13];
  struct t_listboxbr list_info;
  int fpg_info;
  int thumb_on;
  t_thumb thumb[1000];
  byte version;
} FPG;

void fpg_sort(FPG *Fpg);
int fpg_read_header(HeadFPG *MiHeadFPG, FILE *fpg);
void fpg_write_header(HeadFPG *MiHeadFPG, short *points, char *imagen, FILE *fpg);
void fpg_create_listbox(FPG *Fpg);

void fpg_create(FPG *Fpg, char *Name);
int fpg_open(FPG *Fpg, char *Name);
int fpg_add(FPG *Fpg, int COD, char *desc, char *filename, int Ancho, int Alto, int num_points,
            char *points, char *Imagen, int delete_old, int get_info);
int fpg_delete(FPG *Fpg, int COD);
void fpg_read_image_header(HeadFPG *MiHeadFPG, FILE *fpg);
int remap_all_file(FPG *Fpg);
int fpg_remap_to_pal(FPG *Fpg);

void FPG_create_thumbs(void);
void FPG_show_thumb(struct t_listboxbr *l, int num);
void FPG_paint_listbox_br(struct t_listboxbr *l);
void FPG_paint_slider_br(struct t_listboxbr *l);
void FPG_create_listbox_br(struct t_listboxbr *l);
void FPG_update_listbox_br(struct t_listboxbr *l);
void create_thumb_FPG(struct t_listboxbr *l);
