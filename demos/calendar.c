#include <curses.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../demos/snprintf.c"

static void _find_max_min( int *minval, int *maxval, const int *values,
                         int n_values)
{
   *minval = *maxval = values[n_values - 1];
   n_values--;
   while( n_values--)
      {
      if( *minval > *values)
         *minval = *values;
      if( *maxval < *values)
         *maxval = *values;
      }
}

/* Draws a grid using alt-charset glyphs.  There must be at least
two xvals and two yvals (if nx = ny = 2,  you just get a box).
The values need not be on-screen,  which can help if you've zoomed
in on a table that's larger than the screen.    */

int show_table( const int *xvals, const int *yvals,
            const int nx, const int ny)
{
   int min_x, min_y, max_x, max_y, x, y;

   assert( nx > 1);
   assert( ny > 1);
   _find_max_min( &min_x, &max_x, xvals, nx);
   _find_max_min( &min_y, &max_y, yvals, ny);
   y = min_y;
   if( y < 0)
      y = 0;
   while( y <= max_y && y < LINES)
      {
      int i = 0;

      while( i < ny && y != yvals[i])
         i++;
      if( i != ny)
         {
         x = min_x;
         if( x < 0)
            x = 0;
         move( y, x);
         while( x <= max_x && x < COLS)
            {
            chtype ochar = ACS_HLINE;

            i = 0;
            while( i < nx && x != xvals[i])
               i++;
            if( x == min_x)
               {
               if( y == min_y)
                  ochar = ACS_ULCORNER;
               else if( y == max_y)
                  ochar = ACS_LLCORNER;
               else
                  ochar = ACS_LTEE;
               }
            else if( x == max_x)
               {
               if( y == min_y)
                  ochar = ACS_URCORNER;
               else if( y == max_y)
                  ochar = ACS_LRCORNER;
               else
                  ochar = ACS_RTEE;
               }
            else if( i != nx)
               {
               if( y == min_y)
                  ochar = ACS_TTEE;
               else if( y == max_y)
                  ochar = ACS_BTEE;
               else
                  ochar = ACS_PLUS;
               }
            addch( ochar);
            x++;
            }
         }
      else
         {
         for( i = 0; i < nx; i++)
            if( xvals[i] >= 0 && xvals[i] < COLS)
               mvaddch( y, xvals[i], ACS_VLINE);
         }
     y++;
     }
   return( 0);
}

/* Cheap,  simple codelets for Gregorian calendar conversions.
Works from year -99999999 (about 100 million years ago) for
64-bit long integers,  and about year -5880000 with 32-bit
long integers.  */

static long _mjd_for_jan_0( const long year)
{
   return( year * 365L + (year + 99999999) / 4 - (year + 99999999) / 100
                 + (year + 99999999) / 400 - 24928941);
}

long mjd_from_dmy( const long year, const int month, const int day)
{
   const char month_offset[13] = { 0, 2, 1, 3, 4, 6, 7, 9, 11, 12, 14, 15, 17 };

   long mjd1 = _mjd_for_jan_0( year), mjd2 = _mjd_for_jan_0( year + 1);
   long rval = mjd1 + (month - 1) * 29 + month_offset[month - 1] + day;

   if( mjd2 - mjd1 == 366 && month > 2)   /* leap year,  after Feb */
      rval++;
   return( rval);
}

void dmy_from_mjd( const long mjd, long *year, int *month, int *day)
{
   long mjd1;

   *year = (long)( (mjd + 2400000 - 1721058) / 365) + 1;
   mjd1 = _mjd_for_jan_0( *year);
   assert( mjd1 >= mjd);
   while( mjd1 >= mjd)
      {
      (*year)--;
      mjd1 = _mjd_for_jan_0( *year);
      }
   *month = (mjd - mjd1) / 29 + 1;
   while( (mjd1 = mjd_from_dmy( *year, *month, 1)) > mjd)
      (*month)--;
   *day = (int)( mjd - mjd1) + 1;
}

