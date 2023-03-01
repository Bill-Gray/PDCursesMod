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

struct instance_data
{
    Uint32 bg_color;
    Uint32 fg_color; /* The most significant 8 bits contain the attribute data */
    Uint32 glyph;
};

static struct instance_data* instances = NULL;
static size_t instances_w = 0, instances_h = 0;
static int cache_attr_index = 0;

static unsigned next_pow_2(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

static void enlarge_glyph_cache()
{
    GLuint new_font_texture = 0;
    unsigned new_glyph_cache_w = 2 * pdc_glyph_cache_w;
    unsigned new_glyph_cache_h = 2 * pdc_glyph_cache_h;
    GLint max_texture_size = 0; 
    if(new_glyph_cache_w == 0 || new_glyph_cache_h == 0)
    {
        new_glyph_cache_h = new_glyph_cache_w = next_pow_2(
            (pdc_fwidth > pdc_fheight ? pdc_fwidth : pdc_fheight) * 16);
    }

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    if(
        new_glyph_cache_w > max_texture_size ||
        new_glyph_cache_h > max_texture_size
    ){
        new_glyph_cache_w = max_texture_size;
        new_glyph_cache_h = max_texture_size;
    }

    glGenTextures(1, &new_font_texture);
    glBindTexture(GL_TEXTURE_2D, new_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        new_glyph_cache_w,
        new_glyph_cache_h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL
    );

    if(
        new_glyph_cache_w != pdc_glyph_cache_w ||
        new_glyph_cache_h != pdc_glyph_cache_h
    ){
        /* Enlarging the texture should be possible if we're here. */
        if(pdc_font_texture != 0)
        {
            /* And this is why we require OpenGL 4.3 instead of 3.3... */
            glCopyImageSubData(
                pdc_font_texture, GL_TEXTURE_2D, 0, 0, 0, 0,
                new_font_texture, GL_TEXTURE_2D, 0, 0, 0, 0,
                pdc_glyph_cache_w, pdc_glyph_cache_h, 1
            );
        }
        pdc_glyph_cache_w = new_glyph_cache_w;
        pdc_glyph_cache_h = new_glyph_cache_h;
        int new_glyph_row_capacity = pdc_glyph_cache_h / pdc_fheight;
        pdc_glyph_col_capacity = pdc_glyph_cache_w / pdc_fwidth;
        pdc_glyph_start_col = realloc(
            pdc_glyph_start_col,
            sizeof(unsigned) * new_glyph_row_capacity
        );
        for(
            unsigned i = pdc_glyph_row_capacity;
            i < new_glyph_row_capacity;
            ++i
        ){
            pdc_glyph_start_col[i] = 0;
        }

        /* Reserve room for index 0 (we want to use index 0 to mark empty) */
        pdc_glyph_start_col[0] =
            pdc_glyph_start_col[0] > 1 ? pdc_glyph_start_col[0] : 1;
        pdc_glyph_row_capacity = new_glyph_row_capacity;
    }
    else
    {
        /* If we're here, it's not possible to enlarge the texture, so we have 
         * to evict everything that's not needed out of the texture. This can
         * be really slow, but at least the output should be fine...
         */
        for(unsigned i = 0; i < pdc_glyph_row_capacity; ++i)
            pdc_glyph_start_col[i] = 0;
        /* Reserve room for index 0 (we want to use index 0 to mark empty) */
        pdc_glyph_start_col[0] = 1;

        for(int attr = 0; attr < 4; ++attr)
        for(size_t i = 0; i < pdc_glyph_cache_size[attr]; ++i)
        {
            Uint32* cached_glyph = pdc_glyph_cache[attr]+i;
            Uint32 old_glyph = *cached_glyph;
            if(old_glyph == 0)
                continue;

            bool used = FALSE;
            for(size_t j = 0; j < instances_w * instances_h; ++j)
            {
                if(instances[j].glyph == old_glyph)
                {
                    used = TRUE;
                    break;
                }
            }

            *cached_glyph = 0;
            if(!used) /* Unused glyphs get thrown out. */
                continue;

            /* Used glyphs are given a new index and copied over. */
            for(unsigned row = 0; row < pdc_glyph_row_capacity; ++row)
            {
                unsigned *col = &pdc_glyph_start_col[row];
                if(*col < pdc_glyph_col_capacity)
                {
                    Uint32 index = (Uint32)(*col) | (((Uint32)row)<<16);
                    *cached_glyph = index;

                    glCopyImageSubData(
                        pdc_font_texture, GL_TEXTURE_2D, 0,
                        (old_glyph&0xFFFF) * pdc_fwidth,
                        (old_glyph>>16) * pdc_fheight, 0,
                        new_font_texture, GL_TEXTURE_2D, 0,
                        (*col) * pdc_fwidth, row * pdc_fheight, 0,
                        pdc_fwidth, pdc_fheight, 1
                    );

                    (*col)++;
                    break;
                }
            }

            for(size_t j = 0; j < instances_w * instances_h; ++j)
            {
                if(instances[j].glyph == old_glyph)
                {
                    /* Setting the top-most bit allows us to not mix the
                     * already-changed glyph indices up with old ones that need
                     * updating. It's just used as an arbitrary marker. */
                    instances[j].glyph = *cached_glyph | (1u<<31);
                }
            }
        }

        for(size_t j = 0; j < instances_w * instances_h; ++j)
        {
            /* We can clear the top-most bit on all glyphs now. */
            instances[j].glyph &= ~(1u<<31);
        }

    }
    if(pdc_font_texture != 0)
        glDeleteTextures(1, &pdc_font_texture);
    pdc_font_texture = new_font_texture;
}

static Uint32 alloc_glyph_cache()
{
    /* Keep trying until we succeed. */
    for(;;)
    {
        for(unsigned row = 0; row < pdc_glyph_row_capacity; ++row)
        {
            unsigned *col = &pdc_glyph_start_col[row];
            if(*col < pdc_glyph_col_capacity)
            {
                Uint32 index = (Uint32)(*col) | (((Uint32)row)<<16);
                (*col)++;
                return index;
            }
        }

        /* If we're here, we failed to allocate the glyph, so we need to enlarge
         * the glyph cache. */
        enlarge_glyph_cache();
    }
}

static void ensure_instances()
{
    if(SP->cols != instances_w || SP->lines != instances_h)
    {
        struct instance_data* new_instances = malloc(sizeof(struct instance_data) * SP->lines * SP->cols);

        for(int j = 0; j < SP->lines; ++j)
        for(int i = 0; i < SP->cols; ++i)
        {
            struct instance_data* vd = &new_instances[i + j * SP->cols];
            vd->bg_color = 0;
            vd->fg_color = 0;
            vd->glyph = 0;
        }

        for(int j = 0; j < instances_h && j < SP->lines; ++j)
        for(int i = 0; i < instances_w && i < SP->cols; ++i)
        {
            memcpy(
                &new_instances[i + j * SP->cols],
                &instances[i + j * instances_w],
                sizeof(struct instance_data)
            );
        }

        free(instances);
        instances = new_instances;
        instances_w = SP->cols;
        instances_h = SP->lines;
    }
}

static Uint32 get_pdc_color( const int color_idx)
{
    const PACKED_RGB rgb = PDC_get_palette_entry( color_idx > 0 ? color_idx : 0);
    return ((Uint32)Get_RValue(rgb) | (Uint32)Get_GValue(rgb)<<8 |  (Uint32)Get_BValue(rgb)<<16);
}

static int get_glyph_texture_index(Uint32 ch32)
{
    SDL_Color white = {255,255,255,255};
    size_t *cache_size = &pdc_glyph_cache_size[cache_attr_index];
    Uint32 **cache = &pdc_glyph_cache[cache_attr_index];

#ifndef PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT
    /* no support for supplementary planes */
    if (ch32 > 0xffff)
        ch32 = '?';
#endif

    if(ch32 < *cache_size && (*cache)[ch32] > 0)
    {
        return (*cache)[ch32];
    }
    else
    {
        SDL_Surface* surf = NULL;

#ifdef PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT
        surf = TTF_RenderGlyph32_Blended(pdc_ttffont, ch32, white);
#else
        surf = TTF_RenderGlyph_Blended(pdc_ttffont, (Uint16)ch32, white);
#endif
        SDL_LockSurface(surf);
        Uint32 index = alloc_glyph_cache();
        glPixelStorei(GL_UNPACK_ROW_LENGTH, surf->pitch/surf->format->BytesPerPixel);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            (index&0xFFFF) * pdc_fwidth,
            (index>>16) * pdc_fheight,
            surf->w,
            surf->h,
            GL_RGBA,
            GL_UNSIGNED_INT_8_8_8_8,
            surf->pixels
        );
        SDL_UnlockSurface(surf);
        SDL_FreeSurface(surf);

        if(ch32 >= *cache_size)
        {
            int new_cache_size = *cache_size;
            if(new_cache_size == 0) new_cache_size = 256;
            while(new_cache_size < ch32) new_cache_size *= 2;

            *cache = realloc(*cache, sizeof(Uint32)*new_cache_size);
            memset(
                (*cache) + *cache_size, 0,
                sizeof(Uint32)*(new_cache_size - *cache_size)
            );
            *cache_size = new_cache_size;
        }
        (*cache)[ch32] = index;
        return index;
    }
}

