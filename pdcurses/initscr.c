/* PDCursesMod */

#include <curspriv.h>
#include <panel.h>
#include <assert.h>

/*man-start**************************************************************

initscr
-------

### Synopsis

    WINDOW *initscr(void);
    WINDOW *Xinitscr(int argc, char **argv);
    int endwin(void);
    bool isendwin(void);
    SCREEN *newterm(const char *type, FILE *outfd, FILE *infd);
    SCREEN *set_term(SCREEN *new);
    void delscreen(SCREEN *sp);

    int resize_term(int nlines, int ncols);
    bool is_termresized(void);
    const char *curses_version(void);
    void PDC_get_version(PDC_VERSION *ver);

    int set_tabsize(int tabsize);

### Description

   initscr() should be the first curses routine called. It will
   initialize all curses data structures, and arrange that the first
   call to refresh() will clear the screen. In case of error, initscr()
   will write a message to standard error and end the program.

   endwin() should be called before exiting or escaping from curses mode
   temporarily. It will restore tty modes, move the cursor to the lower
   left corner of the screen and reset the terminal into the proper
   non-visual mode. To resume curses after a temporary escape, call
   refresh() or doupdate().

   isendwin() returns TRUE if endwin() has been called without a
   subsequent refresh, unless SP is NULL.

   In some implementations of curses, newterm() allows the use of
   multiple terminals. Here, it's just an alternative interface for
   initscr(). It always returns SP, or NULL.

   delscreen() frees the memory allocated by newterm() or initscr(),
   since it's not freed by endwin(). This function is usually not
   needed. In PDCurses, the parameter must be the value of SP, and
   delscreen() sets SP to NULL.

   set_term() does nothing meaningful in PDCurses, but is included for
   compatibility with other curses implementations.

   resize_term() is effectively two functions: When called with nonzero
   values for nlines and ncols, it attempts to resize the screen to the
   given size. When called with (0, 0), it merely adjusts the internal
   structures to match the current size after the screen is resized by
   the user. On the currently supported platforms, SDL, Windows console,
   and X11 allow user resizing, while DOS, OS/2, SDL and Windows console
   allow programmatic resizing. If you want to support user resizing,
   you should check for getch() returning KEY_RESIZE, and/or call
   is_termresized() at appropriate times.   Then, with either user or
   programmatic resizing, you'll have to resize any windows you've
   created, as appropriate; resize_term() only handles stdscr and curscr.

   is_termresized() returns TRUE if the curses screen has been resized
   by the user, and a call to resize_term() is needed. Checking for
   KEY_RESIZE is generally preferable, unless you're not handling the
   keyboard.

   curses_version() returns a string describing the version of PDCursesMod.

   PDC_get_version() fills a PDC_VERSION structure provided by the user
   with more detailed version info (see curses.h).

   set_tabsize() sets the tab interval, stored in TABSIZE.

### Return Value

   All functions return NULL on error, except endwin(), which always
   returns OK, and resize_term(), which returns either OK or ERR.

### Portability
   Function              | X/Open | ncurses | NetBSD
   :---------------------|:------:|:-------:|:------:
   initscr               |    Y   |    Y    |   Y
   endwin                |    Y   |    Y    |   Y
   isendwin              |    Y   |    Y    |   Y
   newterm               |    Y   |    Y    |   Y
   set_term              |    Y   |    Y    |   Y
   delscreen             |    Y   |    Y    |   Y
   resize_term           |    -   |    Y    |   Y
   set_tabsize           |    -   |    Y    |   Y
   curses_version        |    -   |    Y    |   -
   is_termresized        |    -   |    -    |   -

**man-end****************************************************************/

#include <stdlib.h>

char ttytype[128];

#if PDC_VER_MONTH == 1
   #define PDC_VER_MONTH_STR "Jan"
#elif PDC_VER_MONTH == 2
   #define PDC_VER_MONTH_STR "Feb"
#elif PDC_VER_MONTH == 3
   #define PDC_VER_MONTH_STR "Mar"
#elif PDC_VER_MONTH == 4
   #define PDC_VER_MONTH_STR "Apr"
#elif PDC_VER_MONTH == 5
   #define PDC_VER_MONTH_STR "May"
