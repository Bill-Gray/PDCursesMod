
/*
 * This is a test program for PDCurses. Originally by
 * John Burnell <johnb@kea.am.dsir.govt.nz>
 *
 *  wrs (1993-05-28) -- modified to be consistent (perform identically)
 *                      with either PDCurses or under Unix System V, R4
 */

#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
#endif

/* Uncomment the following #define to test the 'classic' (undocumented
SysV) mouse functions in inputTest.  Otherwise,  the ncurses mouse
interface will be used.       */

// #define CLASSIC_MOUSE_INTERFACE

#if defined( CLASSIC_MOUSE_INTERFACE)
   #define BUTTON_MOVE_EVENTS (BUTTON1_MOVED | BUTTON2_MOVED | BUTTON3_MOVED \
                          | BUTTON4_MOVED | BUTTON5_MOVED)
   #define ALL_MOVE_EVENTS  (BUTTON_MOVE_EVENTS | REPORT_MOUSE_POSITION)
#else
   #define PDC_NCMOUSE
   #define ALL_MOVE_EVENTS   REPORT_MOUSE_POSITION
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>

#ifdef WACS_S1
# define HAVE_WIDE 1
#else
# define HAVE_WIDE 0
#endif

#include <locale.h>

#if HAVE_WIDE
# include <wchar.h>
#endif

#if defined(PDCURSES) && !defined(XCURSES)
# define HAVE_RESIZE 1
#else
# define HAVE_RESIZE 0
#endif

#ifdef A_COLOR
# define HAVE_COLOR 1
#else
# define HAVE_COLOR 0
#endif

#ifdef PDCURSES
# define HAVE_CLIPBOARD 1
#else
# define HAVE_CLIPBOARD 0
#endif

void inputTest(WINDOW *);
void scrollTest(WINDOW *);
void introTest(WINDOW *);
int initTest(WINDOW **, int, char **);
void outputTest(WINDOW *);
void padTest(WINDOW *);
void acsTest(WINDOW *);
void attrTest(WINDOW *);

#if HAVE_COLOR
void colorTest(WINDOW *);
#endif

#if HAVE_RESIZE
void resizeTest(WINDOW *);
#endif

#if HAVE_CLIPBOARD
void clipboardTest(WINDOW *);
#endif

#if HAVE_WIDE
void wideTest(WINDOW *);
#endif

void display_menu(int, int);

struct commands
{
    const char *text;
    void (*function)(WINDOW *);
};

typedef struct commands COMMAND;

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)
#define MAX_OPTIONS (7 + HAVE_COLOR + HAVE_RESIZE + HAVE_CLIPBOARD + HAVE_WIDE)

COMMAND command[MAX_OPTIONS] =
{
    {"Intro Test", introTest},
    {"Pad Test", padTest},
#if HAVE_RESIZE
    {"Resize Test", resizeTest},
#endif
    {"Scroll Test", scrollTest},
    {"Input Test", inputTest},
    {"Output Test", outputTest},
    {"ACS Test", acsTest},
    {"Attrib Test", attrTest},
#if HAVE_COLOR
    {"Color Test", colorTest},
#endif
#if HAVE_CLIPBOARD
    {"Clipboard Test", clipboardTest},
#endif
#if HAVE_WIDE
    {"Wide Input", wideTest}
#endif
};

int width, height;
static mmask_t test_mouse_mask = ALL_MOUSE_EVENTS;
static SCREEN *screen_pointer;

static mmask_t parse_mouse_mask( const char *str)
{
   test_mouse_mask = 0;
   while( *str)
      {
      const mmask_t *event_ptr = NULL;
      const mmask_t pressed[5] = { BUTTON1_PRESSED, BUTTON2_PRESSED,
                  BUTTON3_PRESSED, BUTTON4_PRESSED, BUTTON5_PRESSED };
      const mmask_t released[5] = { BUTTON1_RELEASED, BUTTON2_RELEASED,
                  BUTTON3_RELEASED, BUTTON4_RELEASED, BUTTON5_RELEASED };
      const mmask_t clicked[5] = { BUTTON1_CLICKED, BUTTON2_CLICKED,
                  BUTTON3_CLICKED, BUTTON4_CLICKED, BUTTON5_CLICKED };
      const mmask_t dblclk[5] = { BUTTON1_DOUBLE_CLICKED, BUTTON2_DOUBLE_CLICKED,
                  BUTTON3_DOUBLE_CLICKED, BUTTON4_DOUBLE_CLICKED, BUTTON5_DOUBLE_CLICKED };
      const mmask_t triclk[5] = { BUTTON1_TRIPLE_CLICKED, BUTTON2_TRIPLE_CLICKED,
                  BUTTON3_TRIPLE_CLICKED, BUTTON4_TRIPLE_CLICKED, BUTTON5_TRIPLE_CLICKED };
#ifdef BUTTON1_MOVED
      const mmask_t moved[5] = { BUTTON1_MOVED, BUTTON2_MOVED,
                  BUTTON3_MOVED, BUTTON4_MOVED, BUTTON5_MOVED };
#endif

      switch( *str)
         {
         case 'p':
            event_ptr = pressed;
            break;
         case 'r':
            event_ptr = released;
            break;
         case 'c':
            event_ptr = clicked;
            break;
         case 'd':
            event_ptr = dblclk;
            break;
         case 't':
            event_ptr = triclk;
            break;
#ifdef BUTTON1_MOVED
         case 'm':
            event_ptr = moved;
            break;
#endif
#ifdef MOUSE_WHEEL_SCROLL
         case 'w':
            test_mouse_mask |= MOUSE_WHEEL_SCROLL;
            break;
#endif
#ifdef BUTTON_MODIFIER_SHIFT
         case 's':
            test_mouse_mask |= BUTTON_MODIFIER_SHIFT;
            break;
#endif
#ifdef BUTTON_MODIFIER_CONTROL
         case 'x':
            test_mouse_mask |= BUTTON_MODIFIER_CONTROL;
            break;
#endif
#ifdef BUTTON_MODIFIER_ALT
         case 'a':
            test_mouse_mask |= BUTTON_MODIFIER_ALT;
            break;
#endif
         case 'M':
            test_mouse_mask |= REPORT_MOUSE_POSITION;
            break;
         }
      str++;
      if( event_ptr)
         while( *str >= '1' && *str <= '5')
            test_mouse_mask |= event_ptr[*str++ - '1'];
      }
   return test_mouse_mask;
}

int main(int argc, char *argv[])
{
    WINDOW *win;
    int key, old_option = -1, new_option = 0, i;
    bool quit = FALSE;

    if( !setlocale( LC_CTYPE, "C.UTF-8"))
        setlocale( LC_CTYPE, "en_US.utf8");

#ifdef __PDCURSESMOD__
#ifdef PDC_VER_MAJOR   /* so far only seen in 4.0+ */
    PDC_set_resize_limits( 20, 50, 70, 200);
#endif
#endif

    if (initTest(&win, argc, argv))
        return 1;

    for( i = 1; i < argc; i++)
        if( argv[i][0] == '-')
            switch( argv[i][1])
            {
                case 'd':
                    {
                        unsigned flags;

                        if( 1 == sscanf( argv[i] + 2, "%x", &flags))
                            curses_trace( flags);
                    }
                    break;
                case 'i': case 'I':
                    mouseinterval( atoi( argv[i] + 2));
                    break;
                case 'l': case 'L':
                    setlocale( LC_CTYPE, argv[i] + 2);
                    break;
                case 's':
                    {
                        char tbuff[200];

                        if( fgets( tbuff, sizeof( tbuff), stdin))
                           printf( "Got a line\n%s", tbuff);
                    }
                    break;
#ifdef __PDCURSESMOD__
                case 'b': case 'B':
                    PDC_set_blink( TRUE);
                    break;
                case 'm': case 'M':
                    PDC_return_key_modifiers( TRUE);
                    break;
                case 't':
                    traceon( );
                    break;
#ifdef PDC_VER_MAJOR   /* so far only seen in 4.0+ */
                case 'r':     /* allow user-resizable windows */
                    {
                        int min_lines, max_lines, min_cols, max_cols;

                        if( sscanf( argv[i] + 2, "%d,%d,%d,%d",
                                       &min_lines, &max_lines,
                                       &min_cols, &max_cols) == 4)
                            PDC_set_resize_limits( min_lines, max_lines,
                                                   min_cols, max_cols);
                    }
                    break;
#endif
#endif
                case 'z':
                    if( !argv[i][2])
                       test_mouse_mask = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
                    else
                       test_mouse_mask = parse_mouse_mask( argv[i] + 2);
                    break;
                default:
                    break;
            }
#ifdef A_COLOR
    if (has_colors())
    {
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        wbkgd(win, COLOR_PAIR(1));
    }
    else
#endif
        wbkgd(win, A_REVERSE);

    erase();
    display_menu(old_option, new_option);

    while (1)
    {
        noecho();
        keypad(stdscr, TRUE);
        raw();

        key = getch();

        switch(key)
        {
        case 10:
        case 13:
        case KEY_ENTER:
            old_option = -1;
            erase();
            refresh();
            (*command[new_option].function)(win);
            erase();
            display_menu(old_option, new_option);
            break;

        case KEY_PPAGE:
        case KEY_HOME:
            old_option = new_option;
            new_option = 0;
            display_menu(old_option, new_option);
            break;

        case KEY_NPAGE:
        case KEY_END:
            old_option = new_option;
            new_option = MAX_OPTIONS - 1;
            display_menu(old_option, new_option);
            break;

        case KEY_UP:
            old_option = new_option;
            new_option = (new_option == 0) ?
                new_option : new_option - 1;
            display_menu(old_option, new_option);
            break;

        case KEY_DOWN:
            old_option = new_option;
            new_option = (new_option == MAX_OPTIONS - 1) ?
                new_option : new_option + 1;
            display_menu(old_option, new_option);
            break;
#ifdef KEY_RESIZE
        case KEY_RESIZE:
# ifdef PDCURSES
            resize_term(0, 0);
# endif
            old_option = -1;
            erase();
            display_menu(old_option, new_option);
            break;
#endif
        case 'Q':
        case 'q':
            quit = TRUE;
        }

        if (quit == TRUE)
            break;
    }

    delwin(win);
    endwin();
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return 0;
}

