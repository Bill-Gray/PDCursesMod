#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "psf.h"

/* Code for the PSF font format,  both psf1 and psf2,  as
described at

https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html

   and for the (much simpler) 'vgafont' font format;  see _load_vgafont()
below. The PSF fonts can contain Unicode information,  a table basically
saying "Unicode point x corresponds to glyph y".  This code reads that
information (if it's provided) and sorts it by Unicode point,  so that
when the glyph for a given Unicode point is desired,  we can
binary-search for it. */


#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

struct psf1_header {
        unsigned char magic[2];     /* Magic number */
        unsigned char mode;         /* PSF font mode */
        unsigned char charsize;     /* Character size */
};

static int _compare_unicode_info( const void *a, const void *b)
{
   const int32_t diff = *(int32_t *)a - *(int32_t *)b;

   return( (int)diff);
}

static int _load_psf1( struct font_info *f, const uint8_t *buff, const long filelen)
{
   struct psf1_header hdr;
   int n_references_found = 0;

   if( buff[0] != PSF1_MAGIC0 && buff[1] != PSF1_MAGIC1)
      return( -1);
   memcpy( &hdr, buff, sizeof( hdr));
   f->font_type = 1;
   f->n_glyphs = ((hdr.mode & PSF1_MODE512) ? 512 : 256);
   f->headersize = 4;
   f->charsize = f->height = hdr.charsize;
   f->width = 8;
   f->glyphs = buff + f->headersize;
   if( hdr.mode & PSF1_MODEHASTAB)
      {
      size_t i = (size_t) 4 + f->n_glyphs * hdr.charsize;
      unsigned glyph_num = 0;
      unsigned max_info_size = (unsigned)( (filelen - i) / 2 - f->n_glyphs);
      uint32_t *tptr = (uint32_t *)malloc( max_info_size * 2 * sizeof( uint32_t));

      f->unicode_info = tptr;
      for( ; i < (size_t)filelen; i += 2)
         {
         const unsigned ival = buff[i] | ((unsigned)buff[i + 1] << 8);

         if( ival == PSF1_SEPARATOR)
            glyph_num++;
         else if( ival != PSF1_STARTSEQ)
            {
            assert( n_references_found < (int)max_info_size);
            *tptr++ = (uint32_t)ival;
            *tptr++ = glyph_num;
            n_references_found++;
            }
         }
      qsort( f->unicode_info, n_references_found, 2 * sizeof( uint32_t),
                              _compare_unicode_info);
      }
   else
      f->unicode_info = NULL;
   f->unicode_info_size = n_references_found;
   return( 0);
}

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

/* 'Right half of fullwidth characters' are encoded as the 'usual' Unicode
point for that character,  with 0x200000 added.  Therefore,  we're
potentially encoding/decoding values as high as 0x30ffff;  in such a
case,  the starting UTF-8 byte can be as high as 0xFC (instead of the usual
limit of 0xF4).  */

#define IS_UTF8_STARTING_BYTE( c) (!((c) & 0x80) || ((c) > 0xc1 && (c) <= 0xfc))

