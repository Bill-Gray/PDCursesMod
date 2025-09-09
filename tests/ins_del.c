#include <curses.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* Program to test the insertln()/deleteln()/wscrl() functions.
Prompted by https://github.com/wmcbrine/PDCurses/issues/165 .

gcc -Wall -Wextra -pedantic -o ins_del ins_del.c -lncursesw    */

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
    SCREEN *screen_pointer;
    WINDOW *win;
    int i, c = 0, cursor_y = 7;
    int top_scroll = 3, bottom_scroll = 10;
    int scroll_rate = 1;
    int rval;
    int reset_screen = 1;

    INTENTIONALLY_UNUSED_PARAMETER( argc);
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    screen_pointer = newterm(NULL, stdout, stdin);
    noecho( );
    raw( );
    start_color( );
    mvprintw( 0, 0, "Test of insertln(), deleteln(), wscrl()");
    mvprintw( 1, 0, "Up/down to move cursor up/down; 'i'/'d' to insert/delete line");
    mvprintw( 2, 0, "'u' to scroll up, 's' to scroll down _within marked region_");
    mvprintw( 3, 0, "Any number 0...9 to set number of lines to scroll");
    mvprintw( 4, 0, "'q' to quit         Space to reset");
    mvaddstr( 6 + top_scroll, 0, "Top scroll->");
    mvaddstr( 6 + bottom_scroll, 0, "Bottom scroll->");
    refresh( );
    win = newwin( 14, 30, 6, 16);
    init_pair( 1, COLOR_WHITE, COLOR_BLUE);
    wbkgd( win, '.' | COLOR_PAIR( 1));
    scrollok( win, TRUE);
            /* For PDCurses,  the cursor must be within the scroll region */
            /* you're setting.  For ncurses and PDCursesMod,  it doesn't. */
#if defined( __PDCURSES__) && !defined( __PDCURSESMOD__)
    wmove( win, top_scroll, 0);
#endif
    rval = wsetscrreg( win, top_scroll, bottom_scroll);
    if( OK != rval)
    {
        attr_on( A_REVERSE, NULL);
        mvaddstr( 5, 0, "wsetscrreg( ) failed");
        attr_off( A_REVERSE, NULL);
    }
    wclear( win);
    keypad( win, TRUE);
    while( c != 'q')
      {
      if( reset_screen)
         for( i = 0; i < 14; i++)
            mvwprintw( win, i, 0, "This is line %d", i);
      reset_screen = 0;
      wmove( win, cursor_y, 1);
      wsyncup( win);
      wrefresh( win);
      c = wgetch( win);
      if( c >= '0' && c <= '9')
         scroll_rate = c - '0';
      else switch( c)
         {
         case KEY_UP:
            cursor_y--;
            break;
         case KEY_DOWN:
            cursor_y++;
            break;
         case 'i':
            winsertln( win);
            break;
         case 'd':
            wdeleteln( win);
            break;
         case 's':
            wscrl( win, -scroll_rate);
            break;
         case 'u':
            wscrl( win, scroll_rate);
            break;
         case 'D':
            winsdelln( win, -scroll_rate);
            break;
         case 'I':
            winsdelln( win, scroll_rate);
            break;
         case ' ':
            reset_screen = 1;
            break;
         case 'q':
            break;
         default:
            flash( );
            beep( );
            break;
         }
      }
    endwin();
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return( 0);
}