void Continue(WINDOW *win)
{
    mvwaddstr(win, 10, 1, " Press any key to continue");
    wrefresh(win);
    raw();
    wgetch(win);
}

void Continue2(void)
{
    move(LINES - 1, 1);
    clrtoeol();
    mvaddstr(LINES - 2, 1, " Press any key to continue");
    refresh();
    raw();
    getch();
}

int initTest(WINDOW **win, int argc, char *argv[])
{
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    INTENTIONALLY_UNUSED_PARAMETER( argc);
    screen_pointer = newterm(NULL, stdout, stdin);
#ifdef A_COLOR
    if (has_colors())
        start_color();
#endif
    /* Create a drawing window */

    width  = 60;
    height = 13;

    *win = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);

    if (*win == NULL)
    {
        endwin();
        return 1;
    }

    return 0;
}

void introTest(WINDOW *win)
{
    werase(win);
    wmove(win, height / 2 - 5, width / 2);
    wvline(win, ACS_VLINE, 10);
    wmove(win, height / 2, width / 2 - 10);
    whline(win, ACS_HLINE, 20);
    Continue(win);

    beep();
    werase(win);

    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);

    cbreak();
    mvwaddstr(win, 1, 1,
        "You should have a rectangle in the middle of the screen");
    mvwaddstr(win, 2, 1, "You should have heard a beep");
    Continue(win);

    flash();
    mvwaddstr(win, 3, 1, "You should have seen a flash");
    Continue(win);
}

void scrollTest(WINDOW *win)
{
    int i, OldY;
#if !defined (PDCURSES) && !defined (NCURSES_VERSION)
    int OldX;
#endif
    werase(win);
    mvwaddstr(win, height - 2, 1, "The window will now scroll slowly");
    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);
    scrollok(win, TRUE);
    napms(500);

    for (i = 1; i <= height; i++)
    {
        napms(150);
        scroll(win);
        wrefresh(win);
    };

#if defined (PDCURSES) || defined (NCURSES_VERSION)
    OldY = getmaxy(win);
#else
    getmaxyx(win, OldY, OldX);
#endif
    mvwaddstr(win, 6, 1, "The top of the window will scroll");
    wmove(win, 1, 1);
    wsetscrreg(win, 0, 4);
    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);

    for (i = 1; i <= 5; i++)
    {
        napms(500);
        scroll(win);
        wrefresh(win);
    }

    mvwaddstr(win, 3, 1, "The bottom of the window will scroll");
    wmove(win, 8, 1);
    wsetscrreg(win, 5, --OldY);
    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);

    for (i = 5; i <= OldY; i++)
    {
        napms(300);
        wscrl(win, -1);
        wrefresh(win);
    }

    wsetscrreg(win, 0, OldY);
}

