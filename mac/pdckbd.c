#include "pdcmac.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(__MWERKS__)
#include "::common:mouse.c"
#else
#include "../common/mouse.c"
#endif

int PDC_key_queue[PDC_KEY_QUEUE_SIZE];
int PDC_key_queue_head = 0;
int PDC_key_queue_tail = 0;

static int held_mouse_button = -1;
static Point last_mouse_local;
static long mouse_wait_until = 0;
static unsigned long last_event_tick = 0;
static int prev_special_key = -1;

static const struct
{
    unsigned char keycode;
    unsigned char numkeypad;
    unsigned short normal;
    unsigned short shifted;
    unsigned short control;
    unsigned short alt;
} key_table[] =
{
    { 0x7B, 0, KEY_LEFT,    KEY_SLEFT,     CTL_LEFT,     ALT_LEFT },
    { 0x7C, 0, KEY_RIGHT,   KEY_SRIGHT,    CTL_RIGHT,    ALT_RIGHT },
    { 0x7E, 0, KEY_UP,      KEY_SUP,       CTL_UP,       ALT_UP },
    { 0x7D, 0, KEY_DOWN,    KEY_SDOWN,     CTL_DOWN,     ALT_DOWN },
    { 0x73, 0, KEY_HOME,    KEY_SHOME,     CTL_HOME,     ALT_HOME },
    { 0x77, 0, KEY_END,     KEY_SEND,      CTL_END,      ALT_END },
    { 0x74, 0, KEY_PPAGE,   KEY_SPREVIOUS, CTL_PGUP,     ALT_PGUP },
    { 0x79, 0, KEY_NPAGE,   KEY_SNEXT,     CTL_PGDN,     ALT_PGDN },
    { 0x72, 0, KEY_IC,      KEY_SIC,       CTL_INS,      ALT_INS },
    { 0x75, 0, KEY_DC,      KEY_SDC,       CTL_DEL,      ALT_DEL },
    { 0x33, 0, 0x08,        0x08,          CTL_BKSP,     ALT_BKSP },
    { 0x30, 0, 0x09,        KEY_BTAB,      CTL_TAB,      ALT_TAB },
    { 0x35, 0, 0x1B,        0x1B,          0x1B,         ALT_ESC },
    { 0x24, 0, 0x0D,        0x0D,          0x0D,         0x0D },
    { 0x4C, 1, PADENTER,    PADENTER,      CTL_PADENTER, ALT_PADENTER },
    { 0x45, 1, PADPLUS,     '+',           CTL_PADPLUS,  ALT_PADPLUS },
    { 0x4E, 1, PADMINUS,    '-',           CTL_PADMINUS, ALT_PADMINUS },
    { 0x43, 1, PADSTAR,     '*',           CTL_PADSTAR,  ALT_PADSTAR },
    { 0x4B, 1, PADSLASH,    '/',           CTL_PADSLASH, ALT_PADSLASH },
    { 0x41, 1, PADSTOP,     '.',           CTL_PADSTOP,  ALT_PADSTOP },
    { 0x52, 1, PAD0,        '0',           CTL_PAD0,     ALT_PAD0 },
    { 0x53, 1, KEY_C1,      '1',           CTL_PAD1,     ALT_PAD1 },
    { 0x54, 1, KEY_C2,      '2',           CTL_PAD2,     ALT_PAD2 },
    { 0x55, 1, KEY_C3,      '3',           CTL_PAD3,     ALT_PAD3 },
    { 0x56, 1, KEY_B1,      '4',           CTL_PAD4,     ALT_PAD4 },
    { 0x57, 1, KEY_B2,      '5',           CTL_PAD5,     ALT_PAD5 },
    { 0x58, 1, KEY_B3,      '6',           CTL_PAD6,     ALT_PAD6 },
    { 0x59, 1, KEY_A1,      '7',           CTL_PAD7,     ALT_PAD7 },
    { 0x5B, 1, KEY_A2,      '8',           CTL_PAD8,     ALT_PAD8 },
    { 0x5C, 1, KEY_A3,      '9',           CTL_PAD9,     ALT_PAD9 },
    { 0x47, 1, KEY_CLEAR,   KEY_CLEAR,     KEY_CLEAR,    KEY_CLEAR },
    { 0x7A, 0, KEY_F(1),    KEY_F(13),     KEY_F(25),    KEY_F(37) },
    { 0x78, 0, KEY_F(2),    KEY_F(14),     KEY_F(26),    KEY_F(38) },
    { 0x63, 0, KEY_F(3),    KEY_F(15),     KEY_F(27),    KEY_F(39) },
    { 0x76, 0, KEY_F(4),    KEY_F(16),     KEY_F(28),    KEY_F(40) },
    { 0x60, 0, KEY_F(5),    KEY_F(17),     KEY_F(29),    KEY_F(41) },
    { 0x61, 0, KEY_F(6),    KEY_F(18),     KEY_F(30),    KEY_F(42) },
    { 0x62, 0, KEY_F(7),    KEY_F(19),     KEY_F(31),    KEY_F(43) },
    { 0x64, 0, KEY_F(8),    KEY_F(20),     KEY_F(32),    KEY_F(44) },
    { 0x65, 0, KEY_F(9),    KEY_F(21),     KEY_F(33),    KEY_F(45) },
    { 0x6D, 0, KEY_F(10),   KEY_F(22),     KEY_F(34),    KEY_F(46) },
    { 0x67, 0, KEY_F(11),   KEY_F(23),     KEY_F(35),    KEY_F(47) },
    { 0x6F, 0, KEY_F(12),   KEY_F(24),     KEY_F(36),    KEY_F(48) },
    { 0x69, 0, KEY_F(13),   KEY_F(25),     KEY_F(37),    KEY_F(49) },
    { 0x6B, 0, KEY_F(14),   KEY_F(26),     KEY_F(38),    KEY_F(50) },
    { 0x71, 0, KEY_F(15),   KEY_F(27),     KEY_F(39),    KEY_F(51) },
    { 0, 0, 0, 0, 0, 0 }
};

