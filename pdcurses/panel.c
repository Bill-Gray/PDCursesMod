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

   del_panel() deletes pan, but not its associated window.

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

struct panel
{
    WINDOW *win;
    struct panel *below;
    struct panel *above;
    const void *user;
};

#define _startx( pan)  (getbegx( (pan)->win))
#define _starty( pan)  (getbegy( (pan)->win))
#define _endx( pan)    (_startx( pan) + getmaxx( (pan)->win))
#define _endy( pan)    (_starty( pan) + getmaxy( (pan)->win))

static PANEL _stdscr_pseudo_panel;

/* The 'deck' of panels is maintained as a circularly linked list,
with the stdscr pseudo-panel always in the list.  Thus,  the bottom
panel is the one above the stdscr pseudo-panel,  and the top panel
is the one below the stdscr pseudo-panel.  The advantage of this is
that the list always has at least one element and the links are
never NULLs.  So there are no edge cases to check.  The bit about
the top panel being below stdscr can be a little disorienting,
though. */

#define _bottom_panel  _stdscr_pseudo_panel.above
#define _top_panel     _stdscr_pseudo_panel.below

static bool _panels_overlapped( const PANEL *pan1, const PANEL *pan2)
{
    assert( pan1);
    assert( pan2);
    assert( pan1 != pan2);
    if (!pan1 || !pan2)
        return FALSE;

    return ((_starty( pan1) >= _starty( pan2) && _starty( pan1) < _endy( pan2))
         || (_starty( pan2) >= _starty( pan1) && _starty( pan2) < _endy( pan1)))
        && ((_startx( pan1) >= _startx( pan2) && _startx( pan1) < _endx( pan2))
         || (_startx( pan2) >= _startx( pan1) && _startx( pan2) < _endx( pan1)));
}

static void _pairwise_override( const PANEL *pan, PANEL *pan2)
{
    if( _panels_overlapped( pan, pan2))
    {
        const int start_y = max( _starty( pan), _starty( pan2));
        const int end_y   = min( _endy( pan),   _endy( pan2));
        const int start_x = max( _startx( pan), _startx( pan2));
        const int end_x =   min( _endx( pan),   _endx( pan2));
        int firstch, lastch, y;

        for( y = start_y; y < end_y; y++)
            if( PDC_touched_range( pan->win, y - _starty( pan),
                           &firstch, &lastch))
            {
                firstch += _startx( pan);
                lastch += _startx( pan);
                if( firstch < end_x && lastch > start_x)
                {
                    firstch -= _startx( pan2);
                    if( firstch < 0)
                        firstch = 0;
                    lastch -= _startx( pan2);
                    if( lastch > getmaxx( pan2->win) - 1)
                        lastch = getmaxx( pan2->win) - 1;
                    PDC_mark_cells_as_changed( pan2->win, y - _starty( pan2),
                        firstch, lastch);
                }
            }
    }
}

/* When a panel is hidden or deleted,  we need to update any
parts of panels that intersect that rectangle.  So we call
_override( pan, ALL_PANELS_IN_DECK).

When a panel is added or moved to the top,  we just have to make
sure that that panel is touched.  update_panels() will ensure that
panels above it get touched.

Replacing or moving a panel combined both of the above : first,
we 'hide'/'delete' it from its current location,  then add it at
its new location,  touched so it'll get updated at that location.

When a panel is added at the bottom,  any parts of panels above
it need to be redrawn.  So we call _override( pan, PANELS_ABOVE)
to ensure the overlapping regions are touched. */

#define PANELS_ABOVE 1
#define PANELS_BELOW 2
#define ALL_PANELS_IN_DECK (PANELS_ABOVE | PANELS_BELOW)

static void _override( const PANEL *pan, const int flags)
{
    PANEL *tpan;

    if( flags & PANELS_BELOW)       /* go from stdscr and work up */
    {
        for( tpan = &_stdscr_pseudo_panel; tpan != pan; tpan = tpan->above)
           _pairwise_override( pan, tpan);
    }
    if( flags & PANELS_ABOVE)
    {
        for( tpan = pan->above; tpan != &_stdscr_pseudo_panel; tpan = tpan->above)
           _pairwise_override( pan, tpan);
    }
}

/* check to see if panel is in the stack */

static bool _panel_is_linked(const PANEL *pan)
{
    assert( (pan->below && pan->above) || (!pan->below && !pan->above));
    return( pan->above != NULL);
}

/* link panel into stack at top */