static uint32_t *_decipher_psf2_unicode_table( const uint8_t *buff, const size_t info_len,
                     uint32_t *unicode_info_size)
{
   unsigned glyph_num = 0;
   size_t i, n1 = 0;
   uint32_t n_references_found = 0;
   uint32_t *unicode_info, *tptr;

   for( i = 0; i < info_len; i++)
      if( IS_UTF8_STARTING_BYTE( buff[i]))
         n1++;
   unicode_info = tptr = (uint32_t *)malloc( n1 * 2 * sizeof( uint32_t));
   for( i = 0; i < info_len; i++)
      if( buff[i] == PSF2_SEPARATOR)
         glyph_num++;
      else if( buff[i] != PSF2_STARTSEQ)
         {
         unsigned cval;    /* decipher UTF8 value */

         if( !(buff[i] & 0x80))           /* plain ASCII */
            cval = (unsigned)buff[i];
         else if( (buff[i] & 0xe0) == 0xc0) /* two-byte UTF8 : code */
            {                               /* points U+80 to U+7FF */
            cval = ((buff[i] & 0x1f) << 6) | (buff[i + 1] & 0x3f);
            i++;
            }
         else
            {           /* three-byte UTF: U+800 to U+FFFF */
            cval = ((buff[i] & 0x0f) << 12) | ((buff[i + 1] & 0x3f) << 6) | (buff[i + 2] & 0x3f);
            if( (buff[i] & 0xf0) == 0xf0)     /* Four-byte UTF:  */
               {                              /* U+10000 and beyond (SMP) */
               cval = (cval << 6) | (buff[i + 3] & 0x3f);
               i++;
               }
            i += 2;
            }
         *tptr++ = cval;
         *tptr++ = glyph_num;
         n_references_found++;
         }
   qsort( unicode_info, n_references_found, 2 * sizeof( uint32_t),
                           _compare_unicode_info);
   *unicode_info_size = n_references_found;
   return( unicode_info);
}


static int _load_psf2( struct font_info *f, const uint8_t *buff, const long filelen)
{
   struct psf2_header hdr;

   if( buff[0] != PSF2_MAGIC0 || buff[1] != PSF2_MAGIC1
            || buff[2] != PSF2_MAGIC2 || buff[3] != PSF2_MAGIC3)
      return( -1);
   memcpy( &hdr, buff, sizeof( hdr));
   f->font_type = 2;
   f->n_glyphs = hdr.length;
   f->headersize = hdr.headersize;
   f->charsize = hdr.charsize;
   f->height = hdr.height;
   f->width = hdr.width;
   f->glyphs = buff + f->headersize;
   if( hdr.flags & PSF2_HAS_UNICODE_TABLE)
      {
      uint32_t start_of_unicode_data = hdr.headersize + hdr.length * hdr.charsize;

      f->unicode_info = _decipher_psf2_unicode_table( buff + start_of_unicode_data,
               (size_t)( filelen - start_of_unicode_data), &f->unicode_info_size);
      }
   else
      {
      f->unicode_info = NULL;
      f->unicode_info_size = 0;
      }
   return( 0);
}

/* The 'vgafont' format is about as simple as you can get and
still call it a font format.  There are 256 glyphs,  each
(filesize/256) pixels high by eight pixels wide.  Each byte
contains exactly one line of eight pixels.  There are no magic
numbers,  so I have adopted the expedient of saying that if
the buffer is a multiple of 256 bytes and 6 <= font height <= 20,
assume it's a vgafont.   */

static int _load_vgafont( struct font_info *f, const uint8_t *buff, const long filelen)
{
   if( filelen % 256 || filelen < 6 * 256 || filelen > 20 * 256)
      return( -1);
   else
      {
      f->unicode_info = NULL;
      f->unicode_info_size = 0;
      f->font_type = 0;
      f->n_glyphs = 256;
      f->headersize = 0;
      f->charsize = f->height = (uint32_t)( filelen / 256);
      f->width = 8;
      f->glyphs = buff;
      return( 0);
      }
}

int load_psf_or_vgafont( struct font_info *f, const uint8_t *buff, const long filelen)
{
   if( _load_psf1( f, buff, filelen) && _load_psf2( f, buff, filelen)
                     && _load_vgafont( f, buff, filelen))
      return( -1);
   else
      return( 0);
}

int find_psf_or_vgafont_glyph( struct font_info *f, const uint32_t unicode_point)
{
   int rval = -1;

   if( f->unicode_info)
      {
      const uint32_t *tptr = (const uint32_t *)bsearch( &unicode_point, f->unicode_info,
                  f->unicode_info_size, 2 * sizeof( uint32_t), _compare_unicode_info);

      if( tptr)
         rval = (int)( tptr[1]);
      }
   else if( unicode_point < f->n_glyphs)
      rval = (int)unicode_point;
   return( rval);
}
