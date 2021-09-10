#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef PDC_WIDE
    #define USE_UNICODE_ACS_CHARS 1
#else
    #define USE_UNICODE_ACS_CHARS 0
#endif

#include "curspriv.h"
#include "pdcfb.h"
#include "psf.h"
#include "../common/acs_defs.h"
#include "../common/pdccolor.h"

extern int PDC_blink_state;

static int64_t _nanoseconds_since_1970( void)
{
   struct timeval now;
   const int rv = gettimeofday( &now, NULL);
   int64_t rval;
   const int64_t one_billion = (int64_t)1000000000;

   if( !rv)
      rval = (int64_t)now.tv_sec * one_billion
           + (int64_t)now.tv_usec * (int64_t)1000;
   else
      rval = 0;
   return( rval);
}

/* Blinking of text and the cursor in this port has to be handled a
little strangely.  "When possible",  we check to see if blink_interval
nanoseconds (currently set to 0.5 seconds) has elapsed since the
blinking text was drawn.  If it has,  we flip the PDC_blink_state
bit and redraw all blinking text and the cursor.

Currently,  "when possible" is in PDC_napms( ) and in check_key( )
(see vt/pdckbd.c for the latter).  This does mean that if you set up
some blinking text,  and then do some processor-intensive stuff and
aren't checking for keyboard input,  the text will stop blinking. */

void PDC_check_for_blinking( void)
{
   static int64_t prev_time = 0;
   const int64_t t = _nanoseconds_since_1970( );
   const int64_t blink_interval = (int64_t)500000000;

   if( t > prev_time + blink_interval)
   {
      int x1, y, x2;

      prev_time = t;
      PDC_blink_state ^= 1;
      for( y = 0; y < SP->lines; y++)
      {
         chtype *c = curscr->_y[y];

         for( x1 = 0; x1 < SP->cols; x1++)
            if( c[x1] & A_BLINK)
            {
               x2 = x1 + 1;
               while( x2 < SP->cols && c[x2] & A_BLINK)
                  x2++;
               PDC_transform_line( y, x1, x2 - x1, c + x1);
               x1 = x2;
            }
         if( SP->visibility && y == SP->cursrow)
            PDC_transform_line( y, SP->curscol, 1, c + SP->curscol);
      }
   }
}

                   /* Rarely,  writes to stdout fail if a signal handler is
                      called.  In which case we just try to write out the
                      remainder of the buffer until success happens.     */

#define TBUFF_SIZE 512

static void put_to_stdout( const char *buff, size_t bytes_out)
{
    static char *tbuff = NULL;
    static size_t bytes_cached;
    const int stdout_fd = 1;

    if( !buff && !tbuff)
        return;

    if( !buff && bytes_out == 1)        /* release memory at shutdown */
    {
        free( tbuff);
        tbuff = NULL;
        bytes_cached = 0;
        return;
    }

    if( buff && !tbuff)
        tbuff = (char *)malloc( TBUFF_SIZE);
    while( bytes_out || (!buff && bytes_cached))
    {
        if( buff)
        {
            size_t n_copy = bytes_out;

            if( n_copy > TBUFF_SIZE - bytes_cached)
                n_copy = TBUFF_SIZE - bytes_cached;
            memcpy( tbuff + bytes_cached, buff, n_copy);
            buff += n_copy;
            bytes_out -= n_copy;
            bytes_cached += n_copy;
        }
        if( bytes_cached == TBUFF_SIZE || !buff)
            while( bytes_cached)
            {
#ifdef _WIN32
                const size_t bytes_written = _write( stdout_fd, tbuff,
                                             (unsigned int)bytes_cached);
#else
                const size_t bytes_written = write( stdout_fd, tbuff, bytes_cached);
#endif

                bytes_cached -= bytes_written;
                if( bytes_cached)
                    memmove( tbuff, tbuff + bytes_written, bytes_cached);
            }
    }
}

void PDC_puts_to_stdout( const char *buff)
{
   put_to_stdout( buff, (buff ? strlen( buff) : 1));
}

void PDC_gotoyx( int row, int col)
{
    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

    if( !SP || !curscr)
         return;

                /* clear the old cursor,  if it's on-screen: */
    if( SP->cursrow >= 0 && SP->curscol >= 0
         && SP->cursrow < SP->lines && SP->curscol < SP->cols
         && (SP->cursrow != row || SP->curscol != col))
    {
        const int temp_visibility = SP->visibility;

        SP->visibility = 0;
        PDC_transform_line( SP->cursrow, SP->curscol, 1,
                           curscr->_y[SP->cursrow] + SP->curscol);
        SP->visibility = temp_visibility;
    }

               /* ...then draw the new  */
    if( row >= 0 && col >= 0
         && row < SP->lines && col < SP->cols)
    {
        SP->cursrow = row;
        SP->curscol = col;
        PDC_transform_line( row, col, 1, curscr->_y[row] + col);
    }
}

static const uint8_t *_get_glyph_bytes( struct font_info *font, int unicode_point)
{
    int glyph_idx = find_psf_or_vgafont_glyph( font, unicode_point);
    const int font_char_size_in_bytes = (font->width + 7) >> 3;

    if( glyph_idx < 0)
        glyph_idx = find_psf_or_vgafont_glyph( font, '?');
    if( glyph_idx < 0)
        glyph_idx = 0;
    return( font->glyphs + glyph_idx * font_char_size_in_bytes * font->height);
}

#define MAX_RUN 80

const chtype MAX_UNICODE = 0x110000;

/* see 'addch.c' for an explanation of how combining chars are handled. */

