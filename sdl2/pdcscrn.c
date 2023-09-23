/* PDCurses */

#include "pdcsdl.h"

#include <stdlib.h>
#include <limits.h>
#ifndef PDC_WIDE
# include "../common/font437.h"
#endif
#include "../common/iconbmp.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifdef PDC_WIDE
# ifndef PDC_FONT_PATH
#  ifdef _WIN32
#   define PDC_FONT_PATH "C:/Windows/Fonts/consola.ttf"
#  elif defined(__APPLE__)
#   define PDC_FONT_PATH "/System/Library/Fonts/Menlo.ttc"
#  else
#   define PDC_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#   define PDC_FONT_PATH2 "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf"
#  endif
# endif
TTF_Font *pdc_ttffont = NULL;
int pdc_font_size =
# ifdef _WIN32
 16;
# else
 17;
# endif
int pdc_sdl_render_mode = PDC_SDL_RENDER_BLENDED;
#endif

SDL_Window *pdc_window = NULL;
SDL_Surface *pdc_screen = NULL, *pdc_font = NULL, *pdc_icon = NULL,
            *pdc_back = NULL, *pdc_tileback = NULL;
int pdc_sheight = 0, pdc_swidth = 0, pdc_yoffset = 0, pdc_xoffset = 0;

int pdc_fheight, pdc_fwidth, pdc_fthick, pdc_flastc;
bool pdc_own_window;

#ifndef PDC_WIDE

/* Load a bitmap from bmp_rw and also determine its palette size.
   ->format->palette->ncolors may be greater than the actual size.
   To get the real palette size, some custom bitmap parsing is needed. */

static SDL_Surface *_load_bmp_and_palette_size(SDL_RWops *bmp_rw, int *palette_size)
{
    SDL_Surface *bmp = NULL;
    Sint64 start_offset;

    *palette_size = -1;
    if (!bmp_rw)
        return( NULL);

    start_offset = SDL_RWtell(bmp_rw);

    /* Non-monochromatic bitmaps only */
    if( start_offset >= 0 && (bmp = SDL_LoadBMP_RW(bmp_rw, 0)) != NULL
                            && bmp->format->palette)
    {
        *palette_size = bmp->format->palette->ncolors;

        /* Offsets from https://en.wikipedia.org/wiki/BMP_file_format#DIB_header_(bitmap_information_header) */
        if (SDL_RWseek(bmp_rw, start_offset + 14, RW_SEEK_SET) >= 0)
        {
            const Uint32 header_size = SDL_ReadLE32(bmp_rw);

            /* If the info header is at least 40 bytes in size, palette size
               information is available. */
            if (header_size >= 40
                    && SDL_RWseek(bmp_rw, start_offset + 46, RW_SEEK_SET) >= 0)
            {
                Uint32 num_colors = SDL_ReadLE32(bmp_rw);

                if (num_colors > 0 && num_colors <= (Uint32)INT_MAX &&
                    (int)num_colors <= bmp->format->palette->ncolors)
                         *palette_size = (int)num_colors;
                else
                {
                    /* Fall back to calculating num_colors from bits_per_pixel. */

                    if (SDL_RWseek(bmp_rw, start_offset + 28, RW_SEEK_SET) >= 0)
                    {
                        const Uint16 bits_per_pixel = SDL_ReadLE16(bmp_rw);

                        num_colors = (Uint32)1 << bits_per_pixel;
                        if (num_colors > 0 && num_colors <= (Uint32)INT_MAX &&
                            (int)num_colors <= bmp->format->palette->ncolors)
                                      *palette_size = (int)num_colors;
                    }
                }
            }
        }
    }

    SDL_RWclose(bmp_rw);
    return bmp;
}

#endif

static void _clean(void)
{
#ifdef PDC_WIDE
    if (pdc_ttffont)
    {
        TTF_CloseFont(pdc_ttffont);
        TTF_Quit();
        pdc_ttffont = NULL;
    }
#endif
    if( pdc_tileback)
        SDL_FreeSurface(pdc_tileback);
    if( pdc_back)
        SDL_FreeSurface(pdc_back);
    if( pdc_icon)
        SDL_FreeSurface(pdc_icon);
    if( pdc_font)
        SDL_FreeSurface(pdc_font);
    if( pdc_screen)
        SDL_FreeSurface(pdc_screen);
    pdc_screen = pdc_tileback = pdc_back = pdc_icon = pdc_font = NULL;
    if( pdc_own_window && pdc_window)
    {
        SDL_DestroyWindow(pdc_window);
        pdc_window = NULL;
    }

    SDL_Quit();
    pdc_sheight = pdc_swidth = 0;
}

