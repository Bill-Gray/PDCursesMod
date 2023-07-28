/* PDCurses */

#include "pdcx11.h"

#include <string.h>

#ifdef PDC_WIDE
   #define USE_UNICODE_ACS_CHARS 1
#else
   #define USE_ISO8859_CHARSET
#endif

#include "../common/acs_defs.h"
#include "../common/pdccolor.h"

bool pdc_blinked_off;
bool pdc_visible_cursor = FALSE;
bool pdc_vertical_cursor = FALSE;

/* Convert character positions x and y to pixel positions, stored in
   xpos and ypos */

static void _make_xy(int x, int y, int *xpos, int *ypos)
{
    *xpos = x * pdc_fwidth;
    *ypos = pdc_app_data.normalFont->ascent + (y * pdc_fheight);
}

static void _set_cursor_color(chtype *ch, int *fore, int *back)
{
    int attr;

    attr = PAIR_NUMBER(*ch);

    if (attr)
    {
        int f, b;

        extended_pair_content(attr, &f, &b);
        *fore = 7 - (f % 8);
        *back = 7 - (b % 8);
    }
    else
    {
        if (*ch & A_REVERSE)
        {
            *back = COLOR_BLACK;
            *fore = COLOR_WHITE;
        }
        else
        {
            *back = COLOR_WHITE;
            *fore = COLOR_BLACK;
        }
    }
}

static void _display_cursor(int old_row, int old_x, int new_row, int new_x)
{
    int xpos, ypos, i;
    chtype *ch;
    int fore = 0, back = 0;

    PDC_LOG(("_display_cursor() - draw char at row: %d col %d\n",
             old_row, old_x));

    /* if the cursor position is outside the boundary of the screen,
       ignore the request */

    if (old_row >= SP->lines || old_x >= COLS ||
        new_row >= SP->lines || new_x >= COLS)
        return;

    /* display the character at the current cursor position */

    PDC_LOG(("_display_cursor() - draw char at row: %d col %d\n",
             old_row, old_x));

    PDC_transform_line(old_row, old_x, 1, curscr->_y[old_row] + old_x);

    /* display the cursor at the new cursor position */

    if (!SP->visibility)
        return;     /* cursor not displayed, no more to do */

    _make_xy(new_x, new_row, &xpos, &ypos);

    ch = curscr->_y[new_row] + new_x;
    _set_cursor_color(ch, &fore, &back);

    if (pdc_vertical_cursor)
    {
        XSetForeground(XCURSESDISPLAY, pdc_cursor_gc, PDC_get_pixel( back));

        for (i = 1; i <= SP->visibility; i++)
            XDrawLine(XCURSESDISPLAY, XCURSESWIN, pdc_cursor_gc,
                      xpos + i, ypos - pdc_app_data.normalFont->ascent,
                      xpos + i, ypos - pdc_app_data.normalFont->ascent +
                      pdc_fheight - 1);
    }
    else
    {
        /* For block cursors, paint the block with invert. */

        int yp, yh;

        if (SP->visibility == 2)
        {
            yp = ypos - pdc_fheight + pdc_fdescent;
            yh = pdc_fheight;
        }
        else
        {
            yp = ypos - pdc_fheight / 4 + pdc_fdescent;
            yh = pdc_fheight / 4;
        }

        XSetFunction(XCURSESDISPLAY, pdc_cursor_gc, GXinvert);
        XFillRectangle(XCURSESDISPLAY, XCURSESWIN, pdc_cursor_gc,
            xpos, yp, pdc_fwidth, yh);
    }

    PDC_LOG(("_display_cursor() - draw cursor at row %d col %d\n",
             new_row, new_x));
}

void PDC_redraw_cursor(void)
{
    _display_cursor(SP->cursrow, SP->curscol, SP->cursrow, SP->curscol);
}

