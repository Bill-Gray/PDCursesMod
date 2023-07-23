/* Tests alloc_pair( ) family of ncurses extension functions.  */

#define PDC_NCMOUSE

#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <assert.h>
#include <locale.h>

#ifdef WACS_S1
# define HAVE_WIDE
#endif

/* Let's say character cells are twice as high as they are wide.
Font-dependent and we don't really know,  but it'll be pretty close. */

const double aspect_ratio = 2.0;

static int _get_mandelbrot( const double x, const double y, const int max_iter)
{
   double x1 = x, y1 = y;
   int i = max_iter;

   while( i--)
   {
      const double new_x1 = x1 * x1 - y1 * y1 + x;

      if( x1 * x1 + y1 * y1 > 4.)
         return( max_iter - i);
      y1 = 2. * x1 * y1 + y;
      x1 = new_x1;
   }
   return( 0);
}

/* Remap a value from 0 to 1 to an RGB pseudocolor value */

static long cvt_to_rgb( const double ival)
{
   const double scaled = ival * 9.999;
   long red, grn, blu;
   const int range = (int)scaled;
   const int remain = (int)( 256. * (scaled - (double)range));
   const short palette[33] = {      /* a reasonably appealing pseudocolor palette */
          0,   0, 192,
          0,  96, 192,
          0, 192, 255,
        160, 255,  96,
         80, 255,  48,
        168, 255,   0,
        255, 255,   0,
        255, 128, 128,
        255,   0, 255,
        255,   0, 128,
        255,   0,   0 };
   const short *tptr = &palette[range * 3];

   assert( ival >= 0.);
   assert( ival <= 1.);
   assert( range >= 0 && range < 10);
   red = (long)tptr[0] + (((long)(tptr[3] - tptr[0]) * remain) >> 8);
   assert( red >= 0 && red < 256);
   grn = (long)tptr[1] + (((long)(tptr[4] - tptr[1]) * remain) >> 8);
   assert( grn >= 0 && grn < 256);
   blu = (long)tptr[2] + (((long)(tptr[5] - tptr[2]) * remain) >> 8);
   assert( blu >= 0 && blu < 256);
   return( red | (grn << 8) | (blu << 16));
}

#define UNICODE_BBLOCK      0x2584
#define UNICODE_UBLOCK      0x2580
#define UNUSED_COLOR_INDEX   0

static int _n_colors;

#define COLOR0 2

static void show_mandelbrot( const double x0, const double y0,
            const double scale, const int max_iter)
{

    int i, j;

    for( j = 0; j < LINES; j++)
    {
        move( j, 0);
        for( i = 0; i < COLS; i++)
        {
            const double x = x0 + scale * (double)( i - COLS / 2);
            const double y = y0 - scale * (double)( j - LINES / 2) * aspect_ratio;
            const int top_iter = _get_mandelbrot( x, y, max_iter);
            const int bot_iter = _get_mandelbrot( x, y - scale * aspect_ratio / 2., max_iter);
            long top_rgb = 0, bot_rgb = 0;

            if( _n_colors > 32000)        /* use PDCursesMod's extended RGB */
            {
                if( top_iter > 1)
                    top_rgb = cvt_to_rgb( (double)top_iter / (double)max_iter) + 256;
                if( bot_iter > 1)
                    bot_rgb = cvt_to_rgb( (double)bot_iter / (double)max_iter) + 256;
            }
            else           /* use 'real' palette indexing */
            {
                if( top_iter > 1)
                    top_rgb = ((int)top_iter % _n_colors) + COLOR0;
                if( bot_iter > 1)
                    bot_rgb = ((int)bot_iter % _n_colors) + COLOR0;
            }
            if( bot_rgb < top_rgb)
            {
                int idx = alloc_pair( bot_rgb, top_rgb);
#ifdef HAVE_WIDE
                wchar_t bblock_char = (wchar_t)UNICODE_BBLOCK;
#endif

                color_set( UNUSED_COLOR_INDEX, &idx);
#ifdef HAVE_WIDE
                addnwstr( &bblock_char, 1);
#else
                addch( ACS_BBLOCK);
#endif
            }
            else
            {
                int idx = alloc_pair( top_rgb, bot_rgb);
#ifdef HAVE_WIDE
                wchar_t ublock_char = (wchar_t)UNICODE_UBLOCK;
#endif

                color_set( UNUSED_COLOR_INDEX, &idx);
#ifdef HAVE_WIDE
                addnwstr( &ublock_char, 1);
#else
                addch( ACS_UBLOCK);
#endif
            }
        }
    }
}

