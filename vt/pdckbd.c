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
#include "curspriv.h"
#include "pdcvt.h"
#include "../common/mouse.c"

#if defined( __BORLANDC__)
   #define WINDOWS_VERSION_OF_KBHIT kbhit
#else
   #define WINDOWS_VERSION_OF_KBHIT _kbhit
#endif

#ifdef __DMC__
   int dmc_getch( void);         /* see 'getch2.c' */
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
#ifdef __DMC__
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

   INTENTIONALLY_UNUSED_PARAMETER( key1);
   for( i = 0; tbl[i] && !rval; i += 2)
      if( key2 == tbl[i + 1])
         rval = tbl[i];
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

typedef struct
{
   unsigned short key_code;
   unsigned char modifiers;
   const char *xlation;
} xlate_t;

/* Short versions of the 'modifier' #defines,  to keep the columns
below reasonably short. */

#define SHF             PDC_KEY_MODIFIER_SHIFT
#define CTL             PDC_KEY_MODIFIER_CONTROL
#define ALT             PDC_KEY_MODIFIER_ALT

static const xlate_t xlates[] =  {
             { KEY_END,    0,        "OF"      },
             { KEY_HOME,   0,        "OH"      },
             { KEY_F(1),   0,        "OP"      },
             { KEY_F(2),   0,        "OQ"      },
             { KEY_F(3),   0,        "OR"      },
             { KEY_F(4),   0,        "OS"      },
             { KEY_F(1),   0,        "[11~"    },
             { KEY_F(2),   0,        "[12~"    },
             { KEY_F(3),   0,        "[13~"    },
             { KEY_F(4),   0,        "[14~"    },
             { KEY_F(17),  SHF,      "[15;2~"  },   /* shift-f5 */
             { KEY_F(29),  CTL,      "[15;5~"  },   /* ctrl-f5 */
             { KEY_F(5),   0,        "[15~"    },
             { KEY_F(18),  SHF,      "[17;2~"  },   /* shift-f6 */
             { KEY_F(30),  CTL,      "[17;5~"  },   /* ctrl-f6 */
             { KEY_F(6),   0,        "[17~"    },
             { KEY_F(19),  SHF,      "[18;2~"  },   /* shift-f7 */
             { KEY_F(31),  CTL,      "[18;5~"  },   /* ctrl-f7 */
             { KEY_F(7),   0,        "[18~"    },
             { KEY_F(20),  SHF,      "[19;2~"  },   /* shift-f8 */
             { KEY_F(32),  CTL,      "[19;5~"  },   /* ctrl-f8 */
             { KEY_F(8),   0,        "[19~"    },
             { KEY_SUP,    SHF,      "[1;2A"   },
             { KEY_SDOWN,  SHF,      "[1;2B"   },
             { KEY_SRIGHT, SHF,      "[1;2C"   },
             { KEY_SLEFT,  SHF,      "[1;2D"   },
             { KEY_SEND,   SHF,      "[1;2F"   },   /* shift-end */
             { KEY_SHOME,  SHF,      "[1;2H"   },   /* shift-home */
             { KEY_F(13),  SHF,      "[1;2P"   },   /* shift-f1 */
             { KEY_F(14),  SHF,      "[1;2Q"   },   /* shift-f2 */
             { KEY_F(15),  SHF,      "[1;2R"   },   /* shift-f3 */
             { KEY_F(16),  SHF,      "[1;2S"   },   /* shift-f4 */
             { ALT_UP,     ALT,      "[1;3A"   },
             { ALT_DOWN,   ALT,      "[1;3B"   },
             { ALT_RIGHT,  ALT,      "[1;3C"   },
             { ALT_LEFT,   ALT,      "[1;3D"   },
             { ALT_PAD5,   ALT,      "[1;3E"   },
             { ALT_END,    ALT,      "[1;3F"   },
             { ALT_HOME,   ALT,      "[1;3H"   },
             { CTL_UP,     CTL,      "[1;5A"   },
             { CTL_DOWN,   CTL,      "[1;5B"   },
             { CTL_RIGHT,  CTL,      "[1;5C"   },
             { CTL_LEFT,   CTL,      "[1;5D"   },
             { CTL_END,    CTL,      "[1;5F"   },
             { CTL_HOME,   CTL,      "[1;5H"   },
             { KEY_F(25),  CTL,      "[1;5P"   },   /* ctrl-f1 */
             { KEY_F(26),  CTL,      "[1;5Q"   },   /* ctrl-f2 */
             { KEY_F(27),  CTL,      "[1;5R"   },   /* ctrl-f3 */
             { KEY_F(28),  CTL,      "[1;5S"   },   /* ctrl-f4 */
             { KEY_HOME,   0,        "[1~"     },
             { KEY_F(21),  SHF,      "[20;2~"  },   /* shift-f9 */
             { KEY_F(33),  CTL,      "[20;5~"  },   /* ctrl-f9 */
             { KEY_F(9),   0,        "[20~"    },
             { KEY_F(22),  SHF,      "[21;2~"  },   /* shift-f10 */
             { KEY_F(34),  CTL,      "[21;5~"  },   /* ctrl-f10 */
             { KEY_F(10),  0,        "[21~"    },
             { KEY_F(23),  SHF,      "[23$"    },   /* shift-f11 on rxvt */
             { KEY_F(23),  SHF,      "[23;2~"  },   /* shift-f11 */
             { KEY_F(35),  CTL,      "[23;5~"  },   /* ctrl-f11 */
             { KEY_F(11),  0,        "[23~"    },
             { KEY_F(24),  SHF,      "[24$"    },   /* shift-f12 on rxvt */
             { KEY_F(24),  SHF,      "[24;2~"  },   /* shift-f12 */
             { KEY_F(36),  CTL,      "[24;5~"  },   /* ctrl-f12 */
             { KEY_F(12),  0,        "[24~"    },
             { KEY_F(15),  SHF,      "[25~"    },   /* shift-f3 on rxvt */
             { KEY_F(16),  SHF,      "[26~"    },   /* shift-f4 on rxvt */
             { KEY_F(17),  SHF,      "[28~"    },   /* shift-f5 on rxvt */
             { KEY_F(18),  SHF,      "[29~"    },   /* shift-f6 on rxvt */
             { KEY_SIC,    SHF,      "[2;2~"   },   /* shift-ins */
             { ALT_INS,    ALT,      "[2;3~"   },
             { CTL_INS,    CTL,      "[2;5~"   },   /* ctrl-ins */
             { KEY_IC,     0,        "[2~"     },
             { KEY_F(19),  SHF,      "[31~"    },   /* shift-f7 on rxvt */
             { KEY_F(20),  SHF,      "[32~"    },   /* shift-f8 on rxvt */
             { KEY_F(21),  SHF,      "[33~"    },   /* shift-f9 on rxvt */
             { KEY_F(22),  SHF,      "[34~"    },   /* shift-f10 on rxvt */
             { KEY_SDC,    SHF,      "[3;2~"   },   /* shift-del */
             { ALT_DEL,    ALT,      "[3;3~"   },
             { CTL_DEL,    CTL,      "[3;5~"   },
             { CTL_DEL,    CTL,      "[3;5~"   },   /* ctrl-del */
             { KEY_DC,     0,        "[3~"     },
             { KEY_END,    0,        "[4~"     },
             { KEY_SPREVIOUS, SHF,   "[5;2~"   },   /* shift-pgup */
             { ALT_PGUP,   ALT,      "[5;3~"   },
             { CTL_PGUP,   CTL,      "[5;5~"   },
             { KEY_PPAGE,  0,        "[5~"     },
             { KEY_SNEXT,  SHF,      "[6;2~"   },   /* shift-pgdn */
             { ALT_PGDN,   ALT,      "[6;3~"   },
             { CTL_PGDN,   CTL,      "[6;5~"   },
             { KEY_NPAGE,  0,        "[6~"     },
             { KEY_HOME,   0,        "[7~"     },    /* rxvt */
             { KEY_END,    0,        "[8~"     },    /* rxvt */
#ifdef __HAIKU__
             { KEY_UP,     0,        "OA"      },
             { KEY_DOWN,   0,        "OB"      },
             { KEY_RIGHT,  0,        "OC"      },
             { KEY_LEFT,   0,        "OD"      },
#else
             { KEY_UP,     0,        "[A"      },
             { KEY_DOWN,   0,        "[B"      },
             { KEY_RIGHT,  0,        "[C"      },
             { KEY_LEFT,   0,        "[D"      },
#endif
             { KEY_B2,     0,        "[E"      },
             { KEY_END,    0,        "[F"      },
             { KEY_HOME,   0,        "[H"      },
             { KEY_BTAB,   SHF,      "[Z"      },    /* Shift-Tab */
             { KEY_F(1),   0,        "[[A"     },    /* Linux console */
             { KEY_F(2),   0,        "[[B"     },
             { KEY_F(3),   0,        "[[C"     },
             { KEY_F(4),   0,        "[[D"     },
             { KEY_F(5),   0,        "[[E"     },
             };
const size_t n_keycodes = sizeof( xlates) / sizeof( xlates[0]);

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

         while( !check_key( &key2))
            ;
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
