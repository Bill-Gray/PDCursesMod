#include <curses.h>
#include <stdlib.h>
#include <stdio.h>

/* Small program to find out how many times a second you can call
getch( ) with no timeout;  i.e.,  how fast is getch( ) on various
platforms and curses implementations?  A result of investigating

https://github.com/Bill-Gray/PDCursesMod/issues/197

We note the starting time and go into a tight loop checking getch( ).  When
the time (in seconds) changes,  we post an updated count of iterations and
of iterations/second to the screen.  The count between screen updates is
doubled if a second hasn't passed and halved if two seconds have passed;  this
quickly sets 'freq' to a point where you get an update every second or so.

WinGUI on Wine : about 1.5 million/s
WinCon on Wine : about 20000/s
SDL2: 260000/s
SDL1: 520000/s
DOS in DOSBox: 35000/s
VT,  Linux framebuffer : 1.4 million/s
   (getch( ) code for those two is basically the same)
X11: 360000/s
ncurses : 1.6 million/s

   From old habit,  I avoid calling getch( ) at every pass through a
tight loop.  In the following,  I might say "if( i % (freq / 4) == 1),
do a getch()",  i.e.,  check for key input only about four times a
second.  But a fast getch( ) is still a good idea; most people just
assume it'll return almost instantly.

   NOTE that the above timings predate some work to speed up getch( )
_immensely_ on most platforms.  See pdcurses/getch.c.
If PDC_check_key( ) returns FALSE,  we assume it will continue to do
so for the current millisecond.  Thus,  we only call PDC_check_key( )
a thousand times a second.  If PDC_check_key( ) is slow,  and time
checking is fast,  this results in a _huge_ speedup.  This raised
the SDLs to about 20 million/s,  X11 to 11 million/s,  and WinGUI
to about 6.7 million/s.  DOS and DOSVGA are the odd systems out;  their
time checks are about as fast as key checks.  (Exact results may vary
with compilers and,  of course,  machines.)  */

PDCEX long PDC_millisecs( void);       /* undocumented function;  see getch.c */

int main( const int argc, const char **argv)
{
    long i, freq = 8, prev_t, t0;

    initscr( );
    timeout( argc > 1 ? atoi( argv[1]) : 0);
    mvprintw( 1, 0, "Test of raw getch( ) speed");
    mvprintw( 2, 0, "Let run a few seconds until calls/second stabilizes;");
    mvprintw( 3, 0, "then hit any key to exit");
    t0 = prev_t = PDC_millisecs( );
    for( i = 0; getch() == ERR; ++i) {
        if( i % freq == 1)   {
           const long t = PDC_millisecs( );

           if( t - prev_t > 1000)   /* update once a second */
           {
               mvprintw( 4, 0, "  %.0f calls/sec  ",
                          (double)i * 1000. / (double)( t - t0));
               prev_t = t;
               freq /= 2;
           }
           else
              freq += freq;
           refresh();
        }
    }
    endwin();
    puts("Done.");
    return( 0);
}
