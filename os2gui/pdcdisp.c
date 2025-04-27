/* Public Domain Curses */

#include "pdcwin.h"
#ifdef PDC_WIDE
#define USE_UNICODE_ACS_CHARS 1
#else
#define USE_UNICODE_ACS_CHARS 0
#endif
#include "../common/acs_defs.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

/* Cursors may be added to the 'shapes' array.  A 'shapes' string
defines the cursor as one or more rectangles,  separated by semicolons.
The coordinates of the upper left and lower right corners are given,
usually just as integers from zero to eight.  Thus,  "0488" means a
rectangle running from (0,4),  middle of the left side,  to (8,8),
bottom right corner:  a rectangle filling the bottom half of the
character cell. "0048" would fill the left half of the cell,  and
"0082;6088" would fill the top and bottom quarters of the cell.

   However,  a coordinate may be followed by a + or -,  and then by a
single-digit offset in pixels.  So "08-4" refers to a point on the
left-hand side of the character cell,  four pixels from the bottom. I
admit that the cursor descriptions themselves look a little strange!
But this way of describing cursors is compact and lends itself to some
pretty simple code.

   The first three lines are standard PDCurses cursors:  0=no cursor,
1=four-pixel thick line at bottom of the cell,  2="high-intensity",
i.e., a filled block.  The rest are extended cursors,  not currently
available in other PDCurses flavors. */

#define N_CURSORS 9

static void redraw_cursor_from_index( HPS hps, const int idx)
{
    const char *shapes[N_CURSORS] = {
        "",                         /* 0: invisible */
        "08-488",                    /* 1: normal:  four lines at bottom */
        "0088",                       /* 2: full block */
        "0088;0+10+18-18-1",           /* 3: outlined block */
        "28-368;4-10+34+18-3;2060+3",  /* 4: caret */
        "0488",                       /* 5: bottom half block */
        "2266",                      /* 6: central block     */
        "0385;3053;3558",           /* 7: cross */
        "0088;0+10+48-18-4"  };    /* 8: outlined block: heavy top/bottom*/
    const char *sptr;
    RECTL cell_rect;

    if (idx < 0 || N_CURSORS <= idx)
        sptr = shapes[1];
    else
        sptr = shapes[idx];
    PDC_screen_to_pixel(SP->curscol, SP->cursrow, &cell_rect);

    while( *sptr)
    {
        int i;
        LONG coords[4];
        RECTL rect;

        for( i = 0; i < 4; i++)
        {
            coords[i] = (( i & 1) ?
                         cell_rect.yTop - (PDC_cyChar * (*sptr - '0') + 4) / 8 :
                         cell_rect.xLeft + (PDC_cxChar * (*sptr - '0') + 4) / 8);
            sptr++;
            if( *sptr == '+' || *sptr == '-')
            {
                if( (*sptr == '+') ^ (i & 1))
                   coords[i] += sptr[1] - '0';
                else
                   coords[i] -= sptr[1] - '0';
                sptr += 2;
            }
        }
        rect.xLeft = coords[0];
        rect.yTop = coords[1];
        rect.xRight = coords[2];
        rect.yBottom = coords[3];
        WinInvertRect( hps, &rect);
        if( *sptr == ';')
            sptr++;
    }
}

/* PDC_current_cursor_state( ) determines which cursor,  if any,
is currently shown.  This may depend on the blink state.  Also,
if the window currently lacks the focus,  we show cursor 3 (a hollow
box) in place of any visible cursor.  */

static int PDC_current_cursor_state( void)
{
    const int shift_amount = (SP->blink_state ? 0 : 8);
    const int cursor_style_for_unfocussed_window =
               PDC_CURSOR( PDC_CURSOR_OUTLINE, PDC_CURSOR_OUTLINE);
    int cursor_style;

            /* for unfocussed windows, show an hollow box: */
    if( SP->visibility && (PDC_hWnd != WinQueryActiveWindow( HWND_DESKTOP)))
        cursor_style = cursor_style_for_unfocussed_window;
    else    /* otherwise,  just show the cursor "normally" */
        cursor_style = SP->visibility;
    return( (cursor_style >> shift_amount) & 0xff);
}

static void redraw_cursor( HPS hps)
{
    const int cursor_style = PDC_current_cursor_state( );

    if( cursor_style > 0 && cursor_style < N_CURSORS)
        redraw_cursor_from_index( hps, cursor_style);
}

/* position "hardware" cursor at (y, x).  We don't have a for-real hardware */
/* cursor in this version,  of course,  but we can fake it.  Note that much */
/* of the logic was borrowed from the SDL version.  In particular,  the     */
/* cursor is moved by first overwriting the "original" location.            */

void PDC_gotoyx(int row, int col)
{
    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

                /* clear the old cursor,  if it's on-screen: */
    if( SP->cursrow >= 0 && SP->curscol >= 0 &&
         SP->cursrow < SP->lines && SP->curscol < SP->cols)
    {
        const int temp_visibility = SP->visibility;

        SP->visibility = 0;
        PDC_transform_line_sliced( SP->cursrow, SP->curscol, 1,
                           curscr->_y[SP->cursrow] + SP->curscol);
        SP->visibility = temp_visibility;
    }

               /* ...then draw the new (assuming it's actually visible).    */
               /* This used to require some logic.  Now the redraw_cursor() */
               /* function figures out what cursor should be drawn, if any. */
    if( SP->visibility)
    {
        HPS hps = WinGetPS( PDC_hWnd) ;
        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);

        SP->curscol = col;
        SP->cursrow = row;
        redraw_cursor( hps);
        WinReleasePS( hps) ;
    }
}

int PDC_choose_a_new_font(char font[FACESIZE], int *fontsize)
{
    FONTDLG dlg;
    HPS hps = WinGetPS(PDC_hWnd);
    char new_font[FACESIZE];
    HWND hwndDlg;
    BOOL ok;

    memset(&dlg, 0, sizeof(dlg));
    strcpy(new_font, font);
    dlg.cbSize = sizeof(dlg);
    dlg.hpsScreen = hps;
    dlg.pszFamilyname = (PSZ)new_font;
    dlg.usFamilyBufLen = sizeof(new_font);
    dlg.fl = FNTS_FIXEDWIDTHONLY | FNTS_CENTER | FNTS_INITFROMFATTRS;
    dlg.fxPointSize = *fontsize << 16;
    dlg.pszPreview = (PSZ)"The quick brown fox jumps over the lazy dog.";
    dlg.clrFore = SYSCLR_WINDOWTEXT;
    dlg.clrBack = SYSCLR_WINDOW;

    hwndDlg = WinFontDlg(HWND_DESKTOP, PDC_hWnd, &dlg);

    WinReleasePS(hps);

    ok = hwndDlg != NULLHANDLE && dlg.lReturn == DID_OK;
    if (ok)
    {
        strcpy(font, new_font);
        *fontsize = dlg.fxPointSize >> 16;
    }

    return ok;
}

/* Implement certain alternate characters by special means, because they are
   not present in IBM 437, and are lacking in many fonts */
/* Return true if ch requires a special draw */
static int
do_special_draw(HPS hps, PRECTL rect, chtype ch)
{
    int dy = rect->yTop - rect->yBottom - 2;
    POINTL p;
    int y2 = 0;

    p.y = rect->yBottom + 1;
    /* Watcom C doesn't like 64 bit int in switch */
    switch ((unsigned)(ch & (A_CHARTEXT | A_ALTCHARSET)))
    {
    case ACS_S1:
#ifdef PDC_WIDE
    case 0x23BA:
#endif
        p.y += dy;
        y2 = p.y - 1;
        break;

    case ACS_S3:
#ifdef PDC_WIDE
    case 0x23BB:
#endif
        p.y += dy - dy/4;
        y2 = p.y - 1;
        break;

    case ACS_S7:
#ifdef PDC_WIDE
    case 0x23BC:
#endif
        p.y += dy/4;
        y2 = p.y + 1;
        break;

    case ACS_S9:
#ifdef PDC_WIDE
    case 0x23BD:
#endif
        y2 = p.y + 1;
        break;

    default:
        return FALSE;
    }
    p.x = rect->xLeft;
    GpiMove(hps, &p);
    p.x = rect->xRight - 1;
    GpiLine(hps, &p);
    if (ch & A_BOLD)
    {
        p.y = y2;
        p.x = rect->xLeft;
        GpiMove(hps, &p);
        p.x = rect->xRight - 1;
        GpiLine(hps, &p);
    }
    return TRUE;
}

/* OS/2 expects RGB triplets with red as the most significant byte; but
   PDCursesMod expects red to be least significant. Switch the components
   here, and also request the color index. */
static LONG switch_colors(HPS hps, LONG color)
{
    color = ((color & 0xFF0000) >> 16)
          |  (color & 0x00FF00)
          | ((color & 0x0000FF) << 16);
    color = GpiQueryColorIndex(hps, 0, color);
    return color;
}

/* Code pages used for rendering text.
   If PDC_WIDE is not in effect, the configured system code page renders
   normal characters. Alternate characters are drawn from code page 437,
   except the not-equals sign, which is drawn from code page 1275.
   If PDC_WIDE is defined, we use the system code page for ASCII (it renders
   faster than 1200). For non-ASCII, we use code page 1200, if we have it;
   other code pages are used if 1200 is not available.
   OS/2 Warp 4.52 and ArcaOS 5.1 have all of the code pages listed here.
   OS/2 Warp 4.0 has all except 1200.
   OS/2 Warp 3.0 has 437, 850, 852, 857 and 1004.
   OS/2 2.0 is documented as having 437, 850, 852, 857 and 1004. */
static const unsigned short code_pages[] = {
    437,    /* For alternative character set */
    1275,   /* For the not-equal sign and a few Unicode characters */
#ifdef PDC_WIDE
    1200,   /* DBCS supporting the Unicode Basic Multilingual Plane */
    850,    /* Latin-1 */
    852,    /* Latin-2 */
    857,    /* Turkish */
    1004,   /* Windows-1252 (1252 is same, but not present in older OS/2s) */
    1257,   /* Windows-1257 */
    869,    /* Greek */
    1251,   /* Windows-1251 */
#endif
};

enum CodePage {
    cp_native = 0,
    cp_437,
    cp_1275
#ifdef PDC_WIDE
    ,
    cp_1200,
    cp_850,
    cp_852,
    cp_857,
    cp_1004,
    cp_1257,
    cp_869,
    cp_1251
#endif
};

static bool have_cp[sizeof(code_pages)/sizeof(code_pages[0]) + 1];
static bool cp_inited = FALSE;

#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
static uint32_t *PDC_normalize(uint32_t ch, bool compose);
#endif

static void render_char(HPS hps, PPOINTL point, PRECTL rect, ULONG ch, int charset);
static void set_charset(HPS hps, enum CodePage code_page, int charset);

/* update the given physical line to look like the corresponding line in
curscr.

   NOTE that if x > 0,  we decrement it and srcp,  and increment the
length.  In other words,  we draw the preceding character,  too.  This
is done because,  at certain font sizes,  characters break out and
overwrite the preceding character.  That results in debris left on
the screen.

   The code also now increments the length only,  drawing one more
character (i.e.,  draws the character following the "text we really
want").  Again,  this helps to avoid debris left on the screen. */

