#include <curses.h>

/* Code to test is_cbreak(), is_echo(), is_nl(), is_raw().  These
were added to ncurses on 2023 Aug 12,  and to PDCursesMod later.
As far as I know,  they don't exist elsewhere.  Compile with :

gcc -Wall -Wextra -pedantic -o opaque opaque.c -lncurses
gcc -Wall -Wextra -pedantic -I.. -o opaque opaque.c libpdcurses.a
*/

#ifdef PDC_VERSION_PATCH
   #if PDC_VERSION_PATCH >= 20240311
      #define _HAVE_OPAQUE_SCREEN_FUNCS
   #endif
#endif

#ifdef NCURSES_VERSION_PATCH
   #if NCURSES_VERSION_PATCH >= 20230812
      #define _HAVE_OPAQUE_SCREEN_FUNCS
   #endif
#endif

int main(void)
{
   SCREEN *screen_pointer = newterm( NULL, stdout, stdin);

   intrflush(stdscr, FALSE);

#ifdef _HAVE_OPAQUE_SCREEN_FUNCS
   printw( "cbreak = %d   echo = %d   nl = %d   raw = %d\n",
         is_cbreak(), is_echo(), is_nl(), is_raw());
#else
   addstr( "is_cbreak(), is_echo(), is_nl(), is_raw() are not available\n");
#endif

   addstr("Hit a key to exit :");
   getch( );
   endwin();
                             /* Not really needed,  but ensures Valgrind  */
   delscreen( screen_pointer);          /* says all memory was freed */
   return( 0);
}
