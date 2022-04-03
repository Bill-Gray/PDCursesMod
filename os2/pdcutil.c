/* PDCurses */

#include "pdcos2.h"
#include "time.h"

#if defined(OS2) && !defined(__EMX__)
APIRET APIENTRY DosSleep(ULONG ulTime);
#endif

void PDC_beep(void)
{
    PDC_LOG(("PDC_beep() - called\n"));

    DosBeep(1380, 100);
}

static ULONG _PDC_ms_count(void)
{
    const ULONG now = (ULONG)clock( );
    const ULONG n_seconds = now / (ULONG)CLOCKS_PER_SEC;
    const ULONG fraction  = now % (ULONG)CLOCKS_PER_SEC;

    return( n_seconds * 1000UL + fraction * 1000UL / (ULONG)CLOCKS_PER_SEC);
}

void PDC_napms(int ms)
{
    static ULONG _last_blink_ms = (ULONG)0;
    const ULONG blink_interval_ms = 500;   /* blink twice a second */

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    if( SP->termattrs & A_BLINK)
    {
        const ULONG curr_ms = _PDC_ms_count( );

        if( curr_ms < _last_blink_ms || curr_ms >= _last_blink_ms + blink_interval_ms)
        {
            PDC_blink_text();
            _last_blink_ms = curr_ms;
        }
    }
    DosSleep(ms);
}

const char *PDC_sysname(void)
{
    return "OS/2";
}

enum PDC_port PDC_port_val = PDC_PORT_OS2;
