/* PDCurses */

#include "pdcgl.h"
#define GLFUNC_IMPL
#include "glfuncs.h"

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
int pdc_resize_mode = PDC_GL_RESIZE_NORMAL;
int pdc_interpolation_mode = PDC_GL_INTERPOLATE_BILINEAR;

Uint32 *pdc_glyph_cache[4] = {NULL, NULL, NULL, NULL};
int pdc_glyph_cache_size[4] = {0, 0, 0, 0};
int pdc_glyph_row_capacity = 0, pdc_glyph_col_capacity = 0;
int pdc_glyph_cache_w = 0, pdc_glyph_cache_h = 0;
int* pdc_glyph_start_col = NULL;

SDL_Window *pdc_window = NULL;
SDL_Surface *pdc_icon = NULL;
int pdc_sheight = 0, pdc_swidth = 0;

int pdc_fheight, pdc_fwidth, pdc_fthick;
unsigned pdc_color_buffer = 0, pdc_glyph_buffer = 0;
unsigned pdc_background_shader_program = 0, pdc_foreground_shader_program = 0;
unsigned pdc_font_texture = 0, pdc_render_target_texture = 0;
unsigned pdc_tex_fbo = 0;
static GLuint pdc_vao = 0;

static SDL_GLContext pdc_gl_context = NULL;

/* The same vertex shader is used in both shader programs, the one that draws
 * the background colors and the one that draws the foreground glyphs. Vertex
 * attribs are actually instance attributes here, and vertices are just
 * calculated on-the-fly instead of being read from a buffer.
 */
static const char* pdc_vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in uint v_bg;\n"
    "layout(location = 1) in uint v_fg;\n"
    "layout(location = 2) in uint v_glyph;\n"
    "uniform ivec2 screen_size;\n"
    "out vertex_data {\n"
    "    flat ivec2 glyph_offset;\n"
    "    flat int attr;\n"
    "    vec3 bg;\n"
    "    vec3 fg;\n"
    "    vec2 uv;\n"
    "} v_out;\n"
    "const vec2[6] uv_table = vec2[](\n"
    "    vec2(0, 0),\n"
    "    vec2(0, 1),\n"
    "    vec2(1, 0),\n"
    "    vec2(0, 1),\n"
    "    vec2(1, 1),\n"
    "    vec2(1, 0)\n"
    ");\n"
    "void main(void)\n"
    "{\n"
    "   int cell_index = gl_InstanceID;\n"
    "   vec2 uv = uv_table[gl_VertexID%6];\n"
    "   int x = cell_index % screen_size.x;\n"
    "   int y = cell_index / screen_size.x;\n"
    "   uv.x *= float(v_glyph >> 30);\n"
    "   vec2 pos = 2.0f * (vec2(x, y)+uv)/vec2(screen_size) - 1.0f;\n"
    "   gl_Position = vec4(pos.x, -pos.y, 0.0f, 1.0f);\n"
    "   v_out.glyph_offset = ivec2(int(v_glyph&0x7FFFu), int(v_glyph>>15u&0x7FFFu));\n"
    "   v_out.attr = int(v_fg>>24);\n"
    "   v_out.bg = vec3((v_bg&0xFFu), (v_bg>>8u)&0xFFu, (v_bg>>16u)&0xFFu)/float(0xFF);\n"
    "   v_out.fg = vec3((v_fg&0xFFu), (v_fg>>8u)&0xFFu, (v_fg>>16u)&0xFFu)/float(0xFF);\n"
    "   v_out.uv = uv;\n"
    "}\n";

static const char* pdc_background_fragment_shader_src =
    "#version 330 core\n"
    "in vertex_data {\n"
    "    flat ivec2 glyph_offset;\n"
    "    flat int attr;\n"
    "    vec3 bg;\n"
    "    vec3 fg;\n"
    "    vec2 uv;\n"
    "} v_in;\n"
    "out vec4 color;\n"
    "void main(void)\n"
    "{\n"
    "   color = vec4(v_in.bg, 1.0f);\n"
    "}\n";