void PDC_mac_add_key(int key)
{
    int next;

    if (!key)
        return;
    next = PDC_key_queue_tail + 1;
    if (next >= PDC_KEY_QUEUE_SIZE)
        next = 0;
    if (next == PDC_key_queue_head)
        return;
    if (key == KEY_RESIZE)
    {
        int i;

        i = PDC_key_queue_head;
        while (i != PDC_key_queue_tail)
        {
            if (PDC_key_queue[i] == KEY_RESIZE)
                return;
            i++;
            if (i >= PDC_KEY_QUEUE_SIZE)
                i = 0;
        }
        if (SP)
            SP->resized = TRUE;
    }
    PDC_key_queue[PDC_key_queue_tail] = key;
    PDC_key_queue_tail = next;
}

static int queue_has_key(void)
{
    if (PDC_key_queue_head != PDC_key_queue_tail)
        return 1;
    return 0;
}

static int pop_key(void)
{
    int key;

    if (PDC_key_queue_head == PDC_key_queue_tail)
        return -1;
    key = PDC_key_queue[PDC_key_queue_head];
    PDC_key_queue_head++;
    if (PDC_key_queue_head >= PDC_KEY_QUEUE_SIZE)
        PDC_key_queue_head = 0;
    if (key == KEY_MOUSE)
        _get_mouse_event(&SP->mouse_status);
    return key;
}

static unsigned long modifiers_from_event(const EventRecord *event)
{
    unsigned long mods;

    mods = 0;
    if (event->modifiers & shiftKey)
        mods |= PDC_KEY_MODIFIER_SHIFT;
    if (event->modifiers & controlKey)
        mods |= PDC_KEY_MODIFIER_CONTROL;
    if (event->modifiers & optionKey)
        mods |= PDC_KEY_MODIFIER_ALT;
    if (event->modifiers & cmdKey)
        mods |= PDC_KEY_MODIFIER_SUPER;
    if (event->modifiers & alphaLock)
        mods |= PDC_KEY_MODIFIER_CAPSLOCK;
    return mods;
}