static void PDC_transform_line_given_hps( HPS hps, const int lineno,
                             int x, int len, const chtype *srcp)
{
#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
    /* A selection of characters to be overstruck, to provide combining characters that
       cannot be supported via composition */
    static const unsigned short cchars[] = {
        /* 0x0300 */ 0x0060,
        /* 0x0301 */ 0x00B4,
        /* 0x0302 */ 0x005E,
        /* 0x0303 */ 0x02DC,
        /* 0x0304 */ 0x00AF,
        /* 0x0305 */ 0x203E,
        /* 0x0306 */ 0x02D8,
        /* 0x0307 */ 0x02D9,
        /* 0x0308 */ 0x00A8,
        /* 0x0309 */ ' ',
        /* 0x030A */ 0x02DA,
        /* 0x030B */ 0x02DD,
        /* 0x030C */ 0x02C7,
        /* 0x030D */ ' ',
        /* 0x030E */ ' ',
        /* 0x030F */ ' ',
        /* 0x0310 */ ' ',
        /* 0x0311 */ ' ',
        /* 0x0312 */ ' ',
        /* 0x0313 */ ' ',
        /* 0x0314 */ ' ',
        /* 0x0315 */ ' ',
        /* 0x0316 */ ' ',
        /* 0x0317 */ ' ',
        /* 0x0318 */ ' ',
        /* 0x0319 */ ' ',
        /* 0x031A */ ' ',
        /* 0x031B */ ' ',
        /* 0x031C */ ' ',
        /* 0x031D */ ' ',
        /* 0x031E */ ' ',
        /* 0x031F */ ' ',
        /* 0x0320 */ ' ',
        /* 0x0321 */ ' ',
        /* 0x0322 */ ' ',
        /* 0x0323 */ ' ',
        /* 0x0324 */ ' ',
        /* 0x0325 */ ' ',
        /* 0x0326 */ ' ',
        /* 0x0327 */ 0x00B8,
        /* 0x0328 */ 0x02DB,
        /* 0x0329 */ ' ',
        /* 0x032A */ ' ',
        /* 0x032B */ ' ',
        /* 0x032C */ ' ',
        /* 0x032D */ ' ',
        /* 0x032E */ ' ',
        /* 0x032F */ ' ',
        /* 0x0330 */ ' ',
        /* 0x0331 */ ' ',
        /* 0x0332 */ 0x005F,
        /* 0x0333 */ 0x2017,
        /* 0x0334 */ ' ',
        /* 0x0335 */ ' ',
        /* 0x0336 */ ' ',
        /* 0x0337 */ ' ',
        /* 0x0338 */ '/',
        /* 0x0339 */ ' ',
        /* 0x033A */ ' ',
        /* 0x033B */ ' ',
        /* 0x033C */ ' ',
        /* 0x033D */ ' ',
        /* 0x033E */ 0x2E2F,
        /* 0x033F */ ' ',
        /* 0x0340 */ ' ',
        /* 0x0341 */ ' ',
        /* 0x0342 */ 0x1FC0,
        /* 0x0343 */ 0x1FBD,
        /* 0x0344 */ 0x0385,
        /* 0x0345 */ 0x037A,
    };
#endif
    int cursor_overwritten = FALSE;
    FATTRS fattrs;
    STR8 synth;
    BOOL have_bold;
    PACKED_RGB foreground_rgb;
    PACKED_RGB background_rgb;
    RECTL clip_rect;
    int i;
    FONTMETRICS fm;

                     /* Seems to me as if the input text to this function */
    if( x < 0)       /* should _never_ be off-screen.  But it sometimes is. */
    {                /* Clipping is therefore necessary. */
        len += x;
        srcp -= x;
        x = 0;
    }
    if( len < SP->cols - x && (srcp[len] & A_CHARTEXT) < MAX_UNICODE)
       len++;    /* draw an extra char to avoid leaving garbage on screen */
    if( len > SP->cols - x)
        len = SP->cols - x;
    if( lineno >= SP->lines || len <= 0 || lineno < 0)
        return;
    assert( (srcp[len - 1] & A_CHARTEXT) != MAX_UNICODE);
    if( x && (srcp[-1] & A_CHARTEXT) < MAX_UNICODE)
    {                /* back up by one character to avoid */
        x--;         /* leaving garbage on the screen */
        len++;
        srcp--;
    }
    if( lineno == SP->cursrow && SP->curscol >= x && SP->curscol < x + len)
        if( PDC_current_cursor_state( ))
            cursor_overwritten = TRUE;

    /* Do we have a real bold font? */
    set_charset(hps, cp_native, 2);
    GpiQueryLogicalFont(hps, 2 + 1, (PSTR8)synth, &fattrs, sizeof(fattrs));
    have_bold = synth[0] == '\0';

    /* Erase the background */
    PDC_get_rgb_values( srcp[0], &foreground_rgb, &background_rgb);
    PDC_screen_to_pixel(x, lineno, &clip_rect);
    clip_rect.xRight = clip_rect.xLeft + PDC_cxChar;
    for (i = 1; i < len; ++i)
    {
        PACKED_RGB new_bg;
        PDC_get_rgb_values( srcp[i], &foreground_rgb, &new_bg);
        if (new_bg != background_rgb)
        {
            WinFillRect(hps, &clip_rect, switch_colors(hps, background_rgb));
            clip_rect.xLeft = clip_rect.xRight;
            background_rgb = new_bg;
        }
        clip_rect.xRight += PDC_cxChar;
    }
    if (x + i < SP->cols && (srcp[i] & A_CHARTEXT) == MAX_UNICODE)
        clip_rect.xRight += PDC_cxChar;
    WinFillRect(hps, &clip_rect, switch_colors(hps, background_rgb));

    /* Expand by one more column */
    if( len < SP->cols - x && (srcp[len] & A_CHARTEXT) < MAX_UNICODE)
       len++;
    if( x && (srcp[-1] & A_CHARTEXT) < MAX_UNICODE)
    {
        x--;
        len++;
        srcp--;
    }

    /* Render the text */
    GpiQueryFontMetrics(hps, sizeof(fm), &fm);
    for (i = 0; i < len; ++i)
    {
        const attr_t attrib = (attr_t)( srcp[i] & ~A_CHARTEXT);
        uint32_t cp_ch = (uint32_t)( srcp[i] & A_CHARTEXT);
        int charset;
#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
        uint32_t *seq = NULL;
#endif

        if (cp_ch == MAX_UNICODE)
            continue;

        PDC_get_rgb_values( attrib, &foreground_rgb, &background_rgb);
        foreground_rgb = switch_colors(hps, foreground_rgb);

        PDC_screen_to_pixel(x + i, lineno, &clip_rect);
        clip_rect.xRight = clip_rect.xLeft + PDC_cxChar;

#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
        if (cp_ch > MAX_UNICODE)
        {
            seq = PDC_normalize(cp_ch, TRUE);
            cp_ch = seq[0];
        }
#endif

        charset = 0;
        if ((attrib & A_BOLD) && have_bold)
            charset |= 2;
        if (attrib & A_ITALIC)
            charset |= 1;

        GpiSetColor(hps, foreground_rgb);
        if (!do_special_draw(hps, &clip_rect, srcp[i]))
        {
            POINTL pt;

#ifdef PDC_WIDE
            if (_is_altcharset(attrib))
                cp_ch = acs_map[cp_ch & 0xFF];
#else
            cp_ch = srcp[i] & (A_CHARTEXT | A_ALTCHARSET);
#endif

            pt.x = clip_rect.xLeft;
            pt.y = clip_rect.yBottom + fm.lMaxDescender;
            render_char(hps, &pt, &clip_rect, cp_ch, charset);
            if ((attrib & A_BOLD) && !have_bold)
            {
                /* Overstrike to make a bold font */
                ++pt.x;
                render_char(hps, &pt, &clip_rect, cp_ch, charset);
                --pt.x;
            }
#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
            if (seq)
            {
                unsigned j;
                for (j = 1; seq[j] != 0; ++j)
                {
                    uint32_t cch = seq[j];
                    if (0x0300 <= cch && cch < 0x0300 + sizeof(cchars)/sizeof(cchars[0]))
                    {
                        cch = cchars[cch - 0x0300];
                        render_char(hps, &pt, &clip_rect, cch, charset);
                        if ((attrib & A_BOLD) && !have_bold)
                        {
                            /* Overstrike to make a bold font */
                            ++pt.x;
                            render_char(hps, &pt, &clip_rect, cch, charset);
                            --pt.x;
                        }
                    }
                }
            }
#endif
        }
#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)
        free(seq);
#endif

        if (attrib & A_UNDERLINE)
        {
            POINTL pt;
            pt.x = clip_rect.xLeft;
            pt.y = clip_rect.yBottom;
            GpiMove(hps, &pt);
            pt.x = clip_rect.xRight - 1;
            GpiLine(hps, &pt);
        }

        if (attrib & A_STRIKEOUT)
        {
            POINTL pt;
            pt.x = clip_rect.xLeft;
            pt.y = (clip_rect.yBottom + clip_rect.yTop) / 2;
            GpiMove(hps, &pt);
            pt.x = clip_rect.xRight - 1;
            GpiLine(hps, &pt);
        }

        if (attrib & A_TOP)
        {
            POINTL pt;
            pt.x = clip_rect.xLeft;
            pt.y = clip_rect.yTop - 1;
            GpiMove(hps, &pt);
            pt.x = clip_rect.xRight - 1;
            GpiLine(hps, &pt);
        }

        if (attrib & A_LEFT)
        {
            POINTL pt;
            pt.x = clip_rect.xLeft;
            pt.y = clip_rect.yBottom;
            GpiMove(hps, &pt);
            pt.y = clip_rect.yTop - 1;
            GpiLine(hps, &pt);
        }

        if (attrib & A_RIGHT)
        {
            POINTL pt;
            pt.x = clip_rect.xLeft + PDC_cxChar - 1;
            pt.y = clip_rect.yBottom;
            GpiMove(hps, &pt);
            pt.y = clip_rect.yTop - 1;
            GpiLine(hps, &pt);
        }
    }

               /* ...did we step on the cursor?  If so,  redraw it: */
    if( cursor_overwritten)
        redraw_cursor( hps);
}

static void set_charset(HPS hps, enum CodePage code_page, int charset)
{
    struct font_type {
        const char *suffix;
        unsigned flags;
    };
    static struct font_type const font_types[] = {
        { "", 0 },
        { " Italic", FATTR_SEL_ITALIC },
        { " Bold", FATTR_SEL_BOLD },
        { " Bold Italic",  FATTR_SEL_BOLD | FATTR_SEL_ITALIC },
    };

    int cs = code_page*4 + charset + 1;
    BOOL rc = GpiSetCharSet(hps, cs);
    if (!rc)
    {
        FATTRS fattrs;
        LONG lmatch;
        const char *facename = "";
        /* GCC makes the facename parameter to GpiCreateLogFont a const
           pointer to STR8. Watcom uses a non-const pointer.  This causes
           warnings in one compiler or the other unless it is resolved here. */
#ifdef __GNUC__
        typedef const STR8 *p_facename;
#else
        typedef STR8 *p_facename;
#endif

        memset(&fattrs, 0, sizeof(fattrs));
        fattrs.usRecordLength = sizeof(fattrs);
        fattrs.fsType = FATTR_TYPE_ANTIALIASED;
        fattrs.fsFontUse = FATTR_FONTUSE_OUTLINE | FATTR_FONTUSE_TRANSFORMABLE;

        fattrs.usCodePage = (code_page == cp_native) ? 0 : code_pages[code_page-1];
        fattrs.fsSelection = 0;
        snprintf(fattrs.szFacename, sizeof(fattrs.szFacename), "%s%s",
                 PDC_font_name, font_types[charset].suffix);
        /* Null already written to last element fo fattrs.szFacename */
        lmatch = GpiCreateLogFont(hps, (p_facename)facename, cs, &fattrs);
        if (lmatch != FONT_MATCH || strcmp(fattrs.szFacename, PDC_font_name) == 0)
        {
            snprintf(fattrs.szFacename, sizeof(fattrs.szFacename), "%s",
                     PDC_font_name);
            fattrs.fsSelection = font_types[charset].flags;
            facename = "synth";
            GpiCreateLogFont(hps, (p_facename)facename, cs, &fattrs);
        }
    }
    GpiSetCharSet(hps, cs);
}

struct CharRec {
    unsigned short uni_ch;
    struct {
        unsigned char code_page;
        unsigned char code_point;
    } glyphs[3];
};

#ifdef PDC_WIDE
/* Table of Unicode characters, and the code pages and code points needed to
   render them. Where multiple code points are listed, they are overstruck. */
