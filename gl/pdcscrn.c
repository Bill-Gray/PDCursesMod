/* PDCurses */

#include "pdcsdl.h"

#include <stdlib.h>
#include <limits.h>
#include "../common/iconbmp.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifndef PDC_FONT_PATH
# ifdef _WIN32
#  define PDC_FONT_PATH "C:/Windows/Fonts/consola.ttf"
# elif defined(__APPLE__)
#  define PDC_FONT_PATH "/System/Library/Fonts/Menlo.ttc"
# else
#  define PDC_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#  define PDC_FONT_PATH2 "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf"
# endif
#endif
TTF_Font *pdc_ttffont = NULL;
int pdc_font_size =
#ifdef _WIN32
 16;
#else
 17;
#endif
int pdc_sdl_render_mode = PDC_SDL_RENDER_BLENDED;
SDL_Texture **pdc_glyph_cache[4] = {NULL, NULL, NULL, NULL};
int pdc_glyph_cache_size[4] = {0, 0, 0, 0};

int pdc_sdl_scaling = 1;

SDL_Window *pdc_window = NULL;
SDL_Renderer *pdc_renderer = NULL;
SDL_Surface *pdc_icon = NULL;
SDL_Texture *pdc_render_target = NULL;
int pdc_sheight = 0, pdc_swidth = 0, pdc_yoffset = 0, pdc_xoffset = 0;

int pdc_fheight, pdc_fwidth, pdc_fthick;
bool pdc_own_window;
bool pdc_own_renderer;

static void _clean(void)
{
    if (pdc_ttffont)
    {
        TTF_CloseFont(pdc_ttffont);
        TTF_Quit();
        pdc_ttffont = NULL;
    }
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < pdc_glyph_cache_size[i]; ++j)
        {
            if(pdc_glyph_cache[i][j])
                SDL_DestroyTexture(pdc_glyph_cache[i][j]);
        }
        if(pdc_glyph_cache[i])
            free(pdc_glyph_cache[i]);
        pdc_glyph_cache[i] = NULL;
        pdc_glyph_cache_size[i] = 0;
    }

    if( pdc_icon)
        SDL_FreeSurface(pdc_icon);

    pdc_icon = NULL;

    if(pdc_render_target)
    {
        SDL_DestroyTexture(pdc_render_target);
        pdc_render_target = NULL;
    }

    if(pdc_own_renderer && pdc_renderer)
    {
        SDL_DestroyRenderer(pdc_renderer);
        pdc_renderer = NULL;
    }

    if( pdc_own_window && pdc_window)
    {
        SDL_DestroyWindow(pdc_window);
        pdc_window = NULL;
    }

    SDL_Quit();
    pdc_sheight = pdc_swidth = 0;
}

void PDC_scr_close(void)
{
    PDC_LOG(("PDC_scr_close() - called\n"));
}

void PDC_scr_free(void)
{
    PDC_free_palette( );
    _clean( );
}


/* find the display where the mouse pointer is */

int _get_displaynum(void)
{
    SDL_Rect size;
    int i, xpos, ypos, displays;

    displays = SDL_GetNumVideoDisplays();

    if (displays > 1)
    {
        SDL_GetGlobalMouseState(&xpos, &ypos);

        for (i = 0; i < displays; i++)
        {
            SDL_GetDisplayBounds(i, &size);
            if (size.x <= xpos && xpos < size.x + size.w &&
                size.y <= ypos && ypos < size.y + size.h)
                return i;
        }
    }

    return 0;
}

/* open the physical screen -- miscellaneous initialization */

int PDC_scr_open(void)
{
    SDL_Event event;
    int displaynum = 0;
    const char *ptsz, *fname;

    PDC_LOG(("PDC_scr_open() - called\n"));

    pdc_own_window = !pdc_window;
    pdc_own_renderer = !pdc_renderer;

    if (pdc_own_window)
    {
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) < 0)
        {
            fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());
            return ERR;
        }

        atexit(_clean);

        displaynum = _get_displaynum();
    }

    if (TTF_Init() == -1)
    {
        fprintf(stderr, "Could not start SDL_TTF: %s\n", SDL_GetError());
        return ERR;
    }

    ptsz = getenv("PDC_FONT_SIZE");
    if (ptsz != NULL)
        pdc_font_size = atoi(ptsz);
    if (pdc_font_size <= 0)
        pdc_font_size = 18;

    fname = getenv("PDC_FONT");
    pdc_ttffont = TTF_OpenFont(fname ? fname : PDC_FONT_PATH,
                               pdc_font_size);
# ifdef PDC_FONT_PATH2
    if (!pdc_ttffont && !fname)
    {
        pdc_ttffont = TTF_OpenFont(PDC_FONT_PATH2,
                                   pdc_font_size);
    }
