/* Public Domain Curses */

#include "pdcwin.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "../common/pdccolor.h"

static int min_lines = 25, max_lines = 25;
static int min_cols = 80, max_cols = 80;

static int set_default_sizes_from_registry( const int n_cols, const int n_rows,
               const int xloc, const int yloc);

/* We have a 'base' standard palette of 256 colors,  plus a true-color
cube of 16 million colors. */

#define N_COLORS 256 + 256 * 256 * 256;

   /* If PDC_MAX_MOUSE_BUTTONS is undefined,  it means the user hasn't     */
   /* gotten a current 'curses.h' in which five-button mice are supported. */
   /* To handle this gracefully,  we'll just fall back to three buttons.   */

#ifndef PDC_MAX_MOUSE_BUTTONS
   #define PDC_MAX_MOUSE_BUTTONS 3
#endif

#define WHEEL_EVENT      PDC_MAX_MOUSE_BUTTONS

/* RR: Removed statis on next line */
bool PDC_bDone = FALSE;

static int debug_printf( const char *format, ...)
{
    static bool debugging = TRUE;

    if( debugging)
    {
        const char *output_filename = getenv( "PDC_DEBUG");

        if( !output_filename)
            debugging = FALSE;       /* don't bother trying again */
        else
        {
            FILE *ofile = fopen( output_filename, "a");

            if( ofile)
            {
                va_list argptr;
                va_start( argptr, format);
                vfprintf( ofile, format, argptr);
                va_end( argptr);
                fclose( ofile);
            }
            else
            {
                printf( "Opening '%s' failed\n", output_filename);
                exit( 1);
            }
        }
    }
    return( 0);
}

static HAB PDC_hab;
static HMQ PDC_hmq;
HWND PDC_hWnd;
static int PDC_argc = 0;
static char **PDC_argv = NULL;

static void final_cleanup( void)
{
    debug_printf( "final_cleanup: SP = %p\n", SP);
    if (SP)
    {
        SWP swp;

        WinQueryWindowPos( PDC_hWnd, &swp);
        set_default_sizes_from_registry( SP->cols, SP->lines, swp.x, swp.y);
    }
#if 0 //RLC
    PDC_LOG(( "final_cleanup: freeing fonts\n"));
    PDC_transform_line( 0, 0, 0, NULL);  /* free any fonts */
    if( originally_focussed_window)
        SetForegroundWindow( originally_focussed_window);
#endif //RLC
    if( PDC_argc)
    {
        int i;

        for( i = 0; i < PDC_argc; i++)
            free( PDC_argv[i]);
        free( PDC_argv);
        PDC_argc = 0;
        PDC_argv = NULL;
    }
#if 0 //RLC
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters( 0, NULL);       /* free internal buffer */
#endif
#endif //RLC
    debug_printf( "reset foreground window\n");
}

void PDC_scr_close(void)
{
    PDC_LOG(("PDC_scr_close() - called\n"));
    final_cleanup( );
    PDC_bDone = TRUE;
}

/* NOTE that PDC_scr_free( ) is called only from delscreen( ),    */
/* which is rarely called.  It appears that most programs simply  */
/* rely on the memory getting freed when the program terminates.  */

void PDC_scr_free(void)
{
    PDC_free_palette( );
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters( 0, NULL);       /* free internal buffer */
#endif
    WinDestroyWindow( PDC_hWnd);
    WinDestroyMsgQueue( PDC_hmq);
    WinTerminate( PDC_hab);
    PDC_hWnd = NULLHANDLE;
    PDC_hmq = NULLHANDLE;
    PDC_hab = NULLHANDLE;
    ttytype[1] = 0;
}

int PDC_n_rows, PDC_n_cols;
int PDC_cxChar, PDC_cyChar, PDC_key_queue_low = 0, PDC_key_queue_high = 0;
int PDC_key_queue[KEY_QUEUE_SIZE];

   /* If the following is true,  you can enter Unicode values by hitting */
   /* Alt and holding it down while typing the value of the character on */
   /* the numeric keypad (for decimal entry);  _or_ you can hit Alt-Padplus */
   /* and then enter a hex value,  while holding down the Alt key.  In   */
   /* either case,  when you release the Alt key,  the Unicode character */
   /* is added to the queue.  For hex entry,  0-9 can come either from   */
   /* the numeric keypad or the "usual" keyboard.                        */
bool PDC_allow_numpad_unicode = TRUE;
static int numpad_unicode_value = 0;

static BOOL minimized = FALSE;

static void add_key_to_queue( const int new_key)
{
    const int new_idx = ((PDC_key_queue_high + 1) % KEY_QUEUE_SIZE);
                /* This is usually 10,  but is set to 16 if the user   */
                /* hits ALT_PADPLUS and is about to enter a hex value: */
    static int unicode_radix = 10;

    if( PDC_allow_numpad_unicode)
    {
        int digit = -1;

        if( new_key >= ALT_PAD0 && new_key <= ALT_PAD9)
            digit = new_key - ALT_PAD0;
                  /* In hex Unicode entry,  you can enter digits on both */
                  /* the numeric and "standard" keyboards :              */
        if( unicode_radix == 16 && new_key >= ALT_0 && new_key <= ALT_9)
            digit = new_key - ALT_0;
        if( unicode_radix == 16 && new_key >= ALT_A && new_key <= ALT_F)
            digit = new_key - ALT_A + 10;
        if( digit >= 0)
        {
            numpad_unicode_value = numpad_unicode_value * unicode_radix + digit;
            if (numpad_unicode_value > MAX_UNICODE - 1)
                numpad_unicode_value = MAX_UNICODE - 1;
            return;
        }
        if( new_key == ALT_PADPLUS)
        {                            /* signal to begin hex Unicode entry */
            unicode_radix = 16;
            return;
        }
    }
    unicode_radix = 10;
    if( new_key && new_key == PDC_get_function_key( FUNCTION_KEY_ABORT))
    {
        exit( -1);
    }
    else if( new_key && new_key == PDC_get_function_key( FUNCTION_KEY_ENLARGE_FONT))
    {
        PDC_font_size += 2;
        if (PDC_font_size > 40)
            PDC_font_size = 40;
        PDC_set_font_box(PDC_hWnd);
    }
    else if( new_key && new_key == PDC_get_function_key( FUNCTION_KEY_SHRINK_FONT))
    {
        PDC_font_size -= 2;
        if (PDC_font_size < 2)
            PDC_font_size = 2;
        PDC_set_font_box(PDC_hWnd);
    }
    else if( new_key && new_key == PDC_get_function_key( FUNCTION_KEY_CHOOSE_FONT))
    {
        if( PDC_choose_a_new_font(PDC_font_name, &PDC_font_size))
            PDC_set_font_box(PDC_hWnd);
    }
    else if( new_idx != PDC_key_queue_low)
    {
        PDC_key_queue[PDC_key_queue_high] = new_key;
        PDC_key_queue_high = new_idx;
    }
}

/************************************************************************
 *    Table for key code translation of function keys in keypad mode    *
 *    These values are for strict IBM keyboard compatibles only         *
 ************************************************************************/

typedef struct
{
    unsigned short normal;
    unsigned short shift;
    unsigned short control;
    unsigned short alt;
    unsigned short flags;
} KPTAB;

enum kptab_flags
{
    KP_NUMLOCK = 0x1, /* NumLock inverts shift */
    KP_SHIFT   = 0x2  /* Key is a shift key */
};

/* Index by virtual key */
static const KPTAB kptab[] =
{
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  0 */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  1  VK_BUTTON1 */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  2  VK_BUTTON2 */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  3  VK_BUTTON3 */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  4  VK_BREAK   */
    {0x08,           0x08,          0x7F,      ALT_BKSP,       0   }, /*  5  VK_BACKSPACE */
    {0x09,           KEY_BTAB,      CTL_TAB,   ALT_TAB,        0   }, /*  6  VK_TAB     */
    {0x09,           KEY_BTAB,      CTL_TAB,   ALT_TAB,        0   }, /*  7  VK_BACKTAB */
    {0x0D,           0x0D,          CTL_ENTER, ALT_ENTER,      0   }, /*  8  VK_NEWLINE */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /*  9  VK_SHIFT   HANDLED SEPARATELY */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 10  VK_CONTROL HANDLED SEPARATELY */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 11  VK_ALT */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 12  VK_ALTGRAF */
    {KEY_PAUSE,      KEY_PAUSE,     KEY_PAUSE, 0xFFFF,         0   }, /* 13  VK_PAUSE */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 14  VK_CAPSLOCK */
    {0x1B,           0x1B,          0x1B,      ALT_ESC,        0   }, /* 15  VK_ESCAPE  */
    {' ',            ' ',           ' ',       ' ',            0   }, /* 16  VK_SPACE */
    {KEY_PPAGE,      KEY_SPREVIOUS, CTL_PGUP,  ALT_PGUP,       0   }, /* 17  VK_PAGEUP */
    {KEY_NPAGE,      KEY_SNEXT,     CTL_PGDN,  ALT_PGDN,       0   }, /* 18  VK_PAGEDOWN */
    {KEY_END,        KEY_SEND,      CTL_END,   ALT_END,        0   }, /* 19  VK_END */
    {KEY_HOME,       KEY_SHOME,     CTL_HOME,  ALT_HOME,       0   }, /* 20  VK_HOME */
    {KEY_LEFT,       KEY_SLEFT,     CTL_LEFT,  ALT_LEFT,       0   }, /* 21  VK_LEFT */
    {KEY_UP,         KEY_SUP,       CTL_UP,    ALT_UP,         0   }, /* 22  VK_UP */
    {KEY_RIGHT,      KEY_SRIGHT,    CTL_RIGHT, ALT_RIGHT,      0   }, /* 23  VK_RIGHT */
    {KEY_DOWN,       KEY_SDOWN,     CTL_DOWN,  ALT_DOWN,       0   }, /* 24  VK_DOWN */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 25  VK_PRINTSCRN */
    {KEY_IC,         KEY_SIC,       CTL_INS,   ALT_INS,        0   }, /* 26  VK_INSERT */
    {KEY_DC,         KEY_SDC,       CTL_DEL,   ALT_DEL,        0   }, /* 27  VK_DELETE */
    {KEY_SCROLLLOCK, 0xFFFF,        0xFFFF,    KEY_SCROLLLOCK, 0   }, /* 28  VK_SCRLLOCK */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 29  VK_NUMLOCK */
    {0xFFFF,         0xFFFF,        0xFFFF,    ALT_PADENTER,   0   }, /* 30  VK_ENTER */
    {0xFFFF,         0xFFFF,        0xFFFF,    0xFFFF,         0   }, /* 31  VK_SYSRQ */
    {KEY_F(1),       KEY_F(13),     KEY_F(25), KEY_F(37),      0   }, /* 32  VK_F1      */
    {KEY_F(2),       KEY_F(14),     KEY_F(26), KEY_F(38),      0   }, /* 33  VK_F2      */
    {KEY_F(3),       KEY_F(15),     KEY_F(27), KEY_F(39),      0   }, /* 34  VK_F3      */
    {KEY_F(4),       KEY_F(16),     KEY_F(28), KEY_F(40),      0   }, /* 35  VK_F4      */
    {KEY_F(5),       KEY_F(17),     KEY_F(29), KEY_F(41),      0   }, /* 36  VK_F5      */
    {KEY_F(6),       KEY_F(18),     KEY_F(30), KEY_F(42),      0   }, /* 37  VK_F6      */
    {KEY_F(7),       KEY_F(19),     KEY_F(31), KEY_F(43),      0   }, /* 38  VK_F7      */
    {KEY_F(8),       KEY_F(20),     KEY_F(32), KEY_F(44),      0   }, /* 39  VK_F8      */
    {KEY_F(9),       KEY_F(21),     KEY_F(33), KEY_F(45),      0   }, /* 40  VK_F9      */
    {KEY_F(10),      KEY_F(22),     KEY_F(34), KEY_F(46),      0   }, /* 41  VK_F10     */
    {KEY_F(11),      KEY_F(23),     KEY_F(35), KEY_F(47),      0   }, /* 42  VK_F11     */
    {KEY_F(12),      KEY_F(24),     KEY_F(36), KEY_F(48),      0   }, /* 43  VK_F12     */
};
/* End of kptab[] */