void PDC_blink_text(XtPointer unused, XtIntervalId *id)
{
    int row;
    int j, k;
    chtype *ch;

    INTENTIONALLY_UNUSED_PARAMETER( unused);
    INTENTIONALLY_UNUSED_PARAMETER( id);
    PDC_LOG(("PDC_blink_text() - called:\n"));

    PDC_blink_state = pdc_blinked_off = !pdc_blinked_off;

    /* Redraw changed lines on the screen to match the blink state */

    for (row = 0; row < SP->lines; row++)
    {
        ch = curscr->_y[row];

        for (j = 0; j < COLS; j++)
            if (ch[j] & A_BLINK)
            {
                k = j;
                while (ch[k] & A_BLINK && k < COLS)
                    k++;

                PDC_transform_line(row, j, k - j, ch + j);

                j = k;
            }
    }

    PDC_redraw_cursor();

    if ((SP->termattrs & A_BLINK) || !pdc_blinked_off)
        XtAppAddTimeOut(pdc_app_context, pdc_app_data.textBlinkRate,
                        PDC_blink_text, NULL);
}

static void _toggle_cursor(void)
{
    PDC_LOG(("_toggle_cursor - called. Vis now: "));
    PDC_LOG((pdc_visible_cursor ? "1\n" : "0\n"));

    /* If the window is not active, ignore this command. The
       cursor will stay solid. */

    if (pdc_window_entered)
    {
        if (pdc_visible_cursor)
        {
            /* Cursor currently ON, turn it off */

            int save_visibility = SP->visibility;
            SP->visibility = 0;
            PDC_redraw_cursor();
            SP->visibility = save_visibility;
            pdc_visible_cursor = FALSE;
        }
        else
        {
            /* Cursor currently OFF, turn it on */

            PDC_redraw_cursor();
            pdc_visible_cursor = TRUE;
        }
    }
}

int PDC_display_cursor(int oldrow, int oldcol, int newrow, int newcol,
                       int visibility)
{
    PDC_LOG(("PDC_display_cursor() - called: NEW row %d col %d, vis %d\n",
             newrow, newcol, visibility));

    if (visibility == -1)
        _toggle_cursor();
    else
    {
        pdc_visible_cursor = TRUE;
        _display_cursor(oldrow, oldcol, newrow, newcol);
    }

    return OK;
}

void PDC_blink_cursor(XtPointer unused, XtIntervalId *id)
{
    PDC_LOG(("PDC_blink_cursor() - called:\n"));

    INTENTIONALLY_UNUSED_PARAMETER( unused);
    INTENTIONALLY_UNUSED_PARAMETER( id);
    _toggle_cursor();
    XtAppAddTimeOut(pdc_app_context, pdc_app_data.cursorBlinkRate,
                    PDC_blink_cursor, NULL);
}

/* position hardware cursor at (y, x) */

void PDC_gotoyx(int row, int col)
{
    PDC_LOG(("PDC_gotoyx() - called: row %d col %d\n", row, col));

    PDC_display_cursor(SP->cursrow, SP->curscol, row, col, SP->visibility);
}

#define reverse_bytes( rgb) ((rgb >> 16) | (rgb & 0xff00) | ((rgb & 0xff) << 16))

/* update the given physical line to look like the corresponding line in
   curscr */

/* Output a block of characters with common attributes */

static int _new_packet(const chtype attr, const int len, const int col, const int row,
#ifdef PDC_WIDE
                       const XChar2b *text)
#else
                       const char *text)
