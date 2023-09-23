/* PDCurses */

#include "pdcgl.h"
#include "glfuncs.h"

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


struct color_data
{
    /* The low three bytes are just the 8-bit RGB channel values. The most
     * significant byte is currently unused.
     */
    Uint32 bg_color;
    /* The most significant 8 bits contain the attribute data. The low three
     * bytes are just the 8-bit RGB channel values.
     */
    Uint32 fg_color;
};

/* We store all of the color pairs in the same format in which we'll also upload
 * them to OpenGL buffers. There's one per character grid cell on screen.
 * They're uploaded in one batch, the entire screen at once. If you want to
 * modify the contents of color_data, you'll also have to update the shaders
 * and vertex attrib arrays in pdcscrn.c!
 */
static struct color_data* color_grid = NULL;

#define BUILD_GLYPH_INDEX(col, row, w) \
    ((Uint32)(col) | (((Uint32)row) << 15u) | (((Uint32)w) << 30u))

/* Glyphs are stored in multiple layers, in order to support combining chars.
 * If none are used, there should only ever be one layer. The first layer is
 * always the one with regular characters.
 */
struct glyph_grid_layer
{
    /* We track the number of non-empty glyphs in the grid, so that we can know
     * when we can delete the layer.
     */
    Uint32 occupancy;

    /* The glyph info format is as below:
     *  0-14: character x offset in glyph cache
     * 15-29: character y offset in glyph cache
     * 30-31: Width; 0 = empty, 1 = normal, 2 = fullwidth.
     *
     * See also: BUILD_GLYPH_INDEX
     */
    Uint32* grid;
};
static struct glyph_grid_layer* glyph_grid_layers = NULL;
static int grid_w = 0, grid_h = 0, grid_layers = 0;
static int cur_render_target_w = 0, cur_render_target_h = 0;
static int cache_attr_index = 0;

static int next_pow_2(int n)
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

/* This function attempts to double the glyph cache size, but can also evict
 * unused characters out if growing is not an option.
 */
