#include "pdcmac.h"

void PDC_beep(void)
{
    PDC_LOG(("PDC_beep() - called\n"));
    SysBeep(1);
}

void PDC_napms(int ms)
{
    unsigned long start;
    unsigned long waitticks;
    unsigned long fiftieths;
    unsigned long remainder;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));
    if (ms <= 0)
        return;
    /* milliseconds->TickCount() ticks calculation
        to prevent overflow and be reasonably performant */
    fiftieths = (unsigned long)ms / 50UL;
    remainder = ((unsigned long)ms 
        - ((fiftieths << 5) + (fiftieths << 4) + (fiftieths << 1))) * 3UL;
    waitticks = fiftieths * 3UL;
    if (remainder > 100UL)
        waitticks += 3UL;
    else if (remainder > 50UL)
        waitticks += 2UL;
    else if (remainder)
        waitticks += 1UL;
    start = (unsigned long)TickCount();
    PDC_check_for_blinking();
    while ((unsigned long)TickCount() - start < waitticks)
    { /* PDC_mac_process_events() calls WaitNextEvent() which will yield to
            other threads and avoid spinning the CPU */
        PDC_mac_process_events(1);
        PDC_check_for_blinking();
    }
}

const char *PDC_sysname(void)
{
    return "Mac";
}

enum PDC_port PDC_port_val = PDC_PORT_MAC;
