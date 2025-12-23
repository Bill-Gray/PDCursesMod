#define USE_UNICODE_ACS_CHARS 1

#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "curspriv.h"
#include "pdcx11.h"
#include "../common/acs_defs.h"
#include "../common/pdccolor.h"
#include "../common/blink.c"

#ifdef USING_COMBINING_CHARACTER_SCHEME
   int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);  /* addch.c */

static size_t _unpack_combined_character( wchar_t *obuff, const size_t buffsize,
                                       const cchar_t ch)
{
    cchar_t root, newchar;
    size_t rval = 1;

    root = ch;
    while( rval < buffsize && (root = PDC_expand_combined_characters( root,
                       &newchar)) > MAX_UNICODE)
       obuff[rval++] = (wchar_t)newchar;
    obuff[0] = (wchar_t)root;
    if( rval < buffsize)
       obuff[rval++] = (wchar_t)newchar;
    assert( rval < buffsize);
    assert( rval > 1);
    return( rval);
}
#endif

#define OBUFF_SIZE 100

static PACKED_RGB _reversed( const PACKED_RGB ival)
{
   return( (ival & 0xff00) | (ival >> 16) | ((ival << 16) & 0xff0000));
}

static int _put_into_xchar2b_string( XChar2b *string, const int32_t ch)
{
   string->byte1 = ch >> 8;
   string->byte2 = ch & 0xff;
   return( 1);
}

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    XChar2b string[OBUFF_SIZE];
    static PACKED_RGB prev_bg = (PACKED_RGB)-2;
    static PACKED_RGB prev_fg = (PACKED_RGB)-2;

    if( !srcp)
    {
        prev_bg = prev_fg = (PACKED_RGB)-2;
        return;
    }
    assert( x >= 0);
    assert( len <= SP->cols - x);
    assert( lineno >= 0);
    assert( lineno < SP->lines);
    assert( len > 0);
    assert( len < MAX_PACKET_LEN);
    while( len)
    {
       int i = 0, j, chars_out = 0;
       PACKED_RGB bg, fg;
       int xpix = x * PDC_font_width;
       const int ypix = (lineno + 1) * PDC_font_height;
       chtype attr_out;

       while( i < len && !((srcp[i] ^ srcp[0]) & ~A_CHARTEXT))
          {
          int32_t ch;

          if( _is_altcharset( srcp[i]))
             ch = (int32_t)acs_map[srcp[i] & 0x7f];
          else
             ch = (int32_t)( srcp[i] & A_CHARTEXT);
#ifdef PDC_WIDE
          if( ch < (int)MAX_UNICODE)
#endif
             chars_out += _put_into_xchar2b_string( string + chars_out, ch);
#ifdef USING_COMBINING_CHARACTER_SCHEME
          else if( ch > (int)MAX_UNICODE)       /* combining character sequence */
             {
             wchar_t expanded[10];
             size_t i, n_wchars = _unpack_combined_character( expanded, 10, ch);

             for( i = 0; i < n_wchars; i++)
                chars_out += _put_into_xchar2b_string( string + chars_out, expanded[i]);
             }
#endif
          i++;
          }
       PDC_get_rgb_values( *srcp & ~A_REVERSE, &fg, &bg);
       if( bg == (PACKED_RGB)-1)   /* default background */
          bg = (PACKED_RGB)0;
       if( fg == (PACKED_RGB)-1)   /* default foreground */
          fg = (PACKED_RGB)0xffffff;
       if( *srcp & A_REVERSE)
          {
          const PACKED_RGB temp_rgb = fg;

          fg = bg;
          bg = temp_rgb;
          }
       if( fg != prev_fg)
          {
          XSetForeground(dis, curr_gc, _reversed( fg));
          prev_fg = fg;
          }
       if( bg != prev_bg)
          {
          XSetBackground(dis, curr_gc, _reversed( bg));
          prev_bg = bg;
          }
       XDrawImageString16( dis, win, curr_gc, xpix,
                                 ypix - PDC_font_descent, string, chars_out);
       attr_out = *srcp;
       if( SP->drawing_cursor)
       {
          int cursor_height = PDC_font_height / 4;

          if( SP->drawing_cursor == 2)    /* full-block */
              cursor_height = PDC_font_height;
          else if( SP->drawing_cursor == 5)    /* bottom half block */
              cursor_height = PDC_font_height / 2;
          else if( SP->drawing_cursor == 3)    /* outlined */
          {
              cursor_height = 0;
              attr_out ^= A_LEFT | A_RIGHT | A_TOP | A_UNDERLINE;
          }
          else if( SP->drawing_cursor == 4)    /* caret */
          {
              cursor_height = 0;
              attr_out ^= A_LEFT;
          }
          if( cursor_height)
          {
              XSetFunction( dis, curr_gc, GXinvert);
              XFillRectangle( dis, win, curr_gc, SP->curscol * PDC_font_width,
                          ypix - cursor_height, PDC_font_width, cursor_height);
              XSetFunction( dis, curr_gc, GXcopy);
          }
       }
       if( attr_out & (A_LEFT | A_RIGHT | A_UNDERLINE | A_TOP | A_STRIKEOUT))
       {
          const int xend = xpix + len * PDC_font_width - 1;

          if (SP->line_color != -1)
          {
             prev_fg = PDC_get_palette_entry( SP->line_color);
             XSetForeground(dis, curr_gc, _reversed( prev_fg));
          }
          if( attr_out & A_UNDERLINE)
             XDrawLine( dis, win, curr_gc, xpix, ypix - 1, xend, ypix - 1);
          if( attr_out & A_STRIKEOUT)
             XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height / 2,
                           xend, ypix - PDC_font_height / 2);
          if( attr_out & A_TOP)
             XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height,
                           xend, ypix - PDC_font_height);
          if( attr_out & (A_LEFT | A_RIGHT))
             for( j = i; j; j--)
             {
                 if( attr_out & A_LEFT)
                    XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height,
                                                  xpix, ypix);
                 xpix += PDC_font_width - 1;
                 if( attr_out & A_RIGHT)
                    XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height,
                                                  xpix, ypix);
                 xpix++;
             }
       }
       srcp += i;
       len -= i;
       x += i;
       assert( i);
    }
}

void PDC_doupdate(void)
{
   XSync( dis, False);
}
