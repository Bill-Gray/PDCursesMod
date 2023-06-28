/* This file is only used with the ncurses test programs.
 *
 * Have ncurses-6.4 unpacked in your $(HOME) (you don't need to build
 * it), or edit ncurses_testdir appropriately in the Makefile or
 * nctests.mif. Configure and build PDCursesMod, and:
 *
 * "make ncurses_tests" to start.
 * "make ncurses_clean" when you're done.
 *
 * Builds: bs gdc hanoi knight tclock ncurses (many others)
 */

#define PDC_NCMOUSE
#define DATA_DIR "."
#define NCURSES_CONST
#define NCURSES_EXPORT(foo) foo

#define RETSIGTYPE void
#define TIME_WITH_SYS_TIME 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TIME_SELECT 1
#define HAVE_UNISTD_H 1
#define HAVE_TERMATTRS 1

#include <curspriv.h>

#define ExitProgram exit

#define HAVE_ALLOC_PAIR 1
#define HAVE_ASSUME_DEFAULT_COLORS 1
#define HAVE_CHGAT 1
#define HAVE_COLOR_CONTENT 1
#define HAVE_COPYWIN 1
#define HAVE_CURSES_VERSION 1
#define HAVE_GETBEGX 1
#define HAVE_GETCURX 1
#define HAVE_GETMAXX 1
#define HAVE_GETNSTR 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GETWIN 1
#define HAVE_HALFDELAY 1
#define HAVE_INIT_EXTENDED_COLOR 1
#define HAVE_LIBPANEL 1
#define HAVE_LIBFORM 1
#define HAVE_LIBMENU 1
#define HAVE_LOCALE_H 1
#define HAVE_MATH_H 1
#define HAVE_MATH_FUNCS 1
#define HAVE_NAPMS 1
#define HAVE_NEWPAD 1
#define HAVE_PANEL_H 1
#define HAVE_PUTWIN 1
/* #define HAVE_SCR_DUMP 1 */
#define HAVE_SLK_COLOR 1
#define HAVE_SLK_INIT 1
#define HAVE_STRSTR 1
#define HAVE_USE_DEFAULT_COLORS 1
#define HAVE_WRESIZE 1
#define USE_STRING_HACKS 1

#ifdef PDC_WIDE
# define USE_WIDEC_SUPPORT 1
# define HAVE_WCSRTOMBS 1
# define HAVE_MBSRTOWCS 1
# define HAVE_MBRTOWC 1
# define HAVE_MBRLEN 1
#endif

/* Fool ncurses.c so it gives us all the tests, and doesn't redefine
   ACS_ chars
*/
#ifdef __GNUC__
   #define GCC_UNUSED __attribute__((unused))
   #define GCC_NORETURN __attribute__((noreturn))
#else
   #define GCC_UNUSED
   #define GCC_NORETURN
#endif

#define NCURSES_VERSION PDCURSES

#ifdef PDC_WIDE
   #define wcwidth PDC_wcwidth
   PDCEX int PDC_wcwidth( const int32_t ucs);
#endif

char *tigetstr( const char *capname);
