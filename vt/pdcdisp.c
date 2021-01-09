#define USE_UNICODE_ACS_CHARS 1

#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#define USE_UNICODE_ACS_CHARS 1

#include "curspriv.h"
#include "pdcvt.h"
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

void PDC_gotoyx(int y, int x)
{
   char tbuff[50];

   snprintf( tbuff, sizeof( tbuff), "\033[%d;%dH", y + 1, x + 1);
   PDC_puts_to_stdout( tbuff);
}

#define RESET_ATTRS   "\033[0m"
#define ITALIC_ON     "\033[3m"
#define ITALIC_OFF    "\033[23m"
#define UNDERLINE_ON  "\033[4m"
#define UNDERLINE_OFF "\033[24m"
#define BLINK_ON      "\033[5m"
#define BLINK_OFF     "\033[25m"
#define BOLD_ON       "\033[1m"
#define BOLD_OFF      "\033[22m"
#define DIM_ON        "\033[2m"
#define DIM_OFF       "\033[22m"
#define REVERSE_ON    "\033[7m"
#define STRIKEOUT_ON  "\033[9m"

const chtype MAX_UNICODE = 0x110000;

/* see 'addch.c' for an explanation of how combining chars are handled. */

#ifdef USING_COMBINING_CHARACTER_SCHEME
   int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);  /* addch.c */
#endif

static void color_string( char *otext, const PACKED_RGB rgb)
{
   extern bool PDC_has_rgb_color;      /* pdcscrn.c */
   const int red = Get_RValue( rgb);
   const int green = Get_GValue( rgb);
   const int blue = Get_BValue( rgb);

   if( PDC_has_rgb_color)
      sprintf( otext, "2;%d;%d;%dm", red, green, blue);
   else
      {
      int idx;

      if( red == green && red == blue)   /* gray scale: indices from */
         {
         if( red < 27)     /* this would underflow; remap to black */
            idx = COLOR_BLACK;
         else if( red >= 243)    /* this would overflow */
            idx = COLOR_WHITE;
         else
            idx = (red - 3) / 10 + 232;     /* 232 to 255 */
         }
      else
         idx = ((blue - 35) / 40) + ((green - 35) / 40) * 6
                  + ((red - 35) / 40) * 36 + 16;

      sprintf( otext, "5;%dm", idx);
      }
}

static int get_sixteen_color_idx( const PACKED_RGB rgb)
{
    int rval = 0;

    if( rgb & 0x80)    /* red value >= 128 */
        rval = 1;
    if( rgb & 0x8000)      /* green value >= 128 */
        rval |= 2;
    if( rgb & 0x800000)        /* blue value >= 128 */
        rval |= 4;
    return( rval);
}

static void reset_color( char *obuff, const chtype ch)
{
    static PACKED_RGB prev_bg = (PACKED_RGB)-1;
    static PACKED_RGB prev_fg = (PACKED_RGB)-1;
    PACKED_RGB bg, fg;

    if( !obuff)
        {
        prev_bg = prev_fg = (PACKED_RGB)-1;
        return;
        }
    PDC_get_rgb_values( ch, &fg, &bg);
    *obuff = '\0';
    if( bg != prev_bg)
        {
        if( bg == (PACKED_RGB)-1)   /* default background */
            strcpy( obuff, "\033[49m");
        else if( !bg)
            strcpy( obuff, "\033[40m");
        else if( COLORS == 16)
            sprintf( obuff, "\033[4%dm", get_sixteen_color_idx( bg));
        else
            {
            strcpy( obuff, "\033[48;");
            color_string( obuff + 5, bg);
            }
        prev_bg = bg;
        }

    if( fg != prev_fg)
        {
        obuff += strlen( obuff);
        if( fg == (PACKED_RGB)-1)   /* default foreground */
            strcpy( obuff, "\033[39m");
        else if( COLORS == 16)
            sprintf( obuff, "\033[3%dm", get_sixteen_color_idx( fg));
        else
            {
            strcpy( obuff, "\033[38;");
            color_string( obuff + 5, fg);
            }
        prev_fg = fg;
        }
}

int PDC_wc_to_utf8( char *dest, const int32_t code);

