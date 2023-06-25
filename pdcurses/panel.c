/* PDCurses */

#include <curspriv.h>
#include <assert.h>

/*man-start**************************************************************

panel
-----

### Synopsis

    int bottom_panel(PANEL *pan);
    int del_panel(PANEL *pan);
    int hide_panel(PANEL *pan);
    int move_panel(PANEL *pan, int starty, int startx);
    PANEL *new_panel(WINDOW *win);
    PANEL *panel_above(const PANEL *pan);
    PANEL *panel_below(const PANEL *pan);
    PANEL *ground_panel(SCREEN *sp);
    PANEL *ceiling_panel(SCREEN *sp);
    int panel_hidden(const PANEL *pan);
    const void *panel_userptr(const PANEL *pan);
    WINDOW *panel_window(const PANEL *pan);
    int replace_panel(PANEL *pan, WINDOW *win);
    int set_panel_userptr(PANEL *pan, const void *uptr);
    int show_panel(PANEL *pan);
    int top_panel(PANEL *pan);
    void update_panels(void);

### Description

   For historic reasons, and for compatibility with other versions of
   curses, the panel functions are prototyped in a separate header,
   panel.h. In many implementations, they're also in a separate library,
   but PDCurses incorporates them.

   The panel functions provide a way to have depth relationships between
   curses windows. Panels can overlap without making visible the
   overlapped portions of underlying windows. The initial curses window,
   stdscr, lies beneath all panels. The set of currently visible panels
   is the 'deck' of panels.

   You can create panels, fetch and set their associated windows,
   shuffle panels in the deck, and manipulate them in other ways.

   bottom_panel() places pan at the bottom of the deck. The size,
   location and contents of the panel are unchanged.

   del_panel() deletes pan, but not its associated winwow.

   hide_panel() removes a panel from the deck and thus hides it from
   view.

   move_panel() moves the curses window associated with pan, so that its
   upper lefthand corner is at the supplied coordinates. (Don't use
   mvwin() on the window.)

   new_panel() creates a new panel associated with win and returns the
   panel pointer. The new panel is placed at the top of the deck.

   panel_above() returns a pointer to the panel in the deck above pan,
   or NULL if pan is the top panel. If the value of pan passed is NULL,
   this function returns a pointer to the bottom panel in the deck.

   panel_below() returns a pointer to the panel in the deck below pan,
   or NULL if pan is the bottom panel. If the value of pan passed is
   NULL, this function returns a pointer to the top panel in the deck.

   ground_panel() returns a pointer to the bottom panel in the deck.

   ceiling_panel() returns a pointer to the top panel in the deck.

   panel_hidden() returns OK if pan is hidden and ERR if it is not.

   panel_userptr() - Each panel has a user pointer available for
   maintaining relevant information. This function returns a pointer to
   that information previously set up by set_panel_userptr().

   panel_window() returns a pointer to the curses window associated with
   the panel.

   replace_panel() replaces the current window of pan with win.

   set_panel_userptr() - Each panel has a user pointer available for
   maintaining relevant information. This function sets the value of
   that information.

   show_panel() makes a previously hidden panel visible and places it
   back in the deck on top.

   top_panel() places pan on the top of the deck. The size, location and
   contents of the panel are unchanged.

   update_panels() refreshes the virtual screen to reflect the depth
   relationships between the panels in the deck. The user must use
   doupdate() to refresh the physical screen.

### Return Value

   Each routine that returns a pointer to an object returns NULL if an
   error occurs. Each panel routine that returns an integer, returns OK
   if it executes successfully and ERR if it does not.

### Portability
                             X/Open  ncurses  NetBSD
    bottom_panel                -       Y       Y
    del_panel                   -       Y       Y
    hide_panel                  -       Y       Y
    move_panel                  -       Y       Y
    new_panel                   -       Y       Y
    panel_above                 -       Y       Y
    panel_below                 -       Y       Y
    ground_panel                -       Y       N
    ceiling_panel               -       Y       N
    panel_hidden                -       Y       Y
    panel_userptr               -       Y       Y
    panel_window                -       Y       Y
    replace_panel               -       Y       Y
    set_panel_userptr           -       Y       Y
    show_panel                  -       Y       Y
    top_panel                   -       Y       Y
    update_panels               -       Y       Y

  Credits:
    Original Author - Warren Tucker <wht@n4hgf.mt-park.ga.us>

**man-end****************************************************************/

#include <panel.h>
#include <stdlib.h>

typedef struct panelobs PANELOBS;

struct panelobs
{
    struct panelobs *above;
    struct panel *pan;
};

struct panel
{
    WINDOW *win;
    struct panel *below;
    struct panel *above;
    const void *user;
    struct panelobs *obscure;
};

#define _startx( pan)  (getbegx( (pan)->win))
#define _starty( pan)  (getbegy( (pan)->win))
#define _endx( pan)    (_startx( pan) + getmaxx( (pan)->win))
#define _endy( pan)    (_starty( pan) + getmaxy( (pan)->win))

