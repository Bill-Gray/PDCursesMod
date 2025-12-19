#include <curses.h>
#include <stdio.h>
#include <stdlib.h>

/* Quick,  somewhat dirty test code for the weirdnesses that occur
in the DOS PDC_napms() code.  For example,  naps have to be broken
into 18-minute segments,  midnight crossing requires special handling,
and some odd math has to be done to get timing as exact as possible
while avoiding integer overflows.  Compiles (in Digital Mars C/C++) as

dmc -I.. -msd -DCHTYPE_32 -o+space naptest.exe pdcurses.lib naptest.c

   (also tested with OpenWATCOM).

   Note that a 'special' PDC_napmsl() function,  taking a long integer
for an argument,  is used.  Otherwise,  we couldn't do any testing
past 32.767 seconds.  We also borrow PDCursesMod's internal millisecond
counter.  */

void PDC_napmsl( long ms);
PDCEX long    PDC_millisecs( void);

int main( const int argc, const char **argv)
{
   long nap_time, t1, t2;

   if( argc != 2)
      {
      fprintf( stderr, "naptest needs a nap time,  in milliseconds,  on the command line\n");
      return( -1);
      }
   nap_time = atol( argv[1]);
   printf( "Napping %ld milliseconds\n", nap_time);
   t1 = PDC_millisecs( );
   printf( "Start millisecond count : %ld\n", t1);
   PDC_napmsl( nap_time);
   t2 = PDC_millisecs( );
   printf( "Final millisecond count : %ld\n", t2);
   printf( "Elapsed time: %ld ms\n", t2 - t1);
   printf( "Hit enter:");
   getchar( );
   return( 0);
}
