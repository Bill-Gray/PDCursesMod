/* Demonstrates/tests panel library.  Based loosely on example from

http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/panels.html

   Should look like this when you run it (note that your editor
should be set to UTF-8;  otherwise,  you'll see some garbage) :

            -------- Top of screen -----------
 Hit 1-3 to move panels 1-3 to the top
 Cursor keys move the top panel
 Tab toggles display of top panel
 Click on a panel to bring to top/send to bottom
 Escape or q exits the program
 r causes the panel to be repositioned at random
    ┌──────────────────────────────────────┐
    │    ┌──────────────────────────────────────┐
    │    │    ┌──────────────────────────────────────┐
    │    │    │                                      │
    │    │    │                                      │
    │    │    │                                      │
    │    │    │                                      │
    │    │    │    Window 3                          │
    │    │    │                                      │
    └────│    │                                      │
         └────│                                      │
              └──────────────────────────────────────┘
*/

#define PDC_NCMOUSE
#define _XOPEN_SOURCE_EXTENDED   1

#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>

#include "../demos/snprintf.c"

#if !defined( __PDCURSESMOD__) && !NCURSES_SP_FUNCS
      /* Older ncurses may lack ceiling_panel() and ground_panel() */

PANEL *ceiling_panel( SCREEN *sp)
{
   return( panel_below( NULL));
}

PANEL *ground_panel( SCREEN *sp)
{
   return( panel_above( NULL));
}
#endif         /* #ifndef NCURSES_SP_FUNCS */

/* To determine on which panel (if any) the mouse was clicked,
cycle through panel_below() until you find that (x, y) is within
the window extents,  or you get a NULL panel.         */

PANEL *panel_from_point( const int y, const int x)
{
   PANEL *curr = NULL;

   while( (curr = panel_below( curr)) != NULL)
      {
      WINDOW *win = panel_window( curr);
      const int x0 = getbegx( win), y0 = getbegy( win);

      if( x >= x0 && y >= y0 && x - x0 < getmaxx( win)
                             && y - y0 < getmaxy( win))
         return( curr);
      }
   return( NULL);
}

#ifdef WACS_S1
# define HAVE_WIDE 1
#endif

#ifdef HAVE_WIDE

#define CHOOSE( A, B)   (B)

#define BOX_SD_RTEE                       CHOOSE( 0xb5,  0x2561)
#define BOX_DS_RTEE                       CHOOSE( 0xb6,  0x2562)
#define BOX_DS_URCORNER                   CHOOSE( 0xb7,  0x2556)
#define BOX_SD_URCORNER                   CHOOSE( 0xb8,  0x2555)
#define BOX_D_RTEE                        CHOOSE( 0xb9,  0x2563)
#define BOX_D_VLINE                       CHOOSE( 0xba,  0x2551)
#define BOX_D_URCORNER                    CHOOSE( 0xbb,  0x2557)
#define BOX_D_LRCORNER                    CHOOSE( 0xbc,  0x255D)
#define BOX_DS_LRCORNER                   CHOOSE( 0xbd,  0x255c)
#define BOX_SD_LRCORNER                   CHOOSE( 0xbe,  0x255b)

#define BOX_SD_LTEE                       CHOOSE( 0xc6,  0x255e)
#define BOX_DS_LTEE                       CHOOSE( 0xc7,  0x255f)
#define BOX_D_LLCORNER                    CHOOSE( 0xc8,  0x255A)
#define BOX_D_ULCORNER                    CHOOSE( 0xc9,  0x2554)
#define BOX_D_BTEE                        CHOOSE( 0xca,  0x2569)
#define BOX_D_TTEE                        CHOOSE( 0xcb,  0x2566)
#define BOX_D_LTEE                        CHOOSE( 0xcc,  0x2560)
#define BOX_D_HLINE                       CHOOSE( 0xcd,  0x2550)
#define BOX_D_PLUS                        CHOOSE( 0xce,  0x256C)
#define BOX_SD_BTEE                       CHOOSE( 0xcf,  0x2567)

