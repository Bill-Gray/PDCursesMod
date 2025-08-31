#define NCURSES_WIDECHAR 1
#define HAVE_NCURSESW
#define _XOPEN_SOURCE_EXTENDED 1

#include <locale.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int PDC_wc_to_utf8( char *dest, const int32_t code);

static void show_unicode( const unsigned start, const unsigned val)
{
   unsigned i;

   clear( );
   mvprintw( 0, 0, "Unicode/UTF-8 display and demo");
   for( i = 0; i < 16; i++)
      {
      mvprintw( 2, i * 3 + 3, "%X0", i);
      mvprintw( 19, i * 3 + 3, "%X0", i);
      mvprintw( i + 3, 0, "%X", i);
      mvprintw( i + 3, 51, "%X", i);
      }
   for( i = 0; i < 256; i++)
      {
      wchar_t c[2];

      move( (i % 16) + 3, (i / 16) * 3 + 3);
      c[0] = (wchar_t)( i + start);
      c[1] = ' ';
      addnwstr( c, 2);
      }
   mvchgat( (val - start) % 16 + 3, ((val - start) / 16) * 3 + 3,
               2, A_REVERSE, 0, NULL);
   mvprintw( 21, 0, "%d = U+%x   ", val, val);
   if( val >=0x80 && val < 0x800)
      printw( "UTF-8 encoding : %02x %02x", ((val >> 6) & 0x1f) | 0xc0,
                           (val & 0x3f) | 0x80);
   if( val >=0x800 && val < 0x10000)
      printw( "UTF-8 encoding : %02x %02x %02x", ((val >> 12) & 0x1f) | 0xe0,
                                ((val >> 6) & 0x3f) | 0x80,
                                (val & 0x3f) | 0x80);
   if( val >= 0x10000)
      printw( "UTF-8 encoding : %02x %02x %02x %02x", ((val >> 18) & 0x0f) | 0xf0,
                                ((val >> 12) & 0x3f) | 0x80,
                                ((val >> 6) & 0x3f) | 0x80,
                                (val & 0x3f) | 0x80);
         {
         char tbuff[6];
         int n_bytes = PDC_wc_to_utf8( tbuff, val);

         printw( "\n%d bytes :", n_bytes);
         for( i = 0; i < (unsigned)n_bytes; i++)
            printw( " %02x", (unsigned char)tbuff[i]);
         }
   mvprintw( 23, 0, "Arrow keys/PgUp/PgDown to move around   "
                        "Enter hex digits to find a code point");
   mvprintw( 24, 0, "Hit q to quit");
}

static void show_help( void)
{
   clear( );
   mvprintw( 0, 0, "Keyboard commands are :");
   mvprintw( 2, 3, "- Arrow keys and PageUp/PageDown to move around");
   mvprintw( 3, 3, "- Enter hex digits to search for a desired codepoint");
   mvprintw( 4, 3, "- Escape or 'q' to quit");
   mvprintw( 6, 10, termname( ));
   mvprintw( 7, 10, longname( ));
   mvprintw( 8, 10, curses_version( ));
   mvprintw( 10, 3, "Hit any key:");
   getch( );
}

int main( const int argc, const char **argv)
{
    unsigned start, val = 0;
    char search[7];
    int c;
    SCREEN *screen_pointer;
    const unsigned MAX_UNICODE = 0x4fffff;

    if( argc > 1)
        sscanf( argv[1], "%x", &val);
    start = val & 0xff;
    setlocale( LC_ALL, "C.UTF-8");
    screen_pointer = newterm(NULL, stdout, stdin);
    initscr();
    noecho();
    cbreak();
    keypad( stdscr, 1);
    show_unicode( start, val);
    *search = '\0';
    while( (c = getch( )) != 'q')
      {
      int is_digit = 0;

      switch( c)
         {
         case KEY_C3:         /* "PgDn" = lower right key in keypad */
         case KEY_NPAGE:
            val += 256;
            break;
         case KEY_A3:         /* "PgUp" = upper right key in keypad */
         case KEY_PPAGE:
            if( val > 255)
               val -= 256;
            break;
         case KEY_LEFT:
            if( val > 15)
               val -= 16;
            break;
         case KEY_RIGHT:
            val += 16;
            break;
         case KEY_UP:
            if( val)
               val--;
            break;
         case KEY_DOWN:
            val++;
            break;
         default:
            if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
                                       || (c >= 'A' && c <= 'F'))
               {
               const size_t len = strlen( search);
               unsigned new_val;

               search[len] = (char)c;
               search[len + 1] = '\0';
               sscanf( search, "%x", &new_val);
               if( new_val <= MAX_UNICODE)
                  val = new_val;
               else
                  *search = '\0';
               is_digit = 1;
               }
            else
               show_help( );
         }
      if( !is_digit)
         *search = '\0';
      if( val > MAX_UNICODE)
         val = MAX_UNICODE;
      start = val & ~0xff;
      show_unicode( start, val);
      if( *search)
         mvprintw( 0, 40, "Search : %s", search);
      refresh( );
      }
    endwin( );
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return(0);
}