static const struct CharRec characters[] = {
    { 0x00A0, { { cp_850, 0xFF } } },
    { 0x00A1, { { cp_850, 0xAD } } },
    { 0x00A2, { { cp_850, 0xBD } } },
    { 0x00A3, { { cp_850, 0x9C } } },
    { 0x00A4, { { cp_850, 0xCF } } },
    { 0x00A5, { { cp_850, 0xBE } } },
    { 0x00A6, { { cp_850, 0xDD } } },
    { 0x00A7, { { cp_850, 0xF5 } } },
    { 0x00A8, { { cp_850, 0xF9 } } },
    { 0x00A9, { { cp_850, 0xB8 } } },
    { 0x00AA, { { cp_850, 0xA6 } } },
    { 0x00AB, { { cp_850, 0xAE } } },
    { 0x00AC, { { cp_850, 0xAA } } },
    { 0x00AD, { { cp_850, 0xF0 } } },
    { 0x00AE, { { cp_850, 0xA9 } } },
    { 0x00AF, { { cp_850, 0xEE } } },
    { 0x00B0, { { cp_850, 0xF8 } } },
    { 0x00B1, { { cp_850, 0xF1 } } },
    { 0x00B2, { { cp_850, 0xFD } } },
    { 0x00B3, { { cp_850, 0xFC } } },
    { 0x00B4, { { cp_850, 0xEF } } },
    { 0x00B5, { { cp_850, 0xE6 } } },
    { 0x00B6, { { cp_850, 0xF4 } } },
    { 0x00B7, { { cp_850, 0xFA } } },
    { 0x00B8, { { cp_850, 0xF7 } } },
    { 0x00B9, { { cp_850, 0xFB } } },
    { 0x00BA, { { cp_850, 0xA7 } } },
    { 0x00BB, { { cp_850, 0xAF } } },
    { 0x00BC, { { cp_850, 0xAC } } },
    { 0x00BD, { { cp_850, 0xAB } } },
    { 0x00BE, { { cp_850, 0xF3 } } },
    { 0x00BF, { { cp_850, 0xA8 } } },
    { 0x00C0, { { cp_850, 0xB7 } } },
    { 0x00C1, { { cp_850, 0xB5 } } },
    { 0x00C2, { { cp_850, 0xB6 } } },
    { 0x00C3, { { cp_850, 0xC7 } } },
    { 0x00C4, { { cp_850, 0x8E } } },
    { 0x00C5, { { cp_850, 0x8F } } },
    { 0x00C6, { { cp_850, 0x92 } } },
    { 0x00C7, { { cp_850, 0x80 } } },
    { 0x00C8, { { cp_850, 0xD4 } } },
    { 0x00C9, { { cp_850, 0x90 } } },
    { 0x00CA, { { cp_850, 0xD2 } } },
    { 0x00CB, { { cp_850, 0xD3 } } },
    { 0x00CC, { { cp_850, 0xDE } } },
    { 0x00CD, { { cp_850, 0xD6 } } },
    { 0x00CE, { { cp_850, 0xD7 } } },
    { 0x00CF, { { cp_850, 0xD8 } } },
    { 0x00D0, { { cp_850, 0xD1 } } },
    { 0x00D1, { { cp_850, 0xA5 } } },
    { 0x00D2, { { cp_850, 0xE3 } } },
    { 0x00D3, { { cp_850, 0xE0 } } },
    { 0x00D4, { { cp_850, 0xE2 } } },
    { 0x00D5, { { cp_850, 0xE5 } } },
    { 0x00D6, { { cp_850, 0x99 } } },
    { 0x00D7, { { cp_850, 0x9E } } },
    { 0x00D8, { { cp_850, 0x9D } } },
    { 0x00D9, { { cp_850, 0xEB } } },
    { 0x00DA, { { cp_850, 0xE9 } } },
    { 0x00DB, { { cp_850, 0xEA } } },
    { 0x00DC, { { cp_850, 0x9A } } },
    { 0x00DD, { { cp_850, 0xED } } },
    { 0x00DE, { { cp_850, 0xE8 } } },
    { 0x00DF, { { cp_850, 0xE1 } } },
    { 0x00E0, { { cp_850, 0x85 } } },
    { 0x00E1, { { cp_850, 0xA0 } } },
    { 0x00E2, { { cp_850, 0x83 } } },
    { 0x00E3, { { cp_850, 0xC6 } } },
    { 0x00E4, { { cp_850, 0x84 } } },
    { 0x00E5, { { cp_850, 0x86 } } },
    { 0x00E6, { { cp_850, 0x91 } } },
    { 0x00E7, { { cp_850, 0x87 } } },
    { 0x00E8, { { cp_850, 0x8A } } },
    { 0x00E9, { { cp_850, 0x82 } } },
    { 0x00EA, { { cp_850, 0x88 } } },
    { 0x00EB, { { cp_850, 0x89 } } },
    { 0x00EC, { { cp_850, 0x8D } } },
    { 0x00ED, { { cp_850, 0xA1 } } },
    { 0x00EE, { { cp_850, 0x8C } } },
    { 0x00EF, { { cp_850, 0x8B } } },
    { 0x00F0, { { cp_850, 0xD0 } } },
    { 0x00F1, { { cp_850, 0xA4 } } },
    { 0x00F2, { { cp_850, 0x95 } } },
    { 0x00F3, { { cp_850, 0xA2 } } },
    { 0x00F4, { { cp_850, 0x93 } } },
    { 0x00F5, { { cp_850, 0xE4 } } },
    { 0x00F6, { { cp_850, 0x94 } } },
    { 0x00F7, { { cp_850, 0xF6 } } },
    { 0x00F8, { { cp_850, 0x9B } } },
    { 0x00F9, { { cp_850, 0x97 } } },
    { 0x00FA, { { cp_850, 0xA3 } } },
    { 0x00FB, { { cp_850, 0x96 } } },
    { 0x00FC, { { cp_850, 0x81 } } },
    { 0x00FD, { { cp_850, 0xEC } } },
    { 0x00FE, { { cp_850, 0xE7 } } },
    { 0x00FF, { { cp_850, 0x98 } } },
    { 0x0100, { { cp_1257, 0xC2 } } },
    { 0x0101, { { cp_1257, 0xE2 } } },
    { 0x0102, { { cp_852, 0xC6 } } },
    { 0x0103, { { cp_852, 0xC7 } } },
    { 0x0104, { { cp_852, 0xA4 } } },
    { 0x0105, { { cp_852, 0xA5 } } },
    { 0x0106, { { cp_852, 0x8F } } },
    { 0x0107, { { cp_852, 0x86 } } },
    { 0x0108, { { cp_850, 'C' }, { cp_850, '^' } } },
    { 0x0109, { { cp_850, 'c' }, { cp_850, '^' } } },
    { 0x010A, { { cp_852, 'C' }, { cp_852, 0xFA } } },
    { 0x010B, { { cp_852, 'c' }, { cp_852, 0xFA } } },
    { 0x010C, { { cp_852, 0xAC } } },
    { 0x010D, { { cp_852, 0x9F } } },
    { 0x010E, { { cp_852, 0xD2 } } },
    { 0x010F, { { cp_852, 0xD4 } } },
    { 0x0110, { { cp_852, 0xD1 } } },
    { 0x0111, { { cp_852, 0xD0 } } },
    { 0x0112, { { cp_1257, 0xC7 } } },
    { 0x0113, { { cp_1257, 0xE7 } } },
    { 0x0114, { { cp_852, 'E' }, { cp_852, 0xF4 } } },
    { 0x0115, { { cp_852, 'e' }, { cp_852, 0xF4 } } },
    { 0x0116, { { cp_1257, 0xCB } } },
    { 0x0117, { { cp_1257, 0xEB } } },
    { 0x0118, { { cp_852, 0xA8 } } },
    { 0x0119, { { cp_852, 0xA9 } } },
    { 0x011A, { { cp_852, 0xB7 } } },
    { 0x011B, { { cp_852, 0xD8 } } },
    { 0x011C, { { cp_850, 'G' }, { cp_850, '^' } } },
    { 0x011D, { { cp_850, 'g' }, { cp_850, '^' } } },
    { 0x011E, { { cp_857, 0xA6 } } },
    { 0x011F, { { cp_857, 0xA7 } } },
    { 0x0120, { { cp_852, 'G' }, { cp_852, 0xFA } } },
    { 0x0121, { { cp_852, 'g' }, { cp_852, 0xFA } } },
    { 0x0122, { { cp_1257, 0xCC } } },
    { 0x0123, { { cp_1257, 0xEC } } },
    { 0x0124, { { cp_850, 'H' }, { cp_850, '^' } } },
    { 0x0125, { { cp_850, 'h' }, { cp_850, '^' } } },
    { 0x0128, { { cp_1004, 'I' }, { cp_1004, 0x98 } } },
    { 0x0129, { { cp_1004, 'i' }, { cp_1004, 0x98 } } },
    { 0x012A, { { cp_1257, 0xCE } } },
    { 0x012B, { { cp_1257, 0xEE } } },
    { 0x012C, { { cp_852, 'I' }, { cp_852, 0xF4 } } },
    { 0x012D, { { cp_852, 'i' }, { cp_852, 0xF4 } } },
    { 0x012E, { { cp_1257, 0xC1 } } },
    { 0x012F, { { cp_1257, 0xE1 } } },
    { 0x0130, { { cp_857, 0x98 } } },
    { 0x0131, { { cp_857, 0x8D } } },
    { 0x0134, { { cp_850, 'J' }, { cp_850, '^' } } },
    { 0x0135, { { cp_850, 'j' }, { cp_850, '^' } } },
    { 0x0136, { { cp_1257, 0xCD } } },
    { 0x0137, { { cp_1257, 0xED } } },
    { 0x0139, { { cp_852, 0x91 } } },
    { 0x013A, { { cp_852, 0x92 } } },
    { 0x013B, { { cp_1257, 0xCF } } },
    { 0x013C, { { cp_1257, 0xEF } } },
    { 0x013D, { { cp_852, 0x95 } } },
    { 0x013E, { { cp_852, 0x96 } } },
    { 0x0141, { { cp_852, 0x9D } } },
    { 0x0142, { { cp_852, 0x88 } } },
    { 0x0143, { { cp_852, 0xE3 } } },
    { 0x0144, { { cp_852, 0xE4 } } },
    { 0x0145, { { cp_1257, 0xD2 } } },
    { 0x0146, { { cp_1257, 0xF2 } } },
    { 0x0147, { { cp_852, 0xD5 } } },
    { 0x0148, { { cp_852, 0xE5 } } },
    { 0x014C, { { cp_1257, 0xD4 } } },
    { 0x014D, { { cp_1257, 0xF4 } } },
    { 0x014E, { { cp_852, 'O' }, { cp_852, 0xF4 } } },
    { 0x014F, { { cp_852, 'o' }, { cp_852, 0xF4 } } },
    { 0x0150, { { cp_852, 0x8A } } },
    { 0x0151, { { cp_852, 0x8B } } },
    { 0x0152, { { cp_1004, 0x8C } } },
    { 0x0153, { { cp_1004, 0x9C } } },
    { 0x0154, { { cp_852, 0xE8 } } },
    { 0x0155, { { cp_852, 0xEA } } },
    { 0x0156, { { cp_1257, 0xAA } } },
    { 0x0157, { { cp_1257, 0xBA } } },
    { 0x0158, { { cp_852, 0xFC } } },
    { 0x0159, { { cp_852, 0xFD } } },
    { 0x015A, { { cp_852, 0x97 } } },
    { 0x015B, { { cp_852, 0x98 } } },
    { 0x015C, { { cp_850, 'S' }, { cp_850, '^' } } },
    { 0x015D, { { cp_850, 's' }, { cp_850, '^' } } },
    { 0x015E, { { cp_852, 0xB8 } } },
    { 0x015F, { { cp_852, 0xAD } } },
    { 0x0160, { { cp_852, 0xE6 } } },
    { 0x0161, { { cp_852, 0xE7 } } },
    { 0x0162, { { cp_852, 0xDD } } },
    { 0x0163, { { cp_852, 0xEE } } },
    { 0x0164, { { cp_852, 0x9B } } },
    { 0x0165, { { cp_852, 0x9C } } },
    { 0x0168, { { cp_1004, 'U' }, { cp_1004, 0x98 } } },
    { 0x0169, { { cp_1004, 'u' }, { cp_1004, 0x98 } } },
    { 0x016A, { { cp_1257, 0xDB } } },
    { 0x016B, { { cp_1257, 0xFB } } },
    { 0x016C, { { cp_852, 'U' }, { cp_852, 0xF4 } } },
    { 0x016D, { { cp_852, 'u' }, { cp_852, 0xF4 } } },
    { 0x016E, { { cp_852, 0xDE } } },
    { 0x016F, { { cp_852, 0x85 } } },
    { 0x0170, { { cp_852, 0xEB } } },
    { 0x0171, { { cp_852, 0xFB } } },
    { 0x0172, { { cp_1257, 0xD8 } } },
    { 0x0173, { { cp_1257, 0xF8 } } },
    { 0x0174, { { cp_850, 'W' }, { cp_850, '^' } } },
    { 0x0175, { { cp_850, 'w' }, { cp_850, '^' } } },
    { 0x0176, { { cp_850, 'Y' }, { cp_850, '^' } } },
    { 0x0177, { { cp_850, 'y' }, { cp_850, '^' } } },
    { 0x0178, { { cp_1004, 0x9F } } },
    { 0x0179, { { cp_852, 0x8D } } },
    { 0x017A, { { cp_852, 0xAB } } },
    { 0x017B, { { cp_852, 0xBD } } },
    { 0x017C, { { cp_852, 0xBE } } },
    { 0x017D, { { cp_852, 0xA6 } } },
    { 0x017E, { { cp_852, 0xA7 } } },
    { 0x0192, { { cp_850, 0x9F } } },
    { 0x01CD, { { cp_852, 'A' }, { cp_852, 0xF3 } } },
    { 0x01CE, { { cp_852, 'a' }, { cp_852, 0xF3 } } },
    { 0x01CF, { { cp_852, 'I' }, { cp_852, 0xF3 } } },
    { 0x01D0, { { cp_852, 'i' }, { cp_852, 0xF3 } } },
    { 0x01D1, { { cp_852, 'O' }, { cp_852, 0xF3 } } },
    { 0x01D2, { { cp_852, 'o' }, { cp_852, 0xF3 } } },
    { 0x01D3, { { cp_852, 'U' }, { cp_852, 0xF3 } } },
    { 0x01D4, { { cp_852, 'u' }, { cp_852, 0xF3 } } },
    { 0x01D5, { { cp_850, 0x9A }, { cp_850, 0xEE } } },
    { 0x01D6, { { cp_850, 0x81 }, { cp_850, 0xEE } } },
    { 0x01D7, { { cp_850, 0x9A }, { cp_850, 0xEF } } },
    { 0x01D8, { { cp_850, 0x81 }, { cp_850, 0xEF } } },
    { 0x01D9, { { cp_850, 0x9A }, { cp_852, 0xF3 } } },
    { 0x01DA, { { cp_850, 0x81 }, { cp_852, 0xF3 } } },
    { 0x01DB, { { cp_850, 0x9A }, { cp_850, '`' } } },
    { 0x01DC, { { cp_850, 0x81 }, { cp_850, '`' } } },
    { 0x01DE, { { cp_850, 0x8E }, { cp_850, 0xEE } } },
    { 0x01DF, { { cp_850, 0x84 }, { cp_850, 0xEE } } },
    { 0x01E0, { { cp_852, 'A' }, { cp_852, 0xFA }, { cp_850, 0xEE } } },
    { 0x01E1, { { cp_852, 'a' }, { cp_852, 0xFA }, { cp_850, 0xEE } } },
    { 0x01E2, { { cp_850, 0x92 }, { cp_850, 0xEE } } },
    { 0x01E3, { { cp_850, 0x91 }, { cp_850, 0xEE } } },
    { 0x01E6, { { cp_852, 'G' }, { cp_852, 0xF3 } } },
    { 0x01E7, { { cp_852, 'g' }, { cp_852, 0xF3 } } },
    { 0x01E8, { { cp_852, 'K' }, { cp_852, 0xF3 } } },
    { 0x01E9, { { cp_852, 'k' }, { cp_852, 0xF3 } } },
    { 0x01EA, { { cp_852, 'O' }, { cp_852, 0xF2 } } },
    { 0x01EB, { { cp_852, 'o' }, { cp_852, 0xF2 } } },
    { 0x01EC, { { cp_852, 'O' }, { cp_852, 0xF2 }, { cp_850, 0xEE } } },
    { 0x01ED, { { cp_852, 'o' }, { cp_852, 0xF2 }, { cp_850, 0xEE } } },
    { 0x01F0, { { cp_852, 'j' }, { cp_852, 0xF3 } } },
    { 0x01F4, { { cp_850, 'G' }, { cp_850, 0xEF } } },
    { 0x01F5, { { cp_850, 'g' }, { cp_850, 0xEF } } },
    { 0x01F8, { { cp_850, 'N' }, { cp_850, '`' } } },
    { 0x01F9, { { cp_850, 'n' }, { cp_850, '`' } } },
    { 0x01FA, { { cp_850, 0x8F }, { cp_850, 0xEF } } },
    { 0x01FB, { { cp_850, 0x86 }, { cp_850, 0xEF } } },
    { 0x01FC, { { cp_850, 0x92 }, { cp_850, 0xEF } } },
    { 0x01FD, { { cp_850, 0x91 }, { cp_850, 0xEF } } },
    { 0x01FE, { { cp_850, 0x9D }, { cp_850, 0xEF } } },
    { 0x01FF, { { cp_850, 0x9B }, { cp_850, 0xEF } } },
    { 0x021E, { { cp_852, 'H' }, { cp_852, 0xF3 } } },
    { 0x021F, { { cp_852, 'h' }, { cp_852, 0xF3 } } },
    { 0x0226, { { cp_852, 'A' }, { cp_852, 0xFA } } },
    { 0x0227, { { cp_852, 'a' }, { cp_852, 0xFA } } },
    { 0x0228, { { cp_850, 'E' }, { cp_850, 0xF7 } } },
    { 0x0229, { { cp_850, 'e' }, { cp_850, 0xF7 } } },
    { 0x022A, { { cp_850, 0x99 }, { cp_850, 0xEE } } },
    { 0x022B, { { cp_850, 0x94 }, { cp_850, 0xEE } } },
    { 0x022C, { { cp_850, 0xE5 }, { cp_850, 0xEE } } },
    { 0x022D, { { cp_850, 0xE4 }, { cp_850, 0xEE } } },
    { 0x022E, { { cp_852, 'O' }, { cp_852, 0xFA } } },
    { 0x022F, { { cp_852, 'o' }, { cp_852, 0xFA } } },
    { 0x0230, { { cp_852, 'O' }, { cp_852, 0xFA }, { cp_850, 0xEE } } },
    { 0x0231, { { cp_852, 'o' }, { cp_852, 0xFA }, { cp_850, 0xEE } } },
    { 0x0232, { { cp_850, 'Y' }, { cp_850, 0xEE } } },
    { 0x0233, { { cp_850, 'y' }, { cp_850, 0xEE } } },
    { 0x02C6, { { cp_1004, 0x88 } } },
    { 0x02C7, { { cp_852, 0xF3 } } },
    { 0x02D8, { { cp_852, 0xF4 } } },
    { 0x02D9, { { cp_852, 0xFA } } },
    { 0x02DA, { { cp_1275, 0xFB } } },
    { 0x02DB, { { cp_852, 0xF2 } } },
    { 0x02DC, { { cp_1004, 0x98 } } },
    { 0x02DD, { { cp_852, 0xF1 } } },
    { 0x0384, { { cp_869, 0xEF } } },
    { 0x0385, { { cp_869, 0xF7 } } },
    { 0x0386, { { cp_869, 0x86 } } },
    { 0x0387, { { cp_869, 0x88 } } },
    { 0x0388, { { cp_869, 0x8D } } },
    { 0x0389, { { cp_869, 0x8F } } },
    { 0x038A, { { cp_869, 0x90 } } },
    { 0x038C, { { cp_869, 0x92 } } },
    { 0x038E, { { cp_869, 0x95 } } },
    { 0x038F, { { cp_869, 0x98 } } },
    { 0x0390, { { cp_869, 0xA1 } } },
    { 0x0391, { { cp_869, 0xA4 } } },
    { 0x0392, { { cp_869, 0xA5 } } },
    { 0x0393, { { cp_869, 0xA6 } } },
    { 0x0394, { { cp_869, 0xA7 } } },
    { 0x0395, { { cp_869, 0xA8 } } },
    { 0x0396, { { cp_869, 0xA9 } } },
    { 0x0397, { { cp_869, 0xAA } } },
    { 0x0398, { { cp_869, 0xAC } } },
    { 0x0399, { { cp_869, 0xAD } } },
    { 0x039A, { { cp_869, 0xB5 } } },
    { 0x039B, { { cp_869, 0xB6 } } },
    { 0x039C, { { cp_869, 0xB7 } } },
    { 0x039D, { { cp_869, 0xB8 } } },
    { 0x039E, { { cp_869, 0xBD } } },
    { 0x039F, { { cp_869, 0xBE } } },
    { 0x03A0, { { cp_869, 0xC6 } } },
    { 0x03A1, { { cp_869, 0xC7 } } },
    { 0x03A3, { { cp_869, 0xCF } } },
    { 0x03A4, { { cp_869, 0xD0 } } },
    { 0x03A5, { { cp_869, 0xD1 } } },
    { 0x03A6, { { cp_869, 0xD2 } } },
    { 0x03A7, { { cp_869, 0xD3 } } },
    { 0x03A8, { { cp_869, 0xD4 } } },
    { 0x03A9, { { cp_869, 0xD5 } } },
    { 0x03AA, { { cp_869, 0x91 } } },
    { 0x03AB, { { cp_869, 0x96 } } },
    { 0x03AC, { { cp_869, 0x9B } } },
    { 0x03AD, { { cp_869, 0x9D } } },
    { 0x03AE, { { cp_869, 0x9E } } },
    { 0x03AF, { { cp_869, 0x9F } } },
    { 0x03B0, { { cp_869, 0xFC } } },
    { 0x03B1, { { cp_869, 0xD6 } } },
    { 0x03B2, { { cp_869, 0xD7 } } },
    { 0x03B3, { { cp_869, 0xD8 } } },
    { 0x03B4, { { cp_869, 0xDD } } },
    { 0x03B5, { { cp_869, 0xDE } } },
    { 0x03B6, { { cp_869, 0xE0 } } },
    { 0x03B7, { { cp_869, 0xE1 } } },
    { 0x03B8, { { cp_869, 0xE2 } } },
    { 0x03B9, { { cp_869, 0xE3 } } },
    { 0x03BA, { { cp_869, 0xE4 } } },
    { 0x03BB, { { cp_869, 0xE5 } } },
    { 0x03BC, { { cp_869, 0xE6 } } },
    { 0x03BD, { { cp_869, 0xE7 } } },
    { 0x03BE, { { cp_869, 0xE8 } } },
    { 0x03BF, { { cp_869, 0xE9 } } },
    { 0x03C0, { { cp_869, 0xEA } } },
    { 0x03C1, { { cp_869, 0xEB } } },
    { 0x03C2, { { cp_869, 0xED } } },
    { 0x03C3, { { cp_869, 0xEC } } },
    { 0x03C4, { { cp_869, 0xEE } } },
    { 0x03C5, { { cp_869, 0xF2 } } },
    { 0x03C6, { { cp_869, 0xF3 } } },
    { 0x03C7, { { cp_869, 0xF4 } } },
    { 0x03C8, { { cp_869, 0xF6 } } },
    { 0x03C9, { { cp_869, 0xFA } } },
    { 0x03CA, { { cp_869, 0xA0 } } },
    { 0x03CB, { { cp_869, 0xFB } } },
    { 0x03CC, { { cp_869, 0xA2 } } },
    { 0x03CD, { { cp_869, 0xA3 } } },
    { 0x03CE, { { cp_869, 0xFD } } },
    { 0x0400, { { cp_1251, 0xC5 }, { cp_1251, '`' } } },
    { 0x0401, { { cp_1251, 0xA8 } } },
    { 0x0402, { { cp_1251, 0x80 } } },
    { 0x0403, { { cp_1251, 0x81 } } },
    { 0x0404, { { cp_1251, 0xAA } } },
    { 0x0405, { { cp_1251, 0xBD } } },
    { 0x0406, { { cp_1251, 0xB2 } } },
    { 0x0407, { { cp_1251, 0xAF } } },
    { 0x0408, { { cp_1251, 0xA3 } } },
    { 0x0409, { { cp_1251, 0x8A } } },
    { 0x040A, { { cp_1251, 0x8C } } },
    { 0x040B, { { cp_1251, 0x8E } } },
    { 0x040C, { { cp_1251, 0x8D } } },
    { 0x040D, { { cp_1251, 0xC8 }, { cp_1251, '`' } } },
    { 0x040E, { { cp_1251, 0xA1 } } },
    { 0x040F, { { cp_1251, 0x8F } } },
    { 0x0410, { { cp_1251, 0xC0 } } },
    { 0x0411, { { cp_1251, 0xC1 } } },
    { 0x0412, { { cp_1251, 0xC2 } } },
    { 0x0413, { { cp_1251, 0xC3 } } },
    { 0x0414, { { cp_1251, 0xC4 } } },
    { 0x0415, { { cp_1251, 0xC5 } } },
    { 0x0416, { { cp_1251, 0xC6 } } },
    { 0x0417, { { cp_1251, 0xC7 } } },
    { 0x0418, { { cp_1251, 0xC8 } } },
    { 0x0419, { { cp_1251, 0xC9 } } },
    { 0x041A, { { cp_1251, 0xCA } } },
    { 0x041B, { { cp_1251, 0xCB } } },
    { 0x041C, { { cp_1251, 0xCC } } },
    { 0x041D, { { cp_1251, 0xCD } } },
    { 0x041E, { { cp_1251, 0xCE } } },
    { 0x041F, { { cp_1251, 0xCF } } },
    { 0x0420, { { cp_1251, 0xD0 } } },
    { 0x0421, { { cp_1251, 0xD1 } } },
    { 0x0422, { { cp_1251, 0xD2 } } },
    { 0x0423, { { cp_1251, 0xD3 } } },
    { 0x0424, { { cp_1251, 0xD4 } } },
    { 0x0425, { { cp_1251, 0xD5 } } },
    { 0x0426, { { cp_1251, 0xD6 } } },
    { 0x0427, { { cp_1251, 0xD7 } } },
    { 0x0428, { { cp_1251, 0xD8 } } },
    { 0x0429, { { cp_1251, 0xD9 } } },
    { 0x042A, { { cp_1251, 0xDA } } },
    { 0x042B, { { cp_1251, 0xDB } } },
    { 0x042C, { { cp_1251, 0xDC } } },
    { 0x042D, { { cp_1251, 0xDD } } },
    { 0x042E, { { cp_1251, 0xDE } } },
    { 0x042F, { { cp_1251, 0xDF } } },
    { 0x0430, { { cp_1251, 0xE0 } } },
    { 0x0431, { { cp_1251, 0xE1 } } },
    { 0x0432, { { cp_1251, 0xE2 } } },
    { 0x0433, { { cp_1251, 0xE3 } } },
    { 0x0434, { { cp_1251, 0xE4 } } },
    { 0x0435, { { cp_1251, 0xE5 } } },
    { 0x0436, { { cp_1251, 0xE6 } } },
    { 0x0437, { { cp_1251, 0xE7 } } },
    { 0x0438, { { cp_1251, 0xE8 } } },
    { 0x0439, { { cp_1251, 0xE9 } } },
    { 0x043A, { { cp_1251, 0xEA } } },
    { 0x043B, { { cp_1251, 0xEB } } },
    { 0x043C, { { cp_1251, 0xEC } } },
    { 0x043D, { { cp_1251, 0xED } } },
    { 0x043E, { { cp_1251, 0xEE } } },
    { 0x043F, { { cp_1251, 0xEF } } },
    { 0x0440, { { cp_1251, 0xF0 } } },
    { 0x0441, { { cp_1251, 0xF1 } } },
    { 0x0442, { { cp_1251, 0xF2 } } },
    { 0x0443, { { cp_1251, 0xF3 } } },
    { 0x0444, { { cp_1251, 0xF4 } } },
    { 0x0445, { { cp_1251, 0xF5 } } },
    { 0x0446, { { cp_1251, 0xF6 } } },
    { 0x0447, { { cp_1251, 0xF7 } } },
    { 0x0448, { { cp_1251, 0xF8 } } },
    { 0x0449, { { cp_1251, 0xF9 } } },
    { 0x044A, { { cp_1251, 0xFA } } },
    { 0x044B, { { cp_1251, 0xFB } } },
    { 0x044C, { { cp_1251, 0xFC } } },
    { 0x044D, { { cp_1251, 0xFD } } },
    { 0x044E, { { cp_1251, 0xFE } } },
    { 0x044F, { { cp_1251, 0xFF } } },
    { 0x0450, { { cp_1251, 0xE5 }, { cp_1251, '`' } } },
    { 0x0451, { { cp_1251, 0xB8 } } },
    { 0x0452, { { cp_1251, 0x90 } } },
    { 0x0453, { { cp_1251, 0x83 } } },
    { 0x0454, { { cp_1251, 0xBA } } },
    { 0x0455, { { cp_1251, 0xBE } } },
    { 0x0456, { { cp_1251, 0xB3 } } },
    { 0x0457, { { cp_1251, 0xBF } } },
    { 0x0458, { { cp_1251, 0xBC } } },
    { 0x0459, { { cp_1251, 0x9A } } },
    { 0x045A, { { cp_1251, 0x9C } } },
    { 0x045B, { { cp_1251, 0x9E } } },
    { 0x045C, { { cp_1251, 0x9D } } },
    { 0x045D, { { cp_1251, 0xE8 }, { cp_1251, '`' } } },
    { 0x045E, { { cp_1251, 0xA2 } } },
    { 0x045F, { { cp_1251, 0x9F } } },
    { 0x0490, { { cp_1251, 0xA5 } } },
    { 0x0491, { { cp_1251, 0xB4 } } },
    { 0x04C1, { { cp_1251, 0xC6 }, { cp_852, 0xF4 } } },
    { 0x04C2, { { cp_1251, 0xE6 }, { cp_852, 0xF4 } } },
    { 0x04D0, { { cp_1251, 0xC0 }, { cp_852, 0xF4 } } },
    { 0x04D1, { { cp_1251, 0xE0 }, { cp_852, 0xF4 } } },
    { 0x04D2, { { cp_1251, 0xC0 }, { cp_850, 0xF9 } } },
    { 0x04D3, { { cp_1251, 0xE0 }, { cp_850, 0xF9 } } },
    { 0x04D6, { { cp_1251, 0xC5 }, { cp_852, 0xF4 } } },
    { 0x04D7, { { cp_1251, 0xE5 }, { cp_852, 0xF4 } } },
    { 0x04DC, { { cp_1251, 0xC6 }, { cp_850, 0xF9 } } },
    { 0x04DD, { { cp_1251, 0xE6 }, { cp_850, 0xF9 } } },
    { 0x04DE, { { cp_1251, 0xC7 }, { cp_850, 0xF9 } } },
    { 0x04DF, { { cp_1251, 0xE7 }, { cp_850, 0xF9 } } },
    { 0x04E2, { { cp_1251, 0xC8 }, { cp_850, 0xEE } } },
    { 0x04E3, { { cp_1251, 0xE8 }, { cp_850, 0xEE } } },
    { 0x04E4, { { cp_1251, 0xC8 }, { cp_850, 0xF9 } } },
    { 0x04E5, { { cp_1251, 0xE8 }, { cp_850, 0xF9 } } },
    { 0x04E6, { { cp_1251, 0xCE }, { cp_850, 0xF9 } } },
    { 0x04E7, { { cp_1251, 0xEE }, { cp_850, 0xF9 } } },
    { 0x04EC, { { cp_1251, 0xDD }, { cp_850, 0xF9 } } },
    { 0x04ED, { { cp_1251, 0xFD }, { cp_850, 0xF9 } } },
    { 0x04EE, { { cp_1251, 0xD3 }, { cp_850, 0xEE } } },
    { 0x04EF, { { cp_1251, 0xF3 }, { cp_850, 0xEE } } },
    { 0x04F0, { { cp_1251, 0xD3 }, { cp_850, 0xF9 } } },
    { 0x04F1, { { cp_1251, 0xF3 }, { cp_850, 0xF9 } } },
    { 0x04F2, { { cp_1251, 0xD3 }, { cp_852, 0xF1 } } },
    { 0x04F3, { { cp_1251, 0xF3 }, { cp_852, 0xF1 } } },
    { 0x04F4, { { cp_1251, 0xD7 }, { cp_850, 0xF9 } } },
    { 0x04F5, { { cp_1251, 0xF7 }, { cp_850, 0xF9 } } },
    { 0x04F8, { { cp_1251, 0xDB }, { cp_850, 0xF9 } } },
    { 0x04F9, { { cp_1251, 0xFB }, { cp_850, 0xF9 } } },
    { 0x1E02, { { cp_852, 'B' }, { cp_852, 0xFA } } },
    { 0x1E03, { { cp_852, 'b' }, { cp_852, 0xFA } } },
    { 0x1E08, { { cp_850, 0x80 }, { cp_850, 0xEF } } },
    { 0x1E09, { { cp_850, 0x87 }, { cp_850, 0xEF } } },
    { 0x1E0A, { { cp_852, 'D' }, { cp_852, 0xFA } } },
    { 0x1E0B, { { cp_852, 'd' }, { cp_852, 0xFA } } },
    { 0x1E10, { { cp_850, 'D' }, { cp_850, 0xF7 } } },
    { 0x1E11, { { cp_850, 'd' }, { cp_850, 0xF7 } } },
    { 0x1E14, { { cp_850, 'E' }, { cp_850, 0xEE }, { cp_850, '`' } } },
    { 0x1E15, { { cp_850, 'e' }, { cp_850, 0xEE }, { cp_850, '`' } } },
    { 0x1E16, { { cp_850, 'E' }, { cp_850, 0xEE }, { cp_850, 0xEF } } },
    { 0x1E17, { { cp_850, 'e' }, { cp_850, 0xEE }, { cp_850, 0xEF } } },
    { 0x1E1C, { { cp_850, 'E' }, { cp_850, 0xF7 }, { cp_852, 0xF4 } } },
    { 0x1E1D, { { cp_850, 'e' }, { cp_850, 0xF7 }, { cp_852, 0xF4 } } },
    { 0x1E1E, { { cp_852, 'F' }, { cp_852, 0xFA } } },
    { 0x1E1F, { { cp_852, 'f' }, { cp_852, 0xFA } } },
    { 0x1E20, { { cp_850, 'G' }, { cp_850, 0xEE } } },
    { 0x1E21, { { cp_850, 'g' }, { cp_850, 0xEE } } },
    { 0x1E22, { { cp_852, 'H' }, { cp_852, 0xFA } } },
    { 0x1E23, { { cp_852, 'h' }, { cp_852, 0xFA } } },
    { 0x1E26, { { cp_850, 'H' }, { cp_850, 0xF9 } } },
    { 0x1E27, { { cp_850, 'h' }, { cp_850, 0xF9 } } },
    { 0x1E28, { { cp_850, 'H' }, { cp_850, 0xF7 } } },
    { 0x1E29, { { cp_850, 'h' }, { cp_850, 0xF7 } } },
    { 0x1E2E, { { cp_850, 0xD8 }, { cp_850, 0xEF } } },
    { 0x1E2F, { { cp_850, 0x8B }, { cp_850, 0xEF } } },
    { 0x1E30, { { cp_850, 'K' }, { cp_850, 0xEF } } },
    { 0x1E31, { { cp_850, 'k' }, { cp_850, 0xEF } } },
    { 0x1E3E, { { cp_850, 'M' }, { cp_850, 0xEF } } },
    { 0x1E3F, { { cp_850, 'm' }, { cp_850, 0xEF } } },
    { 0x1E40, { { cp_852, 'M' }, { cp_852, 0xFA } } },
    { 0x1E41, { { cp_852, 'm' }, { cp_852, 0xFA } } },
    { 0x1E44, { { cp_852, 'N' }, { cp_852, 0xFA } } },
    { 0x1E45, { { cp_852, 'n' }, { cp_852, 0xFA } } },
    { 0x1E4C, { { cp_850, 0xE5 }, { cp_850, 0xEF } } },
    { 0x1E4D, { { cp_850, 0xE4 }, { cp_850, 0xEF } } },
    { 0x1E4E, { { cp_850, 0xE5 }, { cp_850, 0xF9 } } },
    { 0x1E4F, { { cp_850, 0xE4 }, { cp_850, 0xF9 } } },
    { 0x1E50, { { cp_850, 'O' }, { cp_850, 0xEE }, { cp_850, '`' } } },
    { 0x1E51, { { cp_850, 'o' }, { cp_850, 0xEE }, { cp_850, '`' } } },
    { 0x1E52, { { cp_850, 'O' }, { cp_850, 0xEE }, { cp_850, 0xEF } } },
    { 0x1E53, { { cp_850, 'o' }, { cp_850, 0xEE }, { cp_850, 0xEF } } },
    { 0x1E54, { { cp_850, 'P' }, { cp_850, 0xEF } } },
    { 0x1E55, { { cp_850, 'p' }, { cp_850, 0xEF } } },
    { 0x1E56, { { cp_852, 'P' }, { cp_852, 0xFA } } },
    { 0x1E57, { { cp_852, 'p' }, { cp_852, 0xFA } } },
    { 0x1E58, { { cp_852, 'R' }, { cp_852, 0xFA } } },
    { 0x1E59, { { cp_852, 'r' }, { cp_852, 0xFA } } },
    { 0x1E60, { { cp_852, 'S' }, { cp_852, 0xFA } } },
    { 0x1E61, { { cp_852, 's' }, { cp_852, 0xFA } } },
    { 0x1E64, { { cp_852, 0x97 }, { cp_852, 0xFA } } },
    { 0x1E65, { { cp_852, 0x98 }, { cp_852, 0xFA } } },
    { 0x1E66, { { cp_852, 0xE6 }, { cp_852, 0xFA } } },
    { 0x1E67, { { cp_852, 0xE7 }, { cp_852, 0xFA } } },
    { 0x1E6A, { { cp_852, 'T' }, { cp_852, 0xFA } } },
    { 0x1E6B, { { cp_852, 't' }, { cp_852, 0xFA } } },
    { 0x1E78, { { cp_1004, 'U' }, { cp_1004, 0x98 }, { cp_850, 0xEF } } },
    { 0x1E79, { { cp_1004, 'u' }, { cp_1004, 0x98 }, { cp_850, 0xEF } } },
    { 0x1E7A, { { cp_850, 'U' }, { cp_850, 0xEE }, { cp_850, 0xF9 } } },
    { 0x1E7B, { { cp_850, 'u' }, { cp_850, 0xEE }, { cp_850, 0xF9 } } },
    { 0x1E7C, { { cp_1004, 'V' }, { cp_1004, 0x98 } } },
    { 0x1E7D, { { cp_1004, 'v' }, { cp_1004, 0x98 } } },
    { 0x1E80, { { cp_850, 'W' }, { cp_850, '`' } } },
    { 0x1E81, { { cp_850, 'w' }, { cp_850, '`' } } },
    { 0x1E82, { { cp_850, 'W' }, { cp_850, 0xEF } } },
    { 0x1E83, { { cp_850, 'w' }, { cp_850, 0xEF } } },
    { 0x1E84, { { cp_850, 'W' }, { cp_850, 0xF9 } } },
    { 0x1E85, { { cp_850, 'w' }, { cp_850, 0xF9 } } },
    { 0x1E86, { { cp_852, 'W' }, { cp_852, 0xFA } } },
    { 0x1E87, { { cp_852, 'w' }, { cp_852, 0xFA } } },
    { 0x1E8A, { { cp_852, 'X' }, { cp_852, 0xFA } } },
    { 0x1E8B, { { cp_852, 'x' }, { cp_852, 0xFA } } },
    { 0x1E8C, { { cp_850, 'X' }, { cp_850, 0xF9 } } },
    { 0x1E8D, { { cp_850, 'x' }, { cp_850, 0xF9 } } },
    { 0x1E8E, { { cp_852, 'Y' }, { cp_852, 0xFA } } },
    { 0x1E8F, { { cp_852, 'y' }, { cp_852, 0xFA } } },
    { 0x1E90, { { cp_850, 'Z' }, { cp_850, '^' } } },
    { 0x1E91, { { cp_850, 'z' }, { cp_850, '^' } } },
    { 0x1E97, { { cp_850, 't' }, { cp_850, 0xF9 } } },
    { 0x1E98, { { cp_1275, 'w' }, { cp_1275, 0xFB } } },
    { 0x1E99, { { cp_1275, 'y' }, { cp_1275, 0xFB } } },
    { 0x1EA4, { { cp_850, 0xB6 }, { cp_850, 0xEF } } },
    { 0x1EA5, { { cp_850, 0x83 }, { cp_850, 0xEF } } },
    { 0x1EA6, { { cp_850, 0xB6 }, { cp_850, '`' } } },
    { 0x1EA7, { { cp_850, 0x83 }, { cp_850, '`' } } },
    { 0x1EAA, { { cp_850, 0xB6 }, { cp_1004, 0x98 } } },
    { 0x1EAB, { { cp_850, 0x83 }, { cp_1004, 0x98 } } },
    { 0x1EAE, { { cp_852, 0xC6 }, { cp_850, 0xEF } } },
    { 0x1EAF, { { cp_852, 0xC7 }, { cp_850, 0xEF } } },
    { 0x1EB0, { { cp_852, 0xC6 }, { cp_852, '`' } } },
    { 0x1EB1, { { cp_852, 0xC7 }, { cp_852, '`' } } },
    { 0x1EB4, { { cp_852, 0xC6 }, { cp_1004, 0x98 } } },
    { 0x1EB5, { { cp_852, 0xC7 }, { cp_1004, 0x98 } } },
    { 0x1EBC, { { cp_1004, 'E' }, { cp_1004, 0x98 } } },
    { 0x1EBD, { { cp_1004, 'e' }, { cp_1004, 0x98 } } },
    { 0x1EBE, { { cp_850, 0xD2 }, { cp_850, 0xEF } } },
    { 0x1EBF, { { cp_850, 0x88 }, { cp_850, 0xEF } } },
    { 0x1EC0, { { cp_850, 0xD2 }, { cp_850, '`' } } },
    { 0x1EC1, { { cp_850, 0x88 }, { cp_850, '`' } } },
    { 0x1EC4, { { cp_850, 0xD2 }, { cp_1004, 0x98 } } },
    { 0x1EC5, { { cp_850, 0x88 }, { cp_1004, 0x98 } } },
    { 0x1ED0, { { cp_850, 0xE2 }, { cp_850, 0xEF } } },
    { 0x1ED1, { { cp_850, 0x93 }, { cp_850, 0xEF } } },
    { 0x1ED2, { { cp_850, 0xE2 }, { cp_850, '`' } } },
    { 0x1ED3, { { cp_850, 0x93 }, { cp_850, '`' } } },
    { 0x1ED6, { { cp_850, 0xE2 }, { cp_1004, 0x98 } } },
    { 0x1ED7, { { cp_850, 0x93 }, { cp_1004, 0x98 } } },
    { 0x1EF2, { { cp_850, 'Y' }, { cp_850, '`' } } },
    { 0x1EF3, { { cp_850, 'y' }, { cp_850, '`' } } },
    { 0x1EF8, { { cp_1004, 'Y' }, { cp_1004, 0x98 } } },
    { 0x1EF9, { { cp_1004, 'y' }, { cp_1004, 0x98 } } },
    { 0x1F70, { { cp_869, 0xD6 }, { cp_869, '`' } } },
    { 0x1F72, { { cp_869, 0xDE }, { cp_869, '`' } } },
    { 0x1F74, { { cp_869, 0xE1 }, { cp_869, '`' } } },
    { 0x1F76, { { cp_869, 0xE3 }, { cp_869, '`' } } },
    { 0x1F78, { { cp_869, 0xE9 }, { cp_869, '`' } } },
    { 0x1F7A, { { cp_869, 0xF2 }, { cp_869, '`' } } },
    { 0x1F7C, { { cp_869, 0xFA }, { cp_869, '`' } } },
    { 0x1FB0, { { cp_869, 0xD6 }, { cp_852, 0xF4 } } },
    { 0x1FB1, { { cp_869, 0xD6 }, { cp_850, 0xEE } } },
    { 0x1FB8, { { cp_869, 0xA4 }, { cp_852, 0xF4 } } },
    { 0x1FB9, { { cp_869, 0xA4 }, { cp_850, 0xEE } } },
    { 0x1FBA, { { cp_869, 0xA4 }, { cp_869, '`' } } },
    { 0x1FC8, { { cp_869, 0xA8 }, { cp_869, '`' } } },
    { 0x1FCA, { { cp_869, 0xAA }, { cp_869, '`' } } },
    { 0x1FD0, { { cp_869, 0xE3 }, { cp_852, 0xF4 } } },
    { 0x1FD1, { { cp_869, 0xE3 }, { cp_850, 0xEE } } },
    { 0x1FD2, { { cp_869, 0xA0 }, { cp_869, '`' } } },
    { 0x1FD8, { { cp_869, 0xAD }, { cp_852, 0xF4 } } },
    { 0x1FD9, { { cp_869, 0xAD }, { cp_850, 0xEE } } },
    { 0x1FDA, { { cp_869, 0xAD }, { cp_869, '`' } } },
    { 0x1FE0, { { cp_869, 0xF2 }, { cp_852, 0xF4 } } },
    { 0x1FE1, { { cp_869, 0xF2 }, { cp_850, 0xEE } } },
    { 0x1FE2, { { cp_869, 0xFB }, { cp_869, '`' } } },
    { 0x1FE8, { { cp_869, 0xD1 }, { cp_852, 0xF4 } } },
    { 0x1FE9, { { cp_869, 0xD1 }, { cp_850, 0xEE } } },
    { 0x1FEA, { { cp_869, 0xD1 }, { cp_869, '`' } } },
    { 0x1FED, { { cp_850, 0xF9 }, { cp_850, '`' } } },
    { 0x1FF8, { { cp_869, 0xBE }, { cp_869, '`' } } },
    { 0x1FFA, { { cp_869, 0xD5 }, { cp_869, '`' } } },
    { 0x2013, { { cp_1004, 0x96 } } },
    { 0x2014, { { cp_1004, 0x97 } } },
    { 0x2015, { { cp_869, 0x8E } } },
    { 0x2017, { { cp_850, 0xF2 } } },
    { 0x2018, { { cp_1004, 0x91 } } },
    { 0x2019, { { cp_1004, 0x92 } } },
    { 0x201A, { { cp_1004, 0x82 } } },
    { 0x201C, { { cp_1004, 0x93 } } },
    { 0x201D, { { cp_1004, 0x94 } } },
    { 0x201E, { { cp_1004, 0x84 } } },
    { 0x2020, { { cp_1004, 0x86 } } },
    { 0x2021, { { cp_1004, 0x87 } } },
    { 0x2022, { { cp_437, 0x07 } } },
    { 0x2026, { { cp_1004, 0x85 } } },
    { 0x2030, { { cp_1004, 0x89 } } },
    { 0x2039, { { cp_1004, 0x8B } } },
    { 0x203A, { { cp_1004, 0x9B } } },
    { 0x203C, { { cp_437, 0x13 } } },
    { 0x2044, { { cp_1275, 0xDA } } },
    { 0x207F, { { cp_437, 0xFC } } },
    { 0x20A7, { { cp_437, 0x9E } } },
    { 0x20AC, { { cp_1004, 0x80 } } },
    { 0x2116, { { cp_1251, 0xB9 } } },
    { 0x2122, { { cp_1004, 0x99 } } },
    { 0x2190, { { cp_437, 0x1B } } },
    { 0x2191, { { cp_437, 0x18 } } },
    { 0x2192, { { cp_437, 0x1A } } },
    { 0x2193, { { cp_437, 0x19 } } },
    { 0x2194, { { cp_437, 0x1D } } },
    { 0x2195, { { cp_437, 0x12 } } },
    { 0x219A, { { cp_437, 0x1B }, { cp_437, '/' } } },
    { 0x219B, { { cp_437, 0x1A }, { cp_437, '/' } } },
    { 0x21A8, { { cp_437, 0x17 } } },
    { 0x21AE, { { cp_437, 0x1D }, { cp_437, '/' } } },
    { 0x2202, { { cp_1275, 0xB6 } } },
    { 0x2206, { { cp_1275, 0xC6 } } },
    { 0x220F, { { cp_1275, 0xB8 } } },
    { 0x2211, { { cp_1275, 0xB7 } } },
    { 0x2219, { { cp_437, 0xF9 } } },
    { 0x221A, { { cp_437, 0xFB } } },
    { 0x221E, { { cp_437, 0xEC } } },
    { 0x221F, { { cp_437, 0x1C } } },
    { 0x2229, { { cp_437, 0xEF } } },
    { 0x222B, { { cp_1275, 0xBA } } },
    { 0x2248, { { cp_437, 0xF7 } } },
    { 0x2249, { { cp_437, 0xF7 }, { cp_437, '/' } } },
    { 0x2260, { { cp_1275, 0xAD } } },
    { 0x2261, { { cp_437, 0xF0 } } },
    { 0x2262, { { cp_437, 0xF0 }, { cp_437, '/' } } },
    { 0x2264, { { cp_437, 0xF3 } } },
    { 0x2265, { { cp_437, 0xF2 } } },
    { 0x226E, { { 0, '<' }, { 0, '/' } } },
    { 0x226F, { { 0, '>' }, { 0, '/' } } },
    { 0x2270, { { cp_437, 0xF3 }, { cp_437, '/' } } },
    { 0x2271, { { cp_437, 0xF2 }, { cp_437, '/' } } },
    { 0x2302, { { cp_437, 0x7F } } },
    { 0x2310, { { cp_437, 0xA9 } } },
    { 0x2320, { { cp_437, 0xF4 } } },
    { 0x2321, { { cp_437, 0xF5 } } },
    { 0x2500, { { cp_850, 0xC4 } } },
    { 0x2502, { { cp_850, 0xB3 } } },
    { 0x250C, { { cp_850, 0xDA } } },
    { 0x2510, { { cp_850, 0xBF } } },
    { 0x2514, { { cp_850, 0xC0 } } },
    { 0x2518, { { cp_850, 0xD9 } } },
    { 0x251C, { { cp_850, 0xC3 } } },
    { 0x2524, { { cp_850, 0xB4 } } },
    { 0x252C, { { cp_850, 0xC2 } } },
    { 0x2534, { { cp_850, 0xC1 } } },
    { 0x253C, { { cp_850, 0xC5 } } },
    { 0x2550, { { cp_850, 0xCD } } },
    { 0x2551, { { cp_850, 0xBA } } },
    { 0x2552, { { cp_437, 0xD5 } } },
    { 0x2553, { { cp_437, 0xD6 } } },
    { 0x2554, { { cp_850, 0xC9 } } },
    { 0x2555, { { cp_437, 0xB8 } } },
    { 0x2556, { { cp_437, 0xB7 } } },
    { 0x2557, { { cp_850, 0xBB } } },
    { 0x2558, { { cp_437, 0xD4 } } },
    { 0x2559, { { cp_437, 0xD3 } } },
    { 0x255A, { { cp_850, 0xC8 } } },
    { 0x255B, { { cp_437, 0xBE } } },
    { 0x255C, { { cp_437, 0xBD } } },
    { 0x255D, { { cp_850, 0xBC } } },
    { 0x255E, { { cp_437, 0xC6 } } },
    { 0x255F, { { cp_437, 0xC7 } } },
    { 0x2560, { { cp_850, 0xCC } } },
    { 0x2561, { { cp_437, 0xB5 } } },
    { 0x2562, { { cp_437, 0xB6 } } },
    { 0x2563, { { cp_850, 0xB9 } } },
    { 0x2564, { { cp_437, 0xD1 } } },
    { 0x2565, { { cp_437, 0xD2 } } },
    { 0x2566, { { cp_850, 0xCB } } },
    { 0x2567, { { cp_437, 0xCF } } },
    { 0x2568, { { cp_437, 0xD0 } } },
    { 0x2569, { { cp_850, 0xCA } } },
    { 0x256A, { { cp_437, 0xD8 } } },
    { 0x256B, { { cp_437, 0xD7 } } },
    { 0x256C, { { cp_850, 0xCE } } },
    { 0x2580, { { cp_850, 0xDF } } },
    { 0x2584, { { cp_850, 0xDC } } },
    { 0x2588, { { cp_850, 0xDB } } },
    { 0x258C, { { cp_437, 0xDD } } },
    { 0x2590, { { cp_437, 0xDE } } },
    { 0x2591, { { cp_850, 0xB0 } } },
    { 0x2592, { { cp_850, 0xB1 } } },
    { 0x2593, { { cp_850, 0xB2 } } },
    { 0x25A0, { { cp_850, 0xFE } } },
    { 0x25AC, { { cp_437, 0x16 } } },
    { 0x25B2, { { cp_437, 0x1E } } },
    { 0x25BA, { { cp_437, 0x10 } } },
    { 0x25BC, { { cp_437, 0x1F } } },
    { 0x25C4, { { cp_437, 0x11 } } },
    { 0x25CA, { { cp_1275, 0xD7 } } },
    { 0x25CB, { { cp_437, 0x09 } } },
    { 0x25D8, { { cp_437, 0x08 } } },
    { 0x25D9, { { cp_437, 0x0A } } },
    { 0x263A, { { cp_437, 0x01 } } },
    { 0x263B, { { cp_437, 0x02 } } },
    { 0x263C, { { cp_437, 0x0F } } },
    { 0x2640, { { cp_437, 0x0C } } },
    { 0x2642, { { cp_437, 0x0B } } },
    { 0x2660, { { cp_437, 0x06 } } },
    { 0x2663, { { cp_437, 0x05 } } },
    { 0x2665, { { cp_437, 0x03 } } },
    { 0x2666, { { cp_437, 0x04 } } },
    { 0x266A, { { cp_437, 0x0D } } },
    { 0x266B, { { cp_437, 0x0E } } },
    { 0xFB01, { { cp_1275, 0xDE } } },
    { 0xFB02, { { cp_1275, 0xDF } } },
};

