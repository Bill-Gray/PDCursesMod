#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "pdcfb.h"
#include "psf.c"
#ifdef PDC_WIDE
   #include "lat2_vga.h"
#else
   #include "../dosvga/font.h"
#endif

static struct termios orig_term;

#include <assert.h>
#include "curspriv.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifdef USING_COMBINING_CHARACTER_SCHEME
int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);
#endif

int PDC_rows = -1, PDC_cols = -1;
bool PDC_resize_occurred = FALSE;
const int STDIN = 0;
chtype PDC_capabilities = 0;
static mmask_t _stored_trap_mbe;

/* COLOR_PAIR to attribute encoding table. */

void PDC_reset_prog_mode( void)
{
    struct termios term;

    tcgetattr( STDIN, &orig_term);
    memcpy( &term, &orig_term, sizeof( term));
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN, TCSANOW, &term);
    PDC_puts_to_stdout( "\033[?1006h");    /* Set SGR mouse tracking,  if available */
#ifdef HOW_DO_WE_PRESERVE_THE_SCREEN
    if( !SP->_preserve)
       PDC_puts_to_stdout( "\033[?47h");      /* Save screen */
#endif
    SP->_trap_mbe = _stored_trap_mbe;
    PDC_mouse_set( );          /* clear any mouse event captures */
    PDC_resize_occurred = FALSE;
}

void PDC_reset_shell_mode( void)
{
}

