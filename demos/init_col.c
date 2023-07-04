#include <stdlib.h>
#include <string.h>
#include <curses.h>

/* Code to test and compare behavior of ncurses and PDCurses
color routines.  (Specifically the VT flavor of PDCurses;  I
did try others,  but the main thing I was interested in was the
handling of 'default' colors.)  Some findings :

   -- I _think_ text displayed with a non-zero color pair,  with
that color pair uninitialized,  results in undefined behavior.
I've certainly found no definition,  and in ncurses,  the text is
invisible (until you select it).  In PDCurses,  it's black on
white.  I'm almost inclined to make that, say,  blinking underlined
bold red on blue... something that will enable you to see it,  but
will also make it clear to the programmer that something's wrong
and you shouldn't rely on that text being visible.

   -- For both ncurses and (VT) PDCurses,  if start_color() is
not called,  text is displayed in the default colors.  (Which
you'd expect.)  If start_color( ) is called after text has been
displayed,  ncurses will ignore start_color( ).  (In the following,
that would be after the getch( ) call has resulted in the screen
being refreshed.)  PDCurses will respond to start_color( ) at that
point by causing all text to be shown in white on black.

   I don't really know what behavior is expected.  The man page
says that "...to use these [color] routines start_color must be
called, usually right after initscr."   A couple of paragraphs
later,  it says "It is good practice to call this routine right
after initscr()."

   It appears that in ncurses,  start_color() _must_ be called
before text is displayed.

   -- The Windows console and SDL1 and SDL2 flavors of PDCurses
claim to support the concept of an 'original' background and foreground.
I've not checked yet to see how that works out in practice.  (In
theory,  they use the same underlying code for the purpose as
the VT flavor of PDCurses.  In theory,  practice and theory are
the same.  In practice,  they usually aren't.)        */

int main( const int argc, const char *argv[])
{
    int line = 1, i;
    int reset_defaults = 0;
    int show_text_without_start_color = 0;
    SCREEN *screen_pointer;

    for( i = 1; i < argc; i++)
        if( argv[i][0] == '-')
            switch( argv[i][1])
            {
                case 'd':
                    show_text_without_start_color = 1;
                    break;
#ifdef __PDCURSES__
                case 'p':
#ifdef _WIN32
                    _putenv( (char *)"PDC_PRESERVE_SCREEN=1");
#else
                    putenv( (char *)"PDC_PRESERVE_SCREEN=1");
#endif
                    break;
#endif
                case 'w':     /* switch defaults to be black text on white */
                    reset_defaults = 1;
                    break;
                default:
                    fprintf( stderr, "Unrecognized option '%s'\n", argv[i]);
                    return( -1);
            }
    screen_pointer = newterm(NULL, stdout, stdin);
    if( show_text_without_start_color)
    {
        mvaddstr( 0, 12, longname( ));
        mvaddstr( line++, 2, "Curses started,  start_color() not called yet");
        mvaddstr( line++, 2, "Foreground & background should remain at default");
        mvaddstr( line++, 2, "colors until it is.  Hit a key...");
        while( getch( ) == KEY_RESIZE)
            resize_term( 0, 0);
    }
    start_color( );
    if( reset_defaults)       /* set black text on white bkgd */
        assume_default_colors( COLOR_BLACK, COLOR_WHITE);

    noecho();

    if( !show_text_without_start_color)
        mvaddstr( 0, 12, longname( ));
    mvaddstr( line++, 2, "start_color() called,  but no colors set.  We should have");
    mvaddstr( line++, 2, reset_defaults ?
                   "a white background and black foreground everywhere." :
                   "a black background and white foreground everywhere.");
    mvaddstr( line++, 2, "Hit a key");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
    line++;

    init_pair( 1, COLOR_GREEN, COLOR_BLUE);
    attrset( COLOR_PAIR( 1));
    mvaddstr( line++, 12, "This should be  green text on blue.");
    attrset( COLOR_PAIR( 4));
    mvaddstr( line++, 2, "This uses an uninitialized color pair.  Its behavior is");
    mvaddstr( line++, 2, "undefined,  but it'll be red-on-blue in PDCurses.");
    attrset( COLOR_PAIR( 0));
    mvaddstr( line++, 2, "The above two lines were drawn with an uninitialized color");
    mvaddstr( line++, 2, "pair.  The resulting behavior is undefined.  PDCurses will use");
    mvaddstr( line++, 2, "unusual colors in hopes you'll notice your mistake.  On ncurses,");
    mvaddstr( line++, 2, "you just get black text.  Hit a key...");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);

    use_default_colors( );
    mvaddstr( line++, 2, "The background everywhere should now be default.");
    line++;
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);

    init_pair( 2, -1, COLOR_GREEN);
    attrset( COLOR_PAIR( 2));
    mvaddstr( line++, 2, "Default foreground on green background");
    init_pair( 3, COLOR_BLUE, -1);
    attrset( COLOR_PAIR( 3));
    mvaddstr( line++, 2, "Blue on default background.");
    attrset( COLOR_PAIR( 0));
    mvaddstr( line++, 2, "Hit a key");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
    line++;

    assume_default_colors( COLOR_RED, COLOR_CYAN); /* light blue background */
    mvaddstr( line++, 2, "Red on cyan (default foreground is now red; def back is cyan)");
    mvaddstr( line++, 2, "Hit a key");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);

#ifdef __PDCURSESMOD__
    reset_color_pairs( );
    mvaddstr( line++, 2, "And here's what things look like after reset_color_pairs().");
    mvaddstr( line++, 2, "We've discarded all color-pair info.  So all non-default");
    mvaddstr( line++, 2, "(i.e.,  not color pair 0) text will become black (in ncurses)");
    mvaddstr( line++, 2, "or red on blue (in PDCurses).");
    mvaddstr( line++, 2, "Hit a key");
    while( getch( ) == KEY_RESIZE)
        resize_term( 0, 0);
#endif

    endwin( );
                         /* Not really needed,  but ensures Valgrind  */
    delscreen( screen_pointer);          /* says all memory was freed */
    return( 0);
}
