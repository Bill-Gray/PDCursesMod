/* PDCurses */

#include "pdcx11.h"

#include <string.h>
#include <assert.h>

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
   Function              | X/Open | ncurses | NetBSD
   :---------------------|:------:|:-------:|:------:
   PDC_set_blink         |    -   |    -    |   -
   PDC_set_title         |    -   |    -    |   -

**man-end****************************************************************/

int PDC_curs_set(int visibility)
{
    int ret_vis = SP->visibility;

    PDC_LOG(("PDC_curs_set() - called: visibility=%d\n", visibility));

    if (visibility != -1)
        SP->visibility = visibility;

    PDC_display_cursor(SP->cursrow, SP->curscol, SP->cursrow,
                       SP->curscol, visibility);

    return ret_vis;
}

void PDC_set_title(const char *title)
{
    PDC_LOG(("PDC_set_title() - called:<%s>\n", title));

    XtVaSetValues(pdc_toplevel, XtNtitle, title, NULL);
}

int PDC_set_blink(bool blinkon)
{
    if (!SP)
        return ERR;

    if (SP->color_started)          /* see ../common/pdccolor.txt */
        COLORS = 256 + (1 << 24);

    if (blinkon)
    {
        if (!(SP->termattrs & A_BLINK))
        {
            SP->termattrs |= A_BLINK;
            pdc_blinked_off = FALSE;
            XtAppAddTimeOut(pdc_app_context, pdc_app_data.textBlinkRate,
                            PDC_blink_text, NULL);
        }
    }
    else if (SP->termattrs & A_BLINK)
    {
        curscr->_clear = TRUE;
        SP->termattrs &= ~A_BLINK;
     }
    return OK;
}

int PDC_set_bold(bool boldon)
{
    attr_t old_attrs;

    assert( SP);
    if (!SP)
        return ERR;

    old_attrs = SP->termattrs;
    if (boldon)
        SP->termattrs |= A_BOLD;
    else
        SP->termattrs &= ~A_BOLD;
    if( old_attrs != SP->termattrs)
        curscr->_clear = TRUE;
    return OK;
}
