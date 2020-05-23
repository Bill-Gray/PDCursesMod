/*
 *  newtest.c   -   Tests certain PDCurses functions,
 *    mostly those in Win32a,  including some of the
 *    new attributes for 64-bit chtype.  I wanted to be
 *    sure the PDC_set_blink and PDC_set_line_color
 *    functions worked,  and that A_OVERLINE and A_STRIKEOUT
 *    displayed properly.  Also tests "extended" SLK functions.
 *
 */
#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
#endif

#if defined (PDC_WIDE)
   #include <curses.h>
   #define HAVE_WIDE
#elif defined (HAVE_NCURSESW)
   #include <ncursesw/curses.h>
   #define HAVE_WIDE
#else
   #include <curses.h>
#endif

#include <string.h>
#include <stdio.h>
#include <locale.h>

int PDC_write_screen_to_file( const char *filename, WINDOW *win);

#ifndef A_OVERLINE
   #define A_OVERLINE   0
#endif

static const char *labels[] = {
               "Quit", "No labels", "431", "2134", "55",
               "62-really-longer-than-it-should-be-just-for-testing",
               "83", "7", "b", "25 (seven total)", "32", NULL };

static void slk_setup( const int slk_format)
{
    int i;
#ifdef PDCURSES
    static int old_format = 0xa;

    if( slk_format != old_format)
       slk_init( slk_format);
    old_format = slk_format;
#endif
    for( i = 0; labels[i]; i++)
       slk_set( i + 1, labels[i], 1);
    slk_refresh( );
}

static const char *on_off_text( const chtype attrib)
{
   return( attrib ? "On " : "Off");
}

   /* Uses the left/right/under/overline capabilities of Win32a */
   /* to ensure the text is "boxed".  */

void text_in_a_box( const char *istr)
{
   const int len = (int)strlen( istr);

#if defined( A_OVERLINE) && defined( A_UNDERLINE) && defined( A_LEFTLINE) && defined( A_RIGHTLINE)
   attron( A_OVERLINE | A_UNDERLINE | A_LEFTLINE);
   if( len == 1)
      attron( A_RIGHTLINE);
#endif
   addnstr( istr, 1);
   if( len > 1)
      {
#ifdef A_LEFTLINE
      attroff( A_LEFTLINE);
#endif
      if( len > 2)
         addnstr( istr + 1, len - 2);
#ifdef A_RIGHTLINE
      attron( A_RIGHTLINE);
#endif
      addnstr( istr + len - 1, 1);
      }
#if defined( A_OVERLINE) && defined( A_UNDERLINE) && defined( A_LEFTLINE) && defined( A_RIGHTLINE)
   attroff( A_OVERLINE | A_UNDERLINE | A_LEFTLINE | A_RIGHTLINE);
#endif
}

#define COL1 2
#define COL2 (COL1 + 30)
#define COL3 72

#define N_CURSORS 9
      /* There are nine different cursor types;  see below for the list.  */
      /* You specify two types,  and the cursor blinks between them.      */
      /* Default is between "underline" and "invisible".  Set both states */
      /* to the same value to get an unblinking cursor.                   */

#if defined( _WIN32) && !defined( __BORLANDC__)
#define PURE_WINDOWS_VERSION  1
#endif

/* Among other things,  'newtest' demonstrates how to make a Win32a
PDCurses app that is a for-real,  "pure Windows" version (instead of
a console application).  Doing this is quite easy,  and has certain
advantages.  If the app is invoked from a command prompt,  the only
difference you'll see is that the app runs separately (that is,  you
can continue to use the command prompt,  switching between it,  your
PDCurses/Win32a app,  and other processes).  Which is the main reason
I did it;  it meant that I could invoke a PDCurses-based text editor,
for example,  and still have use of the command line.

   (NOTE that,  for reasons I don't actually understand,  this happens
when the Visual C++ compiler is used.  With MinGW or OpenWatcom,  it's
still "really" a console app.)

   To do it,  we ensure that the usual main() function has an alternative
dummy_main() form,  taking the same arguments as main().  We add a
WinMain() function,  whose sole purpose is to reformulate lpszCmdLine
into argc/argv form,  and pass it on to dummy_main().  And,  of course,
we can switch back to a "normal" console app by removing the above
#define PURE_WINDOWS_VERSION line.             */

#ifdef PURE_WINDOWS_VERSION
#undef MOUSE_MOVED
#include <windows.h>

int dummy_main( int argc, char **argv);

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
   char *argv[30];
   int i, argc = 1;

   argv[0] = "newtest";
   for( i = 0; lpszCmdLine[i]; i++)
       if( lpszCmdLine[i] != ' ' && (!i || lpszCmdLine[i - 1] == ' '))
          argv[argc++] = lpszCmdLine + i;

   for( i = 0; lpszCmdLine[i]; i++)
       if( lpszCmdLine[i] == ' ')
          lpszCmdLine[i] = '\0';

   return dummy_main( argc, (char **)argv);
}