void inputTest(WINDOW *win)
{
    int w, h, bx, by, sw, sh, i, c, num = 0;
    char buffer[80];
    WINDOW *subWin;
    static const char spinner[5] = "/-\\|";
    int spinner_count = 0;
    int line = 3;
#ifndef CLASSIC_MOUSE_INTERFACE
    int mouse_buttons_held = 0;
#endif

    wclear(win);

    getmaxyx(win, h, w);
    getbegyx(win, by, bx);

    sw = w / 3;
    sh = h / 3;

    if ((subWin = subwin(win, sh, sw, by + h - sh - 2, bx + w - sw - 2))
        == NULL)
        return;

#ifdef A_COLOR
    if (has_colors())
    {
        init_pair(2, COLOR_WHITE, COLOR_RED);
        wbkgd(subWin, COLOR_PAIR(2) | A_BOLD);
    }
    else
#endif
        wbkgd(subWin, A_BOLD);

    box(subWin, ACS_VLINE, ACS_HLINE);
    wrefresh(win);

    nocbreak();

    wclear (win);
    mvwaddstr(win, 1, 1,
        "Press keys (or mouse buttons) to show their names");
    mvwaddstr(win, 2, 1, "Press spacebar to finish, Ctrl-A to return to main menu");
    wrefresh(win);

    keypad(win, TRUE);
    raw();
    noecho();

    wtimeout(win, 200);

#ifdef CLASSIC_MOUSE_INTERFACE
    mouse_set( test_mouse_mask);
#else
    mousemask( test_mouse_mask, NULL);
#endif
#ifdef NCURSES_VERSION
    if( test_mouse_mask & REPORT_MOUSE_POSITION)
      printf( "\x1b\x5b?1003h\n");        /* command string to enable mouse movement */
#endif
    curs_set(0);        /* turn cursor off */

    while (1)
    {
        if( line >= h - 2)
            line = 3;
        wmove(win, line + 1, 3);
        wclrtoeol(win);
        wmove(win, line, 3);
        wclrtoeol(win);
        while (1)
        {
#ifdef PDC_WIDE
            wint_t wch;

            if( ERR == wget_wch( win, &wch))
               c = ERR;
            else
               c = (int)wch;
#else
            c = wgetch(win);
#endif

            if (c == ERR)
            {
                spinner_count++;
                if (spinner_count == 4)
                    spinner_count = 0;
                mvwaddch(win, line, 3, spinner[spinner_count]);
                wrefresh(win);
            }
            else
                break;
        }
        mvwaddstr(win, line, 5, "Key Pressed: ");

        wprintw( win, "(%x) ", c);
        if( has_key( c))
            wprintw(win, "%s", keyname(c));
        else if (isprint(c) || c > 0x7f)
            waddch( win, c);
        else
            wprintw(win, "%s", unctrl(c));
#ifdef CLASSIC_MOUSE_INTERFACE
        if (c == KEY_MOUSE)
        {
            int button = 0;
            request_mouse_pos();

            for( i = 0; i < PDC_MAX_MOUSE_BUTTONS; i++)
                if (BUTTON_CHANGED(i))
                    button = i;

            wmove(win, line, 5);
            wclrtoeol(win);
            if( button)
                wprintw(win, "Button %d: ", button);

            if (MOUSE_MOVED)
                waddstr(win, "moved: ");
            else if (MOUSE_WHEEL_UP)
                waddstr(win, "wheel up: ");
            else if (MOUSE_WHEEL_DOWN)
                waddstr(win, "wheel dn: ");
            else if (MOUSE_WHEEL_LEFT)
                waddstr(win, "wheel lt: ");
            else if (MOUSE_WHEEL_RIGHT)
                waddstr(win, "wheel rt: ");
            else if ((BUTTON_STATUS(button) &
                BUTTON_ACTION_MASK) == BUTTON_PRESSED)
                waddstr(win, "pressed: ");
            else if ((BUTTON_STATUS(button) &
                BUTTON_ACTION_MASK) == BUTTON_CLICKED)
                waddstr(win, "clicked: ");
            else if ((BUTTON_STATUS(button) &
                BUTTON_ACTION_MASK) == BUTTON_DOUBLE_CLICKED)
                waddstr(win, "double: ");
            else if ((BUTTON_STATUS(button) &
                BUTTON_ACTION_MASK) == BUTTON_TRIPLE_CLICKED)
                waddstr(win, "triple: ");
            else
                waddstr(win, "released: ");

            if( !button)
                button = 1;     /* to allow button modifiers to be read */
            if( BUTTON_STATUS(button) & BUTTON_MODIFIER_MASK)
            {
                if (BUTTON_STATUS(button) & BUTTON_SHIFT)
                    waddstr(win, "SHIFT ");

                if (BUTTON_STATUS(button) & BUTTON_CONTROL)
                    waddstr(win, "CONTROL ");

                if (BUTTON_STATUS(button) & BUTTON_ALT)
                    waddstr(win, "ALT ");
            }

            wprintw(win, "Posn: Y: %d X: %d", MOUSE_Y_POS, MOUSE_X_POS);
        }
        else if (PDC_get_key_modifiers())
        {
            waddstr(win, " Modifier(s):");
            if (PDC_get_key_modifiers() & PDC_KEY_MODIFIER_SHIFT)
                waddstr(win, " SHIFT");

            if (PDC_get_key_modifiers() & PDC_KEY_MODIFIER_CONTROL)
                waddstr(win, " CONTROL");

            if (PDC_get_key_modifiers() & PDC_KEY_MODIFIER_ALT)
                waddstr(win, " ALT");

            if (PDC_get_key_modifiers() & PDC_KEY_MODIFIER_NUMLOCK)
                waddstr(win, " NUMLOCK");
        }
#else             /* ncurses mouse handling */
        if (c == KEY_MOUSE)
        {
            MEVENT mevent;

            wmove(win, line, 5);
            wclrtoeol(win);
            if( OK == getmouse( &mevent))
            {
                const mmask_t events[] = { BUTTON1_PRESSED, BUTTON1_RELEASED,
                        BUTTON1_CLICKED, BUTTON1_DOUBLE_CLICKED, BUTTON1_TRIPLE_CLICKED,
                        BUTTON2_PRESSED, BUTTON2_RELEASED, BUTTON2_CLICKED,
                         BUTTON2_DOUBLE_CLICKED, BUTTON2_TRIPLE_CLICKED,
                        BUTTON3_PRESSED, BUTTON3_RELEASED, BUTTON3_CLICKED,
                         BUTTON3_DOUBLE_CLICKED, BUTTON3_TRIPLE_CLICKED,
                        BUTTON4_PRESSED, BUTTON4_RELEASED, BUTTON4_CLICKED,
                         BUTTON4_DOUBLE_CLICKED, BUTTON4_TRIPLE_CLICKED,
                        BUTTON5_PRESSED, BUTTON5_RELEASED, BUTTON5_CLICKED,
                         BUTTON5_DOUBLE_CLICKED, BUTTON5_TRIPLE_CLICKED,
#ifdef BUTTON_CTRL
                  BUTTON_CTRL,
#else
                  BUTTON_CONTROL,
#endif
                        BUTTON_SHIFT, BUTTON_ALT };

                for( i = 0; (size_t)i < sizeof( events) / sizeof( events[0]); i++)
                    if( mevent.bstate & events[i])
                    {
                        const char *text[8] = { "pressed", "released", "clicked",
                               "dblclick", "triple-click", "Ctrl", "Shift", "Alt" };

                        if( i < 25)
                           wprintw( win, "button %d %s", i / 5 + 1, text[i % 5]);
                        else
                           wprintw( win, "  %s", text[i - 20]);
                        if( i % 5 == 0)    /* button pressed */
                           mouse_buttons_held |= (1 << (i / 5));
                        if( i % 5 == 1)    /* button released */
                           mouse_buttons_held &= ~(1 << (i / 5));
                    }
                if( mevent.bstate & ALL_MOVE_EVENTS)
                {
                    wprintw( win, "moved <%d>", mouse_buttons_held);
                    for( i = 0; i < 3; i++)
                        if( (mouse_buttons_held >> i) & 1)
                            wprintw( win, "  Button %d", i + 1);
                }
                wprintw(win, "  Posn: Y: %d X: %d", mevent.y, mevent.x);
            }
            else
                wprintw( win, "  ? getmouse failed ?");
        }
#endif
        wrefresh(win);
        line++;

        if (c == ' ' || c == 1)
            break;
    }

    wtimeout(win, -1);  /* turn off timeout() */
    curs_set(1);        /* turn cursor back on */

#ifdef CLASSIC_MOUSE_INTERFACE
    mouse_set(0L);
    PDC_return_key_modifiers(FALSE);
#else
    mousemask( (mmask_t)0, NULL);
#endif
#ifdef NCURSES_VERSION
    if( test_mouse_mask & REPORT_MOUSE_POSITION)
      printf( "\x1b\x5b?1003l\n");        /* disables mouse movement reports */
#endif
    wclear(win);
    if( c == 1)
    {
       delwin( subWin);
       return;
    }
    mvwaddstr(win, 2, 1, "Press some keys for 5 seconds");
    mvwaddstr(win, 1, 1, "Pressing ^C should do nothing");
    wrefresh(win);

    werase(subWin);
    box(subWin, ACS_VLINE, ACS_HLINE);

    for (i = 0; i < 5; i++)
    {
        mvwprintw(subWin, 1, 1, "Time = %d", i);
        wrefresh(subWin);
        napms(1000);
        flushinp();
    }

    delwin(subWin);
    werase(win);
    flash();
    wrefresh(win);
    napms(500);
    flushinp();

    mvwaddstr(win, 2, 1, "Press a key, followed by ENTER");
    wmove(win, 9, 10);
    wrefresh(win);
    echo();

    keypad(win, TRUE);
    raw();
    wgetnstr(win, buffer, 3);
    flushinp();

    wmove(win, 9, 10);
    wdelch(win);
    mvwaddstr(win, 4, 1, "The character should now have been deleted");
    Continue(win);

    refresh();
    wclear(win);
    echo();
    buffer[0] = '\0';
    mvwaddstr(win, 3, 2, "The window should have moved");
    mvwaddstr(win, 4, 2,
              "This text should have appeared without you pressing a key");
    mvwaddstr(win, 6, 2, "Enter a number then a string separated by space");
    mvwin(win, 2, 1);
    wrefresh(win);
    mvwscanw(win, 7, 6, "%d %79s", &num, buffer);
    mvwprintw(win, 8, 6, "String: %s Number: %d", buffer, num);
    Continue(win);

    refresh();
    wclear(win);
    echo();
    mvwaddstr(win, 3, 2, "Enter a 5 character string: ");
    wgetnstr(win, buffer, 5);
    mvwprintw(win, 4, 2, "String: %s", buffer);
    Continue(win);
}

