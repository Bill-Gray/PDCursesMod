#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>

#include "../demos/snprintf.c"

#define PDC_NCMOUSE

#include "curses.h"
#include <math.h>
#include <assert.h>
#ifndef _WIN32
    #define CONVERT_IMAGES
    #include <unistd.h>
#endif

#ifdef __WATCOMC__
   #include <io.h>
#endif

#ifdef WACS_S1
# define HAVE_WIDE
#endif

/* This is inspired by,  but not really based on,  Thomas E. Dickey's
'picsmap' program for ncurses.  Its real purpose is to demonstrate the
ability to use large palettes (basically,  full RGB) and large numbers
of color pairs (up to 2^20 = 1048576 pairs).  That ability is currently
available only in the VT and WinGUI flavors of PDCurses,  but will be
expanded to X11,  SDLx,  and probably WinCon.

   Run with an image file as a command-line argument.  The image will
be displayed using PDCurses' extended color palette and color pairs.
You can pan around the image with the mouse and keyboard, and zoom
in/out using the mouse wheel,  and hit 'r' to rotate the image 90 degrees.

   If you don't specify an image file,  a fake image showing color
gradients is generated.  On Linux,  it'll use 'convert' (ImageMagick)
to turn JPEGs or other image files into PNMs or PGMs.  On DOS/Windows,
you have to give it a PNM or PGM file.

   This program relies on the availability of a 2^24+256=16777472-color
palette and 2^20=1048576 color pairs.  Thus,  any color can be specified,
and up to a million combinations of foreground and background color.
In each cell,  an ACS_BBLOCK ('bottom half block') is shown;  the
bottom half then appears in the foreground color for that character
cell,  and the upper half in the background color for that cell.  Thus,
the 'pixels' making up our image are one character wide,  but only
half a character high.

   We start out with the image scaled to fit entirely within the window.
The loop below goes through 2 * LINES iterations.  On even passes,  we
compute RGB values for the pixels that will go into the top half of each
character cell.  On odd passes,  we're computing the RGB for the bottom
halves,  and each time one is computed,  we spit out another character.
(And,  if the colors have changed,  we allocate another color pair.
So this can chew through as many color pairs as there are character
cells on the current screen,  or LINES * COLS.  In practice,  'pair_num'
is reset at twice that value.  This avoids flickering that could be
caused by setting a color pair for the 'new' image that was in use by
the 'old' image that we're replacing.)  */

double aspect = 1.1;       /* The 'bottom block' (half-height cell character) */
               /* is _almost_ square,  but is about 10% wider than it is tall */

/* PNM files give each pixel as an RGB triplet of bytes.  A width by height
image consumes exactly width * height * 3 bytes. */

#define UNICODE_BBLOCK      0x2584
#ifndef ACS_BBLOCK
   #define ACS_BBLOCK       0xdc
#endif

static int32_t get_rgb_value( const char *iptr)
{
   unsigned char red = (unsigned char)iptr[0];
   unsigned char grn = (unsigned char)iptr[1];
   unsigned char blu = (unsigned char)iptr[2];

   return( (int32_t)red | ((int32_t)grn << 8) | ((int32_t)blu << 16));
}

/* Image rotation isn't done fancily here.  We allocate another array of equal
size,  copy pixels into it rearranged to their rotated positions,  and copy
that new array back into the original.  */

static void rotate_pixels_ninety_degrees( const int xsize, const int ysize, char *pixels)
{
   int i, j;
   char *tmp = (char *)malloc( xsize * ysize * 3);

   assert( tmp);
   for( i = 0; i < ysize; i++)
      {
      char *tptr = tmp + (ysize - 1 - i) * 3;

      for( j = xsize; j; j--)
         {
         *tptr++ = *pixels++;
         *tptr++ = *pixels++;
         *tptr++ = *pixels++;
         tptr += (ysize - 1) * 3;
         }
      }
   pixels -= xsize * ysize * 3;
   memcpy( pixels, tmp, xsize * ysize * 3);
   free( tmp);
}

static void invert_pixels( const int xsize, const int ysize, char *pixels)
{
   char *endpix = pixels + (xsize * ysize - 1) * 3;

   while( endpix > pixels)
      {
      char tbuff[3];

      tbuff[0] = pixels[0];  pixels[0] = endpix[0];   endpix[0] = tbuff[0];
      tbuff[1] = pixels[1];  pixels[1] = endpix[1];   endpix[1] = tbuff[1];
      tbuff[2] = pixels[2];  pixels[2] = endpix[2];   endpix[2] = tbuff[2];
      pixels += 3;
      endpix -= 3;
      }
}