static int mouse_modifs(const EventRecord *event)
{
    int mods;

    mods = 0;
    if (event->modifiers & shiftKey)
        mods |= BUTTON_SHIFT;
    if (event->modifiers & controlKey)
        mods |= BUTTON_CONTROL;
    if (event->modifiers & optionKey)
        mods |= BUTTON_ALT;
    return mods;
}

static int mouse_button_from_event(const EventRecord *event)
{
    if (event->modifiers & controlKey)
        return 2;
    if (event->modifiers & optionKey)
        return 1;
    return 0;
}

static void local_mouse(const EventRecord *event, Point *local)
{
    *local = event->where;
    PDC_mac_set_port();
    GlobalToLocal(local);
}

static void announce_mouse_events(void)
{
    int announced;
    int i;

    mouse_wait_until = 0;
    announced = 0;
    for (i = PDC_key_queue_head; i != PDC_key_queue_tail; )
    {
        if (PDC_key_queue[i] == KEY_MOUSE)
            announced++;
        i++;
        if (i >= PDC_KEY_QUEUE_SIZE)
            i = 0;
    }
    while (announced < _mlist_count)
    {
        PDC_mac_add_key(KEY_MOUSE);
        announced++;
    }
}

static void check_mouse_wait(void)
{
    if (mouse_wait_until && PDC_millisecs() - mouse_wait_until >= 0)
        announce_mouse_events();
}

static void mouse_cell(const Point *local, int *x, int *y)
{
    *x = local->h / PDC_font_width;
    *y = local->v / PDC_font_height;
    if (*x < 0)
        *x = 0;
    if (*y < 0)
        *y = 0;
    if (SP && *x >= SP->cols)
        *x = SP->cols - 1;
    if (SP && *y >= SP->lines)
        *y = SP->lines - 1;
}

static void queue_mouse(int button, int event_type, const EventRecord *event)
{
    Point local;
    int x;
    int y;

    local_mouse(event, &local);
    last_mouse_local = local;
    mouse_cell(&local, &x, &y);
    if (_add_raw_mouse_event(button, event_type, mouse_modifs(event), x, y)
        && SP->mouse_wait > 0)
        mouse_wait_until = PDC_millisecs() + SP->mouse_wait;
    else
        announce_mouse_events();
}

static int lookup_special(unsigned char keycode, unsigned long modifiers)
{
    int i;

    for (i = 0; key_table[i].normal; i++)
    {
        if (key_table[i].keycode != keycode)
            continue;
        if (modifiers & PDC_KEY_MODIFIER_SHIFT)
            return key_table[i].shifted;
        if (modifiers & PDC_KEY_MODIFIER_CONTROL)
            return key_table[i].control;
        if (modifiers & PDC_KEY_MODIFIER_ALT)
            return key_table[i].alt;
        return key_table[i].normal;
    }
    return 0;
}