static PANEL *_bottom_panel = (PANEL *)0;
static PANEL *_top_panel = (PANEL *)0;
static PANEL _stdscr_pseudo_panel;

#define Wnoutrefresh(pan) wnoutrefresh((pan)->win)
#define Touchpan(pan) touchwin((pan)->win)
#define Touchline(pan, start, count) touchline((pan)->win, start, count)

static bool _panels_overlapped(PANEL *pan1, PANEL *pan2)
{
    assert( pan1);
    assert( pan2);
    if (!pan1 || !pan2)
        return FALSE;

    return ((_starty( pan1) >= _starty( pan2) && _starty( pan1) < _endy( pan2))
         || (_starty( pan2) >= _starty( pan1) && _starty( pan2) < _endy( pan1)))
        && ((_startx( pan1) >= _startx( pan2) && _startx( pan1) < _endx( pan2))
         || (_startx( pan2) >= _startx( pan1) && _startx( pan2) < _endx( pan1)));
}

static void _free_obscure(PANEL *pan)
{
    PANELOBS *tobs = pan->obscure;  /* "this" one */
    PANELOBS *nobs;                 /* "next" one */

    while (tobs)
    {
        nobs = tobs->above;
        free((char *)tobs);
        tobs = nobs;
    }
    pan->obscure = (PANELOBS *)0;
}

static void _pairwise_override( PANEL *pan, PANEL *pan2)
{
    int y = (_starty( pan) > _starty( pan2) ?
                 _starty( pan) : _starty( pan2));
    const int end_y = (_endy( pan) < _endy( pan2) ?
                 _endy( pan) : _endy( pan2));

    while( y < end_y)
    {
       if( is_linetouched(pan->win, y - _starty( pan)))
          Touchline(pan2, y - _starty( pan2), 1);
       y++;
    }
}

static void _override(PANEL *pan, int show)
{
    PANEL *pan2;
    PANELOBS *tobs = pan->obscure;      /* "this" one */

    if (show == 1)
        Touchpan(pan);
    else if (!show)
    {
        Touchpan(pan);
        Touchpan(&_stdscr_pseudo_panel);
    }
    else if (show == -1)
        while (tobs && (tobs->pan != pan))
            tobs = tobs->above;

    while (tobs)
    {
        if ((pan2 = tobs->pan) != pan)
            _pairwise_override( pan, pan2);
        tobs = tobs->above;
    }
    _pairwise_override( &_stdscr_pseudo_panel, pan);
}

static void _calculate_obscure(void)
{
    PANEL *pan, *pan2;
    PANELOBS *tobs;     /* "this" one */
    PANELOBS *lobs;     /* last one */

    pan = _bottom_panel;

    while (pan)
    {
        if (pan->obscure)
            _free_obscure(pan);

        lobs = (PANELOBS *)0;
        pan2 = _bottom_panel;

        while (pan2)
        {
            if (_panels_overlapped(pan, pan2))
            {
                if ((tobs = malloc(sizeof(PANELOBS))) == NULL)
                    return;

                tobs->pan = pan2;
                tobs->above = (PANELOBS *)0;

                if (lobs)
                    lobs->above = tobs;
                else
                    pan->obscure = tobs;

                lobs  = tobs;
            }

            pan2 = pan2->above;
        }

        _override(pan, 1);
        pan = pan->above;
    }
}

/* check to see if panel is in the stack */

static bool _panel_is_linked(const PANEL *pan)
{
    PANEL *pan2 = _bottom_panel;

    while (pan2)
    {
        if (pan2 == pan)
            return TRUE;

        pan2 = pan2->above;
    }

    return FALSE;
}

/* link panel into stack at top */

static void _panel_link_top(PANEL *pan)
{
#ifdef PANEL_DEBUG
    if (_panel_is_linked(pan))
        return;
#endif
    pan->above = (PANEL *)0;
    pan->below = (PANEL *)0;

    if (_top_panel)
    {
        _top_panel->above = pan;
        pan->below = _top_panel;
    }

    _top_panel = pan;

    if (!_bottom_panel)
        _bottom_panel = pan;

    _calculate_obscure();
}

/* link panel into stack at bottom */

static void _panel_link_bottom(PANEL *pan)
{
#ifdef PANEL_DEBUG
    if (_panel_is_linked(pan))
        return;
#endif
    pan->above = (PANEL *)0;
    pan->below = (PANEL *)0;

    if (_bottom_panel)
    {
        _bottom_panel->below = pan;
        pan->above = _bottom_panel;
    }

    _bottom_panel = pan;

    if (!_top_panel)
        _top_panel = pan;

    _calculate_obscure();
}

