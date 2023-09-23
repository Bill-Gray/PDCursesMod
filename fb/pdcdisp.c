#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "curspriv.h"
#include "pdcfb.h"
#include "psf.h"

#ifdef PDC_WIDE
    #define USE_UNICODE_ACS_CHARS 1
#else
    #define USE_UNICODE_ACS_CHARS 0
#endif

#include "../common/acs_defs.h"
#include "../common/pdccolor.h"

extern int PDC_blink_state;

/* Blinking of text and the cursor in this port has to be handled a
little strangely.  "When possible",  we check to see if blink_interval
milliseconds (currently set to 0.5 seconds) has elapsed since the
blinking text was drawn.  If it has,  we flip the PDC_blink_state
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

   if( !t || t > prev_time + blink_interval)
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

static const uint8_t *_get_raw_glyph_bytes( struct font_info *font, int unicode_point)
{
    int glyph_idx = find_psf_or_vgafont_glyph( font, unicode_point);
    const int font_char_size_in_bytes = (font->width + 7) >> 3;

    if( glyph_idx < 0 || glyph_idx >= (int)font->n_glyphs)
        glyph_idx = find_psf_or_vgafont_glyph( font, '?');
    if( glyph_idx < 0)
        glyph_idx = 0;
    return( font->glyphs + glyph_idx * font_char_size_in_bytes * font->height);
}

extern struct font_info PDC_font_info;
extern struct video_info PDC_fb;

void PDC_draw_rectangle( const int xpix, const int ypix,
                  const int xsize, const int ysize, const uint32_t color)
{
    const int line_len = PDC_fb.line_length * 8 / PDC_fb.bits_per_pixel;
    int x, y;
    const long video_offset = xpix + ypix * line_len;

    if( PDC_fb.bits_per_pixel == 32)
    {
        uint32_t *tptr = (uint32_t *)PDC_fb.framebuf + video_offset;

        for( y = ysize; y; y--, tptr += line_len - xsize)
            for( x = xsize; x; x--)
                *tptr++ = color;
    }
    if( PDC_fb.bits_per_pixel == 8)
    {
        uint8_t *tptr = (uint8_t *)PDC_fb.framebuf + video_offset;

        for( y = ysize; y; y--, tptr += line_len - xsize)
            for( x = xsize; x; x--)
                *tptr++ = color;
    }
}

/* see 'addch.c' for an explanation of how combining chars are handled. */

#ifdef USING_COMBINING_CHARACTER_SCHEME
   int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);  /* addch.c */

static void _add_combining_character_glyph( uint8_t *glyph, const int code_point)
{
    const uint8_t *add_in = _get_raw_glyph_bytes( &PDC_font_info, code_point);
    int i;

    for( i = 0; i < (int)PDC_font_info.charsize; i++)
        glyph[i] ^= add_in[i];
}
#endif

#define LINE_ATTRIBS (WA_UNDERLINE | WA_TOP | WA_LEFT | WA_RIGHT | WA_STRIKEOUT)

/* Usually,  we can just return a pointer to the glyph data from
the PSF file (using _get_raw_glyph_bytes()).  If the glyph has to
be modified for line drawings or a cursor,  or it's a combined
character,  or it's bold or italic,  we build the glyph in the
scratch space and return 'scratch' instead.  */

