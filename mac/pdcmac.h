#ifndef PDC_MAC_H
#define PDC_MAC_H 1

#if !defined(TARGET_API_MAC_CARBON) && defined(TARGET_CARBON)
# define TARGET_API_MAC_CARBON TARGET_CARBON
#endif

#define erase PDC_mac_qd_erase /* CW8 non-Pro has erase as an enum */
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Events.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <Sound.h>
#include <Scrap.h>
#include <OSUtils.h>
#include <Timer.h>
#include <Gestalt.h>
#include <Devices.h>
#include <AppleEvents.h>
#include <Resources.h>
#undef erase

#undef MOUSE_MOVED
#include <curspriv.h>

#ifndef kScrapFlavorTypeText
#define kScrapFlavorTypeText 'TEXT'
#endif

#define PDC_MAC_GROW_MARGIN 15

#ifndef zoomDocProc
#define zoomDocProc 8
#endif

#if !defined(NewAEEventHandlerUPP) && defined(NewAEEventHandlerProc)
#define NewAEEventHandlerUPP NewAEEventHandlerProc
#endif

#ifndef kFontIDSymbol
#define kFontIDSymbol 23
#endif

#ifndef monaco
#define monaco 4
#endif

#ifndef bold
#define bold 1
#endif

#ifndef italic
#define italic 2
#endif

#ifndef everyEvent
#define everyEvent 0xFFFF
#endif

#ifndef mouseMovedMessage
#define mouseMovedMessage 0xFA
#endif

#ifndef suspendResumeMessage
#define suspendResumeMessage 1
#endif

#ifndef resumeFlag
#define resumeFlag 1
#endif

#ifndef kHighLevelEvent
#define kHighLevelEvent 23
#endif

#ifndef gestaltAppleEventsPresent
#define gestaltAppleEventsPresent 0
#endif

#ifndef noTypeErr
#define noTypeErr -204
#endif

#ifndef memFullErr
#define memFullErr -108
#endif

#ifndef charCodeMask
#define charCodeMask 0x000000FF
#endif

#ifndef keyCodeMask
#define keyCodeMask 0x0000FF00
#endif

#ifndef activeFlag
#define activeFlag 0x0001
#endif

#define PDC_KEY_QUEUE_SIZE 64

extern WindowPtr PDC_window;
extern int PDC_rows;
extern int PDC_cols;
extern int PDC_font_width;
extern int PDC_font_height;
extern int PDC_font_ascent;
extern int PDC_font_size;
extern int PDC_key_queue[PDC_KEY_QUEUE_SIZE];
extern int PDC_key_queue_head;
extern int PDC_key_queue_tail;
extern RgnHandle PDC_mouse_rgn;
extern int PDC_min_lines;
extern int PDC_max_lines;
extern int PDC_min_cols;
extern int PDC_max_cols;

#define PDC_MENU_APPLE 128
#define PDC_MENU_FILE 129
#define PDC_MENU_EDIT 130
#define PDC_ITEM_QUIT 1
#define PDC_ITEM_CUT 1
#define PDC_ITEM_COPY 2
#define PDC_ITEM_PASTE 3
#define PDC_ITEM_CLEAR 4

void PDC_mac_set_port(void);
void PDC_mac_port_bounds(Rect *bounds);
void PDC_mac_invalidate(const Rect *bounds);
void PDC_mac_invalidate_all(void);
void PDC_mac_cell_rect(int row, int col, Rect *cell);
void PDC_mac_add_key(int key);
void PDC_mac_process_events(int sleepticks);
void PDC_mac_apply_font(void);
void PDC_mac_reset_mouse_rgn(void);
void PDC_mac_drag(EventRecord *event);
void PDC_mac_grow(EventRecord *event);
void PDC_mac_zoom(EventRecord *event, short part);
void PDC_mac_goaway(EventRecord *event);
void PDC_mac_redraw(void);
void PDC_mac_handle_menu(long menuresult);
void PDC_mac_adjust_size(int pixelwidth, int pixelheight, int queue_resize);
void PDC_mac_adjust_cells(int cols, int rows, int queue_resize);
void PDC_mac_default_background(RGBColor *color);
unsigned short PDC_macroman_to_unicode(unsigned char ch);
unsigned char PDC_unicode_to_macroman(unsigned long code);

void PDC_check_for_blinking(void);

#endif