#define BOX_DS_BTEE                       CHOOSE( 0xd0,  0x2568)
#define BOX_SD_TTEE                       CHOOSE( 0xd1,  0x2564)
#define BOX_DS_TTEE                       CHOOSE( 0xd2,  0x2565)
#define BOX_DS_LLCORNER                   CHOOSE( 0xd3,  0x2559)
#define BOX_SD_LLCORNER                   CHOOSE( 0xd4,  0x2558)
#define BOX_SD_ULCORNER                   CHOOSE( 0xd5,  0x2552)
#define BOX_DS_ULCORNER                   CHOOSE( 0xd6,  0x2553)
#define BOX_DS_PLUS                       CHOOSE( 0xd7,  0x256b)
#define BOX_SD_PLUS                       CHOOSE( 0xd8,  0x256a)

#define BOX_URROUNDED                     CHOOSE( 0xbf,  0x256e)
#define BOX_LLROUNDED                     CHOOSE( 0xc0,  0x2570)
#define BOX_LRROUNDED                     CHOOSE( 0xd9,  0x256f)
#define BOX_ULROUNDED                     CHOOSE( 0xda,  0x256d)

#define BOX_VLINE                         CHOOSE( 0xb3,  0x2502)
#define BOX_RTEE                          CHOOSE( 0xb4,  0x2524)
#define BOX_URCORNER                      CHOOSE( 0xbf,  0x2510)
#define BOX_LLCORNER                      CHOOSE( 0xc0,  0x2514)
#define BOX_LRCORNER                      CHOOSE( 0xd9,  0x2518)
#define BOX_ULCORNER                      CHOOSE( 0xda,  0x250c)
#define BOX_BTEE                          CHOOSE( 0xc1,  0x2534)
#define BOX_TTEE                          CHOOSE( 0xc2,  0x252c)
#define BOX_LTEE                          CHOOSE( 0xc3,  0x251c)
#define BOX_HLINE                         CHOOSE( 0xc4,  0x2500)
#define BOX_PLUS                          CHOOSE( 0xc5,  0x253c)

#define BOX_URROUNDED                     CHOOSE( 0xbf,  0x256e)
#define BOX_LLROUNDED                     CHOOSE( 0xc0,  0x2570)
#define BOX_LRROUNDED                     CHOOSE( 0xd9,  0x256f)
#define BOX_ULROUNDED                     CHOOSE( 0xda,  0x256d)

#define BOX_T_URCORNER                    CHOOSE( 0xbf,  0x2513)
#define BOX_T_LRCORNER                    CHOOSE( 0xd9,  0x251b)
#define BOX_T_ULCORNER                    CHOOSE( 0xda,  0x250f)
#define BOX_T_LLCORNER                    CHOOSE( 0xc0,  0x2517)
#define BOX_T_BTEE                        CHOOSE( 0xc1,  0x253b)
#define BOX_T_TTEE                        CHOOSE( 0xc2,  0x2533)
#define BOX_T_LTEE                        CHOOSE( 0xc3,  0x2523)
#define BOX_T_RTEE                        CHOOSE( 0xb4,  0x252b)
#define BOX_T_HLINE                       CHOOSE( 0xc4,  0x2501)
#define BOX_T_VLINE                       CHOOSE( 0xb3,  0x2503)
#define BOX_T_PLUS                        CHOOSE( 0xc5,  0x254b)

#define BOX_DOUBLED_V        1
#define BOX_DOUBLED_H        2
#define BOX_THICK            4
#define BOX_ROUNDED          8

