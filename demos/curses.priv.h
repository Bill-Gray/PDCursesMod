#include <curses.h>

#define NCURSES_SP_NAME(name) name
#define NCURSES_API
#define T(a)
#define returnAttr(code)   return code
#define returnBool(code)   return code
#define returnCode(code)   return code
#define returnCPtr(code)   return code
#define returnPtr(code)    return code
#define returnVoidPtr(code)   return code
#define returnWin(code)    return code
#define CURRENT_SCREEN SP
#define NCURSES_INLINE inline
#define Min(a,b)  ((a) > (b)  ?  (b)  :  (a))
#define TR_FUNC_BFR(max)
#define TR(n, a)
#define NCURSES_SP_DCLx

#define StdScreen(sp)  stdscr
#define UChar(c)   ((unsigned char)(c))
#define ChCharOf(c)  ((chtype)(c) & (chtype)A_CHARTEXT)
#define CharOf(c) ChCharOf(c)
#define BLANK     ' '
#define ZEROS     '\0'
#define _nc_SPRINTF              snprintf
#define _nc_SLIMIT(n)          ((size_t)(n)),
#define FreeIfNeeded(p)  if ((p) != 0) free(p)

#ifdef PDC_WIDE
   #define ISBLANK(ch)  ((ch).chars[0] == L' ' && (ch).chars[1] == L'\0')
#else
   #define ISBLANK(ch)  (CharOf(ch) == ' ')
#endif

#define IsValidScreen(sp)           true
unsigned addch_used;       /* number of 'pending' bytes for a multi-byte character */
