#include "pdcmac.h"
#include <stdlib.h>
#include <string.h>
#include <pdccolor.h>
#include <pdccolor.c>

#if UNIVERSAL_INTERFACES_VERSION >= 0x0340
    #define AEEVENTCBPARAM3	long
#else
    #define AEEVENTCBPARAM3	UInt32
#endif

WindowPtr PDC_window = NULL;
int PDC_rows = 25;
int PDC_cols = 80;
int PDC_font_width = 6;
int PDC_font_height = 12;
int PDC_font_ascent = 9;
int PDC_font_size = 12;
RgnHandle PDC_mouse_rgn = NULL;
int PDC_min_lines = 2;
int PDC_max_lines = 200;
int PDC_min_cols = 2;
int PDC_max_cols = 300;

static int toolbox_ready = 0;
static int initial_rows = 0;
static int initial_cols = 0;
static MenuHandle apple_menu;
static MenuHandle file_menu;
static MenuHandle edit_menu;

static void c_to_pstr(const char *src, Str255 dst)
{
    unsigned int length;
    unsigned int i;

    length = 0;
    while (src[length] != 0 && length < 255)
        length++;
    dst[0] = (unsigned char)length;
    for (i = 0; i < length; i++)
        dst[i + 1] = (unsigned char)src[i];
}

void PDC_mac_set_port(void)
{
    if (!PDC_window)
        return;
#if TARGET_API_MAC_CARBON
    SetPortWindowPort(PDC_window);
#else
    SetPort((GrafPtr)PDC_window);
#endif
}

void PDC_mac_port_bounds(Rect *bounds)
{
#if TARGET_API_MAC_CARBON || ACCESSOR_CALLS_ARE_FUNCTIONS
    GetWindowPortBounds(PDC_window, bounds);
#else
    *bounds = PDC_window->portRect;
#endif
}

void PDC_mac_invalidate(const Rect *bounds)
{
    PDC_mac_set_port();
#if TARGET_API_MAC_CARBON
    InvalWindowRect(PDC_window, (Rect *)bounds);
#else
    InvalRect((Rect *)bounds);
#endif
}

void PDC_mac_invalidate_all(void)
{
    Rect bounds;

    PDC_mac_port_bounds(&bounds);
    PDC_mac_invalidate(&bounds);
}

void PDC_mac_cell_rect(int row, int col, Rect *cell)
{
    cell->left = (short)(col * PDC_font_width);
    cell->top = (short)(row * PDC_font_height);
    cell->right = (short)(cell->left + PDC_font_width);
    cell->bottom = (short)(cell->top + PDC_font_height);
}

void PDC_mac_reset_mouse_rgn(void)
{
    Point pt;
    Rect r;

    if (!PDC_mouse_rgn)
        return;
#if TARGET_API_MAC_CARBON
    GetGlobalMouse(&pt);
#else
    GetMouse(&pt);
    PDC_mac_set_port();
    LocalToGlobal(&pt);
#endif
    SetRect(&r, pt.h, pt.v, pt.h + 1, pt.v + 1);
    RectRgn(PDC_mouse_rgn, &r);
}

void PDC_mac_apply_font(void)
{
    FontInfo info;

    PDC_mac_set_port();
    TextFont(monaco);
    TextFace(0);
    TextSize(PDC_font_size);
    GetFontInfo(&info);
    PDC_font_ascent = info.ascent;
    PDC_font_height = info.ascent + info.descent + info.leading;
    PDC_font_width = CharWidth('M');
    if (PDC_font_width < 1)
        PDC_font_width = CharWidth('W');
    if (PDC_font_width < 1)
        PDC_font_width = PDC_font_size / 2 + 1;
    if (PDC_font_height < 1)
        PDC_font_height = PDC_font_size + 2;
}

static void screen_work_rect(Rect *bounds)
{
#if TARGET_API_MAC_CARBON
    BitMap screenbits;

    GetQDGlobalsScreenBits(&screenbits);
    *bounds = screenbits.bounds;
#else
    *bounds = qd.screenBits.bounds;
#endif
    bounds->top = (short)(bounds->top + GetMBarHeight());
}

static void paint_margins(void)
{
    Rect bounds;
    Rect strip;
    RGBColor background;

    PDC_mac_set_port();
    PDC_mac_port_bounds(&bounds);
    PDC_mac_default_background(&background);
    RGBForeColor(&background);
    strip = bounds;
    strip.left = (short)(bounds.right - PDC_MAC_GROW_MARGIN);
    PaintRect(&strip);
    strip = bounds;
    strip.top = (short)(bounds.bottom - PDC_MAC_GROW_MARGIN);
    PaintRect(&strip);
    DrawGrowIcon(PDC_window);
}