/* Index by scan code */
static const KPTAB scan_kptab[] =
{
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x00 */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_ESC,          0          }, /* 0x01 ESC */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_1,            0          }, /* 0x02 '1' */
    {0xFFFF,           0xFFFF,           0x00,             ALT_2,            0          }, /* 0x03 '2' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_3,            0          }, /* 0x04 '3' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_4,            0          }, /* 0x05 '4' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_5,            0          }, /* 0x06 '5' */
    {0xFFFF,           0xFFFF,           0x1E,             ALT_6,            0          }, /* 0x07 '6' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_7,            0          }, /* 0x08 '7' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_8,            0          }, /* 0x09 '8' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_9,            0          }, /* 0x0A '9' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_0,            0          }, /* 0x0B '0' */
    {0xFFFF,           0xFFFF,           0x1F,             ALT_MINUS,        0          }, /* 0x0C '-' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_EQUAL,        0          }, /* 0x0D '=' */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x0E Backspace */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x0F Tab */
    {0xFFFF,           0xFFFF,           0x11,             ALT_Q,            0          }, /* 0x10 'q' */
    {0xFFFF,           0xFFFF,           0x17,             ALT_W,            0          }, /* 0x11 'w' */
    {0xFFFF,           0xFFFF,           0x05,             ALT_E,            0          }, /* 0x12 'e' */
    {0xFFFF,           0xFFFF,           0x12,             ALT_R,            0          }, /* 0x13 'r' */
    {0xFFFF,           0xFFFF,           0x14,             ALT_T,            0          }, /* 0x14 't' */
    {0xFFFF,           0xFFFF,           0x19,             ALT_Y,            0          }, /* 0x15 'y' */
    {0xFFFF,           0xFFFF,           0x15,             ALT_U,            0          }, /* 0x16 'u' */
    {0xFFFF,           0xFFFF,           0x09,             ALT_I,            0          }, /* 0x17 'i' */
    {0xFFFF,           0xFFFF,           0x0F,             ALT_O,            0          }, /* 0x18 'o' */
    {0xFFFF,           0xFFFF,           0x10,             ALT_P,            0          }, /* 0x19 'p' */
    {0xFFFF,           0xFFFF,           0x1B,             ALT_LBRACKET,     0          }, /* 0x1A '[' */
    {0xFFFF,           0xFFFF,           0x1D,             ALT_RBRACKET,     0          }, /* 0x1B ']' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_ENTER,        0          }, /* 0x1C Enter */
    {KEY_CONTROL_L,    KEY_CONTROL_L,    KEY_CONTROL_L,    KEY_CONTROL_L,    KP_SHIFT   }, /* 0x1D Left Ctrl */
    {0xFFFF,           0xFFFF,           0x01,             ALT_A,            0          }, /* 0x1E 'a' */
    {0xFFFF,           0xFFFF,           0x13,             ALT_S,            0          }, /* 0x1F 's' */
    {0xFFFF,           0xFFFF,           0x04,             ALT_D,            0          }, /* 0x20 'd' */
    {0xFFFF,           0xFFFF,           0x06,             ALT_F,            0          }, /* 0x21 'f' */
    {0xFFFF,           0xFFFF,           0x07,             ALT_G,            0          }, /* 0x22 'g' */
    {0xFFFF,           0xFFFF,           0x08,             ALT_H,            0          }, /* 0x23 'h' */
    {0xFFFF,           0xFFFF,           0x0A,             ALT_J,            0          }, /* 0x24 'j' */
    {0xFFFF,           0xFFFF,           0x0B,             ALT_K,            0          }, /* 0x25 'k' */
    {0xFFFF,           0xFFFF,           0x0C,             ALT_L,            0          }, /* 0x26 'l' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_SEMICOLON,    0          }, /* 0x27 ';' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_FQUOTE,       0          }, /* 0x28 "'" */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_BQUOTE,       0          }, /* 0x29 '`' */
    {KEY_SHIFT_L,      KEY_SHIFT_L,      KEY_SHIFT_L,      KEY_SHIFT_L,      KP_SHIFT   }, /* 0x2A Left Shift */
    {0xFFFF,           0xFFFF,           0x1C,             ALT_BSLASH,       0          }, /* 0x2B '\' */
    {0xFFFF,           0xFFFF,           0x1A,             ALT_Z,            0          }, /* 0x2C 'z' */
    {0xFFFF,           0xFFFF,           0x18,             ALT_X,            0          }, /* 0x2D 'x' */
    {0xFFFF,           0xFFFF,           0x03,             ALT_C,            0          }, /* 0x2E 'c' */
    {0xFFFF,           0xFFFF,           0x16,             ALT_V,            0          }, /* 0x2F 'v' */
    {0xFFFF,           0xFFFF,           0x02,             ALT_B,            0          }, /* 0x30 'b' */
    {0xFFFF,           0xFFFF,           0x0E,             ALT_N,            0          }, /* 0x31 'n' */
    {0xFFFF,           0xFFFF,           0x0D,             ALT_M,            0          }, /* 0x32 'm' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_COMMA,        0          }, /* 0x33 ',' */
    {0xFFFF,           0xFFFF,           0xFFFF,           ALT_STOP,         0          }, /* 0x34 '.' */
    {0xFFFF,           0xFFFF,           0x7F,             ALT_FSLASH,       0          }, /* 0x35 '/' */
    {KEY_SHIFT_R,      KEY_SHIFT_R,      KEY_SHIFT_R,      KEY_SHIFT_R,      KP_SHIFT   }, /* 0x36 Right Shift */
    {PADSTAR,          SHF_PADSTAR,      CTL_PADSTAR,      ALT_PADSTAR,      0          }, /* 0x37 Pad '*' */
    {KEY_ALT_L,        KEY_ALT_L,        KEY_ALT_L,        KEY_ALT_L,        KP_SHIFT   }, /* 0x38 Left Alt */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x39 ' ' */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3A Caps Lock */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3B F1 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3C F2 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3D F3 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3E F4 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x3F F5 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x40 F6 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x41 F7 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x42 F8 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x43 F9 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x44 F10 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x45 Num Lock */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x46 Scroll Lock */
    {KEY_A1,           '7',              CTL_PAD7,         ALT_PAD7,         KP_NUMLOCK }, /* 0x47 Pad '7' */
    {KEY_A2,           '8',              CTL_PAD8,         ALT_PAD8,         KP_NUMLOCK }, /* 0x48 Pad '8' */
    {KEY_A3,           '9',              CTL_PAD9,         ALT_PAD9,         KP_NUMLOCK }, /* 0x49 Pad '9' */
    {PADMINUS,         SHF_PADMINUS,     CTL_PADMINUS,     ALT_PADMINUS,     0          }, /* 0x4A Pad '-' */
    {KEY_B1,           '4',              CTL_PAD4,         ALT_PAD4,         KP_NUMLOCK }, /* 0x4B Pad '4' */
    {KEY_B2,           '5',              CTL_PAD5,         ALT_PAD5,         KP_NUMLOCK }, /* 0x4C Pad '5' */
    {KEY_B3,           '6',              CTL_PAD6,         ALT_PAD6,         KP_NUMLOCK }, /* 0x4D Pad '6' */
    {PADPLUS,          SHF_PADPLUS,      CTL_PADPLUS,      ALT_PADPLUS,      0          }, /* 0x4E Pad '+' */
    {KEY_C1,           '1',              CTL_PAD1,         ALT_PAD1,         KP_NUMLOCK }, /* 0x4F Pad '1' */
    {KEY_C2,           '2',              CTL_PAD2,         ALT_PAD2,         KP_NUMLOCK }, /* 0x50 Pad '2' */
    {KEY_C3,           '3',              CTL_PAD3,         ALT_PAD3,         KP_NUMLOCK }, /* 0x51 Pad '3' */
    {PAD0,             '0',              CTL_PAD0,         ALT_PAD0,         KP_NUMLOCK }, /* 0x52 Pad '0' */
    {PADSTOP,          '.',              CTL_PADSTOP,      ALT_PADSTOP,      0          }, /* 0x53 Pad '.' */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x54 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x55 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x56 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x57 F11 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x58 F12 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x59 */
    {PADENTER,         SHF_PADENTER,     CTL_PADENTER,     ALT_PADENTER,     0          }, /* 0x5A Pad Enter */
    {KEY_CONTROL_R,    KEY_CONTROL_R,    KEY_CONTROL_R,    KEY_CONTROL_R,    KP_SHIFT   }, /* 0x5B Right Ctrl */
    {PADSLASH,         SHF_PADSLASH,     CTL_PADSLASH,     ALT_PADSLASH,     0          }, /* 0x5C Pad '/' */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x5D Print Screen */
    {KEY_ALT_R,        KEY_ALT_R,        KEY_ALT_R,        KEY_ALT_R,        KP_SHIFT   }, /* 0x5E Right Alt */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x5F Pause/Break */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x60 Home */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x61 Up */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x62 Pg Up */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x63 Left */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x64 Right */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x65 End */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x66 Down */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x67 Pg Dn */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x68 Insert */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x69 Delete */
    {KEY_BROWSER_BACK, KEY_BROWSER_BACK, KEY_BROWSER_BACK, KEY_BROWSER_BACK, 0          }, /* 0x6A Browser Back */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x6B My Computer */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x6C */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x6D Media Folder */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x6E */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x6F */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x70 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x71 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x72 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x73 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x74 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x75 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x76 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x77 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x78 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x79 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7A */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7B */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7C Menu */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7D */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7E Left Meta */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x7F Right Meta */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x80 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x81 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x82 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x83 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x84 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x85 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x86 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x87 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x88 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x89 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x8A */
    {KEY_SEARCH,       KEY_SEARCH,       KEY_SEARCH,       KEY_SEARCH,       0          }, /* 0x8B Browser Search */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x8C */
    {KEY_BROWSER_REF,  KEY_BROWSER_REF,  KEY_BROWSER_REF,  KEY_BROWSER_REF,  0          }, /* 0x8D Browser Reload */
    {KEY_BROWSER_STOP, KEY_BROWSER_STOP, KEY_BROWSER_STOP, KEY_BROWSER_STOP, 0          }, /* 0x8E Browser Stop */
    {KEY_BROWSER_FWD,  KEY_BROWSER_FWD,  KEY_BROWSER_FWD,  KEY_BROWSER_FWD,  0          }, /* 0x8F Browser Forward */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x90 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x91 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x92 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x93 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x94 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x95 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x96 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x97 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x98 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x99 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9A */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9B */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9C */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9D */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9E */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0x9F */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA0 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA1 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA2 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA3 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA4 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA5 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA6 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA7 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA8 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xA9 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAA */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAB */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAC */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAD */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAE */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xAF */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB0 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB1 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB2 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB3 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB4 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB5 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB6 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB7 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB8 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xB9 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBA */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBB */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBC */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBD */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBE */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xBF */
    {KEY_PREV_TRACK,   KEY_PREV_TRACK,   KEY_PREV_TRACK,   KEY_PREV_TRACK,   0          }, /* 0xC0 Media Back */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC1 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC2 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC3 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC4 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC5 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC6 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC7 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xC8 */
    {KEY_NEXT_TRACK,   KEY_NEXT_TRACK,   KEY_NEXT_TRACK,   KEY_NEXT_TRACK,   0          }, /* 0xC9 Media Forward */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xCA */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xCB */
    {KEY_VOLUME_DOWN,  KEY_VOLUME_DOWN,  KEY_VOLUME_DOWN,  KEY_VOLUME_DOWN,  0          }, /* 0xCC Volume Down */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xCD */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xCE */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xCF */
    {KEY_VOLUME_MUTE,  KEY_VOLUME_MUTE,  KEY_VOLUME_MUTE,  KEY_VOLUME_MUTE,  0          }, /* 0xD0 Mute */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD1 Calculator */
    {KEY_PLAY_PAUSE,   KEY_PLAY_PAUSE,   KEY_PLAY_PAUSE,   KEY_PLAY_PAUSE,   0          }, /* 0xD2 Media Play */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD3 */
    {KEY_MEDIA_STOP,   KEY_MEDIA_STOP,   KEY_MEDIA_STOP,   KEY_MEDIA_STOP,   0          }, /* 0xD4 Media Stop */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD5 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD6 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD7 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD8 */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xD9 */
    {KEY_BROWSER_HOME, KEY_BROWSER_HOME, KEY_BROWSER_HOME, KEY_BROWSER_HOME, 0          }, /* 0xDA Browser Home */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xDB */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xDC */
    {0xFFFF,           0xFFFF,           0xFFFF,           0xFFFF,           0          }, /* 0xDD */
    {KEY_VOLUME_UP,    KEY_VOLUME_UP,    KEY_VOLUME_UP,    KEY_VOLUME_UP,    0          }, /* 0xDE Volume Up */
};
/* End of scan_kptab[] */

