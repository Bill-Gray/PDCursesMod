/* Public Domain Curses */

#include "pdcwin.h"

void PDC_set_keyboard_binary(bool on)
{
    PDC_LOG(("PDC_set_keyboard_binary() - called\n"));

    INTENTIONALLY_UNUSED_PARAMETER( on);
}

/* check if a key or mouse event is waiting */

bool PDC_check_key(void)
{
    HAB hab = WinQueryAnchorBlock( PDC_hWnd);
    QMSG msg;

    while( PDC_key_queue_low == PDC_key_queue_high
                       && WinPeekMsg( hab, &msg, NULLHANDLE, 0, 0, PM_REMOVE) )
       WinDispatchMsg(hab, &msg);
    return( PDC_key_queue_low != PDC_key_queue_high);
}

/* return the next available key or mouse event */

int PDC_get_key(void)
{
    int rval = -1;

    if( PDC_key_queue_low != PDC_key_queue_high)
    {
        rval = PDC_key_queue[PDC_key_queue_low++];
        if( PDC_key_queue_low == KEY_QUEUE_SIZE)
            PDC_key_queue_low = 0;
        if( rval == KEY_RESIZE)
            while( PDC_key_queue[PDC_key_queue_low] == KEY_RESIZE
                           && PDC_key_queue_low != PDC_key_queue_high)
            {
                PDC_key_queue_low++;
                if( PDC_key_queue_low == KEY_QUEUE_SIZE)
                    PDC_key_queue_low = 0;
            }
         if( rval == KEY_MOUSE)
            PDC_get_mouse_event_from_queue( );
    }
    return rval;
}

/* discard any pending keyboard or mouse input -- this is the core
   routine for flushinp() */

void PDC_flushinp(void)
{
    PDC_LOG(("PDC_flushinp() - called\n"));
    PDC_key_queue_low = PDC_key_queue_high = 0;
    while( !PDC_get_mouse_event_from_queue( ))
        ;
}

bool PDC_has_mouse( void)
{
    return TRUE;
}

int PDC_mouse_set(void)
{
    /* If turning on mouse input: Set ENABLE_MOUSE_INPUT, and clear
       all other flags, including the extended flags;
       If turning off the mouse: Set QuickEdit Mode to the status it
       had on startup, and clear all other flags */

    return OK;
}

int PDC_modifiers_set(void)
{
    return OK;
}
