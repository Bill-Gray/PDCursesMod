#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#if !defined( _WIN32) && !defined( DOS)
#define USE_TERMIOS
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig_term;
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef MOUSE_MOVED
#endif

#include <assert.h>
#include "curspriv.h"
#include "pdcvt.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifdef DOS
int PDC_is_ansi = TRUE;
#else
int PDC_is_ansi = FALSE;
#endif

int PDC_rows = -1, PDC_cols = -1;

#ifdef _WIN32

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT      0x0200
#endif
#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN        0x0008
#endif

/* In DOS/Windows,  we have two possible modes of operation.  If we can
successfully use SetConsoleMode to ENABLE_VIRTUAL_TERMINAL_INPUT,
we have access to most of what we'd use on xterm.  If not,  we can only
use what ANSI.SYS or ANSI.COM (or their NANSI or NNANSI variants) support.
We'll get sixteen colors,  no mouse events,  no resizable windows,  etc.

   So we check the return value from SetConsoleMode and set PDC_is_ansi
accordingly.  (In DOS,  PDC_is_ansi is always true -- there's no xterm-like
support there.  On non-MS platforms,  PDC_is_ansi is always false...
though that should be revisited for the Linux console,  and probably
elsewhere.)  */

static int PDC_get_screen_size( int *n_cols, int *n_rows)
{
    const HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;

    assert( INVALID_HANDLE_VALUE != hOut);
    GetConsoleScreenBufferInfo( hOut, &ScreenBufferInfo);
    *n_cols = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
    *n_rows = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;
    return( 0);
}

static int set_win10_for_vt_codes( const bool setting_mode)
{
    const HANDLE hIn = GetStdHandle( STD_INPUT_HANDLE);
    HANDLE hOut;
    DWORD dwMode = 0;
    static DWORD old_input_mode;
    const DWORD out_mask = ENABLE_VIRTUAL_TERMINAL_PROCESSING
                               | DISABLE_NEWLINE_AUTO_RETURN;

    if( hIn == INVALID_HANDLE_VALUE)
        return GetLastError( );
    PDC_is_ansi = TRUE;
    if( setting_mode)
        {
        GetConsoleMode( hIn, &old_input_mode);
        dwMode = ENABLE_VIRTUAL_TERMINAL_INPUT;
        }
    else       /* restoring initial mode */
        dwMode = old_input_mode;
    if( !SetConsoleMode( hIn, dwMode))
        return GetLastError( );
        /* Set output mode to handle virtual terminal sequences */
    hOut = GetStdHandle( STD_OUTPUT_HANDLE);
    if( hOut == INVALID_HANDLE_VALUE)
        return GetLastError( );

    if( !GetConsoleMode( hOut, &dwMode))
        return GetLastError( );
    if( setting_mode)
        dwMode |= out_mask;
    else                    /* clearing VT mode,  not setting it */
        dwMode &= ~out_mask;
    if( !SetConsoleMode( hOut, dwMode))
        return GetLastError( );
              /* If we've gotten this far,  the terminal has been  */
              /* set up to process xterm-like sequences : */
    PDC_is_ansi = FALSE;
    if( setting_mode)
        PDC_get_screen_size( &PDC_cols, &PDC_rows);
    return( 0);
}
#endif

bool PDC_resize_occurred = FALSE;
static mmask_t _stored_trap_mbe;

/* COLOR_PAIR to attribute encoding table. */

