/* PDCurses */

#include "pdcgl.h"

#include <stdlib.h>
#include <string.h>

# ifdef PDC_WIDE
#  define USE_UNICODE_ACS_CHARS 1
# else
#  define USE_UNICODE_ACS_CHARS 0
# endif

# include "../common/acs_defs.h"
# include "../common/pdccolor.h"

static chtype oldch = (chtype)(-1);    /* current attribute */
static int foregr = -2, backgr = -2; /* current foreground, background */
static bool blinked_off = FALSE;

static SDL_Color get_pdc_color( const int color_idx)
{
    SDL_Color c;
    const PACKED_RGB rgb = PDC_get_palette_entry( color_idx > 0 ? color_idx : 0);

    c.r = (Uint8)Get_RValue( rgb);
    c.g = (Uint8)Get_GValue( rgb);
    c.b = (Uint8)Get_BValue( rgb);
    return c;
}

static int cache_attr_index = 0;
static SDL_Texture* get_glyph_texture(Uint32 ch32)
{
    SDL_Texture* glyph = NULL;
    SDL_Color color = get_pdc_color(foregr);
    SDL_Color white = {255,255,255,255};

    int *cache_size = &pdc_glyph_cache_size[cache_attr_index];
    SDL_Texture*** cache = &pdc_glyph_cache[cache_attr_index];

    if(ch32 < *cache_size && (*cache)[ch32] != NULL)
    {
        glyph = (*cache)[ch32];
    }
    else
    {
        SDL_Surface* surf = NULL;

#ifdef PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT
        surf = TTF_RenderGlyph32_Blended(pdc_ttffont, ch32, white);
#else
        /* no support for supplementary planes */
        if (ch > 0xffff)
            ch = '?';

        surf = TTF_RenderGlyph_Blended(pdc_ttffont, (Uint16)ch32, white);
#endif
        glyph = SDL_CreateTextureFromSurface(pdc_renderer, surf);
        SDL_FreeSurface(surf);

        if(ch32 >= *cache_size)
        {
            int new_cache_size = *cache_size;
            if(new_cache_size == 0) new_cache_size = 256;
            while(new_cache_size < ch32) (new_cache_size) *= 2;

            *cache = realloc(*cache, sizeof(SDL_Texture*)*new_cache_size);
            memset(
                (*cache) + *cache_size, 0,
                sizeof(SDL_Texture*)*(new_cache_size - *cache_size)
            );
            *cache_size = new_cache_size;
        }
        (*cache)[ch32] = glyph;
    }

    SDL_SetTextureColorMod(glyph, color.r, color.g, color.b);
    return glyph;
}

