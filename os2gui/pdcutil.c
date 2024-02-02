/* Public Domain Curses */

#include "pdcwin.h"

void PDC_beep(void)
{
    PDC_LOG(("PDC_beep() - called\n"));
    DosBeep(440, 500);
}

void PDC_napms(int ms)     /* 'ms' = milli,  _not_ microseconds! */
{
    /* RR: keep GUI window responsive while PDCurses sleeps */
    HAB hab = WinQueryAnchorBlock( PDC_hWnd);
    QMSG msg;
    ULONG curr_ms = WinGetCurrentTime(hab);
    const ULONG milliseconds_sleep_limit = curr_ms + ms;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    /* Pump all pending messages from WIN32 to the window handler */
    while( !PDC_bDone && curr_ms < milliseconds_sleep_limit )
    {
        const ULONG max_sleep_ms = 50;      /* check msgs 20 times/second */
        ULONG sleep_millisecs;

        while( WinPeekMsg(hab, &msg, NULLHANDLE, 0, 0, PM_REMOVE) )
           WinDispatchMsg(hab, &msg);
        curr_ms = WinGetCurrentTime(hab);
        sleep_millisecs = milliseconds_sleep_limit - curr_ms;
        if( sleep_millisecs > max_sleep_ms)
            sleep_millisecs = max_sleep_ms;
        DosSleep( sleep_millisecs);
    }
}

const char *PDC_sysname(void)
{
   return "OS2GUI";
}

enum PDC_port PDC_port_val = PDC_PORT_WINGUI;
