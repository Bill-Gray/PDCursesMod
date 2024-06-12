#include <curses.h>
#include <stdlib.h>

/* Small test program to show default color table,  then to test
the ability of printw() to handle longer strings */

int main( void)
{
    int i;
    SCREEN *screen_pointer = newterm(NULL, stdout, stdin);
    char *buff = (char *)malloc( 1001);

    noecho();
    start_color();
    for( i = 0; i < COLORS && i < LINES * (COLS / 7); i++)
    {
        init_pair( (short)i, 0, (short)i);
        move( i % LINES, (i / LINES) * 7);
        color_set( (short)i, NULL);
        printw( " %3d ", i);
    }
    getch( );
    color_set( 0, NULL);
    mvprintw( 0, 1, "Test of printw( ) with a 1000-byte long string\n");
    for( i = 0; i < 250; i++)
        sprintf( buff + i * 4, "%3d%s", i, (i % 19 == 18 ? "\n" : " "));
    i = mvprintw( 1, 0, "%s", buff);
    free( buff);
    printw( "\n Return value : %d (should be 1000) ", i);
    getch( );
    endwin( );
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return(0);
}