static void fill_rect(SDL_Rect* rect, SDL_Color col)
{
    SDL_SetRenderDrawColor(pdc_renderer, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(pdc_renderer, rect);
}

static void blit_texture(SDL_Texture* glyph, SDL_Rect* src, SDL_Rect* dest)
{
    SDL_RenderCopy(pdc_renderer, glyph, src, dest);
}

/* set the font colors to match the chtype's attribute */

static void _set_attr(chtype ch)
{
    attr_t sysattrs = SP->termattrs;

#ifdef PDC_WIDE
    int bold = (ch & A_BOLD) && (sysattrs & A_BOLD);
    int italic = (ch & A_ITALIC) && (sysattrs & A_ITALIC);
#else
    int bold = 0;
    int italic = 0;
#endif
    cache_attr_index = (bold ? 1 : 0) | (italic ? 2 : 0);
    TTF_SetFontStyle(
        pdc_ttffont,
        (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0)
    );

    ch &= (A_COLOR|A_BOLD|A_BLINK|A_REVERSE);

    if (oldch != ch)
    {
        int newfg, newbg;

        if (SP->mono)
            return;

        extended_pair_content(PAIR_NUMBER(ch), &newfg, &newbg);

        if ((ch & A_BOLD) && !(sysattrs & A_BOLD))
            newfg |= 8;
        if ((ch & A_BLINK) && !(sysattrs & A_BLINK))
            newbg |= 8;

        if (ch & A_REVERSE)
        {
            int tmp = newfg;
            newfg = newbg;
            newbg = tmp;
        }

        foregr = newfg;

        if (newbg != backgr)
        {
            backgr = newbg;
        }

        oldch = ch;
    }
}

/* Draw some of the ACS_* "graphics" */

#define BIT_UP       1
#define BIT_DN       2
#define BIT_RT       4
#define BIT_LT       8
#define HORIZ   (BIT_LT | BIT_RT)
#define VERTIC  (BIT_UP | BIT_DN)
         /* Macros used for deciphering ACS_S1, ACS_S3, ACS_S7, ACS_S9     */
#define Sn_CHARS      0x10
#define SCAN_LINE( n)      (Sn_CHARS | ((n - 1) << 8))

static bool _grprint(chtype ch, const SDL_Rect dest)
{
    int i = 0;
    bool rval;
    const int remap_tbl[] = {
            ACS_ULCORNER, BIT_DN | BIT_RT,   ACS_LLCORNER, BIT_UP | BIT_RT,
            ACS_URCORNER, BIT_DN | BIT_LT,   ACS_LRCORNER, BIT_UP | BIT_LT,
            ACS_LTEE, VERTIC | BIT_RT,       ACS_RTEE, VERTIC | BIT_LT,
            ACS_TTEE, HORIZ | BIT_DN,        ACS_BTEE, HORIZ | BIT_UP,
            ACS_HLINE, HORIZ,                ACS_VLINE, VERTIC,
            ACS_PLUS, HORIZ | VERTIC,        ACS_BLOCK, 0,
            ACS_S1, HORIZ | SCAN_LINE( 1),     ACS_S3, HORIZ | SCAN_LINE( 3),
            ACS_S7, HORIZ | SCAN_LINE( 7),     ACS_S9, HORIZ | SCAN_LINE( 9),    0 };

    while( remap_tbl[i] && remap_tbl[i] != (int)ch)
        i += 2;
    if( remap_tbl[i] == (int)ch)
    {
        const int hmid = (pdc_fheight - pdc_fthick) >> 1;
        const int wmid = (pdc_fwidth - pdc_fthick) >> 1;
        const int mask = remap_tbl[i + 1];
        SDL_Rect temp = dest;
        const SDL_Color col = get_pdc_color( foregr);

        if( ch == ACS_BLOCK)
            fill_rect(&temp, col);
        if( mask & HORIZ)
        {
            temp.h = pdc_fthick;
            if( mask & Sn_CHARS)    /* extract scan line for ACS_Sn characters */
                temp.y += (mask >> 8) * hmid >> 2;
            else
                temp.y += hmid;
            switch( mask & HORIZ)
            {
                case BIT_RT:
                    temp.x += wmid;
                    temp.w -= wmid;
                    break;
                case BIT_LT:
                    temp.w = wmid + pdc_fthick;
                    break;
                case HORIZ:
                    break;
            }
            fill_rect(&temp, col);
        }
        temp = dest;
        if( mask & VERTIC)
        {
            temp.x += wmid;
            temp.w = pdc_fthick;
            switch( mask & VERTIC)
            {
                case BIT_DN:
                    temp.y += hmid;
                    temp.h -= hmid;
                    break;
                case BIT_UP:
                    temp.h = hmid + pdc_fthick;
                    break;
                case VERTIC:
                    break;
            }
            fill_rect(&temp, col);
        }
        rval = TRUE;
    }
    else
        rval = FALSE;   /* didn't draw it -- fall back to acs_map */
    return( rval);
}

/* draw a cursor at (y, x) */

void PDC_gotoyx(int row, int col)
{
    SDL_Rect src, dest;
    chtype ch;
    int oldrow, oldcol;

    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

    oldrow = SP->cursrow;
    oldcol = SP->curscol;

    /* clear the old cursor */

    PDC_transform_line(oldrow, oldcol, 1, curscr->_y[oldrow] + oldcol);

    if (!SP->visibility)
    {
        PDC_doupdate();
        return;
    }

    /* draw a new cursor by overprinting the existing character in
       reverse, either the full cell (when visibility == 2) or the
       lowest quarter of it (when visibility == 1) */

    ch = curscr->_y[row][col] ^ A_REVERSE;

    _set_attr(ch);

    src.h = (SP->visibility == 1) ? pdc_fheight >> 2 : pdc_fheight;
    src.w = pdc_fwidth;

    dest.y = (row + 1) * pdc_fheight - src.h;
    dest.x = col * pdc_fwidth;
    dest.h = src.h;
    dest.w = src.w;

    fill_rect(&dest, get_pdc_color( backgr));

    if (!(SP->visibility == 2 && _is_altcharset( ch) &&
        _grprint(ch & (0x7f | A_ALTCHARSET), dest)))
    {
        if( _is_altcharset( ch))
            ch = acs_map[ch & 0x7f];

        Uint32 ch32 = (Uint32)(ch & A_CHARTEXT);

        SDL_Texture* glyph = get_glyph_texture(ch32);

        if (glyph)
        {
            int w = 0;
            SDL_QueryTexture(glyph, NULL, NULL, &w, NULL);
            int center = pdc_fwidth > w ? (pdc_fwidth - w) >> 1 : 0;
            src.x = 0;
            src.y = pdc_fheight - src.h;
            dest.x += center;
            blit_texture(glyph, &src, &dest);
            dest.x -= center;
            glyph = NULL;
        }
    }

    PDC_doupdate();
}

void _new_packet(attr_t attr, int lineno, int x, int len, const chtype *srcp)
{
    SDL_Rect src, dest;
    int j;
    Uint32 prev_ch = 0;
    attr_t sysattrs = SP->termattrs;
    short hcol = SP->line_color;
    bool blink = blinked_off && (attr & A_BLINK) && (sysattrs & A_BLINK);

    src.x = 0;
    src.y = 0;
    src.h = pdc_fheight;
    src.w = pdc_fwidth;

    dest.y = pdc_fheight * lineno;
    dest.x = pdc_fwidth * x;
    dest.h = pdc_fheight;
    dest.w = pdc_fwidth * len;

    _set_attr(attr);

    fill_rect(&dest, get_pdc_color( backgr));

    if (hcol == -1)
        hcol = (short)foregr;

    SDL_Texture* glyph = NULL;
    for (j = 0; j < len; j++)
    {
        chtype ch = srcp[j];

        if (blink)
            ch = ' ';

        dest.w = pdc_fwidth;

        if( _is_altcharset( ch))
        {
            if (_grprint(ch & (0x7f | A_ALTCHARSET), dest))
            {
                dest.x += pdc_fwidth;
                continue;
            }
            ch = acs_map[ch & 0x7f];
        }

        ch &= A_CHARTEXT;

        if (ch != ' ')
        {
            if (prev_ch != ch)
            {
                prev_ch = ch;

                glyph = get_glyph_texture(prev_ch);
            }

            if (glyph)
            {
                int w = 0;
                SDL_QueryTexture(glyph, NULL, NULL, &w, NULL);
                int center = pdc_fwidth > w ? (pdc_fwidth - w) >> 1 : 0;
                dest.x += center;
                blit_texture(glyph, &src, &dest);
                dest.x -= center;
            }
        }

        if (!blink && (attr & (A_LEFT | A_RIGHT)))
        {
            dest.w = pdc_fthick;

            if (attr & A_LEFT)
                fill_rect(&dest, get_pdc_color( hcol));

            if (attr & A_RIGHT)
            {
                dest.x += pdc_fwidth - pdc_fthick;
                fill_rect(&dest, get_pdc_color( hcol));
                dest.x -= pdc_fwidth - pdc_fthick;
            }
        }

        dest.x += pdc_fwidth;
    }

    if (!blink && (attr & (A_UNDERLINE | A_OVERLINE | A_STRIKEOUT)))
    {
        dest.x = pdc_fwidth * x;
        dest.h = pdc_fthick;
        dest.w = pdc_fwidth * len;
        if( attr & A_OVERLINE)
           fill_rect(&dest, get_pdc_color( hcol));
        if( attr & A_UNDERLINE)
        {
           dest.y += pdc_fheight - pdc_fthick;
           fill_rect(&dest, get_pdc_color( hcol));
           dest.y -= pdc_fheight - pdc_fthick;
        }
        if( attr & A_STRIKEOUT)
        {
           dest.y += (pdc_fheight - pdc_fthick) / 2;
           fill_rect(&dest, get_pdc_color( hcol));
        }
    }
}

/* update the given physical line to look like the corresponding line in
   curscr */

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    attr_t old_attr, attr;
    int i, j;

    PDC_LOG(("PDC_transform_line() - called: lineno=%d\n", lineno));

    old_attr = *srcp & (A_ATTRIBUTES ^ A_ALTCHARSET);

    for (i = 1, j = 1; j < len; i++, j++)
    {
        attr = srcp[i] & (A_ATTRIBUTES ^ A_ALTCHARSET);

        if (attr != old_attr)
        {
            _new_packet(old_attr, lineno, x, i, srcp);
            old_attr = attr;
            srcp += i;
            x += i;
            i = 0;
        }
    }

    _new_packet(old_attr, lineno, x, i, srcp);
}

