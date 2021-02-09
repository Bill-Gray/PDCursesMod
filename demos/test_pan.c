/* Demonstrates/tests panel library.  Based loosely on example from

http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/panels.html

   Should look like this when you run it :

            -------- Top of screen -----------
 Hit 1-3 to move panels 1-3 to the top
 Cursor keys move the top panel
 Tab toggles display of top panel
 Click on a panel to bring to top/send to bottom
 Escape or q exits the program

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

#include <panel.h>
#include <stdio.h>
#include <assert.h>

#if !defined( __PDCURSES__) && !NCURSES_SP_FUNCS
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

int main()
{
   WINDOW *my_wins[3];
   PANEL  *my_panels[3];
   int lines = 10, cols = 40, y = 7, x = 4, i, c = 0;

   initscr();
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
      box(my_wins[i], 0, 0);

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
   mvaddstr( 5, 1, "Escape or q exits the program");

   /* Update the stacking order. 2nd panel will be on top */
   while( c != 27 && c != 'q')
      {
      PANEL *curr_top = ceiling_panel( NULL);

      assert( curr_top);
      update_panels();

      /* Show it on the screen */
      doupdate();

      c = getch();
      switch( c)
         {
         case '1': case '2': case '3':
            top_panel( my_panels[c - '1']);
            break;
         case 9: case 'h':
#ifdef __PDCURSES__
            if( panel_hidden( curr_top) == OK)
#else
            if( panel_hidden( curr_top) == TRUE)
#endif
               show_panel( curr_top);
            else
               hide_panel( curr_top);
            break;
         case KEY_LEFT: case KEY_RIGHT: case KEY_UP: case KEY_DOWN:
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
#ifdef PDCURSES
    delscreen( SP);
#endif
    return( 0);
}
