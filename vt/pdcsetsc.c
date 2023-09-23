#include <stdio.h>
#include <curspriv.h>
#include "pdcvt.h"

/*man-start**************************************************************

pdcsetsc
--------

### Synopsis

    int PDC_set_blink(bool blinkon);
    int PDC_set_bold(bool boldon);
    void PDC_set_title(const char *title);

### Description

   PDC_set_blink() toggles whether the A_BLINK attribute sets an actual
   blink mode (TRUE), or sets the background color to high intensity
   (FALSE). The default is platform-dependent (FALSE in most cases). It
   returns OK if it could set the state to match the given parameter,
   ERR otherwise.

   PDC_set_bold() toggles whether the A_BOLD attribute selects an actual
   bold font (TRUE), or sets the foreground color to high intensity
   (FALSE). It returns OK if it could set the state to match the given
   parameter, ERR otherwise.

   PDC_set_title() sets the title of the window in which the curses
   program is running. This function may not do anything on some
   platforms.

### Portability
                             X/Open  ncurses  NetBSD
    PDC_set_blink               -       -       -
    PDC_set_title               -       -       -

**man-end****************************************************************/

    /* #define BLINKING_CURSOR   CSI "?12h"    */
#define BLINKING_BLOCK      CSI "1 q"
#define STEADY_BLOCK        CSI "2 q"
#define BLINKING_UNDERLINE  CSI "3 q"
#define STEADY_UNDERLINE    CSI "4 q"
      /* "bar" = "vertical line".  xterm only. */
#define BLINKING_BAR        CSI "5 q"
#define STEADY_BAR          CSI "6 q"
#define CURSOR_ON           CSI "?25h"
#define CURSOR_OFF          CSI "?25l"

int PDC_curs_set( int visibility)
{
    int ret_vis;

    PDC_LOG(("PDC_curs_set() - called: visibility=%d\n", visibility));

    ret_vis = SP->visibility;

#ifndef LINUX_FRAMEBUFFER_PORT
    if( !SP->visibility && visibility)    /* turn cursor back on */
        PDC_puts_to_stdout( CURSOR_ON);
    else if( SP->visibility && !visibility)
        PDC_puts_to_stdout( CURSOR_OFF);
#endif
    SP->visibility = visibility;
#ifdef LINUX_FRAMEBUFFER_PORT
    PDC_gotoyx( SP->cursrow, SP->curscol);
#else
    if( !PDC_is_ansi)
    {
        const int vis1 = visibility & 0xff;
        const int vis2 = (visibility >> 8) & 0xff;
        const char *command;

        if( vis1 && vis2)      /* show solid */
            switch( vis1)
            {
                case 1:        /* "normal" = underline */
                case 5:        /* bottom half block;  we don't actually have that */
                    command = STEADY_UNDERLINE;
                    break;
                case 2:        /* full block */
                    command = STEADY_BLOCK;
                    break;
                case 4:       /* caret */
                    command = STEADY_BAR;
                    break;
                default:      /* since we can't do outline, cross, etc. */
                    command = STEADY_UNDERLINE;
                    break;
            }
        else switch( vis1 ? vis1 : vis2)
            {
                case 0:        /* just turning it off */
                    command = CURSOR_OFF;
                    break;
                case 1:        /* "normal" = underline */
                case 5:        /* bottom half block;  we don't actually have that */
                    command = BLINKING_UNDERLINE;
                    break;
                case 2:        /* full block */
                    command = BLINKING_BLOCK;
                    break;
                case 4:        /* caret */
                    command = BLINKING_BAR;
                    break;
                default:      /* since we can't do outline, cross, etc. */
                    command = BLINKING_UNDERLINE;
                    break;
            }

        PDC_puts_to_stdout( command);
    }
#endif
    return ret_vis;
}

static int reset_attr( const attr_t attr, const bool attron)
{
    attr_t prev_termattrs;

    if (!SP)
        return ERR;
    prev_termattrs = SP->termattrs;
    if( attron)
        SP->termattrs |= attr;
    else
        SP->termattrs &= ~attr;
    if( prev_termattrs != SP->termattrs)
       curscr->_clear = TRUE;
    return OK;
}

int PDC_set_blink(bool blinkon)
{
   return( reset_attr( A_BLINK, blinkon));
}

int PDC_set_bold(bool boldon)
{
   return( reset_attr( A_BOLD, boldon));
}

void PDC_set_title( const char *title)
{
    PDC_LOG(("PDC_set_title() - called:<%s>\n", title));

#if !defined( DOS) && !defined( LINUX_FRAMEBUFFER_PORT)
    if( !PDC_is_ansi)
    {
        PDC_puts_to_stdout( OSC "2;");
        PDC_puts_to_stdout( title);
        PDC_puts_to_stdout( "\a");
    }
#else
    INTENTIONALLY_UNUSED_PARAMETER( title);
#endif
}