void PDC_retile(void)
{
    if (pdc_tileback)
        SDL_FreeSurface(pdc_tileback);

    pdc_tileback = SDL_ConvertSurface(pdc_screen, pdc_screen->format, 0);
    if (pdc_tileback == NULL)
        return;

    if (pdc_back)
    {
        SDL_Rect dest;

        dest.y = 0;

        while (dest.y < pdc_tileback->h)
        {
            dest.x = 0;

            while (dest.x < pdc_tileback->w)
            {
                SDL_BlitSurface(pdc_back, 0, pdc_tileback, &dest);
                dest.x += pdc_back->w;
            }

            dest.y += pdc_back->h;
        }

        SDL_BlitSurface(pdc_tileback, 0, pdc_screen, 0);
    }
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
#ifdef PDC_WIDE
    const char *ptsz, *fname;
#endif

    PDC_LOG(("PDC_scr_open() - called\n"));

    pdc_own_window = !pdc_window;

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

#ifdef PDC_WIDE
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
#else
    if (pdc_font)
    {
        if (pdc_font->format->palette)
            pdc_flastc = pdc_font->format->palette->ncolors - 1;
    }
    else
    {
        int palette_size;

        const char *fname = getenv("PDC_FONT");
        SDL_RWops *file = SDL_RWFromFile(fname ? fname : "pdcfont.bmp", "r");
        if (file)
            pdc_font = _load_bmp_and_palette_size(file, &palette_size);

        if (!pdc_font)
            pdc_font = _load_bmp_and_palette_size(
                SDL_RWFromMem(font437, sizeof(font437)), &palette_size);

        if (!pdc_font)
        {
            fprintf(stderr, "Could not load font\n");
            return ERR;
        }

        if (pdc_font->format->palette)
            pdc_flastc = palette_size - 1;
    }

    SP->mono = !pdc_font->format->palette;
#endif

    if (!SP->mono && !pdc_back)
    {
        const char *bname = getenv("PDC_BACKGROUND");
        pdc_back = SDL_LoadBMP(bname ? bname : "pdcback.bmp");
    }

    if (!SP->mono && (pdc_back || !pdc_own_window))
    {
        SP->orig_attr = TRUE;
        SP->orig_fore = COLOR_WHITE;
        SP->orig_back = -1;
    }
    else
        SP->orig_attr = FALSE;

#ifdef PDC_WIDE
    TTF_SizeText(pdc_ttffont, "W", &pdc_fwidth, &pdc_fheight);
    pdc_fthick = pdc_font_size / 20 + 1;
#else
    pdc_fheight = pdc_font->h / 8;
    pdc_fwidth = pdc_font->w / 32;
    pdc_fthick = 1;
#endif

    if (pdc_own_window && !pdc_icon)
    {
        const char *iname = getenv("PDC_ICON");
        pdc_icon = SDL_LoadBMP(iname ? iname : "pdcicon.bmp");

        if (!pdc_icon)
            pdc_icon = SDL_LoadBMP_RW(SDL_RWFromMem(iconbmp,
                                                    sizeof(iconbmp)), 0);
    }

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
            pdc_swidth, pdc_sheight, SDL_WINDOW_RESIZABLE);

        if (pdc_window == NULL)
        {
            fprintf(stderr, "Could not open SDL window: %s\n", SDL_GetError());
            return ERR;
        }

        SDL_SetWindowIcon(pdc_window, pdc_icon);
    }

    /* Events must be pumped before calling SDL_GetWindowSurface, or
       initial modifiers (e.g. numlock) will be ignored and out-of-sync. */

    SDL_PumpEvents();

    /* Wait until window is exposed before getting surface */

    while (SDL_PollEvent(&event))
        if (SDL_WINDOWEVENT == event.type &&
            SDL_WINDOWEVENT_EXPOSED == event.window.event)
            break;

    if( !pdc_screen)
    {
        pdc_screen = SDL_GetWindowSurface(pdc_window);

        if( !pdc_screen)
        {
            fprintf(stderr, "Could not open SDL window surface: %s\n",
                    SDL_GetError());
            return ERR;
        }
    }

    if (!pdc_sheight)
        pdc_sheight = pdc_screen->h - pdc_yoffset;

    if (!pdc_swidth)
        pdc_swidth = pdc_screen->w - pdc_xoffset;

    if (SP->orig_attr)
        PDC_retile();

    SDL_StartTextInput();

    PDC_mouse_set();

    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->audible = FALSE;

    SP->termattrs = A_COLOR | WA_UNDERLINE | WA_LEFT | WA_RIGHT |
                    WA_REVERSE | WA_STRIKEOUT | WA_TOP | WA_BLINK | WA_DIM | WA_BOLD;
#ifdef PDC_WIDE
    SP->termattrs |= WA_ITALIC;
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
        pdc_screen = SDL_GetWindowSurface(pdc_window);
    }

    if (pdc_tileback)
        PDC_retile();

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