#define OBUFF_SIZE 100

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    static chtype prev_ch = 0;
    static bool force_reset_all_attribs = TRUE;
    char obuff[OBUFF_SIZE];

    if( !srcp)
    {
        prev_ch = 0;
        force_reset_all_attribs = TRUE;
        PDC_puts_to_stdout( RESET_ATTRS);
        return;
    }
    assert( x >= 0);
    assert( len <= SP->cols - x);
    assert( lineno >= 0);
    assert( lineno < SP->lines);
    assert( len > 0);
    PDC_gotoyx( lineno, x);
    if( force_reset_all_attribs || (!x && !lineno))
    {
        force_reset_all_attribs = FALSE;
        prev_ch = ~*srcp;
    }
    while( len)
    {
       int ch = (int)( *srcp & A_CHARTEXT), count = 1;
       chtype changes = *srcp ^ prev_ch;
       size_t bytes_out = 0;

       if( (*srcp & A_ALTCHARSET) && ch < 0x80)
          ch = (int)acs_map[ch & 0x7f];
       if( ch < (int)' ' || (ch >= 0x80 && ch <= 0x9f))
          ch = ' ';
       *obuff = '\0';
       if( (prev_ch & ~*srcp) & (A_REVERSE | A_STRIKEOUT))
       {
          prev_ch = 0;
          changes = *srcp;
          strcpy( obuff, RESET_ATTRS);
          reset_color( NULL, 0);
       }
       if( SP->termattrs & changes & A_BOLD)
          strcat( obuff, (*srcp & A_BOLD) ? BOLD_ON : BOLD_OFF);
       if( changes & A_UNDERLINE)
          strcat( obuff, (*srcp & A_UNDERLINE) ? UNDERLINE_ON : UNDERLINE_OFF);
       if( changes & A_ITALIC)
          strcat( obuff, (*srcp & A_ITALIC) ? ITALIC_ON : ITALIC_OFF);
       if( changes & A_REVERSE)
          strcat( obuff, REVERSE_ON);
#ifndef _WIN32                /* MS doesn't support strikeout text */
       if( changes & A_STRIKEOUT)
          strcat( obuff, STRIKEOUT_ON);
#endif
       if( SP->termattrs & changes & A_BLINK)
          strcat( obuff, (*srcp & A_BLINK) ? BLINK_ON : BLINK_OFF);
       if( changes & (A_COLOR | A_STANDOUT | A_BLINK | A_REVERSE))
          reset_color( obuff + strlen( obuff), *srcp & ~A_REVERSE);
       PDC_puts_to_stdout( obuff);
#ifdef USING_COMBINING_CHARACTER_SCHEME
       if( ch > (int)MAX_UNICODE)      /* chars & fullwidth supported */
       {
           cchar_t root, newchar;

           root = ch;
           while( (root = PDC_expand_combined_characters( root,
                              &newchar)) > MAX_UNICODE)
               ;
           bytes_out = PDC_wc_to_utf8( obuff, (wchar_t)root);
           root = ch;
           while( (root = PDC_expand_combined_characters( root,
                              &newchar)) > MAX_UNICODE)
               {
               bytes_out += PDC_wc_to_utf8( obuff + bytes_out, (wchar_t)newchar);
               if( bytes_out > OBUFF_SIZE - 6)
                  {
                  put_to_stdout( obuff, bytes_out);
                  bytes_out = 0;
                  }
               }
           bytes_out += PDC_wc_to_utf8( obuff + bytes_out, (wchar_t)newchar);
       }
       else if( ch < (int)MAX_UNICODE)
#endif
       {
           bytes_out = PDC_wc_to_utf8( obuff, (wchar_t)ch);
           while( count < len && !((srcp[0] ^ srcp[count]) & ~A_CHARTEXT)
                        && (ch = (srcp[count] & A_CHARTEXT)) < MAX_UNICODE)
           {
               if( (srcp[count] & A_ALTCHARSET) && ch < 0x80)
                  ch = (int)acs_map[ch & 0x7f];
               if( ch < (int)' ' || (ch >= 0x80 && ch <= 0x9f))
                  ch = ' ';
               bytes_out += PDC_wc_to_utf8( obuff + bytes_out, (wchar_t)ch);
               if( bytes_out > OBUFF_SIZE - 6)
                  {
                  put_to_stdout( obuff, bytes_out);
                  bytes_out = 0;
                  }
               count++;
           }
       }
       put_to_stdout( obuff, bytes_out);
       bytes_out = 0;
       prev_ch = *srcp;
       srcp += count;
       len -= count;
   }
}

void PDC_doupdate(void)
{
    put_to_stdout( NULL, 0);
}
