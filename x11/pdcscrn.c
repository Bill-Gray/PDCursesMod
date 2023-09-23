/* PDCurses */

#include "pdcx11.h"

#include <xpm.h>

#include <stdlib.h>
#include <string.h>

/* Default icons for XCurses applications.  */

#include "../common/icon64.xpm"
#include "../common/icon32.xpm"

#include "../common/pdccolor.c"

#ifdef PDC_WIDE
# define DEFNFONT "-misc-fixed-medium-r-normal--20-200-75-75-c-100-iso10646-1"
# define DEFIFONT "-misc-fixed-medium-o-normal--20-200-75-75-c-100-iso10646-1"
# define DEFBFONT "-misc-fixed-bold-r-normal--20-200-75-75-c-100-iso10646-1"
#else
# define DEFNFONT "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1"
# define DEFIFONT "-misc-fixed-medium-o-normal--13-120-75-75-c-70-iso8859-1"
# define DEFBFONT "-misc-fixed-bold-r-normal--13-120-75-75-c-70-iso8859-1"
#endif

#ifndef MAX_PATH
# define MAX_PATH 256
#endif

/* Macros just for app_resources */

#define APPDATAOFF(n) XtOffsetOf(XCursesAppData, n)

#define RINT(name1, name2, value) { \
                #name1, #name2, XtRInt, \
                sizeof(int), APPDATAOFF(name1), XtRImmediate, \
                (XtPointer)value \
        }

#define RPIXEL(name1, name2, value) { \
                #name1, #name2, XtRPixel, \
                sizeof(Pixel), APPDATAOFF(name1), XtRString, \
                (XtPointer)#value \
        }

#define RCOLOR(name, value) RPIXEL(color##name, Color##name, value)


#define RSTRINGP(name1, name2, param) { \
                #name1, #name2, XtRString, \
                MAX_PATH, APPDATAOFF(name1), XtRString, (XtPointer)param \
        }

#define RSTRING(name1, name2) RSTRINGP(name1, name2, "")

#define RFONT(name1, name2, value) { \
                #name1, #name2, XtRFontStruct, \
                sizeof(XFontStruct), APPDATAOFF(name1), XtRString, \
                (XtPointer)value \
        }

#define RCURSOR(name1, name2, value) { \
                #name1, #name2, XtRCursor, \
                sizeof(Cursor), APPDATAOFF(name1), XtRString, \
                (XtPointer)#value \
        }

static XtResource app_resources[] =
{
    RINT(lines, Lines, -1),
    RINT(cols, Cols, -1),

    RCOLOR(Black, Black),
    RCOLOR(Red, red3),
    RCOLOR(Green, green3),
    RCOLOR(Yellow, yellow3),
    RCOLOR(Blue, blue3),
    RCOLOR(Magenta, magenta3),
    RCOLOR(Cyan, cyan3),
    RCOLOR(White, Grey),

    RCOLOR(BoldBlack, grey40),
    RCOLOR(BoldRed, red1),
    RCOLOR(BoldGreen, green1),
    RCOLOR(BoldYellow, yellow1),
    RCOLOR(BoldBlue, blue1),
    RCOLOR(BoldMagenta, magenta1),
    RCOLOR(BoldCyan, cyan1),
    RCOLOR(BoldWhite, White),

    RFONT(normalFont, NormalFont, DEFNFONT),
    RFONT(italicFont, ItalicFont, DEFIFONT),
    RFONT(boldFont, BoldFont, DEFBFONT),

    RSTRING(bitmap, Bitmap),
    RSTRING(pixmap, Pixmap),
    RCURSOR(pointer, Pointer, xterm),

    RPIXEL(pointerForeColor, PointerForeColor, Black),
    RPIXEL(pointerBackColor, PointerBackColor, White),

    RINT(doubleClickPeriod, DoubleClickPeriod, (PDC_CLICK_PERIOD * 2)),
    RINT(clickPeriod, ClickPeriod, PDC_CLICK_PERIOD),
    RINT(scrollbarWidth, ScrollbarWidth, 15),
    RINT(cursorBlinkRate, CursorBlinkRate, 0),

    RSTRING(textCursor, TextCursor),
    RINT(textBlinkRate, TextBlinkRate, 500)
};

