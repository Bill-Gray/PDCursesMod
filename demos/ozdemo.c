/*
 *  ozdemo.c           - A demo program using PDCurses. The program
 *  (formerly newdemo)   illustrates the use of colors for text output.
 */

#include <signal.h>
#include <string.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>

int WaitForUser(void);
int SubWinTest(WINDOW *);
int BouncingBalls(WINDOW *);
void trap(int);

/* An ASCII map of Australia */

char *AusMap[17] =
{
    "                 _,__        .:",
    "         Darwin <*  /        | \\",
    "            .-./     |.     :  :,",
    "           /    |      '-._/     \\_",
    "          /     |   N.T. | '       \\",
    "        .'      |        |   Qld.  *: Brisbane",
    "     .-'        |        |           ;",
    "     |   W.A.   |----------|         |",
    "     \\          |          |--------/",
    "      |         |   S.A.   | N.S.W./",
    "Perth  *        |__.--._   |-,_   *  Sydney",
    "        \\     _.'       \\:.|Vic'-,|",
    "        >__,-'   Adelaide  \\_/*_.-'",
    "                              Melbourne",
    "                             :--,",
    "                        Tas.  '* Hobart",
    ""
};

/* Funny messages for the scroller */

char *messages[] =
{
    "Hello from the Land Down Under",
    "The Land of crocs, and a big Red Rock",
    "Where the sunflower runs along the highways",
    "The dusty red roads lead one to loneliness",
    "Blue sky in the morning and",
    "Freezing nights and twinkling stars",
    NULL
};

int WaitForUser(void)
{
    chtype ch;

    nodelay(stdscr, TRUE);
    halfdelay(50);

    ch = getch();

    nodelay(stdscr, FALSE);
    nocbreak();     /* Reset the halfdelay() value */
    cbreak();

    return (ch == '\033') ? (int)ch : 0;
}

int SubWinTest(WINDOW *win)
{
    WINDOW *swin1, *swin2, *swin3;
    int w, h, sw, sh, bx, by;

    wattrset(win, 0);
    getmaxyx(win, h, w);
    getbegyx(win, by, bx);

    sw = w / 3;
    sh = h / 3;

    if ((swin1 = derwin(win, sh, sw, 3, 5)) == NULL)
        return 1;
    if ((swin2 = subwin(win, sh, sw, by + 4, bx + 8)) == NULL)
        return 1;
    if ((swin3 = subwin(win, sh, sw, by + 5, bx + 11)) == NULL)
        return 1;

    init_pair(8, COLOR_RED, COLOR_BLUE);
    wbkgd(swin1, COLOR_PAIR(8));
    werase(swin1);
    mvwaddstr(swin1, 0, 3, "Sub-window 1");
    wrefresh(swin1);

    init_pair(9, COLOR_CYAN, COLOR_MAGENTA);
    wbkgd(swin2, COLOR_PAIR(9));
    werase(swin2);
    mvwaddstr(swin2, 0, 3, "Sub-window 2");
    wrefresh(swin2);

    init_pair(10, COLOR_YELLOW, COLOR_GREEN);
    wbkgd(swin3, COLOR_PAIR(10));
    werase(swin3);
    mvwaddstr(swin3, 0, 3, "Sub-window 3");
    wrefresh(swin3);

    delwin(swin1);
    delwin(swin2);
    delwin(swin3);
    WaitForUser();

    return 0;
}

