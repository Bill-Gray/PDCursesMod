#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#if defined( _WIN32) || defined( DOS)
   #include <conio.h>
   #define USE_CONIO
#else
   #include <sys/select.h>
   #include <unistd.h>
#endif

/* Digital Mars and DJGPP for DOS each implement getch() and/or _getch().
The following,  declared _before_ curses.h is #included,  enable us to
evade conflicts with the PDCursesMod getch().   */

#ifdef __DJGPP__
int dmc_getch( void)
{
   return( getch( ));
}
#endif

#if defined( __DMC__)
int dmc_getch( void)         /* see 'getch2.c' */
{
   return( _getch( ));
}
#endif

#include "curspriv.h"
#include "pdcvt.h"
#include "../common/mouse.c"
#include "../common/xlates.h"

#if defined( __BORLANDC__) || defined( __DJGPP__)
   #define WINDOWS_VERSION_OF_KBHIT kbhit
#else
   #define WINDOWS_VERSION_OF_KBHIT _kbhit
#endif

#ifdef USE_CONIO
   #include "../common/dos_key.h"
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
    struct timeval timeout;
    fd_set rdset;
    extern int PDC_n_ctrl_c;

    if( PDC_resize_occurred)
       return( TRUE);

#ifdef HAVE_MOUSE
    _check_mouse( );
#endif
#ifdef LINUX_FRAMEBUFFER_PORT
    PDC_check_for_blinking( );
#endif
    if( PDC_n_ctrl_c)
       {
       if( c)
          {
          PDC_n_ctrl_c--;
          *c = 3;
          }
       return( TRUE);
       }
    FD_ZERO( &rdset);
    FD_SET( fileno( SP->input_fd), &rdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if( select( fileno( SP->input_fd) + 1, &rdset, NULL, NULL, &timeout) > 0)
       {
       rval = TRUE;
       if( c)
          *c = fgetc( SP->input_fd);
       }
    else
       rval = FALSE;
#else
    if( WINDOWS_VERSION_OF_KBHIT( ))
       {
       rval = TRUE;
       if( c)
#if defined( __DMC__) || defined( __DJGPP__)
          *c = dmc_getch( );         /* see 'getch2.c' */
#else
          *c = _getch( );
#endif
       }
    else
       rval = FALSE;
#endif
    return( rval);
}

bool PDC_check_key( void)
{
   if( _mlist_count)
      return TRUE;
   return( check_key( NULL));
}

void PDC_flushinp( void)
{
   int thrown_away_char;

   _mlist_count = 0;
   while( check_key( &thrown_away_char))
      ;
}

#ifdef USE_CONIO
static int xlate_vt_codes_for_dos( const int key1, const int key2)
{
   int rval = 0;
   const int max_key2 = (int)( sizeof( key_table) / sizeof( key_table[0]));

   INTENTIONALLY_UNUSED_PARAMETER( key1);
   if( key2 >= 0 && key2 < max_key2)         /* see ../common/dos_key.h */
      rval = key_table[key2];                /* for key_table[]         */
   return( rval);
}
#endif

#define MAX_COUNT 15

/* If possible,  we use the SGR mouse tracking modes.  These allow
for wheel mice and more than 224 columns and rows.  See

https://invisible-island.net/xterm/ctlseqs/ctlseqs.html

   for details on this and more on how 'traditional' mouse events are
encoded.

   'Traditional' mouse events include six bytes.  First three are

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

static int _single_char_cases( const char c, int *modifiers)
{
   int rval = -1, dummy;

   if( !modifiers)
      modifiers = &dummy;
   if( c >= 'a' && c <= 'z')
      rval = ALT_A + c - 'a';
   else if( c >= 'A' && c <= 'Z')
      {
      rval = ALT_A + c - 'A';
      *modifiers = SHF;
      }
   else if( c >= 1 && c <= 26)
      {
      rval = ALT_A + c - 1;
      *modifiers = CTL;
      }
   else if( c >= '0' && c <= '9')
      rval = ALT_0 + c - '0';
   else
      {
      const char *text = "',./[];`\x1b\\=-\x0a\x7f";
      const char *tptr = strchr( text, c);
      const int codes[] = { ALT_FQUOTE, ALT_COMMA, ALT_STOP, ALT_FSLASH,
                  ALT_LBRACKET, ALT_RBRACKET,
                  ALT_SEMICOLON, ALT_BQUOTE, ALT_ESC,
                  ALT_BSLASH, ALT_EQUAL, ALT_MINUS, ALT_ENTER, ALT_BKSP };

      if( tptr)
          rval = codes[tptr - text];
      else
         {
         rval = c;
         *modifiers = SHF;
         }
      }
   *modifiers |= ALT;
   return( rval);
}

static int _key_defined_ext( const char *definition, int *modifiers)
{
   if( *definition == 0x1b)
      {
      const size_t ilen = strlen( definition) - 1;
      size_t i;

      definition++;
      if( ilen == 1)
         return( _single_char_cases( definition[0], modifiers));
      for( i = 0; i < n_keycodes; i++)
         if( !strcmp( xlates[i].xlation, definition))
            {
            if( modifiers)
               *modifiers = xlates[i].modifiers;
            return( xlates[i].key_code);
            }
         else if( strlen( xlates[i].xlation) > ilen &&
                        !memcmp( xlates[i].xlation, definition, ilen))
            return( -1);  /* conflict w/a longer string */
      }
   return( 0);    /* no keycode bound to that definition */
}