int PDC_resize_screen(int nlines, int ncols)
{
#ifdef NO_RESIZING_YET
   if( PDC_rows == -1)     /* initscr( ) hasn't been called;  we're just */
      {                    /* setting desired size at startup */
      initial_PDC_rows = nlines;
      initial_PDC_cols = ncols;
      }
   else if( nlines > 1 && ncols > 1)
      {
      char tbuff[50];

      PDC_rows = nlines;
      PDC_cols = ncols;
      }
#else
    INTENTIONALLY_UNUSED_PARAMETER( nlines);
    INTENTIONALLY_UNUSED_PARAMETER( ncols);
#endif
   return( 0);
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

bool PDC_has_rgb_color = TRUE;
struct fb_fix_screeninfo PDC_finfo;
struct fb_var_screeninfo PDC_vinfo;
uint8_t *PDC_framebuf;
static int _framebuffer_fd;
struct font_info PDC_font_info;
static uint8_t *_loaded_font_bytes;

void PDC_scr_close( void)
{
   _stored_trap_mbe = SP->_trap_mbe;
   SP->_trap_mbe = 0;
   PDC_mouse_set( );          /* clear any mouse event captures */
   tcsetattr( STDIN, TCSANOW, &orig_term);
   PDC_doupdate( );
   PDC_puts_to_stdout( NULL);      /* free internal cache */
   munmap( PDC_framebuf, PDC_finfo.smem_len);
   close( _framebuffer_fd);
   if( _loaded_font_bytes)
   {
      free( _loaded_font_bytes);
      _loaded_font_bytes = NULL;
   }
   if( PDC_font_info.unicode_info)
      free( PDC_font_info.unicode_info);
   return;
}

void PDC_scr_free( void)
{
    PDC_free_palette( );
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters( 0, NULL);
#endif
}

static void sigintHandler( int sig)
{
    INTENTIONALLY_UNUSED_PARAMETER( sig);
    if( !SP->raw_inp)
    {
        PDC_scr_close( );
        PDC_scr_free( );
        exit( 0);
    }
}

#define MAX_LINES 1000
#define MAX_COLUMNS 1000

int PDC_scr_open(void)
{
    struct sigaction sa;
    int error;
    const char *font_filename = getenv( "PDC_FONT");

    PDC_LOG(("PDC_scr_open called\n"));
    _framebuffer_fd = open( "/dev/fb0", O_RDWR);
    if( _framebuffer_fd < 0)
        return( -1);

    error = ioctl( _framebuffer_fd, FBIOGET_VSCREENINFO, &PDC_vinfo);
    if( error)
        return( -2);
   /*  Get fixed screen information */
    error = ioctl( _framebuffer_fd, FBIOGET_FSCREENINFO, &PDC_finfo);
    if( error)
        return( -3);

    PDC_framebuf = mmap(NULL, PDC_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED,
                  _framebuffer_fd, 0);
    if( PDC_framebuf == MAP_FAILED)
        return( -4);
    PDC_has_rgb_color = (PDC_vinfo.bits_per_pixel > 8);
    if( PDC_has_rgb_color)
       COLORS = 256 + (256 * 256 * 256);
    else
       COLORS = 256;
    assert( SP);
    if (!SP || PDC_init_palette( ))
        return ERR;

    load_psf_or_vgafont( &PDC_font_info, font_bytes, sizeof( font_bytes));
    if( font_filename)
    {
        FILE *font_fp = fopen( font_filename, "rb");

        if( font_fp)
        {
            uint8_t *buff;
            long n_bytes;

            fseek( font_fp, 0L, SEEK_END);
            n_bytes = ftell( font_fp);
            fseek( font_fp, 0L, SEEK_SET);
            buff = (uint8_t *)malloc( n_bytes + 1);
            buff[n_bytes] = '\0';
            if( fread( buff, 1, n_bytes, font_fp) != (size_t)n_bytes)
                return( -5);
            fclose( font_fp);
            if( !load_psf_or_vgafont( &PDC_font_info, buff, n_bytes))
            {
                uint32_t glyph_buff_size = PDC_font_info.n_glyphs * PDC_font_info.charsize;

                _loaded_font_bytes = (uint8_t *)malloc( glyph_buff_size);

                memcpy( _loaded_font_bytes, PDC_font_info.glyphs, glyph_buff_size);
                PDC_font_info.glyphs = _loaded_font_bytes;
            }
            free( buff);
        }
#ifdef DEBUG
        else
        {
            fprintf( stderr, "Couldn't open %s\n", font_filename);
            exit( -1);
        }
#endif
    }

    setbuf( stdin, NULL);
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf( stderr, "Sigaction (INT) failed\n");
        return( -1);
    }
    PDC_cols = PDC_vinfo.xres / PDC_font_info.width;
    PDC_rows = PDC_vinfo.yres / PDC_font_info.height;
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->visibility = 0;                /* no cursor,  by default */
    SP->curscol = SP->cursrow = 0;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->orig_attr = TRUE;
    SP->orig_fore = SP->orig_back = -1;
    SP->termattrs = A_UNDERLINE | A_DIM | A_STANDOUT | A_STRIKEOUT | A_BLINK;

    SP->lines = PDC_get_rows();
    SP->cols = PDC_get_columns();

    if (SP->lines < 2 || SP->lines > MAX_LINES
       || SP->cols < 2 || SP->cols > MAX_COLUMNS)
    {
        fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }

    SP->_preserve = (getenv("PDC_PRESERVE_SCREEN") != NULL);
    PDC_reset_prog_mode();
    PDC_LOG(("PDC_scr_open exit\n"));
    return( 0);
}

int PDC_set_function_key( const unsigned function, const int new_key)
{
   INTENTIONALLY_UNUSED_PARAMETER( function);
   INTENTIONALLY_UNUSED_PARAMETER( new_key);
   return( 0);
}

void PDC_set_resize_limits( const int new_min_lines,
                            const int new_max_lines,
                            const int new_min_cols,
                            const int new_max_cols)
{
   INTENTIONALLY_UNUSED_PARAMETER( new_min_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_min_cols);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_cols);
   return;
}


bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content( int color, int *red, int *green, int *blue)
{
    const PACKED_RGB col = PDC_get_palette_entry( color);

    *red = DIVROUND( Get_RValue(col) * 1000, 255);
    *green = DIVROUND( Get_GValue(col) * 1000, 255);
    *blue = DIVROUND( Get_BValue(col) * 1000, 255);

    return OK;
}

int PDC_init_color( int color, int red, int green, int blue)
{
    const PACKED_RGB new_rgb = PACK_RGB(DIVROUND(red * 255, 1000),
                                 DIVROUND(green * 255, 1000),
                                 DIVROUND(blue * 255, 1000));

    if( !PDC_set_palette_entry( color, new_rgb))
        curscr->_clear = TRUE;
    return OK;
}
