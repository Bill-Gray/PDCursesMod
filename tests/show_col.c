#include <curses.h>
#include <stdlib.h>
#include <locale.h>

/* Small test program to show default color table,  then to test
the ability of printw() to handle longer strings,  then to check
insstr(). */

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
    int i;
    SCREEN *screen_pointer;
    char *buff = (char *)malloc( 1001);
#ifndef WACS_S1
    const char *russian = "Here's some text that would be in Russian if you'd built";
    const char *caption = "in wide mode.  This text ought to be inserted up to here";
#else
    const char *russian = "январь февраль  март апрель май  июнь июль  август сентябрь";
    const char *caption = "^ Names of some months in Russian should be inserted here ^";

    if( !setlocale( LC_CTYPE, "C.UTF-8"))
        setlocale( LC_CTYPE, "en_US.utf8");
#endif
    INTENTIONALLY_UNUSED_PARAMETER( argc);
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    screen_pointer = newterm(NULL, stdout, stdin);
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
    color_set( 1, NULL);
    mvinsstr( 0, 10, russian);
    mvinsstr( 1, 10, caption);
    getch( );
    endwin( );
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return(0);
}
