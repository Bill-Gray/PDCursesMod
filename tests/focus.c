#include <curses.h>

/* Test code for define_key().  At present,  this function
does nothing in PDCurses*.  It could be added to the VT and
Linux framebuffer/DRM platforms,  if that proves desirable.
More generally,  though,  I'm thinking that the ability to get
KEY_FOCUS_IN and KEY_FOCUS_OUT via getch(), on all platforms,
might be useful.  At present,  our programs have no way of
knowing if they've lost/regained focus.

   If we did such a thing,  the following bits would still be
needed for portable programs.  That is to say,  if you weren't
using PDCursesMod,  you would need to #define KEY_FOCUS_*,  make
the two calls to define_key(),  and emit the control sequence
so that those events would be sent to the program.   */

#ifdef NCURSES_VERSION
   #define KEY_FOCUS_IN    (KEY_MAX + 1)
   #define KEY_FOCUS_OUT   (KEY_MAX + 2)
#endif

int main( void)
{
   int input;
   SCREEN *screen_pointer = newterm(NULL, stdout, stdin);

   noecho();
   cbreak();
   keypad( stdscr, 1);
#ifdef NCURSES_VERSION
   define_key( "\033[I", KEY_FOCUS_IN);
   define_key( "\033[O", KEY_FOCUS_OUT);
   printf( "\033[?1004h");   /* control sequence to turn focus events on */
#endif
   addstr( longname());
   printw( "  Hit 'q' to quit.\n");
   while( (input = getch()) != 'q')
#ifdef NCURSES_VERSION
      if( input == KEY_FOCUS_IN)
         addstr( "KEY_FOCUS_IN ");
      else if( input == KEY_FOCUS_OUT)
         addstr( "KEY_FOCUS_OUT ");
      else
#endif
      {
         addstr( keyname( input));
         addch( ' ');
      }
   endwin( );
                            /* Not really needed,  but ensures Valgrind  */
   delscreen( screen_pointer);          /* says all memory was freed */
   return( 0);
}
