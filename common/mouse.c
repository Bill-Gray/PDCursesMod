/* Platform-independent parts of mouse handling.  This is surprisingly
hard to get right.

   The idea here is that the various platforms detect "raw" mouse
events.  The actual methods are,  of course,  platform-dependent,
but the "raw" events will consist of

- button N pressed
- button N released
- mouse moved
- wheel up/down/left/right

   The actual events returned by the PDCursesMod library,  however,
will be the above plus "cooked" events made from presses and releases :

- button N clicked
- button N double-clicked
- button N triple-clicked

   Some of these will be filtered,  according to the bits set in
SP->_trap_mbe.  This code contains the (platform-independent) logic
used to turn "raw" mouse events from the platform into "cooked"
events returned by the library,  with unwanted events filtered out.

   Presses and releases will be combined into clicks _if_ we're actually
looking for clicks from that button.  (If not,  PDCursesMod may return a
press and a click... if,  again,  corresponding bits are turned on.)  Two
clicks will be combined into a double-click, _if_ we're actually looking
for double-clicks.  And so on.

   In some cases,  the raw event may leave us in an "incomplete" state.
Let's say a mouse button is pressed, and we're looking for clicks on that
button (i.e.,  the BUTTONn_CLICKED bit is set in SP->_trap_mbe).  If so,
we need to wait up to SP->mouse_wait milliseconds for a release of that
button.  If that happens,  the button is clicked.  If not,  the button is
pressed (and presumably held,  and sometime later,  we should get a
'release' event for that button... should note that there's not actually
any guarantee that we won't get two presses or two releases in a row from
the same button.)

   If we're looking for double-clicks on that button,  we need to wait
yet another SP->mouse_wait milliseconds for another button press.  If we
get one,  we have to wait again to see if there's a button release.  So
a double-click could be returned as

press,  release,  press,  release (if we aren't looking for clicks,
   _or_ if SP->mouse_wait == 0)
click,  click (if we're looking for single-clicks but not double-clicks)
double-click (if we're looking for both)

   And while these events may get generated,  they may also never be
returned by the PDCursesMod library if corresponding bits in SP->_trap_mbe
are not set.

   From the viewpoint of the platform code,  the logic is :

While a "raw" event is pending and _add_raw_mouse_event() == TRUE,  we're
   in an incomplete state.  Wait for up to SP->mouse_wait milliseconds
   for another mouse event.
Filter out unwanted events.
If we have "cooked" mouse events left after this,  queue them.
Each time a mouse event is returned,  call _get_mouse_event() to retrieve
   that event,  put it into SP->mouse_status,  and remove it.

   Only button presses and releases get "cooked" (and,  as described above,
even they are not always cooked).  The _add_raw_mouse_event() will collect
the raw events and synthesize them into cooked ones.  It returns TRUE
if the platform should wait SP->mouse_wait milliseconds,  or FALSE if
there is no need to wait (i.e.,  the event cannot be combined with a
later event to make a click,  double-click,  or triple-click).  The logic
it uses is :

Add the event to the queue.
If it's a mouse movement or wheel event,  return FALSE to tell the
   platform that it doesn't have to wait SP->mouse_wait milliseconds
   for more mouse input.
(If we get this far,  the event must be a press or release.)
If SP->mouse_wait == 0,  or if BUTTONn_CLICKED isn't set in SP->_trap_mbe,
   we can again return FALSE.  In such cases,  we're just returning
   presses and releases without trying to combine them into clicks.
   Pretty much the same as if it were a wheel event or a move event.
If it's a button-pressed event :
   return TRUE (because we need to wait for a possible button release
   to make a click,  double-click,  or triple)
Else,  it's a button-release event :
  If there's a preceding press event for that button,  combine and
      make it a click.  (If not,  it can't be combined with anything;
      return FALSE.)
  If, preceding the click,  there's another click for that button,  combine
      them into a double-click.  If BUTTONn_TRIPLE_CLICKED is set in
      SP->_trap_mbe,  return TRUE to indicate that we need to wait to see
      if a third click happens.
  If it's preceded by a double-click,  combine them into a triple-click.
      We don't look for BUTTONn_QUADRUPLE_CLICKED in SP->_trap_mbe,
      because PDCursesMod doesn't handle quad-clicks.  We return FALSE.
  If the click can't be combined with previous events,  then our new
      single-click might be followed by another click.  Check the
      BUTTONn_DOUBLE_CLICKED flag in SP->_trap_mbe;  if it's set,  return
      TRUE.
  The current mouse state cannot be combined with further events to make
      new "cooked" events,  so there's no point in waiting for them.
      Return FALSE.

   A few notes :

   (1) In the above,  if we're in an incomplete state, we "wait up to
SP->mouse_wait milliseconds".  Most of the code has traditionally waited
that long,  even if the completing click or release came immediately.  It's
better to take small naps over the SP->mouse_wait milliseconds,  checking
to see if the completing event has come in.  For example :

while( the platform has a new mouse event && TRUE == _add_raw_mouse_event( ))
   {
   const long t_end = PDC_millisecs( ) + SP->mouse_wait;
   long ms_remaining;

   while( (ms_remaining = t_end - PDC_millisecs())  > 0)
      {
      napms( ms_remaining > 20 ? 20 : ms_remaining);
      if( there's a new mouse event pending)
         break;
      }
   }

   (Strictly speaking,  the naps are unnecessary.  Without them,  you
simply go to 100% CPU usage looking for the next mouse event during those
SP->mouse_wait milliseconds.  With enough clicking,  those cycles would
start to add up.)

   (2) In the above logic,  you can't get double-clicks without also getting
single-clicks,  and you can't get triple-clicks without getting doubles and
singles.  This may be changed eventually,  though it seems like an unlikely
use case.

   (3) One _can_ use the PDCurses*-specific BUTTONn_MOVED masks,  and
thereby get motion events only if the corresponding button(s) are held.
But for the sake of portability,  you probably should instead use
REPORT_MOUSE_POSITION to get _all_ mouse events.  Then keep track of which
buttons are held and filter out the events you aren't interested in.  */