static void enlarge_glyph_cache()
{
    GLuint new_font_texture = 0;
    int new_glyph_cache_w = 2 * pdc_glyph_cache_w;
    int new_glyph_cache_h = 2 * pdc_glyph_cache_h;
    GLint max_texture_size = 0;
    int i, j, layer;
    const float clear_color[4] = {0,0,0,0};

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

    glBindFramebuffer(GL_FRAMEBUFFER, pdc_tex_fbo);
    glFramebufferTexture(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, new_font_texture, 0);
    glClearBufferfv(GL_COLOR, 0, clear_color);

    if(pdc_font_texture != 0)
    {
        /* Prepare old texture for re-use */
        glFramebufferTexture(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pdc_font_texture, 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    if(
        new_glyph_cache_w != pdc_glyph_cache_w ||
        new_glyph_cache_h != pdc_glyph_cache_h
    ){
        /* Enlarging the texture should be possible if we're here. */
        int new_glyph_row_capacity = new_glyph_cache_h / pdc_fheight;
        pdc_glyph_col_capacity = new_glyph_cache_w / pdc_fwidth;

        if(pdc_font_texture != 0)
        {
            /* Just keep the old data in the lower left corner; that also lets
             * us keep the old row capacities intact.
             */
            glCopyTexSubImage2D(
                GL_TEXTURE_2D,
                0, 0, 0, 0, 0, pdc_glyph_cache_w, pdc_glyph_cache_h
            );
        }
        pdc_glyph_cache_w = new_glyph_cache_w;
        pdc_glyph_cache_h = new_glyph_cache_h;
        pdc_glyph_start_col = realloc(
            pdc_glyph_start_col,
            sizeof(int) * new_glyph_row_capacity
        );
        for(i = pdc_glyph_row_capacity; i < new_glyph_row_capacity; ++i)
            pdc_glyph_start_col[i] = 0;

        pdc_glyph_row_capacity = new_glyph_row_capacity;
    }
    else
    {
        bool* visited = calloc(grid_w * grid_h * grid_layers, sizeof(bool));
        int attr;

        /* If we're here, it's not possible to enlarge the texture, so we have
         * to evict everything that's not needed out of the texture. This can
         * be really slow, but at least the output should be fine...
         */
        for(i = 0; i < pdc_glyph_row_capacity; ++i)
            pdc_glyph_start_col[i] = 0;

        for(attr = 0; attr < 4; ++attr)
        for(i = 0; i < pdc_glyph_cache_size[attr]; ++i)
        {
            Uint32* cached_glyph = pdc_glyph_cache[attr]+i;
            Uint32 old_glyph = *cached_glyph;
            bool used = FALSE;
            int w = old_glyph >> 30;
            int row;
            if(old_glyph == 0)
                continue;

            /* Check if glyph is used in any layer */
            for(layer = 0; layer < grid_layers; ++layer)
            for(j = 0; j < grid_w * grid_h; ++j)
            {
                if(
                    glyph_grid_layers[layer].grid[j] == old_glyph &&
                    !visited[j + layer * grid_w * grid_h]
                ){
                    used = TRUE;
                    break;
                }
            }

            *cached_glyph = 0;
            if(!used) /* Unused glyphs get thrown out. */
                continue;

            /* Used glyphs are given a new index and copied over. */
            for(row = 0; row < pdc_glyph_row_capacity; ++row)
            {
                int *col = &pdc_glyph_start_col[row];
                if(*col + w <= pdc_glyph_col_capacity)
                {
                    *cached_glyph = BUILD_GLYPH_INDEX(*col, row, w);

                    glCopyTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        (*col)*pdc_fwidth,
                        row * pdc_fheight,
                        (old_glyph&0x7FFF) * pdc_fwidth,
                        (old_glyph>>15&0x7FFF) * pdc_fheight,
                        pdc_fwidth * w, pdc_fheight
                    );

                    (*col) += w;
                    break;
                }
            }

            /* Update existing uses of the updated glyph */
            for(layer = 0; layer < grid_layers; ++layer)
            for(j = 0; j < grid_w * grid_h; ++j)
            {
                if(
                    glyph_grid_layers[layer].grid[j] == old_glyph &&
                    !visited[j + layer * grid_w * grid_h]
                ){
                    glyph_grid_layers[layer].grid[j] = *cached_glyph;
                    visited[j + layer * grid_w * grid_h] = TRUE;
                }
            }
        }

        free(visited);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(pdc_font_texture != 0)
        glDeleteTextures(1, &pdc_font_texture);
    pdc_font_texture = new_font_texture;
}

/* Attempts to find an empty slot in the glyph cache of 'w' character widths.
 * (w = 2 for fullwidth, otherwise typically just 1)
 */
static Uint32 alloc_glyph_cache(int w)
{
    /* Keep trying until we succeed. */
    for(;;)
    {
        int row;
        /* We try to fill the glyph cache in rows, from bottom to top. */
        for(row = 0; row < pdc_glyph_row_capacity; ++row)
        {
            int *col = &pdc_glyph_start_col[row];
            if(*col+w <= pdc_glyph_col_capacity)
            {
                Uint32 index = BUILD_GLYPH_INDEX(*col, row, w);
                (*col) += w;
                return index;
            }
        }

        /* If we're here, we failed to allocate the glyph, so we need to enlarge
         * the glyph cache and try again.
         */
        enlarge_glyph_cache();
    }
}

/* This one makes sure that the color and glyph grids are of the correct size
 * and have enough layers. It's a bit complicated, because resizing shouldn't
 * throw existing characters away.
 */
static void ensure_glyph_grid(int min_layers)
{
    int i, j, layer;
    if(SP->cols != grid_w || SP->lines != grid_h || grid_layers < min_layers)
    {
        /* Update color grid first */
        struct color_data* new_colors = malloc(
            sizeof(struct color_data) * SP->lines * SP->cols
        );
        for(j = 0; j < SP->lines; ++j)
        {
            i = 0;
            if(j < grid_h && color_grid)
            {
                int w = grid_w < SP->cols ? grid_w : SP->cols;
                memcpy(
                    &new_colors[j * SP->cols],
                    &color_grid[j * grid_w],
                    sizeof(struct color_data) * w
                );
                i = w;
            }
            for(; i < SP->cols; ++i)
            {
                struct color_data* cd = &new_colors[i + j * SP->cols];
                cd->bg_color = 0;
                cd->fg_color = 0;
            }
        }
        free(color_grid);
        color_grid = new_colors;

        /* Make sure we have enough grid layers */
        if(grid_layers < min_layers)
        {
            glyph_grid_layers = realloc(
                glyph_grid_layers,
                sizeof(struct glyph_grid_layer) * min_layers
            );
            for(layer = grid_layers; layer < min_layers; ++layer)
            {
                glyph_grid_layers[layer].occupancy = 0;
                glyph_grid_layers[layer].grid = NULL;
            }
            grid_layers = min_layers;
        }

        /* Update the glyph grids on each layer.  */
        for(layer = 0; layer < grid_layers; ++layer)
        {
            Uint32* new_glyphs = malloc(sizeof(Uint32) * SP->lines * SP->cols);
            for(j = 0; j < SP->lines; ++j)
            {
                i = 0;
                if(j < grid_h && glyph_grid_layers[layer].grid)
                {
                    int w = grid_w < SP->cols ? grid_w : SP->cols;
                    memcpy(
                        &new_glyphs[j * SP->cols],
                        &glyph_grid_layers[layer].grid[j * grid_w],
                        sizeof(Uint32) * w
                    );
                    i = w;
                }
                for(; i < SP->cols; ++i)
                    new_glyphs[i + j * SP->cols] = BUILD_GLYPH_INDEX(0, 0, 1);
            }

            free(glyph_grid_layers[layer].grid);
            glyph_grid_layers[layer].grid = new_glyphs;
        }

        grid_w = SP->cols;
        grid_h = SP->lines;
    }
}

/* Tries to find unused layers and delete them, so that we don't waste time on
 * unused glyph grids.
 */
static void shrink_glyph_grid()
{
    int layer;
    for(layer = 1; layer < grid_layers;)
    {
        if(glyph_grid_layers[layer].occupancy != 0)
        {
            ++layer;
            continue;
        }
        free(glyph_grid_layers[layer].grid);
        memmove(
            glyph_grid_layers+layer,
            glyph_grid_layers+layer+1,
            grid_layers-layer-1
        );
        grid_layers--;
    }
}

static Uint32 get_pdc_color( const int color_idx)
{
    const PACKED_RGB rgb = PDC_get_palette_entry(color_idx > 0 ? color_idx : 0);
    return
        (Uint32)Get_RValue(rgb) |
        (Uint32)Get_GValue(rgb)<<8 |
        (Uint32)Get_BValue(rgb)<<16;
}

static Uint32 get_glyph_texture_index(Uint32 ch32)
{
    SDL_Color white = {255,255,255,255};
    int *cache_size = &pdc_glyph_cache_size[cache_attr_index];
    Uint32 **cache = &pdc_glyph_cache[cache_attr_index];

    /* Fullwidth dummy char! 0 makes it stop existing! */
    if(ch32 == 0x110000) return 0;

#ifndef PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT
    /* no support for supplementary planes */
    if (ch32 > 0xffff)
        ch32 = '?';
#endif

    if(ch32 < (Uint32)*cache_size && (*cache)[ch32] > 0)
    {
        /* Nice, the character was alreadt cached */
        return (*cache)[ch32];
    }
    else
    {
        /* Here we need to render the character, it's not cached. */
        int w = 0;
        Uint32 index;
        SDL_Surface* surf = NULL;

#ifdef PDC_SDL_SUPPLEMENTARY_PLANES_SUPPORT
        surf = TTF_RenderGlyph32_Blended(pdc_ttffont, ch32, white);
#else
        surf = TTF_RenderGlyph_Blended(pdc_ttffont, (Uint16)ch32, white);
#endif
        SDL_LockSurface(surf);
        /* Kind-of-fullwidthness-but-not-really: Italics can also overstep
         * and cause w = 2, which should still render completely fine.
         */
        w = (surf->w + pdc_fwidth-1)/pdc_fwidth;
        index = alloc_glyph_cache(w);
        /* The SDL_Surface pitch may not match with the width, so we use this
         * to get glTexSubImage2D to deal with the proper pitch.
         */
        glPixelStorei(
            GL_UNPACK_ROW_LENGTH,
            surf->pitch / surf->format->BytesPerPixel
        );
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            (index&0x7FFF) * pdc_fwidth,
            (index>>15&0x7FFF) * pdc_fheight,
            surf->w,
            surf->h,
            GL_RGBA,
            GL_UNSIGNED_INT_8_8_8_8,
            surf->pixels
        );
        SDL_UnlockSurface(surf);
        SDL_FreeSurface(surf);

        if(ch32 >= (Uint32)*cache_size)
        {
            /* Our list of cached glyph indices is too small, so it has to grow
             * too.
             */
            int new_cache_size = *cache_size;
            if(new_cache_size == 0) new_cache_size = 256;
            while((Uint32)new_cache_size < ch32) new_cache_size *= 2;

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

#ifdef USING_COMBINING_CHARACTER_SCHEME
int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);
#endif

static void draw_glyph(
    int y, int x, attr_t attr, Uint32 ch32,
    Uint32 background,
    Uint32 foreground
){
    struct color_data* cd;
#ifdef USING_COMBINING_CHARACTER_SCHEME
    int layer = 0;
#endif
    int i = x + y * SP->cols;
    Uint32 gl_attrs =
        ((attr & WA_UNDERLINE) ? 1<<2 : 0) |
        ((attr & WA_TOP) ? 1<<3 : 0) |
        ((attr & WA_STRIKEOUT) ? 1<<4 : 0) |
        ((attr & WA_LEFT) ? 1<<5 : 0) |
        ((attr & WA_RIGHT) ? 1<<6 : 0);
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_glyph_grid(1);
    cd = &color_grid[i];
    cd->bg_color = background;
    cd->fg_color = foreground | (gl_attrs << 24);

#ifdef USING_COMBINING_CHARACTER_SCHEME
    /* Clear all layers above the base */
    for(layer = 1; layer < grid_layers; ++layer)
    {
        if(glyph_grid_layers[layer].grid[i] != 0)
        {
            glyph_grid_layers[layer].occupancy--;
            glyph_grid_layers[layer].grid[i] = 0;
        }
    }
    layer = 0;
    /* Go through the glyph combo if necessary, putting each on a different
     * layer. */
    while(ch32 > 0x110000)
    {
        layer++;
        ensure_glyph_grid(layer+1);

        cchar_t added = 0;
        ch32 = PDC_expand_combined_characters(ch32, &added);
        Uint32 glyph_index = get_glyph_texture_index(added);
        if(glyph_index != 0)
        {
            glyph_grid_layers[layer].occupancy++;
            glyph_grid_layers[layer].grid[i] = glyph_index;
        }
    }
#endif

    glyph_grid_layers[0].grid[i] = get_glyph_texture_index(ch32);
}

static void draw_cursor(int y, int x, int visibility)
{
    struct color_data* cd;
    Uint32 gl_attrs = visibility >= 0 && visibility <= 2 ? visibility : 0;
    if(y < 0 || y >= SP->lines || x < 0 || x >= SP->cols)
        return;

    ensure_glyph_grid(1);
    cd = &color_grid[x + y * SP->cols];
    cd->fg_color |= gl_attrs << 24;
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

        if (blink) ch = ' ';

        if( _is_altcharset( ch))
            ch = acs_map[ch & 0x7f];

        ch &= A_CHARTEXT;

        draw_glyph(lineno, x+j, attr, ch,
            get_pdc_color(backgr), get_pdc_color(foregr));
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
    int w, h;
    SDL_Rect viewport = PDC_get_viewport();
    bool use_render_target = pdc_interpolation_mode == PDC_GL_INTERPOLATE_BILINEAR &&
        pdc_resize_mode != PDC_GL_RESIZE_NORMAL;
    int u_screen_size, u_glyph_size, u_fthick, u_line_color;
    short hcol = SP->line_color;
    int layer;

    ensure_glyph_grid(1);

    /* Upload grid buffers at the start, before we queue the commands that need
     * them.
     */
    glBindBuffer(GL_ARRAY_BUFFER, pdc_color_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(struct color_data) * SP->lines * SP->cols,
        color_grid,
        GL_STREAM_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, pdc_glyph_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Uint32) * SP->lines * SP->cols,
        glyph_grid_layers[0].grid,
        GL_STREAM_DRAW
    );

    SDL_GetWindowSize(pdc_window, &w, &h);

    glViewport(0, 0, w, h);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if(use_render_target)
    {
        /* For bilinear interpolation, we unfortunately need a temporary render
         * target. If we tried to do interpolation on the glyphs in the shader,
         * their edges would still appear sharp, as they could not blend
         * between cell edges.
         */
        int content_w = SP->cols * pdc_fwidth;
        int content_h = SP->lines * pdc_fheight;

        if(!pdc_render_target_texture)
        {
            /* Need to allocate the render target texture. */
            glGenTextures(1, &pdc_render_target_texture);
            cur_render_target_w = cur_render_target_h = 0;
        }

        if(cur_render_target_w != content_w || cur_render_target_h != content_h)
        {
            /* Need to resize the render target texture. */
            glBindTexture(GL_TEXTURE_2D, pdc_render_target_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                content_w,
                content_h,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                NULL
            );
            cur_render_target_w = content_w;
            cur_render_target_h = content_h;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, pdc_tex_fbo);
        glFramebufferTexture(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pdc_render_target_texture, 0);

        glViewport(0, 0, content_w, content_h);
        glBindTexture(GL_TEXTURE_2D, pdc_font_texture);
    }
    else
    {
        /* If we're doing nearest-neighbor interpolation, we can just render
         * directly to the default framebuffer.
         */
        if(pdc_render_target_texture)
        {
            /* The user may have disabled bilinear interpolation, so we can
             * just free the render target here since we don't need it anymore.
             */
            glDeleteTextures(1, &pdc_render_target_texture);
            pdc_render_target_texture = 0;
            cur_render_target_w = cur_render_target_h = 0;
        }

        glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
    }

    /* Draw background colors */
    glUseProgram(pdc_background_shader_program);
    u_screen_size = glGetUniformLocation(
        pdc_background_shader_program, "screen_size");
    glUniform2i(u_screen_size, SP->cols, SP->lines);

    u_glyph_size = glGetUniformLocation(
        pdc_background_shader_program, "glyph_size");
    glUniform2i(u_glyph_size, pdc_fwidth, pdc_fheight);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, SP->lines * SP->cols);

    /* Prepare for drawing foreground glyphs. */
    glUseProgram(pdc_foreground_shader_program);

    u_screen_size = glGetUniformLocation(
        pdc_foreground_shader_program, "screen_size");
    glUniform2i(u_screen_size, SP->cols, SP->lines);

    u_glyph_size = glGetUniformLocation(
        pdc_foreground_shader_program, "glyph_size");
    glUniform2i(u_glyph_size, pdc_fwidth, pdc_fheight);

    u_fthick = glGetUniformLocation(pdc_foreground_shader_program, "fthick");
    glUniform1i(u_fthick, pdc_fthick);

    u_line_color = glGetUniformLocation(
        pdc_foreground_shader_program, "line_color");
    hcol = SP->line_color;
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

    /* Draw foreground colors, layer by layer. */
    for(layer = 0; layer < grid_layers; ++layer)
    {
        if(layer != 0)
        {
            /* The first layer had to be uploaded earlier to make sure that we
             * have some data in it for the background shader as well. Which is
             * why we only upload here if the layer isn't the first one.
             */
            glBufferData(
                GL_ARRAY_BUFFER,
                sizeof(Uint32) * SP->lines * SP->cols,
                glyph_grid_layers[layer].grid,
                GL_STREAM_DRAW
            );
        }
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, SP->lines * SP->cols);
    }

    if(use_render_target)
    {
        /* Since the bilinear interpolation case was rendering to a texture,
         * we still need to blit that over to the screen.
         */
        glBindFramebuffer(GL_READ_FRAMEBUFFER, pdc_tex_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0,
            0,
            cur_render_target_w,
            cur_render_target_h,
            viewport.x,
            viewport.y,
            viewport.x+viewport.w,
            viewport.y+viewport.h,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    SDL_GL_SwapWindow(pdc_window);

    shrink_glyph_grid();
}

void PDC_pump_and_peep(void)
{
    SDL_Event event;

    int res = SDL_PeepEvents(
        &event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
    if(res > 0)
    {
        if (SDL_WINDOWEVENT == event.type &&
            (SDL_WINDOWEVENT_RESTORED == event.window.event ||
             SDL_WINDOWEVENT_EXPOSED == event.window.event ||
             SDL_WINDOWEVENT_SHOWN == event.window.event))
        {
            SDL_PollEvent(&event);
            PDC_doupdate();
        }
    }
}