/* Alternative renderings used when code pages 869, 1257 or 1275 aren't
   available. */
static const struct CharRec alt_chars[] = {
    { 0x0100, { { cp_850, 'A' }, { cp_850, 0xEE } } },
    { 0x0101, { { cp_850, 'a' }, { cp_850, 0xEE } } },
    { 0x0112, { { cp_850, 'E' }, { cp_850, 0xEE } } },
    { 0x0113, { { cp_850, 'e' }, { cp_850, 0xEE } } },
    { 0x0116, { { cp_852, 'E' }, { cp_852, 0xFA } } },
    { 0x0117, { { cp_852, 'e' }, { cp_852, 0xFA } } },
    { 0x0122, { { cp_850, 'G' }, { cp_850, 0xF7 } } },
    { 0x0123, { { cp_850, 'g' }, { cp_850, 0xF7 } } },
    { 0x012A, { { cp_850, 'I' }, { cp_850, 0xEE } } },
    { 0x012B, { { cp_850, 'i' }, { cp_850, 0xEE } } },
    { 0x012E, { { cp_852, 'I' }, { cp_852, 0xF2 } } },
    { 0x012F, { { cp_852, 'i' }, { cp_852, 0xF2 } } },
    { 0x0136, { { cp_850, 'K' }, { cp_850, 0xF7 } } },
    { 0x0137, { { cp_850, 'k' }, { cp_850, 0xF7 } } },
    { 0x013B, { { cp_850, 'L' }, { cp_850, 0xF7 } } },
    { 0x013C, { { cp_850, 'l' }, { cp_850, 0xF7 } } },
    { 0x0145, { { cp_850, 'N' }, { cp_850, 0xF7 } } },
    { 0x0146, { { cp_850, 'n' }, { cp_850, 0xF7 } } },
    { 0x014C, { { cp_850, 'O' }, { cp_850, 0xEE } } },
    { 0x014D, { { cp_850, 'o' }, { cp_850, 0xEE } } },
    { 0x0156, { { cp_850, 'R' }, { cp_850, 0xF7 } } },
    { 0x0157, { { cp_850, 'r' }, { cp_850, 0xF7 } } },
    { 0x016A, { { cp_850, 'U' }, { cp_850, 0xEE } } },
    { 0x016B, { { cp_850, 'u' }, { cp_850, 0xEE } } },
    { 0x0172, { { cp_852, 'U' }, { cp_852, 0xF2 } } },
    { 0x0173, { { cp_852, 'u' }, { cp_852, 0xF2 } } },
    { 0x02DA, { { cp_850, 0xF8 } } },
    { 0x0391, { { cp_437, 'A' } } },
    { 0x0392, { { cp_437, 'B' } } },
    { 0x0393, { { cp_437, 0xE2 } } },
    { 0x0395, { { cp_437, 'E' } } },
    { 0x0396, { { cp_437, 'Z' } } },
    { 0x0397, { { cp_437, 'H' } } },
    { 0x0398, { { cp_437, 0xE9 } } },
    { 0x0399, { { cp_437, 'I' } } },
    { 0x039A, { { cp_437, 'K' } } },
    { 0x039C, { { cp_437, 'M' } } },
    { 0x039D, { { cp_437, 'N' } } },
    { 0x039F, { { cp_437, 'O' } } },
    { 0x03A1, { { cp_437, 'P' } } },
    { 0x03A3, { { cp_437, 0xE4 } } },
    { 0x03A4, { { cp_437, 'T' } } },
    { 0x03A5, { { cp_437, 'Y' } } },
    { 0x03A6, { { cp_437, 0xE8 } } },
    { 0x03A7, { { cp_437, 'X' } } },
    { 0x03A9, { { cp_437, 0xEA } } },
    { 0x03B1, { { cp_437, 0xE0 } } },
    { 0x03B4, { { cp_437, 0xEB } } },
    { 0x03B5, { { cp_437, 0xEE } } },
    { 0x03BC, { { cp_437, 0xE6 } } },
    { 0x03BF, { { cp_437, 'o' } } },
    { 0x03C0, { { cp_437, 0xE3 } } },
    { 0x03C3, { { cp_437, 0xE5 } } },
    { 0x03C4, { { cp_437, 0xE7 } } },
    { 0x03C6, { { cp_437, 0xED } } },
    { 0x2260, { { cp_437, '=' }, { cp_437, '/' } } },
};