#elif PDC_VER_MONTH == 6
   #define PDC_VER_MONTH_STR "Jun"
#elif PDC_VER_MONTH == 7
   #define PDC_VER_MONTH_STR "Jul"
#elif PDC_VER_MONTH == 8
   #define PDC_VER_MONTH_STR "Aug"
#elif PDC_VER_MONTH == 9
   #define PDC_VER_MONTH_STR "Sep"
#elif PDC_VER_MONTH == 10
   #define PDC_VER_MONTH_STR "Oct"
#elif PDC_VER_MONTH == 11
   #define PDC_VER_MONTH_STR "Nov"
#elif PDC_VER_MONTH == 12
   #define PDC_VER_MONTH_STR "Dec"
#else
   #define PDC_VER_MONTH_STR "!!!"
#endif

const char *_curses_notice = "PDCursesMod " PDC_VERDOT " - "\
                    PDC_stringize( PDC_VER_YEAR) "-" \
                    PDC_VER_MONTH_STR "-" \
                    PDC_stringize( PDC_VER_DAY);

SCREEN *SP = (SCREEN*)NULL;           /* curses variables */
WINDOW *curscr = (WINDOW *)NULL;      /* the current screen image */
WINDOW *stdscr = (WINDOW *)NULL;      /* the default screen window */

int LINES = 0;                        /* current terminal height */
int COLS = 0;                         /* current terminal width */
int TABSIZE = 8;

MOUSE_STATUS Mouse_status;

/* When the screen is initialized or resized,  we need to figure out
on which lines the ripped-off lines will go.  If initializing,  we
have to create the window for each ripped-off line and call its
corresponding callback function.  If we're just resizing,  we still
may have to expand the window(s) to include new columns.  */

static int _update_ripped_off_lines( SCREEN *sp)
{
    int i, lines_ripped_off_on_top = 0;

    for( i = 0; i < sp->linesrippedoff; i++)
    {
        const int line = (sp->linesripped[i].line < 0 ?
               LINES - 1 + lines_ripped_off_on_top : lines_ripped_off_on_top++);

        if( sp->linesripped[i].win)
        {
            sp->linesripped[i].win->_begy = line;
            if( COLS > sp->linesripped[i].win->_maxx)    /* expand window */
                if( ERR == wresize( sp->linesripped[i].win, 1, COLS))
                    return( -1);
        }
        else
        {             /* initializing ripped-off lines */
            sp->linesripped[i].win = newwin(1, COLS, line, 0);
            wmove( sp->linesripped[i].win, 0, 0);
            (sp->linesripped[i].init)( sp->linesripped[i].win, COLS);
        }
        touchwin( SP->linesripped[i].win);
        wnoutrefresh( SP->linesripped[i].win);
        LINES--;
    }
    return( lines_ripped_off_on_top);
}

