typedef struct _HeadFPG {
  int code;
  int length;
  uint8_t description[32];
  uint8_t filename[12];
  int width;
  int height;
  int num_points;
} HeadFPG;

#define FPG_HEAD 64

typedef struct _FPG {
  FILE *fpg;
  FILE *old_fpg;
  HeadFPG header;
  uint8_t current_file[_MAX_PATH + 14];
  int nIndex;
  int last_used;
  int desc_index[MAX_FPG_GRAPHICS];
  uint8_t code_desc[MAX_FPG_GRAPHICS][40];
  int grf_offsets[MAX_FPG_GRAPHICS];
  byte fpg_name[13];
  struct t_listboxbr list_info;
  int fpg_info;
  int thumb_on;
  t_thumb thumb[MAX_FPG_GRAPHICS];
  byte version;
} FPG;

void fpg_sort(FPG *fpg_file);
int fpg_read_header(HeadFPG *fpg_header, FILE *fpg);
void fpg_write_header(HeadFPG *fpg_header, short *points, char *image, FILE *fpg);
void fpg_create_listbox(FPG *fpg_file);

void fpg_create(FPG *fpg_file, char *name);
int fpg_open(FPG *fpg_file, char *name);
int fpg_add(FPG *fpg_file, int cod, char *desc, char *filename, int width, int height, int num_points,
            char *points, char *img_data, int delete_old, int get_info);
int fpg_delete(FPG *fpg_file, int cod);
void fpg_read_image_header(HeadFPG *fpg_header, FILE *fpg);
int remap_all_file(FPG *fpg_file);
int fpg_remap_to_pal(FPG *fpg_file);

void FPG_create_thumbs(void);
void FPG_show_thumb(struct t_listboxbr *l, int num);
void FPG_paint_listbox_br(struct t_listboxbr *l);
void FPG_paint_slider_br(struct t_listboxbr *l);
void FPG_create_listbox_br(struct t_listboxbr *l);
void FPG_update_listbox_br(struct t_listboxbr *l);
void create_thumb_FPG(struct t_listboxbr *l);