static int translate_key(const EventRecord *event)
{
    unsigned char keycode;
    unsigned char charcode;
    unsigned long modifiers;
    int key;

    keycode = (unsigned char)((event->message & keyCodeMask) >> 8);
    charcode = (unsigned char)(event->message & charCodeMask);
    modifiers = modifiers_from_event(event);
    SP->key_modifiers = modifiers;
    if (event->what == autoKey)
        SP->key_modifiers |= PDC_KEY_MODIFIER_REPEAT;
    if ((event->modifiers & cmdKey) && charcode == 'q')
    {
        key = PDC_get_function_key(FUNCTION_KEY_SHUT_DOWN);
        if (!key)
            exit(0);
        return key;
    }
    if ((event->modifiers & cmdKey) && charcode == 'c')
    {
        key = PDC_get_function_key(FUNCTION_KEY_COPY);
        if (key)
            return key;
        return 3;
    }
    if ((event->modifiers & cmdKey) && charcode == 'v')
    {
        key = PDC_get_function_key(FUNCTION_KEY_PASTE);
        if (key)
            return key;
        return 22;
    }
    if ((event->modifiers & cmdKey) && (charcode == '+' || charcode == '='))
    {
        key = PDC_get_function_key(FUNCTION_KEY_ENLARGE_FONT);
        if (key)
            return key;
        PDC_font_size += 2;
        if (PDC_font_size > 48)
            PDC_font_size = 48;
        PDC_mac_apply_font();
        PDC_mac_adjust_cells(PDC_cols, PDC_rows, 1);
        return -1;
    }
    if ((event->modifiers & cmdKey) && charcode == '-')
    {
        key = PDC_get_function_key(FUNCTION_KEY_SHRINK_FONT);
        if (key)
            return key;
        PDC_font_size -= 2;
        if (PDC_font_size < 6)
            PDC_font_size = 6;
        PDC_mac_apply_font();
        PDC_mac_adjust_cells(PDC_cols, PDC_rows, 1);
        return -1;
    }
    key = lookup_special(keycode, modifiers);
    if (key)
        return key;
    if (event->modifiers & cmdKey)
        return -1;
    if (charcode >= 32)
    {
#ifdef PDC_WIDE
        return (int)PDC_macroman_to_unicode(charcode);
#else
        return (int)charcode;
#endif
    }
    if (charcode > 0)
        return (int)charcode;
    return -1;
}

static void handle_key(EventRecord *event)
{
    int key;

    if (event->modifiers & cmdKey)
    {
        long menuresult;

        menuresult = MenuKey((short)(event->message & charCodeMask));
        if (HiWord(menuresult))
        {
            PDC_mac_handle_menu(menuresult);
            return;
        }
    }
    if (event->what == keyUp)
    {
        unsigned char keycode;

        keycode = (unsigned char)((event->message & keyCodeMask) >> 8);
        if (SP && SP->return_key_modifiers)
        {
            if (keycode == 0x38 || keycode == 0x3C)
                PDC_mac_add_key(KEY_SHIFT_L);
            else if (keycode == 0x3B || keycode == 0x3E)
                PDC_mac_add_key(KEY_CONTROL_L);
            else if (keycode == 0x3A || keycode == 0x3D)
                PDC_mac_add_key(KEY_ALT_L);
        }
        prev_special_key = -1;
        return;
    }
    key = translate_key(event);
    if (key == 3 && SP && !SP->raw_inp)
        exit(0);
    if (key > 0 && key == PDC_get_function_key(FUNCTION_KEY_ABORT))
        exit(-1);
    if (key > 0)
    {
        if (prev_special_key == key)
            SP->key_modifiers |= PDC_KEY_MODIFIER_REPEAT;
        prev_special_key = key;
        PDC_mac_add_key(key);
    }
}

static void handle_content_click(EventRecord *event, int pressed)
{
    int button;

    if (pressed)
    {
        held_mouse_button = mouse_button_from_event(event);
        queue_mouse(held_mouse_button, BUTTON_PRESSED, event);
    }
    else if (held_mouse_button >= 0)
    {
        button = held_mouse_button;
        held_mouse_button = -1;
        queue_mouse(button, BUTTON_RELEASED, event);
    }
}

static void handle_mouse_moved(EventRecord *event)
{
    Point local;
    int button;
    int x;
    int y;

    if (!SP || !SP->_trap_mbe)
    {
        PDC_mac_reset_mouse_rgn();
        return;
    }
    local_mouse(event, &local);
    if (local.h == last_mouse_local.h && local.v == last_mouse_local.v)
    {
        PDC_mac_reset_mouse_rgn();
        return;
    }
    last_mouse_local = local;
    mouse_cell(&local, &x, &y);
    button = 0;
    if (held_mouse_button >= 0)
        button = held_mouse_button;
    _add_raw_mouse_event(button, BUTTON_MOVED, mouse_modifs(event), x, y);
    if (!mouse_wait_until)
        announce_mouse_events();
    PDC_mac_reset_mouse_rgn();
}