SCREEN *newterm(const char *type, FILE *outfd, FILE *infd)
{
    int lines_ripped_off_on_top;

    PDC_LOG(("newterm() - called\n"));
    INTENTIONALLY_UNUSED_PARAMETER( type);

    if (SP && SP->alive)
        return NULL;
    SP = (SCREEN *)calloc(1, sizeof(SCREEN));
    assert( SP);
    if (!SP)
        return NULL;

    /* output_fd, input_fd should be initialized before PDC_src_open */
    SP->output_fd = outfd ? outfd : stdout;
    SP->input_fd = infd ? infd : stdin;

    if (PDC_scr_open() == ERR)
    {
        fprintf(stderr, "initscr(): Unable to create SP\n");
        exit(8);
    }

    SP->autocr = TRUE;       /* cr -> lf by default */
    SP->raw_out = FALSE;     /* tty I/O modes */
    SP->raw_inp = FALSE;     /* tty I/O modes */
    SP->cbreak = TRUE;
    SP->key_modifiers = 0L;
    SP->return_key_modifiers = FALSE;
    SP->echo = TRUE;
    SP->visibility = 1;
    SP->resized = FALSE;
    SP->_trap_mbe = 0L;
    SP->linesrippedoff = 0;
    SP->delaytenths = 0;
    SP->line_color = -1;
    SP->lastscr = (WINDOW *)NULL;
    SP->dbfp = NULL;
    SP->color_started = FALSE;
    SP->dirty = FALSE;
    SP->sel_start = -1;
    SP->sel_end = -1;
    SP->off_screen_windows = OFF_SCREEN_WINDOWS_TO_RIGHT_AND_BOTTOM;

    SP->orig_cursor = PDC_get_cursor_mode();

    LINES = SP->lines = PDC_get_rows();
    COLS = SP->cols = PDC_get_columns();

    if( PDC_init_atrtab())   /* set up default colors */
        return NULL;

    if (LINES < 2 || COLS < 2)
    {
        fprintf(stderr, "initscr(): LINES=%d COLS=%d: too small.\n",
                LINES, COLS);
        exit(4);
    }

    curscr = newwin(LINES, COLS, 0, 0);
    if (!curscr)
    {
        fprintf(stderr, "initscr(): Unable to create curscr.\n");
        exit(2);
    }

    SP->lastscr = newwin(LINES, COLS, 0, 0);
    if (!SP->lastscr)
    {
        fprintf(stderr, "initscr(): Unable to create SP->lastscr.\n");
        exit(2);
    }

    wattrset(SP->lastscr, (chtype)(-1));
    werase(SP->lastscr);

    PDC_slk_initialize();
    LINES -= SP->slklines;

    /* We have to sort out ripped off lines here, and reduce the height
       of stdscr by the number of lines ripped off */

    ripoffline( 0, NULL);   /* copy stored ripped-line data into SP */
    lines_ripped_off_on_top = _update_ripped_off_lines( SP);

    stdscr = newwin(LINES, COLS, lines_ripped_off_on_top, 0);
    if (!stdscr)
    {
        fprintf(stderr, "initscr(): Unable to create stdscr.\n");
        exit(1);
    }

    wclrtobot(stdscr);

    /* If preserving the existing screen, don't allow a screen clear */

    if (SP->_preserve)
    {
        untouchwin(curscr);
        untouchwin(stdscr);
        stdscr->_clear = FALSE;
        curscr->_clear = FALSE;
    }
    else
        curscr->_clear = TRUE;


    MOUSE_X_POS = MOUSE_Y_POS = -1;
    BUTTON_STATUS(1) = BUTTON_RELEASED;
    BUTTON_STATUS(2) = BUTTON_RELEASED;
    BUTTON_STATUS(3) = BUTTON_RELEASED;
    Mouse_status.changes = 0;

    SP->alive = TRUE;

    def_shell_mode();

    longname( );

    SP->c_buffer = (int *)malloc(_INBUFSIZ * sizeof(int));
    if (!SP->c_buffer)
        return NULL;
    SP->c_pindex = 0;
    SP->c_gindex = 1;

    SP->c_ungch = (int *)malloc(NUNGETCH * sizeof(int));
    if (!SP->c_ungch)
        return NULL;
    SP->c_ungind = 0;
    SP->c_ungmax = NUNGETCH;

    return SP;
}

WINDOW *initscr(void)
{
    PDC_LOG(("initscr() - called\n"));
    return( newterm( NULL, NULL, NULL) ? stdscr : NULL);
}

#ifdef XCURSES
WINDOW *Xinitscr(int argc, char **argv)
{
    PDC_LOG(("Xinitscr() - called\n"));

    PDC_set_args(argc, argv);
    return initscr();
}
#endif

int endwin(void)
{
    SP->in_endwin = TRUE;
    PDC_LOG(("endwin() - called\n"));

    /* Allow temporary exit from curses using endwin() */

    def_prog_mode();
    PDC_scr_close();

    assert( SP);
    SP->alive = FALSE;

    SP->in_endwin = FALSE;
    return OK;
}

bool isendwin(void)
{
    PDC_LOG(("isendwin() - called\n"));

    assert( SP);
    return SP ? !(SP->alive) : FALSE;
}

SCREEN *set_term(SCREEN *new_scr)
{
    PDC_LOG(("set_term() - called\n"));

    /* We only support one screen */

    return (new_scr == SP) ? SP : NULL;
}