static const uint8_t *_get_glyph( const chtype ch, const int cursor_type,
                                 uint8_t *scratch)
{
    const uint8_t *rval;
    int c = (int)( ch & A_CHARTEXT);
#ifdef USING_COMBINING_CHARACTER_SCHEME
    cchar_t root, newchar;

    if( c > (int)MAX_UNICODE)      /* chars & fullwidth supported */
    {
        root = c;
        while( (c = PDC_expand_combined_characters( c,
                           &newchar)) > (int)MAX_UNICODE)
            ;
    }
    else
        root = 0;
#endif

    if( _is_altcharset( ch))
        c = (int)acs_map[c & 0x7f];
    else if( c < (int)' ' || (c >= 0x80 && c <= 0x9f))
        c = ' ';
    rval = _get_raw_glyph_bytes( &PDC_font_info, c);
#ifdef USING_COMBINING_CHARACTER_SCHEME
    if( cursor_type || (ch & (LINE_ATTRIBS | A_BOLD | A_ITALIC)) || root)
#else
    if( cursor_type || (ch & (LINE_ATTRIBS | A_BOLD | A_ITALIC)))
#endif
    {
        const int font_char_size_in_bytes = (PDC_font_info.width + 7) >> 3;
        int i, j;

        memcpy( scratch, rval, PDC_font_info.charsize);
        rval = (const uint8_t *)scratch;
        if( ch & A_BOLD)
            for( i = PDC_font_info.charsize - 1; i >= 0; i--)
            {
                scratch[i] |= (scratch[i] >> 1);
                if( (i % font_char_size_in_bytes) && (scratch[i - 1] & 1))
                   scratch[i] |= 0x80;
            }
        if( ch & A_ITALIC)  /* shift half top of glyph by one pixel right */
            for( i = (PDC_font_info.height / 2) * font_char_size_in_bytes; i >= 0; i--)
            {
                scratch[i] >>= 1;
                if( (i % font_char_size_in_bytes) && (scratch[i - 1] & 1))
                   scratch[i] |= 0x80;
            }
#ifdef USING_COMBINING_CHARACTER_SCHEME
        if( root)
        {
            while( (root = PDC_expand_combined_characters( root,
                           &newchar)) > (cchar_t)MAX_UNICODE)
                _add_combining_character_glyph( scratch, (int)newchar);
            _add_combining_character_glyph( scratch, (int)newchar);
        }
#endif
        if( cursor_type)
        {
            i = (cursor_type == 1 ? PDC_font_info.height - 2 : 0);
            scratch += i * font_char_size_in_bytes;
            for( ; i < (int)PDC_font_info.height; i++)
               for( j = 0; j < font_char_size_in_bytes; j++)
                   *scratch++ ^= 0xff;
            scratch -= PDC_font_info.charsize;
        }
        if( ch & (WA_UNDERLINE | WA_TOP | WA_STRIKEOUT))
        {
            if( ch & WA_TOP)
                memset( scratch, 0xff, font_char_size_in_bytes);
            if( ch & WA_STRIKEOUT)
                memset( scratch + (PDC_font_info.height / 2) * font_char_size_in_bytes,
                            0xff, font_char_size_in_bytes);
            if( ch & WA_UNDERLINE)
                memset( scratch + (PDC_font_info.height - 1) * font_char_size_in_bytes,
                            0xff, font_char_size_in_bytes);
        }
        if( ch & WA_LEFT)
            for( i = 0; i < (int)PDC_font_info.height; i++)
               scratch[i * font_char_size_in_bytes] |= 0x80;
        if( ch & WA_RIGHT)
        {
            scratch += font_char_size_in_bytes - 1;
            for( i = 0; i < (int)PDC_font_info.height; i++)
               scratch[i * font_char_size_in_bytes] |= (0x80 >> ((PDC_font_info.width - 1) & 7));
        }
    }
    return( rval);
}

/* The framebuffer appears to store red,  green,  and blue in the opposite
order from what the other platforms expect : */