static void draw_background(int y, int x, Uint32 background)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_instances();
    struct instance_data* vd = &instances[x + y * SP->cols];
    vd->bg_color = background;
    vd->glyph = 0;
}

static void draw_glyph(int y, int x, attr_t attr, Uint32 glyph_index, Uint32 foreground)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_instances();
    struct instance_data* vd = &instances[x + y * SP->cols];
    vd->glyph = glyph_index;
    Uint32 gl_attrs =
        ((attr & A_UNDERLINE) ? 1<<2 : 0) |
        ((attr & A_OVERLINE) ? 1<<3 : 0) |
        ((attr & A_STRIKEOUT) ? 1<<4 : 0) |
        ((attr & A_LEFT) ? 1<<5 : 0) |
        ((attr & A_RIGHT) ? 1<<6 : 0);
    vd->fg_color = foreground | (gl_attrs << 24);
}

static void draw_cursor(int y, int x, int visibility)
{
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_instances();
    struct instance_data* vd = &instances[x + y * SP->cols];
    Uint32 gl_attrs = visibility >= 0 && visibility <= 2 ? visibility : 0;
    vd->fg_color |= gl_attrs << 24;
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

    if (SP->visibility)
        draw_cursor(row, col, SP->visibility);
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
            Uint32 glyph = get_glyph_texture_index(ch);
            if(glyph > 0)
                draw_glyph(lineno, x+j, attr, glyph, get_pdc_color(foregr));
        }
        else
        {
            draw_glyph(lineno, x+j, attr, 0, get_pdc_color(foregr));
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
    ensure_instances();

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(struct instance_data) * SP->lines * SP->cols,
        instances,
        GL_STREAM_DRAW
    );

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

    int u_glyph_size = glGetUniformLocation(pdc_shader_program, "glyph_size");
    glUniform2i(u_glyph_size, pdc_fwidth, pdc_fheight);

    int u_fthick = glGetUniformLocation(pdc_shader_program, "fthick");
    glUniform1i(u_fthick, pdc_fthick);

    int u_line_color = glGetUniformLocation(pdc_shader_program, "line_color");
    short hcol = SP->line_color;
    if(hcol >= 0)
    {
        PACKED_RGB rgb = PDC_get_palette_entry(hcol);
        glUniform3f(u_line_color,
            Get_RValue(rgb)/255.0f,
            Get_GValue(rgb)/255.0f,
            Get_BValue(rgb)/255.0f
        );
    }
    else glUniform3f(u_line_color, -1, -1, -1);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, SP->lines * SP->cols);

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
