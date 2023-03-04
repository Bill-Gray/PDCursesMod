/* PDCurses */

#if defined( PDC_FORCE_UTF8) && !defined( PDC_WIDE)
   #define PDC_WIDE
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <glad/gl.h>
#if SDL_TTF_VERSION_ATLEAST(2,0,18)
/* SDL_ttf 2.0.18 introduced the the functions we use for rendering
   individual characters beyond the BMP. */
#define PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT 1
#endif

#include <curspriv.h>

PDCEX  TTF_Font *pdc_ttffont;
PDCEX  int pdc_font_size;
extern Uint32 *pdc_glyph_cache[4];
extern int pdc_glyph_cache_size[4];
extern int pdc_glyph_row_capacity, pdc_glyph_col_capacity;
extern int pdc_glyph_cache_w, pdc_glyph_cache_h;
extern int* pdc_glyph_start_col;

extern GLuint pdc_vbo;
extern GLuint pdc_background_shader_program, pdc_foreground_shader_program;
extern GLuint pdc_font_texture;
extern GLuint pdc_tex_fbo;

PDCEX  SDL_Window *pdc_window;
PDCEX  SDL_Surface *pdc_icon;
PDCEX  int pdc_sheight, pdc_swidth;

extern int pdc_fheight, pdc_fwidth;  /* font height and width */
extern int pdc_fthick;               /* thickness for highlights and
                                        rendered ACS glyphs */
extern void PDC_pump_and_peep(void);
extern void PDC_blink_text(void);