#define SWAP_RED_AND_BLUE( rgb) (((rgb) & 0xff00) | ((rgb) >> 16) | (((rgb) & 0xff) << 16))

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    const int font_char_size_in_bytes = (PDC_font_info.width + 7) >> 3;
    int cursor_to_draw = 0;
    const int line_len = PDC_fb.line_length * 8 / PDC_fb.bits_per_pixel;
    uint8_t scratch[300];

    assert( srcp);
    assert( x >= 0);
    assert( len <= SP->cols - x);
    assert( lineno >= 0);
    assert( lineno < SP->lines);
    assert( len > 0);
    if( lineno > (int)( PDC_fb.yres / PDC_font_info.height))
        return;
    if( x + len > (int)( PDC_fb.xres / PDC_font_info.width))
    {
        len = (int)( PDC_fb.xres / PDC_font_info.width) - x;
        if( len <= 0)
            return;
    }
    if( lineno == SP->cursrow && x <= SP->curscol && x + len > SP->curscol)
    {
        cursor_to_draw = (PDC_blink_state ? SP->visibility & 0xff : (SP->visibility >> 8));
        if( cursor_to_draw)   /* if there's a cursor appearing in this run of text... */
        {
            if( x < SP->curscol)  /* ...draw the part _before_ the cursor (if any)... */
                PDC_transform_line( lineno, x, SP->curscol - x, srcp);
            len -= SP->curscol - x;
            srcp += SP->curscol - x;
            x = SP->curscol;
            if( len > 1)          /* ...then the part _after the cursor (if any)... */
                PDC_transform_line( lineno, x + 1, len - 1, srcp + 1);
            len = 1;    /* ... then fall through and just draw the cell with the cursor */
        }
    }
    while( len)
    {
        int run_len = 0;
        PACKED_RGB fg, bg;
        const long video_offset = x * PDC_font_info.width
                     + lineno * PDC_font_info.height * line_len;

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
        if( PDC_fb.bits_per_pixel == 32)
        {
            int i;
            uint32_t *tptr = (uint32_t *)PDC_fb.framebuf + video_offset;

            for( i = 0; i < run_len; i++)
            {
                const uint8_t *fontptr = _get_glyph( *srcp, cursor_to_draw, scratch);
                uint32_t *fb_ptr = tptr;
                int i, j;

                for( i = 0; i < (int)PDC_font_info.height; i++)
                {
                    for( j = 0; j < (int)PDC_font_info.width; j++)
                        *fb_ptr++ = ((fontptr[j >> 3] << (j & 7)) & 0x80) ? fg : bg;
                    fb_ptr += line_len - PDC_font_info.width;
                    fontptr += font_char_size_in_bytes;
                }
                srcp++;
                len--;
                tptr += PDC_font_info.width;
                x++;
            }
        }
        if( PDC_fb.bits_per_pixel == 8)
        {
            const int line_len = PDC_fb.line_length; /* / sizeof( uint8_t); */
            int i, integer_fg_idx, integer_bg_idx;
            uint8_t fg_idx, bg_idx;
            uint8_t *tptr = (uint8_t *)PDC_fb.framebuf + video_offset;
            bool reverse_colors = ((*srcp & A_REVERSE) ? TRUE : FALSE);

            extended_pair_content( (*srcp & A_COLOR) >> PDC_COLOR_SHIFT,
                                   &integer_fg_idx, &integer_bg_idx);
            if( *srcp & A_BLINK)
            {
#ifdef TO_FIGURE_OUT
                if( !(SP->termattrs & A_BLINK))   /* convert 'blinking' to 'bold' */
                    intensify_backgnd = TRUE;
#endif
                if( PDC_blink_state)
                    reverse_colors ^= 1;
            }
            if( reverse_colors)
            {
                const int swapval = integer_fg_idx;
                integer_fg_idx = integer_bg_idx;
                integer_bg_idx = swapval;
            }
            fg_idx = (uint8_t)integer_fg_idx;
            bg_idx = (uint8_t)integer_bg_idx;
            for( i = 0; i < run_len; i++)
            {
                const uint8_t *fontptr = _get_glyph( *srcp, cursor_to_draw, scratch);
                uint8_t *fb_ptr = tptr;
                int i, j;

                for( i = 0; i < (int)PDC_font_info.height; i++)
                {
                    for( j = 0; j < (int)PDC_font_info.width; j++)
                        *fb_ptr++ = ((fontptr[j >> 3] << (j & 7)) & 0x80) ? fg_idx : bg_idx;
                    fb_ptr += line_len - PDC_font_info.width;
                    fontptr += font_char_size_in_bytes;
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
