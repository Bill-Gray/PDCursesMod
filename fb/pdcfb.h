struct video_info
{
   void *framebuf;
   unsigned xres, yres, bits_per_pixel;
   unsigned line_length;
   unsigned smem_len;
};

#ifdef HAVE_MOUSE
extern int PDC_mouse_x, PDC_mouse_y;
#endif
