#include <stdlib.h>
#include <string.h>
#include <curses.h>

/* Example to test whether Ctrl-C actually stops the program.  It should
do so when noraw( ) is on,  and shouldn't when raw( ) is on.  Compile with

gcc -Wall -Wextra -pedantic -o brk_test brk_test.c -lncurses
gcc -Wall -Wextra -pedantic -I.. -o brk_test brk_test.c libpdcurses.a
*/

int main( void)
{
    int c;

    initscr();
    cbreak( );
    noecho( );
    clear( );
    echo( );
    keypad( stdscr, 1);

    mvprintw( 1, 2, "%s - test of Ctrl-C stopping curses", longname( ));
    mvaddstr( 3, 2, "raw( ) is on.  Hit Ctrl-C and the code won't break.");
    mvaddstr( 4, 2, "Hit Enter when you're done :");
    move( 5, 2);
    raw( );
    c = 0;
    while( c != 13 && c != 10)
      c = getch( );

    mvaddstr( 6, 2, "noraw( ) is on.  Hit Ctrl-C and the code will now break.");
    mvaddstr( 7, 2, "Hit Enter when you're done :");
    move( 8, 2);
    noraw( );
    c = 0;
    while( c != 13 && c != 10)
      c = getch( );

    endwin( );
    return( 0);
}
