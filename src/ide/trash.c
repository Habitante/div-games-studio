#include "global.h"
#include "fpg.h"


// TRASHCAN Handler

extern HeadFPG drag_fpg_header;
void fpg_dialog2();

void FPG_paint_listbox_br(struct t_listboxbr *l);
void FPG_update_listbox_br(struct t_listboxbr *l);


// Trash pain handerl
void bin1(void) {
  int w = v.w, h = v.h;
  byte *ptr = v.ptr;
  if (big) {
    w /= 2;
    h /= 2;
  }

  wput(ptr, w, h, 2, 10, -219);
}

// Trash click handler (handles mouse-up event for drag)

void bin2(void) {
  FPG *my_fpg;
  struct twindow my_window;

  if (dragging == DRAG_DROPPING) {
    dragging = DRAG_DROPPED;
    free_drag = 0;

    switch (window[1].type) {
    case 100: // Map dragged to bin from desktop (alias for close map)
      my_window = v;
      v = window[1];
      window[1] = my_window;
      close_window();
      break;

    case 101: // Item dragged from FPG to bin. Remove from FPG
      v_title = (char *)texts[60];
      v_text = window[1].mapa->description;
      show_dialog(accept0);

      if (v_accept) {
        my_window = v;
        my_fpg = (FPG *)window[1].aux;
        fpg_delete(my_fpg, window[1].mapa->fpg_code);
        v = window[1];

        //if (my_fpg->list_info.first_visible+my_fpg->list_info.lines*my_fpg->list_info.columns>my_fpg->list_info.total_items
        //    && my_fpg->list_info.first_visible) my_fpg->list_info.first_visible--;

        while (my_fpg->list_info.first_visible +
                   (my_fpg->list_info.lines - 1) * my_fpg->list_info.columns + 1 >
               my_fpg->list_info.total_items) {
          my_fpg->list_info.first_visible -= my_fpg->list_info.columns;
        }

        if (my_fpg->list_info.first_visible < 0)
          my_fpg->list_info.first_visible = 0;

        // FPG_paint_listbox_br(&my_fpg->list_info);

        FPG_update_listbox_br(&my_fpg->list_info);

        v.redraw = 1;
        v = my_window;
      }
      break;
    }
    free_drag = 1;
  }
}

// setup bin

void bin0(void) {
  v.type = WIN_TRASH;
  v.w = 27;
  v.h = 34;
  v.paint_handler = bin1;
  v.click_handler = bin2;
  v.title = (byte *)"";
  v.name = texts[350];
}

void show_trash() {
  new_window(bin0);
}
