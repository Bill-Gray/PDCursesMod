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
two questions above.  Portable curses code should delete windows
only once,  and should never delete a window with a subwindow.
PDCursesMod will assert() if you attempt to do either of these.
More generally,  it maintains a list of "current" windows and will
assert() if you pass a pointer to delwin() that isn't actually a
valid window.   */

int main( void)
{
    WINDOW *win, *sub;
    SCREEN *screen = newterm( NULL, NULL, NULL);
    char buff[90];
    int line = 1;

    noecho();
    mvaddstr( line++, 1, "Code to test deleting windows.  References :");
    mvaddstr( line++, 1, "https://www.invisible-island.net/ncurses/man/curs_window.3x.html");
    mvprintw( line++, 1, "https://lists.gnu.org/archive/html/bug-ncurses/2022-08/msg00006.html");
    mvprintw( line++, 1, "Source code for this program (del_test.c)");
    line++;
    win = newwin( 0, 0, 0, 0);
    snprintf( buff, sizeof( buff), "Allocated new window %p    %s", (void *)win, longname( ));
    mvaddstr( line++, 1, buff);
    sub = subwin( win, 10, 10, 10, 10);
    snprintf( buff, sizeof( buff), "Sub window %p    %s", (void *)sub, curses_version( ));
    mvaddstr( line++, 1, buff);
    mvaddstr( line++, 1, "Next step will attempt to delete the window.  Should fail,  since");
    mvaddstr( line++, 1, "it has a subwindow.  Hit any key :");
    getch( );
    snprintf( buff, sizeof( buff), "Window deleted with return value %d", delwin( win));
    mvaddstr( line++, 1, buff);
    line++;
    mvaddstr( line++, 1, "Will now attempt to delete the sub-window.");
    mvaddstr( line++, 1, "That ought to succeed.  Hit any key :");
    getch( );
    snprintf( buff, sizeof( buff), "Deleted sub : %d (should succeed)", delwin( sub));
    mvaddstr( line++, 1, buff);
    line++;
    mvaddstr( line++, 1, "Will now attempt to delete the parent window.  This should succeed,");
    mvaddstr( line++, 1, "since it no longer has a sub-window.  Hit any key :");
    getch( );
    snprintf( buff, sizeof( buff), "Deleted win : %d", delwin( win));
    mvaddstr( line++, 1, buff);
    line++;
    mvaddstr( line++, 1, "Now we'll delete that window again,  which should fail.");
    mvaddstr( line++, 1, "Hit any key :");
    getch( );
    snprintf( buff, sizeof( buff), "Deleted win : %d", delwin( win));
    mvaddstr( line++, 1, buff);
    mvaddstr( line++, 1, "And we're done.  Hit any key to exit :");
    getch( );
    endwin( );
    delscreen( screen);
    return(0);
}
