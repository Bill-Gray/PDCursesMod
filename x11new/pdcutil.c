#include <unistd.h>
#include "curspriv.h"
#include "pdcx11.h"

void PDC_check_for_blinking( void);

static void _raw_beep( void)
{
   if( !fork( ))        /* we're the child;  beep at 300 Hz for 0.1 second */
      {
      execlp( "play", "play", "-q", "-V0", "-n", "synth", "0.1", "sin", "300",
                      (char *)NULL);
                      /* if we get here,  play/SoX must not be installed */
      flash( );       /* so we'll just flash the screen instead */
      exit( );
      }
}

void PDC_beep(void)
{
    if( !SP->n_beeps_queued)
       {
       const long beep_interval = 400;

       _raw_beep( );
       SP->t_next_beep = PDC_millisecs( ) + beep_interval;
       }
    SP->n_beeps_queued++;
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
