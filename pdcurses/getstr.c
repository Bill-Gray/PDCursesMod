/* PDCursesMod */

#include <curspriv.h>
#include <assert.h>

/*man-start**************************************************************

getstr
------

### Synopsis

    int getstr(char *str);
    int wgetstr(WINDOW *win, char *str);
    int mvgetstr(int y, int x, char *str);
    int mvwgetstr(WINDOW *win, int y, int x, char *str);
    int getnstr(char *str, int n);
    int wgetnstr(WINDOW *win, char *str, int n);
    int mvgetnstr(int y, int x, char *str, int n);
    int mvwgetnstr(WINDOW *win, int y, int x, char *str, int n);

    int get_wstr(wint_t *wstr);
    int wget_wstr(WINDOW *win, wint_t *wstr);
    int mvget_wstr(int y, int x, wint_t *wstr);
    int mvwget_wstr(WINDOW *win, int, int, wint_t *wstr);
    int getn_wstr(wint_t *wstr, int n);
    int wgetn_wstr(WINDOW *win, wint_t *wstr, int n);
    int mvgetn_wstr(int y, int x, wint_t *wstr, int n);
    int mvwgetn_wstr(WINDOW *win, int y, int x, wint_t *wstr, int n);

### Description

   These routines call wgetch() repeatedly to build a string,
   interpreting erase and kill characters along the way, until a newline
   or carriage return is received. When PDCurses is built with wide-
   character support enabled, the narrow-character functions convert the
   wgetch()'d values into a multibyte string in the current locale
   before returning it. The resulting string is placed in the area
   pointed to by *str. The routines with n as the last argument read at
   most n characters.  Note that this does not include the terminating
   '\0' character;  be sure your buffer has room for that.

   Note that there's no way to know how long the buffer passed to
   wgetstr() is, so use wgetnstr() to avoid buffer overflows.

### Return Value

   These functions return ERR on failure or any other value on success.

### Portability
   Function              | X/Open | ncurses | NetBSD
   :---------------------|:------:|:-------:|:------:
   getstr                |    Y   |    Y    |   Y
   wgetstr               |    Y   |    Y    |   Y
   mvgetstr              |    Y   |    Y    |   Y
   mvwgetstr             |    Y   |    Y    |   Y
   getnstr               |    Y   |    Y    |   Y
   wgetnstr              |    Y   |    Y    |   Y
   mvgetnstr             |    Y   |    Y    |   Y
   mvwgetnstr            |    Y   |    Y    |   Y
   get_wstr              |    Y   |    Y    |   Y
   wget_wstr             |    Y   |    Y    |   Y
   mvget_wstr            |    Y   |    Y    |   Y
   mvwget_wstr           |    Y   |    Y    |   Y
   getn_wstr             |    Y   |    Y    |   Y
   wgetn_wstr            |    Y   |    Y    |   Y
   mvgetn_wstr           |    Y   |    Y    |   Y
   mvwgetn_wstr          |    Y   |    Y    |   Y

**man-end****************************************************************/

#define MAXLINE 255

