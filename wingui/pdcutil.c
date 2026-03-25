/* Public Domain Curses */

#include "pdcwin.h"
#ifdef WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#endif
#if defined( __has_include)
   #if( __has_include(<versionhelpers.h>))
      #define USE_VERSION_HELPERS
      #include <versionhelpers.h>
   #endif
#endif

#ifndef USE_VERSION_HELPERS
            /* If we lack version helpers,  assume old Windows */
   #define IsWindows7OrGreater( )         0
#endif

static void _raw_beep( void)
{
    flash( );
    if( IsWindows7OrGreater( )
                   || !PlaySound((LPCTSTR) SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ALIAS_ID | SND_ASYNC))
        Beep(400, 200);
}

void PDC_beep(void)
{
    if( !SP->n_beeps_queued)
       {
       const long beep_interval = 700;

       _raw_beep( );
       SP->t_next_beep = PDC_millisecs( ) + beep_interval;
       }
    SP->n_beeps_queued++;
}

void PDC_check_for_blinking( void);

void PDC_napms(int ms)     /* 'ms' = milli,  _not_ microseconds! */
{
    /* RR: keep GUI window responsive while PDCurses sleeps */
    MSG msg;
    DWORD curr_ms = GetTickCount( );
    const DWORD milliseconds_sleep_limit = ms + curr_ms;
    extern bool PDC_bDone;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    /* Pump all pending messages from WIN32 to the window handler */
    PDC_check_for_blinking( );
    while( !PDC_bDone && curr_ms < milliseconds_sleep_limit )
    {
        const DWORD max_sleep_ms = 50;      /* check msgs 20 times/second */
        DWORD sleep_millisecs;

        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
           TranslateMessage(&msg);
           DispatchMessage(&msg);
        }
        curr_ms = GetTickCount( );
        sleep_millisecs = milliseconds_sleep_limit - curr_ms;
        if( sleep_millisecs > max_sleep_ms)
            sleep_millisecs = max_sleep_ms;
        Sleep( sleep_millisecs);
        curr_ms += sleep_millisecs;
        PDC_check_for_blinking( );
    }
}

const char *PDC_sysname(void)
{
   return "WinGUI";
}

enum PDC_port PDC_port_val = PDC_PORT_WINGUI;
