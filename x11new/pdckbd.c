#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include "curspriv.h"
#include "pdcx11.h"

#if defined( __has_include)
   #if __has_include(<DECkeysym.h>)
      #define HAVE_DECKEYSYM_H  1
      # include <DECkeysym.h>
   #endif

   #if __has_include(<Sunkeysym.h>)
      #define HAVE_SUNKEYSYM_H  1
      # include <Sunkeysym.h>
   #endif

   #if __has_include(<XF86keysym.h>)
      #define HAVE_XF86KEYSYM_H  1
      # include <XF86keysym.h>
   #endif
#endif


static struct
{
    KeySym keycode;
    bool numkeypad;
    unsigned short normal;
    unsigned short shifted;
    unsigned short control;
    unsigned short alt;
} key_table[] =
{
/* keycode      keypad  normal       shifted       control      alt*/
 {XK_Left,      FALSE,  KEY_LEFT,    KEY_SLEFT,    CTL_LEFT,    ALT_LEFT},
 {XK_Right,     FALSE,  KEY_RIGHT,   KEY_SRIGHT,   CTL_RIGHT,   ALT_RIGHT},
 {XK_Up,        FALSE,  KEY_UP,      KEY_SUP,      CTL_UP,      ALT_UP},
 {XK_Down,      FALSE,  KEY_DOWN,    KEY_SDOWN,    CTL_DOWN,    ALT_DOWN},
 {XK_Home,      FALSE,  KEY_HOME,    KEY_SHOME,    CTL_HOME,    ALT_HOME},
/* Sun Type 4 keyboard */
 {XK_R7,        FALSE,  KEY_HOME,    KEY_SHOME,    CTL_HOME,    ALT_HOME},
 {XK_End,       FALSE,  KEY_END,     KEY_SEND,     CTL_END,     ALT_END},
/* Sun Type 4 keyboard */
 {XK_R13,       FALSE,  KEY_END,     KEY_SEND,     CTL_END,     ALT_END},
 {XK_Prior,     FALSE,  KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,    ALT_PGUP},
/* Sun Type 4 keyboard */
 {XK_R9,        FALSE,  KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,    ALT_PGUP},
 {XK_Next,      FALSE,  KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,    ALT_PGDN},
/* Sun Type 4 keyboard */
 {XK_R15,       FALSE,  KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,    ALT_PGDN},
 {XK_Insert,    FALSE,  KEY_IC,      KEY_SIC,      CTL_INS,     ALT_INS},
 {XK_Delete,    FALSE,  KEY_DC,      KEY_SDC,      CTL_DEL,     ALT_DEL},
 {XK_F1,        FALSE,  KEY_F(1),    KEY_F(13),    KEY_F(25),   KEY_F(37)},
 {XK_F2,        FALSE,  KEY_F(2),    KEY_F(14),    KEY_F(26),   KEY_F(38)},
 {XK_F3,        FALSE,  KEY_F(3),    KEY_F(15),    KEY_F(27),   KEY_F(39)},
 {XK_F4,        FALSE,  KEY_F(4),    KEY_F(16),    KEY_F(28),   KEY_F(40)},
 {XK_F5,        FALSE,  KEY_F(5),    KEY_F(17),    KEY_F(29),   KEY_F(41)},
 {XK_F6,        FALSE,  KEY_F(6),    KEY_F(18),    KEY_F(30),   KEY_F(42)},
 {XK_F7,        FALSE,  KEY_F(7),    KEY_F(19),    KEY_F(31),   KEY_F(43)},
 {XK_F8,        FALSE,  KEY_F(8),    KEY_F(20),    KEY_F(32),   KEY_F(44)},
 {XK_F9,        FALSE,  KEY_F(9),    KEY_F(21),    KEY_F(33),   KEY_F(45)},
 {XK_F10,       FALSE,  KEY_F(10),   KEY_F(22),    KEY_F(34),   KEY_F(46)},
 {XK_F11,       FALSE,  KEY_F(11),   KEY_F(23),    KEY_F(35),   KEY_F(47)},
 {XK_F12,       FALSE,  KEY_F(12),   KEY_F(24),    KEY_F(36),   KEY_F(48)},
 {XK_F13,       FALSE,  KEY_F(13),   KEY_F(25),    KEY_F(37),   KEY_F(49)},
 {XK_F14,       FALSE,  KEY_F(14),   KEY_F(26),    KEY_F(38),   KEY_F(50)},
 {XK_F15,       FALSE,  KEY_F(15),   KEY_F(27),    KEY_F(39),   KEY_F(51)},
 {XK_F16,       FALSE,  KEY_F(16),   KEY_F(28),    KEY_F(40),   KEY_F(52)},
 {XK_F17,       FALSE,  KEY_F(17),   KEY_F(29),    KEY_F(41),   KEY_F(53)},
 {XK_F18,       FALSE,  KEY_F(18),   KEY_F(30),    KEY_F(42),   KEY_F(54)},
 {XK_F19,       FALSE,  KEY_F(19),   KEY_F(31),    KEY_F(43),   KEY_F(55)},
 {XK_F20,       FALSE,  KEY_F(20),   KEY_F(32),    KEY_F(44),   KEY_F(56)},
 {XK_BackSpace, FALSE,  0x08,        0x08,         CTL_BKSP,    ALT_BKSP},
 {XK_Tab,       FALSE,  0x09,        KEY_BTAB,     CTL_TAB,     ALT_TAB},
#if defined(XK_ISO_Left_Tab)
 {XK_ISO_Left_Tab, FALSE, 0x09,      KEY_BTAB,     CTL_TAB,     ALT_TAB},
#endif
 {XK_Select,    FALSE,  KEY_SELECT,  KEY_SELECT,   KEY_SELECT,  KEY_SELECT},
 {XK_Print,     FALSE,  KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,   KEY_PRINT},
 {XK_Find,      FALSE,  KEY_FIND,    KEY_SFIND,    KEY_FIND,    KEY_FIND},
 {XK_Pause,     FALSE,  KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {XK_Clear,     FALSE,  KEY_CLEAR,   KEY_CLEAR,    KEY_CLEAR,   KEY_CLEAR},
 {XK_Cancel,    FALSE,  KEY_CANCEL,  KEY_SCANCEL,  KEY_CANCEL,  KEY_CANCEL},
 {XK_Break,     FALSE,  KEY_BREAK,   KEY_BREAK,    KEY_BREAK,   KEY_BREAK},
 {XK_Help,      FALSE,  KEY_HELP,    KEY_SHELP,    KEY_LHELP,   KEY_HELP},
 {XK_L4,        FALSE,  KEY_UNDO,    KEY_SUNDO,    KEY_UNDO,    KEY_UNDO},
 {XK_L6,        FALSE,  KEY_COPY,    KEY_SCOPY,    KEY_COPY,    KEY_COPY},
 {XK_L9,        FALSE,  KEY_FIND,    KEY_SFIND,    KEY_FIND,    KEY_FIND},
 {XK_Menu,      FALSE,  KEY_OPTIONS, KEY_SOPTIONS, KEY_OPTIONS, KEY_OPTIONS},
 {XK_Super_R,   FALSE,  KEY_COMMAND, KEY_SCOMMAND, KEY_COMMAND, KEY_COMMAND},
 {XK_Super_L,   FALSE,  KEY_COMMAND, KEY_SCOMMAND, KEY_COMMAND, KEY_COMMAND},
 {XK_Undo,      FALSE,  KEY_UNDO,    KEY_SUNDO,    KEY_UNDO,    KEY_UNDO},
 {XK_Redo,      FALSE,  KEY_REDO,    KEY_SREDO,    KEY_REDO,    KEY_REDO},
#ifdef HAVE_SUNKEYSYM_H
 {SunXK_F36,    FALSE,  KEY_F(41),   KEY_F(43),    KEY_F(45),   KEY_F(47)},
 {SunXK_F37,    FALSE,  KEY_F(42),   KEY_F(44),    KEY_F(46),   KEY_F(48)},
#endif
#ifdef HAVE_DECKEYSYM_H
 {DXK_Remove,   FALSE,  KEY_DC,      KEY_SDC,      CTL_DEL,     ALT_DEL},
#endif
 {XK_Escape,    FALSE,  0x1B,        0x1B,         0x1B,        ALT_ESC},
 {XK_KP_Enter,  TRUE,   PADENTER,    PADENTER,     CTL_PADENTER,ALT_PADENTER},
 {XK_KP_Add,    TRUE,   PADPLUS,     '+',          CTL_PADPLUS, ALT_PADPLUS},
 {XK_KP_Subtract,TRUE,  PADMINUS,    '-',          CTL_PADMINUS,ALT_PADMINUS},
 {XK_KP_Multiply,TRUE,  PADSTAR,     '*',          CTL_PADSTAR, ALT_PADSTAR},
/* Sun Type 4 keyboard */
 {XK_R6,        TRUE,   PADSTAR,     '*',          CTL_PADSTAR, ALT_PADSTAR},
 {XK_KP_Divide, TRUE,   PADSLASH,    '/',          CTL_PADSLASH,ALT_PADSLASH},
/* Sun Type 4 keyboard */
 {XK_R5,        TRUE,   PADSLASH,    '/',          CTL_PADSLASH,ALT_PADSLASH},
 {XK_KP_Decimal,TRUE,   PADSTOP,     '.',          CTL_PADSTOP, ALT_PADSTOP},
 {XK_KP_0,      TRUE,   PAD0,        '0',          CTL_PAD0,    ALT_PAD0},
 {XK_KP_1,      TRUE,   KEY_C1,      '1',          CTL_PAD1,    ALT_PAD1},
 {XK_KP_2,      TRUE,   KEY_C2,      '2',          CTL_PAD2,    ALT_PAD2},
 {XK_KP_3,      TRUE,   KEY_C3,      '3',          CTL_PAD3,    ALT_PAD3},
 {XK_KP_4,      TRUE,   KEY_B1,      '4',          CTL_PAD4,    ALT_PAD4},
 {XK_KP_5,      TRUE,   KEY_B2,      '5',          CTL_PAD5,    ALT_PAD5},
/* Sun Type 4 keyboard */
 {XK_R11,       TRUE,   KEY_B2,      '5',          CTL_PAD5,    ALT_PAD5},
 {XK_KP_6,      TRUE,   KEY_B3,      '6',          CTL_PAD6,    ALT_PAD6},
 {XK_KP_7,      TRUE,   KEY_A1,      '7',          CTL_PAD7,    ALT_PAD7},
 {XK_KP_8,      TRUE,   KEY_A2,      '8',          CTL_PAD8,    ALT_PAD8},
 {XK_KP_9,      TRUE,   KEY_A3,      '9',          CTL_PAD9,    ALT_PAD9},
/* the following added to support Sun Type 5 keyboards */
 {XK_F21,       FALSE,  KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {XK_F22,       FALSE,  KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,   KEY_PRINT},
 {XK_F24,       TRUE,   PADMINUS,    '-',          CTL_PADMINUS,ALT_PADMINUS},
/* Sun Type 4 keyboard */
 {XK_F25,       TRUE,   PADSLASH,    '/',          CTL_PADSLASH,ALT_PADSLASH},
/* Sun Type 4 keyboard */
 {XK_F26,       TRUE,   PADSTAR,     '*',          CTL_PADSTAR, ALT_PADSTAR},
 {XK_F27,       TRUE,   KEY_A1,      '7',          CTL_PAD7,    ALT_PAD7},
 {XK_F29,       TRUE,   KEY_A3,      '9',          CTL_PAD9,    ALT_PAD9},
 {XK_F31,       TRUE,   KEY_B2,      '5',          CTL_PAD5,    ALT_PAD5},
 {XK_F35,       TRUE,   KEY_C3,      '3',          CTL_PAD3,    ALT_PAD3},
#ifdef XK_KP_Delete
 {XK_KP_Delete, TRUE,   PADSTOP,     '.',          CTL_PADSTOP, ALT_PADSTOP},
#endif
#ifdef XK_KP_Insert
 {XK_KP_Insert, TRUE,   PAD0,        '0',          CTL_PAD0,    ALT_PAD0},
#endif
#ifdef XK_KP_End
 {XK_KP_End,    TRUE,   KEY_C1,      '1',          CTL_PAD1,    ALT_PAD1},
#endif
#ifdef XK_KP_Down
 {XK_KP_Down,   TRUE,   KEY_C2,      '2',          CTL_PAD2,    ALT_PAD2},
#endif
#ifdef XK_KP_Next
 {XK_KP_Next,   TRUE,   KEY_C3,      '3',          CTL_PAD3,    ALT_PAD3},
#endif
#ifdef XK_KP_Left
 {XK_KP_Left,   TRUE,   KEY_B1,      '4',          CTL_PAD4,    ALT_PAD4},
#endif
#ifdef XK_KP_Begin
 {XK_KP_Begin,  TRUE,   KEY_B2,      '5',          CTL_PAD5,    ALT_PAD5},
#endif
#ifdef XK_KP_Right
 {XK_KP_Right,  TRUE,   KEY_B3,      '6',          CTL_PAD6,    ALT_PAD6},
#endif
#ifdef XK_KP_Home
 {XK_KP_Home,   TRUE,   KEY_A1,      '7',          CTL_PAD7,    ALT_PAD7},
#endif
#ifdef XK_KP_Up
 {XK_KP_Up,     TRUE,   KEY_A2,      '8',          CTL_PAD8,    ALT_PAD8},
#endif
#ifdef XK_KP_Prior
 {XK_KP_Prior,  TRUE,   KEY_A3,      '9',          CTL_PAD9,    ALT_PAD9},
#endif

#ifdef XF86XK_Back
 {XF86XK_Back,  FALSE,  KEY_BROWSER_BACK, KEY_BROWSER_BACK,
                        KEY_BROWSER_BACK, KEY_BROWSER_BACK },
#endif

#ifdef XF86XK_Forward
 {XF86XK_Forward, FALSE,  KEY_BROWSER_FWD, KEY_BROWSER_FWD,
                          KEY_BROWSER_FWD, KEY_BROWSER_FWD },
#endif

#ifdef XF86XK_Reload
 {XF86XK_Reload, FALSE,  KEY_BROWSER_REF, KEY_BROWSER_REF,
                         KEY_BROWSER_REF, KEY_BROWSER_REF },
#endif

#ifdef XF86XK_Search
 {XF86XK_Search, FALSE,  KEY_SEARCH, KEY_SEARCH,
                         KEY_SEARCH, KEY_SEARCH },
#endif

#ifdef XF86XK_Favorites
 {XF86XK_Favorites, FALSE,  KEY_FAVORITES, KEY_FAVORITES,
                            KEY_FAVORITES, KEY_FAVORITES },
#endif

#ifdef XF86XK_AudioPlay
 {XF86XK_AudioPlay, FALSE,  KEY_PLAY_PAUSE, KEY_PLAY_PAUSE,
                            KEY_PLAY_PAUSE, KEY_PLAY_PAUSE },
#endif

#ifdef XF86XK_AudioStop
 {XF86XK_AudioStop, FALSE,  KEY_MEDIA_STOP, KEY_MEDIA_STOP,
                            KEY_MEDIA_STOP, KEY_MEDIA_STOP },
#endif

#ifdef XF86XK_AudioPrev
 {XF86XK_AudioPrev, FALSE,  KEY_PREV_TRACK, KEY_PREV_TRACK,
                            KEY_PREV_TRACK, KEY_PREV_TRACK },
#endif

#ifdef XF86XK_AudioNext
 {XF86XK_AudioNext, FALSE,  KEY_NEXT_TRACK, KEY_NEXT_TRACK,
                            KEY_NEXT_TRACK, KEY_NEXT_TRACK },
#endif

#ifdef XF86XK_AudioMute
 {XF86XK_AudioMute, FALSE,  KEY_VOLUME_MUTE, KEY_VOLUME_MUTE,
                            KEY_VOLUME_MUTE, KEY_VOLUME_MUTE },
#endif

#ifdef XF86XK_AudioLowerVolume
 {XF86XK_AudioLowerVolume, FALSE,  KEY_VOLUME_DOWN, KEY_VOLUME_DOWN,
                                   KEY_VOLUME_DOWN, KEY_VOLUME_DOWN },
#endif

#ifdef XF86XK_AudioRaiseVolume
 {XF86XK_AudioRaiseVolume, FALSE,  KEY_VOLUME_UP, KEY_VOLUME_UP,
                                   KEY_VOLUME_UP, KEY_VOLUME_UP },
#endif

#ifdef XF86XK_Tools
 {XF86XK_Tools,     FALSE,  KEY_MEDIA_SELECT, KEY_MEDIA_SELECT,
                            KEY_MEDIA_SELECT, KEY_MEDIA_SELECT },
#endif

#ifdef XF86XK_Save
 {XF86XK_Save,        FALSE,  KEY_SAVE, KEY_SSAVE, KEY_SAVE, KEY_SAVE },
#endif

#ifdef XF86XK_Send
 {XF86XK_Send,        FALSE,  KEY_SEND, KEY_SEND, KEY_SEND, KEY_SEND },
#endif

#ifdef XF86XK_Close
 {XF86XK_Close,        FALSE,  KEY_CLOSE, KEY_CLOSE, KEY_CLOSE, KEY_CLOSE },
#endif

#ifdef XF86XK_Open
 {XF86XK_Open,         FALSE,  KEY_OPEN, KEY_OPEN, KEY_OPEN, KEY_OPEN },
#endif

#ifdef XF86XK_Launch1
 {XF86XK_Launch1,      FALSE, KEY_LAUNCH_APP1, KEY_LAUNCH_APP1, KEY_LAUNCH_APP1, KEY_LAUNCH_APP1 },
#endif

#ifdef XF86XK_Launch2
 {XF86XK_Launch2,      FALSE, KEY_LAUNCH_APP2, KEY_LAUNCH_APP2, KEY_LAUNCH_APP2, KEY_LAUNCH_APP2 },
#endif

#ifdef XF86XK_Launch3
 {XF86XK_Launch3,      FALSE, KEY_LAUNCH_APP3, KEY_LAUNCH_APP3, KEY_LAUNCH_APP3, KEY_LAUNCH_APP3 },
#endif

#ifdef XF86XK_Launch4
 {XF86XK_Launch4,      FALSE, KEY_LAUNCH_APP4, KEY_LAUNCH_APP4, KEY_LAUNCH_APP4, KEY_LAUNCH_APP4 },
#endif

#ifdef XF86XK_Launch5
 {XF86XK_Launch5,      FALSE, KEY_LAUNCH_APP5, KEY_LAUNCH_APP5, KEY_LAUNCH_APP5, KEY_LAUNCH_APP5 },
#endif

#ifdef XF86XK_Launch6
 {XF86XK_Launch6,      FALSE, KEY_LAUNCH_APP6, KEY_LAUNCH_APP6, KEY_LAUNCH_APP6, KEY_LAUNCH_APP6 },
#endif

#ifdef XF86XK_Launch7
 {XF86XK_Launch7,      FALSE, KEY_LAUNCH_APP7, KEY_LAUNCH_APP7, KEY_LAUNCH_APP7, KEY_LAUNCH_APP7 },
#endif

#ifdef XF86XK_Launch8
 {XF86XK_Launch8,      FALSE, KEY_LAUNCH_APP8, KEY_LAUNCH_APP8, KEY_LAUNCH_APP8, KEY_LAUNCH_APP8 },
#endif

#ifdef XF86XK_Launch9
 {XF86XK_Launch9,      FALSE, KEY_LAUNCH_APP9, KEY_LAUNCH_APP9, KEY_LAUNCH_APP9, KEY_LAUNCH_APP9 },
#endif

#ifdef XF86XK_Launch10
 {XF86XK_Launch10,      FALSE, KEY_LAUNCH_APP10, KEY_LAUNCH_APP10, KEY_LAUNCH_APP10, KEY_LAUNCH_APP10 },
#endif

 {0,            0,      0,           0,            0,           0}
};

static XIC _xic;
static XIM _xim;

#define QUEUE_SIZE 10

static int key_queue[QUEUE_SIZE];
static int queue_low, queue_high;

static void add_to_queue( const int c)
{
   if( (queue_high - queue_low + QUEUE_SIZE + 1) % QUEUE_SIZE)
      {
      key_queue[queue_high++] = c;
      if( queue_high == QUEUE_SIZE)
         queue_high = 0;
      }
}

static int xlate_mouse_mask( const unsigned xevent_state)
{
   int rval = 0;

   if( xevent_state & Mod1Mask)
      rval |= PDC_BUTTON_ALT;
   else if( xevent_state & ShiftMask)
      rval |= PDC_BUTTON_SHIFT;
   if( xevent_state & ControlMask)
      rval |= PDC_BUTTON_CONTROL;
   return( rval);
}

#ifdef NOT_YET
static int _modifier_key( KeySym *key)
{
   const KeySym keys_in[6] = { XK_Shift_L, XK_Shift_R, XK_Control_L, XK_Control_R,
                        XK_Alt_L, XK_Alt_R };
   const KeySym keys_out[6] = { KEY_SHIFT_L, KEY_SHIFT_R, KEY_CONTROL_L, KEY_CONTROL_R,
                        KEY_ALT_L, KEY_ALT_R };
   size_t i;

   for( i = 0; i < sizeof( keys_in) / sizeof( keys_in[0]); i++)
      if( *key == keys_in[i])
         {
         *key = keys_out[i];
         return( i / 2);
         }
   return( -1);
}
#endif

static bool check_key( int *c)
{
   XEvent report;
   bool rval = false, ongoing_mouse_event = false;

   if( !_xim)
      _xim = XOpenIM( dis, NULL, NULL, NULL);
   assert( _xim);
   if( !_xic)
       _xic = XCreateIC( _xim, XNInputStyle,
                            XIMPreeditNothing | XIMStatusNothing,
                            XNClientWindow, win, NULL);
   assert( _xic);
   while( queue_low == queue_high && XPending( dis))
      {
      PDC_check_for_blinking( );
      XNextEvent( dis, &report);
      switch (report.type)
         {
         case Expose:
            {
            XWindowAttributes attrib;
            int new_cols, new_rows;

            XGetWindowAttributes( dis, win, &attrib);
            new_cols = attrib.width / PDC_font_width;
            new_rows = attrib.height / PDC_font_height;
            if( new_cols != PDC_cols || new_rows != PDC_rows)
               {
               bool resize_not_in_queue = true;
               int i;

               PDC_resize_screen( new_rows, new_cols);
               for( i = queue_low; i != queue_high; i = (i + 1) % QUEUE_SIZE)
                  if( key_queue[i] == KEY_RESIZE)
                     resize_not_in_queue = false;
               if( resize_not_in_queue)
                  {
                  SP->resized = TRUE;
                  add_to_queue( KEY_RESIZE);
                  }
               }
            else
               {
               touchwin(curscr);
               wrefresh(curscr);
               }
            }
            break;
         case KeyPress:
            {
            int i, key_to_add = -1;
            KeySym key;
            Status status;
            int count;
            wchar_t buff[2];

            count = XwcLookupString( _xic, &(report.xkey), buff, 1,
                            &key, &status);
            if( count)
               key = *buff;
            if( key > 0 && key < 0xff)
               {
               if( report.xkey.state & Mod1Mask)
                   {
                   if (key >= 'A' && key <= 'Z')
                       key += ALT_A - 'A';
                   else if (key >= 'a' && key <= 'z')
                       key += ALT_A - 'a';
                   else if (key >= '0' && key <= '9')
                       key += ALT_0 - '0';
                   }
               if( key == 3 && !SP->raw_inp)
                   exit( 0);
               key_to_add = (int)key;
               }
            else if( key >= XK_BackSpace && key <= XK_Escape)
               key_to_add = (int)( key & 0xff);
            else for( i = 0; (size_t)i < sizeof( key_table) / sizeof( key_table[0]); i++)
               if( key_table[i].keycode == key)
                  {
                  if ((report.xkey.state & ShiftMask) ||
                      (key_table[i].numkeypad &&
                      (report.xkey.state & Mod2Mask)))
                      key = key_table[i].shifted;
                  else if (report.xkey.state & ControlMask)
                      key = key_table[i].control;
                  else if (report.xkey.state & Mod1Mask)
                      key = key_table[i].alt;
                  else
                      key = key_table[i].normal;
                  key_to_add = (int)key;
                  }
            if( key_to_add > -1)
               {
               add_to_queue( key_to_add);
               SP->key_modifiers = 0;
               if( report.xkey.state & Mod2Mask)
                   SP->key_modifiers |= PDC_KEY_MODIFIER_NUMLOCK;

               /* 0x01: shift modifier */

               if( report.xkey.state & ShiftMask)
                   SP->key_modifiers |= PDC_KEY_MODIFIER_SHIFT;

               /* 0x04: control modifier */

               if( report.xkey.state & ControlMask)
                   SP->key_modifiers |= PDC_KEY_MODIFIER_CONTROL;

               /* 0x08: usually, alt modifier */

               if( report.xkey.state & Mod1Mask)
                   SP->key_modifiers |= PDC_KEY_MODIFIER_ALT;
               }
            }
            break;
         case ButtonPress:
         case ButtonRelease:
            {
            const int button_idx = report.xbutton.button - 1;

            if( !ongoing_mouse_event)
               memset( &SP->mouse_status, 0, sizeof( MOUSE_STATUS));
            ongoing_mouse_event = true;
            if( button_idx >=0 && button_idx < PDC_MAX_MOUSE_BUTTONS)
               {
               int state = SP->mouse_status.button[button_idx] & BUTTON_ACTION_MASK;

               if( report.type == ButtonPress)
                  switch( state)
                     {
                     case 0:
                        state = BUTTON_PRESSED;
                        break;
                     case BUTTON_CLICKED:
                        state = BUTTON_DOUBLE_CLICKED;
                        break;
                     case BUTTON_DOUBLE_CLICKED:
                        state = BUTTON_TRIPLE_CLICKED;
                        break;
                     default:
                        break;
                     }
               else        /* for release events... */
                  switch( state)
                     {
                     case 0:
                        state = BUTTON_RELEASED;
                        add_to_queue( KEY_MOUSE);
                        ongoing_mouse_event = false;
                        break;
                     case BUTTON_TRIPLE_CLICKED:
                        add_to_queue( KEY_MOUSE);
                        ongoing_mouse_event = false;
                        break;
                     case BUTTON_PRESSED:
                        state = BUTTON_CLICKED;
                        break;
                     case BUTTON_DOUBLE_CLICKED:
                        break;      /* do nothing,  intentionally */
                     default:
                        break;
                     }
               SP->mouse_status.button[button_idx] = state | xlate_mouse_mask( report.xkey.state);
               }
            SP->mouse_status.changes = (1 << button_idx);
            SP->mouse_status.x = report.xbutton.x / PDC_font_width;
            SP->mouse_status.y = report.xbutton.y / PDC_font_height;
            }
            break;
         case ClientMessage:
            if( (Atom)report.xclient.data.l[0] == wmDeleteMessage)
               {
               PDC_scr_free( );
               exit( 0);
               }
            break;
         case MotionNotify:
            {
            const int x = report.xbutton.x / PDC_font_width;
            const int y = report.xbutton.y / PDC_font_height;

            if( x != SP->mouse_status.x || y != SP->mouse_status.y)
               if( !ongoing_mouse_event)
                  {
                  int i, button = -1;
                  bool report_this = false;

                  for( i = 0; i < 3; i++)
                     {
                     const int action = SP->mouse_status.button[i] & BUTTON_ACTION_MASK;

                     if( action == BUTTON_PRESSED || action == BUTTON_MOVED)
                        button = i;
                     }
                  if( SP->_trap_mbe & REPORT_MOUSE_POSITION)
                     report_this = true;
                  if( button >= 0)
                     if( SP->_trap_mbe & PDC_SHIFTED_BUTTON( BUTTON1_MOVED, button + 1))
                        report_this = true;
                  if( report_this)
                     {
                     int report_event = PDC_MOUSE_MOVED;

                     if( button >= 0)
                        {
                        report_event |= (1 << button);
                        SP->mouse_status.button[button] = BUTTON_MOVED | xlate_mouse_mask( report.xkey.state);
                        }
                     SP->mouse_status.changes = report_event;
                     SP->mouse_status.x = x;
                     SP->mouse_status.y = y;
                     add_to_queue( KEY_MOUSE);
                     }
                  }
            }
            ongoing_mouse_event = false;
            break;
         case ConfigureNotify:
            break;
         case MapNotify :
            break;
         default:
            break;
         }
      if( ongoing_mouse_event)    /* wait for a possible press or release event */
         {
         const long t_end = PDC_millisecs( ) + SP->mouse_wait;
         long t;

         while( (t = PDC_millisecs( )) < t_end && !XPending( dis))
            napms( t_end - t < 20L ? (int)( t_end - t) : 20);
         if( !XPending( dis))
            add_to_queue( KEY_MOUSE);
         }
      }
   if( queue_low != queue_high)
      {
      rval = true;
      if( c)
         {
         *c = key_queue[queue_low++];
         if( queue_low == QUEUE_SIZE)
            queue_low = 0;
         }
      }
   return rval;
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

int PDC_get_key( void)
{
   int rval = -1;

   check_key( &rval);
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

int PDC_mouse_set( void)
{
   return(  OK);
}

void PDC_set_keyboard_binary( bool on)
{
   INTENTIONALLY_UNUSED_PARAMETER( on);
   return;
}

void PDC_free_xim_xic( void)
{
    assert( _xic);
    if( _xic)
        XDestroyIC( _xic);
    assert( _xim);
    if( _xim)
        XCloseIM( _xim);
    _xic = 0;
    _xim = NULL;
}
