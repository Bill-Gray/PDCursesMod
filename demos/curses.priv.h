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
#define NCURSES_SP_DCLx

#define StdScreen(sp)  stdscr
#define UChar(c)   ((unsigned char)(c))
#define ChCharOf(c)  ((chtype)(c) & (chtype)A_CHARTEXT)
#define CharOf(c) ChCharOf(c)
#define BLANK     ' '
#define ZEROS     '\0'

