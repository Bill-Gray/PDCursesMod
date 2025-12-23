/* Screen resizing can put the cursor off-screen.  */

static void _redraw_cursor( void)
{
    if( SP->cursrow >= 0 && SP->curscol >= 0
         && SP->cursrow < SP->lines && SP->curscol < SP->cols)
    {
       chtype *tptr = curscr->_y[SP->cursrow];

       if( tptr)
          PDC_transform_line_sliced( SP->cursrow, SP->curscol, 1,
                                             tptr + SP->curscol);
    }
}

/* Ports lacking hardware blinking (WinGUI,  x11,  DOSVGA,  SDLs,
framebuffer/DRM,  etc.) can use this code to handle blinking of
text and the cursor.  "When possible",  we check to see if blink_interval
milliseconds (currently set to 0.5 seconds) have elapsed since the
blinking text was drawn.  If it has,  we flip the SP->blink_state
bit and redraw all blinking text and the cursor.

Currently,  "when possible" is in PDC_napms( ) and in check_key( )
(see vt/pdckbd.c for the latter).  This does mean that if you set up
some blinking text,  and then do some processor-intensive stuff and
aren't checking for keyboard input,  the text will stop blinking. */

void PDC_check_for_blinking( void)
{
   static long prev_time = 0;
   const long t = PDC_millisecs( );
   const long blink_interval = 500L;

   if( !t || t - prev_time > blink_interval)
   {
      int x1, y, x2;

      prev_time = t;
      SP->blink_state ^= 1;
      for( y = 0; y < SP->lines; y++)
      {
         chtype *c = curscr->_y[y];

         for( x1 = 0; x1 < SP->cols; x1++)
            if( c[x1] & A_BLINK)
            {
               x2 = x1 + 1;
               while( x2 < SP->cols && (c[x2] & A_BLINK))
                  x2++;
               PDC_transform_line_sliced( y, x1, x2 - x1, c + x1);
               x1 = x2;
            }
         if( SP->visibility && y == SP->cursrow)
            _redraw_cursor( );
      }
   }
}

void PDC_gotoyx( int row, int col)
{
    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

    if( !SP || !curscr)
         return;

    if( SP->cursrow != row || SP->curscol != col)
    {            /* clear the old cursor,  if it's on-screen: */
        const int temp_visibility = SP->visibility;

        SP->visibility = 0;
        _redraw_cursor( );
        SP->visibility = temp_visibility;
               /* ...then draw the new  */
        SP->cursrow = row;
        SP->curscol = col;
        _redraw_cursor( );
    }
}
