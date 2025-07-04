#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include "curspriv.h"
#include "pdcx11.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

Atom wmDeleteMessage;
static int initial_PDC_rows, initial_PDC_cols;
static XSizeHints *xshSize;
static XWMHints *xwmhHints;
static XClassHint *xchClass;
static Font font;

Display *dis;
Window win;
GC curr_gc;
int PDC_rows = -1, PDC_cols = -1;
int PDC_font_width, PDC_font_height, PDC_font_descent;

/* COLOR_PAIR to attribute encoding table. */

void PDC_reset_prog_mode( void)
{
    PDC_LOG(("PDC_reset_prog_mode() - called.\n"));
}

void PDC_reset_shell_mode( void)
{
    PDC_LOG(("PDC_reset_shell_mode() - called.\n"));
}

int PDC_resize_screen(int nlines, int ncols)
{
   if( PDC_rows == -1)     /* initscr( ) hasn't been called;  we're just */
      {                    /* setting desired size at startup */
      initial_PDC_rows = nlines;
      initial_PDC_cols = ncols;
      }
   else if( nlines > 1 && ncols > 1)
      {
      PDC_rows = nlines;
      PDC_cols = ncols;
      XResizeWindow( dis, win, ncols * PDC_font_width, nlines * PDC_font_height);
      }
   return( 0);
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_scr_close( void)
{
   return;
}

XTextProperty xtpWinName, xtpIconName;

void PDC_scr_free( void)
{
   PDC_free_palette( );
   PDC_free_xim_xic( );
   PDC_setclipboard( NULL, 0);
   PDC_transform_line( 0, 0, 0, NULL);
   XFree( xchClass);
   XFree( xwmhHints);
   XFree( xshSize);
   XFreeGC( dis, curr_gc);
   XUnloadFont( dis, font);
   XCloseDisplay( dis);
   if( xtpWinName.value)
      XFree( xtpWinName.value);
   XFree( xtpIconName.value);
#ifdef USING_COMBINING_CHARACTER_SCHEME
   PDC_expand_combined_characters( 0, NULL);
#endif
}

static int _font_size = 15;
      /* this is the value requested in the font name.  The actual */
      /* font size may be something else.  Don't use this except   */
      /* in choosing a font name!                                  */

static Font _load_font( Display *dis)
{
    const char *font_template = "-misc-fixed-medium-r-normal--%d-*10646-1*";
    char font_name[80];
    Font rval;
    XFontStruct *xfont;
    int direction, font_ascent, font_descent;
    XCharStruct overall;

    snprintf( font_name, sizeof( font_name), font_template, _font_size);
    xfont = XLoadQueryFont( dis, font_name);
    if( !xfont)
        return( 0);
    rval = xfont->fid;
    XTextExtents( xfont, "A", 1, &direction, &font_ascent, &font_descent, &overall);
    free( xfont->properties);
    free( xfont->per_char);
    XFreeFontInfo( NULL, xfont, 0);
    PDC_font_descent = font_descent;
    PDC_font_width = overall.width;
    PDC_font_height = font_ascent + font_descent;
    return( rval);
}

int PDC_look_for_font( const int step)
{
    int saved_size = _font_size, rval = 0;
    Font new_font = 0;

    while( !new_font && _font_size < 21 && _font_size > 1)
    {
        _font_size += step;
        new_font = _load_font( dis);
    }
    if( new_font)
    {
        XUnloadFont( dis, font);
        XSetFont( dis, curr_gc, new_font);
        font = new_font;
        rval = 1;
    }
    else
        _font_size = saved_size;
    return( rval);
}

#define MAX_LINES 1000
#define MAX_COLUMNS 1000

int PDC_scr_open(void)
{
    const char *env;
    long event_mask = ExposureMask | KeyPressMask | ButtonPressMask | ClientMessage
                | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;
    char *pcIconName = "Icon Name?";
    char *pcProgName = "Test App";

    PDC_LOG(("PDC_scr_open called\n"));
    COLORS = 256 + (256 * 256 * 256);
    assert( SP);
    if (!SP || PDC_init_palette( ))
        return ERR;

    env = getenv( "PDC_LINES");
    if( env)
       PDC_rows = atoi( env);
    if( PDC_rows < 2)
       PDC_rows = 24;
    env = getenv( "PDC_COLS");
    if( env)
       PDC_cols = atoi( env);
    if( PDC_cols < 2)
       PDC_cols = 80;
    if( initial_PDC_rows && initial_PDC_cols)
       {
       PDC_rows = initial_PDC_rows;
       PDC_cols = initial_PDC_cols;
       }
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->visibility = 0;                /* no cursor,  by default */
    SP->curscol = SP->cursrow = 0;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->orig_attr = TRUE;
    SP->orig_fore = SP->orig_back = -1;
    SP->termattrs = A_COLOR | WA_ITALIC | WA_UNDERLINE | WA_LEFT | WA_RIGHT |
                    WA_REVERSE | WA_STRIKEOUT | WA_TOP | WA_BLINK | WA_DIM | WA_BOLD;
    SP->lines = PDC_rows;
    SP->cols = PDC_cols;
    if (SP->lines < 2 || SP->lines > MAX_LINES
       || SP->cols < 2 || SP->cols > MAX_COLUMNS)
    {
        fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }
    SP->_preserve = (getenv("PDC_PRESERVE_SCREEN") != NULL);

    dis = XOpenDisplay(NULL);
    assert( dis);
    font = _load_font( dis);
    win = XCreateSimpleWindow(dis, DefaultRootWindow( dis),
               1, 1, PDC_cols * PDC_font_width, PDC_rows * PDC_font_height,
               0, WhitePixel (dis, 0), BlackPixel (dis, 0));
    assert( win);
    wmDeleteMessage = XInternAtom( dis, "WM_DELETE_WINDOW", True);

    /* Allocate space for the hints */
    xshSize = XAllocSizeHints();
    xwmhHints = XAllocWMHints();
    xchClass = XAllocClassHint();

    xshSize->flags = PPosition | PSize;
    XStringListToTextProperty(&pcIconName, 1, &xtpIconName);

    xwmhHints->initial_state = NormalState;
    xwmhHints->input = True;
    xwmhHints->flags = StateHint | InputHint;

    xchClass->res_name = pcProgName;
    xchClass->res_class = "Base Win";

    XSetWMProperties( dis, win, NULL, &xtpIconName, NULL, 0,
                      xshSize, xwmhHints, xchClass);
    if( xtpWinName.value)
       XSetWMName( dis, win, &xtpWinName);
    XSetWMProtocols( dis, win, &wmDeleteMessage, 1);
    XMapWindow(dis, win);

    curr_gc = XCreateGC(dis, win, 0, 0);
    XSetForeground(dis, curr_gc, 0x20ff20);
    XSetBackground(dis, curr_gc, 0x700000);

    XSelectInput(dis, win, event_mask);

    XSetFont( dis, curr_gc, font);

    PDC_reset_prog_mode();
    PDC_LOG(("PDC_scr_open exit\n"));
    return( 0);
}

void PDC_set_resize_limits( const int new_min_lines,
                            const int new_max_lines,
                            const int new_min_cols,
                            const int new_max_cols)
{
   INTENTIONALLY_UNUSED_PARAMETER( new_min_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_min_cols);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_cols);
   return;
}


bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content( int color, int *red, int *green, int *blue)
{
    const PACKED_RGB col = PDC_get_palette_entry( color);

    *red = DIVROUND( Get_RValue(col) * 1000, 255);
    *green = DIVROUND( Get_GValue(col) * 1000, 255);
    *blue = DIVROUND( Get_BValue(col) * 1000, 255);

    return OK;
}

int PDC_init_color( int color, int red, int green, int blue)
{
    const PACKED_RGB new_rgb = PACK_RGB(DIVROUND(red * 255, 1000),
                                 DIVROUND(green * 255, 1000),
                                 DIVROUND(blue * 255, 1000));

    if( !PDC_set_palette_entry( color, new_rgb))
        curscr->_clear = TRUE;
    return OK;
}
