#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* Code to convert SDL fonts for PDCurses into the more generally
useful PSF2 format.  The SDL fonts are Microsoft(R) .bmp files,
with one bit per pixel.  If you load up the bitmap into a viewing
program,  you'll see 256 glyphs arranged in 8 rows of 32 columns.
Thus,  if the glyphs are xpix pixels wide and ypix high,  the .bmp
will be 32*xpix wide and 8*ypix high.

   This program will read in one or more .bmp files,  reverse the
above math to determine glyph sizes from the .bmp size,  and write
out a PSF2-formatted font file.  */

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct psf2_header {
        uint8_t magic[4];
        uint32_t version;
        uint32_t headersize;    /* offset of bitmaps in file */
        uint32_t flags;
        uint32_t length;        /* number of glyphs */
        uint32_t charsize;      /* number of bytes for each character */
        uint32_t height, width; /* max dimensions of glyphs */
        /* charsize = height * ((width + 7) / 8) */
};

#define BIT_SET( buff, n) ((buff[(n) >> 3] << ((n) & 7)) & 0x80)

static void set_up_psf2_header( struct psf2_header *hdr, const int height, const int width)
{
   hdr->magic[0] = PSF2_MAGIC0;
   hdr->magic[1] = PSF2_MAGIC1;
   hdr->magic[2] = PSF2_MAGIC2;
   hdr->magic[3] = PSF2_MAGIC3;
   hdr->version = 0;
   hdr->headersize = sizeof( struct psf2_header);
   hdr->flags = 0;
   hdr->length = 256;
   hdr->charsize = (uint32_t)( height * ((width + 7) / 8));
   hdr->height = (uint32_t)height;
   hdr->width = (uint32_t)width;
}

static int convert_bmp_font_to_psf2( const char *bmp_filename)
{
   FILE *ifile = fopen( bmp_filename, "rb"), *ofile;
   int32_t header_size, width, height;
   size_t glyph_size;
   char buff[62];
   uint8_t *glyphs;
   int i, j;
   struct psf2_header hdr;

   assert( ifile);
   if( fread( buff, sizeof( buff), 1, ifile) != 1)
      assert( 0);
   assert( buff[0] == 'B');
   assert( buff[1] == 'M');
   memcpy( &header_size, buff + 14, sizeof( int32_t));
   memcpy( &width, buff + 18, sizeof( int32_t));
   memcpy( &height, buff + 22, sizeof( int32_t));
   printf( "Width %d  height %d  header len %d\n", (int)width, (int)height, (int)header_size);
   glyph_size = (size_t)( width * height / 8);
   printf( "Image size should be %d\n", 62 + (int)glyph_size);
   glyphs = (uint8_t *)malloc( glyph_size);
   if( fread( glyphs, glyph_size, 1, ifile) != 1)
      assert( 0);
   fclose( ifile);

   strcpy( buff, bmp_filename);
   strcpy( strstr( buff, ".bmp"), ".psf");
   ofile = fopen( buff, "wb");
   assert( ofile);
   width /= 32;      /* cvt from BMP size to size,  in pixels,  of the */
   height /= 8;      /* individual glyphs in the BMP */
   set_up_psf2_header( &hdr, height, width);
   if( fwrite( &hdr, sizeof( hdr), 1, ofile) != 1)
      assert( 0);
   for( i = 0; i < 256; i++)
      for( j = 0; j < height; j++)
         {
         int x, x1 = (i % 32) * width, k;
         int line_no = 8 * height - 1 - j - (i / 32) * height;
         uint8_t *line_ptr = glyphs + line_no * width * 32 / 8;

         assert( line_ptr < glyphs + glyph_size);
         assert( line_ptr >= glyphs);
         for( k = 0; k < (width + 7) / 8; k++)
            {
            uint8_t output_byte = 0;
            int n_bits = width - k * 8;

            if( n_bits > 8)
               n_bits = 8;
            for( x = 0; x < n_bits; x++, x1++)
               if( BIT_SET( line_ptr, x1))
                  output_byte ^= (uint8_t)(0x80 >> x);
            if( fwrite( &output_byte, 1, 1, ofile) != 1)
               assert( 0);
            }
         }
   fclose( ofile);
   return( 0);
}

int main( const int argc, const char **argv)
{
   int i;

   for( i = 1; i < argc; i++)
      convert_bmp_font_to_psf2( argv[i]);
   return( 0);
}