static const char* pdc_foreground_fragment_shader_src =
    "#version 330 core\n"
    "in vertex_data {\n"
    "    flat ivec2 glyph_offset;\n"
    "    flat int attr;\n"
    "    vec3 bg;\n"
    "    vec3 fg;\n"
    "    vec2 uv;\n"
    "} v_in;\n"
    "out vec4 color;\n"
    "uniform sampler2D glyphs;\n"
    "uniform int fthick;\n"
    "uniform ivec2 glyph_size;\n"
    "uniform vec3 line_color;\n"
    "void main(void)\n"
    "{\n"
    "   float g_alpha = 0;\n"
    "   ivec2 coord = ivec2(v_in.uv * glyph_size);\n"
    "   if(v_in.glyph_offset.x > 0 || v_in.glyph_offset.y > 0)\n"
    "       g_alpha = texelFetch(glyphs, v_in.glyph_offset * glyph_size + coord, 0).r;\n"
    "   if(((v_in.attr & 1) != 0 && coord.y >= 0.75 * glyph_size.y) || (v_in.attr & 2) != 0)\n"
    "       g_alpha = 1 - g_alpha;\n"
    "   vec3 c = v_in.fg;\n"
    "   if(\n"
    "       ((v_in.attr & (1<<2)) != 0 && coord.y >= glyph_size.y-fthick) ||\n" /* Underline */
    "       ((v_in.attr & (1<<3)) != 0 && coord.y < fthick) ||\n" /* Overline */
    "       ((v_in.attr & (1<<4)) != 0 && coord.y >= glyph_size.y/2-fthick && coord.y < glyph_size.y/2) ||\n" /* Strikeout */
    "       ((v_in.attr & (1<<5)) != 0 && coord.x < fthick) ||\n" /* Left */
    "       ((v_in.attr & (1<<6)) != 0 && coord.x >= glyph_size.x-fthick)\n" /* Right */
    "   ){\n"
    "       c = all(greaterThanEqual(line_color, vec3(0))) ? line_color : v_in.fg;\n"
    "       g_alpha = 1.0f;\n"
    "   }\n"
    "   color = vec4(c, g_alpha);\n"
    "}\n";

static void _clean(void)
{
    int i;
    if (pdc_ttffont)
    {
        TTF_CloseFont(pdc_ttffont);
        TTF_Quit();
        pdc_ttffont = NULL;
    }
    for(i = 0; i < 4; ++i)
    {
        if(pdc_glyph_cache[i])
            free(pdc_glyph_cache[i]);
        pdc_glyph_cache[i] = NULL;
        pdc_glyph_cache_size[i] = 0;
    }

    if(pdc_glyph_start_col)
    {
        free(pdc_glyph_start_col);
        pdc_glyph_start_col = NULL;
    }

    if( pdc_icon)
        SDL_FreeSurface(pdc_icon);

    pdc_icon = NULL;

    if(pdc_tex_fbo)
        glDeleteFramebuffers(1, &pdc_tex_fbo);

    if(pdc_font_texture)
        glDeleteTextures(1, &pdc_font_texture);

    if(pdc_render_target_texture)
        glDeleteTextures(1, &pdc_render_target_texture);

    if(pdc_vao)
        glDeleteVertexArrays(1, &pdc_vao);

    if(pdc_color_buffer)
        glDeleteBuffers(1, &pdc_color_buffer);

    if(pdc_glyph_buffer)
        glDeleteBuffers(1, &pdc_glyph_buffer);

    if(pdc_foreground_shader_program)
        glDeleteProgram(pdc_foreground_shader_program);

    if(pdc_background_shader_program)
        glDeleteProgram(pdc_background_shader_program);

    pdc_tex_fbo = pdc_font_texture = pdc_render_target_texture = pdc_vao =
        pdc_color_buffer = pdc_glyph_buffer = pdc_background_shader_program =
        pdc_foreground_shader_program = 0;

    if(pdc_gl_context)
    {
        SDL_GL_DeleteContext(pdc_gl_context);
        pdc_gl_context = NULL;
    }

    if(pdc_window)
    {
        SDL_DestroyWindow(pdc_window);
        pdc_window = NULL;
    }

    SDL_Quit();
    pdc_sheight = pdc_swidth = 0;
}

