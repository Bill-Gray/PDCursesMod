#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <curses.h>

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

/* Code to test combining and SMP (Unicode past 64K) capabilities,
and the getcchar()/setcchar() functions.     */

#ifndef WACS_S1
int main( const int argc, const char *argv[])
{
    INTENTIONALLY_UNUSED_PARAMETER( argc);
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    fprintf( stderr, "'widetest' only works with wide-character builds.\n");
    return( -1);
}

#else

#define LINELEN 59

int main( const int argc, const char *argv[])
{
    const wchar_t *precomposed_string = L"\xc5ngstrom Pi\xf1" L"ata Fa\xe7"
               L"ade \xc6sop caf\xe9 No\xebl c\xf4te (precomposed)";
    const wchar_t *combining_string = L"A\x30angstrom\x327\x302 Pin\x303" L"ata Fac\x327"
#ifdef _WIN32
               L"ade \xc6sop cafe\x301 Noe\x308l co\x302te \xd834\xdd1e (combining)";
#else
               L"ade \xc6sop cafe\x301 Noe\x308l co\x302te \x1d11e (combining)";
#endif
    int n, i, j, shuffle[LINELEN];
    short color_pair;
    wchar_t array[10];
    cchar_t wcval;
    attr_t attrs;
    SCREEN *screen_pointer;

    INTENTIONALLY_UNUSED_PARAMETER( argc);
    INTENTIONALLY_UNUSED_PARAMETER( argv);
    setlocale(LC_ALL, ".utf8");
    screen_pointer = newterm(NULL, stdout, stdin);
    mvaddwstr( 1, 1, precomposed_string);
    mvaddwstr( 2, 1, combining_string);
    mvaddstr( 4, 2, "The top line was done using precomposed characters;  for example,");
    mvaddstr( 5, 2, "A-ring was done with U+00C5.  The lower line was done using");
    mvaddstr( 6, 2, "combining characters;  for example,  A-ring was U+0041 ('ordinary'");
    mvaddstr( 7, 2, "ASCII uppercase A) followed by U+030A (combining ring).  The test");
    mvaddstr( 8, 2, "here is that both lines should look the same.  (On some systems,");
    mvaddstr( 9, 2, "font rendering will make them not _exactly_ the same.  But they");
    mvaddstr( 10, 2, "should be recognizably similar.)");
    mvaddwstr( 12, 2, L"One character _should_ differ.  The 'm' in '\xc5ngstrom' has a");
    mvaddstr( 13, 2, "cedilla and circumflex,  to test display of more than one added");
    mvaddstr( 14, 2, "combining character.  Hit a key to continue.");
    mvaddstr( 16, 2, "Also,  the 'treble clef' symbol (Unicode U+1D11E) is added to the");
    mvaddstr( 17, 2, "second line,  to test SMP display.  This will probably not show");
    mvaddstr( 18, 2, "up correctly.  Few fonts support SMP characters.");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
    move( 4, 1);
    clrtobot( );
    for( i = 0; i < LINELEN; i++)
    {
        char buff[80];

        move( 2, i + 1);
        in_wch( &wcval);
        n = getcchar( &wcval, NULL, &attrs, &color_pair, NULL);
        getcchar( &wcval, array, &attrs, &color_pair, NULL);
        sprintf( buff, "%d: ", n);
        mvaddstr( 11 + i % (LINES - 11), 16 * (i / (LINES - 11)), buff);
        j = 0;
        while( array[j] > 0xff)
           j++;
        printw( "%c ", (char)array[j]);
        for( j = 0; array[j]; j++)
        {
            sprintf( buff, "%x ", array[j]);
            addstr( buff);
        }
    }
    mvaddstr( 5, 14, "Here, getcchar() is used to 'fetch' data about characters");
    mvaddstr( 6, 14, "on the second line,  and the wide character (or,  for the");
    mvaddstr( 7, 14, "combined cases,  characters) is/are shown.  This is used to");
    mvaddstr( 8, 14, "verify that getcchar() is actually returning the characters");
    mvaddstr( 9, 14, "it ought to.  Hit a key...");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
    move( 3, 1);
    clrtobot( );
    mvaddstr( 7, 2, "Here, getcchar() is again used to 'fetch' the same characters");
    mvaddstr( 8, 2, "from both lines,  and setcchar() is used to copy them to two");
    mvaddstr( 9, 2, "lower lines.  The copying is mixed up a bit and slowed down");
    mvaddstr( 10, 2, "to take about two seconds.");
    for( i = 0; i < LINELEN; i++)
        shuffle[i] = i;
    for( i = LINELEN - 1; i; i--)
    {                            /* ignore slight bias in shuffling */
        const int loc = rand( ) % i, tval = shuffle[loc];

        shuffle[loc] = shuffle[i];
        shuffle[i] = tval;
    }
    for( i = 0; i < LINELEN; i++)
        for( j = 1; j < 3; j++)
        {
            const int x = (j == 1 ? shuffle[i] : shuffle[shuffle[i]]) + 1;
            cchar_t wcval_out;

            move( j, x);
            in_wch( &wcval);
            getcchar( &wcval, array, &attrs, &color_pair, NULL);
            move( j + 3, x);
            setcchar( &wcval_out, array, attrs, color_pair, NULL);
            add_wch( &wcval_out);
            napms( 30);
            refresh( );
        }
    mvaddstr( 11, 2, "Done!  Hit any key");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
    endwin( );
                            /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return( -1);
}
#endif
