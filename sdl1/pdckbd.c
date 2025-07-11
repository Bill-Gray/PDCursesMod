/* PDCurses */

#include "pdcsdl.h"

#include <string.h>

static SDL_Event event;
static SDLKey oldkey;
static MOUSE_STATUS old_mouse_status;

static const struct
{
    SDLKey keycode;
    bool numkeypad;
    unsigned short normal;
    unsigned short shifted;
    unsigned short control;
    unsigned short alt;
} key_table[] =
{
/* keycode   keypad   normal        shifted      control   alt*/
 {SDLK_LEFT,   FALSE,   KEY_LEFT,    KEY_SLEFT,    CTL_LEFT,   ALT_LEFT},
 {SDLK_RIGHT,   FALSE,   KEY_RIGHT,   KEY_SRIGHT,   CTL_RIGHT,   ALT_RIGHT},
 {SDLK_UP,   FALSE,   KEY_UP,      KEY_SUP,      CTL_UP,   ALT_UP},
 {SDLK_DOWN,   FALSE,   KEY_DOWN,    KEY_SDOWN,    CTL_DOWN,   ALT_DOWN},
 {SDLK_HOME,   FALSE,   KEY_HOME,    KEY_SHOME,    CTL_HOME,   ALT_HOME},
 {SDLK_END,   FALSE,   KEY_END,     KEY_SEND,      CTL_END,   ALT_END},
 {SDLK_PAGEUP,   FALSE,   KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,   ALT_PGUP},
 {SDLK_PAGEDOWN,FALSE,   KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,   ALT_PGDN},
 {SDLK_INSERT,   FALSE,   KEY_IC,      KEY_SIC,      CTL_INS,   ALT_INS},
 {SDLK_DELETE,   FALSE,   KEY_DC,      KEY_SDC,      CTL_DEL,   ALT_DEL},
 {SDLK_F1,   FALSE,   KEY_F(1),    KEY_F(13),    KEY_F(25),   KEY_F(37)},
 {SDLK_F2,   FALSE,   KEY_F(2),    KEY_F(14),    KEY_F(26),   KEY_F(38)},
 {SDLK_F3,   FALSE,   KEY_F(3),    KEY_F(15),    KEY_F(27),   KEY_F(39)},
 {SDLK_F4,   FALSE,   KEY_F(4),    KEY_F(16),    KEY_F(28),   KEY_F(40)},
 {SDLK_F5,   FALSE,   KEY_F(5),    KEY_F(17),    KEY_F(29),   KEY_F(41)},
 {SDLK_F6,   FALSE,   KEY_F(6),    KEY_F(18),    KEY_F(30),   KEY_F(42)},
 {SDLK_F7,   FALSE,   KEY_F(7),    KEY_F(19),    KEY_F(31),   KEY_F(43)},
 {SDLK_F8,   FALSE,   KEY_F(8),    KEY_F(20),    KEY_F(32),   KEY_F(44)},
 {SDLK_F9,   FALSE,   KEY_F(9),    KEY_F(21),    KEY_F(33),   KEY_F(45)},
 {SDLK_F10,   FALSE,   KEY_F(10),   KEY_F(22),    KEY_F(34),   KEY_F(46)},
 {SDLK_F11,   FALSE,   KEY_F(11),   KEY_F(23),    KEY_F(35),   KEY_F(47)},
 {SDLK_F12,   FALSE,   KEY_F(12),   KEY_F(24),    KEY_F(36),   KEY_F(48)},
 {SDLK_F13,   FALSE,   KEY_F(13),   KEY_F(25),    KEY_F(37),   KEY_F(49)},
 {SDLK_F14,   FALSE,   KEY_F(14),   KEY_F(26),    KEY_F(38),   KEY_F(50)},
 {SDLK_F15,   FALSE,   KEY_F(15),   KEY_F(27),    KEY_F(39),   KEY_F(51)},
 {SDLK_BACKSPACE,FALSE,   0x08,        0x08,      CTL_BKSP,   ALT_BKSP},
 {SDLK_TAB,   FALSE,   0x09,        KEY_BTAB,      CTL_TAB,   ALT_TAB},
 {SDLK_PRINT,   FALSE,   KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,   KEY_PRINT},
 {SDLK_PAUSE,   FALSE,   KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {SDLK_CLEAR,   FALSE,   KEY_CLEAR,   KEY_CLEAR,    KEY_CLEAR,   KEY_CLEAR},
 {SDLK_BREAK,   FALSE,   KEY_BREAK,   KEY_BREAK,    KEY_BREAK,   KEY_BREAK},
 {SDLK_HELP,   FALSE,   KEY_HELP,    KEY_SHELP,    KEY_LHELP,   KEY_HELP},
 {SDLK_MENU,   FALSE,   KEY_OPTIONS, KEY_SOPTIONS, KEY_OPTIONS, KEY_OPTIONS},
 {SDLK_ESCAPE,   FALSE,   0x1B,        0x1B,      0x1B,   ALT_ESC},
 {SDLK_KP_ENTER,TRUE,   PADENTER,    PADENTER,      CTL_PADENTER,ALT_PADENTER},
 {SDLK_KP_PLUS,   TRUE,   PADPLUS,     '+',      CTL_PADPLUS, ALT_PADPLUS},
 {SDLK_KP_MINUS,TRUE,   PADMINUS,    '-',      CTL_PADMINUS,ALT_PADMINUS},
 {SDLK_KP_MULTIPLY,TRUE,PADSTAR,     '*',      CTL_PADSTAR, ALT_PADSTAR},
 {SDLK_KP_DIVIDE,TRUE,   PADSLASH,    '/',      CTL_PADSLASH,ALT_PADSLASH},
 {SDLK_KP_PERIOD,TRUE,   PADSTOP,     '.',      CTL_PADSTOP, ALT_PADSTOP},
 {SDLK_KP0,   TRUE,   PAD0,        '0',      CTL_PAD0,   ALT_PAD0},
 {SDLK_KP1,   TRUE,   KEY_C1,      '1',      CTL_PAD1,   ALT_PAD1},
 {SDLK_KP2,   TRUE,   KEY_C2,      '2',      CTL_PAD2,   ALT_PAD2},
 {SDLK_KP3,   TRUE,   KEY_C3,      '3',      CTL_PAD3,   ALT_PAD3},
 {SDLK_KP4,   TRUE,   KEY_B1,      '4',      CTL_PAD4,   ALT_PAD4},
 {SDLK_KP5,   TRUE,   KEY_B2,      '5',      CTL_PAD5,   ALT_PAD5},
 {SDLK_KP6,   TRUE,   KEY_B3,      '6',      CTL_PAD6,   ALT_PAD6},
 {SDLK_KP7,   TRUE,   KEY_A1,      '7',      CTL_PAD7,   ALT_PAD7},
 {SDLK_KP8,   TRUE,   KEY_A2,      '8',      CTL_PAD8,   ALT_PAD8},
 {SDLK_KP9,   TRUE,   KEY_A3,      '9',      CTL_PAD9,   ALT_PAD9},
#ifdef SDL_DOESNT_RECOGNIZE
 {SDLK_VOLUMEUP,  FALSE, KEY_VOLUME_UP, KEY_VOLUME_UP, KEY_VOLUME_UP, KEY_VOLUME_UP},
 {SDLK_VOLUMEDOWN,  FALSE, KEY_VOLUME_DOWN, KEY_VOLUME_DOWN, KEY_VOLUME_DOWN, KEY_VOLUME_DOWN},
 {SDLK_MUTE, FALSE, KEY_VOLUME_MUTE, KEY_VOLUME_MUTE, KEY_VOLUME_MUTE, KEY_VOLUME_MUTE},
 {SDLK_AGAIN, FALSE, KEY_REDO, KEY_REDO, KEY_REDO, KEY_REDO },
 {SDLK_SCROLLLOCK, FALSE, KEY_SCROLLLOCK, KEY_SCROLLLOCK, KEY_SCROLLLOCK, KEY_SCROLLLOCK },
#endif
 {SDLK_UNDO, FALSE, KEY_UNDO, KEY_UNDO, KEY_UNDO, KEY_UNDO },
 {0,      0,   0,        0,         0,      0}
};

void PDC_set_keyboard_binary(bool on)
{
    INTENTIONALLY_UNUSED_PARAMETER( on);
    PDC_LOG(("PDC_set_keyboard_binary() - called\n"));
}

/* check if a key or mouse event is waiting */

bool PDC_check_key(void)
{
    int haveevent = SDL_PollEvent(&event);

    return haveevent;
}

int SDL_WaitEventTimeout( SDL_Event *event, int timeout_ms)
{
   int rval = 0;

   while( timeout_ms && !rval)
      {
      const int slice_ms = (timeout_ms > 20 ? 20 : timeout_ms);

      napms( slice_ms);
      rval = SDL_PollEvent( event);
      timeout_ms -= slice_ms;
      }
   return( rval);
}

static int _process_key_event(void)
{
    int i, key = 0;
    static int prev_key = -1;     /* used to detect repeats */

    SP->key_modifiers = 0L;

    if (event.type == SDL_KEYUP)
    {
        if (SP->return_key_modifiers && event.key.keysym.sym == oldkey)
        {
            switch (oldkey)
            {
            case SDLK_RSHIFT:
                return KEY_SHIFT_R;
            case SDLK_LSHIFT:
                return KEY_SHIFT_L;
            case SDLK_RCTRL:
                return KEY_CONTROL_R;
            case SDLK_LCTRL:
                return KEY_CONTROL_L;
            case SDLK_RALT:
                return KEY_ALT_R;
            case SDLK_LALT:
                return KEY_ALT_L;
            default:
                break;
            }
        }
        prev_key = -1;
        return -1;
    }

    oldkey = event.key.keysym.sym;

    if (event.key.keysym.mod & KMOD_NUM)
        SP->key_modifiers |= PDC_KEY_MODIFIER_NUMLOCK;

    if (event.key.keysym.mod & KMOD_SHIFT)
        SP->key_modifiers |= PDC_KEY_MODIFIER_SHIFT;

    if (event.key.keysym.mod & KMOD_CTRL)
        SP->key_modifiers |= PDC_KEY_MODIFIER_CONTROL;

    if (event.key.keysym.mod & KMOD_ALT)
        SP->key_modifiers |= PDC_KEY_MODIFIER_ALT;

    for (i = 0; key_table[i].keycode; i++)
    {
        if (key_table[i].keycode == event.key.keysym.sym)
        {
            if ((event.key.keysym.mod & KMOD_SHIFT) ||
                (key_table[i].numkeypad && (event.key.keysym.mod & KMOD_NUM)))
            {
                key = key_table[i].shifted;
            }
            else if (event.key.keysym.mod & KMOD_CTRL)
            {
                key = key_table[i].control;
            }
            else if (event.key.keysym.mod & KMOD_ALT)
            {
                key = key_table[i].alt;
            }

            /* To get here, we ignore all other modifiers */

            else
                key = key_table[i].normal;
            break;
        }
    }

    if (!key)
    {
        key = event.key.keysym.unicode;
#if defined(__OS2__)
        if ( key == 0 && (event.key.keysym.mod & KMOD_CTRL || event.key.keysym.mod & KMOD_ALT) )
        {
            const int idx = event.key.keysym.scancode;
            const int scancode_to_letter_length = 51;
            static const char *scancode_to_letter =
                   "                qwertyuiop    asdfghjkl     zxcvbnm";

            if( idx >= 0 && idx < scancode_to_letter_length)
                if( scancode_to_letter[idx] != ' ')
                {
                    key = scancode_to_letter[idx] - 'a' + 1;
                    if ( event.key.keysym.mod & KMOD_ALT )
                        key = key + 96;  /* for alt set to lower case alphabetic */
                }
        }
#endif

        if (key > 0x7f)
            key = 0;
    }

    /* Handle ALT letters and numbers */

    if (event.key.keysym.mod & KMOD_ALT)
    {
        if (key >= 'A' && key <= 'Z')
            key += ALT_A - 'A';

        if (key >= 'a' && key <= 'z')
            key += ALT_A - 'a';

        if (key >= '0' && key <= '9')
            key += ALT_0 - '0';
    }
    if( key == 3 && !SP->raw_inp)
        exit( 0);

    if( key > 0 && prev_key == key)
        SP->key_modifiers |= PDC_KEY_MODIFIER_REPEAT;
    prev_key = key;

    return key ? key : -1;
}

static int _process_mouse_event(void)
{
    SDLMod keymods;
    short shift_flags = 0;

    memset(&SP->mouse_status, 0, sizeof(MOUSE_STATUS));

    keymods = SDL_GetModState();

    if (keymods & KMOD_SHIFT)
        shift_flags |= BUTTON_SHIFT;

    if (keymods & KMOD_CTRL)
        shift_flags |= BUTTON_CONTROL;

    if (keymods & KMOD_ALT)
        shift_flags |= BUTTON_ALT;

    SP->mouse_status.x = (event.motion.x - pdc_xoffset) / pdc_fwidth;
    SP->mouse_status.y = (event.motion.y - pdc_yoffset) / pdc_fheight;
    if( SP->mouse_status.x >= COLS || SP->mouse_status.y >= SP->lines)
        return -1;

    if (event.type == SDL_MOUSEMOTION)
    {
        int i;

        if (!event.motion.state ||
           (SP->mouse_status.x == old_mouse_status.x &&
            SP->mouse_status.y == old_mouse_status.y))
            return -1;

        SP->mouse_status.changes = PDC_MOUSE_MOVED;

        for (i = 0; i < 3; i++)
        {
            if (event.motion.state & SDL_BUTTON(i + 1))
            {
                SP->mouse_status.button[i] = BUTTON_MOVED | shift_flags;
                SP->mouse_status.changes |= (1 << i);
            }
        }
    }
    else
    {
        short action = (event.button.state == SDL_PRESSED) ?
                       BUTTON_PRESSED : BUTTON_RELEASED;
        Uint8 btn = event.button.button;

        /* handle scroll wheel */

        if ((btn >= 4 && btn <= 7) && action == BUTTON_RELEASED)
        {
            switch (btn)
            {
            case 4:
                SP->mouse_status.changes = PDC_MOUSE_WHEEL_UP;
                break;
            case 5:
                SP->mouse_status.changes = PDC_MOUSE_WHEEL_DOWN;
                break;
            case 6:
                SP->mouse_status.changes = PDC_MOUSE_WHEEL_LEFT;
                break;
            case 7:
                SP->mouse_status.changes = PDC_MOUSE_WHEEL_RIGHT;
            }

            return KEY_MOUSE;
        }

        if (btn < 1 || btn > 3)
            return -1;

        /* check for a click -- a press followed immediately by a release */

        if (action == BUTTON_PRESSED && SP->mouse_wait)
        {
            SDL_Event rel;

            while( action != BUTTON_TRIPLE_CLICKED && SDL_WaitEventTimeout(&rel, SP->mouse_wait))
            {
                if (rel.type == SDL_MOUSEBUTTONUP && rel.button.button == btn)
                {
                    if( action == BUTTON_PRESSED)
                        action = BUTTON_CLICKED;
                    else if( action == BUTTON_CLICKED)
                        action = BUTTON_DOUBLE_CLICKED;
                    else if( action == BUTTON_DOUBLE_CLICKED)
                        action = BUTTON_TRIPLE_CLICKED;
                }
                else if(rel.type != SDL_MOUSEBUTTONDOWN || rel.button.button != btn)
                {
                    SDL_PushEvent(&rel);
                    break;
                }
            }
        }

        btn--;

        SP->mouse_status.button[btn] = action | shift_flags;
        SP->mouse_status.changes = (1 << btn);
    }

    old_mouse_status = SP->mouse_status;

    return KEY_MOUSE;
}

/* return the next available key or mouse event */

int PDC_get_key(void)
{
    switch (event.type)
    {
    case SDL_QUIT:
        if( !PDC_get_function_key( FUNCTION_KEY_SHUT_DOWN))
        {
            exit(1);
        }
        return PDC_get_function_key( FUNCTION_KEY_SHUT_DOWN);
    case SDL_VIDEORESIZE:
        if (pdc_own_screen &&
           (event.resize.h / pdc_fheight != LINES ||
            event.resize.w / pdc_fwidth != COLS))
        {
            pdc_sheight = event.resize.h;
            pdc_swidth = event.resize.w;
            if( curscr)
            {
                touchwin( curscr);
                wrefresh( curscr);
            }

            if (!SP->resized)
            {
                SP->resized = TRUE;
                return KEY_RESIZE;
            }
        }
        break;
    case SDL_MOUSEMOTION:
        SDL_ShowCursor(SDL_ENABLE);
               /* FALLTHRU */
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        oldkey = SDLK_SPACE;
        return _process_mouse_event();
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        PDC_mouse_set();
        return _process_key_event();
    case SDL_USEREVENT:
        PDC_blink_text();
    }

    return -1;
}

/* discard any pending keyboard or mouse input -- this is the core
   routine for flushinp() */

void PDC_flushinp(void)
{
    PDC_LOG(("PDC_flushinp() - called\n"));

    while (PDC_check_key());
}

bool PDC_has_mouse(void)
{
    return TRUE;
}

int PDC_mouse_set(void)
{
    SDL_ShowCursor(SP->_trap_mbe ? SDL_ENABLE : SDL_DISABLE);

    return OK;
}

int PDC_modifiers_set(void)
{
    return OK;
}