#ifdef PDC_WIDE
/* Tables of conversion tables for incoming characters */

static const unsigned short cp437_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

/* Code page 813 encodes the older version of ISO 8859-7, lacking the drachma sign
   (0x20AF) and the ypogegrammeni (0x037A), but adds the euro sign (0x20AC).
   This table encodes the new version of ISO 8859-7. */
static const unsigned short cp813_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x20AF, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x037A, 0x00AB, 0x00AC, 0x00AD, 0xFFFD, 0x2015,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x0387,
    0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
    0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
    0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
    0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
    0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
    0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
    0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD
};

/* Code page 819 == ISO 8859-1 (Latin-1) */
static const unsigned short cp819_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

/* The OS/2 version of code page 850 differs from the one published by the Unicode
   Consortium in that it has the euro sign (0x20AC) instead of the dotless lowercase I
   (0x0131) */
static const unsigned short cp850_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x00D7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
    0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x00F0, 0x00D0, 0x00CA, 0x00CB, 0x00C8, 0x20AC, 0x00CD, 0x00CE,
    0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x00CC, 0x2580,
    0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0x00FE,
    0x00DE, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x00AF, 0x00B4,
    0x00AD, 0x00B1, 0x2017, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
    0x00B0, 0x00A8, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0
};

/* The OS/2 version of code page 852 differs from the one published by the Unicode
   Consortium in that it has the euro sign (0x20AC) instead of the not sign (0x00AC) */
static const unsigned short cp852_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x016F, 0x0107, 0x00E7,
    0x0142, 0x00EB, 0x0150, 0x0151, 0x00EE, 0x0179, 0x00C4, 0x0106,
    0x00C9, 0x0139, 0x013A, 0x00F4, 0x00F6, 0x013D, 0x013E, 0x015A,
    0x015B, 0x00D6, 0x00DC, 0x0164, 0x0165, 0x0141, 0x00D7, 0x010D,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x0104, 0x0105, 0x017D, 0x017E,
    0x0118, 0x0119, 0x20AC, 0x017A, 0x010C, 0x015F, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x011A,
    0x015E, 0x2563, 0x2551, 0x2557, 0x255D, 0x017B, 0x017C, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x0102, 0x0103,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x0111, 0x0110, 0x010E, 0x00CB, 0x010F, 0x0147, 0x00CD, 0x00CE,
    0x011B, 0x2518, 0x250C, 0x2588, 0x2584, 0x0162, 0x016E, 0x2580,
    0x00D3, 0x00DF, 0x00D4, 0x0143, 0x0144, 0x0148, 0x0160, 0x0161,
    0x0154, 0x00DA, 0x0155, 0x0170, 0x00FD, 0x00DD, 0x0163, 0x00B4,
    0x00AD, 0x02DD, 0x02DB, 0x02C7, 0x02D8, 0x00A7, 0x00F7, 0x00B8,
    0x00B0, 0x00A8, 0x02D9, 0x0171, 0x0158, 0x0159, 0x25A0, 0x00A0
};

static const unsigned short cp855_tbl[] = {
    0x0452, 0x0402, 0x0453, 0x0403, 0x0451, 0x0401, 0x0454, 0x0404,
    0x0455, 0x0405, 0x0456, 0x0406, 0x0457, 0x0407, 0x0458, 0x0408,
    0x0459, 0x0409, 0x045A, 0x040A, 0x045B, 0x040B, 0x045C, 0x040C,
    0x045E, 0x040E, 0x045F, 0x040F, 0x044E, 0x042E, 0x044A, 0x042A,
    0x0430, 0x0410, 0x0431, 0x0411, 0x0446, 0x0426, 0x0434, 0x0414,
    0x0435, 0x0415, 0x0444, 0x0424, 0x0433, 0x0413, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x0445, 0x0425, 0x0438,
    0x0418, 0x2563, 0x2551, 0x2557, 0x255D, 0x0439, 0x0419, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x043A, 0x041A,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x043B, 0x041B, 0x043C, 0x041C, 0x043D, 0x041D, 0x043E, 0x041E,
    0x043F, 0x2518, 0x250C, 0x2588, 0x2584, 0x041F, 0x044F, 0x2580,
    0x042F, 0x0440, 0x0420, 0x0441, 0x0421, 0x0442, 0x0422, 0x0443,
    0x0423, 0x0436, 0x0416, 0x0432, 0x0412, 0x044C, 0x042C, 0x2116,
    0x00AD, 0x044B, 0x042B, 0x0437, 0x0417, 0x0448, 0x0428, 0x044D,
    0x042D, 0x0449, 0x0429, 0x0447, 0x0427, 0x00A7, 0x25A0, 0x00A0
};

static const unsigned short cp856_tbl[] = {
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0x00A3, 0xFFFD, 0x00D7, 0x20AA,
    0x200E, 0x200F, 0x202A, 0x202B, 0x202D, 0x202E, 0x202C, 0xFFFD,
    0xFFFD, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0xFFFD, 0xFFFD,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x2580,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x00B5, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x203E, 0x00B4,
    0x00AD, 0x00B1, 0x2017, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
    0x00B0, 0x00A8, 0x2022, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0
};

/* The OS/2 version of code page 857 differs from the one published by the Unicode
   Consortium in that it has the euro sign (0x20AC) in a previously undefined position */
static const unsigned short cp857_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x0131, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x0130, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x015E, 0x015F,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x011E, 0x011F,
    0x00BF, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
    0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x00BA, 0x00AA, 0x00CA, 0x00CB, 0x00C8, 0x20AC, 0x00CD, 0x00CE,
    0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x00CC, 0x2580,
    0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0xFFFD,
    0x00D7, 0x00DA, 0x00DB, 0x00D9, 0x00EC, 0x00FF, 0x00AF, 0x00B4,
    0x00AD, 0x00B1, 0xFFFD, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
    0x00B0, 0x00A8, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0
};

