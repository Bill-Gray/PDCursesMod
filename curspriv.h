/* Private definitions and declarations for use within PDCurses.
   These should generally not be referenced by applications. */

#ifndef __CURSES_INTERNALS__
#define __CURSES_INTERNALS__ 1

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE 1   /* kill nonsense warnings */
#endif

#define CURSES_LIBRARY
#include <curses.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__TURBOC__) || defined(__EMX__) || defined(__DJGPP__) || \
    defined(PDC_99) || defined(__WATCOMC__)
# if !defined( HAVE_VSSCANF) && !defined( __DMC__)
#  define HAVE_VSSCANF 1     /* have vsscanf() */
# endif
#endif

#if defined(PDC_99) || defined(__WATCOMC__)
# if !defined( HAVE_SNPRINTF) && !defined( __DMC__)
#  define HAVE_SNPRINTF 1   /* have snprintf() */
# endif
# if !defined( HAVE_VSNPRINTF) && !defined( __DMC__)
#  define HAVE_VSNPRINTF 1   /* have vsnprintf() */
# endif
#endif

#if defined( PDC_FORCE_UTF8) && !defined( PDC_WIDE)
   #define PDC_WIDE
#endif

/*----------------------------------------------------------------------*/

typedef struct           /* structure for ripped off lines */
{
    int line;
    int (*init)(WINDOW *, int);
    WINDOW *win;
} RIPPEDOFFLINE;

/* Window properties */

#define _SUBWIN    0x01  /* window is a subwindow */
#define _PAD       0x10  /* X/Open Pad. */
#define _SUBPAD    0x20  /* X/Open subpad. */

/* Miscellaneous */

#define _NO_CHANGE -1    /* flags line edge unchanged */

#define _ECHAR     0x08  /* Erase char       (^H) */
#define _DWCHAR    0x17  /* Delete Word char (^W) */
#define _DLCHAR    0x15  /* Delete Line char (^U) */

/*----------------------------------------------------------------------*/

/* Platform implementation functions */

void    PDC_beep(void);
bool    PDC_can_change_color(void);
int     PDC_color_content(int, int *, int *, int *);
bool    PDC_check_key(void);
int     PDC_curs_set(int);
void    PDC_doupdate(void);
void    PDC_flushinp(void);
int     PDC_get_columns(void);
int     PDC_get_cursor_mode(void);
int     PDC_get_key(void);
int     PDC_get_rows(void);
void    PDC_gotoyx(int, int);
bool    PDC_has_mouse(void);
int     PDC_init_color(int, int, int, int);
int     PDC_modifiers_set(void);
int     PDC_mouse_set(void);
void    PDC_napms(int);
void    PDC_reset_prog_mode(void);
void    PDC_reset_shell_mode(void);
int     PDC_resize_screen(int, int);
void    PDC_restore_screen_mode(int);
void    PDC_save_screen_mode(int);
#ifdef XCURSES
void    PDC_set_args(int, char **);
#endif
void    PDC_scr_close(void);
void    PDC_scr_free(void);
int     PDC_scr_open(void);
void    PDC_set_keyboard_binary(bool);
void    PDC_transform_line(int, int, int, const chtype *);
void    PDC_transform_line_sliced(int, int, int, const chtype *);
const char *PDC_sysname(void);

/* Internal cross-module functions */

int     PDC_init_atrtab(void);
void    PDC_free_atrtab(void);
WINDOW *PDC_makelines(WINDOW *);
WINDOW *PDC_makenew(int, int, int, int);
long    PDC_millisecs( void);
int     PDC_mouse_in_slk(int, int);
void    PDC_slk_free(void);
void    PDC_slk_initialize(void);
void    PDC_sync(WINDOW *);
PDCEX void    PDC_set_default_colors( const int, const int);
void    PDC_set_changed_cells_range( WINDOW *, const int y, const int start, const int end);
void    PDC_mark_line_as_changed( WINDOW *win, const int y);
void    PDC_mark_cells_as_changed( WINDOW *, const int y, const int start, const int end);
void    PDC_mark_cell_as_changed( WINDOW *, const int y, const int x);
bool    PDC_touched_range( const WINDOW *win, const int y, int *firstch, int *lastch);

#ifdef PDC_WIDE
int     PDC_mbtowc(wchar_t *, const char *, size_t);
size_t  PDC_mbstowcs(wchar_t *, const char *, size_t);
size_t  PDC_wcstombs(char *, const wchar_t *, size_t);
PDCEX int PDC_wcwidth( const int32_t ucs);
#endif

#define MAX_UNICODE 0x110000

#ifdef PDCDEBUG
# define PDC_LOG(x) if (SP && SP->dbfp) PDC_debug x
#else
# define PDC_LOG(x)
#endif

/* Internal macros for attributes */

#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define DIVROUND(num, divisor) ((num) + ((divisor) >> 1)) / (divisor)

#define PDC_CLICK_PERIOD 150  /* time to wait for a click, if
                                 not set by mouseinterval() */
#define PDC_MAXCOL       768  /* maximum possible COLORS; may be less */

#define _INBUFSIZ        512  /* size of terminal input buffer */
#define NUNGETCH         256  /* max # chars to ungetch() */
#define MAX_PACKET_LEN    90  /* max # chars to send to PDC_transform_line */

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