#undef RCURSOR
#undef RFONT
#undef RSTRING
#undef RCOLOR
#undef RPIXEL
#undef RINT
#undef APPDATAOFF
#undef DEFBFONT
#undef DEFIFONT
#undef DEFNFONT

/* Macros for options */

#define COPT(name) {"-" #name, "*" #name, XrmoptionSepArg, NULL}
#define CCOLOR(name) COPT(color##name)

static XrmOptionDescRec options[] =
{
    COPT(lines), COPT(cols), COPT(normalFont), COPT(italicFont),
    COPT(boldFont), COPT(bitmap), COPT(pixmap), COPT(pointer),
    COPT(clickPeriod), COPT(doubleClickPeriod), COPT(scrollbarWidth),
    COPT(pointerForeColor), COPT(pointerBackColor),
    COPT(cursorBlinkRate), COPT(textCursor), COPT(textBlinkRate),

    CCOLOR(Black), CCOLOR(Red), CCOLOR(Green), CCOLOR(Yellow),
    CCOLOR(Blue), CCOLOR(Magenta), CCOLOR(Cyan), CCOLOR(White),

    CCOLOR(BoldBlack), CCOLOR(BoldRed), CCOLOR(BoldGreen),
    CCOLOR(BoldYellow), CCOLOR(BoldBlue), CCOLOR(BoldMagenta),
    CCOLOR(BoldCyan), CCOLOR(BoldWhite)
};

#undef CCOLOR
#undef COPT

XCursesAppData pdc_app_data;
XtAppContext pdc_app_context;
Widget pdc_toplevel, pdc_drawing;

GC pdc_normal_gc, pdc_cursor_gc, pdc_italic_gc, pdc_bold_gc;
int pdc_fheight, pdc_fwidth, pdc_fascent, pdc_fdescent;
int pdc_wwidth, pdc_wheight;
bool pdc_window_entered = TRUE, pdc_resize_now = FALSE, pdc_return_window_close_as_key = FALSE;

static Atom wm_atom[2];
static String class_name = "XCurses";
static int resize_window_width = 0, resize_window_height = 0;
static int received_map_notify = 0;
static bool exposed = FALSE;
static int _override_lines = 0;
static int _override_cols = 0;

static Pixmap icon_pixmap, icon_pixmap_mask;

static char *prog_name[] = {"PDCurses", NULL};
static char **argv = prog_name;
static int argc = 1;

/* close the physical screen */

void PDC_scr_close(void)
{
    PDC_LOG(("PDC_scr_close() - called\n"));
}

static void _remove_event_handlers( void);

void PDC_scr_free(void)
{
    extern XIM pdc_xim;

    PDC_free_palette( );
    if (icon_pixmap)
    {
        XFreePixmap(XCURSESDISPLAY, icon_pixmap);
        icon_pixmap = 0;
    }
    if (icon_pixmap_mask)
    {
        XFreePixmap(XCURSESDISPLAY, icon_pixmap_mask);
        icon_pixmap_mask = 0;
    }

    if( pdc_normal_gc)
    {
        XFreeGC(XCURSESDISPLAY, pdc_normal_gc);
        pdc_normal_gc = NULL;
    }
    if( pdc_italic_gc)
    {
        XFreeGC(XCURSESDISPLAY, pdc_italic_gc);
        pdc_italic_gc = NULL;
    }
    if( pdc_bold_gc)
    {
        XFreeGC(XCURSESDISPLAY, pdc_bold_gc);
        pdc_bold_gc = 0;
    }
    if( pdc_cursor_gc)
    {
        XFreeGC(XCURSESDISPLAY, pdc_cursor_gc);
        pdc_cursor_gc = 0;
    }
    if( pdc_xic)
    {
        XDestroyIC(pdc_xic);
        pdc_xic = 0;
    }
    if( pdc_xim)
    {
        XCloseIM( pdc_xim);
        pdc_xim = NULL;
    }
    if( pdc_app_context)
    {
        _remove_event_handlers( );
        XtDestroyApplicationContext( pdc_app_context);
        pdc_app_context = 0;
    }
}

