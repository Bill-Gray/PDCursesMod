/* In theory,  you should be able to call endwin( ) and delscreen( ),
shut down Curses completely,  do "traditional" stdio input/output,
and then restart Curses again and repeat the process.  This is
possible with ncurses and most (not all) PDCursesMod platforms,  and
on a few (not many) PDCurses platforms.  Specifically,  it works on
the DOS, DOSVGA,  framebuffer/DRM,  VT,  X11,  and SDL1/2 platforms
in PDCursesMod, and not with Windows console or WinGUI.  (Should be
possible to make those happen,  though,  I'd think.)  I haven't
tried OS/2 or Plan9.

  Compile with either

gcc -Wall -Wextra -pedantic -o restart restart.c -lncurses
gcc -Wall -Wextra -pedantic -o restart restart.c -DPDC_FORCE_UTF8 -I.. libpdcurses.a
*/

#include <curses.h>
#include <stdlib.h>

int main( void)
{
    int i;

    for( i = 0; i < 2; i++)
    {
        SCREEN *sp = newterm( NULL, stdout, stdin);

        mvprintw( 1, 1, "Curses has been %sstarted.  %s",
               (i ? "re" : ""), longname( ));
        mvprintw( 2, 1, "Hit a key...");
        while( getch( ) == KEY_RESIZE)
           addch( '.');
        endwin( );
        delscreen( sp);
        if( !i)
        {
            printf( "Curses has been temporarily closed.  Hit Enter,\n"
                    "and it should restart.\n");
            getchar( );
        }
        else
            printf( "And we're done.\n");
    }
    return(0);
}
