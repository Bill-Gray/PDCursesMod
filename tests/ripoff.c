/* Code to test ripoffline( ) and the SLK functions.  Compile as

gcc -Wall -Wextra -pedantic -Werror -O2 -o ripoff ripoff.c -lncursesw

for ncurses.  For PDCursesMod (some modifications may be required) :

gcc -Wall -Wextra -pedantic -Werror -O2 -fPIC -DPDC_FORCE_UTF8 -I..  -o ripoff ripoff.c libpdcurses.a

   Run as,  for example,

./ripoff s1 b b t b

   to have a format 1 SLK (two sets of four labels),  two lines
ripped off from the bottom,  then one from the top,  then one
from the bottom.  Resizing the window should cause the SLK and
ripped-off lines to be refreshed correctly. (It now does so in
PDCursesMod.  In ncurses,  SLK is refreshed correctly and
ripped-off lines are moved,  but not resized/refreshed.  None
of this works very well in PDCurses.)         */

#include <curses.h>

int ripoff_callback( WINDOW *win, int width)
{
   char buff[100];
   int i;

   snprintf( buff, sizeof( buff), "Window %p :  %d columns",
                  (void *)win, width);
   waddstr( win, buff);
   for( i = 45; i < width - 1; i++)
      mvwaddch( win, 0, i, '0' + i % 10);
   waddch( win, '*');
   wnoutrefresh( win);
   return( 0);
}

int main( const int argc, const char **argv)
{
    int ch = 0;
    SCREEN *screen_pointer;
    int i, slk_format = 0;

    for( i = 1; i < argc; i++)
        switch( argv[i][0])
            {
            case 't':    /* rip off line from top */
            case 'b':    /* rip off line from bottom */
                if( ERR == ripoffline( (argv[i][0] == 't' ? 1 : -1), ripoff_callback))
                {
                    fprintf( stderr, "Only five lines can be ripped off\n");
                    return( -1);
                }
                break;
            case 's':    /* set SLK format   */
            case 'S':    /* set SLK format with index line : PDCursesMod extension */
                {
                sscanf( argv[i] + 1, "%x", (unsigned *)&slk_format);
                slk_init( argv[i][0] == 'S' ? -slk_format : slk_format);
                }
                break;
            default:
                fprintf( stderr, "Argument '%s' unrecognized\n", argv[i]);
                return( -1);
            }

    screen_pointer = newterm( NULL, stdout, stdin);
    cbreak( );
    keypad( stdscr, 1);
    mousemask( ALL_MOUSE_EVENTS, NULL);
    if( slk_format)
        {
        int label_no, err_code = OK;      /* set labels until we run out */
        char buff[20];

        for( label_no = 1; err_code == OK; label_no++)
            {
            snprintf( buff, sizeof( buff), "Lbl%2d", label_no);
            err_code = slk_set( label_no, buff, 1);
            }
        slk_noutrefresh( );
        }
    while( ch != 27 && ch != 'q')
        {
        int x = COLS / 2 - 15, y = LINES / 2 - 1;

        clear( );
        mvprintw( y++, x, "stdscr has %d lines, %d columns", LINES, COLS);
        mvaddstr( y++, x, "Hit Escape or q to quit");
        mvaddstr( y++, x, "Resizing should redraw SLK and ripped-off lines");
//      if( ch >= KEY_F( 1) && ch <= KEY_F( 12))
           mvprintw( y++, x, "Key %d hit", ch);
        for( i = 0; i < LINES; i++)
           mvprintw( i, 0, "Line %d", i + 1);
        printw( "   (Bottom line)");
        refresh( );
        ch = getch( );
        }
    endwin( );
                             /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return( 0);
}
