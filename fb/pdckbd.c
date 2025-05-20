#define LINUX_FRAMEBUFFER_PORT

void PDC_check_for_blinking( void);
int PDC_cycle_font( void);
void PDC_rotate_font( void);

#ifdef HAVE_MOUSE
   static void _check_mouse( );
#endif

#ifdef USE_DRM
int PDC_cycle_display( void);
#endif

#include "../vt/pdckbd.c"

#ifdef HAVE_MOUSE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "psf.h"
#include "pdcfb.h"

int PDC_update_mouse( void);
bool PDC_update_mouse_cursor( int left, int right, int top, int bottom, const bool draw_it);
bool PDC_remove_mouse_cursor( void);

static void _check_mouse( )
{
   const int mx = PDC_mouse_x, my = PDC_mouse_y;
   extern struct font_info PDC_font_info;
   extern int PDC_orientation;
   const int xmax = SP->cols * ((PDC_orientation & 1) ? PDC_font_info.height : PDC_font_info.width);
   const int ymax = SP->lines * ((PDC_orientation & 1) ? PDC_font_info.width : PDC_font_info.height);

   while( PDC_update_mouse( ))
      {
      if( PDC_mouse_x < 0)
         PDC_mouse_x = 0;
      if( PDC_mouse_x > xmax - 1)
         PDC_mouse_x = xmax - 1;
      if( PDC_mouse_y < 0)
         PDC_mouse_y = 0;
      if( PDC_mouse_y > ymax - 1)
         PDC_mouse_y = ymax - 1;
      }
   if( mx != PDC_mouse_x || my != PDC_mouse_y)
      {
      const int tx = PDC_mouse_x, ty = PDC_mouse_y;

      PDC_mouse_x = mx;
      PDC_mouse_y = my;
      PDC_remove_mouse_cursor( );
      PDC_mouse_x = tx;
      PDC_mouse_y = ty;
      PDC_update_mouse_cursor( 0, xmax, 0, ymax, 1);
      }
}
#endif            /* #ifdef HAVE_MOUSE */
