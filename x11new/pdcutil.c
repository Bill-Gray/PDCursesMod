#include <unistd.h>
#include <stdlib.h>
#include "curspriv.h"
#include "pdcx11.h"
#include "../common/beep.c"

void PDC_check_for_blinking( void);

void PDC_napms( const int ms)     /* 'ms' = milli,  _not_ microseconds! */
{
    long curr_ms = PDC_millisecs( );
    const long end_t = curr_ms + ms;
    int remains;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    do
    {
        remains = (int)( end_t - curr_ms);
        PDC_check_for_blinking( );
        if( remains > 0)
        {
            const int max_sleep_ms = 50;      /* check msgs 20 times/second */

            if( remains > max_sleep_ms)
                remains = max_sleep_ms;
            usleep( remains * 1000);
            curr_ms = PDC_millisecs( );
        }
    } while( remains > 0);
}

const char *PDC_sysname(void)
{
   return( "X11 (new)");
}

enum PDC_port PDC_port_val = PDC_PORT_X11;