/* Code page 859 is a modified form of 850, with the euro sign as in the OS/2 version of
   850 and with the following substitutions:
         850:    859:
   0xAB  0x00BD  0x0153
   0xAC  0x00BC  0x0152
   0xDD  0x00A6  0x0160
   0xEF  0x00B4  0x017D
   0xF2  0x2017  undefined
   0xF3  0x00BE  0x0178
   0xF7  0x00B8  0x017E
   0xF9  0x00A8  0x0161 */
static const unsigned short cp859_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x00D7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x00AE, 0x00AC, 0x0153, 0x0152, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
    0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x00F0, 0x00D0, 0x00CA, 0x00CB, 0x00C8, 0x20AC, 0x00CD, 0x00CE,
    0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x0160, 0x00CC, 0x2580,
    0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0x00FE,
    0x00DE, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x00AF, 0x017D,
    0x00AD, 0x00B1, 0xFFFD, 0x0178, 0x00B6, 0x00A7, 0x00F7, 0x017E,
    0x00B0, 0x0161, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0
};

static const unsigned short cp860_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E3, 0x00E0, 0x00C1, 0x00E7,
    0x00EA, 0x00CA, 0x00E8, 0x00CD, 0x00D4, 0x00EC, 0x00C3, 0x00C2,
    0x00C9, 0x00C0, 0x00C8, 0x00F4, 0x00F5, 0x00F2, 0x00DA, 0x00F9,
    0x00CC, 0x00D5, 0x00DC, 0x00A2, 0x00A3, 0x00D9, 0x20A7, 0x00D3,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x00D2, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

static const unsigned short cp861_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00D0, 0x00F0, 0x00DE, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00FE, 0x00FB, 0x00DD,
    0x00FD, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00C1, 0x00CD, 0x00D3, 0x00DA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

/* The OS/2 version of code page 862 differs from that published by the Unicode
   Consortium in these positions:
      Unicode    OS/2
0x9E  0x20A7     undefined
0x9F  0x0192     0x20AA
0xA0  0x00E1     0x200E
0xA1  0x00ED     0x200F
0xA2  0x00F3     0x202A
0xA3  0x00FA     0x202B
0xA4  0x00F1     0x202D
0xA5  0x00D1     0x202E
0xA6  0x00AA     0x202C
0xA7  0x00BA     undefined
0xA8  0x00BF     undefined
0xAD  0x00A1     0x20AC
0xE6  0x00B5     0x03BC */
static const unsigned short cp862_tbl[] = {
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0x00A2, 0x00A3, 0x00A5, 0xFFFD, 0x20AA,
    0x200E, 0x200F, 0x202A, 0x202B, 0x202D, 0x202E, 0x202C, 0xFFFD,
    0xFFFD, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x20AC, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

static const unsigned short cp863_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00C2, 0x00E0, 0x00B6, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x2017, 0x00C0, 0x00A7,
    0x00C9, 0x00C8, 0x00CA, 0x00F4, 0x00CB, 0x00CF, 0x00FB, 0x00F9,
    0x00A4, 0x00D4, 0x00DC, 0x00A2, 0x00A3, 0x00D9, 0x00DB, 0x0192,
    0x00A6, 0x00B4, 0x00F3, 0x00FA, 0x00A8, 0x00B8, 0x00B3, 0x00AF,
    0x00CE, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00BE, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

/* The OS/2 version of code page 864 differs from that published by the Unicode
   Consortium in these positions:
      Unicode    OS/2
0x9F  undefined  0x200B
0xA7  undefined  0x20AC
0xD7  0xFEC1     0xFEC3
0xD8  0xFEC5     0xFEC7
0xF1  0x0651     0xFE7C */
static const unsigned short cp864_tbl[] = {
    0x00B0, 0x00B7, 0x2219, 0x221A, 0x2592, 0x2500, 0x2502, 0x253C,
    0x2524, 0x252C, 0x251C, 0x2534, 0x2510, 0x250C, 0x2514, 0x2518,
    0x03B2, 0x221E, 0x03C6, 0x00B1, 0x00BD, 0x00BC, 0x2248, 0x00AB,
    0x00BB, 0xFEF7, 0xFEF8, 0xFFFD, 0xFFFD, 0xFEFB, 0xFEFC, 0x200B,
    0x00A0, 0x00AD, 0xFE82, 0x00A3, 0x00A4, 0xFE84, 0xFFFD, 0x20AC,
    0xFE8E, 0xFE8F, 0xFE95, 0xFE99, 0x060C, 0xFE9D, 0xFEA1, 0xFEA5,
    0x0660, 0x0661, 0x0662, 0x0663, 0x0664, 0x0665, 0x0666, 0x0667,
    0x0668, 0x0669, 0xFED1, 0x061B, 0xFEB1, 0xFEB5, 0xFEB9, 0x061F,
    0x00A2, 0xFE80, 0xFE81, 0xFE83, 0xFE85, 0xFECA, 0xFE8B, 0xFE8D,
    0xFE91, 0xFE93, 0xFE97, 0xFE9B, 0xFE9F, 0xFEA3, 0xFEA7, 0xFEA9,
    0xFEAB, 0xFEAD, 0xFEAF, 0xFEB3, 0xFEB7, 0xFEBB, 0xFEBF, 0xFEC3,
    0xFEC7, 0xFECB, 0xFECF, 0x00A6, 0x00AC, 0x00F7, 0x00D7, 0xFEC9,
    0x0640, 0xFED3, 0xFED7, 0xFEDB, 0xFEDF, 0xFEE3, 0xFEE7, 0xFEEB,
    0xFEED, 0xFEEF, 0xFEF3, 0xFEBD, 0xFECC, 0xFECE, 0xFECD, 0xFEE1,
    0xFE7D, 0xFE7C, 0xFEE5, 0xFEE9, 0xFEEC, 0xFEF0, 0xFEF2, 0xFED0,
    0xFED5, 0xFEF5, 0xFEF6, 0xFEDD, 0xFED9, 0xFEF1, 0x25A0, 0xFFFD
};

static const unsigned short cp865_tbl[] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00A4,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

static const unsigned short cp866_tbl[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
    0x0401, 0x0451, 0x0404, 0x0454, 0x0407, 0x0457, 0x040E, 0x045E,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x2116, 0x00A4, 0x25A0, 0x00A0
};

/* The OS/2 version of code page 864 differs from that published by the Unicode
   Consortium in these positions:
      Unicode    OS/2
0xA7  undefined  0x20AC
0xA8  0x00B7     0x0387 */
static const unsigned short cp869_tbl[] = {
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x0386, 0x20AC,
    0x0387, 0x00AC, 0x00A6, 0x2018, 0x2019, 0x0388, 0x2015, 0x0389,
    0x038A, 0x03AA, 0x038C, 0xFFFD, 0xFFFD, 0x038E, 0x03AB, 0x00A9,
    0x038F, 0x00B2, 0x00B3, 0x03AC, 0x00A3, 0x03AD, 0x03AE, 0x03AF,
    0x03CA, 0x0390, 0x03CC, 0x03CD, 0x0391, 0x0392, 0x0393, 0x0394,
    0x0395, 0x0396, 0x0397, 0x00BD, 0x0398, 0x0399, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x039A, 0x039B, 0x039C,
    0x039D, 0x2563, 0x2551, 0x2557, 0x255D, 0x039E, 0x039F, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x03A0, 0x03A1,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x03A3,
    0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03B1, 0x03B2,
    0x03B3, 0x2518, 0x250C, 0x2588, 0x2584, 0x03B4, 0x03B5, 0x2580,
    0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD,
    0x03BE, 0x03BF, 0x03C0, 0x03C1, 0x03C3, 0x03C2, 0x03C4, 0x0384,
    0x00AD, 0x00B1, 0x03C5, 0x03C6, 0x03C7, 0x00A7, 0x03C8, 0x0385,
    0x00B0, 0x00A8, 0x03C9, 0x03CB, 0x03B0, 0x03CE, 0x25A0, 0x00A0
};

/* Code page 874 == almost ISO 8859-11 (Thai); check this one out */
/* IBM's documents give position 0xDF as U+20AC, but tests show no euro sign when this
   code page is configured. The correct mapping is unknown. */
static const unsigned short cp874_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x0E48, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
    0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
    0x0E38, 0x0E39, 0x0E3A, 0x0E49, 0x0E4A, 0x0E4B, 0x20AC, 0x0E3F,
    0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47,
    0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
    0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57,
    0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x00A2, 0x00AC, 0x00A6, 0x00A0
};

/* Code page 878 == KOI8-R */
static const unsigned short cp878_tbl[] = {
    0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524,
    0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
    0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248,
    0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
    0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556,
    0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E,
    0x255F, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565,
    0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x00A9,
    0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
    0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
    0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
    0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
    0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
    0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
    0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
    0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A
};

/* Code page 912 == ISO 8859-2 (Latin-2) */
static const unsigned short cp912_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7,
    0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
    0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7,
    0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
};

/* Code page 915 == ISO 8859-5 (Cyrillic) */
static const unsigned short cp915_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
    0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
    0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F
};

/* Code page 921 == ISO 8859-13 (Latin-7) */
static const unsigned short cp921_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019
};

/* Code page 922 == a modified ISO 8859-1 */
static const unsigned short cp922_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x0160, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x017D, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x0161, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x017E, 0x00FF
};

/* Code page 923 == ISO 8859-15 (Latin-9) */
static const unsigned short cp923_tbl[] = {
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160, 0x00A7,
    0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7,
    0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static const unsigned short cp1125_tbl[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
    0x0401, 0x0451, 0x0490, 0x0491, 0x0404, 0x0454, 0x0406, 0x0456,
    0x0407, 0x0457, 0x00F7, 0x00B1, 0x2116, 0x00A4, 0x25A0, 0x00A0
};

static const unsigned short cp1131_tbl[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
    0x0401, 0x0451, 0x0404, 0x0454, 0x0407, 0x0457, 0x040E, 0x045E,
    0x0406, 0x0456, 0x00B7, 0x00A4, 0x0490, 0x0491, 0x2219, 0x00A0
};

static const unsigned short cp1250_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021,
    0xFFFD, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
    0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
};

static const unsigned short cp1251_tbl[] = {
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

static const unsigned short cp1252_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static const unsigned short cp1253_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0xFFFD, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7,
    0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
    0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
    0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
    0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
    0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
    0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
    0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD
};