/* Compare function for the above tables */
static int
char_compare(const void *p1, const void *p2)
{
    const ULONG *key = p1;
    const struct CharRec *val = p2;

    if (*key < val->uni_ch) {
        return -1;
    } else if (*key > val->uni_ch) {
        return +1;
    } else {
        return 0;
    }
}
#endif

/* Render a single character */
static void
render_char(HPS hps, PPOINTL point, PRECTL rect, ULONG ch, int charset)
{
#ifdef PDC_WIDE
    /* ch is a Unicode character */
    unsigned char bytes[2];

    if (ch <= 0xFF) {
        /* Render lower characters in code page 1004; it's faster */
        /* All known versions of OS/2 have code page 1004 */
        bytes[0] = (unsigned char)ch;
        set_charset(hps, cp_1004, (charset & 3));
        GpiCharStringPosAt(hps, point, rect, 0, 1, bytes, NULL);
    } else if (have_cp[cp_1200]) {
        /* We have Unicode support */
        bytes[0] = (ch & 0x00FF) >> 0;
        bytes[1] = (ch & 0xFF00) >> 8;
        set_charset(hps, cp_1200, (charset & 3));
        GpiCharStringPosAt(hps, point, rect, 0, 2, bytes, NULL);
    } else {
        /* Select a matching character set */
        static const struct CharRec bad_char = { 0x003F, { { cp_850, '?' } } };
        const struct CharRec *rec;
        unsigned i;

        /* Search the main character table */
        rec = bsearch(&ch, characters,
                sizeof(characters)/sizeof(characters[0]), sizeof(characters[0]),
                char_compare);
        if (rec != NULL && !have_cp[rec->glyphs[0].code_page]) {
            /* Search the alternate character table */
            const struct CharRec *rec2;
            rec2 = bsearch(&ch, alt_chars,
                    sizeof(alt_chars)/sizeof(alt_chars[0]), sizeof(alt_chars[0]),
                    char_compare);
            if (rec2 != NULL) {
                rec = rec2;
            }
        }
        if (rec == NULL || !have_cp[rec->glyphs[0].code_page]) {
            /* Display '?' if we don't have the character */
            rec = &bad_char;
        }
        /* Overstrike the listed glyphs */
        for (i = 0; i < sizeof(rec->glyphs)/sizeof(rec->glyphs[0]); ++i) {
            bytes[0] = rec->glyphs[i].code_point;
            if (bytes[0] == 0) {
                break;
            }
            set_charset(hps, rec->glyphs[i].code_page, + (charset & 3));
            GpiCharStringPosAt(hps, point, rect, 0, 1, bytes, NULL);
        }
    }
#else
    /* For non-PDC_WIDE build, ch is either an 8 bit character or it has
       A_ALTCHARSET set */
    unsigned char byte;

    if (ch == ACS_NEQUAL) {
        /* Not-equal sign from code page 1275 (which might not be available) */
        if (have_cp[cp_1275]) {
            /* We have code page 1275 */
            byte = 0xAD;
            set_charset(hps, cp_1275, (charset & 3));
            GpiCharStringPosAt(hps, point, rect, 0, 1, (PCH)&byte, NULL);
        } else {
            /* Overstrike = and / */
            set_charset(hps, cp_native, (charset & 3));
            byte = '=';
            GpiCharStringPosAt(hps, point, rect, 0, 1, (PCH)&byte, NULL);
            byte = '/';
            GpiCharStringPosAt(hps, point, rect, 0, 1, (PCH)&byte, NULL);
        }
    } else if (_is_altcharset(ch)) {
        /* Other alternate characters come from code page 437 */
        byte = acs_map[ch & 0xFF];
        set_charset(hps, cp_437, (charset & 3));
        GpiCharStringPosAt(hps, point, rect, 0, 1, (PCH)&byte, NULL);
    } else {
        /* Normal character from the configured default code page */
        byte = ch & 0xFF;
        set_charset(hps, cp_native, (charset & 3));
        GpiCharStringPosAt(hps, point, rect, 0, 1, (PCH)&byte, NULL);
    }
#endif
}