void XCursesExit(void)
{
    PDC_scr_free();
}

Pixel PDC_get_pixel( const int idx)
{
   PACKED_RGB rgb = PDC_get_palette_entry( idx);

   return( (Pixel) ( ((rgb >> 16) & 0xff) | (rgb & 0xff00) | ((rgb & 0xff) << 16)));
}

static void _get_icon(void)
{
    Status rc;

    PDC_LOG(("_get_icon() - called\n"));

    if (pdc_app_data.pixmap && pdc_app_data.pixmap[0]) /* supplied pixmap */
    {
        XpmReadFileToPixmap(XtDisplay(pdc_toplevel),
                            RootWindowOfScreen(XtScreen(pdc_toplevel)),
                            (char *)pdc_app_data.pixmap,
                            &icon_pixmap, &icon_pixmap_mask, NULL);
    }
    else if (pdc_app_data.bitmap && pdc_app_data.bitmap[0]) /* bitmap */
    {
        unsigned file_bitmap_width = 0, file_bitmap_height = 0;
        int x_hot = 0, y_hot = 0;

        rc = XReadBitmapFile(XtDisplay(pdc_toplevel),
                             RootWindowOfScreen(XtScreen(pdc_toplevel)),
                             (char *)pdc_app_data.bitmap,
                             &file_bitmap_width, &file_bitmap_height,
                             &icon_pixmap, &x_hot, &y_hot);

        if (BitmapOpenFailed == rc)
            fprintf(stderr, "bitmap file %s: not found\n",
                    pdc_app_data.bitmap);
        else if (BitmapFileInvalid == rc)
            fprintf(stderr, "bitmap file %s: contents invalid\n",
                    pdc_app_data.bitmap);
    }
    else
    {
        XIconSize *icon_size;
        int size_count = 0, max_height = 0, max_width = 0;

        icon_size = XAllocIconSize();

        rc = XGetIconSizes(XtDisplay(pdc_toplevel),
                           RootWindowOfScreen(XtScreen(pdc_toplevel)),
                           &icon_size, &size_count);

        /* if the WM can advise on icon sizes... */

        if (rc && size_count)
        {
            int i;

            PDC_LOG(("size_count: %d rc: %d\n", size_count, rc));

            for (i = 0; i < size_count; i++)
            {
                if (icon_size[i].max_width > max_width)
                    max_width = icon_size[i].max_width;
                if (icon_size[i].max_height > max_height)
                    max_height = icon_size[i].max_height;

                PDC_LOG(("min: %d %d\n",
                         icon_size[i].min_width, icon_size[i].min_height));

                PDC_LOG(("max: %d %d\n",
                         icon_size[i].max_width, icon_size[i].max_height));

                PDC_LOG(("inc: %d %d\n",
                         icon_size[i].width_inc, icon_size[i].height_inc));
            }
        }

        XFree(icon_size);

        XpmCreatePixmapFromData(XtDisplay(pdc_toplevel),
              RootWindowOfScreen(XtScreen(pdc_toplevel)),
              (max_width >= 64 && max_height >= 64) ? icon64 : icon32,
              &icon_pixmap, &icon_pixmap_mask, NULL);
    }
}

/* Redraw the entire screen */

static void _display_screen(void)
{
    int row;

    PDC_LOG(("_display_screen() - called\n"));

    if (!curscr)
        return;

    for (row = 0; row < SP->lines; row++)
        PDC_transform_line(row, 0, COLS, curscr->_y[row]);

    PDC_redraw_cursor();
}

static void _handle_expose(Widget w, XtPointer client_data, XEvent *event,
                           Boolean *unused)
{
    PDC_LOG(("_handle_expose() - called\n"));

    INTENTIONALLY_UNUSED_PARAMETER( w);
    INTENTIONALLY_UNUSED_PARAMETER( client_data);
    INTENTIONALLY_UNUSED_PARAMETER( unused);
    /* ignore all Exposes except last */

    if (event->xexpose.count)
        return;

    exposed = TRUE;

    if (received_map_notify)
        _display_screen();
}