static const unsigned short cp1254_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0xFFFD, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
};

static const unsigned short cp1255_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0xFFFD, 0x00B9, 0xFFFD, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
    0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7,
    0x05B8, 0x05B9, 0xFFFD, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
    0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0x200E, 0x200F, 0xFFFD
};

static const unsigned short cp1256_tbl[] = {
    0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0xFFFD, 0x2039, 0x0152, 0x0686, 0x0698, 0xFFFD,
    0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0xFFFD, 0x203A, 0x0153, 0x200C, 0x200D, 0xFFFD,
    0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
    0xFFFD, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
    0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7,
    0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
    0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
    0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7,
    0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0xFFFD
};

static const unsigned short cp1257_tbl[] = {
    0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021,
    0xFFFD, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0xFFFD, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0xFFFD, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0xFFFD
};

static const unsigned short cp1275_tbl[] = {
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x2126, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x00A4, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xFFFD, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7
};

struct KbdUnicode {
    unsigned code_page;
    unsigned short const *map;
};

static const struct KbdUnicode kbd_unicode_tables[] = {
    { 437, cp437_tbl },
    { 813, cp813_tbl },
    { 819, cp819_tbl },
    { 850, cp850_tbl },
    { 852, cp852_tbl },
    { 855, cp855_tbl },
    { 856, cp856_tbl },
    { 857, cp857_tbl },
    { 859, cp859_tbl },
    { 860, cp860_tbl },
    { 861, cp861_tbl },
    { 862, cp862_tbl },
    { 863, cp863_tbl },
    { 864, cp864_tbl },
    { 865, cp865_tbl },
    { 866, cp866_tbl },
    { 869, cp869_tbl },
    { 874, cp874_tbl },
    { 878, cp878_tbl },
    { 912, cp912_tbl },
    { 915, cp915_tbl },
    { 921, cp921_tbl },
    { 922, cp922_tbl },
    { 923, cp923_tbl },
    { 1004, cp1252_tbl }, /* 1004 and 1252 are the same code page */
    { 1125, cp1125_tbl },
    { 1131, cp1131_tbl },
    { 1250, cp1250_tbl },
    { 1251, cp1251_tbl },
    { 1252, cp1252_tbl },
    { 1253, cp1253_tbl },
    { 1254, cp1254_tbl },
    { 1255, cp1255_tbl },
    { 1256, cp1256_tbl },
    { 1257, cp1257_tbl },
    { 1275, cp1275_tbl }
};

static unsigned short const *kbd_unicode;
#endif

/* Mouse handling is done as follows.  Windows (*) gives us a
sequence of "raw" mouse events,  which are :

      button pressed
      button released
      wheel up/down/left/right
      mouse moved

   We need to provide a sequence of "combined" mouse events,
in which presses and releases get combined into clicks,
double-clicks, and triple-clicks if the "raw" events are within
SP->mouse_wait milliseconds of each other and the mouse doesn't
move in between.  add_mouse( ) takes the "raw" events and figures
out what "combined" events should be emitted.

   If the raw event is a press or release,  we also set a timer to
trigger in SP->mouse_wait milliseconds.  When that timer event is
triggered,  it calls add_mouse( -1, -1, -1, -1), meaning "synthesize
all events and pass them to add_mouse_event_to_queue( )". Basically,  if
we hit the timeout _or_ the mouse is moved, we can send combined events
to add_mouse_event_to_queue( ).  A corresponding KEY_MOUSE event will
be added to the key queue.

   A mouse move is simply ignored if it's within the current
character cell.  (Note that ncurses does provide 'mouse move' events
even if the mouse has only moved within the character cell.)

   Also,  a note about wheel handling.  Pre-Vista,  you could just say
"the wheel went up" or "the wheel went down".  Vista introduced the possibility
that the mouse motion could be a smoothly varying quantity.  So on each
mouse move,  we add in the amount moved,  then check to see if that's
enough to trigger a wheel up/down event (or possibly several).  The idea
is that whereas before,  each movement would be 120 units (the default),
you might now get a series of 40-unit moves and should emit a wheel up/down
event on every third move.

   (*) This is not necessarily Windows-specific.  The same logic should
apply in any system where a timer can be set.  Or a "timer" could be
synthesized by storing the time of the last mouse event,  comparing
it to the current time,  and saying that if SP->mouse_wait milliseconds
have elapsed,  it's time to call add_mouse( -1, -1, -1, -1) to force
all mouse events to be output.            */

#define TIMER_ID_FOR_BLINKING 0x2000
#define TIMER_ID_FOR_MOUSE    0x2001

#define WM_CHOOSE_FONT        (WM_USER + 6)

static int add_resize_key = 1;
static int resize_limits_set = 0;

/* This flavor of Curses tries to store the window and font sizes on an
app-by-app basis.  To do this, it uses the above get_app_name( ) function, then
sets or gets a corresponding value from an OS/2 profile.  The benefit should be
that one can have one screen size/font for, say, Testcurs, while having
different settings for, say, Firework or Rain or one's own programs. */

static void get_app_name(char *name, size_t name_len)
{
    PTIB tib;
    PPIB pib;
    const char *bname;
    const char *ext;
    size_t len;
    size_t i;

    if (name_len == 0)
        return;

    DosGetInfoBlocks(&tib, &pib);

    /* Start of base name */
    if (pib->pib_pchcmd == NULL)
    {
        bname = "pdcurses";
    }
    else
    {
        bname = strrchr(pib->pib_pchcmd, '\\');
        if (bname == NULL)
            bname = pib->pib_pchcmd;
        else
            ++bname;
    }

    /* Remove extension if present */
    ext = strrchr(bname, '.');
    if (ext == NULL)
        len = strlen(bname);
    else
        len = (size_t)(ext - bname);

    /* Copy to caller-supplied array */
    snprintf(name, name_len, "%.*s", (int)len, bname);
    /* Convert to lowercase */
    for (i = 0; name[i] != '\0'; ++i)
        name[i] = tolower(name[i]);
}

static int set_default_sizes_from_registry( const int n_cols, const int n_rows,
               const int xloc, const int yloc)
{
    char buff[1024];
    char app_name[FILENAME_MAX];
    BOOL rc;

    snprintf( buff, sizeof(buff), "%dx%d,%d,%d,%d,%d;%d,%d,%d,%d:%s",
              n_cols, n_rows, PDC_font_size,
              xloc, yloc, /*menu_shown*/ 0,
              min_lines, max_lines,
              min_cols, max_cols,
              PDC_font_name);

    get_app_name( app_name, sizeof(app_name));
    rc = PrfWriteProfileString(HINI_PROFILE, (PSZ)"PDCurses", (PSZ)app_name, (PSZ)buff);

    debug_printf( "Size: %d %d; %d\n", n_cols, n_rows, rc);
    return rc;
}

static int get_default_sizes_from_registry( int *n_cols, int *n_rows,
                                     int *xloc, int *yloc)
{
    char buff[1024];
    char app_name[FILENAME_MAX];
    ULONG rc;

    get_app_name(app_name, sizeof(app_name));
    rc = PrfQueryProfileString(HINI_PROFILE, (PSZ)"PDCurses", (PSZ)app_name, (PSZ)"",
                buff, sizeof(buff));

    if (rc != 0)
    {
        int x = n_cols ? *n_cols : -1, y = n_rows ? *n_rows : -1;
        int bytes_read = 0;

        sscanf( buff, "%dx%d,%d,%d,%d,%*d;%d,%d,%d,%d:%n",
                         &x, &y, &PDC_font_size,
                         xloc, yloc, /*&menu_shown,*/
                         &min_lines, &max_lines,
                         &min_cols, &max_cols,
                         &bytes_read);
        if( bytes_read > 0 && buff[bytes_read - 1] == ':')
            snprintf(PDC_font_name, sizeof(PDC_font_name), "%s",
                    buff + bytes_read);
        if( n_cols != NULL)
            *n_cols = x;
        if( n_rows != NULL)
            *n_rows = y;
    }
    if( rc == 0)
        debug_printf( "get_default_sizes_from_registry error: %d\n",
                    WinGetLastError(PDC_hab));
    return rc != 0;
}

/*man-start**************************************************************

Resize limits
-------------

### Synopsis

    void PDC_set_resize_limits( const int new_min_lines,
                                const int new_max_lines,
                                const int new_min_cols,
                                const int new_max_cols);

### Description

   For platforms supporting resizable windows (SDLx, WinGUI, X11).  Some
   programs may be unprepared for a resize event;  for these,  calling
   this function with the max and min limits equal ensures that no
   user resizing can be done.  Other programs may require at least a
   certain number,  and/or no more than a certain number,  of columns
   and/or lines.

### Portability

   PDCurses-only function.

**man-end****************************************************************/

void PDC_set_resize_limits( const int new_min_lines, const int new_max_lines,
                  const int new_min_cols, const int new_max_cols)
{
    min_lines = max( new_min_lines, 2);
    max_lines = max( new_max_lines, min_lines);
    min_cols = max( new_min_cols, 2);
    max_cols = max( new_max_cols, min_cols);
    resize_limits_set = 1;
}

typedef void(*resize_callback_fnptr)(void);
static resize_callback_fnptr resize_callback = NULL;
void PDC_set_window_resized_callback(resize_callback_fnptr callback) {
    resize_callback = callback;
}

static void HandleSize( HWND hwnd, int n_xpixels, int n_ypixels)
{
    unsigned new_n_rows, new_n_cols;
    BOOL first_time;

    first_time = (PDC_cxChar == 0 || PDC_cyChar == 0);
    if (first_time)
        PDC_set_font_box(hwnd); /* avoid division by zero */

    /* If window has been minimized, don't mess with the size */
    if (minimized)
        return;

    new_n_rows = n_ypixels / PDC_cyChar;
    new_n_cols = n_xpixels / PDC_cxChar;
    debug_printf( "Size was %d x %d; will be %d x %d\n",
                PDC_n_rows, PDC_n_cols, new_n_rows, new_n_cols);
    SP->resized = FALSE;

    /* If the window will have a different number of rows */
    /* or columns,  we put KEY_RESIZE in the key queue.   */
    /* We don't do this if */
    /* the resizing is the result of the window being    */
    /* initialized,  or as a result of PDC_resize_screen */
    /* being called.  In the latter case,  the user      */
    /* presumably already knows the screen's been resized. */
    if( PDC_n_rows != (int)new_n_rows || PDC_n_cols != (int)new_n_cols)
    {
        PDC_n_cols = new_n_cols;
        PDC_n_rows = new_n_rows;
        if( !first_time && add_resize_key)
        {
            /* don't add a key when the window is initialized */
            add_key_to_queue( KEY_RESIZE);
            SP->resized = TRUE;
            if (resize_callback) {
                resize_callback();
            }
        }
    }

    add_resize_key = 1;
}

