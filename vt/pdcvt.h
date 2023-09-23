   /* If the following is TRUE,  only a limited subset of control codes
    will actually work.  Happens in older Windows, DOS, Linux console. */
extern int PDC_is_ansi;

#if defined( PDC_FORCE_UTF8) && !defined( PDC_WIDE)
   #define PDC_WIDE
#endif

#ifdef PDC_WIDE
   #if !defined( UNICODE)
      # define UNICODE
   #endif
   #if !defined( _UNICODE)
      # define _UNICODE
   #endif
#endif

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE 1   /* kill nonsense warnings */
#endif

#define CSI "\x1b["
#define OSC "\x1b]"

void PDC_puts_to_stdout( const char *buff);        /* pdcdisp.c */