static int _set_up_box( WINDOW *win, const int ls, const int rs,
   const int ts, const int bs, const int tl, const int tr,
   const int bl, const int br)
{
   wchar_t x[8], string[2];
   cchar_t c[8];
   size_t i;

   x[0] = (wchar_t)ls;    x[1] = (wchar_t)rs;
   x[2] = (wchar_t)ts;    x[3] = (wchar_t)bs;
   x[4] = (wchar_t)tl;    x[5] = (wchar_t)tr;
   x[6] = (wchar_t)bl;    x[7] = (wchar_t)br;
   string[1] = '\0';
   for( i = 0; i < 8; i++)
      {
      string[0] = x[i];
      setcchar( c + i, string, 0, 0, NULL);
      }
   return( wborder_set( win, c, c + 1, c + 2, c + 3, c + 4, c + 5, c + 6, c + 7));
}

static int _show_box( WINDOW *win, const int style)
{
   switch( style)
   {
      case 0:        /* 'normal' single-line */
         _set_up_box( win, BOX_VLINE, BOX_VLINE, BOX_HLINE, BOX_HLINE,
                  BOX_ULCORNER, BOX_URCORNER, BOX_LLCORNER, BOX_LRCORNER);
         break;
      case BOX_DOUBLED_H:
         _set_up_box( win, BOX_VLINE, BOX_VLINE, BOX_D_HLINE, BOX_D_HLINE,
                  BOX_SD_ULCORNER, BOX_SD_URCORNER, BOX_SD_LLCORNER, BOX_SD_LRCORNER);
         break;
      case BOX_DOUBLED_V:
         _set_up_box( win, BOX_D_VLINE, BOX_D_VLINE, BOX_HLINE, BOX_HLINE,
                  BOX_DS_ULCORNER, BOX_DS_URCORNER, BOX_DS_LLCORNER, BOX_DS_LRCORNER);
         break;
      case BOX_DOUBLED_V | BOX_DOUBLED_H:
         _set_up_box( win, BOX_D_VLINE, BOX_D_VLINE, BOX_D_HLINE, BOX_D_HLINE,
                  BOX_D_ULCORNER, BOX_D_URCORNER, BOX_D_LLCORNER, BOX_D_LRCORNER);
         break;
      case BOX_THICK:
         _set_up_box( win, BOX_T_VLINE, BOX_T_VLINE, BOX_T_HLINE, BOX_T_HLINE,
                  BOX_T_ULCORNER, BOX_T_URCORNER, BOX_T_LLCORNER, BOX_T_LRCORNER);
         break;
      case BOX_ROUNDED:
         _set_up_box( win, BOX_VLINE, BOX_VLINE, BOX_HLINE, BOX_HLINE,
                  BOX_ULROUNDED, BOX_URROUNDED, BOX_LLROUNDED, BOX_LRROUNDED);
         break;
   }
   return( 0);
}
#else       /* non-wide builds */
#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

static int _show_box( WINDOW *win, const int style)
{
    INTENTIONALLY_UNUSED_PARAMETER( style);
    return( box( win, 0, 0));
}
#endif

