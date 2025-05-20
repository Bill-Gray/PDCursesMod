/*  This code reads in an ASCII art cursor (see below) and converts it
into bitmap form.  The cursor is provided as comments in this file (the
program reads its own source file) starting with a comment line with
'Cursor(n)', size,  and center point.

   Each pixel is either empty (transparent),  or sets a black pixel,
or sets a white pixel.  The ASCII art is really rendered as two bitmaps;
one says "set this pixel",  the second says if it should be black or
white.  See the PDC_update_mouse_cursor( ) function in 'pdcdisp.c' for
how this actually gets used.

   At present,  we're just using the first cursor.  But we may someday
want mouse cursors that are carets,  crosses,  cute animals,  etc.

   Compile with cc -Wall -Wextra -pedantic -o cursor cursor.c

   First (and only currently used) cursor is 19 pixels high,  center
is at x=1, y=1. '..' = display in black,  'xx' = display in white,
'  ' = transparent. */

/* Cursor1 19 1 1
....
......
..xx..
..xx..
..xxxx..
..xxxxxx..
..xxxxxx..
..xxxxxxxx..
..xxxxxxxxxx..
..xxxxxxxxxx..
..xxxxxxxxxxxx..
..xxxxxxxxxxxxxx..
..xxxxxxxx..xxxx..
..xxxx..xxxx..xxxx..
..xx....xxxx........
....    ..xxxx..
          ..xxxx..
          ..xxxx..
            ....
*/

/* Aforementioned 'cross' cursor.  Also 19 lines,  but center is
at x=9, y=9.        */

/* Cursor2 19 9 9
                  ..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
  ..............      ..............
..xxxxxxxxxxxxxx      xxxxxxxxxxxxxx..
  ..............      ..............
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                ..xx..
                  ..
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( "cursor.c", "rb");
   char buff[80];
   int found_it = 0, size, xhot, yhot, width_in_bytes;
   unsigned char *map1, *map2;
   const char searched_cursor = (argc == 2 ? argv[1][0] : '1');

   assert( ifile);
   while( !found_it && fgets( buff, sizeof( buff), ifile))
      found_it = !memcmp( buff, "/* Cursor", 9) && buff[9] == searched_cursor;
   assert( found_it);
   assert( 3 == sscanf( buff + 11, "%d %d %d", &size, &xhot, &yhot));
   width_in_bytes = (size + 7) / 8;
   printf( "const unsigned char cursor_data[%d] = { %d, %d, %d",
                   size * width_in_bytes * 2 + 3, size, xhot, yhot);
   map1 = (unsigned char *)calloc( 2 * width_in_bytes, size);
   map2 = map1 + width_in_bytes * size;
   for( int i = 0; i < size && fgets( buff, sizeof( buff), ifile) && buff[1] != '/'; i++)
      for( int j = 0; buff[j + j] >= ' '; j++)
         {
         if( buff[j + j] == '.' || buff[j + j] == 'x')
            map1[i * width_in_bytes + j / 8] |= 1 << (j & 7);
         if( buff[j + j] == 'x')
            map2[i * width_in_bytes + j / 8] |= 1 << (j & 7);
         }
   for( int i = 0; i < size * 2 * width_in_bytes; i++)
      {
      if( i % 9 == 0)
         printf( ",\n      ");
      else
         printf( ", ");
      printf( " 0x%02x", map1[i]);
      }
   printf( "};\n");
   fclose( ifile);
   free( map1);
   return( 0);
}