void PDC_mac_adjust_size(int pixelwidth, int pixelheight, int queue_resize)
{
    int rows;
    int cols;
    Rect limits;

    cols = (pixelwidth - PDC_MAC_GROW_MARGIN) / PDC_font_width;
    rows = (pixelheight - PDC_MAC_GROW_MARGIN) / PDC_font_height;
    if (cols < PDC_min_cols)
        cols = PDC_min_cols;
    if (rows < PDC_min_lines)
        rows = PDC_min_lines;
    if (cols > PDC_max_cols)
        cols = PDC_max_cols;
    if (rows > PDC_max_lines)
        rows = PDC_max_lines;
    if (cols < 2)
        cols = 2;
    if (rows < 2)
        rows = 2;
    SizeWindow(PDC_window, (short)(cols * PDC_font_width + PDC_MAC_GROW_MARGIN),
               (short)(rows * PDC_font_height + PDC_MAC_GROW_MARGIN), TRUE);
    if (PDC_cols != cols || PDC_rows != rows)
    {
        PDC_cols = cols;
        PDC_rows = rows;
        if (queue_resize)
            PDC_mac_add_key(KEY_RESIZE);
    }
    PDC_mac_port_bounds(&limits);
    PDC_mac_invalidate(&limits);
    if (SP)
        paint_margins();
}

void PDC_mac_adjust_cells(int cols, int rows, int queue_resize)
{
    PDC_mac_adjust_size(cols * PDC_font_width + PDC_MAC_GROW_MARGIN,
                        rows * PDC_font_height + PDC_MAC_GROW_MARGIN, queue_resize);
}

void PDC_mac_drag(EventRecord *event)
{
    Rect screen;

    screen_work_rect(&screen);
    DragWindow(PDC_window, event->where, &screen);
}

void PDC_mac_grow(EventRecord *event)
{
    Rect limits;
    long grow;

    SetRect(&limits,
            (short)(PDC_min_cols * PDC_font_width + PDC_MAC_GROW_MARGIN),
            (short)(PDC_min_lines * PDC_font_height + PDC_MAC_GROW_MARGIN),
            (short)(PDC_max_cols * PDC_font_width + PDC_MAC_GROW_MARGIN),
            (short)(PDC_max_lines * PDC_font_height + PDC_MAC_GROW_MARGIN));
    grow = GrowWindow(PDC_window, event->where, &limits);
    if (grow)
        PDC_mac_adjust_size(LoWord(grow), HiWord(grow), 1);
}

void PDC_mac_zoom(EventRecord *event, short part)
{
    Rect bounds;

    if (!TrackBox(PDC_window, event->where, part))
        return;
    ZoomWindow(PDC_window, part, TRUE);
    PDC_mac_port_bounds(&bounds);
    PDC_mac_adjust_size(bounds.right - bounds.left, bounds.bottom - bounds.top, 1);
}

void PDC_mac_goaway(EventRecord *event)
{
    int key;

    if (!TrackGoAway(PDC_window, event->where))
        return;
    key = PDC_get_function_key(FUNCTION_KEY_SHUT_DOWN);
    if (!key)
        exit(0);
    PDC_mac_add_key(key);
}

void PDC_mac_redraw(void)
{
    Rect bounds;
    int y;

    BeginUpdate(PDC_window);
    PDC_mac_set_port();
    PDC_mac_port_bounds(&bounds);
    EraseRect(&bounds);
    if (curscr && SP)
    {
        int cols;

        cols = SP->cols;
        if (cols > PDC_cols)
            cols = PDC_cols;
        for (y = 0; y < SP->lines && y < PDC_rows; y++)
        {
            if (curscr->_y[y])
                PDC_transform_line(y, 0, cols, curscr->_y[y]);
        }
        paint_margins();
    }
    else
        DrawGrowIcon(PDC_window);
    EndUpdate(PDC_window);
}

