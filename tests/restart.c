/* In theory,  you should be able to call endwin( ) and delscreen( ),
shut down Curses completely,  do "traditional" stdio input/output,
and then restart Curses again and repeat the process.  This is
possible with ncurses and most (not all) PDCursesMod platforms,  and
on a few (not many) PDCurses platforms.  */

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
        {
           addch( '.');
 //        refresh( );
        }
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