/* I don't think we can ever have more than three events in the list.
(For example,  button 2 clicked,  button 2 pressed,  mouse moved.)  But
some platforms (framebuffer/DRM) can have a temporary fourth event if an
extra move event hasn't been merged.  And a larger queue paves the way for
a working ungetmouse() function.                 */

#define MLIST_SIZE 8

static MOUSE_STATUS mouse_list[MLIST_SIZE];
static int _mlist_count = 0;

static short _button_change_flag( const int button)
{
   return( (button < 3 ? 1 : 64) << button);
}

#define TRAPPING_EVENT( event, button) (SP->_trap_mbe & PDC_SHIFTED_BUTTON( event, (button) + 1))

bool _add_raw_mouse_event( int button, int event_type, const int modifiers,
                  const int x, const int y)
{
   bool wait_for_next_event = FALSE;
   MOUSE_STATUS *mptr = mouse_list + _mlist_count;
   static int held_buttons = 0;
   int i;

   assert( _mlist_count >= 0 && _mlist_count < MLIST_SIZE);
   assert( button >= 0 && button < PDC_MAX_MOUSE_BUTTONS);
   mptr->x = x;
   mptr->y = y;
   if( SP->ncurses_mouse)
      {
      if( event_type == PDC_MOUSE_WHEEL_RIGHT
               || event_type == PDC_MOUSE_WHEEL_LEFT)
         return( FALSE);      /* 'real' ncurses interface doesn't know about tilt mice */
      if( event_type == PDC_MOUSE_WHEEL_UP)
         {
         button = 3;
         event_type = BUTTON_PRESSED;
         }
      if( event_type == PDC_MOUSE_WHEEL_DOWN)
         {
         button = 4;
         event_type = BUTTON_PRESSED;
         }
      }
   for( i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
      mptr->button[i] = (short)modifiers;
   switch( event_type)
      {
      case BUTTON_MOVED:
         {
         MOUSE_STATUS *prev = (_mlist_count ? mptr - 1 : &SP->mouse_status);

         if( x != prev->x || y != prev->y)
            {
            mptr->changes = 0;
            if( SP->_trap_mbe & REPORT_MOUSE_POSITION)
               mptr->changes = PDC_MOUSE_MOVED;
            for( i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
               if( TRAPPING_EVENT( BUTTON1_MOVED, i)
                                 && ((held_buttons >> i) & 1))
                  {
                  mptr->button[i] |= BUTTON_MOVED;
                  mptr->changes |= PDC_MOUSE_MOVED | (1 << i);
                  }
            if( mptr->changes)
               {
               if( _mlist_count && prev->changes == mptr->changes)
                  {   /* already got a mouse move event;  just update that one */
                  _mlist_count--;
                  mptr--;
                  }
               mptr->x = x;
               mptr->y = y;
               _mlist_count++;
               }
            }
         }
         break;
      case PDC_MOUSE_WHEEL_UP:
      case PDC_MOUSE_WHEEL_DOWN:
      case PDC_MOUSE_WHEEL_LEFT:
      case PDC_MOUSE_WHEEL_RIGHT:
         if( SP->_trap_mbe & MOUSE_WHEEL_SCROLL)
            {
            _mlist_count++;
            mptr->changes = event_type;
            }
         break;
      case BUTTON_PRESSED:
         mptr->button[button] = BUTTON_PRESSED | (short)modifiers;
         mptr->changes = _button_change_flag( button);
         held_buttons |= (1 << button);
         _mlist_count++;
         if( SP->mouse_wait && TRAPPING_EVENT( BUTTON1_CLICKED, button))
            wait_for_next_event = TRUE;   /* may need to synthesize a click */
         if( SP->ncurses_mouse && button >= 3)
            wait_for_next_event = FALSE;  /* is really mouse wheel event */
         break;
      case BUTTON_RELEASED:
         mptr->button[button] = BUTTON_RELEASED | (short)modifiers;
         mptr->changes = _button_change_flag( button);
         held_buttons &= ~(1 << button);
         mptr->changes = _button_change_flag( button);
         if( _mlist_count && mptr[-1].changes == mptr->changes
                     && mptr[-1].button[button] == (BUTTON_PRESSED | modifiers))
            {        /* can combine press with release to make a click */
            mptr[-1].button[button] = BUTTON_CLICKED | (short)modifiers;
            if( _mlist_count > 1 && mptr[-2].changes == mptr->changes)
               {
               if( mptr[-2].button[button] == (BUTTON_DOUBLE_CLICKED | modifiers))
                  {
                  mptr[-2].button[button] = BUTTON_TRIPLE_CLICKED | (short)modifiers;
                  _mlist_count--;
                  }
               if( mptr[-2].button[button] == (BUTTON_CLICKED | modifiers))
                  {
                  mptr[-2].button[button] = BUTTON_DOUBLE_CLICKED | (short)modifiers;
                  _mlist_count--;
                  if( TRAPPING_EVENT( BUTTON1_TRIPLE_CLICKED, button))
                      wait_for_next_event = TRUE;
                  }                     /* wait to see if we get a triple-click */
               }
            else if( TRAPPING_EVENT( BUTTON1_DOUBLE_CLICKED, button))
               wait_for_next_event = TRUE;
            }
         else if( TRAPPING_EVENT( BUTTON1_RELEASED, button))
            _mlist_count++;   /* just returning an uncooked release event */
         break;
      default:
         fprintf( stderr, "Got event %d,  button %d\n", event_type, button);
         assert( 0);
         break;
      }
   return( wait_for_next_event);
}

bool _get_mouse_event( MOUSE_STATUS *mstatus)
{
   if( !mstatus)                    /* just checking to see if events are queued up */
      return( _mlist_count ? TRUE : FALSE);
   if( _mlist_count)                /* should filter untrapped press/releases */
      {
      *mstatus = mouse_list[0];
      _mlist_count--;
      memmove( mouse_list, mouse_list + 1, _mlist_count * sizeof( MOUSE_STATUS));
      return( TRUE);
      }
   else
      return FALSE;
}
