/* Underlying _raw_beep( ) function for Windows and *nix (see dosutil.c for
the DOS version).  The Windows version is used in WinCon and WinGUI.  The
*nix one is used for the Linux framebuffer,  DRM,  and x11new ports.  Both
versions are used for the SDLn,  GL,  and VT ports.

   On Windows,  if we're on Win8 or later,  we can call Beep( 400, 200);
and get a simple 400 Hz sine wave for a fifth of a second.  On earlier forks,
that'll go to the piezo speaker,  which probably won't actually exist.  So we
use PlaySound( ) and get a more unpleasant noise.     */

#ifdef _WIN32
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
#ifndef __WATCOMC__
    if( IsWindows7OrGreater( )
                   || !PlaySound((LPCTSTR) SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ALIAS_ID | SND_ASYNC))
        Beep(400, 200);
#endif
}
#endif

/* On *nix,  'play' will probably be an installed command,  so we can fork( ) and
have the child process call it.  (Without the forking,  we'd hang while the sound
played.)  If the attempt to call 'play' fails,  we flash the screen. */

#ifndef _WIN32
#include <unistd.h>
#include <stdlib.h>

static void _raw_beep( void)
{
   if( !fork( ))        /* we're the child;  beep at 400 Hz for 0.2 second */
      {
      execlp( "play", "play", "-q", "-V0", "-n", "synth", "0.2", "sin", "400",
                      (char *)NULL);
                      /* if we get here,  play/SoX must not be installed */
      flash( );       /* so we'll just flash the screen instead */
      exit( 0);
      }
}
#endif

static const long beep_interval = 700;

void PDC_beep(void)
{
    if( !SP->n_beeps_queued)
       {

       _raw_beep( );
       SP->t_next_beep = PDC_millisecs( ) + beep_interval;
       }
    SP->n_beeps_queued++;
}
