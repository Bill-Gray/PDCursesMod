/* PDCurses */

#include "pdcgl.h"

void PDC_beep(void)
{
    PDC_LOG(("PDC_beep() - called\n"));
}

void PDC_check_for_blinking( void);

void PDC_napms(int ms)
{
    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    while (ms > 50)
    {
        PDC_pump_and_peep();
        SDL_Delay(50);
        ms -= 50;
        PDC_check_for_blinking( );
        PDC_doupdate();
    }
    PDC_pump_and_peep();
    SDL_Delay(ms);
    PDC_check_for_blinking( );
    PDC_doupdate();
}

const char *PDC_sysname(void)
{
    return "OpenGL";
}

enum PDC_port PDC_port_val = PDC_PORT_OPENGL;
