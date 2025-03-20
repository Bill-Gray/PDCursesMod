#include <unistd.h>
#include "curspriv.h"

void PDC_check_for_blinking( void);

void PDC_beep(void)
{
}

void PDC_napms(int ms)
{
    PDC_check_for_blinking( );
    while( ms > 0)
    {
        const int ms_to_nap = (ms > 50 ? 50 : ms);

        usleep( 1000 * ms_to_nap);
        ms -= ms_to_nap;
        PDC_check_for_blinking( );
    }
}


const char *PDC_sysname(void)
{
   return( "X11 (new)");
}

enum PDC_port PDC_port_val = PDC_PORT_X11;