void outputTest(WINDOW *win)
{
    WINDOW *win1;
    char Buffer[80];
    chtype ch;
    int by, bx;

#ifdef PDCURSES
    PDC_set_blink(TRUE);
#endif
    nl();
    wclear(win);
    mvwaddstr(win, 1, 1, "You should now have a screen in the upper "
                         "left corner, and this text should have wrapped");
    waddstr(win,"\nThis text should be down\n");
    waddstr(win,  "and broken into two here ^");
    Continue(win);

    wclear(win);
    wattron(win, A_BOLD);
    mvwaddstr(win, 1, 1, "A new window will appear with this text in it");
    mvwaddstr(win, 8, 1, "Press any key to continue");
    wrefresh(win);
    wgetch(win);

    getbegyx(win, by, bx);

    if (LINES < 24 || COLS < 75)
    {
        mvwaddstr(win, 5, 1, "Some tests have been skipped as they require a");
        mvwaddstr(win, 6, 1, "display of at least 24 LINES by 75 COLUMNS");
        Continue(win);
    }
    else
    {
        win1 = newwin(10, 50, 14, 25);

        if (win1 == NULL)
        {
            endwin();
            return;
        }

#ifdef A_COLOR
        if (has_colors())
        {
            init_pair(3, COLOR_BLUE, COLOR_WHITE);
            wbkgd(win1, COLOR_PAIR(3));
        }
        else
#endif
            wbkgd(win1, A_NORMAL);

        wclear(win1);
        mvwaddstr(win1, 5, 1, "This text should appear; using overlay option");
        copywin(win, win1, 0, 0, 0, 0, 9, 49, TRUE);
        box(win1, ACS_VLINE, ACS_HLINE);
        wmove(win1, 8, 26);
        wrefresh(win1);
        wgetch(win1);

        wclear(win1);

        wattron(win1, A_BLINK);
        mvwaddstr(win1, 4, 1,
                  "This blinking text should appear in only the second window");
        wattroff(win1, A_BLINK);

        mvwin(win1, by, bx);
        overlay(win, win1);
        mvwin(win1, 14, 25);
        wmove(win1, 8, 26);
        wrefresh(win1);
        wgetch(win1);

        delwin(win1);
    }

    clear();
    wclear(win);
    wrefresh(win);
    mvwaddstr(win, 6, 2, "This line shouldn't appear");
    mvwaddstr(win, 4, 2, "Only half of the next line is visible");
    mvwaddstr(win, 5, 2, "Only half of the next line is visible");
    wmove(win, 6, 1);
    wclrtobot(win);
    wmove(win, 5, 20);
    wclrtoeol(win);
    mvwaddstr(win, 8, 2, "This line also shouldn't appear");
    wmove(win, 8, 1);
    winsdelln(win, -1);
    Continue(win);

    wmove(win, 5, 9);
    ch = winch(win);

    wclear(win);
    wmove(win, 6, 2);
    waddstr(win, "The next char should be l:  ");
    winsch(win, ch);
    Continue(win);

    mvwinsstr(win, 6, 2, "A1B2C3D4E5");
    Continue(win);

    wmove(win, 5, 1);
    winsdelln(win, 1);
    mvwaddstr(win, 5, 2, "The lines below should have moved down");
    Continue(win);

    wclear(win);
    wmove(win, 2, 2);
    wprintw(win, "This is a formatted string in a window: %d %s\n",
            42, "is it");
    mvwaddstr(win, 10, 1, "Enter a string: ");
    wrefresh(win);
    echo();
    wscanw(win, "%79s", Buffer);

    printw("This is a formatted string in stdscr: %d %s\n", 42, "is it");
    mvaddstr(10, 1, "Enter a string: ");
    scanw("%79s", Buffer);

    wclear(win);
    curs_set(2);
    mvwaddstr(win, 1, 1, "The cursor should be in high-visibility mode");
    Continue(win);

    wclear(win);
    curs_set(0);
    mvwaddstr(win, 1, 1, "The cursor should have disappeared");
    Continue(win);

    wclear(win);
    curs_set(1);
    mvwaddstr(win, 1, 1, "The cursor should be normal");
    Continue(win);

#ifdef A_COLOR
    if (has_colors())
    {
        wclear(win);
        mvwaddstr(win, 1, 1, "This window should change to red text on a white");
        mvwaddstr(win, 2, 1, "background after you press a key");
        Continue(win);

        init_pair(1, COLOR_RED, COLOR_WHITE);
        wrefresh(win);
    }
#endif
    werase(win);
    mvwaddstr(win, 1, 1, "Information About Your Terminal");
    mvwaddstr(win, 3, 1, termname());
    mvwaddstr(win, 4, 1, longname());

    if (termattrs() & A_BLINK)
        mvwaddstr(win, 5, 1, "This terminal claims to support blinking.");
    else
        mvwaddstr(win, 5, 1, "This terminal does NOT support blinking.");

    mvwaddnstr(win, 7, 5, "Have a nice day!ok", 16);
    wrefresh(win);

    mvwinnstr(win, 7, 5, Buffer, 18);
    mvaddstr(LINES - 2, 10, Buffer);
    refresh();
    Continue(win);
#ifdef PDCURSES
    PDC_set_blink(FALSE);
#endif
}

#if HAVE_RESIZE
void resizeTest(WINDOW *dummy)
{
    WINDOW *win1;
    int nwidth = 135, nheight = 52;
    int owidth = COLS, oheight = LINES;

    INTENTIONALLY_UNUSED_PARAMETER( dummy);
    savetty();

    resize_term(nheight, nwidth);

    clear();
    refresh();

    win1 = newwin(10, 50, 14, 25);

    if (win1 == NULL)
    {
        endwin();
        return;
    }

#ifdef A_COLOR
    if (has_colors())
    {
        init_pair(3, COLOR_BLUE, COLOR_WHITE);
        wattrset(win1, COLOR_PAIR(3));
    }

    wclear(win1);
#endif
    mvwaddstr(win1, 0, 0, "The screen may now be resized");
    mvwprintw(win1, 1, 4, "Given size: %d by %d", nwidth, nheight);
    mvwprintw(win1, 2, 4, "Actual size: %d by %d", COLS, LINES);
    Continue(win1);

    wclear(win1);
    resetty();

    mvwaddstr(win1, 0, 0, "The screen should now be reset");
    mvwprintw(win1, 1, 6, "Old size: %d by %d", owidth, oheight);
    mvwprintw(win1, 2, 6, "Size now: %d by %d", COLS, LINES);
    Continue(win1);

    delwin(win1);

    clear();
    refresh();
}
#endif /* HAVE_RESIZE */

void padTest(WINDOW *dummy)
{
    WINDOW *pad, *spad;

    INTENTIONALLY_UNUSED_PARAMETER( dummy);
    pad = newpad(50, 100);
    wattron(pad, A_REVERSE);
    mvwaddstr(pad, 5, 2, "This is a new pad");
    wattrset(pad, 0);
    mvwaddstr(pad, 8, 0,
        "The end of this line should be truncated here:except  now");
    mvwaddstr(pad, 11, 1, "This line should not appear.It will now");
    wmove(pad, 10, 1);
    wclrtoeol(pad);
    mvwaddstr(pad, 10, 1, " Press any key to continue");
    prefresh(pad, 0, 0, 0, 0, 10, 45);
    keypad(pad, TRUE);
    raw();
    wgetch(pad);

    spad = subpad(pad, 12, 25, 7, 52);
    mvwaddstr(spad, 2, 2, "This is a new subpad");
    box(spad, 0, 0);
    delwin(spad);
    prefresh(pad, 0, 0, 0, 0, 15, 75);
    keypad(pad, TRUE);
    raw();
    wgetch(pad);

    mvwaddstr(pad, 35, 2, "This is displayed at line 35 in the pad");
    mvwaddstr(pad, 40, 1, " Press any key to continue");
    prefresh(pad, 30, 0, 0, 0, 10, 45);
    keypad(pad, TRUE);
    raw();
    wgetch(pad);

    delwin(pad);
}

#if HAVE_CLIPBOARD
void clipboardTest(WINDOW *win)
{
    static const char *text =
        "This string placed in clipboard by PDCurses test program, testcurs.";
    char *ptr = NULL;
    long i, length = 0;

    INTENTIONALLY_UNUSED_PARAMETER( win);
    mvaddstr(1, 1,
             "This test will display the contents of the system clipboard");

    Continue2();

    scrollok(stdscr, TRUE);
    i = PDC_getclipboard(&ptr, &length);

    switch(i)
    {
    case PDC_CLIP_ACCESS_ERROR:
        mvaddstr(3, 1, "There was an error accessing the clipboard");
        refresh();
        break;

    case PDC_CLIP_MEMORY_ERROR:
        mvaddstr(3, 1,
            "Unable to allocate memory for clipboard contents");
        break;

    case PDC_CLIP_EMPTY:
        mvaddstr(3, 1, "There was no text in the clipboard");
        break;

    default:
        wsetscrreg(stdscr, 0, LINES - 1);
        clear();
        mvaddstr(1, 1, "Clipboard contents...");
        mvprintw(2, 1, "%s\n", ptr);
        PDC_freeclipboard( ptr);
    }

    Continue2();

    clear();
    mvaddstr(1, 1,
        "This test will place the following string in the system clipboard:");
    mvaddstr(2, 1, text);

    i = PDC_setclipboard(text, (long)strlen(text));

    switch(i)
    {
    case PDC_CLIP_ACCESS_ERROR:
        mvaddstr(3, 1, "There was an error accessing the clipboard");
        break;

    case PDC_CLIP_MEMORY_ERROR:
        mvaddstr(3, 1, "Unable to allocate memory for clipboard contents");
        break;

    default:
        mvaddstr(3, 1, "The string was placed in the clipboard successfully");
    }

    Continue2();
}
#endif /* HAVE_CLIPBOARD */