#ifdef USING_COMBINING_CHARACTER_SCHEME
   int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);  /* addch.c */
#endif

int PDC_wc_to_utf8( char *dest, const int32_t code);

/* The framebuffer appears to store red,  green,  and blue in the opposite
order from what the other platforms expect : */

#define SWAP_RED_AND_BLUE( rgb) (((rgb) & 0xff00) | ((rgb) >> 16) | (((rgb) & 0xff) << 16))

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    extern struct fb_fix_screeninfo PDC_finfo;
    extern struct fb_var_screeninfo PDC_vinfo;
    extern uint8_t *PDC_framebuf;
    extern struct font_info PDC_font_info;
    const int font_char_size_in_bytes = (PDC_font_info.width + 7) >> 3;
    int cursor_to_draw = (PDC_blink_state ? SP->visibility & 0xff : (SP->visibility >> 8));

    assert( srcp);
    assert( x >= 0);
    assert( len <= SP->cols - x);
    assert( lineno >= 0);
    assert( lineno < SP->lines);
    assert( len > 0);
    if( lineno != SP->cursrow || x > SP->curscol || x + len < SP->curscol)
        cursor_to_draw = 0;      /* cursor won't be drawn */
    while( len)
    {
        int run_len = 0, ch[MAX_RUN], shift_point;
        PACKED_RGB fg, bg;
        unsigned mask0 = ((*srcp & A_BOLD) ? 0x180 : 0x80);

        if( *srcp & A_ITALIC)
        {
            shift_point = PDC_font_info.height / 2;
            mask0 <<= 1;
        }
        else
            shift_point = 9999;

        PDC_get_rgb_values( *srcp & ~A_REVERSE, &fg, &bg);
        if( fg == (PACKED_RGB)-1)   /* default foreground */
            fg = 0xffffff;
        fg = SWAP_RED_AND_BLUE( fg);
        if( bg == (PACKED_RGB)-1)   /* default background */
            bg = 0;
        bg = SWAP_RED_AND_BLUE( bg);
        if( *srcp & A_REVERSE)
        {
            PACKED_RGB temp_rgb = fg;

            fg = bg;
            bg = temp_rgb;
        }
        while( run_len < len && run_len < MAX_RUN
                     && !((*srcp ^ srcp[run_len]) & A_ATTRIBUTES))
        {
            int c = (int)( srcp[run_len] & A_CHARTEXT);

            if( (srcp[run_len] & A_ALTCHARSET) && c < 0x80)
                c = (int)acs_map[c & 0x7f];
            else if( c < (int)' ' || (c >= 0x80 && c <= 0x9f))
                c = ' ';
            ch[run_len++] = c;
        }
        if( PDC_vinfo.bits_per_pixel == 32)
        {
            const int line_len = PDC_finfo.line_length / sizeof( uint32_t);
            int i;
            uint32_t *tptr = (uint32_t *)PDC_framebuf + x * PDC_font_info.width
                   + lineno * PDC_font_info.height * line_len;

            for( i = 0; i < run_len; i++)
            {

//              if( ch[i] > 0 && ch[i] <= 0xff)
                {
                    const uint8_t *fontptr = _get_glyph_bytes( &PDC_font_info, ch[i]);
                    uint32_t *fb_ptr = tptr;
                    int i, j;
                    unsigned mask = mask0;

                    for( i = 0; i < (int)PDC_font_info.height; i++)
                    {
                        for( j = 0; j < (int)PDC_font_info.width; j++)
                            *fb_ptr++ = ((fontptr[j >> 3] << (j & 7)) & mask) ? fg : bg;
                        fb_ptr += line_len - PDC_font_info.width;
                        fontptr += font_char_size_in_bytes;
                        if( i == shift_point)
                           mask >>= 1;
                    }
                    if( *srcp & A_UNDERLINE)
                    {
                        fb_ptr -= line_len;
                        for( j = PDC_font_info.width; j; j--)
                            *fb_ptr++ = fg;
                    }
                    if( *srcp & A_OVERLINE)
                    {
                        for( j = PDC_font_info.width; j; j--)
                            *tptr++ = fg;
                        tptr -= PDC_font_info.width;
                    }
                    if( *srcp & A_LEFTLINE)
                    {
                        fb_ptr = tptr;
                        for( i = PDC_font_info.height; i; i--, fb_ptr += line_len)
                            *fb_ptr = fg;
                    }
                    if( *srcp & A_RIGHTLINE)
                    {
                        fb_ptr = tptr + PDC_font_info.width - 1;
                        for( i = PDC_font_info.height; i; i--, fb_ptr += line_len)
                            *fb_ptr = fg;
                    }
                    if( *srcp & A_STRIKEOUT)
                    {
                        fb_ptr = tptr + (PDC_font_info.height / 2) * line_len;
                        for( j = PDC_font_info.width; j; j--)
                            *fb_ptr++ = fg;
                    }
                    if( x == SP->curscol && cursor_to_draw)
                    {
                        int n_lines = PDC_font_info.height;

                        fb_ptr = tptr;
                        if( cursor_to_draw == 1)      /* bottom two lines */
                        {
                            fb_ptr += line_len * (PDC_font_info.height - 2);
                            n_lines = 2;
                        }
                        while( n_lines--)
                        {
                            for( j = 0; j < (int)PDC_font_info.width; j++, fb_ptr++)
                               *fb_ptr = fg + bg - *fb_ptr;
                            fb_ptr += line_len - PDC_font_info.width;
                        }
                    }
                }
                srcp++;
                len--;
                tptr += PDC_font_info.width;
                x++;
            }
        }
   }
}

void PDC_doupdate(void)
{
}