static void _handle_nonmaskable(Widget w, XtPointer client_data, XEvent *event,
                                Boolean *unused)
{
    XClientMessageEvent *client_event = (XClientMessageEvent *)event;

    PDC_LOG(("_handle_nonmaskable called: event %d\n", event->type));

    INTENTIONALLY_UNUSED_PARAMETER( w);
    INTENTIONALLY_UNUSED_PARAMETER( client_data);
    INTENTIONALLY_UNUSED_PARAMETER( unused);
    if (event->type == ClientMessage)
    {
        PDC_LOG(("ClientMessage received\n"));

        /* This code used to include handling of WM_SAVE_YOURSELF, but
           it resulted in continual failure of THE on my Toshiba laptop.
           Removed on 3-3-2001. Now only exits on WM_DELETE_WINDOW. */

        if ((Atom)client_event->data.s[0] == wm_atom[0])
        {
            if( !PDC_get_function_key( FUNCTION_KEY_SHUT_DOWN))
                exit(0);
            else
                pdc_return_window_close_as_key = TRUE;
        }
    }
}

static void _handle_enter_leave(Widget w, XtPointer client_data,
                                XEvent *event, Boolean *unused)
{
    PDC_LOG(("_handle_enter_leave called\n"));

    INTENTIONALLY_UNUSED_PARAMETER( w);
    INTENTIONALLY_UNUSED_PARAMETER( client_data);
    INTENTIONALLY_UNUSED_PARAMETER( unused);
    switch(event->type)
    {
    case EnterNotify:
        PDC_LOG(("EnterNotify received\n"));

        pdc_window_entered = TRUE;
        break;

    case LeaveNotify:
        PDC_LOG(("LeaveNotify received\n"));

        pdc_window_entered = FALSE;

        /* Display the cursor so it stays on while the window is
           not current */

        PDC_redraw_cursor();
        break;

    default:
        PDC_LOG(("_handle_enter_leave - unknown event %d\n", event->type));
    }
}

static void _handle_structure_notify(Widget w, XtPointer client_data,
                                     XEvent *event, Boolean *unused)
{
    PDC_LOG(("_handle_structure_notify() - called\n"));

    INTENTIONALLY_UNUSED_PARAMETER( w);
    INTENTIONALLY_UNUSED_PARAMETER( client_data);
    INTENTIONALLY_UNUSED_PARAMETER( unused);
    switch(event->type)
    {
    case ConfigureNotify:
        PDC_LOG(("ConfigureNotify received\n"));

        if( resize_window_width != event->xconfigure.width
          || resize_window_height != event->xconfigure.height)
        {
            /* Window has been resized, change width and height to send to
               place_text and place_graphics in next Expose. */

            resize_window_width = event->xconfigure.width;
            resize_window_height = event->xconfigure.height;

            SP->resized = TRUE;
            pdc_resize_now = TRUE;
        }
        break;

    case MapNotify:
        PDC_LOG(("MapNotify received\n"));

        received_map_notify = 1;

        break;

    default:
        PDC_LOG(("_handle_structure_notify - unknown event %d\n",
                 event->type));
    }
}

static void _get_gc(GC *gc, XFontStruct *font_info, int fore, int back)
{
    XGCValues values;

    /* Create default Graphics Context */

    *gc = XCreateGC(XCURSESDISPLAY, XCURSESWIN, 0L, &values);

    /* specify font */

    XSetFont(XCURSESDISPLAY, *gc, font_info->fid);

    XSetForeground(XCURSESDISPLAY, *gc, PDC_get_pixel( fore));
    XSetBackground(XCURSESDISPLAY, *gc, PDC_get_pixel( back));
}