int wgetnstr(WINDOW *win, char *str, int n)
{
#ifdef PDC_WIDE
    wchar_t wstr[MAXLINE + 1];
    wint_t wintstr[MAXLINE + 1];
    int i;

    if (n < 0 || n > MAXLINE)
        n = MAXLINE;

    if (wgetn_wstr(win, wintstr, n) == ERR)
        return ERR;
    for (i = 0; i < n; ++i) {
        wstr[i] = (wchar_t)wintstr[i];
    }

    return (int)PDC_wcstombs(str, wstr, n);
#else
    int ch, i, num, x, chars;
    char *p;
    bool stop, oldecho, oldcbreak, oldnodelay;

    PDC_LOG(("wgetnstr() - called\n"));

    assert( win);
    assert( str);
    assert( SP);
    if (!win || !str)
        return ERR;

    chars = 0;
    p = str;
    stop = FALSE;

    x = win->_curx;

    oldcbreak = SP->cbreak; /* remember states */
    oldecho = SP->echo;
    oldnodelay = win->_nodelay;

    SP->echo = FALSE;       /* we do echo ourselves */
    cbreak();               /* ensure each key is returned immediately */
    win->_nodelay = FALSE;  /* don't return -1 */

    wrefresh(win);

    while (!stop)
    {
        ch = wgetch(win);

        switch (ch)
        {

        case '\t':
            ch = ' ';
            num = TABSIZE - (win->_curx - x) % TABSIZE;
            for (i = 0; i < num; i++)
            {
                if (chars < n)
                {
                    if (oldecho)
                        waddch(win, ch);
                    *p++ = (char)ch;
                    ++chars;
                }
                else
                    beep();
            }
            break;

        case _ECHAR:        /* CTRL-H -- Delete character */
            if (p > str)
            {
                if (oldecho)
                    waddstr(win, "\b \b");
                ch = (unsigned char)(*--p);
                if ((ch < ' ') && (oldecho))
                    waddstr(win, "\b \b");
                chars--;
            }
            break;

        case _DLCHAR:       /* CTRL-U -- Delete line */
            while (p > str)
            {
                if (oldecho)
                    waddstr(win, "\b \b");
                ch = (unsigned char)(*--p);
                if ((ch < ' ') && (oldecho))
                    waddstr(win, "\b \b");
            }
            chars = 0;
            break;

        case _DWCHAR:       /* CTRL-W -- Delete word */

            while ((p > str) && (*(p - 1) == ' '))
            {
                if (oldecho)
                    waddstr(win, "\b \b");

                --p;        /* remove space */
                chars--;
            }
            while ((p > str) && (*(p - 1) != ' '))
            {
                if (oldecho)
                    waddstr(win, "\b \b");

                ch = (unsigned char)(*--p);
                if ((ch < ' ') && (oldecho))
                    waddstr(win, "\b \b");
                chars--;
            }
            break;

        case '\n':
        case '\r':
            stop = TRUE;
            if (oldecho)
                waddch(win, '\n');
            break;

        default:
            if (chars < n && ch < 0x100)
            {
                *p++ = (char)ch;
                if (oldecho)
                    waddch(win, ch);
                chars++;
            }
            else
                beep();

            break;

        }

        wrefresh(win);
    }

    *p = '\0';

    SP->echo = oldecho;     /* restore old settings */
    SP->cbreak = oldcbreak;
    win->_nodelay = oldnodelay;

    return OK;
#endif
}

int getstr(char *str)
{
    PDC_LOG(("getstr() - called\n"));

    return wgetnstr(stdscr, str, MAXLINE);
}

int wgetstr(WINDOW *win, char *str)
{
    PDC_LOG(("wgetstr() - called\n"));

    return wgetnstr(win, str, MAXLINE);
}

int mvgetstr(int y, int x, char *str)
{
    PDC_LOG(("mvgetstr() - called\n"));

    if (move(y, x) == ERR)
        return ERR;

    return wgetnstr(stdscr, str, MAXLINE);
}

int mvwgetstr(WINDOW *win, int y, int x, char *str)
{
    PDC_LOG(("mvwgetstr() - called\n"));

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wgetnstr(win, str, MAXLINE);
}

int getnstr(char *str, int n)
{
    PDC_LOG(("getnstr() - called\n"));

    return wgetnstr(stdscr, str, n);
}

int mvgetnstr(int y, int x, char *str, int n)
{
    PDC_LOG(("mvgetnstr() - called\n"));

    if (move(y, x) == ERR)
        return ERR;

    return wgetnstr(stdscr, str, n);
}

int mvwgetnstr(WINDOW *win, int y, int x, char *str, int n)
{
    PDC_LOG(("mvwgetnstr() - called\n"));

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wgetnstr(win, str, n);
}

#ifdef PDC_WIDE
static void _clear_preceding_char( WINDOW *win, const int ch)
{
    waddstr(win, "\b \b");
    if( PDC_wcwidth( (int32_t)ch) == 2 || ch < ' ')
       waddstr(win, "\b \b");    /* fullwidth & ctrl chars take two columns */
}

