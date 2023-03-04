/* PDCurses */

#if defined( PDC_FORCE_UTF8) && !defined( PDC_WIDE)
   #define PDC_WIDE
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#if SDL_TTF_VERSION_ATLEAST(2,0,18)
/* SDL_ttf 2.0.18 introduced the the functions we use for rendering
   individual characters beyond the BMP. */
#define PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT 1
#endif

#define PDC_GL_RESIZE_NORMAL 0        /* Actually adds more characters. */
#define PDC_GL_RESIZE_STRETCH 1       /* Stretches the contents to fill the
                                         whole screen. */
#define PDC_GL_RESIZE_SCALE 2         /* Same as above, but preserves aspect
                                         ratio. */
#define PDC_GL_RESIZE_INTEGER 3       /* Resizes in integer steps. */

#define PDC_GL_INTERPOLATE_NEAREST 0  /* Nearest-neighbor interpolation for
                                         text. */
#define PDC_GL_INTERPOLATE_BILINEAR 1 /* Bilinear interpolation for text. */

#include <curspriv.h>

PDCEX  TTF_Font *pdc_ttffont;
PDCEX  int pdc_font_size;
PDCEX  int pdc_resize_mode;        /* PDC_GL_RESIZE_NORMAL by default */
PDCEX  int pdc_interpolation_mode; /* PDC_GL_INTERPOLATE_BILINEAR by default */
PDCEX  SDL_Window *pdc_window;
PDCEX  SDL_Surface *pdc_icon;
PDCEX  int pdc_sheight, pdc_swidth;

extern Uint32 *pdc_glyph_cache[4];
extern int pdc_glyph_cache_size[4];
extern int pdc_glyph_row_capacity, pdc_glyph_col_capacity;
extern int pdc_glyph_cache_w, pdc_glyph_cache_h;
extern int* pdc_glyph_start_col;

extern unsigned pdc_color_buffer, pdc_glyph_buffer;
extern unsigned pdc_background_shader_program, pdc_foreground_shader_program;
extern unsigned pdc_font_texture, pdc_render_target_texture;
extern unsigned pdc_tex_fbo;

extern int pdc_fheight, pdc_fwidth;  /* font height and width */
extern int pdc_fthick;               /* thickness for highlights and
                                        rendered ACS glyphs */
extern void PDC_pump_and_peep(void);
extern void PDC_blink_text(void);
extern SDL_Rect PDC_get_viewport(void);