#define _is_altcharset( ch)  (((ch) & (A_ALTCHARSET | (A_CHARTEXT ^ 0x7f))) == A_ALTCHARSET)

struct _win               /* definition of a window */
{
    int   _cury;          /* current pseudo-cursor */
    int   _curx;
    int   _maxy;          /* max window coordinates */
    int   _maxx;
    int   _begy;          /* origin on screen */
    int   _begx;
    int   _flags;         /* window properties */
    chtype _attrs;        /* standard attributes and colors */
    chtype _bkgd;         /* background, normally blank */
    bool  _clear;         /* causes clear at next refresh */
    bool  _leaveit;       /* leaves cursor where it is */
    bool  _scroll;        /* allows window scrolling */
    bool  _nodelay;       /* input character wait flag */
    bool  _immed;         /* immediate update flag */
    bool  _sync;          /* synchronise window ancestors */
    bool  _use_keypad;    /* flags keypad key mode active */
    chtype **_y;          /* pointer to line pointer array */
    int   *_firstch;      /* first changed character in line */
    int   *_lastch;       /* last changed character in line */
    int   _tmarg;         /* top of scrolling region */
    int   _bmarg;         /* bottom of scrolling region */
    int   _delayms;       /* milliseconds of delay for getch() */
    int   _parx, _pary;   /* coords relative to parent (0,0) */
    struct _win *_parent; /* subwin's pointer to parent win */
    int   _pminrow, _pmincol;    /* saved position used only for pads */
    int   _sminrow, _smaxrow;    /* saved position used only for pads */
    int   _smincol, _smaxcol;    /* saved position used only for pads */
};

#if PDC_COLOR_BITS < 15
    typedef int16_t hash_idx_t;
#else
    typedef int32_t hash_idx_t;
#endif

#define MAX_RIPPEDOFFLINES 5

struct _screen
{
    bool  alive;          /* if initscr() called, and not endwin() */
    bool  autocr;         /* if cr -> lf */
    bool  cbreak;         /* if terminal unbuffered */
    bool  echo;           /* if terminal echo */
    bool  raw_inp;        /* raw input mode (v. cooked input) */
    bool  raw_out;        /* raw output mode (7 v. 8 bits) */
    bool  audible;        /* FALSE if the bell is visual */
    bool  mono;           /* TRUE if current screen is mono */
    bool  resized;        /* TRUE if TERM has been resized */
    bool  orig_attr;      /* TRUE if we have the original colors */
    short orig_fore;      /* original screen foreground color */
    short orig_back;      /* original screen foreground color */
    int   cursrow;        /* position of physical cursor */
    int   curscol;        /* position of physical cursor */
    int   visibility;     /* visibility of cursor */
    int   orig_cursor;    /* original cursor size */
    int   lines;          /* new value for LINES */
    int   cols;           /* new value for COLS */
    mmask_t _trap_mbe;             /* trap these mouse button events */
    int   mouse_wait;              /* time to wait (in ms) for a
                                      button release after a press, in
                                      order to count it as a click */
    int   slklines;                /* lines in use by slk_init() */
    WINDOW *slk_winptr;            /* window for slk */
    int   linesrippedoff;          /* lines ripped off via ripoffline() */
    RIPPEDOFFLINE *linesripped;
    int   delaytenths;             /* 1/10ths second to wait block
                                      getch() for */
    bool  _preserve;               /* TRUE if screen background
                                      to be preserved */
    int   _restore;                /* specifies if screen background
                                      to be restored, and how */
    unsigned long key_modifiers;   /* key modifiers (SHIFT, CONTROL, etc.)
                                      on last key press */
    bool  return_key_modifiers;    /* TRUE if modifier keys are
                                      returned as "real" keys */
    bool  in_endwin;               /* if we're in endwin(),  we should use
                                      only signal-safe code */
    MOUSE_STATUS mouse_status;     /* last returned mouse status */
    short line_color;     /* color of line attributes - default -1 */
    attr_t termattrs;     /* attribute capabilities */
    WINDOW *lastscr;      /* the last screen image */
    FILE *dbfp;           /* debug trace file pointer */
    bool  color_started;  /* TRUE after start_color() */
    bool  dirty;          /* redraw on napms() after init_color() */
    int   sel_start;      /* start of selection (y * COLS + x) */
    int   sel_end;        /* end of selection */
    int  *c_buffer;       /* character buffer */
    int   c_pindex;       /* putter index */
    int   c_gindex;       /* getter index */
    int  *c_ungch;        /* array of ungotten chars */
    int   c_ungind;       /* ungetch() push index */
    int   c_ungmax;       /* allocated size of ungetch() buffer */
    struct _pdc_pair *pairs;
    int pairs_allocated;
    int first_col;
    bool default_colors;
    hash_idx_t *pair_hash_tbl;
    int pair_hash_tbl_size, pair_hash_tbl_used;
    int n_windows;
    WINDOW **window_list;
    unsigned trace_flags;
    bool want_trace_fflush;
    FILE *output_fd, *input_fd;
};

PDCEX  SCREEN       *SP;          /* curses variables */

#ifdef __cplusplus
}
#endif

#endif /* __CURSES_INTERNALS__ */