static void mirror_pixels_top_bottom( const int xsize, const int ysize, char *pixels)
{
   char *tbuff = (char *)malloc( xsize * 3);
   char *tptr1 = pixels, *tptr2 = pixels + (ysize - 1) * xsize * 3;

   while( tptr2 > tptr1)
      {
      memcpy( tbuff, tptr1, 3 * xsize);
      memcpy( tptr1, tptr2, 3 * xsize);
      memcpy( tptr2, tbuff, 3 * xsize);
      tptr1 += xsize * 3;
      tptr2 -= xsize * 3;
      }
   free( tbuff);
}

static void mirror_pixels_left_right( const int xsize, int ysize, char *pixels)
{
   mirror_pixels_top_bottom( xsize, ysize, pixels);
   invert_pixels( xsize, ysize, pixels);
}

/* Make a reasonably interesting image with gradients,  circles,
and hyperbolas */

static void make_fake_image( const char *filename)
{
   FILE *ofile = fopen( filename, "wb");
   int i, j, xsize = 640, ysize = 480, r = 150, r1 = 200;
   unsigned char buff[3];

   fprintf( ofile, "P6\n%d %d\n255\n", xsize, ysize);
   for( i = 0; i < ysize; i++)
      for( j = 0; j < xsize; j++)
         {
         const int dx = j - xsize / 2;
         const int dy = i - ysize / 2;

         buff[0] = (unsigned char)( i * 255 / ysize);
         buff[1] = (unsigned char)( j * 255 / xsize);
         if( dx * dx + dy * dy < r1 * r1)
            buff[1] ^= 0xff;
         if( abs( dx * dx - dy * dy) > r * r)
            buff[2] = 0xc0;
         else
            buff[2] = 0;
         fwrite( buff, 3, 1, ofile);
         }
   fclose( ofile);
}

/* Compute dither offset.  See https://en.wikipedia.org/wiki/Ordered_dithering
for explanation.  This would correspond to a 16x16 'threshold map'.  */

static int calc_dither( int x, int y)
{
   int rval = 1;

   while( !(rval & 0x100))
      {
      rval <<= 2;
      rval |= (x & 1) | ((y & 1) << 1);
      x >>= 1;
      y >>= 1;
      }
   return( rval & 0xff);
}

/* For 'full-color' palettes,  the mapping from RGB to default palette
entry is a simple shift by 256.  For 'traditional' 256-color systems
with a 6x6x6 color cube,  the math is slightly harder.  Dithering is
used to make the results marginally less ugly. */

static int find_in_palette( const int32_t rgb, const int dither)
{
#if !defined( CHTYPE_32) && INT_MAX > 65536
   if( COLORS > 0x100000)
      return( rgb + 256);
   else
#endif
      {        /* find entry in 6x6x6 color cube */
      const int red = (rgb & 0xff);
      const int grn = ((rgb >> 8) & 0xff);
      const int blu = ((rgb >> 16) & 0xff);

      return( 16 + ((blu * 5 + dither) / 256)
                 + ((grn * 5 + dither) / 256) * 6
                 + ((red * 5 + dither) / 256) * 36);
      }
}

#if defined( __linux)
    const char *temp_image_name = "/tmp/ickywax.pnm";
#else
    const char *temp_image_name = "ickywax.pnm";
#endif