static void
HandlePaint(const HWND hwnd)
{
    RECTL update_rect;
    HPS hps = WinBeginPaint(hwnd, NULLHANDLE, &update_rect);
    int i;
    int num_cols = PDC_n_cols;
    int num_rows = PDC_n_rows;
    int row_min, row_max;
    int col_min, col_max;

    if (num_cols > COLS)
        num_cols = COLS;
    if (num_rows > SP->lines)
        num_rows = SP->lines;

    GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
    WinFillRect(hps, &update_rect, GpiQueryColorIndex(hps, 0, 0x00000000));

    PDC_setup_font(hps);

    PDC_pixel_to_screen(update_rect.xLeft, update_rect.yTop, &col_min, &row_min);
    PDC_pixel_to_screen(update_rect.xRight, update_rect.yBottom, &col_max, &row_max);
    if (col_min < 0)
        col_min = 0;
    if (row_min < 0)
        row_min = 0;
    if (col_max > num_cols - 1)
        col_max = num_cols - 1;
    if (row_max > num_rows - 1)
        row_max = num_rows - 1;

    for (i = row_min; i <= row_max; i++)
    {
        if (i < SP->lines && curscr->_y[i])
            PDC_transform_line_sliced( i, col_min, col_max - col_min + 1, curscr->_y[i] + col_min);
    }

    WinEndPaint(hps);
}

static void HandleTimer( void )
{
    int i;           /* see WndProc() notes */

    PDC_blink_state ^= 1;
    for( i = 0; i < SP->lines; i++)
    {
        if( curscr->_y[i])
        {
            int j = 0;
            chtype *line = curscr->_y[i];

            /* skip over starting text that isn't blinking: */
            while( j < SP->cols)
            {
                int k;

                while( j < SP->cols && !(line[j] & A_BLINK))
                    j++;
                k = j;
                while( j < SP->cols && (line[j] & A_BLINK))
                    j++;
                if( k != j)
                    PDC_transform_line_sliced( i, k, j - k, line + k);
            }
        }
    }
    if( PDC_CURSOR_IS_BLINKING && SP->cursrow < SP->lines && SP->curscol < COLS)
        PDC_transform_line_sliced( SP->cursrow, SP->curscol, 1,
                                 curscr->_y[SP->cursrow] + SP->curscol);
}

typedef struct
{
   int x, y;
   int button, action;
   int button_flags;    /* Alt, shift, ctrl */
} PDC_mouse_event;

/* As "combined" mouse events (i.e.,  clicks and double- and triple-clicks
along with the usual mouse moves,  button presses and releases,  and wheel
movements) occur,  we add them to a queue.  They are removed for each
KEY_MOUSE event from getch( ),  and SP->mouse_status is set to reflect
what the mouse was doing at that event.

Seven queued mouse events is possibly overkill.       */

#define MAX_MOUSE_QUEUE       7

static PDC_mouse_event mouse_queue[MAX_MOUSE_QUEUE];
static int n_mouse_queue = 0;

