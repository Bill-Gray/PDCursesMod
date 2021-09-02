#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <unistd.h>

#define USE_UNICODE_ACS_CHARS 0

#include "curspriv.h"
#include "pdcfb.h"
#include "../dosvga/font.h"
#include "../common/acs_defs.h"
#include "../common/pdccolor.h"

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
    const int font_char_size_in_bytes = (PDC_font_width + 7) >> 3;
    int cursor_to_draw = SP->visibility & 0xff;

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
        int run_len = 1;
        PACKED_RGB fg, bg;

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
        while( run_len < len && !((*srcp ^ srcp[run_len]) & A_ATTRIBUTES))
            run_len++;
        if( PDC_vinfo.bits_per_pixel == 32)
        {
            const int line_len = PDC_finfo.line_length / sizeof( uint32_t);
            uint32_t *tptr = (uint32_t *)PDC_framebuf + x * PDC_font_width
                   + lineno * PDC_font_height * line_len;

            while( run_len--)
            {
                int ch = (int)( *srcp & A_CHARTEXT);

                if( (*srcp & A_ALTCHARSET) && ch < 0x80)
                    ch = (int)acs_map[ch & 0x7f];
                if( ch < (int)' ' || (ch >= 0x80 && ch <= 0x9f))
                    ch = ' ';
                if( ch > 0 && ch <= 0xff)
                {
                    uint32_t *fb_ptr = tptr;
                    int i, j, shift_point;
                    const unsigned char *fontptr = font_bytes
                         + ch * font_char_size_in_bytes * PDC_font_height;
                    unsigned mask = ((*srcp & A_BOLD) ? 0x180 : 0x80);

                    if( *srcp & A_ITALIC)
                    {
                        shift_point = PDC_font_height / 2;
                        mask <<= 1;
                    }
                    else
                        shift_point = 9999;
                    for( i = 0; i < PDC_font_height; i++)
                    {
                        for( j = 0; j < PDC_font_width; j++)
                            *fb_ptr++ = ((*fontptr << j) & mask) ? fg : bg;
                        fb_ptr += line_len - PDC_font_width;
                        fontptr += font_char_size_in_bytes;
                        if( i == shift_point)
                           mask >>= 1;
                    }
                    if( *srcp & A_UNDERLINE)
                    {
                        fb_ptr -= line_len;
                        for( j = PDC_font_width; j; j--)
                            *fb_ptr++ = fg;
                    }
                    if( *srcp & A_OVERLINE)
                    {
                        for( j = PDC_font_width; j; j--)
                            *tptr++ = fg;
                        tptr -= PDC_font_width;
                    }
                    if( *srcp & A_LEFTLINE)
                    {
                        fb_ptr = tptr;
                        for( i = PDC_font_height; i; i--, fb_ptr += line_len)
                            *fb_ptr = fg;
                    }
                    if( *srcp & A_RIGHTLINE)
                    {
                        fb_ptr = tptr + PDC_font_width - 1;
                        for( i = PDC_font_height; i; i--, fb_ptr += line_len)
                            *fb_ptr = fg;
                    }
                    if( *srcp & A_STRIKEOUT)
                    {
                        fb_ptr = tptr + (PDC_font_height / 2) * line_len;
                        for( j = PDC_font_width; j; j--)
                            *fb_ptr++ = fg;
                    }
                    if( x == SP->curscol && cursor_to_draw)
                    {
                        int n_lines = PDC_font_height;

                        fb_ptr = tptr;
                        if( cursor_to_draw == 1)      /* bottom two lines */
                        {
                            fb_ptr += line_len * (PDC_font_height - 2);
                            n_lines = 2;
                        }
                        while( n_lines--)
                        {
                            for( j = 0; j < PDC_font_width; j++, fb_ptr++)
                               *fb_ptr = fg + bg - *fb_ptr;
                            fb_ptr += line_len - PDC_font_width;
                        }
                    }
                }
                srcp++;
                len--;
                tptr += PDC_font_width;
                x++;
            }
        }
   }
}

void PDC_doupdate(void)
{
}
