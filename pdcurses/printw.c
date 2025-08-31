/* PDCurses */

#include <curspriv.h>
#include <assert.h>
#include <stdlib.h>

/*man-start**************************************************************

printw
------

### Synopsis

    int printw(const char *fmt, ...);
    int wprintw(WINDOW *win, const char *fmt, ...);
    int mvprintw(int y, int x, const char *fmt, ...);
    int mvwprintw(WINDOW *win, int y, int x, const char *fmt,...);
    int vwprintw(WINDOW *win, const char *fmt, va_list varglist);
    int vw_printw(WINDOW *win, const char *fmt, va_list varglist);

### Description

   The printw() functions add a formatted string to the window at the
   current or specified cursor position. The format strings are the same
   as used in the standard C library's printf(). (printw() can be used
   as a drop-in replacement for printf().)

   The duplication between vwprintw() and vw_printw() is for historic
   reasons. In PDCurses, they're the same.

### Return Value

   All functions return the number of characters printed, or ERR on
   error.  Note that this is nonstandard;  other implementations return
   OK upon successful completion.

### Portability
   Function              | X/Open | ncurses | NetBSD
   :---------------------|:------:|:-------:|:------:
   printw                |    Y   |    Y    |   Y
   wprintw               |    Y   |    Y    |   Y
   mvprintw              |    Y   |    Y    |   Y
   mvwprintw             |    Y   |    Y    |   Y
   vwprintw              |    Y   |    Y    |   Y
   vw_printw             |    Y   |    Y    |   Y

**man-end****************************************************************/

#include <string.h>

/* All Windows compilers (MSVC,  OpenWATCOM,  Borland,  Digital Mars)
appear to have a _vsnprint() function.  Some others have vsnprintf().
A few have neither.   */

#ifdef _WIN32
   #define vsnprint_func _vsnprintf
#endif
#if defined( HAVE_VSNPRINTF) && !defined( _WIN32)
   #define vsnprint_func vsnprintf
#elif defined( __DMC__) && !defined( _WIN32)
   #define vsnprint_func _vsnprintf
#endif


/* _vsnprintf() and earlier vsnprintf() return -1 if the output doesn't
fit in the buffer.  When that happens,  we try again with a
larger buffer, doubling its size until it fits.  C99-compliant
vsnprintf() returns the number of bytes actually needed (minus the
trailing zero). */

#ifndef va_copy
   #define va_copy( dest, src) dest = src
#endif

int vwprintw(WINDOW *win, const char *fmt, va_list varglist)
{
    char printbuf[513];
    int len, rval;
#ifdef vsnprint_func
    char *buf = printbuf;
    va_list varglist_copy;
    size_t buffsize = sizeof( printbuf) - 1;

    PDC_LOG(("vwprintw() - called\n"));
    va_copy( varglist_copy, varglist);
    len = vsnprint_func( buf, buffsize, fmt, varglist_copy);
    while( len < 0 || len > (int)buffsize)
    {
        if( -1 == len)       /* Microsoft,  glibc 2.0 & earlier */
            buffsize <<= 1;
        else                 /* glibc 2.0.6 & later (C99 behavior) */
            buffsize = len + 1;
        if( buf != printbuf)
            free( buf);
        buf = (char *)malloc( buffsize + 1);
        va_copy( varglist_copy, varglist);
        len = vsnprint_func( buf, buffsize, fmt, varglist_copy);
    }
    buf[len] = '\0';
    rval = (waddstr(win, buf) == ERR) ? ERR : len;
    if( buf != printbuf)
        free( buf);
#else       /* no _vsnprintf() or vsnprintf() : buffer may overflow */
    PDC_LOG(("vwprintw() - called\n"));

    len = vsprintf(printbuf, fmt, varglist);
    assert( len < (int)sizeof( printbuf) - 1);
    rval = (waddstr(win, printbuf) == ERR) ? ERR : len;
#endif
    return rval;
}

int printw(const char *fmt, ...)
{
    va_list args;
    int retval;

    PDC_LOG(("printw() - called\n"));

    va_start(args, fmt);
    retval = vwprintw(stdscr, fmt, args);
    va_end(args);

    return retval;
}

int wprintw(WINDOW *win, const char *fmt, ...)
{
    va_list args;
    int retval;

    PDC_LOG(("wprintw() - called\n"));

    va_start(args, fmt);
    retval = vwprintw(win, fmt, args);
    va_end(args);

    return retval;
}

int mvprintw(int y, int x, const char *fmt, ...)
{
    va_list args;
    int retval;

    PDC_LOG(("mvprintw() - called\n"));

    if (move(y, x) == ERR)
        return ERR;

    va_start(args, fmt);
    retval = vwprintw(stdscr, fmt, args);
    va_end(args);

    return retval;
}

int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...)
{
    va_list args;
    int retval;

    PDC_LOG(("mvwprintw() - called\n"));

    if (wmove(win, y, x) == ERR)
        return ERR;

    va_start(args, fmt);
    retval = vwprintw(win, fmt, args);
    va_end(args);

    return retval;
}

int vw_printw(WINDOW *win, const char *fmt, va_list varglist)
{
    PDC_LOG(("vw_printw() - called\n"));

    return vwprintw(win, fmt, varglist);
}