#endif
{
    XRectangle bounds;
    GC gc;
    int xpos, ypos;
    PACKED_RGB fore_rgb, back_rgb;
    attr_t sysattrs;

    PDC_get_rgb_values( attr, &fore_rgb, &back_rgb);
    fore_rgb = reverse_bytes( fore_rgb);
    back_rgb = reverse_bytes( back_rgb);

    /* Specify the color table offsets */

    sysattrs = SP->termattrs;

    /* Determine which GC to use - normal, italic or bold */

    if ((attr & A_ITALIC) && (sysattrs & A_ITALIC))
        gc = pdc_italic_gc;
    else if ((attr & A_BOLD) && (sysattrs & A_BOLD))
        gc = pdc_bold_gc;
    else
        gc = pdc_normal_gc;

    _make_xy(col, row, &xpos, &ypos);

    bounds.x = xpos;
    bounds.y = ypos - pdc_fascent;
    bounds.width = pdc_fwidth * len;
    bounds.height = pdc_fheight;

    XSetClipRectangles(XCURSESDISPLAY, gc, 0, 0, &bounds, 1, Unsorted);

    if (pdc_blinked_off && (sysattrs & A_BLINK) && (attr & A_BLINK))
    {
        XSetForeground(XCURSESDISPLAY, gc, (Pixel)fore_rgb);
        XFillRectangle(XCURSESDISPLAY, XCURSESWIN, gc, xpos, bounds.y,
                       bounds.width, pdc_fheight);
    }
    else
    {
        /* Draw it */

        XSetForeground(XCURSESDISPLAY, gc, (Pixel)fore_rgb);
        XSetBackground(XCURSESDISPLAY, gc, (Pixel)back_rgb);

#ifdef PDC_WIDE
        XDrawImageString16(
#else
        XDrawImageString(
#endif
            XCURSESDISPLAY, XCURSESWIN, gc, xpos, ypos, text, len);

        /* Underline, etc. */

        if (attr & (WA_LEFT | WA_RIGHT | WA_UNDERLINE | WA_TOP | WA_STRIKEOUT))
        {
            int k;
            const int xend = xpos + pdc_fwidth * len;

            if (SP->line_color != -1)
                XSetForeground(XCURSESDISPLAY, gc, PDC_get_pixel( SP->line_color));

            if (attr & WA_UNDERLINE)
                XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
                          xpos, ypos + 1, xend, ypos + 1);

            if (attr & WA_TOP)
                XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
                          xpos, ypos - pdc_fascent, xend,  ypos - pdc_fascent);

            if (attr & WA_STRIKEOUT)
                XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
                          xpos, ypos - pdc_fascent / 2, xend,
                                ypos - pdc_fascent / 2);

            if (attr & WA_LEFT)
                for (k = 0; k < len; k++)
                {
                    int x = xpos + pdc_fwidth * k;
                    XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
                              x, ypos - pdc_fascent, x, ypos + pdc_fdescent);
                }

            if (attr & WA_RIGHT)
                for (k = 0; k < len; k++)
                {
                    int x = xpos + pdc_fwidth * (k + 1) - 1;
                    XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
                              x, ypos - pdc_fascent, x, ypos + pdc_fdescent);
                }
        }
    }

    PDC_LOG(("_new_packet() - row: %d col: %d "
             "num_cols: %d fore: %x back: %x text:<%s>\n",
             row, col, len, fore_rgb, back_rgb, text));

    return OK;
}

/* The core display routine -- update one line of text */

#define MAX_PACKET_SIZE 128

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
#ifdef PDC_WIDE
    XChar2b text[MAX_PACKET_SIZE];
#else
    char text[MAX_PACKET_SIZE];
#endif
    chtype old_attr, attr;
    int i, j;

    PDC_LOG(("PDC_transform_line() - called: lineno: %d x: %d "
             "len: %d\n", lineno, x, len));

    if (!len)
        return;

    old_attr = *srcp & A_ATTRIBUTES;

    for (i = 0, j = 0; j < len; j++)
    {
        chtype curr = srcp[j];

        attr = curr & A_ATTRIBUTES;

        if( _is_altcharset( curr))
        {
            attr ^= A_ALTCHARSET;
            curr = acs_map[curr & 0x7f];
        }

#ifndef PDC_WIDE
        /* Special handling for ACS_BLOCK */

        if (!(curr & A_CHARTEXT))
        {
            curr |= ' ';
            attr ^= A_REVERSE;
        }
#endif
        if (attr != old_attr || i == MAX_PACKET_SIZE - 1)
        {
            if (_new_packet(old_attr, i, x, lineno, text) == ERR)
                return;

            old_attr = attr;
            x += i;
            i = 0;
        }

#ifdef PDC_WIDE
        text[i].byte1 = (curr & 0xff00) >> 8;
        text[i++].byte2 = curr & 0x00ff;
#else
        text[i++] = curr & 0xff;
#endif
    }

    _new_packet(old_attr, i, x, lineno, text);
}

void PDC_doupdate(void)
{
    XSync(XtDisplay(pdc_toplevel), False);
}