int PDC_get_mouse_event_from_queue( void)
{
    size_t i;

    if( !n_mouse_queue)
        return( -1);
    memset(&SP->mouse_status, 0, sizeof(MOUSE_STATUS));
    if( mouse_queue->action == BUTTON_MOVED)
    {
        if( mouse_queue->button < 0)
            SP->mouse_status.changes = PDC_MOUSE_MOVED;
        else
        {
            SP->mouse_status.changes = PDC_MOUSE_MOVED | (1 << mouse_queue->button);
            SP->mouse_status.button[mouse_queue->button] = BUTTON_MOVED;
        }
    }
    else
    {
        if( mouse_queue->button < PDC_MAX_MOUSE_BUTTONS)
        {
            SP->mouse_status.button[mouse_queue->button] = (short)mouse_queue->action;
            if( mouse_queue->button < 3)
               SP->mouse_status.changes = (1 << mouse_queue->button);
            else
               SP->mouse_status.changes = (0x40 << mouse_queue->button);
        }
        else if( mouse_queue->button == WHEEL_EVENT)
             SP->mouse_status.changes |= mouse_queue->action;
    }
    SP->mouse_status.x = mouse_queue->x;
    SP->mouse_status.y = mouse_queue->y;
    for (i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
        SP->mouse_status.button[i] |= mouse_queue->button_flags;
    n_mouse_queue--;
    memmove( mouse_queue, mouse_queue + 1, n_mouse_queue * sizeof( PDC_mouse_event));
    return( 0);
}

static void add_mouse_event_to_queue( const int button, const int action,
            const int x, const int y)
{
    if( x < PDC_n_cols && y < PDC_n_rows && n_mouse_queue < MAX_MOUSE_QUEUE)
    {
        int button_flags = 0;

        mouse_queue[n_mouse_queue].button = button;
        mouse_queue[n_mouse_queue].action = action;
        mouse_queue[n_mouse_queue].x = x;
        mouse_queue[n_mouse_queue].y = y;
        if( WinGetKeyState( HWND_DESKTOP, VK_MENU) & 0x8000)
            button_flags |= PDC_BUTTON_ALT;

        if( WinGetKeyState( HWND_DESKTOP, VK_SHIFT) & 0x8000)
            button_flags |= PDC_BUTTON_SHIFT;

        if( WinGetKeyState( HWND_DESKTOP, VK_CTRL) & 0x8000)
            button_flags |= PDC_BUTTON_CONTROL;
        mouse_queue[n_mouse_queue].button_flags = button_flags;
        n_mouse_queue++;
        add_key_to_queue( KEY_MOUSE);
    }
}

/* 'button_count' is zero if a button hasn't been pressed;  one if it
has been;  two if pressed/released (clicked);  three if clicked and
pressed again... all the way up to six if it's been triple-clicked. */

static int add_mouse( int button, const int action, const int x, const int y)
{
   bool flush_events_to_queue = (button == -1 || action == BUTTON_MOVED);
   static int mouse_state = 0, button_count[PDC_MAX_MOUSE_BUTTONS];
   static int prev_x, prev_y = -1;
   const bool actually_moved = (x != prev_x || y != prev_y);
   size_t i;

   if( action == BUTTON_RELEASED)
   {
       mouse_state &= ~(1 << button);
       if( !button_count[button - 1])   /* a release with no matching press */
       {
           add_mouse_event_to_queue( button - 1, BUTTON_RELEASED, x, y);
           return( 0);
       }
       else if( button_count[button - 1] & 1)
       {
           button_count[button - 1]++;
           if( button_count[button - 1] == 6)    /* triple-click completed */
               flush_events_to_queue = TRUE;
       }
   }
   else if( action == BUTTON_PRESSED && !(button_count[button - 1] & 1))
   {
      mouse_state |= (1 << button);
      button_count[button - 1]++;
   }
   if( button >= 0)
   {
      prev_x = x;
      prev_y = y;
   }
   if( action == BUTTON_MOVED)
   {
       if( !actually_moved)     /* have to move to a new character cell, */
           return( -1);         /* not just a new pixel */
       button = -1;        /* assume no buttons down */
       for( i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
           if( (mouse_state >> i) & 1)
               button = (int)i;
       if( button == -1 && !(SP->_trap_mbe & REPORT_MOUSE_POSITION))
           return( -1);
   }

   if( flush_events_to_queue)
       for( i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
           if( button_count[i])
           {
               const int events[4] = { 0, BUTTON_CLICKED,
                           BUTTON_DOUBLE_CLICKED, BUTTON_TRIPLE_CLICKED };

               assert( button_count[i] > 0 && button_count[i] < 7);
               if( button_count[i] >= 2)
                  add_mouse_event_to_queue( (int)i, events[button_count[i] / 2], prev_x, prev_y);
               if( button_count[i] & 1)
                  add_mouse_event_to_queue( (int)i, BUTTON_PRESSED, prev_x, prev_y);
               button_count[i] = 0;
           }
   if( action == BUTTON_MOVED)
      add_mouse_event_to_queue( button - 1, action, x, y);
   debug_printf( "Button %d, act %d\n", button, action);
   return( 0);
}

/* Note that there are two types of WM_TIMER timer messages.  One type
indicates that SP->mouse_wait milliseconds have elapsed since a mouse
button was pressed;  that's handled as described in the above notes.
The other type,  issued every half second,  indicates that blinking
should take place.  For these,  HandleTimer() is called (see above).

   On WM_PAINT,  we determine what parts of 'curscr' would be covered by
the update rectangle,  and run those through PDC_transform_line. */

static void HandleChar(const MPARAM wParam, const MPARAM lParam);

static MRESULT EXPENTRY WndProc (const HWND hwnd,
                          const ULONG message,
                          const MPARAM wParam,
                          const MPARAM lParam)
{
    int button = 0, action = 0;
    MRESULT rc = 0;

#if 1
    if (message != WM_TIMER)
        debug_printf("message=%08lX wParam=%p lParam=%p\n", message, wParam,lParam);
#endif
    switch (message)
    {
    case WM_CREATE:
        {
            HWND frame = WinQueryWindow(hwnd, QW_PARENT);
            HWND menu = WinWindowFromID(frame, FID_SYSMENU);
            MENUITEM item;
            MRESULT mr;
            SHORT id;

            /* Amend the system menu */
            mr = WinSendMsg(menu, MM_ITEMIDFROMPOSITION, MPFROMSHORT(0), NULL);
            id = SHORT1FROMMR(mr);
            memset(&item, 0, sizeof(item));
            WinSendMsg(menu, MM_QUERYITEM, MPFROM2SHORT(id, FALSE), &item);
            menu = item.hwndSubMenu;
            memset(&item, 0, sizeof(item));
            item.iPosition = MIT_END;
            item.afStyle = MIS_SEPARATOR;
            item.afAttribute = 0;
            item.id = 0;
            WinSendMsg(menu, MM_INSERTITEM, &item, "");
            memset(&item, 0, sizeof(item));
            item.iPosition = MIT_END;
            item.afStyle = MIS_TEXT;
            item.afAttribute = 0;
            item.id = WM_CHOOSE_FONT;
            WinSendMsg(menu, MM_INSERTITEM, &item, "Choose Font");

            WinStartTimer(PDC_hab, hwnd, TIMER_ID_FOR_BLINKING, 500);
        }
        break;

    case WM_MINMAXFRAME:
        {
            PSWP swp = wParam;
            debug_printf( "Flags = %04X\n", swp->fl);
            minimized = (swp->fl & SWP_MINIMIZE) != 0;
        }
        break;

    case WM_SIZE:
        {
            SHORT cx = SHORT1FROMMP(lParam);
            SHORT cy = SHORT2FROMMP(lParam);

            debug_printf("Size: recv x = %d y = %d\n",
                    SHORT1FROMMP(lParam), SHORT2FROMMP(lParam));
            HandleSize( hwnd, cx, cy);
        }
        break;

    case WM_COMMAND:
    case WM_SYSCOMMAND:
        switch (SHORT1FROMMP(wParam))
        {
        case WM_CHOOSE_FONT:
            if( PDC_choose_a_new_font(PDC_font_name, &PDC_font_size))
                PDC_set_font_box(hwnd);
            break;

        default:
            return WinDefWindowProc( hwnd, message, wParam, lParam);
        }
        break;

    case WM_MOUSEMOVE:
        {
            int mouse_x, mouse_y;
            PDC_pixel_to_screen(SHORT1FROMMP(wParam), SHORT2FROMMP(wParam),
                                &mouse_x, &mouse_y);

            add_mouse( 0, BUTTON_MOVED, mouse_x, mouse_y);
        }
        return 0;

    case WM_BUTTON1DOWN:
        WinSetActiveWindow(HWND_DESKTOP, hwnd);
        button = 1;
        action = BUTTON_PRESSED;
        break;

    case WM_BUTTON1UP:
        button = 1;
        action = BUTTON_RELEASED;
        break;

    case WM_BUTTON3DOWN:
        WinSetActiveWindow(HWND_DESKTOP, hwnd);
        button = 3;
        action = BUTTON_PRESSED;
        break;

    case WM_BUTTON3UP:
        button = 3;
        action = BUTTON_RELEASED;
        break;

    case WM_BUTTON2DOWN:
        WinSetActiveWindow(HWND_DESKTOP, hwnd);
        button = 2;
        action = BUTTON_PRESSED;
        break;

    case WM_BUTTON2UP:
        button = 2;
        action = BUTTON_RELEASED;
        break;

    case WM_PAINT:
        HandlePaint(hwnd);
        break;

    case WM_CHAR:
        HandleChar(wParam, lParam);
        break;

    case WM_TRANSLATEACCEL:
        /* Translate no accelerators, but pass all function keys unaltered */
        break;

    case WM_TIMER:
        switch (SHORT1FROMMP(wParam))
        {
        case TIMER_ID_FOR_MOUSE:
            WinStopTimer(PDC_hab, hwnd, TIMER_ID_FOR_MOUSE);
            add_mouse( -1, -1, -1, -1);
            break;

        case TIMER_ID_FOR_BLINKING:
            if( SP && curscr && curscr->_y)
            {
                /* blink the blinking text */
                HandleTimer();
            }
            break;

        default:
            return WinDefWindowProc( hwnd, message, wParam, lParam);
        }
        break;

    default:
        return WinDefWindowProc( hwnd, message, wParam, lParam);
    }

    if (button != 0)
    {
        int col, row;
        PDC_pixel_to_screen(SHORT1FROMMP(wParam), SHORT2FROMMP(wParam),
                            &col, &row);
        add_mouse(button, action, col, row);
        if (action == BUTTON_PRESSED)
            WinSetCapture(HWND_DESKTOP, hwnd);
        else
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
        WinStartTimer(PDC_hab, hwnd, TIMER_ID_FOR_MOUSE, SP->mouse_wait);
    }
    return rc;
}

static void HandleChar(const MPARAM wParam, const MPARAM lParam)
{
    USHORT fsflags = SHORT1FROMMP(wParam);
    UCHAR ucrepeat = CHAR3FROMMP(wParam);
    UCHAR ucscancode = CHAR4FROMMP(wParam);
    ULONG usch = SHORT1FROMMP(lParam);
    USHORT usvk = SHORT2FROMMP(lParam);
    const BOOL shift_pressed = (fsflags & KC_SHIFT) != 0;
    const BOOL ctrl_pressed = (fsflags & KC_CTRL) != 0;
    const BOOL alt_pressed = (fsflags & KC_ALT) != 0;
    const BOOL num_lock = WinGetKeyState( HWND_DESKTOP, VK_NUMLOCK) & 1;

#if 0
    printf("fsflags = 0x%04X ucrepeat = %u ucscancode = %u usch = 0x%04X usvk = 0x%04X\n",
            fsflags, ucrepeat, ucscancode, usch, usvk); fflush(stdout);
#endif

    SP->key_modifiers = 0;
    /* Save the key modifiers if required. Do this first to allow to
       detect e.g. a pressed CTRL key after a hit of NUMLOCK. */

    if( alt_pressed)
        SP->key_modifiers |= PDC_KEY_MODIFIER_ALT;

    if( shift_pressed)
        SP->key_modifiers |= PDC_KEY_MODIFIER_SHIFT;

    if( ctrl_pressed)
        SP->key_modifiers |= PDC_KEY_MODIFIER_CONTROL;

    if( num_lock)
        SP->key_modifiers |= PDC_KEY_MODIFIER_NUMLOCK;

    if( ucrepeat > 0)
        SP->key_modifiers |= PDC_KEY_MODIFIER_REPEAT;

    if ((fsflags & (KC_SCANCODE | KC_KEYUP | KC_LONEKEY)) == (KC_SCANCODE | KC_KEYUP) &&
            (ucscancode == 0x38 || ucscancode == 0x5E) &&
            numpad_unicode_value != 0)
    {
        /* Alt keys */
        /* Place on queue but not if it would match a Curses special key */
        if (numpad_unicode_value < KEY_MIN || KEY_MAX <= numpad_unicode_value)
            add_key_to_queue(numpad_unicode_value);
        numpad_unicode_value = 0;
        return;
    }

    /* Keys that are indexed by scan code */
    if ((fsflags & (KC_SCANCODE | KC_KEYUP)) == KC_SCANCODE &&
            ucscancode < sizeof(scan_kptab)/sizeof(scan_kptab[0]))
    {
        const KPTAB *kptr = &scan_kptab[ucscancode];
        /* If KP_NUMLOCK, num lock inverts shift */
        BOOL shift_inv = num_lock && (kptr->flags & KP_NUMLOCK);
        unsigned key;

        if( alt_pressed)
            key = kptr->alt;
        else if( ctrl_pressed)
            key = kptr->control;
        else if( shift_pressed ^ shift_inv)
            key = kptr->shift;
        else
            key = kptr->normal;
        if (key != 0xFFFF && !(kptr->flags & KP_SHIFT))
        {
            /* CTRL-x and ALT-x should follow the letter on French AZERTY and
               German QWERTZ layouts. Although KC_CHAR is not set, we must
               rely on usch to detect such layouts. */
            if (1 <= key && key <= 26)
            {
                if (('A' <= usch && usch <= 'Z') || ('a' <= usch && usch <= 'z'))
                    key = usch & 0x1F;
            }
            else if (ALT_A <= key && key <= ALT_Z)
            {
                if ('A' <= usch && usch <= 'Z')
                    key = ALT_A - 'A' + usch;
                else if ('a' <= usch && usch <= 'z')
                    key = ALT_A - 'a' + usch;
            }
            add_key_to_queue(key);
            return;
        }
    }

    /* Shift keys taken as single keys */
    if ((fsflags & (KC_SCANCODE | KC_KEYUP | KC_LONEKEY)) == (KC_SCANCODE | KC_KEYUP | KC_LONEKEY) &&
            ucscancode < sizeof(scan_kptab)/sizeof(scan_kptab[0]))
    {
        const KPTAB *kptr = &scan_kptab[ucscancode];
        if (kptr->flags & KP_SHIFT)
        {
            add_key_to_queue(kptr->normal);
            return;
        }
    }

    if ((fsflags & (KC_VIRTUALKEY | KC_KEYUP)) == KC_VIRTUALKEY &&
            usvk < sizeof(kptab)/sizeof(kptab[0]))
    {
        const KPTAB *kptr = &kptab[usvk];
        unsigned key;

        if( alt_pressed)
            key = kptr->alt;
        else if( ctrl_pressed)
            key = kptr->control;
        else if( shift_pressed)
            key = kptr->shift;
        else
            key = kptr->normal;
        if (key != 0xFFFF)
        {
            add_key_to_queue(key);
            return;
        }
    }

    if ((fsflags & (KC_CHAR | KC_KEYUP)) == KC_CHAR)
    {
#ifdef PDC_WIDE
        /* WinSetCp(hmq, 1200) doesn't work, and neither does translation to code
           page 1200.
           Unicode translation is handled as follows:
           * If a table according to kbd_unicode_tables is known, use it
           * Otherwise, translate to code page 1004; that way, at least up to U+00FF
             always works. */
        if (0x80 <= usch && usch <= 0xFF)
        {
            if (kbd_unicode != NULL)
                usch = kbd_unicode[usch - 0x80];
            else
                usch = WinCpTranslateChar(PDC_hab, WinQueryCp(PDC_hmq), usch, 1004);
        }
#endif
        add_key_to_queue(usch);
        return;
    }
}

/* By default,  the user cannot resize the window.  This is because
many apps don't handle KEY_RESIZE,  and one can get odd behavior
in such cases.  There are two ways around this.  If you call
PDC_set_resize_limits( ) before initwin( ),  telling WinGUI exactly how
large/small the window can be,  the window will be user-resizable.  Or
you can set ttytype[0...3] to contain the resize limits.   A call such as

PDC_set_resize_limits( 42, 42, 135, 135);

   will result in the window being fixed at 42 lines by 135 columns.

PDC_set_resize_limits( 20, 50, 70, 200);

   will mean the window can have 20 to 50 lines and 70 to 200 columns.
The user will be able to resize the window freely within those limits.
See 'newtest.c' (in the 'demos' folder) for an example. */

static int set_up_window( void)
{
    const int ID_MAIN = 1;
    ULONG window_style;
    BOOL rval;
    char WindowTitle[FILENAME_MAX];
    int n_default_columns = 80;
    int n_default_rows = 25;
    int xloc = -9999, yloc = -9999;

    PDC_hab = WinInitialize(0);
    PDC_hmq = WinCreateMsgQueue(PDC_hab, 0);
    if (PDC_hmq == NULLHANDLE)
    {
        const ULONG last_error = WinGetLastError( PDC_hab);

        debug_printf( "WinCreateMsgQueue failed: GetLastError = %lx\n", last_error);
        goto error;
    }

#ifdef PDC_WIDE
    {
        /* Select a map to convert incoming key presses to Unicode */
        ULONG code_page = WinQueryCp(PDC_hmq);
        unsigned i;
        kbd_unicode = NULL;
        for (i = 0; i < sizeof(kbd_unicode_tables)/sizeof(kbd_unicode_tables[0]); ++i)
        {
            if (kbd_unicode_tables[i].code_page == code_page)
            {
                kbd_unicode = kbd_unicode_tables[i].map;
                break;
            }
        }
    }
#endif

    rval = WinRegisterClass(PDC_hab, (PCSZ)"PDCurses.main", WndProc, CS_SIZEREDRAW, 0L);
    if( !rval)
    {
        const ULONG last_error = WinGetLastError( PDC_hab);

        debug_printf( "WinRegisterClass failed: GetLastError = %lx\n", last_error);
        goto error;
    }

    get_app_name(WindowTitle, sizeof(WindowTitle));

    /* fixed-size window:  looks "normal",  but w/o a maximize box */
    window_style = FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_TASKLIST
                 | FCF_SHELLPOSITION;
    if( min_lines != max_lines || min_cols != max_cols)
        window_style |= FCF_MAXBUTTON | FCF_SIZEBORDER;

    get_default_sizes_from_registry( &n_default_columns, &n_default_rows,
                     &xloc, &yloc);

    PDC_hWnd = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &window_style,
                                  (PCSZ)"PDCurses.main", (PCSZ)WindowTitle, 0L, 0, ID_MAIN, NULL);
    if (PDC_hWnd == NULLHANDLE)
    {
        const ULONG last_error = WinGetLastError( PDC_hab);

        debug_printf( "WinCreateStdWindow failed; GetLastError = %lx\n", last_error);
        goto error;
    }
    PDC_resize_screen(n_default_rows, n_default_columns);
    if (xloc > -9999 && yloc > -9999)
        WinSetWindowPos(PDC_hWnd, NULLHANDLE, xloc, yloc, 0, 0, SWP_MOVE);

    return 0;

error:
    WinDestroyWindow( PDC_hWnd);
    WinDestroyMsgQueue( PDC_hmq);
    WinTerminate( PDC_hab);
    PDC_hWnd = NULLHANDLE;
    PDC_hmq = NULLHANDLE;
    PDC_hab = NULLHANDLE;
    return( -1);
}

/* open the physical screen -- allocate SP, miscellaneous intialization,
   and may save the existing screen for later restoration.

   Deciding on a for-real maximum screen size has proven difficult.
   But there is really no particularly good reason to set such a maximum.
   If one does,  you get some tricky issues:  suppose the user drags the
   window to create a screen larger than MAX_LINES or MAX_COLUMNS?  My
   hope is to evade that problem by just setting those constants to be...
   well... unrealistically large.  */

#define MAX_LINES   50000
#define MAX_COLUMNS 50000

int PDC_scr_open(void)
{
    PDC_LOG(("PDC_scr_open() - called\n"));

    COLORS = N_COLORS;  /* should give this a try and see if it works! */
    if (!SP || PDC_init_palette( ))
        return ERR;

    debug_printf( "colors alloc\n");
    PDC_bDone = FALSE;
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->visibility = 0;                /* no cursor,  by default */
    SP->curscol = SP->cursrow = 0;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->termattrs = A_COLOR | WA_ITALIC | WA_UNDERLINE | WA_LEFT | WA_RIGHT |
                    WA_REVERSE | WA_STRIKEOUT | WA_TOP | WA_BLINK | WA_DIM | WA_BOLD;

    if( set_up_window( ))
    {
        fprintf( stderr, "set_up_window failed\n");
        return ERR;
    }
    debug_printf( "Back from set_up_window\n");
    while( !PDC_get_rows( )) {   /* wait for screen to be drawn and */
      QMSG qmsg;                 /* actual size to be determined    */
      if (WinPeekMsg( PDC_hab, &qmsg, NULLHANDLE, 0, 0, PM_REMOVE)) {
        WinDispatchMsg(PDC_hab, &qmsg);
      }
    }

    debug_printf( "Back from PDC_get_rows\n");
    SP->lines = PDC_get_rows();
    SP->cols = PDC_get_columns();

    if (SP->lines < 2 || SP->lines > MAX_LINES
       || SP->cols < 2 || SP->cols > MAX_COLUMNS)
    {
        fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }

/*  PDC_reset_prog_mode();   doesn't do anything anyway */
    PDC_set_function_key( FUNCTION_KEY_COPY, 0);
    debug_printf( "...we're done\n");
    return OK;
}

/* the core of resize_term() */

int PDC_resize_screen(int nlines, int ncols)
{
    if( !PDC_hWnd)      /* window hasn't been created yet;  we're */
    {                   /* specifying its size before doing so    */
        PDC_n_rows = nlines;
        PDC_n_cols = ncols;
        return OK;
    }
    SP->resized = FALSE;
    debug_printf( "Incoming: %d %d\n", nlines, ncols);
    if( nlines >= 2 && ncols >= 2 && PDC_cxChar && PDC_cyChar && PDC_hWnd)
    {
        RECTL rect, new_rect;

        /* Get the current window extent */
        WinQueryWindowRect(PDC_hWnd, &rect);

        /* Get the new window extent */
        new_rect.xLeft = 0;
        new_rect.xRight = ncols * PDC_cxChar;
        new_rect.yBottom = 0;
        new_rect.yTop = nlines * PDC_cyChar;
        WinCalcFrameRect(PDC_hWnd, &new_rect, FALSE);

        /* Resize the window */
        if (rect.xRight - rect.xLeft != new_rect.xRight - new_rect.xLeft
        ||  rect.yTop - rect.yBottom != new_rect.yTop - new_rect.yBottom)
        {
            RECTL dtop_rect;
            int xsize = new_rect.xRight - new_rect.xLeft;
            int ysize = new_rect.yTop - new_rect.yBottom;
            int xpos, ypos;

            add_resize_key = 0;

            /* Center on the desktop, but do not place the top higher than
               the top of the desktop */
            WinQueryWindowRect(HWND_DESKTOP, &dtop_rect);
            dtop_rect.xRight -= dtop_rect.xLeft;
            dtop_rect.yTop -= dtop_rect.yBottom;
            xpos = (dtop_rect.xRight - xsize) / 2;
            if (ysize > dtop_rect.yTop)
                ypos = dtop_rect.yTop - ysize;
            else
                ypos = (dtop_rect.yTop - ysize) / 2;

            WinSetWindowPos(PDC_hWnd, NULLHANDLE,
                            xpos, ypos, xsize, ysize,
                            SWP_MOVE | SWP_SIZE | SWP_SHOW);
        }
    }
    return OK;
}

void PDC_reset_prog_mode(void)
{
    PDC_LOG(("PDC_reset_prog_mode() - called.\n"));
#ifdef NOT_CURRENTLY_IN_USE
    if( PDC_bDone == FALSE && PDC_hWnd)
    {
        PDC_bDone = TRUE;
        WinSetActiveWindow( HWND_DESKTOP, PDC_hWnd);
    }
#endif
}

void PDC_reset_shell_mode(void)
{
}

void PDC_restore_screen_mode(int i)
{
   INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
   INTENTIONALLY_UNUSED_PARAMETER( i);
}

bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content( int color, int *red, int *green, int *blue)
{
    LONG col = PDC_get_palette_entry( color);

    *red   = DIVROUND(((col >>  0) & 0xFF) * 1000, 255);
    *green = DIVROUND(((col >>  8) & 0xFF) * 1000, 255);
    *blue  = DIVROUND(((col >> 16) & 0xFF) * 1000, 255);

    return OK;
}

int PDC_init_color( int color, int red, int green, int blue)
{
    const LONG new_rgb = (DIVROUND(red   * 255, 1000) <<  0)
                       | (DIVROUND(green * 255, 1000) <<  8)
                       | (DIVROUND(blue  * 255, 1000) << 16);

    PDC_set_palette_entry( color, new_rgb);
    return OK;
}

/* Convert screen to pixel coordinates */
void PDC_screen_to_pixel(int sx, int sy, RECTL *rect)
{
    RECTL client_rect;

    /* Determine origin of client rectangle */
    WinQueryWindowRect( PDC_hWnd, &client_rect);
    WinCalcFrameRect( PDC_hWnd, &client_rect, TRUE);

    /* Determine extent of character cell */
    rect->xLeft = client_rect.xLeft + sx * PDC_cxChar;
    rect->yBottom = client_rect.yBottom + (PDC_n_rows - 1 - sy) * PDC_cyChar;
    rect->xRight = rect->xLeft + PDC_cxChar;
    rect->yTop = rect->yBottom + PDC_cyChar;
}

/* Convert pixel to screen coordinates */
void PDC_pixel_to_screen(int px, int py, int *sx, int *sy)
{
    RECTL client_rect;

    /* Determine origin of client rectangle */
    WinQueryWindowRect( PDC_hWnd, &client_rect);
    WinCalcFrameRect( PDC_hWnd, &client_rect, TRUE);

    /* Determine coordinates of character cell */
    *sx = (px - client_rect.xLeft) / PDC_cxChar;
    *sy = PDC_n_rows - 1 - (py - client_rect.yBottom) / PDC_cyChar;

    /* Keep screen coordinates in bounds */
    if (*sx < 0)
        *sx = 0;
    if (*sx >= PDC_n_cols)
        *sx = PDC_n_cols - 1;
    if (*sy < 0)
        *sy = 0;
    if (*sy >= PDC_n_rows)
        *sy = PDC_n_rows - 1;
}
