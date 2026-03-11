typedef struct _HeadFPG{
        int COD;
        int LONG;
        uint8_t Descrip[32];
        uint8_t Filename[12];
        int  Ancho;
        int  Alto;
        int  num_points;
}HeadFPG;

#define FPG_HEAD 64

typedef struct _FPG{
        FILE    *fpg;
        FILE    *Oldfpg;
        HeadFPG MiHeadFPG;
        uint8_t    ActualFile[_MAX_PATH+14];
        int     nIndex;
        int     LastUsed;
        int     DesIndex[1000];
        uint8_t    CodDes[1000][40];
        int     OffsGrf[1000];
        byte    NombreFpg[13];
        struct  t_listboxbr lInfoFPG;
        int     FPGInfo;
        int     thumb_on;
        t_thumb thumb[1000];
		byte version;
}FPG;

        void fpg_sort(FPG *Fpg);
        int fpg_read_header(HeadFPG *MiHeadFPG,FILE *fpg);
        void fpg_write_header(HeadFPG *MiHeadFPG,short *points,char *imagen,FILE *fpg);
        void CreateListBox(FPG *Fpg);

        void fpg_create(FPG *Fpg,char *Name);
        int fpg_open(FPG *Fpg,char *Name);
        int fpg_add(FPG *Fpg,int COD,char *tDescrip,char *tFilename,int Ancho,int Alto,int num_points,char *points,char *Imagen,int BorrarAntiguo, int get_info);
        int fpg_delete(FPG *Fpg,int COD);
        void fpg_read_image_header(HeadFPG *MiHeadFPG,FILE *fpg);
        int RemapAllFile(FPG *Fpg);
        int fpg_remap_to_pal(FPG *Fpg);

        void FPG_create_thumbs         (void);
        void FPG_show_thumb        (struct t_listboxbr * l, int num);
        void FPG_paint_listbox_br      (struct t_listboxbr * l);
        void FPG_paint_slider_br       (struct t_listboxbr * l);
        void FPG_create_listbox_br      (struct t_listboxbr * l);
        void FPG_update_listbox_br  (struct t_listboxbr * l);
        void create_thumb_FPG       (struct t_listboxbr * l);