int main( const int argc, const char **argv)
{
   WINDOW *my_wins[3];
   PANEL  *my_panels[3];
   int lines = 10, cols = 40, y = 8, x = 4, i, c = 0;
   int box_style = 0;
   SCREEN *screen_pointer;

   setlocale(LC_ALL, (argc == 1 ? "" : argv[1]));
   screen_pointer = newterm(NULL, stdout, stdin);
   cbreak();
   noecho();
   keypad( stdscr, 1);
   mousemask( ALL_MOUSE_EVENTS, NULL);

   /* Create windows for the panels */
   my_wins[0] = newwin(lines, cols, y, x);
   my_wins[1] = newwin(lines, cols, y + 1, x + 5);
   my_wins[2] = newwin(lines, cols, y + 2, x + 10);

   /*
    * Create borders around the windows so that you can see the effect
    * of panels
    */
   for(i = 0; i < 3; ++i)
      _show_box(my_wins[i], box_style);

   /* Attach a panel to each window */    /* Order is bottom up */
   my_panels[0] = new_panel(my_wins[0]);  /* Push 0, order: stdscr-0 */
   my_panels[1] = new_panel(my_wins[1]);  /* Push 1, order: stdscr-0-1 */
   my_panels[2] = new_panel(my_wins[2]);  /* Push 2, order: stdscr-0-1-2 */
   for( i = 0; i < 3; i++)
      {
      char text[20];

      snprintf( text, sizeof( text), "Window %d", i + 1);
      mvwaddstr( my_wins[i], 5, 5, text);
      }
   mvaddstr( 1, 1, "Hit 1-3 to move panels 1-3 to the top");
   mvaddstr( 2, 1, "Cursor keys move the top panel");
   mvaddstr( 3, 1, "Tab toggles display of top panel");
   mvaddstr( 4, 1, "Click on a panel to bring to top/send to bottom");
#ifdef __PDCURSESMOD__
   mvaddstr( 5, 1, "h, v toggle doubled horiz/vert lines");
#endif
   mvaddstr( 6, 1, "Escape or q exits the program");
   mvaddstr( 7, 1, "r causes the panel to be repositioned at random");

   /* Update the stacking order. 2nd panel will be on top */
   while( c != 27 && c != 'q')
      {
      PANEL *curr_top = ceiling_panel( NULL);

      update_panels();

      /* Show it on the screen */
      doupdate();

      c = getch();
      switch( c)
         {
         case '1': case '2': case '3':
            top_panel( my_panels[c - '1']);
            break;
         case 9:
            if( curr_top)
               {
#ifdef __PDCURSES__
               if( panel_hidden( curr_top) == OK)
#else
               if( panel_hidden( curr_top) == TRUE)
#endif
                  show_panel( curr_top);
               else
                  hide_panel( curr_top);
               }
            break;
#ifdef HAVE_WIDE
         case 'v':
         case 'h':
         case 't':
            if( c == 't')
               {
               if( box_style == BOX_ROUNDED)
                  box_style = 0;
               else if( box_style == BOX_THICK)
                  box_style = BOX_ROUNDED;
               else
                  box_style = BOX_THICK;
               }
            else if( c == 'v')
               box_style ^= BOX_DOUBLED_V;
            else
               box_style ^= BOX_DOUBLED_H;
            for(i = 0; i < 3; ++i)
               _show_box(my_wins[i], box_style);
            update_panels();
            break;
#endif
         case KEY_LEFT: case KEY_RIGHT: case KEY_UP: case KEY_DOWN:
            if( curr_top)
               {
               WINDOW *win = panel_window( curr_top);

               x = getbegx( win);
               y = getbegy( win);
               if( c == KEY_LEFT)
                  x--;
               if( c == KEY_RIGHT)
                  x++;
               if( c == KEY_UP)
                  y--;
               if( c == KEY_DOWN)
                  y++;
               if( move_panel( curr_top, y, x) == ERR)
                  flash( );
               }
            break;
         case 'r':
            if( curr_top)
               move_panel( curr_top, rand( ) % (LINES - lines), rand( ) % (COLS - cols));
            break;
         case KEY_MOUSE:
            {
            MEVENT mouse_event;
            PANEL *clicked_on_panel;

            getmouse( &mouse_event);
            clicked_on_panel = panel_from_point( mouse_event.y, mouse_event.x);
            if( clicked_on_panel)
               {
               if( clicked_on_panel == ceiling_panel( NULL))  /* it's already on top */
                  bottom_panel( clicked_on_panel);
               else
                  top_panel( clicked_on_panel);
               }
            }
            break;
         case KEY_RESIZE:
            resize_term( 0, 0);
            break;
         case 'q':
            break;
         default:
            flash( );
            break;
         }
      }
   endwin();
   for( i = 0; i < 3; ++i)
      {
      del_panel( my_panels[i]);
      delwin( my_wins[i]);
      }
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return( 0);
}