static void add_shader(
    GLuint shader_program,
    GLenum shader_type,
    const char* src
){
    GLint status = GL_FALSE;
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        char* err;
        GLsizei length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        err = malloc(length+1);
        glGetShaderInfoLog(shader, length+1, &length, err);
        fprintf(stderr, "%s shader compilation error: %s\n",
            (shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
            err
        );
        exit(1);
    }

    glAttachShader(shader_program, shader);
    glDeleteShader(shader);
}

static void build_shader_program(GLuint shader_program)
{
    GLint status = GL_FALSE;
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);

    if(status != GL_TRUE)
    {
        char* err;
        GLsizei length = 0;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &length);
        err = malloc(length+1);
        glGetProgramInfoLog(shader_program, length+1, &length, err);
        fprintf(stderr, "Shader program linking: %s\n", err);
        exit(1);
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
    int displaynum = 0;
    int h, w;
    const char *ptsz, *fname;
    const char *resize_mode = getenv( "PDC_RESIZE");

    if( resize_mode)
       pdc_resize_mode = atoi( resize_mode);

    PDC_LOG(("PDC_scr_open() - called\n"));

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) < 0)
    {
        fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());
        return ERR;
    }

    atexit(_clean);

    displaynum = _get_displaynum();

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

    SP->orig_attr = FALSE;

    TTF_SizeText(pdc_ttffont, "W", &pdc_fwidth, &pdc_fheight);
    pdc_fthick = pdc_font_size / 20 + 1;

    if (!pdc_icon)
    {
        const char *iname = getenv("PDC_ICON");
        pdc_icon = SDL_LoadBMP(iname ? iname : "pdcicon.bmp");

        if (!pdc_icon)
            pdc_icon = SDL_LoadBMP_RW(SDL_RWFromMem(iconbmp,
                                                    sizeof(iconbmp)), 0);
    }

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

    pdc_window = SDL_CreateWindow("PDCurses",
        SDL_WINDOWPOS_CENTERED_DISPLAY(displaynum),
        SDL_WINDOWPOS_CENTERED_DISPLAY(displaynum),
        pdc_swidth, pdc_sheight,
        SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);

    if (pdc_window == NULL)
    {
        fprintf(stderr, "Could not open SDL window: %s\n", SDL_GetError());
        return ERR;
    }

    SDL_SetWindowIcon(pdc_window, pdc_icon);

    SDL_GetWindowSize(pdc_window, &w, &h);

    if (!pdc_sheight) pdc_sheight = h;
    if (!pdc_swidth) pdc_swidth = w;

    pdc_gl_context = SDL_GL_CreateContext(pdc_window);
    if (pdc_gl_context == NULL)
    {
        fprintf(stderr, "Could not open OpenGL context: %s\n", SDL_GetError());
        return ERR;
    }
    SDL_GL_MakeCurrent(pdc_window, pdc_gl_context);

    SDL_GL_SetSwapInterval(0);

    /* Load the GL functions we use. */
    load_gl_funcs();

    /* Build foreground shader. */
    pdc_foreground_shader_program = glCreateProgram();
    add_shader(
        pdc_foreground_shader_program,
        GL_VERTEX_SHADER,
        pdc_vertex_shader_src
    );
    add_shader(
        pdc_foreground_shader_program,
        GL_FRAGMENT_SHADER,
        pdc_foreground_fragment_shader_src
    );
    build_shader_program(pdc_foreground_shader_program);

    /* Build background shader. */
    pdc_background_shader_program = glCreateProgram();
    add_shader(
        pdc_background_shader_program,
        GL_VERTEX_SHADER,
        pdc_vertex_shader_src
    );
    add_shader(
        pdc_background_shader_program,
        GL_FRAGMENT_SHADER,
        pdc_background_fragment_shader_src
    );
    build_shader_program(pdc_background_shader_program);

    /* A VAO is just needed in OpenGL 3.3, even though we don't ever change the
     * vertex attribs...
     */
    glGenVertexArrays(1, &pdc_vao);
    glBindVertexArray(pdc_vao);

    glGenBuffers(1, &pdc_color_buffer);
    glGenBuffers(1, &pdc_glyph_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, pdc_color_buffer);

    /* See 'struct color_data' in pdcdisp.c, these map the contents of that
     * struct into the vertex shader inputs.
     */
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(
        0, 1, GL_INT, 2 * sizeof(Uint32), (void*)(0 * sizeof(Uint32)));
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(
        1, 1, GL_INT, 2 * sizeof(Uint32), (void*)(1 * sizeof(Uint32)));
    glVertexAttribDivisor(1, 1);

    /* The glyph buffer is separate, because that makes rendering multiple
     * character layers (needed for combining characters) faster and simpler.
     */
    glBindBuffer(GL_ARRAY_BUFFER, pdc_glyph_buffer);
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(Uint32), NULL);
    glVertexAttribDivisor(2, 1);

    glUniform1i(
        glGetUniformLocation(pdc_foreground_shader_program, "glyphs"), 0);

    /* This FBO is not only used in the bilinear filtering mode, but also
     * temporarily in various glyph cache operations.
     */
    glGenFramebuffers(1, &pdc_tex_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pdc_tex_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Glyphs are alpha-blended on top of the background (and each other, if
     * combing chars)
     */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_StartTextInput();

    PDC_mouse_set();

    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->audible = FALSE;

    SP->termattrs = A_COLOR | WA_UNDERLINE | WA_LEFT | WA_RIGHT |
                    WA_REVERSE | WA_STRIKEOUT | WA_TOP | WA_BLINK | WA_DIM | WA_BOLD;
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

SDL_Rect PDC_get_viewport(void)
{
    int content_w = SP->cols * pdc_fwidth;
    int content_h = SP->lines * pdc_fheight;
    int scale = 0;
    int w, h;
    SDL_Rect rect;

    SDL_GetWindowSize(pdc_window, &w, &h);

    switch(pdc_resize_mode)
    {
    case PDC_GL_RESIZE_NORMAL:
        rect.x = 0;
        rect.y = h-content_h;
        rect.w = content_w;
        rect.h = content_h;
        break;

    case PDC_GL_RESIZE_STRETCH:
        rect.x = 0;
        rect.y = 0;
        rect.w = w;
        rect.h = h;
        break;

    case PDC_GL_RESIZE_SCALE:
        if(content_h == 0) content_h = 1;
        if(content_w == 0) content_w = 1;
        if(w * content_h < content_w * h)
        {
            scale = content_h * w / content_w;
            rect.x = 0;
            rect.y = (h - scale)/2;
            rect.w = w;
            rect.h = scale;
        }
        else
        {
            scale = content_w * h / content_h;
            rect.x = (w - scale)/2;
            rect.y = 0;
            rect.w = scale;
            rect.h = h;
        }
        break;

    case PDC_GL_RESIZE_INTEGER:
        if(content_h == 0) content_h = 1;
        if(content_w == 0) content_w = 1;
        scale = h/content_h < w/content_w ? h/content_h : w/content_w;
        if(scale <= 0) scale = 1;
        content_w = scale * content_w;
        content_h = scale * content_h;
        rect.x = (w-content_w)/2;
        rect.y = (h-content_h)/2;
        rect.w = content_w;
        rect.h = content_h;
        break;
    }
    return rect;
}
