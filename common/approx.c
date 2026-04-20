/* 'dosutil.c' has a couple of non-obvious lines of code that
convert milliseconds to clock ticks :

    ticks_to_wait = ms / 55;
    ticks_to_wait += (ticks_to_wait * 1465 + (ms % 55) * 19663 + 540000) / 1080000;

   The above ensures that the conversion is mathematically exact
and works with 32-bit integers within the range we need.  Here's
the reasoning :

   The DOS clock counts BIOS time 'ticks',  of which there are
1573040 in a full day,  or about 18.20648 per second.  Thus,  a
'tick' is about 54.92549 milliseconds.  The DOS napms() function
has to convert the supplied number of milliseconds to a number of
ticks, then nap for that number of ticks.

   PDCurses does this with a simple

(1) n_ticks = DIVROUND( ms / 50);

   Since we've established that a tick is slightly under 55
milliseconds,  not 50,  (1) gets you naps that are about 10%
too short.  This is an unnecessary error;  simply revising the
above to

(2) n_ticks = DIVROUND( ms / 55);

   would be good to 0.135%,  sufficient for most purposes.  A
mathematically exact solution,  rounded to the nearest tick,
would say that

(3) n_ticks = floor( ms * 1573040 / 86400000 + 0.5);

   (as noted above,  there are 1573040 clock ticks = 86400000
milliseconds in a day). Reducing the fraction by their common
factor of 80,

(4) n_ticks = floor( ms * 19663 / 1080000 + 0.5);

   (there are 19663 ticks = 1080000 milliseconds in 18 minutes,  or
1/80 day).  We'd like to avoid reliance on floating-point math;  in
integer arithmetic,  we can instead do

(5) n_ticks = (ms * 19663 + 540000) / 1080000;

   If asked to nap for more than 18 minutes,  we just take
18-minute naps (of exactly 19663 ticks each) and then nap for a
remaining 0 <= ms < 1080000.  This also helps simplify the logic
required when the clock rolls back to zero at midnight.

   But we are left with the problem that the multiply in (5) will
overflow the bounds of a 32-bit integer after about 109 seconds.
(DOS compilers are usually unable to handle 64-bit integers.)
For some time,  I got around that by approximating the above
expression as

(6) n_ticks = (ms * 859 + 23590) / 47181;

   This uses the fact that 859 / 47181 = 19963 / 1080000 to
within four parts in a billion,  and that 859 * 1079999 won't
overflow a 32-bit signed integer.  Computer clocks tend to be
inaccurate anyway,  so the above is more than good enough for
real world use.

   However,  I eventually realized we could do the following
to get a mathematically exact result that would not overflow :

(7) n_ticks = (ms * 19663 + 540000) / 1080000
   = (((ms / 55) * 55 + ms % 55) * 19663 + 540000) / 1080000
   = ((ms / 55) * 55 * 19663 + (ms % 55) * 19663 + 540000) / 1080000
   = ms / 55 + ((ms / 55) * 1465 + (ms % 55) * 19663 + 540000) / 1080000

   In the last line,  we note that 55 * 19963 = 1080000 + 1465.  You
could think of this as saying that we get a full tick every 55
milliseconds,  plus a small fraction (1465/1080000) of another tick.

   This would still overflow after about 22 hours.  But as mentioned
above,  we're breaking things into 18-minute chunks anyway.   (The
following code tests cases out to just past the range where some
errors occur,  to demonstrate the point.)  If one really wanted to get
around this,  one can note that

   1483 milliseconds is slightly more than 27 ticks
         (1483*19663 = 27*1080000 + 229)

   which,  with the help of the substitution n1 = ms / 1483, n2 = ms % 1483,
   ms = (n1 * 1483 + n2),  leads to

(8) n_ticks = (ms * 19663 + 540000) / 1080000
      = ((n1 * 1483 + n2) * 19663 + 540000) / 1080000
      = (n1 * 1483 * 19663 + n2 * 19663 + 540000) / 1080000
      = (n1 * (27 * 1080000 + 229) + n2 * 19663 + 540000) / 1080000
      = 27 * n1 + (n1 * 229 + n2 * 19663 + 540000) / 1080000

   This works for all positive integral 32-bit ms,  with no overflows.

   Compiles with

cc -Wall -Wextra -pedantic -o approx approx.c         */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main( const int argc, const char **argv)
{
   int n_misses = 0;
   int32_t ms;

   for( ms = 0; ms >= 0 && n_misses < 50; ms++)
      {
      const int32_t t2 = (int32_t)(((int64_t)ms * 19663L + 540000L) / 1080000L);
      int32_t ticks_to_wait;

      if( 2 == argc)
         switch( argv[1][0])
            {
            case '1':          /* good approximation,  but some off-by-ones */
               ticks_to_wait = (ms * 859 * 2 + 47181) / 94362;
               break;
            case '2':         /* exact for all 32-bit integers */
               {
               const int32_t n1 = ms / 1483;

               ticks_to_wait = 27 * n1 +
                               (n1 * 229 + (ms % 1483) * 19663 + 540000) / 1080000;
               }
               break;
            case '3':          /* exact for all 32-bit integers */
               {
               const int32_t n1 = ms / 47181;

               ticks_to_wait = 859 * n1 +
                               (n1 * 3 + (ms % 47181) * 19663 + 540000) / 1080000;
               }
               break;
            default:          /* exact for ms < 1935523014 = ~22.402 days... */
               {              /* not quite all signed 32-bit integers */
               const int32_t n1 = ms / 769;

               ticks_to_wait = 14 * n1 +
                               (n1 * 847 + (ms % 769) * 19663 + 540000) / 1080000;
               }
               break;
            }
      else
         {                 /* fails after 22.38 hours */
         ticks_to_wait = ms / 55;
         ticks_to_wait += (ticks_to_wait * 1465 + (ms % 55) * 19663 + 540000) / 1080000;
         }
      if( t2 != ticks_to_wait)
         {
         printf( "%6ld.%03ld sec   %7ld %7ld %ld\n",
                   (long)ms / 1000L, (long)ms % 1000L,
                   (long)t2,
                   (long)ticks_to_wait,
                   (long)( t2 - ticks_to_wait));
         n_misses++;
         }
      }
   printf( "%d misses\n", n_misses);
   return( 0);
}
