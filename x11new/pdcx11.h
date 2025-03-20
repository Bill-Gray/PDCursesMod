#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

extern Display *dis;
extern Window win;
extern GC curr_gc;
extern Font font;
extern Atom wmDeleteMessage;
extern int PDC_font_width, PDC_font_height;
extern int PDC_cols, PDC_rows;

void PDC_free_xim_xic( void);
void PDC_check_for_blinking( void);
