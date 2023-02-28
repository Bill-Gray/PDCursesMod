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

struct vertex_data
{
    int x;
    int y;
    float bg_r, bg_g, bg_b;
    float fg_r, fg_g, fg_b;
    int glyph;
    int attr;
};

static struct vertex_data* vertices = NULL;
static size_t vertices_w = 0, vertices_h = 0;

static void ensure_vertices()
{
    if(SP->cols != vertices_w || SP->lines != vertices_h)
    {
        struct vertex_data* new_vertices = malloc(sizeof(struct vertex_data) * 6 * SP->lines * SP->cols);

        for(int j = 0; j < SP->lines; ++j)
        for(int i = 0; i < SP->cols; ++i)
        {
            struct vertex_data* vd = &new_vertices[(i + j * SP->cols) * 6];
            for(int k = 0; k < 6; ++k)
            {
                vd[k].bg_r = vd[k].bg_g = vd[k].bg_b = 0.0f;
                vd[k].fg_r = vd[k].fg_g = vd[k].fg_b = 0.0f;
                vd[k].glyph = -1;
                vd[k].attr = 0;
            }
            vd[0].x = vd[1].x = vd[3].x = i;
            vd[2].x = vd[4].x = vd[5].x = i+1;
            vd[0].y = vd[2].y = vd[5].y = j;
            vd[1].y = vd[3].y = vd[4].y = j+1;
        }

        for(int j = 0; j < vertices_h && j < SP->lines; ++j)
        for(int i = 0; i < vertices_w && i < SP->cols; ++i)
        {
            memcpy(
                &new_vertices[(i + j * SP->cols) * 6],
                &vertices[(i + j * vertices_w) * 6],
                sizeof(struct vertex_data) * 6
            );
        }

        free(vertices);
        vertices = new_vertices;
        vertices_w = SP->cols;
        vertices_h = SP->lines;
    }
}

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
static int get_glyph_texture_index(Uint32 ch32)
{
    SDL_Color white = {255,255,255,255};

    int *cache_size = &pdc_glyph_cache_size[cache_attr_index];
    int **cache = &pdc_glyph_cache[cache_attr_index];

    if(ch32 < *cache_size && (*cache)[ch32] >= 0)
    {
        return (*cache)[ch32];
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
        int index = pdc_glyph_index++;
        if(index >= pdc_glyph_capacity)
        {
            GLuint new_font_texture = 0;
            glGenTextures(1, &new_font_texture);
            glBindTexture(GL_TEXTURE_2D_ARRAY, new_font_texture);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                GL_RGBA8,
                pdc_fwidth,
                pdc_fheight,
                pdc_glyph_capacity * 2,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                NULL
            );
            // And this is why we require OpenGL 4.3 instead of 3.3...
            glCopyImageSubData(
                pdc_font_texture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                new_font_texture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                pdc_fwidth, pdc_fheight, pdc_glyph_capacity
            );
            glDeleteTextures(1, &pdc_font_texture);
            pdc_font_texture = new_font_texture;
            pdc_glyph_capacity *= 2;
        }
        SDL_LockSurface(surf);
        // ffs...
        glPixelStorei(GL_UNPACK_ROW_LENGTH, surf->pitch/surf->format->BytesPerPixel);
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0,
            0,
            0,
            index,
            pdc_fwidth,
            pdc_fheight,
            1,
            GL_BGRA,
            GL_UNSIGNED_INT_8_8_8_8_REV,
            surf->pixels
        );
        SDL_UnlockSurface(surf);
        SDL_FreeSurface(surf);

        if(ch32 >= *cache_size)
        {
            int new_cache_size = *cache_size;
            if(new_cache_size == 0) new_cache_size = 256;
            while(new_cache_size < ch32) new_cache_size *= 2;

            *cache = realloc(*cache, sizeof(int)*new_cache_size);
            memset(
                (*cache) + *cache_size, -1,
                sizeof(int)*(new_cache_size - *cache_size)
            );
            *cache_size = new_cache_size;
        }
        (*cache)[ch32] = index;
        return index;
    }
}

static void draw_background(int y, int x, SDL_Color background)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_vertices();
    struct vertex_data* vd = &vertices[(x + y * SP->cols) * 6];
    for(int i = 0; i < 6; ++i)
    {
        vd[i].bg_r = background.r * (1.0f / 255.0f);
        vd[i].bg_g = background.g * (1.0f / 255.0f);
        vd[i].bg_b = background.b * (1.0f / 255.0f);
        vd[i].glyph = -1;
        vd[i].attr = 0;
    }
}

static void draw_glyph(int y, int x, attr_t attr, int glyph_index, SDL_Color foreground)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_vertices();
    struct vertex_data* vd = &vertices[(x + y * SP->cols) * 6];
    for(int i = 0; i < 6; ++i)
    {
        vd[i].fg_r = foreground.r * (1.0f / 255.0f);
        vd[i].fg_g = foreground.g * (1.0f / 255.0f);
        vd[i].fg_b = foreground.b * (1.0f / 255.0f);
        vd[i].glyph = glyph_index;
        // TODO: Other attributes?
        vd[i].attr = 0;
    }
}

static void draw_cursor(int y, int x)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_vertices();
    struct vertex_data* vd = &vertices[(x + y * SP->cols) * 6];
    for(int i = 0; i < 6; ++i)
        vd[i].attr = 1;
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

/* draw a cursor at (y, x) */

void PDC_gotoyx(int row, int col)
{
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

    if( _is_altcharset( ch))
        ch = acs_map[ch & 0x7f];

    Uint32 ch32 = (Uint32)(ch & A_CHARTEXT);

    int glyph = get_glyph_texture_index(ch32);

    if(glyph >= 0)
        draw_glyph(row, col, ch & A_ATTRIBUTES, glyph, get_pdc_color(foregr));

    draw_cursor(row, col);

    PDC_doupdate();
}

void _new_packet(attr_t attr, int lineno, int x, int len, const chtype *srcp)
{
    int j;
    attr_t sysattrs = SP->termattrs;
    bool blink = blinked_off && (attr & A_BLINK) && (sysattrs & A_BLINK);

    _set_attr(attr);

    for (j = 0; j < len; j++)
    {
        chtype ch = srcp[j];

        draw_background(lineno, x+j, get_pdc_color(backgr));

        if (blink) ch = ' ';

        if( _is_altcharset( ch))
            ch = acs_map[ch & 0x7f];

        ch &= A_CHARTEXT;

        if (ch != ' ')
        {
            int glyph = get_glyph_texture_index(ch);
            if(glyph >= 0)
                draw_glyph(lineno, x+j, attr, glyph, get_pdc_color(foregr));
        }
        else
        {
            draw_glyph(lineno, x+j, attr, -1, get_pdc_color(foregr));
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
    ensure_vertices();

    int w, h;
    SDL_GetWindowSize(pdc_window, &w, &h);

    glViewport(0, 0, w, h);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int aligned_w = SP->cols * pdc_fwidth;
    int aligned_h = SP->lines * pdc_fheight;
    glViewport(0, h-aligned_h, aligned_w, aligned_h);

    int u_screen_size = glGetUniformLocation(pdc_shader_program, "screen_size");
    glUniform2i(u_screen_size, SP->cols, SP->lines);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(struct vertex_data) * 6 * SP->lines * SP->cols,
        vertices,
        GL_DYNAMIC_DRAW
    );
    glDrawArrays(GL_TRIANGLES, 0, SP->lines * SP->cols * 6);

    SDL_GL_SwapWindow(pdc_window);
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