void curTest(void)
{
    do {
        int c = getch();

#if defined (PDCURSES) || defined (NCURSES_VERSION)
#ifdef __linux
        const char *screen_dump_filename = "/tmp/screen.xyz";
#else
        const char *screen_dump_filename = "screen.xyz";
#endif

        if (c == KEY_UP)
            move(getcury(stdscr) - 1, getcurx(stdscr));
        else if (c == KEY_DOWN)
            move(getcury(stdscr) + 1, getcurx(stdscr));
        else if (c == KEY_LEFT)
            move(getcury(stdscr), getcurx(stdscr) - 1);
        else if (c == KEY_RIGHT)
            move(getcury(stdscr), getcurx(stdscr) + 1);
        else if (c == 'i')              /* cycle cursor visibility from */
        {                 /* 1 = low to 2 = high to 0 =off to low again */
            const int curr_visibility = curs_set( 1);

            if( curr_visibility == 1 || curr_visibility == 2)
               curs_set( (curr_visibility + 1) % 3);
        }
        else if( c == 's')
            scr_dump( screen_dump_filename);
        else if( c == 'r')
        {
            scr_restore( screen_dump_filename);
            refresh( );
        }
        else
#endif
            break;
    } while (TRUE);
}

void acsTest(WINDOW *win)
{
    static const char *acs_names[] =
    {
        "ACS_ULCORNER", "ACS_URCORNER", "ACS_LLCORNER", "ACS_LRCORNER",
        "ACS_LTEE", "ACS_RTEE", "ACS_TTEE", "ACS_BTEE", "ACS_HLINE",
        "ACS_VLINE", "ACS_PLUS",
        "ACS_D_ULCORNER", "ACS_D_URCORNER", "ACS_D_LLCORNER", "ACS_D_LRCORNER",
        "ACS_D_LTEE", "ACS_D_RTEE", "ACS_D_TTEE", "ACS_D_BTEE", "ACS_D_HLINE",
        "ACS_D_VLINE", "ACS_D_PLUS",

        "ACS_T_ULCORNER", "ACS_T_URCORNER", "ACS_T_LLCORNER", "ACS_T_LRCORNER",
        "ACS_T_LTEE", "ACS_T_RTEE", "ACS_T_TTEE", "ACS_T_BTEE", "ACS_T_HLINE",
        "ACS_T_VLINE", "ACS_T_PLUS",

        "ACS_SD_ULCORNER", "ACS_SD_URCORNER", "ACS_SD_LLCORNER",
        "ACS_SD_LRCORNER", "ACS_SD_LTEE",
        "ACS_SD_RTEE", "ACS_SD_TTEE", "ACS_SD_BTEE", "ACS_SD_PLUS",
        "ACS_DS_ULCORNER", "ACS_DS_URCORNER", "ACS_DS_LLCORNER",
        "ACS_DS_LRCORNER", "ACS_DS_LTEE", "ACS_DS_RTEE", "ACS_DS_TTEE",
        "ACS_DS_BTEE", "ACS_DS_PLUS",
        "ACS_S1",
        "ACS_S3", "ACS_S7",
        "ACS_S9", "ACS_DIAMOND",
        "ACS_CLUB", "ACS_SPADE", "ACS_HEART",
        "ACS_LTBOARD",
        "ACS_BOARD", "ACS_CKBOARD", "ACS_DEGREE", "ACS_PLMINUS",
        "ACS_BULLET",
        "ACS_SM_BULLET", "ACS_MED_BULLET", "ACS_WHITE_BULLET",
        "ACS_PILCROW", "ACS_SECTION", "ACS_SMILE", "ACS_REV_SMILE",
        "ACS_LARROW", "ACS_RARROW", "ACS_UARROW", "ACS_DARROW",
        "ACS_LANTERN", "ACS_BLOCK",
        "ACS_LEQUAL", "ACS_GEQUAL", "ACS_NEQUAL",
        "ACS_PI",  "ACS_STERLING",
        "ACS_CENT", "ACS_YEN", "ACS_PESETA",
        "ACS_ALPHA", "ACS_BETA", "ACS_GAMMA", "ACS_UP_SIGMA",
        "ACS_LO_SIGMA", "ACS_MU", "ACS_TAU", "ACS_UP_PHI", "ACS_LO_PHI",
        "ACS_OMEGA", "ACS_DELTA", "ACS_INFINITY", "ACS_THETA", "ACS_EPSILON",
        "ACS_INTERSECT", "ACS_SUP2", "ACS_SUP_N", "ACS_TRIPLE_BAR",
        "ACS_APPROX_EQ", "ACS_SQUARE_ROOT", "ACS_NOT", "ACS_REV_NOT",
        "ACS_HALF", "ACS_QUARTER", "ACS_DIVISION",
        "ACS_UP_INTEGRAL", "ACS_LO_INTEGRAL",
        "ACS_UBLOCK", "ACS_BBLOCK",
        "ACS_LBLOCK", "ACS_RBLOCK",
        "ACS_A_ORDINAL", "ACS_O_ORDINAL",
        "ACS_INV_BANG", "ACS_INV_QUERY",
        "ACS_LEFT_ANG_QU", "ACS_RIGHT_ANG_QU",
        "ACS_CENTER_SQU", "ACS_F_WITH_HOOK"
    };

    const chtype acs_values[] =
    {
        ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER,
        ACS_LTEE, ACS_RTEE, ACS_TTEE, ACS_BTEE, ACS_HLINE,
        ACS_VLINE, ACS_PLUS,

#ifdef ACS_D_ULCORNER
        ACS_D_ULCORNER, ACS_D_URCORNER, ACS_D_LLCORNER, ACS_D_LRCORNER,
        ACS_D_LTEE, ACS_D_RTEE, ACS_D_TTEE, ACS_D_BTEE, ACS_D_HLINE,
        ACS_D_VLINE, ACS_D_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

#ifdef ACS_T_ULCORNER
        ACS_T_ULCORNER, ACS_T_URCORNER, ACS_T_LLCORNER, ACS_T_LRCORNER,
        ACS_T_LTEE, ACS_T_RTEE, ACS_T_TTEE, ACS_T_BTEE, ACS_T_HLINE,
        ACS_T_VLINE, ACS_T_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

#ifdef ACS_SD_ULCORNER
        ACS_SD_ULCORNER, ACS_SD_URCORNER, ACS_SD_LLCORNER,
        ACS_SD_LRCORNER, ACS_SD_LTEE,
        ACS_SD_RTEE, ACS_SD_TTEE, ACS_SD_BTEE, ACS_SD_PLUS,
        ACS_DS_ULCORNER, ACS_DS_URCORNER, ACS_DS_LLCORNER,
        ACS_DS_LRCORNER, ACS_DS_LTEE, ACS_DS_RTEE, ACS_DS_TTEE,
        ACS_DS_BTEE, ACS_DS_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
        ACS_S1,
#ifdef ACS_S3
        ACS_S3, ACS_S7,
#else
        0, 0,
#endif
        ACS_S9, ACS_DIAMOND,
#ifdef ACS_CLUB
        ACS_CLUB, ACS_SPADE, ACS_HEART, ACS_LTBOARD,
#else
        0, 0, 0, 0,
#endif
        ACS_BOARD, ACS_CKBOARD, ACS_DEGREE, ACS_PLMINUS, ACS_BULLET,
#ifdef ACS_SM_BULLET
        ACS_SM_BULLET, ACS_MED_BULLET, ACS_WHITE_BULLET,
        ACS_PILCROW, ACS_SECTION, ACS_SMILE, ACS_REV_SMILE,
#else
        0, 0, 0, 0, 0, 0, 0,
#endif
        ACS_LARROW, ACS_RARROW, ACS_UARROW, ACS_DARROW,
        ACS_LANTERN, ACS_BLOCK,
#ifdef ACS_LEQUAL
        ACS_LEQUAL, ACS_GEQUAL, ACS_NEQUAL,
        ACS_PI,  ACS_STERLING,
#else
        0, 0, 0, 0, 0,
#endif
#ifdef ACS_CENT
        ACS_CENT, ACS_YEN, ACS_PESETA,
        ACS_ALPHA, ACS_BETA, ACS_GAMMA, ACS_UP_SIGMA,
        ACS_LO_SIGMA, ACS_MU, ACS_TAU, ACS_UP_PHI, ACS_LO_PHI,
        ACS_OMEGA, ACS_DELTA, ACS_INFINITY, ACS_THETA, ACS_EPSILON,
        ACS_INTERSECT, ACS_SUP2, ACS_SUP_N, ACS_TRIPLE_BAR,
        ACS_APPROX_EQ, ACS_SQUARE_ROOT, ACS_NOT, ACS_REV_NOT,
        ACS_HALF, ACS_QUARTER, ACS_DIVISION,
        ACS_UP_INTEGRAL, ACS_LO_INTEGRAL,
        ACS_UBLOCK, ACS_BBLOCK,
        ACS_LBLOCK, ACS_RBLOCK,
        ACS_A_ORDINAL, ACS_O_ORDINAL,
        ACS_INV_BANG, ACS_INV_QUERY,
        ACS_LEFT_ANG_QU, ACS_RIGHT_ANG_QU,
        ACS_CENTER_SQU, ACS_F_WITH_HOOK
#endif
    };

#if HAVE_WIDE && defined( WACS_S1)
    const cchar_t *wacs_values[] =
    {
        WACS_ULCORNER, WACS_URCORNER, WACS_LLCORNER, WACS_LRCORNER,
        WACS_LTEE, WACS_RTEE, WACS_TTEE, WACS_BTEE, WACS_HLINE,
        WACS_VLINE, WACS_PLUS,

#ifdef WACS_D_ULCORNER
        WACS_D_ULCORNER, WACS_D_URCORNER, WACS_D_LLCORNER, WACS_D_LRCORNER,
        WACS_D_LTEE, WACS_D_RTEE, WACS_D_TTEE, WACS_D_BTEE, WACS_D_HLINE,
        WACS_D_VLINE, WACS_D_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

#ifdef WACS_T_ULCORNER
        WACS_T_ULCORNER, WACS_T_URCORNER, WACS_T_LLCORNER, WACS_T_LRCORNER,
        WACS_T_LTEE, WACS_T_RTEE, WACS_T_TTEE, WACS_T_BTEE, WACS_T_HLINE,
        WACS_T_VLINE, WACS_T_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

#ifdef WACS_SD_ULCORNER
        WACS_SD_ULCORNER, WACS_SD_URCORNER, WACS_SD_LLCORNER,
        WACS_SD_LRCORNER, WACS_SD_LTEE,
        WACS_SD_RTEE, WACS_SD_TTEE, WACS_SD_BTEE, WACS_SD_PLUS,
        WACS_DS_ULCORNER, WACS_DS_URCORNER, WACS_DS_LLCORNER,
        WACS_DS_LRCORNER, WACS_DS_LTEE, WACS_DS_RTEE, WACS_DS_TTEE,
        WACS_DS_BTEE, WACS_DS_PLUS,
#else
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
        WACS_S1,
#ifdef WACS_S3
        WACS_S3, WACS_S7,
#else
        0, 0,
#endif
        WACS_S9, WACS_DIAMOND,
#ifdef WACS_CLUB
        WACS_CLUB, WACS_SPADE, WACS_HEART, WACS_LTBOARD,
#else
        0, 0, 0, 0,
#endif
        WACS_BOARD, WACS_CKBOARD, WACS_DEGREE, WACS_PLMINUS, WACS_BULLET,
#ifdef WACS_SM_BULLET
        WACS_SM_BULLET, WACS_MED_BULLET, WACS_WHITE_BULLET,
        WACS_PILCROW, WACS_SECTION, WACS_SMILE, WACS_REV_SMILE,
#else
        0, 0, 0, 0, 0, 0, 0,
#endif
        WACS_LARROW, WACS_RARROW, WACS_UARROW, WACS_DARROW,
        WACS_LANTERN, WACS_BLOCK,
#ifdef WACS_LEQUAL
        WACS_LEQUAL, WACS_GEQUAL, WACS_NEQUAL,
        WACS_PI,  WACS_STERLING,
#else
        0, 0, 0, 0, 0,
#endif
#ifdef WACS_CENT
        WACS_CENT, WACS_YEN, WACS_PESETA,
        WACS_ALPHA, WACS_BETA, WACS_GAMMA, WACS_UP_SIGMA,
        WACS_LO_SIGMA, WACS_MU, WACS_TAU, WACS_UP_PHI, WACS_LO_PHI,
        WACS_OMEGA, WACS_DELTA, WACS_INFINITY, WACS_THETA, WACS_EPSILON,
        WACS_INTERSECT, WACS_SUP2, WACS_SUP_N, WACS_TRIPLE_BAR,
        WACS_APPROX_EQ, WACS_SQUARE_ROOT, WACS_NOT, WACS_REV_NOT,
        WACS_HALF, WACS_QUARTER, WACS_DIVISION,
        WACS_UP_INTEGRAL, WACS_LO_INTEGRAL,
        WACS_UBLOCK, WACS_BBLOCK,
        WACS_LBLOCK, WACS_RBLOCK,
        WACS_A_ORDINAL, WACS_O_ORDINAL,
        WACS_INV_BANG, WACS_INV_QUERY,
        WACS_LEFT_ANG_QU, WACS_RIGHT_ANG_QU,
        WACS_CENTER_SQU, WACS_F_WITH_HOOK
#endif               /* #if WACS_CENT */
    };
#endif               /* #ifdef WACS_S1   */

#if HAVE_WIDE
    static const wchar_t russian[] = {0x0420, 0x0443, 0x0441, 0x0441,
        0x043a, 0x0438, 0x0439, L' ', 0x044f, 0x0437, 0x044b, 0x043a, 0};

    static const wchar_t greek[] = {0x0395, 0x03bb, 0x03bb, 0x03b7,
        0x03bd, 0x03b9, 0x03ba, 0x03ac, 0};

    static const wchar_t georgian[] = {0x10e5, 0x10d0, 0x10e0, 0x10d7,
        0x10e3, 0x10da, 0x10d8, L' ', 0x10d4, 0x10dc, 0x10d0, 0};

    static const wchar_t fullwidth[] = { 0xff26, 0xff55, 0xff4c, 0xff4c,
        0xff57, 0xff49, 0xff44, 0xff54, 0xff48, 0 };  /* "Fullwidth" */

    static const wchar_t combining_marks[] = { L'C', L'o', 0x35c, L'm',
                   L'b', 0x30a, L'i', L'n', L'i', 0x304, L'n', 0x30b, 0x329,
                   L'g', 0x310,
                   L' ', L'C', 0x338, L'h', 0x306,  L'a', 0x361, L'r', L's',
                   0x30e, 0x348, 0 };

#endif

    int i, idx, tmarg = 1, ncols = (COLS - 4) / 19;
    int col_size = (COLS - 4) / ncols;
    int n_items = 0;
    int n_rows = LINES / 2 - 4;

    INTENTIONALLY_UNUSED_PARAMETER( win);
    for( i = 0; i < (int)( sizeof( acs_values) / sizeof( acs_values[0])); i++)
       if( acs_values[i])
          n_items++;
    i = idx = 0;
    while( i < n_items)
    {
        int j, xloc = 3;

        attrset(A_BOLD);
        mvaddstr( 1, (COLS - 23) / 2, "Alternate Character Set");
        attrset(A_NORMAL);
        tmarg = 4;
        while( i < n_items && xloc < COLS - col_size)
        {
            for( j = 0; i < n_items && j < n_rows; j++, i++)
            {
                move( j * 2 + tmarg, xloc);
                while( !acs_values[idx])
                    idx++;
                addch(acs_values[idx]);
                printw(" %s", acs_names[idx]);
                idx++;
            }
            xloc += col_size;
        }

        mvaddstr( tmarg + n_rows * 2, 3, curses_version( ));
        move( tmarg + n_rows * 2 + 1, 3);
        printw( "sizeof( chtype) = %d; sizeof( mmask_t) = %d",
                           (int)sizeof( chtype), (int)sizeof( mmask_t));
        mvaddstr(tmarg + n_rows * 2 + 2, 3, "Press any key to continue");
        curTest( );
        clear( );
    }

#if HAVE_WIDE
    n_items = 0;
    for( i = 0; i < (int)( sizeof( wacs_values) / sizeof( wacs_values[0])); i++)
       if( wacs_values[i])
          n_items++;
    i = idx = 0;
    while( i < n_items)
    {
        int j, xloc = 3;

        attrset(A_BOLD);
        mvaddstr( 1, (COLS - 28) / 2, "Wide Alternate Character Set");
        attrset(A_NORMAL);
        tmarg = 4;
#ifdef WACS_S1
        while( i < n_items && xloc < COLS - col_size)
        {
            for( j = 0; i < n_items && j < n_rows; j++, i++)
            {
                move( j * 2 + tmarg, xloc);
                while( !wacs_values[idx])
                    idx++;
                add_wch( wacs_values[idx]);
                printw(" W%s", acs_names[idx]);
                idx++;
            }
            xloc += col_size;
        }
#endif
    /* Spanish, Russian, Greek, Georgian, fullwidth, combining */

        tmarg += n_rows * 2;
        mvaddwstr(tmarg, COLS / 8 - 5, L"Espa\xf1ol");
        mvaddwstr(tmarg, 3 * (COLS / 8) - 5, russian);
        mvaddwstr(tmarg, 5 * (COLS / 8) - 5, greek);
        mvaddwstr(tmarg, 7 * (COLS / 8) - 5, georgian);
        mvaddwstr(tmarg + 1, COLS / 8 - 5, fullwidth);

        mvaddwstr(tmarg + 1, 3 * (COLS / 8) - 5, combining_marks);
#ifndef CHTYPE_32
        mvaddch( tmarg + 1, 7 * (COLS / 8) - 5, (chtype)0x1d11e);
#endif            /* U+1D11E = musical symbol G clef */

        mvaddstr(tmarg + 2, 3, "Press any key to continue");
        curTest( );
        clear( );
    }
#endif
}

