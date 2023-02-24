/* PDCurses */

#if !defined( PDC_WIDE)
#define PDC_WIDE
#endif

#include <SDL.h>
#include <SDL_ttf.h>

#include <curspriv.h>

PDCEX  TTF_Font *pdc_ttffont;
PDCEX  int pdc_font_size;
#define PDC_SDL_RENDER_SOLID 1
// SHADED is not supported because it doesn't really fit with any glyph caching
// schemes due to how it deals with color.
#define PDC_SDL_RENDER_BLENDED 3

PDCEX int pdc_sdl_render_mode;
PDCEX  SDL_Window *pdc_window;
PDCEX  SDL_Surface *pdc_screen, *pdc_icon;
PDCEX  SDL_Surface** pdc_glyph_cache[4];
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