int BouncingBalls(WINDOW *win)
{
    chtype c1, c2, c3, ball1, ball2, ball3;
    int w, h, x1, y1, xd1, yd1, x2, y2, xd2, yd2, x3, y3, xd3, yd3, c;

    curs_set(0);

    wbkgd(win, COLOR_PAIR(1));
    wrefresh(win);
    wattrset(win, 0);

    init_pair(11, COLOR_RED, COLOR_GREEN);
    init_pair(12, COLOR_BLUE, COLOR_RED);
    init_pair(13, COLOR_YELLOW, COLOR_WHITE);

    ball1 = 'O' | COLOR_PAIR(11);
    ball2 = '*' | COLOR_PAIR(12);
    ball3 = '@' | COLOR_PAIR(13);

    getmaxyx(win, h, w);

    x1 = 2 + rand() % (w - 4);
    y1 = 2 + rand() % (h - 4);
    x2 = 2 + rand() % (w - 4);
    y2 = 2 + rand() % (h - 4);
    x3 = 2 + rand() % (w - 4);
    y3 = 2 + rand() % (h - 4);

    xd1 = 1;
    yd1 = 1;
    xd2 = 1;
    yd2 = -1;
    xd3 = -1;
    yd3 = 1;

    nodelay(stdscr, TRUE);

    while ((c = getch()) == ERR)
    {
        x1 += xd1;
        if (x1 <= 1 || x1 >= w - 2)
            xd1 *= -1;

        y1 += yd1;
        if (y1 <= 1 || y1 >= h - 2)
            yd1 *= -1;

        x2 += xd2;
        if (x2 <= 1 || x2 >= w - 2)
            xd2 *= -1;

        y2 += yd2;
        if (y2 <= 1 || y2 >= h - 2)
            yd2 *= -1;

        x3 += xd3;
        if (x3 <= 1 || x3 >= w - 2)
            xd3 *= -1;

        y3 += yd3;
        if (y3 <= 1 || y3 >= h - 2)
            yd3 *= -1;

        c1 = mvwinch(win, y1, x1);
        c2 = mvwinch(win, y2, x2);
        c3 = mvwinch(win, y3, x3);

        mvwaddch(win, y1, x1, ball1);
        mvwaddch(win, y2, x2, ball2);
        mvwaddch(win, y3, x3, ball3);

        wmove(win, 0, 0);
        wrefresh(win);

        mvwaddch(win, y1, x1, c1);
        mvwaddch(win, y2, x2, c2);
        mvwaddch(win, y3, x3, c3);

        napms(150);
    }

    nodelay(stdscr, FALSE);
    ungetch(c);
    return 0;
}

/* Trap interrupt */