void attrTest(WINDOW *win)
{
    const int tmarg = (LINES > 20 ? (LINES - 20) / 2 : 0);
    const int col1 = (COLS - 36) / 2, col2 = col1 + 20;
    const char *attr_names[] = {
         "Right", "Left", "Underline", "Reverse", "Bold", "Blink", "Top",
#ifdef WA_ITALIC
         "Italic",
#endif
#ifdef WA_STRIKEOUT
         "Strikeout",
#endif
         "Dim", "Horizontal", "Vertical", "Invis", "Low", "Protect",
         "Standout" };
    const attr_t attr_masks[] = {
        WA_RIGHT, WA_LEFT, WA_UNDERLINE, WA_REVERSE, WA_BOLD, WA_BLINK, WA_TOP,
#ifdef WA_ITALIC
        WA_ITALIC,
#endif
#ifdef WA_STRIKEOUT
         WA_STRIKEOUT,
#endif
        WA_DIM, WA_HORIZONTAL, WA_VERTICAL, WA_INVIS, WA_LOW, WA_PROTECT,
        WA_STANDOUT };
    const size_t n_attribs = sizeof( attr_names) / sizeof( attr_names[0]);
    size_t i;

    INTENTIONALLY_UNUSED_PARAMETER( win);
    attrset(A_BOLD);
    mvaddstr(tmarg, (COLS - 20) / 2, "Character Attributes");
    attrset(A_NORMAL);

    refresh();

#ifdef PDCURSES
    PDC_set_blink(TRUE);
    PDC_set_bold(TRUE);
#endif

#ifdef A_ITALIC
    attrset(A_ITALIC);
    mvaddstr(tmarg + 3, col1, "A_ITALIC");
#endif

    attrset(A_BOLD);
    mvaddstr(tmarg + 5, col1, "A_BOLD");

    attrset(A_BLINK);
    mvaddstr(tmarg + 7, col1, "A_BLINK");

    attrset(A_REVERSE);
    mvaddstr(tmarg + 9, col1, "A_REVERSE");

    attrset(A_STANDOUT);
    mvaddstr(tmarg + 11, col1, "A_STANDOUT");

    attrset(A_UNDERLINE);
    mvaddstr(tmarg + 13, col1, "A_UNDERLINE");

#if PDC_COLOR_BITS >= 11
    attrset(A_STRIKEOUT);
    mvaddstr(tmarg + 15, col1, "A_STRIKEOUT");
#endif

    attr_set( WA_TOP, 0, NULL);
    mvaddstr(tmarg + 15, col2, "A_TOP");

    attr_set( WA_DIM, 0, NULL);
    mvaddstr(tmarg + 17, col2, "A_DIM");

#ifdef WA_ITALIC
    attr_set( WA_ITALIC | WA_UNDERLINE, 0, NULL);
    mvaddstr(tmarg + 3, col2, "Underlined Italic");
#endif

    attrset( A_BOLD | A_UNDERLINE);
    mvaddstr(tmarg + 5, col2, "Underlined Bold");

    attrset( A_BLINK | A_UNDERLINE);
    mvaddstr(tmarg + 7, col2, "Underlined Blink");

    attr_set( WA_LEFT, 0, NULL);
    mvaddstr(tmarg + 9, col2, "A_LEFT");

    attr_set( WA_RIGHT, 0, NULL);
    mvaddstr(tmarg + 11, col2, "A_RIGHT");

    attrset(A_BLINK|A_REVERSE);
    mvaddstr(tmarg + 13, col2, "Reverse Blink");
    attrset(A_NORMAL);

    mvaddstr(tmarg + 19, 5, "This platform claims to support:");
    for( i = 0; i < n_attribs; i++)
        if( term_attrs( ) & attr_masks[i])
           {
           if( getcurx( stdscr) + 10 > COLS)
               move( getcury( stdscr) + 1, 10);
           addstr( " ");
           addstr( attr_names[i]);
           }

    mvaddstr(tmarg + 17, 3, "Press any key to continue");
    curTest();

#ifdef PDCURSES
    PDC_set_bold(FALSE);
    PDC_set_blink(FALSE);
#endif
}