static Uint32 _blink_timer(Uint32 interval, void *param)
{
    SDL_Event event;

    INTENTIONALLY_UNUSED_PARAMETER( param);
    event.type = SDL_USEREVENT;
    SDL_PushEvent(&event);
    return(interval);
}

void PDC_blink_text(void)
{
    static SDL_TimerID blinker_id = 0;
    int i, j, k;

    oldch = (chtype)(-1);

    if (!(SP->termattrs & A_BLINK))
    {
        SDL_RemoveTimer(blinker_id);
        blinker_id = 0;
    }
    else if (!blinker_id)
    {
        blinker_id = SDL_AddTimer(500, _blink_timer, NULL);
        blinked_off = TRUE;
    }

    blinked_off = !blinked_off;

    for (i = 0; i < SP->lines; i++)
    {
        const chtype *srcp = curscr->_y[i];

        for (j = 0; j < SP->cols; j++)
            if (srcp[j] & A_BLINK)
            {
                k = j;
                while (k < SP->cols && (srcp[k] & A_BLINK))
                    k++;
                PDC_transform_line(i, j, k - j, srcp + j);
                j = k;
            }
    }

    oldch = (chtype)(-1);

    PDC_doupdate();
}

void PDC_doupdate(void)
{
    SDL_SetRenderTarget(pdc_renderer, NULL);
    SDL_RenderClear(pdc_renderer);
    SDL_RenderCopy(pdc_renderer, pdc_render_target, NULL, NULL);
    SDL_RenderPresent(pdc_renderer);
    SDL_SetRenderTarget(pdc_renderer, pdc_render_target);
}

void PDC_pump_and_peep(void)
{
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
        if (SDL_WINDOWEVENT == event.type &&
            (SDL_WINDOWEVENT_RESTORED == event.window.event ||
             SDL_WINDOWEVENT_EXPOSED == event.window.event ||
             SDL_WINDOWEVENT_SHOWN == event.window.event))
        {
            PDC_doupdate();
        }
        else
            SDL_PushEvent(&event);
    }
}