static void _panel_unlink(PANEL *pan)
{
    PANEL *prev;
    PANEL *next;

#ifdef PANEL_DEBUG
    if (!_panel_is_linked(pan))
        return;
#endif
    _override(pan, 0);
    _free_obscure(pan);

    prev = pan->below;
    next = pan->above;

    /* if non-zero, we will not update the list head */

    if (prev)
        prev->above = next;
    if (next)
        next->below = prev;

    if (pan == _bottom_panel)
        _bottom_panel = next;

    if (pan == _top_panel)
        _top_panel = prev;

    _calculate_obscure();

    pan->above = (PANEL *)0;
    pan->below = (PANEL *)0;
}

/************************************************************************
 *   The following are the public functions for the panels library.     *
 ************************************************************************/

int bottom_panel(PANEL *pan)
{
    assert( pan);
    if (!pan)
        return ERR;

    if (pan == _bottom_panel)
        return OK;

    if (_panel_is_linked(pan))
        hide_panel(pan);

    _panel_link_bottom(pan);

    return OK;
}

int del_panel(PANEL *pan)
{
    assert( pan);
    if (pan)
    {
        if (_panel_is_linked(pan))
            hide_panel(pan);

        free((char *)pan);
        return OK;
    }

    return ERR;
}

int hide_panel(PANEL *pan)
{
    assert( pan);
    if (!pan)
        return ERR;

    if (!_panel_is_linked(pan))
    {
        pan->above = (PANEL *)0;
        pan->below = (PANEL *)0;
        return ERR;
    }

    _panel_unlink(pan);

    return OK;
}

int move_panel(PANEL *pan, int starty, int startx)
{
    WINDOW *win;
    int rval;

    assert( pan);
    if (!pan)
        return ERR;

    if (_panel_is_linked(pan))
        _override(pan, 0);

    win = pan->win;

    rval = mvwin(win, starty, startx);

    if (_panel_is_linked(pan))
        _calculate_obscure();

    return rval;
}

PANEL *new_panel(WINDOW *win)
{
    PANEL *pan;

    assert( win);
    if (!win)
        return (PANEL *)NULL;

    pan  = malloc(sizeof(PANEL));

    if (!_stdscr_pseudo_panel.win)
    {
        _stdscr_pseudo_panel.win = stdscr;
        _stdscr_pseudo_panel.user = "stdscr";
        _stdscr_pseudo_panel.obscure = (PANELOBS *)0;
    }

    if (pan)
    {
        pan->win = win;
        pan->above = (PANEL *)0;
        pan->below = (PANEL *)0;
#ifdef PANEL_DEBUG
        pan->user = "new";
#else
        pan->user = (char *)0;
#endif
        pan->obscure = (PANELOBS *)0;
        show_panel(pan);
    }

    return pan;
}

PANEL *panel_above(const PANEL *pan)
{
    return pan ? pan->above : _bottom_panel;
}

PANEL *panel_below(const PANEL *pan)
{
    return pan ? pan->below : _top_panel;
}

PANEL *ceiling_panel( SCREEN *sp)
{
   INTENTIONALLY_UNUSED_PARAMETER( sp);
   return( panel_below( NULL));
}

PANEL *ground_panel( SCREEN *sp)
{
   INTENTIONALLY_UNUSED_PARAMETER( sp);
   return( panel_above( NULL));
}

int panel_hidden(const PANEL *pan)
{
    assert( pan);
    if (!pan)
        return ERR;

    return _panel_is_linked(pan) ? ERR : OK;
}

const void *panel_userptr(const PANEL *pan)
{
    assert( pan);
    return pan ? pan->user : NULL;
}

WINDOW *panel_window(const PANEL *pan)
{
    PDC_LOG(("panel_window() - called\n"));

    assert( pan);
    if (!pan)
        return (WINDOW *)NULL;

    return pan->win;
}

int replace_panel(PANEL *pan, WINDOW *win)
{
    assert( pan);
    assert( win);
    if (!pan)
        return ERR;

    if (_panel_is_linked(pan))
        _override(pan, 0);

    pan->win = win;

    if (_panel_is_linked(pan))
        _calculate_obscure();

    return OK;
}

int set_panel_userptr(PANEL *pan, const void *uptr)
{
    assert( pan);
    if (!pan)
        return ERR;

    pan->user = uptr;
    return OK;
}

int show_panel(PANEL *pan)
{
    assert( pan);
    if (!pan)
        return ERR;

    if (pan == _top_panel)
        return OK;

    if (_panel_is_linked(pan))
        hide_panel(pan);

    _panel_link_top(pan);

    return OK;
}

int top_panel(PANEL *pan)
{
    assert( pan);
    return show_panel(pan);
}

void update_panels(void)
{
    PANEL *pan;

    PDC_LOG(("update_panels() - called\n"));

    pan = _bottom_panel;

    while (pan)
    {
        _override(pan, -1);
        pan = pan->above;
    }

    if (is_wintouched(stdscr))
        Wnoutrefresh(&_stdscr_pseudo_panel);

    pan = _bottom_panel;

    while (pan)
    {
        if (is_wintouched(pan->win) || !pan->above)
            Wnoutrefresh(pan);

        pan = pan->above;
    }
}
