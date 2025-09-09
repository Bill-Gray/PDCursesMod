/* 'dosutil.c' has a couple of non-obvious lines of code that
convert milliseconds to clock ticks :

    ticks_to_wait = ms / 55;
    ticks_to_wait += (ticks_to_wait * 1465 + (ms % 55) * 19663 + 540000) / 1080000;

   The above ensures that the conversion is mathematically exact
and works with 32-bit integers.  Here's the reasoning :

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

   (there are 86400 seconds in a day).  Reducing the fraction,

(4) n_ticks = floor( ms * 19663 / 1080000 + 0.5);

   We'd like to avoid reliance on floating-point math;  in
integer arithmetic,  we can instead do

(5) n_ticks = (ms * 19663 + 540000) / 1080000;

   We also can (and do) take advantage of the fact that 1080000
milliseconds = 18 minutes = 19663 ticks,  exactly.  So if
asked to nap for more than 18 minutes,  we just take 18-minute
naps (of exactly 19663 ticks each) and then nap for a remaining
0 <= ms < 1080000.  This also gets around some problems with
the clock rolling back to zero at midnight.

   But we are left with the problem that the multiply in (5) will
overflow the bounds of a 32-bit integer after about 109 seconds.
(DOS compilers are usually unable to handle 64-bit integers).
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
errors occur,  to demonstrate the point.)       */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main( void)
{
   int n_misses = 0;
   int32_t ms;

   for( ms = 0; ms < 80562400; ms++)
      {
/*    const int32_t ticks_to_wait = (ms * 859 + 23091) / 47181; */
      const int32_t t2 = (int32_t)(((int64_t)ms * 19663L + 540000L) / 1080000L);
      int32_t ticks_to_wait = ms / 55;

      ticks_to_wait += (ticks_to_wait * 1465 + (ms % 55) * 19663 + 540000) / 1080000;

      if( t2 != ticks_to_wait)
         {
         printf( "%7ld %7ld %7ld %ld\n", (long)ms, (long)t2, (long)ticks_to_wait, (long)( t2 - ticks_to_wait));
         n_misses++;
         }
      }
   printf( "%d misses\n", n_misses);
   return( 0);
}
