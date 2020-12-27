#include <stdio.h>
#include <curses.h>

/* PDCurses/ncurses speed test.  Initializes *curses,  displays
semi-random numbers on the screen for 3000 milliseconds = 3 seconds,
and then tells you how many frames per second you got (i.e.,  how
many complete screen refreshes).

   Results on my somewhat elderly machine,  all in Xubuntu 18.04 :

SDL1 : 140 fps in wide mode,  340 in non-wide
SDL2: 233 fps in wide mode, 624 fps in non-wide mode
X11 wide mode: 180 fps first time,  800 fps subsequently
ncurses : 3700 fps in wide mode
VT : 3600 fps in wide mode,  4500 fps in non-wide mode
WinGUI : 280 fps,  both modes (through Wine)
WinCon : 840 fps,  wide mode;  75 fps in non-wide
DOS (with DosBox) : 223 fps
DOSVGA (with DosBox) : 50 fps

   SDL2 uses TrueType (R) fonts in wide mode,  as opposed to simple
bitmapped fonts in 8-bit mode.  So its ability to be blazingly fast
in the latter is unsurprising.

   X11,  I'll guess,  has some sort of optimization for wide-char fonts
(because they get more use),  and perhaps some sort of caching that
would account for why the first load is a little slower.

   Dunno why Win32 console has such a huge variation,  and haven't
tested it under "real" Windows.
*/

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

/* ftime() is consided obsolete.  But it's all we have for
millisecond precision on older compilers/systems.  We'll
use gettimeofday() when available.        */

#if defined(__TURBOC__) || defined(__EMX__) || defined(__DJGPP__) || \
    defined( __DMC__) || defined(__WATCOMC__) || defined(_MSC_VER)
#include <sys/timeb.h>

static long millisec_clock( )
{
    struct timeb t;

    ftime( &t);
    return( (long)t.time * 1000L + (long)t.millitm);
}
#else
#include <sys/time.h>

static long millisec_clock( )
{
    struct timeval t;

    gettimeofday( &t, NULL);
    return( t.tv_sec * 1000 + t.tv_usec / 1000);
}
#endif

int main( const int argc, const char **argv)
{
    unsigned n_frames = 0;
    long t0;

    INTENTIONALLY_UNUSED_PARAMETER( argv);
    INTENTIONALLY_UNUSED_PARAMETER( argc);
    resize_term( 30, 90);
    initscr();
    cbreak( );
    noecho();
    refresh();
    keypad( stdscr, 1);
    nodelay(stdscr, TRUE);
    t0 = millisec_clock( );
    while( millisec_clock( ) - t0 < 3000L && getch( ) != 'q')
      {
      char buff[11];
      int i, j;
      const int lines = 25, cols = 80;

      n_frames++;
      sprintf( buff, " %9u", (n_frames * 31415926u) % 1000000000u);
      attrset(A_BOLD);
      for( i = 0; i < lines; i++)
         {
         move( i, 0);
         if( i == (int)n_frames % lines)
            attrset( A_NORMAL);
         for( j = 0; j + 10 < cols; j += 10)
            addstr( buff);
         }
      }
    nodelay(stdscr, FALSE);
    move( LINES / 2, COLS / 2 - 20);
    printw( "  %u frames/sec  ", n_frames / 3);
    getch( );
    endwin();
    printf( "%u frames per second\n", n_frames / 3);
    return( 0);
}