void PDC_mac_handle_menu(long menuresult)
{
    short menuid;
    short item;
    int key;

    menuid = HiWord(menuresult);
    item = LoWord(menuresult);
    if (menuid == PDC_MENU_FILE && item == PDC_ITEM_QUIT)
    {
        key = PDC_get_function_key(FUNCTION_KEY_SHUT_DOWN);
        if (!key)
            exit(0);
        PDC_mac_add_key(key);
    }
    else if (menuid == PDC_MENU_EDIT)
    {
        if (item == PDC_ITEM_COPY)
        {
            key = PDC_get_function_key(FUNCTION_KEY_COPY);
            if (!key)
                key = 3;
            PDC_mac_add_key(key);
        }
        else if (item == PDC_ITEM_PASTE)
        {
            key = PDC_get_function_key(FUNCTION_KEY_PASTE);
            if (!key)
                key = 22;
            PDC_mac_add_key(key);
        }
        else if (item == PDC_ITEM_CLEAR)
            PDC_clearclipboard();
    }
#if !TARGET_API_MAC_CARBON
    else if (menuid == PDC_MENU_APPLE && item > 1)
    {
        Str255 name;

        GetMenuItemText(apple_menu, item, name);
        OpenDeskAcc(name);
    }
#endif
    HiliteMenu(0);
}

static pascal OSErr ae_quit(const AppleEvent *evt, AppleEvent *reply,
    AEEVENTCBPARAM3 refcon)
{
    int key;

    INTENTIONALLY_UNUSED_PARAMETER(evt);
    INTENTIONALLY_UNUSED_PARAMETER(reply);
    INTENTIONALLY_UNUSED_PARAMETER(refcon);
    key = PDC_get_function_key(FUNCTION_KEY_SHUT_DOWN);
    if (!key)
        exit(0);
    PDC_mac_add_key(key);
    return noErr;
}

static void init_toolbox(void)
{
    if (toolbox_ready)
        return;
#if TARGET_API_MAC_CARBON
    InitCursor();
#else
    MaxApplZone();
    MoreMasters();
    MoreMasters();
    InitGraf(&qd.thePort);
    InitFonts();
    FlushEvents(everyEvent, 0);
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);
    InitCursor();
#endif
    toolbox_ready = 1;
}

static void init_menus(void)
{
    Str255 title;

    title[0] = 1;
    title[1] = 0x14;
    apple_menu = NewMenu(PDC_MENU_APPLE, title);
    c_to_pstr("About PDCursesMod", title);
    AppendMenu(apple_menu, title);
#if !TARGET_API_MAC_CARBON
    c_to_pstr("(-", title);
    AppendMenu(apple_menu, title);
    AppendResMenu(apple_menu, 'DRVR');
#endif
    c_to_pstr("File", title);
    file_menu = NewMenu(PDC_MENU_FILE, title);
    c_to_pstr("Quit/Q", title);
    AppendMenu(file_menu, title);
    c_to_pstr("Edit", title);
    edit_menu = NewMenu(PDC_MENU_EDIT, title);
    c_to_pstr("Cut/X;Copy/C;Paste/V;Clear", title);
    AppendMenu(edit_menu, title);
    InsertMenu(apple_menu, 0);
    InsertMenu(file_menu, 0);
    InsertMenu(edit_menu, 0);
    DrawMenuBar();
}

static void install_apple_events(void)
{
#if TARGET_API_MAC_CARBON
    AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                          NewAEEventHandlerUPP(ae_quit), 0, FALSE);
#else
    {
        long attr;

        if (Gestalt(gestaltAppleEventsAttr, &attr) == noErr)
        {
            if (attr & (1L << gestaltAppleEventsPresent))
                AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                                      NewAEEventHandlerUPP(ae_quit), 0, FALSE);
        }
    }
#endif
}

static WindowPtr create_window(void)
{
    Rect screen;
    Rect bounds;
    Str255 title;
    int width;
    int height;
    int left;
    int top;
    WindowPtr window;

    screen_work_rect(&screen);
    width = PDC_cols * PDC_font_width + PDC_MAC_GROW_MARGIN;
    height = PDC_rows * PDC_font_height + PDC_MAC_GROW_MARGIN;
    if (width > screen.right - screen.left - 8)
        width = screen.right - screen.left - 8;
    if (height > screen.bottom - screen.top - 8)
        height = screen.bottom - screen.top - 8;
    left = screen.left + (screen.right - screen.left - width) / 2;
    top = screen.top + 16;
    SetRect(&bounds, (short)left, (short)top, (short)(left + width), (short)(top + height));
    c_to_pstr("PDCursesMod", title);
    window = NewCWindow(NULL, &bounds, title, FALSE, zoomDocProc,
                        (WindowPtr)-1L, TRUE, 0);
    return window;
}

void PDC_scr_close(void)
{
    PDC_LOG(("PDC_scr_close() - called\n"));
}

