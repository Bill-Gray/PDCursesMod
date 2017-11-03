/* Public Domain Curses */

#include <curspriv.h>
#include <term.h>

/*man-start**************************************************************
  Name:                                                         compat
  Synopsis:
        int              compat_getlines(void);
        int              compat_getcols(void);
        WINDOW           *compat_getstdscr(void);
        WINDOW           *compat_getcurscr(void);
        SCREEN           *compat_getsp(void);
        MOUSE_STATUS     compat_getmouse_status(void);
        int              compat_getcolors(void);
        int              compat_getcolor_pairs(void);
        int              compat_gettabsize(void);
        void             compat_settabsize(int size);
        chtype           *compat_getacs_map(void);
        char             *compat_getttytype(void);
        PDC_version_info compat_getpdc_version(void);
        TERMINAL         *compat_getcur_term(void);
  Description:
        compat_getlines() is used to retrieve the LINES variable.
        
        compat_getcols() is used to retrieve the COLS variable.
        
        compat_getstdscr() is used to retrieve the stdscr variable.
        
        compat_getcurscr() is used to retrieve the curscr variable.
        
        compat_getsp() is used to retrieve the SP variable.
        
        compat_getmouse_status() is used to retrieve the Mouse_Status variable.
        
        compat_getcolors() is used to retrieve the COLORS variable.
        
        compat_getcolor_pairs() is used to retrieve the COLOR_PAIRS variable.
        
        compat_gettabsize() and compat_settabsize() are used to set and retrieve the TABSIZE variable.
        
        compat_getacs_map() is used to retrieve the acs_map variable.
        
        compat_getttytype() is used to retrieve the ttytype variable.
        
        compat_getpdc_version() is used to retrieve the PDC_version variable.
        
        compat_getcur_term() is used to retrieve the current terminal.
  Portability                                X/Open    BSD    SYS V
        compat_getlines                         Y       Y       Y
        compat_getcols                          Y       Y       Y
        compat_getstdscr                        Y       Y       Y
        compat_getcurscr                        Y       Y       Y
        compat_getsp                            Y       Y       Y
        compat_getmouse_status                  Y       Y       Y
        compat_getcolors                        Y       Y       Y
        compat_getcolor_pairs                   Y       Y       Y
        compat_gettabsize                       Y       Y       Y
        compat_settabsize                       Y       Y       Y
        compat_getacs_map                       Y       Y       Y
        compat_getttytype                       Y       Y       Y
        compat_getpdc_version                   Y       Y       Y
        compat_getcur_term                      Y       Y       Y
**man-end****************************************************************/

int compat_getlines(void)
{
    return LINES;
}

int compat_getcols(void)
{
    return COLS;
}

WINDOW *compat_getstdscr(void)
{
    return stdscr;
}

WINDOW *compat_getcurscr(void)
{
    return curscr;
}

SCREEN *compat_getsp(void)
{
    return SP;
}

MOUSE_STATUS compat_getmouse_status(void)
{
    return Mouse_status;
}

int compat_getcolors(void)
{
    return COLORS;
}

int compat_getcolor_pairs(void)
{
    return COLOR_PAIRS;
}

int compat_gettabsize(void)
{
    return TABSIZE;
}

void compat_settabsize(int size)
{
    TABSIZE = size;
}

chtype *compat_getacs_map(void)
{
    return acs_map;
}

char *compat_getttytype(void)
{
    return ttytype;
}

PDC_version_info compat_getpdc_version(void)
{
    return PDC_version;
}

TERMINAL *compat_getcur_term(void)
{
    return cur_term;
}
