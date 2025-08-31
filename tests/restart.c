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
#ifdef _WIN32
   #undef MOUSE_MOVED
   #include <windows.h>
#endif

int main( void)
{
    int c = 0;
    SCREEN *sp = newterm( NULL, stdout, stdin);

    cbreak( );
    keypad( stdscr, 1);
    while( c != 'q')
    {
        mvaddstr( 1, 1, "Restart test on ");
        addstr( longname( ));
        mvaddstr( 2, 1, "Hit 'q' to quit,  'r' to shut down Curses/restart.");
        mvaddstr( 3, 1, "See PDCursesMod/tests/restart.c for explanations.");
        c = getch( );
        mvaddstr( 4, 1, "Key hit : ");
        addstr( keyname( c));
        clrtoeol( );
        if( c == 'r')
        {
            endwin( );
            delscreen( sp);
            printf( "Curses has been temporarily closed.  Hit Enter,\n"
                    "and it should restart.\n");
#ifdef _WIN32
            Sleep( 1000);
#endif
            getchar( );
            sp = newterm( NULL, stdout, stdin);
            cbreak( );
            keypad( stdscr, 1);
        }
    }
    endwin( );
    delscreen( sp);
    printf( "Exited 'restart' test\n");
    return(0);
}