#if HAVE_COLOR
void remap(int tmarg, const short *colors)
{
    struct
    {
        short red, green, blue;
    } orgcolors[16];
    short i, maxcol = (COLORS >= 16) ? 16 : 8;

    for (i = 0; i < maxcol; i++)
        color_content(i, &(orgcolors[i].red),
                         &(orgcolors[i].green),
                         &(orgcolors[i].blue));

    attrset(A_BOLD);
    mvaddstr(tmarg, (COLS - 22) / 2, " init_color() Example ");
    attrset(A_NORMAL);

    refresh();

    for (i = 0; i < 8; i++)
    {
        init_color(colors[i], (short)( i * 125), 0, (short)( i * 125));

        if (COLORS >= 16)
            init_color((short)( colors[i] + 8), 0, (short)( i * 125), 0);
    }

    mvaddstr(tmarg + 19, 3, "Press any key to continue");
    curTest();

    for (i = 0; i < maxcol; i++)
        init_color(i, orgcolors[i].red,
                      orgcolors[i].green,
                      orgcolors[i].blue);
}

static void show_color_cube( int tmarg)
{
    short i, x, y, z, lmarg = (short)(COLS - 77) / 2;

    erase();
    if( lmarg < 0)
      lmarg = 0;

    curs_set(0);

    attrset(A_BOLD);
    mvaddstr(tmarg, (COLS - 15) / 2, "Extended Colors");
    attrset(A_NORMAL);

    mvaddstr(tmarg + 2, lmarg, "6x6x6 Color Cube (16-231):");

    mvaddstr(tmarg + 4,  lmarg, "Blk      Red");
    mvaddstr(tmarg + 11, lmarg, "Blue    Mgta");
    if( COLS >= 77)
    {
        mvaddstr(tmarg + 4,  lmarg + 65, "Grn      Yel");
        mvaddstr(tmarg + 11, lmarg + 65, "Cyan   White");
    }
    else
    {
        mvaddstr(tmarg + 11, COLS - 12, "Grn      Yel");
        mvaddstr(tmarg + 18, COLS - 12, "Cyan   White");
    }

    for (i = 16; i < 256; i++)
        init_pair(i, COLOR_BLACK, i);

    for (i = 16, x = 0; x < 6; x++)
        for (z = 0; z < 6; z++)
            for (y = 0; y < 6; y++)
            {
                chtype ch = ' ' | COLOR_PAIR(i++);
                int line = tmarg + 5 + y, col = z * 13 + x * 2 + lmarg;

                if( z == 5 && COLS < 77)
                {
                    line += 7;
                    col += COLS - 77;
                }
                mvaddch( line, col, ch);
                addch(ch);
            }

    mvaddstr(tmarg + 13, lmarg, "Greyscale (232-255):");

    for (x = 0; x < 24; x++)
    {
        chtype ch = ' ' | COLOR_PAIR(232 + x);

        mvaddch(tmarg + 15, x * 2 + lmarg, ch);
        addch(ch);
    }

    refresh();
    curs_set(1);

    mvaddstr(tmarg + 19, 3, "Press any key to continue");
    curTest();
}

