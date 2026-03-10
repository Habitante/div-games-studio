#include "global.h"
#include "fpgfile.hpp"


// TRASHCAN Handler

extern HeadFPG HeadFPGArrastre;
void FPG2();

void FPG_paint_listbox_br      (struct t_listboxbr * l);
void FPG_update_listbox_br  (struct t_listboxbr * l);



// Trash pain handerl
void Bin1(void)
{
	int an=v.an,al=v.al;
	byte * ptr=v.ptr;
	if (big) {
		an/=2; 
		al/=2; 
	}
	
	wput(ptr,an,al,2,10,-219);
}

// Trash click handler (handles mouse-up event for drag)

void Bin2(void)
{
	FPG *MiMemoFPG;
	struct tventana MiVentana;

	if(dragging==4) {
		
		dragging=5; 
		free_drag=0;
		
		switch(ventana[1].type) {

			case 100: // Map dragged to bin from desktop (alias for close map)
				MiVentana=v;
				v=ventana[1];
				ventana[1]=MiVentana;
				close_window();
			break;

			case 101: // Item dragged from FPG to bin. Remove from FPG
				v_title=(char *)texto[60];
				v_text=ventana[1].mapa->description;
				show_dialog(aceptar0);

				if(v_accept) {
					MiVentana=v;
					MiMemoFPG=(FPG *)ventana[1].aux;
					Borrar_FPG(MiMemoFPG,ventana[1].mapa->fpg_code);
					v=ventana[1];

					//if (MiMemoFPG->lInfoFPG.first_visible+MiMemoFPG->lInfoFPG.lines*MiMemoFPG->lInfoFPG.columns>MiMemoFPG->lInfoFPG.total_items
					//    && MiMemoFPG->lInfoFPG.first_visible) MiMemoFPG->lInfoFPG.first_visible--;

					while (MiMemoFPG->lInfoFPG.first_visible+(MiMemoFPG->lInfoFPG.lines-1)*MiMemoFPG->lInfoFPG.columns+1>MiMemoFPG->lInfoFPG.total_items) {
						MiMemoFPG->lInfoFPG.first_visible-=MiMemoFPG->lInfoFPG.columns;
					}

					if (MiMemoFPG->lInfoFPG.first_visible<0) MiMemoFPG->lInfoFPG.first_visible=0;

					// FPG_paint_listbox_br(&MiMemoFPG->lInfoFPG);

					FPG_update_listbox_br(&MiMemoFPG->lInfoFPG);

					v.redraw=1;
					v=MiVentana;
				}
			break;
		} 
		free_drag=1;
	}
}

// setup bin

void Bin0(void)
{
	v.type=5;
	v.an=27;
	v.al=34;
	v.paint_handler=Bin1;
	v.click_handler=Bin2;
	v.title=(byte *)"";
	v.name=texto[350];
}

void muestra_papelera()
{
  new_window(Bin0);
}