void PDC_reset_prog_mode( void)
{
#ifdef USE_TERMIOS
    struct termios term;

    tcgetattr( fileno( SP->input_fd), &orig_term);
    memcpy( &term, &orig_term, sizeof( term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_iflag &= ~ICRNL;
    term.c_cc[VSUSP] = _POSIX_VDISABLE;   /* disable Ctrl-Z */
    term.c_cc[VSTOP] = _POSIX_VDISABLE;   /* disable Ctrl-S */
    term.c_cc[VSTART] = _POSIX_VDISABLE;   /* disable Ctrl-Q */
    tcsetattr( fileno( SP->input_fd), TCSANOW, &term);
#endif
#if !defined( _WIN32) && !defined( DOS)
    if( !PDC_is_ansi)
        PDC_puts_to_stdout( CSI "?1006h");    /* Set SGR mouse tracking,  if available */
#endif
#ifndef DOS
    if( !SP->_preserve)
       PDC_puts_to_stdout( CSI "?47h");      /* Save screen */
#endif
    PDC_puts_to_stdout( "\033" "7");         /* save cursor & attribs (VT100) */

    SP->_trap_mbe = _stored_trap_mbe;
    PDC_mouse_set( );          /* clear any mouse event captures */
    PDC_resize_occurred = FALSE;
}

void PDC_reset_shell_mode( void)
{
}

static int initial_PDC_rows, initial_PDC_cols;

int PDC_resize_screen(int nlines, int ncols)
{
   if( PDC_rows == -1)     /* initscr( ) hasn't been called;  we're just */
      {                    /* setting desired size at startup */
      initial_PDC_rows = nlines;
      initial_PDC_cols = ncols;
      }
   else if( nlines > 1 && ncols > 1 && !PDC_is_ansi)
      {
      char tbuff[50];

#ifdef HAVE_SNPRINTF
      snprintf( tbuff, sizeof( tbuff), CSI "8;%d;%dt", nlines, ncols);
#else
      sprintf( tbuff, CSI "8;%d;%dt", nlines, ncols);
#endif
      PDC_puts_to_stdout( tbuff);
      PDC_rows = nlines;
      PDC_cols = ncols;
      }
   return( 0);
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_scr_close( void)
{
#if !defined( _WIN32) && !defined( DOS)
   if( !PDC_is_ansi)
       PDC_puts_to_stdout( CSI "?1006l");    /* Turn off SGR mouse tracking */
#endif
   PDC_puts_to_stdout( "\033" "8");         /* restore cursor & attribs (VT100) */
   PDC_puts_to_stdout( CSI "m");         /* set default screen attributes */
#if !defined( DOS)
   PDC_puts_to_stdout( CSI "?47l");      /* restore screen */
   PDC_curs_set( 2);          /* blinking block cursor */
#endif
   PDC_gotoyx( PDC_cols - 1, 0);
   _stored_trap_mbe = SP->_trap_mbe;
   SP->_trap_mbe = 0;
   PDC_mouse_set( );          /* clear any mouse event captures */
#ifdef _WIN32
   set_win10_for_vt_codes( FALSE);
#else
   #if !defined( DOS)
      tcsetattr( fileno( SP->input_fd), TCSANOW, &orig_term);
   #endif
#endif
   PDC_doupdate( );
   PDC_flushinp( );
   PDC_puts_to_stdout( NULL);      /* free internal cache */
   return;
}

void PDC_scr_free( void)
{
    PDC_free_palette( );
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters( 0, NULL);
#endif
}

#ifdef USE_TERMIOS

int PDC_get_terminal_fd( void);        /* pdckbd.c */

static void sigwinchHandler( int sig)
{
   struct winsize ws;

   INTENTIONALLY_UNUSED_PARAMETER( sig);
   if( -1 != ioctl( PDC_get_terminal_fd( ), TIOCGWINSZ, &ws))
      if( PDC_rows != ws.ws_row || PDC_cols != ws.ws_col)
         {
         PDC_rows = ws.ws_row;
         PDC_cols = ws.ws_col;
         PDC_resize_occurred = TRUE;
         if (SP)
            SP->resized = TRUE;
         }
}

int PDC_n_ctrl_c = 0;

static void sigintHandler( int sig)
{
    INTENTIONALLY_UNUSED_PARAMETER( sig);
    if( !SP->raw_inp)
    {
        PDC_scr_close( );
        PDC_scr_free( );
        exit( 0);
    }
    else
        PDC_n_ctrl_c++;
}
#endif

#define MAX_LINES 1000
#define MAX_COLUMNS 1000

bool PDC_has_rgb_color = FALSE;

int PDC_scr_open(void)
{
    char *capabilities = getenv( "PDC_VT");
    char *term_env = getenv( "TERM");
    const char *colorterm = getenv( "COLORTERM");
    chtype PDC_capabilities = 0;
#ifdef USE_TERMIOS
    struct sigaction sa;
#endif
#ifdef _WIN32
    set_win10_for_vt_codes( TRUE);
#endif

    PDC_LOG(("PDC_scr_open called\n"));
    if( term_env && !strcmp( term_env, "linux"))
       PDC_is_ansi = TRUE;
    else if( colorterm && !strcmp( colorterm, "truecolor"))
       PDC_has_rgb_color = TRUE;
    if( capabilities)      /* these should really come from terminfo! */
       {
       if( strstr( capabilities, "RGB"))
          PDC_has_rgb_color = TRUE;
       if( strstr( capabilities, "UND"))
          PDC_capabilities |= A_UNDERLINE;
       if( strstr( capabilities, "BLI"))
          PDC_capabilities |= A_BLINK;
       if( strstr( capabilities, "DIM"))
          PDC_capabilities |= A_DIM;
       if( strstr( capabilities, "STA"))
          PDC_capabilities |= A_STANDOUT;
       if( strstr( capabilities, "STR"))
          PDC_capabilities |= A_STRIKEOUT;
       }
    COLORS = (PDC_is_ansi ? 16 : 256);
    if( PDC_has_rgb_color)
       COLORS = 256 + (256 * 256 * 256);
    assert( SP);
    if (!SP || PDC_init_palette( ))
        return ERR;

    setbuf( SP->input_fd, NULL);
#ifdef USE_TERMIOS
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigwinchHandler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        fprintf( stderr, "Sigaction failed\n");
        return( -1);
    }
    sigwinchHandler( 0);

    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf( stderr, "Sigaction (INT) failed\n");
        return( -1);
    }
#else
    {
        const char *env = getenv("PDC_LINES");

        if( env)
           PDC_rows = atoi( env);
        if( PDC_rows < 2)
           PDC_rows = 24;
        env = getenv( "PDC_COLS");
        if( env)
           PDC_cols = atoi( env);
        if( PDC_cols < 2)
           PDC_cols = 80;
    }
#endif
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->visibility = 0;                /* no cursor,  by default */
    SP->curscol = SP->cursrow = 0;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->orig_attr = TRUE;
    SP->orig_fore = SP->orig_back = -1;
    SP->termattrs = PDC_capabilities & ~A_BLINK;

    while( PDC_get_rows( ) < 1 && PDC_get_columns( ) < 1)
      ;     /* wait for screen to be drawn and size determined */
    if( initial_PDC_rows > 1 && initial_PDC_cols > 1)
    {
        PDC_resize_screen( initial_PDC_rows, initial_PDC_cols);
        while( PDC_get_rows( ) != initial_PDC_rows
            && PDC_get_columns( ) != initial_PDC_rows)
           ;
    }

    SP->lines = PDC_get_rows();
    SP->cols = PDC_get_columns();

    if (SP->lines < 2 || SP->lines > MAX_LINES
       || SP->cols < 2 || SP->cols > MAX_COLUMNS)
    {
        fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }

    SP->_preserve = (getenv("PDC_PRESERVE_SCREEN") != NULL);
    PDC_reset_prog_mode();
    PDC_LOG(("PDC_scr_open exit\n"));
    return( 0);
}

void PDC_set_resize_limits( const int new_min_lines,
                            const int new_max_lines,
                            const int new_min_cols,
                            const int new_max_cols)
{
   INTENTIONALLY_UNUSED_PARAMETER( new_min_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_min_cols);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_cols);
   return;
}


bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content( int color, int *red, int *green, int *blue)
{
    const PACKED_RGB col = PDC_get_palette_entry( color);

    *red =   DIVROUND( (int32_t)Get_RValue(col) * (int32_t)1000, (int32_t)255);
    *green = DIVROUND( (int32_t)Get_GValue(col) * (int32_t)1000, (int32_t)255);
    *blue =  DIVROUND( (int32_t)Get_BValue(col) * (int32_t)1000, (int32_t)255);

    return OK;
}

int PDC_init_color( int color, int red, int green, int blue)
{
    const PACKED_RGB new_rgb = PACK_RGB(DIVROUND((int32_t)red   * (int32_t)255, (int32_t)1000),
                                        DIVROUND((int32_t)green * (int32_t)255, (int32_t)1000),
                                        DIVROUND((int32_t)blue  * (int32_t)255, (int32_t)1000));

    if( !PDC_set_palette_entry( color, new_rgb))
        curscr->_clear = TRUE;
    return OK;
}
