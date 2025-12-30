#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "curspriv.h"
#include "pdcfb.h"
#include "../common/psf.h"

#ifdef PDC_WIDE
    #define USE_UNICODE_ACS_CHARS 1
#else
    #define USE_UNICODE_ACS_CHARS 0
#endif

#include "../common/acs_defs.h"
#include "../common/pdccolor.h"
#include "../common/blink.c"

extern struct font_info PDC_font_info;
extern struct video_info PDC_fb;

static bool _is_fullwidth_glyph( const chtype c)
{
   const int unicode_point = (int)( c & A_CHARTEXT) | 0x200000;
   const int glyph_idx = find_psf_or_vgafont_glyph( &PDC_font_info, unicode_point);

   return( glyph_idx >= 0 && glyph_idx < (int)PDC_font_info.n_glyphs);
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

static void _add_combining_character_glyph( uint8_t *glyph, const int code_point)
{
    const uint8_t *add_in = _get_raw_glyph_bytes( &PDC_font_info, code_point);
    int i;

    for( i = 0; i < (int)PDC_font_info.charsize; i++)
        glyph[i] ^= add_in[i];
}

extern int PDC_orientation;

#define SIDE_LINE_ATTRIBS (A_UNDERLINE | A_TOP | A_LEFT | A_RIGHT)
#define LINE_ATTRIBS (SIDE_LINE_ATTRIBS | A_STRIKEOUT)

/* Usually,  we can just return a pointer to the glyph data from
the PSF file (using _get_raw_glyph_bytes()).  If the glyph has to
be modified for line drawings or a cursor,  or it's a combined
character,  or it's bold or italic,  we build the glyph in the
scratch space and return 'scratch' instead.  */

static int fullwidth_offset;

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
    rval = _get_raw_glyph_bytes( &PDC_font_info, c | fullwidth_offset);
#ifdef USING_COMBINING_CHARACTER_SCHEME
    if( cursor_type || (ch & (LINE_ATTRIBS | A_BOLD | A_ITALIC)) || root)
#else
    if( cursor_type || (ch & (LINE_ATTRIBS | A_BOLD | A_ITALIC)))
#endif
    {
        const int font_char_size_in_bytes = (PDC_font_info.width + 7) >> 3;
        int i;

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
        if( cursor_type == 1 || cursor_type >= 5)
        {
            const int cursors[8] = { 0, '_', FULL_BLOCK, 0,   /*outlined block */
                           LEFT_HALF_BLOCK, LOWER_HALF_BLOCK, CENTERED_SQUARE, '+' };

            _add_combining_character_glyph( scratch, cursors[cursor_type]);
        }
        if( ch & WA_STRIKEOUT)
        {
            if( PDC_orientation & 1)   /* rotated left or right */
            {
               const int half_width = (int)PDC_font_info.width / 2;

               for( i = 0; i < (int)PDC_font_info.height; i++)
                  scratch[i * font_char_size_in_bytes + (half_width >> 3)]
                             |= (0x80 >> (half_width & 7));
            }
            else        /* unrotated or upside down */
                memset( scratch + (PDC_font_info.height / 2) * font_char_size_in_bytes,
                        0xff, font_char_size_in_bytes);
        }
        if( ch & WA_TOP)
            memset( scratch, 0xff, font_char_size_in_bytes);
        if( ch & WA_UNDERLINE)
            memset( scratch + (PDC_font_info.height - 1) * font_char_size_in_bytes,
                        0xff, font_char_size_in_bytes);
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

static const uint8_t *_get_rotated_glyph( chtype ch, const int cursor_type,
                                 uint8_t *scratch)
{
   if( PDC_orientation && (ch & SIDE_LINE_ATTRIBS))
      {
      chtype new_ch = ch & ~SIDE_LINE_ATTRIBS;
      size_t i;
      const chtype masks[4] = { A_TOP, A_RIGHT, A_UNDERLINE, A_LEFT};

      for( i = 0; i < 4; i++)
         if( ch & masks[i])
            new_ch |= masks[(i + PDC_orientation) & 3];
      ch = new_ch;
      }
   return( _get_glyph( ch, cursor_type, scratch));
}

#ifdef HAVE_MOUSE

int PDC_mouse_x = 317, PDC_mouse_y = 131;
int32_t mouse_pixel = 0xff;

         /* see 'cursor.c' for an explanation of this array */
const unsigned char cursor_data[117] = { 19, 1, 1,
       0x03,  0x00,  0x00,  0x07,  0x00,  0x00,  0x07,  0x00,  0x00,
       0x07,  0x00,  0x00,  0x0f,  0x00,  0x00,  0x1f,  0x00,  0x00,
       0x1f,  0x00,  0x00,  0x3f,  0x00,  0x00,  0x7f,  0x00,  0x00,
       0x7f,  0x00,  0x00,  0xff,  0x00,  0x00,  0xff,  0x01,  0x00,
       0xff,  0x01,  0x00,  0xff,  0x03,  0x00,  0xff,  0x03,  0x00,
       0xf3,  0x00,  0x00,  0xe0,  0x01,  0x00,  0xe0,  0x01,  0x00,
       0xc0,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
       0x02,  0x00,  0x00,  0x02,  0x00,  0x00,  0x06,  0x00,  0x00,
       0x0e,  0x00,  0x00,  0x0e,  0x00,  0x00,  0x1e,  0x00,  0x00,
       0x3e,  0x00,  0x00,  0x3e,  0x00,  0x00,  0x7e,  0x00,  0x00,
       0xfe,  0x00,  0x00,  0xde,  0x00,  0x00,  0xb6,  0x01,  0x00,
       0x32,  0x00,  0x00,  0x60,  0x00,  0x00,  0xc0,  0x00,  0x00,
       0xc0,  0x00,  0x00,  0x00,  0x00,  0x00};

static void _draw_pixel( int xpix, int ypix, const int black)
{
    const int line_len = PDC_fb.line_length * 8 / PDC_fb.bits_per_pixel;
    int tval;
    long video_offset;

    switch( PDC_orientation)
    {
        case 1:
            tval = SP->lines * PDC_font_info.width - ypix - 1;
            ypix = xpix;
            xpix = tval;
            break;
        case 2:
            xpix = COLS * PDC_font_info.width - xpix - 1;
            ypix = SP->lines * PDC_font_info.height - ypix - 1;
            break;
        case 3:
            tval = COLS * PDC_font_info.height - xpix - 1;
            xpix = ypix;
            ypix = tval;
            break;
        default:   /* a.k.a. case 0... do nothing */
            break;
    }
    video_offset = xpix + ypix * line_len;
    if( PDC_fb.bits_per_pixel == 32)
    {
        uint32_t *tptr = (uint32_t *)PDC_fb.framebuf + video_offset;

        *tptr = (black ? 0 : 0xffffff);
    }
    if( PDC_fb.bits_per_pixel == 8)
    {
        uint8_t *tptr = (uint8_t *)PDC_fb.framebuf + video_offset;

        *tptr = (black ? 0x0 : 0x7);
    }
}

static bool _mouse_cursor_shown = TRUE;

static int _orig_font_width( void)
{
   return (PDC_orientation & 1) ? PDC_font_info.height : PDC_font_info.width;
}

static int _orig_font_height( void)
{
   return (PDC_orientation & 1) ? PDC_font_info.width : PDC_font_info.height;
}

bool PDC_remove_mouse_cursor( void)
{
   const int mx = PDC_mouse_x - cursor_data[1];
   const int my = PDC_mouse_y - cursor_data[2];
   int left = mx / _orig_font_width( );
   int right = (mx + cursor_data[0] - 1) / _orig_font_width( );
   int y = my / _orig_font_height( );
   int bottom = (my + cursor_data[0] - 1) / _orig_font_height( );

   if( left < 0)
      left = 0;
   if( right > SP->cols - 1)
      right = SP->cols - 1;
   if( y < 0)
      y = 0;
   _mouse_cursor_shown = FALSE;
   assert( right >= left);
   while( y <= bottom && y < SP->lines && right >= left)
      {
      PDC_transform_line_sliced( y, left, right - left + 1, curscr->_y[y] + left);
      y++;
      }
   _mouse_cursor_shown = TRUE;
   return( TRUE);
}

bool PDC_update_mouse_cursor( int left, int right, int top, int bottom, const bool draw_it)
{
   const int mouse_size = cursor_data[0], bytes_per_line = (mouse_size + 7) >> 3;
   const int mx = PDC_mouse_x - cursor_data[1];
   const int my = PDC_mouse_y - cursor_data[2];

   mouse_pixel = rand( ) & 0xffffff;
   if( left < mx)
      left = mx;
   if( right >= mx + mouse_size)
      right = mx + mouse_size - 1;
   if( right <= left)      /* cursor is off the left or right edge */
      return FALSE;
   if( top < my)
      top = my;
   if( bottom >= my + mouse_size)
      bottom = my + mouse_size - 1;
   if( top >= bottom)      /* cursor above or below desired rectangle */
      return FALSE;
   if( draw_it)
      {
      const unsigned char *mask1 = cursor_data + 3 + (top - my) * bytes_per_line;
      const unsigned char *mask2 = mask1 + bytes_per_line * mouse_size;
      int x, y, i;

      for( y = top; y < bottom; y++)
         {
         for( x = left, i = left - mx; x < right; x++, i++)
            if( (mask1[i >> 3] >> (i & 7)) & 1)
               _draw_pixel( x, y, (mask2[i >> 3] >> (i & 7)) & 1);
         mask1 += bytes_per_line;
         mask2 += bytes_per_line;
         }
      }
   return( TRUE);
}

#endif         /* #ifdef HAVE_MOUSE */

/* The framebuffer appears to store red,  green,  and blue in the opposite
order from what the other platforms expect : */

#define SWAP_RED_AND_BLUE( rgb) (((rgb) & 0xff00) | ((rgb) >> 16) | (((rgb) & 0xff) << 16))

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    const int font_char_size_in_bytes = (PDC_font_info.width + 7) >> 3;
    const int line_len = PDC_fb.line_length * 8 / PDC_fb.bits_per_pixel;
    uint8_t scratch[300];
    bool is_fullwidth = FALSE;
    chtype cursor_ch;
#ifdef HAVE_MOUSE
    const int t_width = _orig_font_width( );
    const int left = x * t_width, right = (x + len) * t_width;
#endif

    assert( srcp);
    assert( x >= 0);
    assert( len <= SP->cols - x);
    assert( lineno >= 0);
    assert( lineno < SP->lines);
    assert( len > 0);
    fullwidth_offset = 0;
    if( SP->drawing_cursor >= 2 && SP->drawing_cursor <= 4)
    {           /* blink full cell, outlined box, or caret */
        const chtype flags[3] = { A_REVERSE, SIDE_LINE_ATTRIBS, A_LEFT };

        cursor_ch = *srcp ^ flags[SP->drawing_cursor - 2];
        srcp = &cursor_ch;
    }
    if( x + len < SP->cols)     /* check for fullwidth character at end */
      if( _is_fullwidth_glyph( srcp[len - 1]))
         {
         if( len > 1)
            PDC_transform_line( lineno, x, len - 1, srcp);
         x += len - 1;
         srcp += len - 1;
         len = 2;
         is_fullwidth = TRUE;
         }
    while( len)
    {
        int run_len = 0, x1, y1;
        PACKED_RGB fg, bg;
        long video_offset, next_glyph;

        assert( PDC_orientation >= 0 && PDC_orientation < 4);
        switch( PDC_orientation)
            {
            case 0:   /* 'normal' */
               x1 = x;
               y1 = lineno;
               next_glyph = PDC_font_info.width;
               break;
            case 1:   /* rotated 90 degrees clockwise */
               x1 = SP->lines - lineno - 1;
               y1 = x;
               next_glyph = PDC_font_info.height * line_len;
               break;
            case 2:   /* 180-degree rotation */
               x1 = (SP->cols - x) - 1;
               y1 = (SP->lines - lineno) - 1;
               next_glyph = -(long)PDC_font_info.width;
               break;
            case 3:   /* rotated 90 degrees CCW */
               x1 = lineno;
               y1 = SP->cols - x - 1;
               next_glyph = -(long)PDC_font_info.height * line_len;
               break;
            }
        video_offset = x1 * PDC_font_info.width
                     + y1 * PDC_font_info.height * line_len;

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
            uint32_t *tptr = (uint32_t *)PDC_fb.framebuf + video_offset;

            len -= run_len;
            x += run_len;
            while( run_len--)
            {
                const uint8_t *fontptr = _get_rotated_glyph( *srcp, SP->drawing_cursor, scratch);
                uint32_t *fb_ptr = tptr;
                int i, j;

                for( i = 0; i < (int)PDC_font_info.height; i++)
                {
                    for( j = 0; j < (int)PDC_font_info.width; j++)
                        *fb_ptr++ = ((fontptr[j >> 3] << (j & 7)) & 0x80) ? fg : bg;
                    fb_ptr += line_len - PDC_font_info.width;
                    fontptr += font_char_size_in_bytes;
                }
                if( !is_fullwidth)
                   srcp++;
                else
                    fullwidth_offset = 0x200000;
                tptr += next_glyph;
            }
        }
        if( PDC_fb.bits_per_pixel == 8)
        {
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
                if( SP->blink_state)
                    reverse_colors ^= 1;
            }
            if( reverse_colors)
            {
                const int swapval = integer_fg_idx;
                integer_fg_idx = integer_bg_idx;
                integer_bg_idx = swapval;
            }
            if( integer_bg_idx == -1)
               integer_bg_idx = 0;
            fg_idx = (uint8_t)integer_fg_idx;
            bg_idx = (uint8_t)integer_bg_idx;
            for( i = 0; i < run_len; i++)
            {
                const uint8_t *fontptr = _get_rotated_glyph( *srcp, SP->drawing_cursor, scratch);
                uint8_t *fb_ptr = tptr;
                int k, j;

                for( k = 0; k < (int)PDC_font_info.height; k++)
                {
                    for( j = 0; j < (int)PDC_font_info.width; j++)
                        *fb_ptr++ = ((fontptr[j >> 3] << (j & 7)) & 0x80) ? fg_idx : bg_idx;
                    fb_ptr += PDC_fb.line_length - PDC_font_info.width;
                    fontptr += font_char_size_in_bytes;
                }
                if( !is_fullwidth)
                   srcp++;
                else
                    fullwidth_offset = 0x200000;
                len--;
                tptr += next_glyph;
                x++;
            }
        }
    }
#ifdef HAVE_MOUSE
    if( _mouse_cursor_shown)
        PDC_update_mouse_cursor( left, right, lineno * _orig_font_height( ),
                                   (lineno + 1) * _orig_font_height( ), TRUE);
#endif
}

void PDC_doupdate(void)
{
}