void PDC_scr_free(void)
{
    PDC_free_palette();
    if (PDC_window)
    {
        DisposeWindow(PDC_window);
        PDC_window = NULL;
    }
    if (PDC_mouse_rgn)
    {
        DisposeRgn(PDC_mouse_rgn);
        PDC_mouse_rgn = NULL;
    }
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters(0, NULL);
#endif
}

int PDC_scr_open(void)
{
    const char *env;

    PDC_LOG(("PDC_scr_open() - called\n"));
    if (!SP)
        return ERR;
    if (PDC_init_palette())
        return ERR;
    init_toolbox();
    env = getenv("PDC_FONT_SIZE");
    if (env)
        PDC_font_size = atoi(env);
    if (PDC_font_size < 6)
        PDC_font_size = 12;
    env = getenv("PDC_LINES");
    if (env)
        PDC_rows = atoi(env);
    env = getenv("PDC_COLS");
    if (env)
        PDC_cols = atoi(env);
    if (initial_rows > 1)
        PDC_rows = initial_rows;
    if (initial_cols > 1)
        PDC_cols = initial_cols;
    if (PDC_rows < 2)
        PDC_rows = 25;
    if (PDC_cols < 2)
        PDC_cols = 80;
    if (!PDC_window)
    {
        PDC_font_width = PDC_font_size / 2 + 1;
        PDC_font_height = PDC_font_size + 3;
        PDC_font_ascent = PDC_font_size - 2;
        PDC_window = create_window();
        if (!PDC_window)
            return ERR;
        init_menus();
        install_apple_events();
        PDC_mouse_rgn = NewRgn();
        ShowWindow(PDC_window);
        SelectWindow(PDC_window);
        PDC_mac_set_port();
        PDC_mac_apply_font();
        PDC_mac_adjust_cells(PDC_cols, PDC_rows, 0);
        PDC_mac_reset_mouse_rgn();
    }
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->orig_attr = TRUE;
    SP->orig_fore = COLOR_WHITE;
    SP->orig_back = COLOR_BLACK;
    SP->termattrs = A_COLOR | A_UNDERLINE | A_LEFT | A_RIGHT | A_REVERSE
                    | A_STRIKEOUT | A_TOP | A_BLINK | A_DIM | A_BOLD | A_ITALIC;
    SP->lines = PDC_rows;
    SP->cols = PDC_cols;
    COLORS = 256 + (256 * 256 * 256);
    PDC_reset_prog_mode();
    return OK;
}

int PDC_resize_screen(int nlines, int ncols)
{
    if (!stdscr)
    {
        if (nlines > 1)
            initial_rows = nlines;
        if (ncols > 1)
            initial_cols = ncols;
        if (nlines > 1)
            PDC_rows = nlines;
        if (ncols > 1)
            PDC_cols = ncols;
        return OK;
    }
    if (nlines > 1 && ncols > 1)
        PDC_mac_adjust_cells(ncols, nlines, 0);
    return OK;
}

void PDC_reset_prog_mode(void)
{
    PDC_LOG(("PDC_reset_prog_mode() - called.\n"));
    if (PDC_window)
    {
        ShowWindow(PDC_window);
        SelectWindow(PDC_window);
        PDC_mac_set_port();
    }
}

void PDC_reset_shell_mode(void)
{
    PDC_LOG(("PDC_reset_shell_mode() - called.\n"));
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER(i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER(i);
}

void PDC_set_resize_limits(const int new_min_lines, const int new_max_lines,
                           const int new_min_cols, const int new_max_cols)
{
    PDC_min_lines = new_min_lines;
    PDC_max_lines = new_max_lines;
    PDC_min_cols = new_min_cols;
    PDC_max_cols = new_max_cols;
    if (PDC_min_lines < 2)
        PDC_min_lines = 2;
    if (PDC_min_cols < 2)
        PDC_min_cols = 2;
}

bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content(int color, int *red, int *green, int *blue)
{
    const PACKED_RGB col = PDC_get_palette_entry(color);

    *red = DIVROUND(Get_RValue(col) * 1000, 255);
    *green = DIVROUND(Get_GValue(col) * 1000, 255);
    *blue = DIVROUND(Get_BValue(col) * 1000, 255);
    return OK;
}

int PDC_init_color(int color, int red, int green, int blue)
{
    const PACKED_RGB new_rgb = PACK_RGB(DIVROUND(red * 255, 1000),
                                        DIVROUND(green * 255, 1000),
                                        DIVROUND(blue * 255, 1000));

    if (!PDC_set_palette_entry(color, new_rgb))
    {
        if (curscr)
            curscr->_clear = TRUE;
    }
    return OK;
}