void trap(int sig)
{
    if (sig == SIGINT)
    {
        endwin();

        exit(0);
    }
}

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main(int argc, char **argv)
{
    WINDOW *win;
    chtype save[80], ch;
    time_t seed;
    const int width = 52, height = 22, msg_line = 9;
    int w, x, y, i, j;
    const char *versions_1 =
            " DOS, DOSVGA, OS/2, Plan 9, SDL 1/2,";
    const char *versions_2 =
            " VT, Windows console & GUI, X11";
    const char *hit_any_key =
            "       Type a key to continue or ESC to quit     ";

#ifdef XCURSES
    Xinitscr(argc, argv);
#else
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    INTENTIONALLY_UNUSED_PARAMETER( argc);
    initscr();
#endif
    seed = time((time_t *)0);
    srand( (unsigned)seed);

    start_color();
# if defined(NCURSES_VERSION) || (defined(PDC_BUILD) && PDC_BUILD > 3000)
    use_default_colors();
# endif
    cbreak();
    noecho();

    curs_set(0);

#if !defined(__TURBOC__) && !defined(OS2)
    signal(SIGINT, trap);
#endif
    noecho();

    /* refresh stdscr so that reading from it will not cause it to
       overwrite the other windows that are being created */

    refresh();

    /* Create a drawing window */

    win = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);

    if (win == NULL)
    {
        endwin();

        return 1;
    }

    for (;;)
    {
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        wbkgd(win, COLOR_PAIR(1));
        werase(win);

        init_pair(2, COLOR_RED, COLOR_RED);
        wattrset(win, COLOR_PAIR(2));
        box(win, ' ', ' ');
        wrefresh(win);

        wattrset(win, 0);

        /* Do random output of a character */

        ch = 'a';

        nodelay(stdscr, TRUE);

        for (i = 0; i < 5000; ++i)
        {
            x = rand() % (width - 2) + 1;
            y = rand() % (height - 2) + 1;

            mvwaddch(win, y, x, ch);
            wrefresh(win);

            if (getch() != ERR)
                break;

            if (i == 2000)
            {
                ch = 'b';
                init_pair(3, COLOR_CYAN, COLOR_YELLOW);
                wattrset(win, COLOR_PAIR(3));
            }
        }

        nodelay(stdscr, FALSE);

        SubWinTest(win);

        /* Erase and draw green window */

        init_pair(4, COLOR_YELLOW, COLOR_GREEN);
        wbkgd(win, COLOR_PAIR(4));
        wattrset(win, A_BOLD);
        werase(win);
        wrefresh(win);

        /* Draw RED bounding box */

        wattrset(win, COLOR_PAIR(2));
        box(win, ' ', ' ');
        wrefresh(win);

        /* Display Australia map */

        wattrset(win, A_BOLD);
        i = 0;

        while (*AusMap[i])
        {
            mvwaddstr(win, i + 1, 3, AusMap[i]);
            wrefresh(win);
            napms(100);
            ++i;
        }

        init_pair(5, COLOR_BLUE, COLOR_WHITE);
        wattrset(win, COLOR_PAIR(5) | A_BLINK);
        mvwaddstr( win, height - 5, 2, longname( ));
        mvwaddstr( win, height - 4, 2, curses_version( ));
        mvwaddstr( win, height - 3, 2, versions_1);
        mvwaddstr( win, height - 2, 2, versions_2);
        wrefresh(win);

        /* Draw running messages */

        init_pair(6, COLOR_BLACK, COLOR_WHITE);
        wattrset(win, COLOR_PAIR(6));
        w = width - 2;
        nodelay(win, TRUE);

        mvwhline(win, msg_line, 1, ' ', w);

        for (j = 0; messages[j] != NULL; j++)
        {
            char *message = messages[j];
            int msg_len = (int)strlen(message);
            int stop = 0;
            int xpos, start, count;

            for (i = 0; i <= w + msg_len; i++)
            {
                if (i < w)
                {
                    xpos = w - i;
                    start = 0;
                    count = (i > msg_len) ? msg_len : i;
                }
                else
                {
                    xpos = 0;
                    start = i - w;
                    count = (w > msg_len - start) ? msg_len - start : w;
                }

                mvwaddnstr(win, msg_line, xpos + 1, message + start, count);
                if (xpos + count < w)
                    waddstr(win, " ");

                wrefresh(win);

                if (wgetch(win) != ERR)
                {
                    flushinp();
                    stop = 1;
                    break;
                }

                napms(100);
            }

            if (stop)
                break;
        }

        j = 0;

        /*  Draw running 'A's across in RED */

        init_pair(7, COLOR_RED, COLOR_GREEN);
        wattron(win, COLOR_PAIR(7));

        for (i = 2; i < width - 4; ++i)
        {
            ch = mvwinch(win, 5, i);
            save[j++] = ch;
            ch = ch & 0x7f;
            mvwaddch(win, 5, i, ch);
        }

        wrefresh(win);

        /* Put a message up; wait for a key */

        wattrset(win, COLOR_PAIR(5));
        mvwaddstr(win, msg_line, 2, hit_any_key);
        wrefresh(win);

        if (WaitForUser() == '\033')
            break;

        /* Restore the old line */

        wattrset(win, 0);

        for (i = 2, j = 0; i < width - 4; ++i)
            mvwaddch(win, 5, i, save[j++]);

        wrefresh(win);

        BouncingBalls(win);

        /* BouncingBalls() leaves a keystroke in the queue */

        if (WaitForUser() == '\033')
            break;
    }

    endwin();

    return 0;
}
