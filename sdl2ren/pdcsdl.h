/* PDCurses */

#if !defined( PDC_WIDE)
#define PDC_WIDE
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#if SDL_TTF_VERSION_ATLEAST(2,0,18)
/* SDL_ttf 2.0.18 introduced the the functions we use for rendering
   individual characters beyond the BMP. */
#define PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT 1
#endif

#include <curspriv.h>

PDCEX  TTF_Font *pdc_ttffont;
PDCEX  int pdc_font_size;
#define PDC_SDL_RENDER_SOLID 1
// SHADED is not supported because it doesn't really fit with any glyph caching
// schemes due to how it deals with color.
#define PDC_SDL_RENDER_BLENDED 3

PDCEX int pdc_sdl_render_mode;
PDCEX int pdc_sdl_scaling;
PDCEX  SDL_Window *pdc_window;
PDCEX  SDL_Renderer* pdc_renderer;
PDCEX  SDL_Surface *pdc_icon;
PDCEX  SDL_Texture* pdc_render_target;
PDCEX  SDL_Texture** pdc_glyph_cache[4];
PDCEX  int pdc_glyph_cache_size[4];
PDCEX  int pdc_sheight, pdc_swidth, pdc_yoffset, pdc_xoffset;

extern int pdc_fheight, pdc_fwidth;  /* font height and width */
extern int pdc_fthick;               /* thickness for highlights and
                                        rendered ACS glyphs */
extern bool pdc_own_window;          /* if pdc_window was not set
                                        before initscr(), PDCurses is
                                        responsible for (owns) it */
extern void PDC_pump_and_peep(void);
extern void PDC_blink_text(void);
