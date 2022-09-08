#if (defined(_MSC_VER) && _MSC_VER < 1900) || defined( __DMC__)

   /* Bodge to get around shortcomings of older MSVCs and of the
Digital Mars C compiler.  Some configurations lack any sort of
range-checked snprintf;  in such cases,  we may overwrite the end
of the buffer.  Some MSVCs have a 'sort of' snprintf,  but it
won't take a NULL string to determine the necessary buffer size,
and it won't put a '\0' at the end of the string if it would
overflow.  Basically,  there's no "safe" way to do an snprintf
in such situations,  but this will work as long as you don't
actually overflow the buffer. */

#include <stdarg.h>

int snprintf( char *string, const size_t max_len, const char *format, ...)
{
   va_list argptr;
   int rval;

   va_start( argptr, format);
#if _MSC_VER <= 1100 || defined( __DMC__)
   rval = vsprintf( string, format, argptr);
#else
   rval = vsnprintf( string, max_len, format, argptr);
#endif
   string[max_len - 1] = '\0';
   va_end( argptr);
   return( rval);
}
#endif