/* The following will return a value within 1.3e-6 of the actual natural
log of x.  Its only advantage is that it avoids a need to link in the
math library.  It uses the first three terms of the series expansion

ln( x) = 2(z + z^3/3 + z^5/5 + z^7/7 + ...)

where z = (x-1) / (x+1).  For sqrt(.5) < x < sqrt(2), -0.1716 < z < 0.1716,
so convergence is good enough for this particular use case.

n terms     worst error                   n terms     worst error
 1          3.5e-3                         4          2.9e-8
 2          6.1e-5                         5          7.1e-10
 3          1.3e-6   (this code)           6          1.8e-11        */

static double _approx_ln( double x)
{
   const double sqrt_2 =
          1.4142135623730950488016887242096980785696718753769480731766797379907325;
   const double ln_2 =
          0.6931471805599453094172321214581765680755001343602552541206800094933936;
   int two_power = 0;
   double z, z2;

   assert( x > 0.);
   if( x < sqrt_2 / 2.)
      return( -_approx_ln( 1. / x));
   while( x > sqrt_2)
      {
      x /= 2.;
      two_power++;
      }
   z = (x - 1.) / (x + 1.);
   z2 = z * z;
   return( z * (2. + z2 * (2. / 3 + z2 * 0.4)) + (double)two_power * ln_2);
}

static void splash_screen( void)
{
   mvprintw( 4, 0, "'mbrot' is a demonstration of the ncurses extended color functions");
   mvprintw( 5, 0, "alloc_pair(), free_pair(), find_pair().  Navigation keys are listed");
   mvprintw( 6, 0, "below.");
}

static void show_key_help( void)
{
   mvprintw( LINES - 3, 0, "Cursor keys to pan                * / to zoom in/out");
   mvprintw( LINES - 2, 0, "Home to return to initial view    + / - for more/fewer iterations");
   mvprintw( LINES - 1, 0, "Mouse buttons to pan              Mouse wheel to zoom in/out");
}

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
    int c = 0, max_iter = 256, i;
    double x = 0., y = 0., scale;
    bool show_splash_screen = TRUE, show_keys = TRUE;
    SCREEN *screen_pointer;
    FILE *input_fp = stdin;
#ifdef PDC_COLOR_PAIR_DEBUGGING_FUNCTIONS
    int results[5], rval;
#endif

    setlocale(LC_ALL, ".utf8");
    for( i = 1; i < argc; i++)
        if( argv[i][0] == '-')
            switch( argv[i][1])
            {
                case 'c':
                   _n_colors = atoi( argv[i] + 2);
                   break;
                case 'i':
                   input_fp = fopen( argv[i] + 2, "rb");
                   break;
                default:
                   fprintf( stderr, "Unrecognized parameter '%s'\n", argv[i]);
                   return( -1);
            }
    screen_pointer = newterm(NULL, stdout, input_fp);
    cbreak( );
    noecho( );
    clear( );
    keypad( stdscr, 1);
    start_color( );
#ifdef __PDCURSESMOD__
    mousemask( BUTTON1_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED | MOUSE_WHEEL_SCROLL, NULL);