void delscreen(SCREEN *sp)
{
    int i;

    PDC_LOG(("delscreen() - called\n"));

    assert( SP && sp == SP);
    if (!SP || sp != SP)
        return;

    traceoff( );
    free(SP->c_ungch);
    free(SP->c_buffer);

    PDC_slk_free();     /* free the soft label keys, if needed */

         /* Mark all windows as 'parentless'.  That way,  we can */
         /* delete all windows associated with SP.               */
    for( i = 0; i < SP->n_windows; i++)
        SP->window_list[i]->_parent = NULL;
    while( SP->n_windows)
        delwin( SP->window_list[0]);
                    /* With all windows deleted,  the window  */
                    /* list should be empty. */
    assert( !SP->window_list);
    if( SP->linesripped)
        free( SP->linesripped);

    PDC_free_atrtab( );
    stdscr = (WINDOW *)NULL;
    curscr = (WINDOW *)NULL;
    SP->lastscr = (WINDOW *)NULL;

    SP->alive = FALSE;

    PDC_scr_free();

    free(SP);
    SP = (SCREEN *)NULL;
}

int resize_term(int nlines, int ncols)
{
    PANEL *panel_ptr = NULL;

    PDC_LOG(("resize_term() - called: nlines %d\n", nlines));

    if( PDC_resize_screen(nlines, ncols) == ERR)
        return ERR;

    if( !stdscr)
        return OK;

    SP->resized = FALSE;

    SP->lines = PDC_get_rows();
    LINES = SP->lines - SP->linesrippedoff - SP->slklines;
    SP->cols = COLS = PDC_get_columns();

    if (SP->cursrow >= SP->lines)
        SP->cursrow = SP->lines - 1;
    if (SP->curscol >= SP->cols)
        SP->curscol = SP->cols - 1;

    if (wresize(curscr, SP->lines, SP->cols) == ERR ||
        wresize(stdscr, LINES, COLS) == ERR ||
        wresize(SP->lastscr, SP->lines, SP->cols) == ERR)
        return ERR;

    werase(SP->lastscr);
    curscr->_clear = TRUE;

    if (SP->slk_winptr)
    {
        if (wresize(SP->slk_winptr, SP->slklines, COLS) == ERR)
            return ERR;

        wmove(SP->slk_winptr, 0, 0);
        wclrtobot(SP->slk_winptr);
        PDC_slk_initialize();
        slk_noutrefresh();
    }

    LINES += SP->linesrippedoff;
    if( 0 > _update_ripped_off_lines( SP))
        return( ERR);

    touchwin(stdscr);
    wnoutrefresh(stdscr);

    while( (panel_ptr = panel_above( panel_ptr)) != NULL)
    {
        touchwin(panel_window(panel_ptr));
        wnoutrefresh(panel_window(panel_ptr));
    }
    return OK;
}

bool is_termresized(void)
{
    PDC_LOG(("is_termresized() - called\n"));

    return SP->resized;
}

const char *curses_version(void)
{
    return _curses_notice;
}

void PDC_get_version(PDC_VERSION *ver)
{
    extern enum PDC_port PDC_port_val;

    assert( ver);
    if (!ver)
        return;

    ver->flags = 0
#ifdef PDCDEBUG
        | PDC_VFLAG_DEBUG
#endif
#ifdef PDC_WIDE
        | PDC_VFLAG_WIDE
#endif
#ifdef PDC_FORCE_UTF8
        | PDC_VFLAG_UTF8
#endif
#ifdef PDC_DLL_BUILD
        | PDC_VFLAG_DLL
#endif
#ifdef PDC_RGB
        | PDC_VFLAG_RGB
#endif
        ;

    ver->build = PDC_BUILD;
    ver->major = PDC_VER_MAJOR;
    ver->minor = PDC_VER_MINOR;
    ver->change = PDC_VER_CHANGE;
    ver->csize = sizeof(chtype);
    ver->bsize = sizeof(bool);
    ver->port = PDC_port_val;
}

int set_tabsize(int tabsize)
{
    PDC_LOG(("set_tabsize() - called: tabsize %d\n", tabsize));

    if (tabsize < 1)
        return ERR;

    TABSIZE = tabsize;

    return OK;
}
