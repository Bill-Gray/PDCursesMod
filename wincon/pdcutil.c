/* PDCursesMod */

#include "pdcwin.h"
#include "../common/beep.c"

void PDC_napms(int ms)     /* 'ms' = milli,  _not_ microseconds! */
{
    long t = PDC_millisecs( );
    const long end_t = t + ms;
    int remains;

    do
    {
        remains = (int)( end_t - t);

        if( SP->n_beeps_queued && (int)( t - SP->t_next_beep) > 0)
        {
            SP->n_beeps_queued--;
            SP->t_next_beep = t + beep_interval;
            if( SP->n_beeps_queued > 0)
               _raw_beep( );
        }
        if ((SP->termattrs & A_BLINK) && (GetTickCount() >= pdc_last_blink + 500))
            PDC_blink_text();
        if( remains > 0)
        {
           Sleep( remains > 50 ? 50 : remains);
           t = PDC_millisecs( );
        }
    } while( remains > 0);
}

const char *PDC_sysname(void)
{
    return "Windows";
}

enum PDC_port PDC_port_val = PDC_PORT_WINCON;