#else
    mousemask( BUTTON1_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
#endif
#ifndef CHTYPE_32
    if( COLORS > 0x1000000 && COLOR_PAIRS >= 4096 && !_n_colors)
       _n_colors = COLORS;    /* use PDCursesMod's rgb palette */
    else
#endif
    if( !_n_colors)
    {
        while( _n_colors * (_n_colors + 1) / 2 < COLOR_PAIRS - COLOR0 &&
                         _n_colors < COLORS - COLOR0)
           _n_colors++;
        _n_colors--;
        for( i = 0; i < _n_colors; i++)
        {
            const long rgb = cvt_to_rgb( (double)i / (double)_n_colors);
            const int red = (rgb & 0xff);
            const int grn = ((rgb >> 8) & 0xff);
            const int blu = ((rgb >> 16) & 0xff);

            init_extended_color( i + COLOR0, red * 35 / 9, grn * 35 / 9, blu * 35 / 9);
        }
    }
    init_color( 1, 999, 999, 999);
    scale = 4. / (double)COLS;
    while( c != 'q' && c != 27)
    {
        init_pair( 1, 1, 0);
        show_mandelbrot( x, y, scale, max_iter);
        color_set( 1, NULL);
        if( show_splash_screen)
           splash_screen( );
        if( show_keys)
           show_key_help( );
        show_splash_screen = FALSE;
        show_keys = FALSE;
        mvprintw( 0, 0, "x=%f y=%f max_iter=%d zoom %.1f ", x, y, max_iter,
               -_approx_ln( scale * (double)COLS / 4.));
#ifdef PDC_COLOR_PAIR_DEBUGGING_FUNCTIONS
        rval = PDC_check_color_pair_table( results);
        assert( !rval);
        mvprintw( 1, 0, "%d used/%d free; total %d (%d); hash tbl size %d (%d used) ", results[0],
                     results[1], results[2], results[0] + results[1],
                                 results[3], results[4]);
#endif
        c = getch( );
        switch( c)
        {
            case KEY_MOUSE:
               {
                  MEVENT mouse_event;
                  int dx, dy;

                  getmouse( &mouse_event);
                  dx = mouse_event.x - COLS / 2;
                  dy = mouse_event.y - LINES / 2;
                  if( mouse_event.bstate & (BUTTON4_PRESSED | BUTTON5_PRESSED))
                     {
                     const double new_scale = scale *
                          ((mouse_event.bstate & BUTTON5_PRESSED) ? 1.1 : 1. / 1.1);

                     x -= dx * (new_scale - scale);
                     y += dy * (new_scale - scale) * aspect_ratio;
                     scale = new_scale;
                     }
                  else     /* button 1,  just pan over */
                     {
                     x += dx * scale;
                     y -= dy * scale * aspect_ratio;
                     }
               }
               break;
            case '*':
               scale /= 2.;
               break;
            case '/':
               scale *= 2.;
               break;
            case 'a':
            case KEY_LEFT:
               x -= scale * (double)COLS / 4.;
               break;
            case 's':
            case KEY_RIGHT:
               x += scale * (double)COLS / 4.;
               break;
            case 'w':
            case KEY_UP:
               y += scale * (double)LINES / 4.;
               break;
            case 'z':
            case KEY_DOWN:
               y -= scale * (double)LINES / 4.;
               break;
            case 'h':
            case KEY_HOME:
               x = y = 0.;
               scale = 4. / (double)COLS;
               max_iter = 32;
               break;
            case '+':
#ifdef PADPLUS
            case PADPLUS:
#endif
               max_iter <<= 1;
               break;
            case '-':
#ifdef PADMINUS
            case PADMINUS:
#endif
               if( max_iter)
                  max_iter >>= 1;
               break;
            case 'r':
               reset_color_pairs( );
               break;
            default:
               show_keys = TRUE;
               break;
        }
    }

    endwin( );
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    if( input_fp != stdin)
        fclose( input_fp);
    return( 0);
}