HPS PDC_override_hps;

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    if( !srcp)    /* just freeing up fonts */
        PDC_transform_line_given_hps( 0, 0, 0, 0, NULL);
    else
    {
        const HPS hps = (PDC_override_hps ? PDC_override_hps : WinGetPS( PDC_hWnd)) ;
        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
        PDC_setup_font(hps);

        assert( len);
        assert( (srcp[len - 1] & A_CHARTEXT) != MAX_UNICODE);
        PDC_transform_line_given_hps( hps, lineno, x, len, srcp);
        if (!PDC_override_hps)
            WinReleasePS( hps);
    }
}

void PDC_doupdate(void)
{
}

char PDC_font_name[FACESIZE];
int PDC_font_size = 0;

void PDC_set_font_box(HWND hwnd)
{
    HPS hps;
    POINTL pt[TXTBOX_COUNT];

    /* Determine font dimensions */
    hps = WinGetPS(hwnd);
    PDC_setup_font(hps);

    memset(pt, 0, sizeof(pt));
    GpiQueryTextBox(hps, 1, (PCH) "\xC5", TXTBOX_COUNT, pt);
    PDC_cxChar = pt[TXTBOX_TOPRIGHT].x - pt[TXTBOX_BOTTOMLEFT].x;
    PDC_cyChar = pt[TXTBOX_TOPRIGHT].y - pt[TXTBOX_BOTTOMLEFT].y;
    WinReleasePS(hps);
    if (PDC_n_rows != 0 && PDC_n_cols != 0)
        PDC_resize_screen(PDC_n_rows, PDC_n_cols);
}