static void _pointer_setup(void)
{
    XColor pointerforecolor, pointerbackcolor;
    XrmValue rmfrom, rmto;

    XDefineCursor(XCURSESDISPLAY, XCURSESWIN, pdc_app_data.pointer);
    rmfrom.size = sizeof(Pixel);
    rmto.size = sizeof(XColor);

    rmto.addr = (XPointer)&pointerforecolor;
    rmfrom.addr = (XPointer)&(pdc_app_data.pointerForeColor);
    XtConvertAndStore(pdc_drawing, XtRPixel, &rmfrom, XtRColor, &rmto);

    rmfrom.size = sizeof(Pixel);
    rmto.size = sizeof(XColor);

    rmfrom.addr = (XPointer)&(pdc_app_data.pointerBackColor);
    rmto.addr = (XPointer)&pointerbackcolor;
    XtConvertAndStore(pdc_drawing, XtRPixel, &rmfrom, XtRColor, &rmto);

    XRecolorCursor(XCURSESDISPLAY, pdc_app_data.pointer,
                   &pointerforecolor, &pointerbackcolor);
}

void PDC_set_args(int c, char **v)
{
    argc = c;
    argv = v;
}

/* open the physical screen -- miscellaneous initialization */

int PDC_scr_open(void)
{
    bool italic_font_valid, bold_font_valid;
    int minwidth, minheight;

    PDC_LOG(("PDC_scr_open() - called\n"));

    /* Start defining X Toolkit things */

#if XtSpecificationRelease > 4
    XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);
