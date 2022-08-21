#ifdef _WIN32
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   #undef MOUSE_MOVED
#else
   #include <unistd.h>
#endif
#include "curspriv.h"

void PDC_beep(void)
{
}

void PDC_napms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
#ifndef DOS
#ifdef LINUX_FRAMEBUFFER_PORT
    PDC_check_for_blinking( );
    while( ms > 0)
    {
        const int ms_to_nap = (ms > 50 ? 50 : ms);

        usleep( 1000 * ms_to_nap);
        ms -= ms_to_nap;
        PDC_check_for_blinking( );
    }
#else
    usleep(1000 * ms);
#endif
#endif
#endif
}


const char *PDC_sysname(void)
{
#ifdef USE_DRM
   return( "Direct Rendering Manager");
#else
   #ifdef LINUX_FRAMEBUFFER_PORT
      return( "LinuxFB");
   #else
      return( "VTx00");
   #endif
#endif
}

#ifdef LINUX_FRAMEBUFFER_PORT
enum PDC_port PDC_port_val = PDC_PORT_LINUX_FB;
#else
enum PDC_port PDC_port_val = PDC_PORT_VT;
#endif