void PDC_setup_font(HPS hps)
{
    SIZEF box;

    /* Determine the name of the font */
    if (PDC_font_size == 0)
    {
        const char *env_size = getenv("PDC_FONT_SIZE");
        if (env_size != NULL)
            PDC_font_size = strtol(env_size, NULL, 10);
    }
    if (PDC_font_name[0] == '\0')
    {
        const char *env_font = getenv("PDC_FONT");
        if (env_font != NULL)
        {
            const char *colon = strchr(env_font, ':');
            size_t len;
            if (colon != NULL)
            {
                if (PDC_font_size < 2)
                    PDC_font_size = strtol(colon + 1, NULL, 10);
                len = colon - env_font;
            }
            else
            {
                len = strlen(env_font);
            }
            snprintf(PDC_font_name, sizeof(PDC_font_name), "%.*s",
                    (int)len, env_font);
        }
        else
        {
            /* Assumed present on all OS/2s */
            strcpy(PDC_font_name, "Courier");
        }
    }
    if (PDC_font_size < 2)
        PDC_font_size = 12;

    /* Query available code pages */
    if (!cp_inited) {
        /* There seems to be no way to query the number of code pages. 200 is
           more than enough for ArcaOS 5.1. */
        ULONG cp_list[200];
        ULONG num_cps;
        ULONG m, n;

        cp_inited = TRUE;
        num_cps = WinQueryCpList(WinQueryAnchorBlock(PDC_hWnd),
                sizeof(cp_list)/sizeof(cp_list[0]), cp_list);
        have_cp[0] = TRUE;
        for (m = 0; m < sizeof(code_pages)/sizeof(code_pages[0]); ++m) {
            have_cp[m+1] = FALSE;
            for (n = 0; n < num_cps; ++n) {
                if (cp_list[n] == code_pages[m]) {
                    have_cp[m+1] = TRUE;
                    break;
                }
            }
        }
    }

    set_charset(hps, cp_native, 0);
    GpiQueryCharBox(hps, &box);
    box.cx = (FIXED)(((long long)box.cx * PDC_font_size) / 12);
    box.cy = (FIXED)(((long long)box.cy * PDC_font_size) / 12);
    GpiSetCharBox(hps, &box);
}