#endif

    /* Exit if no DISPLAY variable set */

    if (!getenv("DISPLAY"))
    {
        fprintf(stderr, "Error: no DISPLAY variable set\n");
        return ERR;
    }

    /* Initialise the top level widget */

    pdc_toplevel = XtVaAppInitialize(&pdc_app_context, class_name, options,
                                 XtNumber(options), &argc, argv, NULL, NULL);

    XtVaGetApplicationResources(pdc_toplevel, &pdc_app_data, app_resources,
                                XtNumber(app_resources), NULL);

    /* Check application resource values here */

    pdc_fwidth = pdc_app_data.normalFont->max_bounds.width;

    pdc_fascent = pdc_app_data.normalFont->ascent;
    pdc_fdescent = pdc_app_data.normalFont->descent;
    pdc_fheight = pdc_fascent + pdc_fdescent;

    /* Check that the italic font and normal fonts are the same size */

    italic_font_valid = pdc_fwidth ==
        pdc_app_data.italicFont->max_bounds.width;

    bold_font_valid = pdc_fwidth ==
        pdc_app_data.boldFont->max_bounds.width;

    /* Calculate size of display window */

    if( _override_lines && _override_cols)
    {
        COLS = _override_cols;
        LINES = _override_lines;
    }
    else
    {
        COLS = pdc_app_data.cols;
        LINES = pdc_app_data.lines;
    }

    if (-1 == COLS)
    {
        const char *env = getenv("PDC_COLS");
        if (env)
            COLS = atoi(env);

        if (COLS <= 0)
            COLS = 80;
    }

    if (-1 == LINES)
    {
        const char *env = getenv("PDC_LINES");
        if (env)
            LINES = atoi(env);

        if (LINES <= 0)
            LINES = 24;
    }

    pdc_wwidth = pdc_fwidth * COLS;
    pdc_wheight = pdc_fheight * LINES;

    minwidth = pdc_fwidth * 2;
    minheight = pdc_fheight * 2;

    /* Set up the icon for the application; the default is an internal
       one for PDCurses. Then set various application level resources. */

    _get_icon();

    XtVaSetValues(pdc_toplevel, XtNminWidth, minwidth, XtNminHeight,
                  minheight, XtNbaseWidth, 0, XtNbaseHeight, 0,
                  XtNbackground, 0, XtNiconPixmap, icon_pixmap,
                  XtNiconMask, icon_pixmap_mask, NULL);

    /* Create a widget in which to draw */

    if (!PDC_scrollbar_init(argv[0]))
    {
        pdc_drawing = pdc_toplevel;

        XtVaSetValues(pdc_toplevel, XtNwidth, pdc_wwidth, XtNheight,
            pdc_wheight, XtNwidthInc, pdc_fwidth, XtNheightInc,
            pdc_fheight, NULL);
    }

    /* Determine text cursor alignment from resources */

    if (!strcmp(pdc_app_data.textCursor, "vertical"))
        pdc_vertical_cursor = TRUE;

    SP->lines = LINES;
    SP->cols = COLS;

    SP->mouse_wait = pdc_app_data.clickPeriod;
    SP->audible = TRUE;

    SP->termattrs = A_COLOR | WA_ITALIC | WA_UNDERLINE | WA_LEFT | WA_RIGHT |
                    WA_REVERSE | WA_STRIKEOUT | WA_TOP | WA_BLINK | WA_DIM | WA_BOLD;

    /* Add Event handlers to the drawing widget */

    XtAddEventHandler(pdc_drawing, ExposureMask, False, _handle_expose, NULL);
    XtAddEventHandler(pdc_drawing, StructureNotifyMask, False,
                      _handle_structure_notify, NULL);
    XtAddEventHandler(pdc_drawing, EnterWindowMask | LeaveWindowMask, False,
                      _handle_enter_leave, NULL);
    XtAddEventHandler(pdc_toplevel, 0, True, _handle_nonmaskable, NULL);

    /* If there is a cursorBlink resource, start the Timeout event */

    if (pdc_app_data.cursorBlinkRate)
        XtAppAddTimeOut(pdc_app_context, pdc_app_data.cursorBlinkRate,
                        PDC_blink_cursor, NULL);

    XtRealizeWidget(pdc_toplevel);

    /* Handle trapping of the WM_DELETE_WINDOW property */

    wm_atom[0] = XInternAtom(XtDisplay(pdc_toplevel), "WM_DELETE_WINDOW",
                             False);

    /* Make sure we tell X that we'd like to take focus */
    wm_atom[1] = XInternAtom(XtDisplay(pdc_toplevel), "WM_TAKE_FOCUS",
                             False);
    XSetWMProtocols(XtDisplay(pdc_toplevel), XtWindow(pdc_toplevel),
                    wm_atom, 2);

    /* Create the Graphics Context for drawing. This MUST be done AFTER
       the associated widget has been realized. */

    PDC_LOG(("before _get_gc\n"));

    _get_gc(&pdc_normal_gc, pdc_app_data.normalFont, COLOR_WHITE, COLOR_BLACK);

    _get_gc(&pdc_italic_gc, italic_font_valid ? pdc_app_data.italicFont :
            pdc_app_data.normalFont, COLOR_WHITE, COLOR_BLACK);

    _get_gc(&pdc_bold_gc, bold_font_valid ? pdc_app_data.boldFont :
            pdc_app_data.normalFont, COLOR_WHITE, COLOR_BLACK);

    _get_gc(&pdc_cursor_gc, pdc_app_data.normalFont,
            COLOR_WHITE, COLOR_BLACK);

    XSetLineAttributes(XCURSESDISPLAY, pdc_cursor_gc, 2,
                       LineSolid, CapButt, JoinMiter);

    /* Set the pointer for the application */

    _pointer_setup();

    if (ERR == PDC_kb_setup())
        return ERR;

    while (!exposed)
    {
        XEvent event;

        XtAppNextEvent(pdc_app_context, &event);
        XtDispatchEvent(&event);
    }

    if (SP->resized)
    {
        pdc_wwidth = resize_window_width;
        pdc_wheight = resize_window_height;
        pdc_resize_now = FALSE;
        SP->resized = FALSE;
    }

    PDC_init_palette( );

    SP->orig_attr = FALSE;

    atexit(PDC_scr_free);

    XSync(XtDisplay(pdc_toplevel), True);
    SP->resized = pdc_resize_now = FALSE;

    /* Make sure that we say that we're allowed to have input focus.
       Otherwise some window managers will refuse to focus the window. */
    XWMHints* hints = XGetWMHints(XtDisplay(pdc_toplevel), XtWindow(pdc_toplevel));
    hints->input=TRUE;
    XSetWMHints(XtDisplay(pdc_toplevel), XtWindow(pdc_toplevel), hints);
    XFree(hints);

    return OK;
}

