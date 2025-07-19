#include <conio.h>

/* Both the curses library and the Digital Mars runtime library have
functions called int getch( void).   With the following,  code in pdckbd.c
can call dmc_getch() and get the DMC library version.  It is used _only_
with Digital Mars; other compilers don't have the name collision.  */

int dmc_getch( )
{
   return( _getch( ));
}
