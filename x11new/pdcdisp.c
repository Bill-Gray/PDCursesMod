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

/* Blinking of text and the cursor in this port has to be handled a
little strangely.  "When possible",  we check to see if blink_interval
milliseconds (currently set to 0.5 seconds) has elapsed since the
blinking text was drawn.  If it has,  we flip the SP->blink_state
bit and redraw all blinking text and the cursor.

Currently,  "when possible" is in PDC_napms( ) and in check_key( )
(see vt/pdckbd.c for the latter).  This does mean that if you set up
some blinking text,  and then do some processor-intensive stuff and
aren't checking for keyboard input,  the text will stop blinking. */

void PDC_check_for_blinking( void)
{
   static long prev_time = 0;
   const long t = PDC_millisecs( );
   const long blink_interval = 500L;

   if( !t || t - prev_time > blink_interval)
   {
      int x1, y, x2;

      prev_time = t;
      SP->blink_state ^= 1;
      for( y = 0; y < SP->lines; y++)
      {
         chtype *c = curscr->_y[y];

         for( x1 = 0; x1 < SP->cols; x1++)
            if( c[x1] & A_BLINK)
            {
               x2 = x1 + 1;
               while( x2 < SP->cols && (c[x2] & A_BLINK) && x2 - x1 < MAX_PACKET_LEN - 1)
                  x2++;
               PDC_transform_line( y, x1, x2 - x1, c + x1);
               x1 = x2;
            }
         if( SP->visibility && y == SP->cursrow)
            PDC_transform_line( y, SP->curscol, 1, c + SP->curscol);
      }
   }
}

void PDC_gotoyx(int y, int x)
{

   INTENTIONALLY_UNUSED_PARAMETER( y);
   INTENTIONALLY_UNUSED_PARAMETER( x);
   if( SP->visibility && curscr && curscr->_y && curscr->_y[SP->cursrow])
      {
      const int temp_visibility = SP->visibility;

      SP->visibility = 0;
      PDC_transform_line( SP->cursrow, SP->curscol, 1,
                    curscr->_y[SP->cursrow] + SP->curscol);
      SP->visibility = temp_visibility;
      return;
      }
}

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
    int cursor_to_draw = 0;

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
    if( lineno == SP->cursrow && SP->curscol >= x && SP->curscol < x + len)
        cursor_to_draw = (SP->blink_state ? SP->visibility & 0xff : (SP->visibility >> 8));
    while( len)
    {
       int i = 0, j, chars_out = 0;
       PACKED_RGB bg, fg;
       int xpix = x * PDC_font_width;
       const int ypix = (lineno + 1) * PDC_font_height;

       while( i < len && !((srcp[i] ^ srcp[0]) & ~A_CHARTEXT))
          {
          int32_t ch;

          if( _is_altcharset( srcp[i]))
             ch = (int32_t)acs_map[srcp[i] & 0x7f];
          else
             ch = (int32_t)( srcp[i] & A_CHARTEXT);
          if( ch < (int)MAX_UNICODE)
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
       if( x <= SP->curscol && x + i > SP->curscol && cursor_to_draw)
          {
          const int cursor_height = PDC_font_height / (cursor_to_draw == 2 ? 1 : 4);

          XSetFunction( dis, curr_gc, GXinvert);
          XFillRectangle( dis, win, curr_gc, SP->curscol * PDC_font_width,
                      ypix - cursor_height, PDC_font_width, cursor_height);
          XSetFunction( dis, curr_gc, GXcopy);
          }
       if( *srcp & (A_LEFT | A_RIGHT | A_UNDERLINE | A_TOP | A_STRIKEOUT))
       {
          const int xend = xpix + len * PDC_font_width - 1;

          if (SP->line_color != -1)
          {
             prev_fg = PDC_get_palette_entry( SP->line_color);
             XSetForeground(dis, curr_gc, _reversed( prev_fg));
          }
          if( *srcp & A_UNDERLINE)
             XDrawLine( dis, win, curr_gc, xpix, ypix - 1, xend, ypix - 1);
          if( *srcp & A_STRIKEOUT)
             XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height / 2,
                           xend, ypix - PDC_font_height / 2);
          if( *srcp & A_TOP)
             XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height,
                           xend, ypix - PDC_font_height);
          if( *srcp & (A_LEFT | A_RIGHT))
             for( j = i; j; j--)
             {
                 if( *srcp & A_LEFT)
                    XDrawLine( dis, win, curr_gc, xpix, ypix - PDC_font_height,
                                                  xpix, ypix);
                 xpix += PDC_font_width - 1;
                 if( *srcp & A_RIGHT)
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
