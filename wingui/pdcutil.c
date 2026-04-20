/* Public Domain Curses */

#include "pdcwin.h"
#include "../common/beep.c"

void PDC_check_for_blinking( void);

void PDC_napms(int ms)     /* 'ms' = milli,  _not_ microseconds! */
{
    /* RR: keep GUI window responsive while PDCurses sleeps */
    MSG msg;
    DWORD curr_ms = PDC_millisecs( );
    const DWORD end_t = curr_ms + ms;
    extern bool PDC_bDone;
    int remains;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    /* Pump all pending messages from WIN32 to the window handler */
    do
    {
        PDC_check_for_blinking( );
        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
           TranslateMessage(&msg);
           DispatchMessage(&msg);
        }
        remains = (int)( end_t - curr_ms);
        if( remains > 0)
        {
            const int max_sleep_ms = 50;      /* check msgs 20 times/second */

            Sleep( remains > max_sleep_ms ? max_sleep_ms : remains);
            curr_ms = PDC_millisecs( );
        }
    } while( !PDC_bDone && remains > 0);
}

const char *PDC_sysname(void)
{
   return "WinGUI";
}

enum PDC_port PDC_port_val = PDC_PORT_WINGUI;