static int _look_up_key( const int *c, const int count, int *modifiers)
{
   char definition[MAX_COUNT];
   int i;

   assert( count < (int)sizeof( definition) - 2);
   *definition = 0x1b;
   for( i = 0; i < count; i++)
      definition[i + 1] = (char)c[i];
   definition[count + 1] = '\0';
   return( _key_defined_ext( definition, modifiers));
}

static int xlate_vt_codes( const int *c, const int count, int *modifiers)
{
   int rval = -1;

   assert( count);
   *modifiers = 0;
   if( count == 5 && c[0] == '[' && c[1] == 'M')
      rval = KEY_MOUSE;
   else if( count > 6 && c[0] == '[' && c[1] == '<'
                             && (c[count - 1] == 'M' || c[count - 1] == 'm'))
      rval = KEY_MOUSE;    /* SGR mouse mode */
   else
      {
      rval = _look_up_key( c, count, modifiers);
      if( !rval)
         rval = -1;
      }
   return( rval);
}

int PDC_get_key( void)
{
   int rval = -1;
   static bool recursed = FALSE;

   if( PDC_resize_occurred)
      {
      PDC_resize_occurred = FALSE;
      return( KEY_RESIZE);
      }
   if( !recursed && _mlist_count)
      {
      _get_mouse_event( &SP->mouse_status);
      return( KEY_MOUSE);
      }
   if( check_key( &rval))
      {
      int c[MAX_COUNT], modifiers = 0;

#ifdef USE_CONIO
      if( rval == 0 || rval == 224)
         {
         int key2;

#ifdef __DJGPP__                 /* DJGPP returns kbhit() = 1 only once for */
         key2 = dmc_getch( );    /* escape sequences,  not twice as other  */
#else                            /* compilers for MS-DOS do */
         while( !check_key( &key2))
            ;
#endif
         rval = xlate_vt_codes_for_dos( rval, key2);
         return( rval);
         }

#endif
      if( rval == 27)
         {
         int count = 0;

         rval = -1;
         while( rval == -1 && count < MAX_COUNT && check_key( &c[count]))
            {
            count++;
            rval = xlate_vt_codes( c, count, &modifiers);
            if( (rval == ALT_LBRACKET || rval == ALT_O) && check_key( NULL))
               rval = -1;
            }
#ifdef LINUX_FRAMEBUFFER_PORT
         if( rval == ALT_MINUS)
            {
            PDC_cycle_font( );
            rval = -1;
            }
         if( rval == ALT_FSLASH)
            {
            PDC_rotate_font( );
            rval = -1;
            }
#ifdef USE_DRM
         if( rval == ALT_EQUAL)
            {
            PDC_cycle_display( );
            rval = -1;
            }
#endif
#endif
         if( !count)             /* Escape hit */
            rval = 27;
         count--;
         if( rval == KEY_MOUSE)
            {
            int idx, button, flags = 0, i, x, y;
            int event_type = -1;
            static int held = 0;

            if( c[1] == 'M')     /* 'traditional' mouse encoding */
               {
               x = (unsigned char)( c[3] - ' ' - 1);
               y = (unsigned char)( c[4] - ' ' - 1);
               idx = c[2];
               button = idx & 3;

               if( 3 == button)     /* need to determine which button released */
                  {
                  event_type = BUTTON_PRESSED;
                  button = 0;
                  while( button < 3 && !((held >> button) & 1))
                     button++;
                  }
               else
                  event_type = BUTTON_PRESSED;
               }
            else                 /* SGR mouse encoding */
               {
               int n_fields;
               int n_bytes;
               char tbuff[MAX_COUNT];

               assert( c[1] == '<');
               for( i = 0; i < count; i++)
                  tbuff[i] = (char)c[i];
               tbuff[count] = '\0';
               n_fields = sscanf( tbuff + 2, "%d;%d;%d%n", &idx,
                                                    &x, &y, &n_bytes);
               assert( n_fields == 3);
               assert( c[count] == 'M' || c[count] == 'm');
               assert( n_bytes == count - 2);
               button = idx & 3;
               if( 3 == n_fields)
                  {
                  event_type = ((c[count] == 'm') ? BUTTON_RELEASED : BUTTON_PRESSED);
#ifdef __HAIKU__
                  if( !release)
                     {
                     if( button == 3 || (held >> button) & 1)
                        idx |= 0x40;         /* it's actually a mouse movement */
                     }
                  else if( button < 3)
                     held &= ~(1 << button);
                  idx &= ~0x20;
#else
                  if( idx & 0x40)            /* (SGR) wheel mouse event; */
                     idx |= 0x20;            /* requires this bit set in 'traditional' encoding */
                  else if( idx & 0x20)       /* (SGR) mouse move event sets a different bit */
                     idx ^= 0x60;            /* in the traditional encoding */
#endif
                  x--;
                  y--;
                  }
               }
            if( idx & 4)
               flags |= BUTTON_SHIFT;
            if( idx & 8)
               flags |= BUTTON_ALT;
            if( idx & 16)
               flags |= BUTTON_CONTROL;
            if( (idx & 0x60) == 0x40)
               event_type = BUTTON_MOVED;
            if( (idx & 0x60) == 0x60)    /* actually mouse wheel event */
               event_type = (button ? PDC_MOUSE_WHEEL_DOWN : PDC_MOUSE_WHEEL_UP);
            if( _add_raw_mouse_event( button, event_type, flags, x, y))
               {
               bool more_mouse = TRUE;

               if( recursed)
                  return KEY_MOUSE;
               recursed = TRUE;
               while( more_mouse)
                  {
                  const long t_end = PDC_millisecs( ) + SP->mouse_wait;

                  while( !check_key( NULL) && PDC_millisecs( ) < t_end)
                     PDC_napms( 20);
                  more_mouse = (check_key( NULL) && PDC_get_key( ) == KEY_MOUSE);
                  }
               recursed = FALSE;
               }
            else if( recursed)
               return 0;
            if( _mlist_count)
               {
               _get_mouse_event( &SP->mouse_status);
               return KEY_MOUSE;
               }
            else        /* event filtered out */
               return -1;
            }
         }
      else if( (rval & 0xc0) == 0xc0)      /* start of UTF-8 */
         {
         if( !check_key( &c[0]))    /* we should get at least one */
            {                       /* continuation byte    */
            assert( 0);
            return( -1);
            }
         assert( (c[0] & 0xc0) == 0x80);
         c[0] &= 0x3f;
         if( !(rval & 0x20))      /* two-byte : U+0080 to U+07ff */
            rval = c[0] | ((rval & 0x1f) << 6);
         else
            {
            if( !check_key( &c[1]))   /* in this situation,  we should get */
               {                     /* at least one more continuation byte */
               assert( 0);
               return( -1);
               }
            assert( (c[1] & 0xc0) == 0x80);
            c[1] &= 0x3f;
            if( !(rval & 0x10))   /* three-byte : U+0800 - U+ffff */
               rval = (c[1] | (c[0] << 6) | ((rval & 0xf) << 12));
            else              /* four-byte : U+FFFF - U+10FFFF : SMP */
               {              /* (Supplemental Multilingual Plane) */
               if( !check_key( &c[2]))
                  {
                  assert( 0);
                  return( -1);
                  }
               assert( (c[2] & 0xc0) == 0x80);
#if WCHAR_MAX > 65535
               c[2] &= 0x3f;
               rval = (c[2] | (c[1] << 6) | (c[0] << 12) | ((rval & 0xf) << 18));
#endif
               }
            }
         }
      else if( rval == 127)
         rval = 8;
      else if( rval > 0 && rval < 127)
         {
               /* The following 128 bits indicate which keys require Shift */
               /* on a US keyboard with no CapsLock on.  The hack is strong */
               /* in this one.  Lower-level keyboard access might let us */
               /* avoid this problem,  _and_ allow access to other keys */
               /* that are currently filtered out before the code 'sees' them. */
         static const unsigned short shifted_keys[8] = {
                  0, 0,                   /* keys 0-31 */
                  0x0f7e, 0xd400,         /* keys 32-63 */
                  0xffff, 0xc7ff,         /* keys 64-95 */
                  0, 0x7800 };            /* keys 96-127 */

         if( rval < 32 && rval != 13 && rval != 10 && rval != 9)
            modifiers = CTL;
         if( shifted_keys[rval >> 4] & (1 << (rval & 0xf)))
            modifiers = SHF;
         }
#if defined( LINUX_FRAMEBUFFER_PORT) && defined( HAVE_MOUSE)
      SP->key_modifiers = _key_modifiers;
#else
      SP->key_modifiers = modifiers;
#endif
      }
   if( rval > 0 && rval == PDC_get_function_key( FUNCTION_KEY_ABORT))
      raise( SIGINT);
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
#ifndef LINUX_FRAMEBUFFER_PORT
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
         char tbuff[80];

         if( curr_tracking_state > 0)
            {
#ifdef HAVE_SNPRINTF
            snprintf( tbuff, sizeof( tbuff), CSI "?%dl", curr_tracking_state);
#else
            sprintf( tbuff, CSI "?%dl", curr_tracking_state);
#endif
            PDC_puts_to_stdout( tbuff);
            }
         if( tracking_state)
            {
#ifdef HAVE_SNPRINTF
            snprintf( tbuff, sizeof( tbuff), CSI "?%dh", tracking_state);
#else
            sprintf( tbuff, CSI "?%dh", tracking_state);
#endif
            PDC_puts_to_stdout( tbuff);
            }
         curr_tracking_state = tracking_state;
         PDC_doupdate( );
         }
      }
#endif      /* #ifndef LINUX_FRAMEBUFFER_PORT */
   return(  OK);
}

void PDC_set_keyboard_binary( bool on)
{
   INTENTIONALLY_UNUSED_PARAMETER( on);
   return;
}