static void show_calendar( const long mjd, const char *path)
{
   int xval[8], yval[8], i;
   int day, month, n_weeks;
   const int top_margin = 1;
   long year, mjd0, mjd1, week0, week1;
   char text[80];
   const char *month_names[13] = { NULL, "January", "February",
                 "March", "April", "May", "June", "July", "August",
                 "September", "October", "November", "December" };
   FILE *ifile;

   i = (int)strlen( path);
   while( i && path[i - 1] != '/' && path[i - 1] != '\\')
      i--;
   if( i < (int)( sizeof( text) - 10))
      {
      memcpy( text, path, i);
      strcpy( text + i, "dates.txt");
      ifile = fopen( text, "rb");
      }
   else
      ifile = NULL;
   clear( );
   dmy_from_mjd( mjd, &year, &month, &day);
   mjd0 = mjd - day + 1;         /* MJD of first of month */
   mjd1 = mjd_from_dmy( year + month / 12, month % 12 + 1, 1) - 1;
                                 /* MJD of last day of month */
   week0 = (mjd0 + 3) / 7;
   week1 = (mjd1 + 3) / 7;
   n_weeks = (int)( week1 - week0) + 1;
   for( i = 0; i < 8; i++)
     xval[i] = (COLS - 1) * i / 7;
   yval[0] = top_margin;
   for( i = 0; i <= n_weeks; i++)
     yval[i + 1] = top_margin + 2 + (LINES - top_margin - 3) * i / n_weeks;
   show_table( xval, yval, 8, n_weeks + 2);
   for( i = 0; i < 7; i++)
      {
      const char *weekdays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

      move( yval[0] + 1, xval[i] + (xval[1] - xval[0]) / 2 -1);
      addstr( weekdays[i]);
      }
   for( i = 1; i <= (int)( mjd1 - mjd0) + 1; i++)
      {
      const long curr_mjd = mjd0 + (long)i - 1;
      const int xloc = (curr_mjd + 3) % 7, yloc = (curr_mjd + 3) / 7 - (int)week0;

      move( yval[yloc + 1] + 1, xval[xloc] + 1);
      snprintf( text, sizeof( text), "%d      ", i);
      if( i == day)
         attrset( A_REVERSE);
      addstr( text);
      if( i == day)
         attrset( A_NORMAL);
      }
   snprintf( text, sizeof( text), "%s %ld", month_names[month], year);
   mvaddstr( top_margin - 1, (COLS - (int)strlen( text)) / 2, text);
   snprintf( text, sizeof( text), "MJD %ld", mjd);
   mvaddstr( top_margin - 1, COLS - (int)strlen( text), text);
   if( ifile)
      {
      char search[8];

      snprintf( search, sizeof( search), "%4ld %02d", year, month);
      while( fgets( text, sizeof( text), ifile))
         if( !memcmp( text, search, 7))
            {
            const long curr_mjd = mjd0 + atol( text + 8) - 1;
            const int xloc = (curr_mjd + 3) % 7, yloc = (curr_mjd + 3) / 7 - (int)week0;

            move( yval[yloc + 1] + 2, xval[xloc] + 1);
            text[strlen( text) - 1] = '\0';
            addstr( text + 11);
            }
      fclose( ifile);
      }
}

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
   long mjd0 = 40587 + (long)time( NULL) / 86400, mjd = mjd0;
   int quit = 0;

   INTENTIONALLY_UNUSED_PARAMETER( argc);
   INTENTIONALLY_UNUSED_PARAMETER( argv);
   if( argc == 2)
      mjd = atol( argv[1]);
   initscr( );
   noecho();
   keypad(stdscr, TRUE);
   raw();
   while( !quit)
      {
      int c;

      show_calendar( mjd, "../demos/");
      switch( c = getch( ))
         {
         case KEY_UP:
            mjd -= 7;
            break;
         case KEY_DOWN:
            mjd += 7;
            break;
         case KEY_PPAGE:
            mjd -= 28;
            break;
         case KEY_NPAGE:
            mjd += 28;
            break;
         case KEY_LEFT:
            mjd--;
            break;
         case KEY_RIGHT:
            mjd++;
            break;
         case KEY_HOME:
            mjd = mjd0;
            break;
#ifdef KEY_RESIZE
         case KEY_RESIZE:
            resize_term(0, 0);
            break;
# endif
         case 'q': case 27:
            quit = 1;
            break;
         }
      }

   endwin( );
   return( 0);
}
