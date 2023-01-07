/* PDCurses */

#if defined( PDC_FORCE_UTF8) && !defined( PDC_WIDE)
   #define PDC_WIDE
#endif

#include <SDL.h>
#ifdef PDC_WIDE
# include <SDL_ttf.h>
#if SDL_TTF_VERSION_ATLEAST(2,0,18)
/* SDL_ttf 2.0.18 introduced the the functions we use for rendering
   individual characters beyond the BMP. */
#define PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT 1
#endif
#endif

#include <curspriv.h>

#ifdef PDC_WIDE
PDCEX  TTF_Font *pdc_ttffont;
PDCEX  int pdc_font_size;
#define PDC_SDL_RENDER_SOLID 1
#define PDC_SDL_RENDER_SHADED 2
#define PDC_SDL_RENDER_BLENDED 3

PDCEX int pdc_sdl_render_mode;
#endif
PDCEX  SDL_Window *pdc_window;
PDCEX  SDL_Surface *pdc_screen, *pdc_font, *pdc_icon, *pdc_back;
PDCEX  int pdc_sheight, pdc_swidth, pdc_yoffset, pdc_xoffset;

extern SDL_Surface *pdc_tileback;    /* used to regenerate the background
                                        of "transparent" cells */
extern int pdc_fheight, pdc_fwidth;  /* font height and width */
extern int pdc_fthick;               /* thickness for highlights and
                                        rendered ACS glyphs */
extern int pdc_flastc;               /* font palette's last color
                                        (treated as the foreground) */
extern bool pdc_own_window;          /* if pdc_window was not set
                                        before initscr(), PDCurses is
                                        responsible for (owns) it */

PDCEX  void PDC_update_rects(void);
PDCEX  void PDC_retile(void);

extern void PDC_pump_and_peep(void);
extern void PDC_blink_text(void);
