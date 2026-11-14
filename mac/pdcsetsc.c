#include "pdcmac.h"

int PDC_curs_set(int visibility)
{
    int ret_vis;

    PDC_LOG(("PDC_curs_set() - called: visibility=%d\n", visibility));
    ret_vis = SP->visibility;
    SP->visibility = visibility;
    PDC_gotoyx(SP->cursrow, SP->curscol);
    return ret_vis;
}

void PDC_set_title(const char *title)
{
    Str255 pname;

    PDC_LOG(("PDC_set_title() - called:<%s>\n", title));
    if (!PDC_window)
        return;
    {
        unsigned int length;
        unsigned int i;

        length = 0;
        while (title[length] != 0 && length < 255)
            length++;
        pname[0] = (unsigned char)length;
        for (i = 0; i < length; i++)
            pname[i + 1] = (unsigned char)title[i];
    }
    SetWTitle(PDC_window, pname);
}

static int reset_attr(attr_t attr, bool attron)
{
    attr_t prev;

    if (!SP)
        return ERR;
    prev = SP->termattrs;
    if (attron)
        SP->termattrs |= attr;
    else
        SP->termattrs &= ~attr;
    if (prev != SP->termattrs && curscr)
        curscr->_clear = TRUE;
    return OK;
}

int PDC_set_blink(bool blinkon)
{
    if (SP && SP->color_started)
        COLORS = 256 + (256 * 256 * 256);
    return reset_attr(A_BLINK, blinkon);
}

int PDC_set_bold(bool boldon)
{
    return reset_attr(A_BOLD, boldon);
}