# endif

    if (!pdc_ttffont)
    {
        if (fname)
            fprintf(stderr, "Could not load specified font: %s\n",fname);
        else
        {
# ifdef PDC_FONT_PATH2
            fprintf(stderr, "Could not load default font: %s or %s\n",
                    PDC_FONT_PATH, PDC_FONT_PATH2);
# else
            fprintf(stderr, "Could not load default font: %s\n",
                    PDC_FONT_PATH);
# endif

        }
        return ERR;
    }

    TTF_SetFontKerning(pdc_ttffont, 0);
    TTF_SetFontHinting(pdc_ttffont, TTF_HINTING_MONO);

    SP->mono = FALSE;

    if (!SP->mono && !pdc_own_window)
    {
        SP->orig_attr = TRUE;
        SP->orig_fore = COLOR_WHITE;
        SP->orig_back = -1;
    }
    else
        SP->orig_attr = FALSE;

    TTF_SizeText(pdc_ttffont, "W", &pdc_fwidth, &pdc_fheight);
    pdc_fthick = pdc_font_size / 20 + 1;

    if (pdc_own_window && !pdc_icon)
    {
        const char *iname = getenv("PDC_ICON");
        pdc_icon = SDL_LoadBMP(iname ? iname : "pdcicon.bmp");

        if (!pdc_icon)
            pdc_icon = SDL_LoadBMP_RW(SDL_RWFromMem(iconbmp,
                                                    sizeof(iconbmp)), 0);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    if (pdc_own_window)
    {
        if( !pdc_sheight)
        {
            const char *env = getenv("PDC_LINES");
            pdc_sheight = (env ? atoi(env) : 25);
        }
        pdc_sheight *= pdc_fheight;

        if( !pdc_swidth)
        {
            const char *env = getenv("PDC_COLS");
            pdc_swidth = (env ? atoi(env) : 80);
        }
        pdc_swidth *= pdc_fwidth;

        pdc_window = SDL_CreateWindow("PDCurses",
            SDL_WINDOWPOS_CENTERED_DISPLAY(displaynum),
            SDL_WINDOWPOS_CENTERED_DISPLAY(displaynum),
            pdc_swidth * pdc_sdl_scaling, pdc_sheight * pdc_sdl_scaling,
            SDL_WINDOW_RESIZABLE);

        if (pdc_window == NULL)
        {
            fprintf(stderr, "Could not open SDL window: %s\n", SDL_GetError());
            return ERR;
        }

        SDL_SetWindowIcon(pdc_window, pdc_icon);
    }

    if (pdc_own_renderer)
    {
        pdc_renderer = SDL_CreateRenderer(pdc_window, -1, SDL_RENDERER_ACCELERATED);

        if (pdc_renderer == NULL)
        {
            fprintf(stderr, "Could not open SDL renderer: %s\n", SDL_GetError());
            return ERR;
        }
    }

    int h, w;
    SDL_GetWindowSize(pdc_window, &w, &h);
    w /= pdc_sdl_scaling;
    h /= pdc_sdl_scaling;
    pdc_render_target = SDL_CreateTexture(pdc_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetRenderTarget(pdc_renderer, pdc_render_target);

    /* Events must be pumped before calling SDL_GetWindowSurface, or
       initial modifiers (e.g. numlock) will be ignored and out-of-sync. */

    SDL_PumpEvents();

    /* Wait until window is exposed before getting surface */

    while (SDL_PollEvent(&event))
        if (SDL_WINDOWEVENT == event.type &&
            SDL_WINDOWEVENT_EXPOSED == event.window.event)
            break;

    if (!pdc_sheight)
        pdc_sheight = h - pdc_yoffset;

    if (!pdc_swidth)
        pdc_swidth = w - pdc_xoffset;

    SDL_StartTextInput();

    PDC_mouse_set();

    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->audible = FALSE;

    SP->termattrs = A_COLOR | A_UNDERLINE | A_LEFT | A_RIGHT | A_REVERSE
                            | A_OVERLINE | A_STRIKEOUT;
#ifdef PDC_WIDE
    SP->termattrs |= A_ITALIC;
#endif

    PDC_reset_prog_mode();

    return OK;
}

/* the core of resize_term() */

int PDC_resize_screen(int nlines, int ncols)
{
    if( !stdscr)     /* Specifying the  initial screen size */
    {                /* before calling initscr().           */
        pdc_sheight = nlines;
        pdc_swidth = ncols;
        return OK;
    }

    if (!pdc_own_window)
        return ERR;

    if (nlines && ncols)
    {
#if SDL_VERSION_ATLEAST(2, 0, 5)
        SDL_Rect max;
        int top, left, bottom, right;

        SDL_GetDisplayUsableBounds(0, &max);
        SDL_GetWindowBordersSize(pdc_window, &top, &left, &bottom, &right);
        max.h -= top + bottom;
        max.w -= left + right;

        while (nlines * pdc_fheight > max.h)
            nlines--;
        while (ncols * pdc_fwidth > max.w)
            ncols--;
#endif
        pdc_sheight = nlines * pdc_fheight;
        pdc_swidth = ncols * pdc_fwidth;

        SDL_SetWindowSize(pdc_window, pdc_swidth, pdc_sheight);
    }

    return OK;
}

void PDC_reset_prog_mode(void)
{
    PDC_LOG(("PDC_reset_prog_mode() - called.\n"));

    PDC_flushinp();
}

void PDC_reset_shell_mode(void)
{
    PDC_LOG(("PDC_reset_shell_mode() - called.\n"));

    PDC_flushinp();
}

void PDC_restore_screen_mode(int i)
{
   INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
   INTENTIONALLY_UNUSED_PARAMETER( i);
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

/* Does nothing in the SDL flavors of PDCurses.  That may change,  eventually,
allowing one to limit the range of user-resizable windows.  See X11 or Win32a
versions of this function for details. */

void PDC_set_resize_limits( const int new_min_lines, const int new_max_lines,
                  const int new_min_cols, const int new_max_cols)
{
   INTENTIONALLY_UNUSED_PARAMETER( new_min_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_min_cols);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_cols);
}
