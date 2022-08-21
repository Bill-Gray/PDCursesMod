#ifdef PDC_WIDE
   #if !defined( UNICODE)
      # define UNICODE
   #endif
   #if !defined( _UNICODE)
      # define _UNICODE
   #endif
#endif

void PDC_puts_to_stdout( const char *buff);        /* pdcdisp.c */

struct video_info
{
   void *framebuf;
   unsigned xres, yres, bits_per_pixel;
   unsigned line_length;
   unsigned smem_len;
};