static void _panel_link_top(PANEL *pan)
{
    assert( !_panel_is_linked(pan));
    assert( pan != _top_panel);

    pan->above = &_stdscr_pseudo_panel;
    pan->below = _top_panel;
    pan->above->below = pan->below->above = pan;
}

/* link panel into stack at bottom */

static void _panel_link_bottom(PANEL *pan)
{
    assert( !_panel_is_linked(pan));
    assert( pan != _bottom_panel);

    pan->above = _bottom_panel;
    pan->below = &_stdscr_pseudo_panel;
    pan->above->below = pan->below->above = pan;
}

static void _panel_unlink(PANEL *pan)
{
    PANEL *above = pan->above;
    PANEL *below = pan->below;

    assert( pan->below);
    assert( pan->above);
    assert( pan != &_stdscr_pseudo_panel);
    assert( _bottom_panel);
    pan->above->below = below;
    pan->below->above = above;
    pan->above = pan->below = NULL;
    assert( _bottom_panel);
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
        _panel_unlink(pan);

    _panel_link_bottom(pan);
    touchwin( pan->win);
    _override( pan, PANELS_ABOVE);

    return OK;
}

int del_panel(PANEL *pan)
{
    assert( pan);
    if (pan)
    {
        hide_panel(pan);
        free((char *)pan);
        return OK;
    }

    return ERR;
}

int hide_panel(PANEL *pan)
{
    assert( pan);
    assert( pan != &_stdscr_pseudo_panel);
    if (!pan)
        return ERR;
    if (!_panel_is_linked(pan))
    {
        assert( !pan->above);
        assert( !pan->below);
        pan->above = (PANEL *)0;
        pan->below = (PANEL *)0;
        return ERR;
    }

    touchwin( pan->win);
    _override( pan, ALL_PANELS_IN_DECK);
    _panel_unlink(pan);

    return OK;
}

int move_panel(PANEL *pan, int starty, int startx)
{
    WINDOW *win;

    assert( pan);
    if (!pan)
        return ERR;

    if (_panel_is_linked(pan))
    {
        touchwin( pan->win);
        _override( pan, ALL_PANELS_IN_DECK);
    }

    win = pan->win;

    return mvwin(win, starty, startx);
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
        _top_panel = _bottom_panel = &_stdscr_pseudo_panel;
    }

    if (pan)
    {
        pan->win = win;
        pan->above = (PANEL *)0;
        pan->below = (PANEL *)0;
        pan->user = NULL;
        show_panel(pan);
    }

    return pan;
}

PANEL *panel_above(const PANEL *pan)
{
    PANEL *rval = (pan ? pan->above : _bottom_panel);

    if( rval == &_stdscr_pseudo_panel)
        rval = NULL;
    return rval;
}

PANEL *panel_below(const PANEL *pan)
{
    PANEL *rval = (pan ? pan->below : _top_panel);

    if( rval == &_stdscr_pseudo_panel)
        rval = NULL;
    return rval;
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
    {
        touchwin( pan->win);
        _override(pan, ALL_PANELS_IN_DECK);
    }

    pan->win = win;

    if (_panel_is_linked(pan))
        touchwin( pan->win);

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
        _panel_unlink( pan);

    touchwin( pan->win);
    _panel_link_top(pan);

    return OK;
}

int top_panel(PANEL *pan)
{
    assert( pan);
    return show_panel(pan);
}

/* When we call update_panels(),  we have to look at every panel,
starting from _stdscr_pseudo_panel and going up.  If a panel
has been touched,  and the touched region corresponds to an
overlapping panel,  then the overlapping parts need to be touched
as well.  This boils down to looping through the linked list of
panels and calling _override( PANELS_ABOVE) for each one.  */

void update_panels(void)
{
    PANEL *pan = _bottom_panel;

    PDC_LOG(("update_panels() - called\n"));

    assert( pan);
    while( pan != &_stdscr_pseudo_panel)     /* look at each panel;  update */
    {                                        /* any panels that overlap it */
        _pairwise_override( &_stdscr_pseudo_panel, pan);
        _override( pan, PANELS_ABOVE);
        pan = pan->above;
    }

    if (is_wintouched(stdscr))
        wnoutrefresh( stdscr);

    pan = _bottom_panel;

    while (pan != &_stdscr_pseudo_panel)
    {
        if (is_wintouched(pan->win))
            wnoutrefresh( pan->win);

        pan = pan->above;
    }
}