/****************************************************************************/
/*   Canonical composition and decomposition to aid in Unicode rendering    */
/****************************************************************************/

#if defined(PDC_WIDE) && defined(USING_COMBINING_CHARACTER_SCHEME)

/* Data types for Unicode tables: */
/* Combining class */
struct CClassTable {
    uint32_t cp;
    unsigned cclass;
};

/* Canonical decomposition */
struct DecompTable {
    uint32_t cp;
    uint32_t decomp1;
    uint32_t decomp2;
};

/* Canonical composition */
struct CompTable {
    uint32_t cp1;
    uint32_t cp2;
    uint32_t comp;
};

/* The actual tables, generated from Unicode data */
#include "unitable.h"

/* Comparison function for CClassTable */
static int cclass_compare(const void *p1, const void *p2)
{
    const uint32_t *cp = p1;
    const struct CClassTable *rec = p2;

    if (*cp < rec->cp)
        return -1;
    if (*cp > rec->cp)
        return +1;
    return 0;
}

/* Comparison function for DecompTable */
static int decomp_compare(const void *p1, const void *p2)
{
    const uint32_t *cp = p1;
    const struct DecompTable *rec = p2;

    if (*cp < rec->cp)
        return -1;
    if (*cp > rec->cp)
        return +1;
    return 0;
}

/* Comparison function for CompTable */
static int comp_compare(const void *p1, const void *p2)
{
    const uint32_t *key = p1;
    const struct CompTable *rec = p2;

    if (key[0] < rec->cp1)
        return -1;
    if (key[0] > rec->cp1)
        return +1;
    if (key[1] < rec->cp2)
        return -1;
    if (key[1] > rec->cp2)
        return +1;
    return 0;
}

/* Search for canonical decomposition of character ch */
/* Return true if a decomposition exists; seq returns the decomposition */
static bool PDC_find_decomp(uint32_t ch, uint32_t seq[2])
{
    const struct DecompTable *rec;

    rec = bsearch(&ch, decomp_table,
            sizeof(decomp_table)/sizeof(decomp_table[0]), sizeof(decomp_table[0]),
            decomp_compare);
    if (rec)
    {
        seq[0] = rec->decomp1;
        seq[1] = rec->decomp2;
        return TRUE;
    }
    return FALSE;
}

/* Find the full canonical decomposition of the character ch */
/* Return the length of the decomposition; the decomposed code points are
   in seq */
/* As of Unicode 15.1, no canonical decomposition is longer than four
   characters */
static size_t PDC_decompose(uint32_t ch, uint32_t seq[])
{
    uint32_t decomp[2];
    size_t len, i, j;

    if (0xAC00 <= ch && ch <= 0xD7A3)
    {
        /* Hangul syllables */
        unsigned s_index = (unsigned)(ch - 0xAC00);
        unsigned t_index = s_index % 28;
        unsigned v_index = (s_index / 28) % 21;
        unsigned l_index = s_index / (28*21);
        seq[0] = 0x1100 + l_index; /* leading consonant */
        seq[1] = 0x1161 + v_index; /* vowel */
        if (t_index != 0)
        {
            seq[1] = 0x11A7 + t_index; /* trailing consonant */
            return 3;
        }
        return 2;
    }

    /* Find canonical decompositions until a non-decomposable character
       is found */
    /* Store the suffixes in reverse order */
    seq[0] = ch;
    len = 1;
    while (PDC_find_decomp(seq[0], decomp))
    {
        seq[0] = decomp[0];
        seq[len++] = decomp[1];
    }

    /* Reverse the order of the suffixes */
    for (i = 1, j = len - 1; i < j; ++i, --j)
    {
        uint32_t swap;
        swap = seq[i];
        seq[i] = seq[j];
        seq[j] = swap;
    }

    return len;
}

/* Return the canonical composition of ch1 and ch2, or 0 if there is no
   such composition */
static uint32_t PDC_compose(uint32_t ch1, uint32_t ch2)
{
    const struct CompTable *rec;
    uint32_t key[2];

    key[0] = ch1;
    key[1] = ch2;
    if (0x1100 <= ch1 && ch1 <= 0x1112 && 0x1161 <= ch2 && ch2 <= 0x1175)
    {
        /* Hangul syllable without trailing consonant */
        return 0xAC00
             + (ch1 - 0x1100) * (28*21)
             + (ch2 - 0x1161) * 28;
    }

    if (0xAC00 <= ch1 && ch1 <= 0xD7A3 && 0x11A8 <= ch2 && ch2 <= 0x11C2)
    {
        /* Hangul syllable with trailing consonant */
        if ((ch1 - 0xAC00) % 28 == 0)
            return ch1 + ch2 - 0x11A7;
        else
            return 0;
    }

    rec = bsearch(key, comp_table,
            sizeof(comp_table)/sizeof(comp_table[0]), sizeof(comp_table[0]),
            comp_compare);

    return rec ? rec->comp : 0;
}

/* Return the combining class of the character ch */
/* Non-combining characters have combining class 0 */
static unsigned PDC_cclass(uint32_t ch)
{
    const struct CClassTable *rec;

    rec = bsearch(&ch, cclass_table,
            sizeof(cclass_table)/sizeof(cclass_table[0]), sizeof(cclass_table[0]),
            cclass_compare);

    return rec ? rec->cclass : 0;
}

/* Retrieve the combining sequence indexed by ch, and convert it to a
   normalization form */
/* If compose is true, the normalization form is NFC (characters are composed);
   otherwise, it is NFD */
/* Return a malloc'd buffer containing the sequence, terminated with a 0 */
static uint32_t *PDC_normalize(uint32_t ch, bool compose)
{
    uint32_t *comp;
    cchar_t added;
    uint32_t ch2;
    uint32_t seq[10];
    size_t count;
    size_t i, j;

    /* If not combining sequence, return identity */
    if (ch <= MAX_UNICODE)
    {
        comp = malloc(sizeof(comp[0]) * 2);
        comp[0] = ch;
        comp[1] = 0;
        return comp;
    }

    /* Count characters needed for sequence */
    count = 0;
    ch2 = ch;
    while (ch2 > MAX_UNICODE)
    {
        ch2 = PDC_expand_combined_characters(ch2, &added);
        count += PDC_decompose(added, seq);
    }
    count += PDC_decompose(ch2, seq);

    /* Allocate array */
    comp = malloc(sizeof(comp[0]) * (count + 1));

    /* Build complete expanded sequence */
    i = count;
    ch2 = ch;
    while (ch2 > MAX_UNICODE)
    {
        size_t l;

        ch2 = PDC_expand_combined_characters(ch2, &added);
        l = PDC_decompose(added, seq);
        i -= l;
        memcpy(comp + i, seq, l * sizeof(comp[0]));
    }
    PDC_decompose(ch2, comp);

    /* Sort according to combining class, preserving order for equal classes */
    /* We can't use qsort. We need a stable sort here, and qsort is not
       guaranteed to be stable.
       Combining sequences are expected to be short, so insertion sort should
       work well, though it is O(n^2). Each entry in comp has an unused byte
       that can hold the combining class. */

    /* Tag each combining character with its combining class */
    for (i = 1; i < count; ++i)
    {
        unsigned cclass = PDC_cclass(comp[i]);
        comp[i] |= cclass << 24;
    }

    /* Sort by combining class */
    for (i = 2; i < count; ++i)
    {
        /* Portion before i is sorted */
        uint32_t swap = comp[i];
        unsigned cclass = (unsigned)(swap >> 24);
        for (j = i; j > 1; --j)
        {
            if ((comp[j-1] >> 24) <= cclass)
                break;
            comp[j] = comp[j-1];
        }
        comp[j] = swap;
    }

    if (compose)
    {
        /* Compose according to Unicode rules: a combining character is
           blocked from composing with the base character if another combining
           character with the same combining class occurs before it */
        i = 1;
        while (i < count)
        {
            uint32_t new_ch = PDC_compose(comp[0], comp[i] & 0x00FFFFFF);
            if (new_ch != 0)
            {
                /* Replace the pair with the composed character */
                comp[0] = new_ch;
                comp[i++] = 0;
            }
            else
            {
                /* Skip this combining character and all others of the same
                   combining class */
                unsigned cclass = (unsigned)(comp[i++] >> 24);
                while (i < count && (comp[i] >> 24) <= cclass)
                    ++i;
            }
        }
        /* Remove deleted characters from the array */
        j = 1;
        for (i = 1; i < count; ++i)
        {
            if (comp[i] != 0)
                comp[j++] = comp[i];
        }
        count = j;
    }

    /* Remove combining classes from characters */
    for (i = 1; i < count; ++i)
        comp[i] &= 0x00FFFFFF;

    comp[count] = 0;
    return comp;
}

#endif /* PDC_WIDE && USING_COMBINING_CHARACTER_SCHEME */