int main( const int argc, const char *argv[])
{
   FILE *ifile;
   int xsize, ysize;
   char *pixels, buff[100];
   const char *filename = temp_image_name;
   const char *filename_to_show = temp_image_name;
   int c = 0, i, bytes_per_pixel = 3, pair_num = 16;
   double scale = 0., xpix = 0., ypix = 0.;
   bool show_help = TRUE;
   SCREEN *screen_pointer;

   setlocale(LC_ALL, ".utf8");
   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-')
         switch( argv[i][1])
            {
            case 'a':
               aspect = atof( argv[i] + 2);
               break;
            }
   if( argc > 1 && argv[1][0] != '-')
      {
      filename_to_show = argv[1];
#ifdef CONVERT_IMAGES
      snprintf( buff, sizeof( buff), "convert %s %s", argv[1], temp_image_name);
      if( system( buff))
         {
         printf( "Couldn't convert %s\n", filename);
         return( -1);
         }
#else
      filename = argv[1];
      temp_image_name = NULL;
#endif
      }
   else
      make_fake_image( temp_image_name);
   ifile = fopen( filename, "rb");
   if( !ifile)
      {
      printf( "'%s' not opened\n", filename_to_show);
      return( -2);
      }
   if( !fgets( buff, sizeof( buff), ifile) || *buff != 'P'
               || buff[1] > '6' || buff[1] < '5')
      {
      printf( "'%s' was not a .pnm file\n", filename_to_show);
      return( -3);
      }
   if( buff[1] == '5')
      bytes_per_pixel = 1;
   if( fgets( buff, sizeof( buff), ifile))
      sscanf( buff, "%d %d", &xsize, &ysize);
   else
      {
      fprintf( stderr, "Unable to read '%s'\n", filename_to_show);
      return( -9);
      }
   if( !fgets( buff, sizeof( buff), ifile))
      return( -4);
   assert( xsize < 300000 && ysize < 300000);
   pixels = (char *)calloc( xsize, ysize * 3);
   assert( pixels);
   if( !fread( pixels, xsize * ysize, bytes_per_pixel, ifile))
      {
      printf( "%d x %d pixels not read\n", xsize, ysize);
      return( -5);
      }
   fclose( ifile);
   if( temp_image_name)
#ifdef _WIN32
      _unlink( temp_image_name);
#else
      unlink( temp_image_name);
#endif
   if( bytes_per_pixel == 1)     /* expand grayscale to RGB */
      for( i = xsize * ysize - 1; i >= 0; i--)
         pixels[i * 3] = pixels[i * 3 + 1] = pixels[i * 3 + 2] = pixels[i];
   screen_pointer = newterm(NULL, stdout, stdin);
   start_color( );
   cbreak( );
   noecho();
   keypad( stdscr, 1);
   mousemask( ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
   while( c != 27 && c != 'q')
      {
      int *xloc = (int *)calloc( 2 * COLS, sizeof( int));
      int *idxs = xloc + COLS;
      int prev_idx = -1, prev_low_idx = -1;
      int j;
      MEVENT mouse_event;
      double xpix1, ypix1;

      assert( xloc);
      memset( &mouse_event, 0, sizeof( MEVENT));
      if( !scale)       /* recompute scale */
         {
         double scale1 = (double)xsize / (double)COLS;

         scale = (double)ysize / ((double)LINES * 2. * aspect);
         if( scale < scale1)
            scale = scale1;
         xpix = (double)xsize / 2.;
         ypix = (double)ysize / 2.;
         }
      for( i = 0; i < COLS; i++)
         xloc[i] = (int)( xpix + (double)i * scale) - (int)( COLS * scale) / 2;
      erase( );
      for( j = 0; j < LINES * 2; j++)
         {
         const int yloc = (int)( ypix + (double)j * scale * aspect)
                        - (int)( LINES * scale * aspect);
         const char *pptr = pixels + yloc * 3 * xsize;

         if( yloc < 0 || yloc >= ysize)
            pptr = NULL;
         for( i = 0; i < COLS && xloc[i] < 0; i++)
            ;
         if( !(j % 2))
            {
            memset( idxs, 0, COLS * sizeof( int));
            if( pptr)
               for( ; i < COLS && xloc[i] < xsize; i++)
                  idxs[i] = find_in_palette( get_rgb_value( pptr + xloc[i] * 3),
                                    calc_dither( i, j));
            }
         else
            {
            move( j / 2, i);
            for( ; i < COLS && xloc[i] < xsize; i++)
               {
               int low_rgb = (pptr ? get_rgb_value( pptr + xloc[i] * 3) : 0);
               int low_idx = find_in_palette( low_rgb, calc_dither( i, j));
#ifdef HAVE_WIDE
               wchar_t bblock_char = (wchar_t)UNICODE_BBLOCK;
#endif

               if( low_idx != prev_low_idx || idxs[i] != prev_idx)
                  {
#ifdef NCURSES_VERSION
                  init_pair( pair_num, low_idx, idxs[i]);
#else
                  init_extended_pair( pair_num, low_idx, idxs[i]);
#endif
                  attrset( COLOR_PAIR( pair_num));
                  pair_num++;
                  if( pair_num == 2 * LINES * COLS + 16)   /* see comments */
                     pair_num = 16;        /* above about avoiding flicker */
                  prev_low_idx = low_idx;
                  prev_idx = idxs[i];
                  }
#ifdef HAVE_WIDE
               addnwstr( &bblock_char, 1);
#else
               addch( ACS_BBLOCK);
#endif
               }
            }
         }
      free( xloc);
      attrset( 0);
      mvaddstr( LINES - 1, 0, filename_to_show);
      snprintf( buff, sizeof( buff), " %d x %d pixels", xsize, ysize);
      addstr( buff);
      snprintf( buff, sizeof( buff), "     %d color pairs used", pair_num);
      addstr( buff);
      if( show_help)
         {
         mvaddstr( LINES - 8, 0, "Home key sets default view (image fit to screen)");
         mvaddstr( LINES - 7, 0, "Click mouse to pan;  scroll wheel to zoom in/out");
         mvaddstr( LINES - 6, 0, "Cursor keys also pan; * zooms in, / zooms out");
         mvaddstr( LINES - 5, 0, "'r' rotates clockwise, 'R' CCW, 'i' inverts image");
         mvaddstr( LINES - 4, 0, "'m' mirrors image left/right, 'f' flips top/bottom");
         mvaddstr( LINES - 3, 0, "Any other key causes this help to appear");
         show_help = FALSE;
         }
      refresh();
      xpix1 = xpix;
      ypix1 = ypix;
      do
         {
         c = getch( );
         if( c == KEY_MOUSE)
            {
            getmouse( &mouse_event);
            xpix1 = xpix + (double)(mouse_event.x - COLS / 2) * scale;
            ypix1 = ypix + (double)(mouse_event.y - LINES / 2) * scale * 2. * aspect;
            if( mouse_event.bstate & REPORT_MOUSE_POSITION)
               {
               c = -1;
               snprintf( buff, sizeof( buff), "x=%5d y=%5d", (int)xpix1, (int)ypix1);
               mvaddstr( LINES - 2, 0, buff);
               }
            }
         }
         while( c == -1);
      switch( c)
         {
         case KEY_MOUSE:
            {
            double rescale = 1.;

            if( mouse_event.bstate & BUTTON4_PRESSED)
               rescale = 1. / 1.2;
#ifdef __PDCURSES__
            else if( mouse_event.bstate & BUTTON5_PRESSED)
               rescale = 1.2;
#endif
            else
               {
               xpix = xpix1;
               ypix = ypix1;
               }
            scale *= rescale;
            xpix += (xpix1 - xpix) * (1. - rescale);
            ypix += (ypix1 - ypix) * (1. - rescale);
            }
            break;
         case '*':
            scale /= 1.2;
            break;
         case '/':
            scale *= 1.2;
            break;
         case KEY_A1:
         case KEY_HOME:
            scale = 0.;       /* recompute/recenter */
            break;
         case KEY_UP:
#ifdef KEY_A2
         case KEY_A2:
#endif
            ypix -= LINES * scale / 2.;
            break;
         case KEY_LEFT:
#ifdef KEY_B1
         case KEY_B1:
#endif
            xpix -= COLS * scale / 4.;
            break;
         case KEY_DOWN:
#ifdef KEY_C2
         case KEY_C2:
#endif
            ypix += LINES * scale / 2.;
            break;
         case KEY_RIGHT:
#ifdef KEY_B3
         case KEY_B3:
#endif
            xpix += COLS * scale / 4.;
            break;
#ifdef KEY_RESIZE
         case KEY_RESIZE:
            resize_term( 0, 0);
            break;
#endif
         case 'r':
         case 'R':
            {
            const int tval = xsize;

            rotate_pixels_ninety_degrees( xsize, ysize, pixels);
            if( c == 'R')
               invert_pixels( xsize, ysize, pixels);
            xsize = ysize;
            ysize = tval;
            scale = 0.;
            }
            break;
         case 'i':
            invert_pixels( xsize, ysize, pixels);
            break;
         case 'f':
            mirror_pixels_top_bottom( xsize, ysize, pixels);
            ypix = ysize - ypix;
            break;
         case 'm':
            mirror_pixels_left_right( xsize, ysize, pixels);
            xpix = xsize - xpix;
            break;
         default:
            show_help = TRUE;
            break;
         }
      }
   free( pixels);
   endwin();
                            /* Not really needed,  but ensures Valgrind  */
   delscreen( screen_pointer);          /* says all memory was freed */
   return( 0);
}
