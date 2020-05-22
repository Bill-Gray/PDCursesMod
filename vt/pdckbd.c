#include <stdio.h>
#include <string.h>
#include <assert.h>
#if defined( _WIN32) || defined( DOS)
   #include <conio.h>
   #define USE_CONIO
#else
   #include <sys/select.h>
   #include <unistd.h>
#endif
#include "curspriv.h"

#if defined( __BORLANDC__) || defined( DOS)
   #define WINDOWS_VERSION_OF_KBHIT kbhit
#else
   #define WINDOWS_VERSION_OF_KBHIT _kbhit
#endif

/* Modified from the accepted answer at

https://stackoverflow.com/questions/33025599/move-the-cursor-in-a-c-program

_kbhit( ) returns -1 if no key has been hit,  and the keycode if one
has been hit.  You can just loop on it until the return value is >= 0.
Hitting a function or arrow or similar key will cause 27 (escape) to
be returned,  followed by cryptic codes that depend on what terminal
emulation is in place.

   Further info on VT100/ANSI control sequences is at

https://www.gnu.org/software/screen/manual/html_node/Control-Sequences.html
*/

extern bool PDC_resize_occurred;

static bool check_key( int *c)
{
    bool rval;
#ifndef USE_CONIO
    const int STDIN = 0;
    struct timeval timeout;
    fd_set rdset;

    if( PDC_resize_occurred)
       return( TRUE);
    FD_ZERO( &rdset);
    FD_SET( STDIN, &rdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if( select( STDIN + 1, &rdset, NULL, NULL, &timeout) > 0)
       {
       rval = TRUE;
       if( c)
          *c = getchar( );
       }
    else
       rval = FALSE;
#else
    if( WINDOWS_VERSION_OF_KBHIT( ))
       {
       rval = TRUE;
       if( c)
          *c = _getch( );
       }
    else
       rval = FALSE;
#endif
    return( rval);
}

bool PDC_check_key( void)
{
   return( check_key( NULL));
}

void PDC_flushinp( void)
{
   int thrown_away_char;

   while( check_key( &thrown_away_char))
      ;
}

#ifdef USE_CONIO
static int xlate_vt_codes_for_dos( const int key1, const int key2)
{
   static const int tbl[] =   {
      KEY_UP,      72,
      KEY_DOWN,    80,
      KEY_LEFT,    75,
      KEY_RIGHT,   77,
      KEY_F(11),  133,
      KEY_F(12),  134,
      KEY_IC,      82,
      KEY_DC,      83,
      KEY_PPAGE,   73,
      KEY_NPAGE,   81,
      KEY_HOME, 2, '[', 'H',
      KEY_END,  2, 'O', 'F',

      KEY_F(1),   59,
      KEY_F(2),   60,
      KEY_F(3),   61,
      KEY_F(4),   62,
      KEY_F(5),   63,
      KEY_F(6),   64,
      KEY_F(7),   65,
      KEY_F(8),   66,
      KEY_F(9),   67,
      KEY_F(10),   68,
      0, 0 };
   int i, rval = 0;

   for( i = 0; tbl[i] && !rval; i += 2)
      if( key2 == tbl[i + 1])
         rval = tbl[i];
   return( rval);
}

#endif

#define MAX_COUNT 15

/* Mouse events include six bytes.  First three are

ESC [ M

   Next byte is 96 for mouse wheel up,  97 for down,  or (for more
"traditional" mouse events) 32 plus :

   0 for button 1
   1 for button 2
   2 for button 3
   3 for release
   4 if Shift is pressed
   8 if Alt (Meta) is pressed
   16 if Ctrl is pressed

   Note that 'release' doesn't tell you _which_ is released.  If only
one has been pressed (the usual case),  it's presumably the one you
released.  If two or more buttons are pressed simultaneously,  the
"releases" are reported in the numerical order of the buttons,  not
the order in which they're actually released (which we don't know).

   My tilt mouse reports 'tilt left' as a left button (1) and 'tilt right'
as a middle button press.  Wheel events get shift,  alt,  ctrl added in
(but that doesn't seem to be getting through in PDCurses... to be fixed).
Button events only get Ctrl (though I think you might get the other events
on some terminals).

   "Correct" mouse handling will require that we detect a button-down,
then hold off for SP->mouse_wait to see if we get a release event.  */

static int xlate_vt_codes( const int *c, const int count)
{
   static const int tbl[] =  {
               KEY_UP,   2, '[', 'A',
               KEY_DOWN, 2, '[', 'B',
               KEY_LEFT, 2, '[', 'D',
               KEY_RIGHT,2, '[', 'C',
               KEY_HOME, 2, 'O', 'H',
               KEY_HOME, 2, '[', 'H',
               KEY_END,  2, 'O', 'F',
               KEY_END,  2, '[', 'F',
               KEY_B2,   2, '[', 'E',
               KEY_BTAB, 2, '[', 'Z',     /* Shift-Tab */
               KEY_IC,   3, '[', '2', '~',
               KEY_DC,   3, '[', '3', '~',
               KEY_PPAGE, 3, '[', '5', '~',
               KEY_NPAGE, 3, '[', '6', '~',

               CTL_LEFT,  5, '[', '1', ';', '5', 'D',
               CTL_RIGHT, 5, '[', '1', ';', '5', 'C',
               CTL_UP,    5, '[', '1', ';', '5', 'A',
               CTL_DOWN,  5, '[', '1', ';', '5', 'B',

               ALT_PGUP, 5, '[', '5', ';', '3', '~',
               ALT_PGDN, 5, '[', '6', ';', '3', '~',

               KEY_F(1), 3, '[', '[', 'A',      /* Linux console */
               KEY_F(2), 3, '[', '[', 'B',
               KEY_F(3), 3, '[', '[', 'C',
               KEY_F(4), 3, '[', '[', 'D',
               KEY_F(5), 3, '[', '[', 'E',
               KEY_END,  3, '[', '4', '~',
               KEY_HOME, 3, '[', '1', '~',

               KEY_F(1), 2, 'O', 'P',
               KEY_F(1), 4, '[', '1', '1', '~',
               KEY_F(2), 2, 'O', 'Q',
               KEY_F(2), 4, '[', '1', '2', '~',
               KEY_F(3), 2, 'O', 'R',
               KEY_F(3), 4, '[', '1', '3', '~',
               KEY_F(4), 2, 'O', 'S',
               KEY_F(4), 4, '[', '1', '4', '~',
               KEY_F(5), 4, '[', '1', '5', '~',
               KEY_F(6), 4, '[', '1', '7', '~',
               KEY_F(7), 4, '[', '1', '8', '~',
               KEY_F(8), 4, '[', '1', '9', '~',
               KEY_F(9), 4, '[', '2', '0', '~',
               KEY_F(10), 4, '[', '2', '1', '~',
               KEY_F(11), 4, '[', '2', '3', '~',
               KEY_F(12), 4, '[', '2', '4', '~',
               KEY_F(13), 5, '[', '1', ';', '2', 'P',      /* shift-f1 */
               KEY_F(14), 5, '[', '1', ';', '2', 'Q',
               KEY_F(15), 5, '[', '1', ';', '2', 'R',
               KEY_F(16), 5, '[', '1', ';', '2', 'S',
               KEY_F(17), 6, '[', '1', '5', ';', '2', '~',  /* shift-f5 */
               KEY_F(18), 6, '[', '1', '7', ';', '2', '~',
               KEY_F(19), 6, '[', '1', '8', ';', '2', '~',
               KEY_F(20), 6, '[', '1', '9', ';', '2', '~',
               KEY_F(21), 6, '[', '2', '0', ';', '2', '~',
               KEY_F(22), 6, '[', '2', '1', ';', '2', '~',
               KEY_F(23), 6, '[', '2', '3', ';', '2', '~',  /* shift-f11 */
               KEY_F(24), 6, '[', '2', '4', ';', '2', '~',
               ALT_UP, 5, '[', '1', ';', '3', 'A',
               ALT_RIGHT, 5, '[', '1', ';', '3', 'C',
               ALT_DOWN, 5, '[', '1', ';', '3', 'B',
               ALT_LEFT, 5, '[', '1', ';', '3', 'D',
               CTL_UP, 5, '[', '1', ';', '5', 'A',
               CTL_RIGHT, 5, '[', '1', ';', '5', 'C',
               CTL_DOWN, 5, '[', '1', ';', '5', 'B',
               CTL_LEFT, 5, '[', '1', ';', '5', 'D',
               KEY_SUP, 5, '[', '1', ';', '2', 'A',
               KEY_SRIGHT, 5, '[', '1', ';', '2', 'C',
               KEY_SDOWN, 5, '[', '1', ';', '2', 'B',
               KEY_SLEFT, 5, '[', '1', ';', '2', 'D',
               0 };
   int i, rval = -1;
   const int *tptr;

   if( count == 1)
      {
      if( c[0] >= 'a' && c[0] <= 'z')
         rval = ALT_A + c[0] - 'a';
      else if( c[0] >= '0' && c[0] <= '9')
         rval = ALT_0 + c[0] - '0';
      else
         {
         const char *text = "',./[];`\x1b\\=-\x0a\x7f";
         const char *tptr = strchr( text, c[0]);
         const int codes[] = { ALT_FQUOTE, ALT_COMMA, ALT_STOP, ALT_FSLASH,
                     ALT_LBRACKET, ALT_RBRACKET,
                     ALT_SEMICOLON, ALT_BQUOTE, ALT_ESC,
                     ALT_BSLASH, ALT_EQUAL, ALT_MINUS, ALT_ENTER, ALT_BKSP };

         if( tptr)
             rval = codes[tptr - text];
         }
      }
   else if( count == 5 && c[0] == '[' && c[1] == 'M')
      rval = KEY_MOUSE;
   else if( count > 6 && c[0] == '[' && c[1] == '<'
                             && (c[count - 1] == 'M' || c[count - 1] == 'm'))
      rval = KEY_MOUSE;    /* SGR mouse mode */
   for( tptr = tbl; rval == -1 && *tptr; tptr += 2 + tptr[1])
      if( count == tptr[1])
         {
         i = 0;
         while( i < count && tptr[i + 2] == c[i])
            i++;
         if( i == count)
            rval = tptr[0];
         }
   return( rval);
}

int PDC_get_key( void)
{
   int rval = -1;

   if( PDC_resize_occurred)
      {
      PDC_resize_occurred = FALSE;
      return( KEY_RESIZE);
      }
   if( check_key( &rval))
      {
      int c[MAX_COUNT];

#ifdef USE_CONIO
      SP->key_code = (rval == 0 || rval == 224);
      if( SP->key_code)
         {
         int key2;

         while( !check_key( &key2))
            ;
         rval = xlate_vt_codes_for_dos( rval, key2);
         return( rval);
         }

#endif
      SP->key_code = (rval == 27);
      if( rval == 27)
         {
         int count = 0;

         rval = -1;
         while( rval == -1 && count < MAX_COUNT && check_key( &c[count]))
            {
            count++;
            rval = xlate_vt_codes( c, count);
            if( rval == ALT_LBRACKET && check_key( NULL))
               rval = -1;
            }
         if( !count)             /* Escape hit */
            rval = 27;
         count--;
         if( rval == KEY_MOUSE)
            {
            int idx, button, flags = 0, i, x, y;
            static int held = 0;
            bool release;

            if( c[1] == 'M')     /* 'traditional' mouse encoding */
               {
               x = (unsigned char)( c[3] - ' ' - 1);
               y = (unsigned char)( c[4] - ' ' - 1);
               idx = c[2];
               button = idx & 3;
               release = (button == 3);
               if( release)         /* which button was released? */
                  {
                  button = 0;
                  while( button < 3 && !((held >> button) & 1))
                     button++;
                  }
               }
            else                 /* SGR mouse encoding */
               {
               int n_fields, n_bytes, i;
               char tbuff[MAX_COUNT];

               assert( c[1] == '<');
               for( i = 0; i < count; i++)
                  tbuff[i] = (char)c[i];
               tbuff[count] = '\0';
               n_fields = sscanf( tbuff + 2, "%d;%d;%d%n", &idx,
                                                    &x, &y, &n_bytes);
               assert( n_fields == 3);
               assert( c[count] == 'M' || c[count] == 'm');
               release = (c[count] == 'm');
               button = idx & 3;
               if( idx & 0x40)            /* (SGR) wheel mouse event; */
                  idx |= 0x20;            /* requires this bit set in 'traditional' encoding */
               else if( idx & 0x20)       /* (SGR) mouse move event sets a different bit */
                  idx ^= 0x60;            /* in the traditional encoding */
               x--;
               y--;
               }
            if( idx & 4)
               flags |= BUTTON_SHIFT;
            if( idx & 8)
               flags |= BUTTON_ALT;
            if( idx & 16)
               flags |= BUTTON_CONTROL;
            if( (idx & 0x60) == 0x40)    /* mouse move */
               {
               if( x != SP->mouse_status.x || y != SP->mouse_status.y)
                  {
                  int report_event = PDC_MOUSE_MOVED;

                  if( button == 0)
                     report_event |= 1;
                  if( button == 1)
                     report_event |= 2;
                  if( button == 2)
                     report_event |= 4;
                  SP->mouse_status.changes = report_event;
                  for( i = 0; i < 3; i++)
                     SP->mouse_status.button[i] = (i == button ? BUTTON_MOVED : 0);
                  for( i = 0; i < 3; i++)
                     SP->mouse_status.button[i] |= flags;
                  button = 3;
                  }
               else              /* mouse didn't actually move */
                  return( -1);
               }
            if( button < 3)
               {
               memset(&SP->mouse_status, 0, sizeof(MOUSE_STATUS));
               SP->mouse_status.button[button] =
                              (release ? BUTTON_RELEASED : BUTTON_PRESSED);
               if( (idx & 0x60) == 0x60)    /* actually mouse wheel event */
                  SP->mouse_status.changes =
                        (button ? PDC_MOUSE_WHEEL_DOWN : PDC_MOUSE_WHEEL_UP);
               else     /* "normal" mouse button */
                  SP->mouse_status.changes = (1 << button);
               if( !release && !(idx & 64))   /* wait for a possible release */
                  {
                  int n_events = 0;

                  while( n_events < 6)
                     {
                     PDC_napms( SP->mouse_wait);
                     if( check_key( c))
                        {
                        count = 0;
                        while( count < MAX_COUNT && check_key( &c[count]))
                           count++;
                        n_events++;
                        }
                     else
                        break;
                     }
                  if( !n_events)   /* just a click,  no release(s) */
                     held ^= (1 << button);
                  else if( n_events == 1)
                      SP->mouse_status.button[button] = BUTTON_CLICKED;
                  else if( n_events <= 3)
                      SP->mouse_status.button[button] = BUTTON_DOUBLE_CLICKED;
                  else if( n_events <= 5)
                      SP->mouse_status.button[button] = BUTTON_TRIPLE_CLICKED;
                  }
               }
            for( i = 0; i < 3; i++)
               SP->mouse_status.button[i] |= flags;
            SP->mouse_status.x = x;
            SP->mouse_status.y = y;
            }
         }
      else if( (rval & 0xc0) == 0xc0)      /* start of UTF-8 */
         {
         check_key( &c[0]);
         assert( (c[0] & 0xc0) == 0x80);
         c[0] &= 0x3f;
         if( !(rval & 0x20))      /* two-byte : U+0080 to U+07ff */
            rval = c[0] | ((rval & 0x1f) << 6);
         else
            {
            check_key( &c[1]);
            assert( (c[1] & 0xc0) == 0x80);
            c[1] &= 0x3f;
            if( !(rval & 0x10))   /* three-byte : U+0800 - U+ffff */
               rval = (c[1] | (c[0] << 6) | ((rval & 0xf) << 12));
            else              /* four-byte : U+FFFF - U+10FFFF : SMP */
               {              /* (Supplemental Multilingual Plane) */
               check_key( &c[2]);
               assert( (c[2] & 0xc0) == 0x80);
               c[2] &= 0x3f;
               rval = (c[2] | (c[1] << 6) | (c[0] << 12) | ((rval & 0xf) << 18));
               }
            }
         }
      else if( rval == 127)
         rval = 8;
      }
   return( rval);
}

int PDC_modifiers_set( void)
{
   return( OK);
}

bool PDC_has_mouse( void)
{
    return TRUE;
}

/* Xterm defaults to reporting no mouse events.  If you request mouse movement
events even with no button pressed,  state 1003 is set ("report everything").
If you don't request such movements,  but _do_ want to know about movements
with one of the first three buttons down,  state 1002 is set.  If you just
want certain mouse events (clicks and doubleclicks,  say),  state 1000 is
set.  And if the mouse mask is zero ("don't tell me anything about the
mouse"),  mouse events are shut off.

At first,  this code just set state 1003.  Xterm reported bazillions of
events (which were filtered out according to SP->_trap_mbe).  I don't think
this really mattered much on my machine,  but I assume Xterm supports this
sort of filtering at a higher level for a reason.  */

int PDC_mouse_set( void)
{
   extern bool PDC_is_ansi;

   if( !PDC_is_ansi)
      {
      static int curr_tracking_state = -1;
      int tracking_state;

      if( SP->_trap_mbe & REPORT_MOUSE_POSITION)
         tracking_state = 1003;
      else if( SP->_trap_mbe & (BUTTON1_MOVED | BUTTON2_MOVED | BUTTON3_MOVED))
         tracking_state = 1002;
      else
         tracking_state = (SP->_trap_mbe ? 1000 : 0);
      if( curr_tracking_state != tracking_state)
         {
         if( curr_tracking_state > 0)
            printf( "\033[?%dl", curr_tracking_state);
         if( tracking_state)
            printf( "\033[?%dh", tracking_state);
         curr_tracking_state = tracking_state;
         PDC_doupdate( );
         }
      }
   return(  OK);
}

void PDC_set_keyboard_binary( bool on)
{
   return;
}