#if defined( __PDCURSESMOD__)
static void supergradient(int tmarg)
{
    int i, j, pair_num = 256;

    erase();
    refresh();
    for( i = 0; i < LINES; i++)
    {
        const int green1 = ((i * 2 + 1) * 255 / (LINES * 2)) << 8;
        const int green2 = (  (i * 2)   * 255 / (LINES * 2)) << 8;

        move( i, 0);
        for( j = 0; j < COLS && pair_num < COLOR_PAIRS; j++)
        {
            const int red = (j * 255 / COLS);

            init_extended_pair( pair_num, red + green1 + 256, red + green2 + 256);
            attrset( COLOR_PAIR( pair_num));
            addch( ACS_BBLOCK);
            pair_num++;
        }
    }
    refresh();
    curs_set(1);

    attrset( COLOR_PAIR( 0));
    mvaddstr(tmarg, 3, " Generalized gradients ");
    mvaddstr( 0, 1, "Black");
    mvaddstr( 0, COLS - 4, "Red");
    mvaddstr( LINES - 1, COLS - 7, "Yellow");
    mvaddstr( LINES - 1, 1, "Green");

    mvaddstr(tmarg + 19, 3, "Press any key to continue");
    curTest( );
}
#endif


void gradient(int tmarg)
{
    int i;
    short cnum = 256, pnum = 16;

    erase();
    refresh();

    curs_set(0);

    attrset(A_BOLD);
    mvaddstr(tmarg, (COLS - 17) / 2, "Colors Beyond 256");
    attrset(A_NORMAL);

    for (i = 0; i < 6; i++)
    {
        int j;
        const char *output_text[6] = {
            "Red on green to white on black | "
            "   (gradients work just as well with",
            "Blue on yellow to black on red | "
            "palettes, if you have enough colors)",
            "White on red to green on blue,  underlined (if available)",
            "We can keep going on and on until we "
            "run out of color pairs or colors.",
            "Some platforms will have plenty of both "
            "and this won't be a real problem. ",
            "Others can be made to work that way "
            "without too much trouble." };

        const int len = (int)strlen(output_text[i]);

        move(tmarg + 3 + i, (COLS - 69) / 2);
        for (j = 0; j < len && cnum < COLORS && pnum < COLOR_PAIRS; j++)
        {
            const short oval = (short)( j * 1000 / len);
            const short reverse = 1000 - oval;

            if (!i || i == 3)
            {
                init_color(cnum,  (short)( i ? 0 : 1000), oval, oval);
                cnum++;
                init_color( cnum, (short)( i ? 1000 : 0), reverse, 0);
            }
            else if (i == 1 || i == 4)
            {
                init_color( cnum, (short)( i == 4 ? 1000 : 0), 0, reverse);
                cnum++;
                init_color( cnum, (short)( i == 4 ? 0 : 1000), reverse, 0);
            }
            else if( i == 2 || i == 5)
            {
                init_color(cnum, reverse, (short)( i == 2 ? 1000 : 0), reverse);
                cnum++;
                if( i == 2)
                   init_color(cnum, reverse, 0, oval);
                else
                   init_color(cnum, oval, oval, oval);
            }
            init_pair(pnum, (short)( cnum - 1), cnum);
            cnum++;
            attrset(COLOR_PAIR(pnum));
            if (i == 2)
                attron(A_UNDERLINE);
            else
                attroff(A_UNDERLINE);
            addch(output_text[i][j]);
            pnum++;
        }
    }

    refresh();
    curs_set(1);

    attrset(A_NORMAL);
    if( cnum >= COLORS || pnum >= COLOR_PAIRS)
       mvaddstr(tmarg + 18, 3, "RAN OUT OF COLORS");
    mvaddstr(tmarg + 19, 3, "Press any key to continue");
    curTest();
}

void colorTest(WINDOW *win)
{
    static const short colors[] =
    {
        COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_BLUE,
        COLOR_CYAN, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
    };

    static const char *colornames[] =
    {
        "COLOR_BLACK", "COLOR_RED", "COLOR_GREEN", "COLOR_BLUE",
        "COLOR_CYAN", "COLOR_MAGENTA", "COLOR_YELLOW", "COLOR_WHITE"
    };

    chtype fill = ACS_BLOCK;
    bool widecol = (COLORS >= 16);

    int i, j, tmarg, col1, col2, col3;

    INTENTIONALLY_UNUSED_PARAMETER( win);
    if (!has_colors())
        return;

    tmarg = (LINES - 19) / 2;
    col1 = (COLS - 60) / 2;
    col2 = col1 + 20;
    col3 = col2 + 20;

    attrset(A_BOLD);
    mvaddstr(tmarg, (COLS - 22) / 2, "Color Attribute Macros");
    attrset(A_NORMAL);

    if (widecol)
    {
        mvaddstr(tmarg + 3, col2 + 3, "Colors 0-7");
        mvaddstr(tmarg + 3, col3 + 2, "Colors 8-15");
    }
    else
    {
        mvaddstr(tmarg + 3, col2 + 4, "A_NORMAL");
        mvaddstr(tmarg + 3, col3 + 5, "A_BOLD");
    }

    for (i = 0; i < 8; i++)
    {
        init_pair((short)( i + 4), colors[i], COLOR_BLACK);
        if (widecol)
            init_pair((short)( i + 12), (short)( colors[i] + 8), COLOR_BLACK);

        mvaddstr(tmarg + i + 5, col1, colornames[i]);

        for (j = 0; j < 16; j++)
        {
            mvaddch(tmarg + i + 5, col2 + j, fill | COLOR_PAIR(i + 4));
            mvaddch(tmarg + i + 5, col3 + j, fill | (widecol ?
                    COLOR_PAIR(i + 12) : (COLOR_PAIR(i + 4) | A_BOLD) ));
        }
    }

    mvprintw(tmarg + 15, col1, "COLORS = %d", COLORS);
    mvprintw(tmarg + 16, col1, "COLOR_PAIRS = %d", COLOR_PAIRS);

    mvaddstr(tmarg + 19, 3, "Press any key to continue");
    curTest();

    if (can_change_color())
        remap(tmarg, colors);

    if( COLORS >= 256 && COLOR_PAIRS >= 256)
        show_color_cube( tmarg);

#if defined( __PDCURSESMOD__)
    if (can_change_color() && (long)COLORS == ((1L << 24) + 256L)
               && LINES * COLS + 256 < COLOR_PAIRS)
        supergradient( tmarg);
#endif

    if (can_change_color() && COLORS >= 768 && COLOR_PAIRS > 256)
        gradient(tmarg);
}
#endif

#if HAVE_WIDE
void wideTest(WINDOW *win)
{
    wchar_t tmp[513];
    size_t i;

    INTENTIONALLY_UNUSED_PARAMETER( win);
    attrset(A_BOLD);
    mvaddstr(1, (COLS - 25) / 2, "Wide Character Input Test");
    attrset(A_NORMAL);

    mvaddstr(4, 1, "Enter a string: ");

    echo();

    get_wstr((wint_t *)tmp);
    addstr("\n\n String:\n\n ");
    addwstr(tmp);
    addstr("\n\n\n Hex:\n\n ");

    for (i = 0; tmp[i]; i++)
    {
        printw("%04x ", tmp[i]);
        addnwstr(tmp + i, 1);
        addstr( getcurx( stdscr) > COLS - 8 ? "\n " : "  ");
    }

    noecho();

    Continue2();
}
#endif

void display_menu(int old_option, int new_option)
{
    int lmarg = (COLS - 14) / 2,
        tmarg = (LINES - (MAX_OPTIONS + 2)) / 2;

    if (old_option == -1)
    {
        int i;

        attrset(A_BOLD);
        mvprintw( tmarg - 3, (COLS - (int)strlen( longname( ))) / 2 - 6,
                            "%s Test Program", longname( ));
        attrset(A_NORMAL);

        for (i = 0; i < MAX_OPTIONS; i++)
            mvaddstr(tmarg + i, lmarg, command[i].text);
    }
    else
        mvaddstr(tmarg + old_option, lmarg, command[old_option].text);

    attrset(A_REVERSE);
    mvaddstr(tmarg + new_option, lmarg, command[new_option].text);
    attrset(A_NORMAL);

    mvaddstr(tmarg + MAX_OPTIONS + 2, lmarg - 23,
             "Use Up and Down Arrows to select - Enter to run - Q to quit");
    refresh();
}