int wgetn_wstr(WINDOW *win, wint_t *wstr, int n)
{
    int i, num, x, chars;
    wint_t *p;
    bool stop, oldecho, oldcbreak, oldnodelay;

    PDC_LOG(("wgetn_wstr() - called\n"));

    assert( win);
    assert( wstr);
    assert( SP);
    if (!win || !wstr)
        return ERR;

    chars = 0;
    p = wstr;
    stop = FALSE;

    x = win->_curx;

    oldcbreak = SP->cbreak; /* remember states */
    oldecho = SP->echo;
    oldnodelay = win->_nodelay;

    SP->echo = FALSE;       /* we do echo ourselves */
    cbreak();               /* ensure each key is returned immediately */
    win->_nodelay = FALSE;  /* don't return -1 */

    wrefresh(win);

    while (!stop)
    {
        wint_t ch;

        wget_wch( win, &ch);

        switch (ch)
        {

        case '\t':
            ch = ' ';
            num = TABSIZE - (win->_curx - x) % TABSIZE;
            for (i = 0; i < num; i++)
            {
                if (chars < n)
                {
                    if (oldecho)
                        waddch(win, ch);
                    *p++ = (wint_t)ch;
                    ++chars;
                }
                else
                    beep();
            }
            break;

        case _ECHAR:        /* CTRL-H -- Delete character */
            if (p > wstr)
            {
                ch = *--p;
                if (oldecho)
                   _clear_preceding_char( win, ch);
                chars--;
            }
            break;

        case _DLCHAR:       /* CTRL-U -- Delete line */
            while (p > wstr)
            {
                ch = *--p;
                if (oldecho)
                   _clear_preceding_char( win, ch);
            }
            chars = 0;
            break;

        case _DWCHAR:       /* CTRL-W -- Delete word */

            while ((p > wstr) && (*(p - 1) == ' '))
            {
                --p;        /* remove space */
                if (oldecho)
                   _clear_preceding_char( win, *p);
                chars--;
            }
            while ((p > wstr) && (*(p - 1) != ' '))
            {
                ch = *--p;
                if (oldecho)
                   _clear_preceding_char( win, ch);
                chars--;
            }
            break;

        case '\n':
        case '\r':
            stop = TRUE;
            if (oldecho)
                waddch(win, '\n');
            break;

        default:
            if (chars < n)
            {
                if( ch < KEY_MIN || ch >= KEY_MAX)
                {
                    *p++ = ch;
                    if (oldecho)
                        waddch(win, ch);
                    chars++;
                }
            }
            else
                beep();

            break;

        }

        wrefresh(win);
    }

    *p = '\0';

    SP->echo = oldecho;     /* restore old settings */
    SP->cbreak = oldcbreak;
    win->_nodelay = oldnodelay;

    return OK;
}

int get_wstr(wint_t *wstr)
{
    PDC_LOG(("get_wstr() - called\n"));

    return wgetn_wstr(stdscr, wstr, MAXLINE);
}

int wget_wstr(WINDOW *win, wint_t *wstr)
{
    PDC_LOG(("wget_wstr() - called\n"));

    return wgetn_wstr(win, wstr, MAXLINE);
}

int mvget_wstr(int y, int x, wint_t *wstr)
{
    PDC_LOG(("mvget_wstr() - called\n"));

    if (move(y, x) == ERR)
        return ERR;

    return wgetn_wstr(stdscr, wstr, MAXLINE);
}

int mvwget_wstr(WINDOW *win, int y, int x, wint_t *wstr)
{
    PDC_LOG(("mvwget_wstr() - called\n"));

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wgetn_wstr(win, wstr, MAXLINE);
}

int getn_wstr(wint_t *wstr, int n)
{
    PDC_LOG(("getn_wstr() - called\n"));

    return wgetn_wstr(stdscr, wstr, n);
}

int mvgetn_wstr(int y, int x, wint_t *wstr, int n)
{
    PDC_LOG(("mvgetn_wstr() - called\n"));

    if (move(y, x) == ERR)
        return ERR;

    return wgetn_wstr(stdscr, wstr, n);
}

int mvwgetn_wstr(WINDOW *win, int y, int x, wint_t *wstr, int n)
{
    PDC_LOG(("mvwgetn_wstr() - called\n"));

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wgetn_wstr(win, wstr, n);
}
#endif