static void dispatch_event(EventRecord *event)
{
    WindowPtr which;
    short part;

    switch (event->what)
    {
    case mouseDown:
        part = FindWindow(event->where, &which);
        if (part == inMenuBar)
        {
            PDC_mac_handle_menu(MenuSelect(event->where));
            break;
        }
#if !TARGET_API_MAC_CARBON
        if (part == inSysWindow)
        {
            SystemClick(event, which);
            break;
        }
#endif
        if (which != PDC_window)
        {
            if (which)
                SelectWindow(which);
            break;
        }
        if (part == inDrag)
            PDC_mac_drag(event);
        else if (part == inGrow)
            PDC_mac_grow(event);
        else if (part == inGoAway)
            PDC_mac_goaway(event);
        else if (part == inZoomIn || part == inZoomOut)
            PDC_mac_zoom(event, part);
        else if (part == inContent)
        {
            SelectWindow(PDC_window);
            handle_content_click(event, 1);
        }
        break;
    case mouseUp:
        handle_content_click(event, 0);
        break;
    case keyDown:
    case autoKey:
    case keyUp:
        handle_key(event);
        break;
    case updateEvt:
        if ((WindowPtr)event->message == PDC_window)
            PDC_mac_redraw();
        break;
    case activateEvt:
        if ((WindowPtr)event->message == PDC_window)
        {
            PDC_mac_set_port();
            DrawGrowIcon(PDC_window);
        }
        break;
    case osEvt:
        {
            unsigned char kind;

            kind = (unsigned char)((event->message >> 24) & 0xFF);
            if (kind == mouseMovedMessage)
                handle_mouse_moved(event);
            else if (kind == suspendResumeMessage)
            {
#if !TARGET_API_MAC_CARBON
                if (event->message & resumeFlag)
                    LoadScrap();
                else
                    UnloadScrap();
#endif
            }
        }
        break;
    case kHighLevelEvent:
        AEProcessAppleEvent(event);
        break;
    default:
        break;
    }
}

void PDC_mac_process_events(int sleepticks)
{
    EventRecord event;
    Boolean got;
    RgnHandle rgn;

    rgn = PDC_mouse_rgn;
    got = WaitNextEvent(everyEvent, &event, sleepticks, rgn);
    if (got)
        dispatch_event(&event);
    while (EventAvail(everyEvent, &event))
    {
        got = WaitNextEvent(everyEvent, &event, 0, rgn);
        if (!got)
            break;
        dispatch_event(&event);
    }
}

void PDC_set_keyboard_binary(bool on)
{
    INTENTIONALLY_UNUSED_PARAMETER(on);
    PDC_LOG(("PDC_set_keyboard_binary() - called\n"));
}

bool PDC_check_key(void)
{
    unsigned long tick;

    PDC_check_for_blinking();
    tick = TickCount();
    if (tick != last_event_tick)
    {
        last_event_tick = tick;
        PDC_mac_process_events(0);
    }
    check_mouse_wait();
    if (queue_has_key())
        return TRUE;
    return FALSE;
}

int PDC_get_key(void)
{
    int key;

    if (!queue_has_key())
        PDC_mac_process_events(GetCaretTime());
    check_mouse_wait();
    key = pop_key();
    return key;
}

void PDC_flushinp(void)
{
    PDC_LOG(("PDC_flushinp() - called\n"));
    PDC_key_queue_head = 0;
    PDC_key_queue_tail = 0;
    mouse_wait_until = 0;
    held_mouse_button = -1;
    while (_get_mouse_event(&SP->mouse_status))
        ;
    FlushEvents(keyDownMask | keyUpMask | autoKeyMask | mDownMask | mUpMask, 0);
}

bool PDC_has_mouse(void)
{
    return TRUE;
}

int PDC_mouse_set(void)
{
    PDC_mac_reset_mouse_rgn();
    return OK;
}

int PDC_modifiers_set(void)
{
    return OK;
}
