#define LINUX_FRAMEBUFFER_PORT

void PDC_check_for_blinking( void);
int PDC_cycle_font( void);
void PDC_rotate_font( void);

#ifdef HAVE_MOUSE
   static void _check_mouse( void);
   static int _key_modifiers;
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

static int _key_to_mouse_modifiers( const int key_modifs)
{
#ifdef NOT_CURRENTLY_NEEDED
   int mouse_modifs = 0;

   if( key_modifs & PDC_KEY_MODIFIER_SHIFT)     /* At least at present,   */
      mouse_modifs = PDC_BUTTON_SHIFT;          /* we can use the simpler */
   if( key_modifs & PDC_KEY_MODIFIER_ALT)       /* bit-twiddling method   */
      mouse_modifs |= PDC_BUTTON_ALT;           /* shown below.  If       */
   if( key_modifs & PDC_KEY_MODIFIER_CONTROL)   /* things change,  they   */
      mouse_modifs |= PDC_BUTTON_CONTROL;       /* will be different      */
   return mouse_modifs;
#endif
   return( (key_modifs & 7) << 3);
}

#define PDC_MOUSE_WHEEL_EVENTS (PDC_MOUSE_WHEEL_UP | PDC_MOUSE_WHEEL_DOWN \
                        | PDC_MOUSE_WHEEL_RIGHT | PDC_MOUSE_WHEEL_LEFT)

int PDC_update_mouse( int *button);
bool PDC_update_mouse_cursor( int left, int right, int top, int bottom, const bool draw_it);
bool PDC_remove_mouse_cursor( void);
int PDC_get_modifiers( void);

static void _check_mouse( void)
{
   const int mx = PDC_mouse_x, my = PDC_mouse_y;
   extern struct font_info PDC_font_info;
   extern int PDC_orientation;
   int button, event;
   const int xper = ((PDC_orientation & 1) ? PDC_font_info.height : PDC_font_info.width);
   const int yper = ((PDC_orientation & 1) ? PDC_font_info.width : PDC_font_info.height);
   const int xmax = SP->cols * xper;
   const int ymax = SP->lines * yper;
   long timeout = 0;

   _key_modifiers = PDC_get_modifiers( );
   if( _get_mouse_event( NULL))      /* already got events queued up */
      return;
   while( (event = PDC_update_mouse( &button)) >= 0 || PDC_millisecs() < timeout)
      {
      int x, y;

      if( PDC_mouse_x < 0)
         PDC_mouse_x = 0;
      if( PDC_mouse_x > xmax - 1)
         PDC_mouse_x = xmax - 1;
      if( PDC_mouse_y < 0)
         PDC_mouse_y = 0;
      if( PDC_mouse_y > ymax - 1)
         PDC_mouse_y = ymax - 1;
      x = PDC_mouse_x / xper;
      y = PDC_mouse_y / yper;
      if( event >= 0)
         {
         const int modifs = _key_to_mouse_modifiers( _key_modifiers);

         if( x != mx / xper || y != my / yper)
            _add_raw_mouse_event( 0, event, modifs, x, y);
         else if( event & PDC_MOUSE_WHEEL_EVENTS)
            {
            _add_raw_mouse_event( 0, event, modifs, x, y);
            break;
            }
         else if( event == BUTTON_PRESSED || event == BUTTON_RELEASED)
            {
            if( _add_raw_mouse_event( button - 1, event, modifs, x, y))
               timeout = PDC_millisecs( ) + SP->mouse_wait;
            else
               break;
            }
         }
      else if( timeout)
         napms( 10);
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
