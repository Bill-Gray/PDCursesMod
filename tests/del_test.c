#include <curses.h>
#include <stdlib.h>

/* What happens when you call delwin( ) on a window that has a
subwindow?   Or call delwin( ) twice?  (tl;dr : don't do those
things;  they will cause segfaults in many Curses variants.)

   To test,  compile with

gcc -Wall -Wextra -pedantic -o del_test del_test.c -lncurses

   On ncurses-6.2,  one gets the behavior listed below : failure,
success,  success,  failure.  If instead windows were deleted
recursively,  you'd expect a success (both windows freed) followed
by three failures (because the windows in question have both been
deleted).

   Thomas Dickey notes,  at

https://lists.gnu.org/archive/html/bug-ncurses/2022-08/msg00012.html

   that "fwiw, NetBSD curses hangs (infinite loop) on the first delwin
in [this] example,  while Solaris dumps core on the last delwin."
'Original' PDCurses deletes 'win' the first time (it can't tell that
it still has a subwindow and shouldn't be deleted),  which causes it
to crash the second time 'win' is deleted.

   We were not clear as to what is 'supposed' to be done here
according to the X/Open and SVr4 standards.  But in actual practice,
it's clearly dangerous to do either of the things described in the
two questions above.

   PDCursesMod will assert() if you delete a window with a parent.
(It maintains a list of active windows,  and will also assert() if
you attempt to 're-delete' a window,  or more generally,  pass in
a pointer to delwin() that isn't actually a current window.)   */

int main( void)
{
    WINDOW *win, *sub;
    SCREEN *screen = newterm( NULL, NULL, NULL);
    char buff[90];

    noecho();
    win = newwin( 0, 0, 0, 0);
    move( 1, 1);
    snprintf( buff, sizeof( buff), "New window %p    %s", (void *)win, longname( ));
    addstr( buff);
    move( 2, 1);
    sub = subwin( win, 10, 10, 10, 10);
    snprintf( buff, sizeof( buff), "Sub window %p    %s", (void *)sub, curses_version( ));
    mvaddstr( 2, 1, buff);
    snprintf( buff, sizeof( buff), "Deleted win : %d (should fail,  it still has a subwindow)", delwin( win));
    mvaddstr( 4, 1, buff);
    snprintf( buff, sizeof( buff), "Deleted sub : %d (should succeed)", delwin( sub));
    mvaddstr( 5, 1, buff);
    snprintf( buff, sizeof( buff), "Deleted win : %d (should succeed,  it doesn't have a subwin now)", delwin( win));
    mvaddstr( 6, 1, buff);
    snprintf( buff, sizeof( buff), "Deleted win : %d (should fail,  it'd be a re-deletion)", delwin( win));
    mvaddstr( 7, 1, buff);

    mvaddstr( 9, 1, "References :");
    mvaddstr( 10, 1, "https://www.invisible-island.net/ncurses/man/curs_window.3x.html");
    mvprintw( 11, 1, "https://lists.gnu.org/archive/html/bug-ncurses/2022-08/msg00006.html");
    mvaddstr( 12, 1, "Hit the any key:");
    getch( );
    endwin( );
    delscreen( screen);
    return(0);
}