static void _remove_event_handlers( void)
{
    XtRemoveEventHandler(pdc_drawing, ExposureMask, False, _handle_expose, NULL);
    XtRemoveEventHandler(pdc_drawing, StructureNotifyMask, False,
                      _handle_structure_notify, NULL);
    XtRemoveEventHandler(pdc_drawing, EnterWindowMask | LeaveWindowMask, False,
                      _handle_enter_leave, NULL);
    XtRemoveEventHandler(pdc_toplevel, 0, True, _handle_nonmaskable, NULL);
}

/* the core of resize_term() */

int PDC_resize_screen(int nlines, int ncols)
{
    PDC_LOG(("PDC_resize_screen() - called. Lines: %d Cols: %d\n",
             nlines, ncols));

    if( !stdscr)        /* screen hasn't been created yet;  we're */
    {                   /* specifying its size before initscr()   */
        _override_lines = nlines;
        _override_cols = ncols;
        return OK;
    }

    if (nlines || ncols || !SP->resized)
        return ERR;

    pdc_wwidth = resize_window_width;
    pdc_wheight = resize_window_height;
    pdc_visible_cursor = TRUE;

    return OK;
}

void PDC_reset_prog_mode(void)
{
    PDC_LOG(("PDC_reset_prog_mode() - called.\n"));
}

void PDC_reset_shell_mode(void)
{
    PDC_LOG(("PDC_reset_shell_mode() - called.\n"));
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content(int color, int *red, int *green, int *blue)
{
    XColor tmp;
    Colormap cmap = DefaultColormap(XCURSESDISPLAY,
                                    DefaultScreen(XCURSESDISPLAY));

    tmp.pixel = PDC_get_pixel( color);
    XQueryColor(XCURSESDISPLAY, cmap, &tmp);

    *red   = (int)( ((long)tmp.red   * 1000L + 32767L) / 65535L);
    *green = (int)( ((long)tmp.green * 1000L + 32767L) / 65535L);
    *blue  = (int)( ((long)tmp.blue  * 1000L + 32767L) / 65535L);

    return OK;
}

int PDC_init_color(int color, int red, int green, int blue)
{
    XColor tmp;

    tmp.red    = (unsigned short)( ((long)red   * 65535L + 500L) / 1000L);
    tmp.green  = (unsigned short)( ((long)green * 65535L + 500L) / 1000L);
    tmp.blue   = (unsigned short)( ((long)blue  * 65535L + 500L) / 1000L);

    Colormap cmap = DefaultColormap(XCURSESDISPLAY,
                                    DefaultScreen(XCURSESDISPLAY));

    if (XAllocColor(XCURSESDISPLAY, cmap, &tmp))
        PDC_set_palette_entry( color,
                  PACK_RGB( (PACKED_RGB)tmp.red >> 8,
                            (PACKED_RGB)tmp.green >> 8,
                            (PACKED_RGB)tmp.blue >> 8));
    return OK;
}

/*man-start**************************************************************

Resize limits
-------------

### Synopsis

    void PDC_set_resize_limits( const int new_min_lines,
                                const int new_max_lines,
                                const int new_min_cols,
                                const int new_max_cols);

### Description

   For platforms supporting resizable windows (SDLx, WinGUI, X11).  Some
   programs may be unprepared for a resize event;  for these,  calling
   this function with the max and min limits equal ensures that no
   user resizing can be done.  Other programs may require at least a
   certain number,  and/or no more than a certain number,  of columns
   and/or lines.

### Portability

   PDCurses-only function.

**man-end****************************************************************/

/* Note that at least at present,  only WinGUI pays any attention to
resize limits. */

int PDC_min_lines = 25, PDC_min_cols = 80;
int PDC_max_lines = 25, PDC_max_cols = 80;

void PDC_set_resize_limits( const int new_min_lines, const int new_max_lines,
                  const int new_min_cols, const int new_max_cols)
{
    PDC_min_lines = max( new_min_lines, 2);
    PDC_max_lines = max( new_max_lines, PDC_min_lines);
    PDC_min_cols = max( new_min_cols, 2);
    PDC_max_cols = max( new_max_cols, PDC_min_cols);
}
