/* PDCurses */

#include "pdcgl.h"

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
int *pdc_glyph_cache[4] = {NULL, NULL, NULL, NULL};
int pdc_glyph_cache_size[4] = {0, 0, 0, 0};
int pdc_glyph_index = 0;
int pdc_glyph_capacity = 0;

SDL_Window *pdc_window = NULL;
SDL_Surface *pdc_icon = NULL;
int pdc_sheight = 0, pdc_swidth = 0;

int pdc_fheight, pdc_fwidth, pdc_fthick;
GLuint pdc_vbo = 0;
GLuint pdc_shader_program = 0;
GLuint pdc_font_texture = 0;
static GLuint pdc_vao = 0;

static SDL_GLContext pdc_gl_context = NULL;
static const char* pdc_vertex_shader_src =
    "#version 430 core\n"
    "layout(location = 0) in ivec2 v_pos;\n"
    "layout(location = 1) in vec3 v_bg;\n"
    "layout(location = 2) in vec3 v_fg;\n"
    "layout(location = 3) in int v_glyph;\n"
    "layout(location = 4) in int v_attr; // R = texture index, G = attr_t\n"
    "uniform ivec2 screen_size;\n"
    "out vertex_data {\n"
    "    flat int glyph;\n"
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
    "   vec2 pos = 2.0f * vec2(v_pos)/vec2(screen_size) - 1.0f;\n"
    "   gl_Position = vec4(pos.x, -pos.y, 0.0f, 1.0f);\n"
    "   v_out.glyph = v_glyph;\n"
    "   v_out.attr = v_attr;\n"
    "   v_out.bg = v_bg;\n"
    "   v_out.fg = v_fg;\n"
    "   v_out.uv = uv_table[gl_VertexID%6];\n"
    "}\n"
    ;
static const char* pdc_fragment_shader_src =
    "#version 430 core\n"
    "in vertex_data {\n"
    "    flat int glyph;\n"
    "    flat int attr;\n"
    "    vec3 bg;\n"
    "    vec3 fg;\n"
    "    vec2 uv;\n"
    "} v_in;\n"
    "out vec4 color;\n"
    "uniform sampler2DArray glyphs;\n"
    "void main(void)\n"
    "{\n"
    "   vec4 g_color = vec4(0);\n"
    "   if(v_in.glyph >= 0)\n"
    "       g_color = texture(glyphs, vec3(v_in.uv, v_in.glyph));\n"
    "   if(v_in.attr != 0 && v_in.uv.y > 0.8) g_color.a = 1;\n" // TODO: use fthick somehow?
    "   color = vec4(mix(v_in.bg, v_in.fg, g_color.a), 1.0f);\n"
    //"   color = vec4(v_in.uv, 0.0f, 1.0f);\n"
    "}\n";

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
        if(pdc_glyph_cache[i])
            free(pdc_glyph_cache[i]);
        pdc_glyph_cache[i] = NULL;
        pdc_glyph_cache_size[i] = 0;
    }

    if( pdc_icon)
        SDL_FreeSurface(pdc_icon);

    pdc_icon = NULL;

    if(pdc_font_texture)
    {
        glDeleteTextures(1, &pdc_font_texture);
        pdc_font_texture = 0;
    }

    if(pdc_vao)
    {
        glDeleteVertexArrays(1, &pdc_vao);
        pdc_vao = 0;
    }

    if(pdc_vbo)
    {
        glDeleteBuffers(1, &pdc_vbo);
        pdc_vbo = 0;
    }

    if(pdc_shader_program)
    {
        glDeleteProgram(pdc_shader_program);
        pdc_shader_program = 0;
    }

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

static void add_shader(GLenum shader_type, const char* src)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        GLsizei length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* err = malloc(length+1);
        glGetShaderInfoLog(shader, length+1, &length, err);
        fprintf(stderr, "%s shader compilation error: %s\n",
            (shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
            err
        );
        exit(1);
    }

    glAttachShader(pdc_shader_program, shader);
    glDeleteShader(shader);
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

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

    int h, w;
    SDL_GetWindowSize(pdc_window, &w, &h);

    pdc_gl_context = SDL_GL_CreateContext(pdc_window);
    if (pdc_gl_context == NULL)
    {
        fprintf(stderr, "Could not open OpenGL context: %s\n", SDL_GetError());
        return ERR;
    }
    SDL_GL_MakeCurrent(pdc_window, pdc_gl_context);

    SDL_GL_SetSwapInterval(0);
    gladLoadGL(SDL_GL_GetProcAddress);

    pdc_shader_program = glCreateProgram();
    add_shader(GL_VERTEX_SHADER, pdc_vertex_shader_src);
    add_shader(GL_FRAGMENT_SHADER, pdc_fragment_shader_src);
    glLinkProgram(pdc_shader_program);

    GLint status = GL_FALSE;
    glGetProgramiv(pdc_shader_program, GL_LINK_STATUS, &status);

    if(status != GL_TRUE)
    {
        GLsizei length = 0;
        glGetProgramiv(pdc_shader_program, GL_INFO_LOG_LENGTH, &length);
        char* err = malloc(length+1);
        glGetProgramInfoLog(pdc_shader_program, length+1, &length, err);
        fprintf(stderr, "Shader program linking: %s\n", err);
        exit(1);
    }

    glUseProgram(pdc_shader_program);

    glGenVertexArrays(1, &pdc_vao);
    glBindVertexArray(pdc_vao);

    glGenBuffers(1, &pdc_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pdc_vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_INT, 10 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, 10 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_INT, 10 * sizeof(float), (void*)(9 * sizeof(float)));

    glGenTextures(1, &pdc_font_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, pdc_font_texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    pdc_glyph_index = 0;
    pdc_glyph_capacity = 256;
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_RGBA8,
        pdc_fwidth,
        pdc_fheight,
        pdc_glyph_capacity,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL
    );

    glUniform1i(glGetUniformLocation(pdc_shader_program, "glyphs"), 0);


    /* Events must be pumped before calling SDL_GetWindowSurface, or
       initial modifiers (e.g. numlock) will be ignored and out-of-sync. */

    SDL_PumpEvents();

    /* Wait until window is exposed before getting surface */

    while (SDL_PollEvent(&event))
        if (SDL_WINDOWEVENT == event.type &&
            SDL_WINDOWEVENT_EXPOSED == event.window.event)
            break;

    if (!pdc_sheight)
        pdc_sheight = h;

    if (!pdc_swidth)
        pdc_swidth = w;

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