int dummy_main( int argc, char **argv)
#else       /* "usual",  console-app version: */
int main( int argc, char **argv)
#endif
{
    int quit = 0, i,  use_slk = 1;
    bool show_mouse_moves = FALSE;
#ifdef PDCURSES
    int fmt = 0xa;
    const char *title_text = "NewTest: tests various PDCurses features";
#else
    int fmt = 3;   /* for ncurses,  this is the 4-4-4 SLK format */
#endif
    int cursor_state_1 = 0, cursor_state_2 = 1;
    int cursor_y = 19, cursor_x = 51;
    int show_slk_index_line = 0;
    int redraw = 1;
    const char *extra_characters_to_show = "";
#ifdef HAVE_WIDE
    unsigned unicode_offset = 0x80;
#endif

    setlocale(LC_ALL, "");
    ttytype[0] = 25;   ttytype[1] = 90;         /* Allow 25 to 90 lines... */
    ttytype[2] = 80;   ttytype[3] = (char)200;  /* ...and 80 to 200 columns */
         /* (This program gets weird artifacts when smaller than 25x80.) */
    for( i = 1; i < argc; i++)
        if( argv[i][0] == '-')
            switch( argv[i][1])
            {
                case 's':
                    use_slk = 0;
                    break;
                case 'l':
                    setlocale( LC_ALL, argv[i] + 2);
                    break;
                case 'e':
                    extra_characters_to_show = argv[i] + 2;
                    break;
                case 'f':
                    sscanf( argv[i] + 2, "%x", (unsigned *)&fmt);
                    break;
                case 'i':
                    show_slk_index_line = 1;
                    break;
                case 'r':     /* allow user-resizable windows */
                    {
                        int min_lines, max_lines, min_cols, max_cols;

                        if( sscanf( argv[i] + 2, "%d,%d,%d,%d",
                                       &min_lines, &max_lines,
                                       &min_cols, &max_cols) == 4)
                        {
                            ttytype[0] = min_lines;
                            ttytype[1] = max_lines;
                            ttytype[2] = min_cols;
                            ttytype[3] = max_cols;
                        }
                    }
                    break;
                case 'd':     /* set window size before initscr */
                    {
                        int n_lines, n_cols;

                        if( sscanf( argv[i] + 2, "%d,%d", &n_lines,
                                    &n_cols) == 2)
                            resize_term( n_lines, n_cols);
                    }
                    break;
#ifdef PDCURSES
                case 'z':
                    traceon( );
                    PDC_debug( "Debugging is on\n");
                    break;
                case 't':
                    title_text = argv[i] + 2;
                    break;
#endif
#ifdef HAVE_WIDE
                case 'u':
                    sscanf( argv[i] + 2, "%x", &unicode_offset);
                    break;
#endif
                case 'm':
                    show_mouse_moves = TRUE;
                    break;
                default:
                    printf( "Option '%s' unrecognized\n", argv[i]);
                    break;
            }
    if( use_slk)
       slk_init( show_slk_index_line ? -fmt : fmt);
#ifdef XCURSES
    Xinitscr(argc, argv);
#else
    initscr();
#endif
    if( use_slk)
       slk_setup( show_slk_index_line ? -fmt : fmt);

    start_color();

# if defined(NCURSES_VERSION) || (defined(PDC_BUILD) && PDC_BUILD > 3000)
    use_default_colors();
# endif
    cbreak();
    noecho();
    clear();
    refresh();
#ifdef __PDCURSES__
    PDC_set_title( title_text);
#endif
    keypad( stdscr, TRUE);
    init_pair( 1, COLOR_WHITE, COLOR_BLACK);
    init_pair( 2, COLOR_BLACK, COLOR_YELLOW);

    mousemask( ALL_MOUSE_EVENTS | (show_mouse_moves ? REPORT_MOUSE_POSITION : 0), NULL);
    attrset( COLOR_PAIR( 1));
    while( !quit)
    {
        char buff[80];
        const int xmax = getmaxx( stdscr);
        const int ymax = getmaxy( stdscr);
        const int color_block_start = 54;
        int c, color_block_cols = (xmax - color_block_start) / 2;
        const int color_block_lines = 19;
        const char *cursor_state_text[N_CURSORS] = {
                  "Invisible (click to change) ",
                  "Underscore (click to change)",
                  "Block (click to change)     ",
                  "Outline (click to change)   ",
                  "Caret (click to change)     ",
                  "Half-block (click to change)",
                  "Central (click to change)   ",
                  "Cross (click to change)     ",
                  "Heavy box (click to change) " };

        if( color_block_cols < 0)
            color_block_cols = 0;
        if( redraw)
        {
            mvaddstr( 1, COL1, "'Normal' white-on-black");
            mvaddstr( 2, COL1, longname( ));
#ifdef A_DIM
            attron( A_DIM);
            mvaddstr( 15, 41, "Dimmed text");
            attroff( A_DIM);
#endif
#ifdef A_STANDOUT
            attron( A_STANDOUT);
            mvaddstr( 16, 41, "Standout text");
            attroff( A_STANDOUT);
#endif
#ifdef HAVE_WIDE
            mvaddwstr( 3, COL1, L"'N\xf3rm\xe4\x142' text,  bu\x163 w\xee\x1e0b\xea");
#endif
            attron( A_BLINK);
            sprintf( buff, "Blink %s", on_off_text( termattrs( ) & A_BLINK));

            mvaddstr( 6, 40, buff);
            attron( A_BOLD);
            mvaddstr( 8, 40, "BlinkBold");
            attrset( A_BOLD);
            sprintf( buff, "Bold %s", on_off_text( termattrs( ) & A_BOLD));
            mvaddstr( 7, 40, buff);
            attron( A_BLINK);
#ifdef A_ITALIC
            attron( A_ITALIC);
            mvaddstr( 0, COL2, "BlinkBoldItalic");
#endif
            attrset( COLOR_PAIR( 3));
            attron( A_UNDERLINE);
#ifdef HAVE_WIDE
            mvaddstr( 1, COL2, "Underlined");
            addwstr( L"WideUnder");
#endif
            attrset( COLOR_PAIR( 1));
#ifdef A_ITALIC
            attron( A_UNDERLINE | A_ITALIC);
            mvaddstr( 2, COL2, "UnderlinedItalic");
#endif
            attrset( COLOR_PAIR( 2));
            attron( A_BLINK);
            mvaddstr( 4, COL1, "Black-on-yellow blinking");

            attrset( COLOR_PAIR( 1));
            move( 4, COL2);
            text_in_a_box( "Text in a box");

#if defined( CHTYPE_64) && defined( A_STRIKEOUT)
            attrset( COLOR_PAIR( 6));
            attron( A_STRIKEOUT);
            mvaddstr( 10, 40, "Strikeout");
            attrset( COLOR_PAIR( 1));
#endif

#ifdef HAVE_WIDE
            move( 11, 40);
            text_in_a_box( "Next Ucode pg");
            if( unicode_offset)
               {
               move( 12, 40);
               text_in_a_box( "Prev Ucode pg");
               }
            mvprintw( 13, 40, "U+%04x ", unicode_offset);

#endif

            mvaddstr( 5, 1, "   0 1 2 3 4 5 6 7 8 9 a b c d e f");
            for( i = 0; i < 8; i++)
                {
                char buff[4];

                sprintf( buff, "%02x",
#ifdef HAVE_WIDE
                                (unsigned)( i * 16 + unicode_offset) & 0xff);
#else
                                (unsigned)( i * 16 + 128) & 0xff);
#endif
                mvaddstr( 6 + i, 1, buff);
                mvaddstr( 6 + i, 36, buff);
                }
            for( i = 0; i < 128; i++)
            {                 /* Show extended characters: */
#ifdef HAVE_WIDE
                wchar_t buff[2];

                buff[0] = (wchar_t)( i + unicode_offset);
                buff[1] = '\0';
                mvaddwstr( 6 + i / 16, 4 + 2 * (i % 16), buff);
#else
                move( 6 + i / 16, 4 + 2 * (i % 16));
                addch( i + 128);
#endif
                addch( ' ');
            }
#ifdef HAVE_WIDE
            if( unicode_offset == 0x80)
            {
                mvaddstr( 6, 1, "   Click on 'bold on/off or 'blink ->");
                mvaddstr( 7, 1, "   on/off' to toggle those attribs   ");
            }
#endif
            redraw = 0;
            attrset( COLOR_PAIR( 1));
            if( *extra_characters_to_show && ymax > 23)
            {
                unsigned long ival;
                int bytes_read;
                const char *tptr = extra_characters_to_show;

                move( 23, 63);
                while( sscanf( tptr, "%lx%n", &ival, &bytes_read) > 0)
                {
                    addch( (chtype)ival);
                    tptr += bytes_read;
                    if( *tptr)
                        tptr++;
                }
            }
#ifdef HAVE_WIDE
            for( i = 0; i < 6; i++)
            {
                static const wchar_t spanish[] = L"Espa\xf1ol";

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

                static const wchar_t *texts[6] = { spanish, russian, greek,
                                georgian, fullwidth, combining_marks};

                mvaddwstr( 15 + i / 2, 2 + 20 * (i % 2), texts[i]);
            }
#ifdef CHTYPE_64
             mvaddch( 17, 41, (chtype)0x1d11e);
#endif            /* U+1D11E = musical symbol G clef */
#endif
        mvaddstr( 19, 1, curses_version( ));

#ifdef MAYBE_TRY_THIS_SOMEWHERE_ELSE
        mvaddstr(  1, COL3, "Click on cursor descriptions to");
        mvaddstr(  2, COL3, "cycle through possible cursors");
        mvaddstr(  3, COL3, "Click on colors at left to change");
        mvaddstr(  4, COL3, "colors used for under/over/outlining");
        mvaddstr(  5, COL3, "Click 'Blink' at bottom to toggle");
        mvaddstr(  6, COL3, "'real' blinking vs. 'highlit' blink");
#endif
        }

        mvaddnstr( 19, color_block_start, cursor_state_text[cursor_state_1],
                                 xmax - color_block_start);
        mvaddnstr( 20, color_block_start, cursor_state_text[cursor_state_2],
                                 xmax - color_block_start);
        curs_set( (cursor_state_1 << 8) | cursor_state_2);
        for( i = 0; i < color_block_cols * color_block_lines; i++)
        {
            const int n_color_blocks = (COLOR_PAIRS < 256 ? COLOR_PAIRS : 256);

            attrset( COLOR_PAIR( i >= n_color_blocks ? 2 : i));
            if( i > 2 && i < n_color_blocks)
               init_pair((short)i, (short)i, COLOR_BLACK);
            if( !(i % color_block_cols))
               move( i / color_block_cols, color_block_start);
            attron( A_REVERSE);
            addstr( "  ");
        }
        move( cursor_y, cursor_x);
        refresh();
        c = getch( );
        attrset( COLOR_PAIR( 1));
        if( c == KEY_RESIZE)
        {
            redraw = 1;
            resize_term( 0, 0);
        }
        else if( c == KEY_F(1) || c == 27)
            quit = 1;
        else if( c == KEY_F(2))   /* toggle SLKs */
        {
            use_slk ^= 1;
            if( use_slk)
                slk_restore( );
            else
                slk_clear( );
        }
        else if( c >= KEY_F(3) && c < KEY_F(12))
        {
            sscanf( labels[c - KEY_F(1)], "%x", (unsigned *)&fmt);
            if( use_slk)
                slk_setup( show_slk_index_line ? -fmt : fmt);
        }
        if( c != KEY_MOUSE)
        {
            sprintf( buff, "Key %s", keyname( c));
            if( !memcmp( buff + 4, "UNKNOWN", 7))
                sprintf( buff + 11, " (%x)", c);
            strcat( buff, " hit                 ");
            buff[COL2 - COL1] = '\0';
            mvaddstr( 0, COL1, buff);
        }
        else
        {
            MEVENT mouse_event;
#ifdef __PDCURSES__
            nc_getmouse( &mouse_event);
#else
            getmouse( &mouse_event);
#endif
            sprintf( buff, "Mouse at %d x %d: %x     ", mouse_event.x,
                              mouse_event.y, (unsigned)mouse_event.bstate);
            cursor_x = mouse_event.x;
            cursor_y = mouse_event.y;
            mvaddstr( 0, COL1, buff);
            if( mouse_event.x >= color_block_start
                            && mouse_event.y < color_block_lines)
            {
                int new_color = (mouse_event.x - color_block_start) / 2
                              + mouse_event.y * color_block_cols;

                if( new_color >= 256)
                    new_color = -1;
#ifdef PDCURSES
                PDC_set_line_color( (short)new_color);
#endif
            }
#ifdef PDCURSES
            else if( mouse_event.x >= color_block_start)
            {
                int shift = ((mouse_event.bstate & BUTTON_MODIFIER_SHIFT) ?
                           N_CURSORS - 1 : 1);

                if( mouse_event.y == 19)  /* blink/non-blink toggle */
                    cursor_state_1 = (cursor_state_1 + shift) % N_CURSORS;
                else if( mouse_event.y == 20)  /* cycle cursor state */
                    cursor_state_2 = (cursor_state_2 + shift) % N_CURSORS;
            }
#endif
            else if( mouse_event.x >= 40 && mouse_event.x <= 52)
               switch( mouse_event.y)
               {
#ifdef HAVE_WIDE
                  case 11:
                     redraw = 1;
                     unicode_offset += 0x80;
                     break;
                  case 12:
                     if( unicode_offset)
                     {
                        redraw = 1;
                        unicode_offset -= 0x80;
                     }
                     break;
#endif
#ifdef PDCURSES
                  case 6:
                     PDC_set_blink( termattrs( ) & A_BLINK ? FALSE : TRUE);
                     redraw = 1;
                     break;
                  case 7:
                     PDC_set_bold( termattrs( ) & A_BOLD ? FALSE : TRUE);
                     redraw = 1;
                     break;
#endif
                  default:
                     break;
               }
        }
    }

    endwin();

    return 0;
}
