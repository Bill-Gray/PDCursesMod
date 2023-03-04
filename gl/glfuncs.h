/* This header declares the internally-used OpenGL stuff. OpenGL is a bit
 * annoying in that its functions have to be loaded during runtime; we use
 * SDL_GL_GetProcAddress for that. GLEW or Glad could be used for this as well,
 * but it's hard to justify adding such large dependencies for this kind of
 * minimal OpenGL usage.
 */
#include "pdcgl.h"
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

/* We can't just declare our own function pointers with the real function
 * names, since those prototypes already come from SDL_opengl.h :/ But what we
 * can do, is use defines to make the OpenGL functions look like they should :)
 */
#define glCreateProgram pdc_glCreateProgram
#define glGenVertexArrays pdc_glGenVertexArrays
#define glBindVertexArray pdc_glBindVertexArray
#define glGenBuffers pdc_glGenBuffers
#define glBindBuffer pdc_glBindBuffer
#define glEnableVertexAttribArray pdc_glEnableVertexAttribArray
#define glVertexAttribIPointer pdc_glVertexAttribIPointer
#define glVertexAttribDivisor pdc_glVertexAttribDivisor
#define glGetUniformLocation pdc_glGetUniformLocation
#define glUniform1i pdc_glUniform1i
#define glUniform2i pdc_glUniform2i
#define glUniform3i pdc_glUniform3i
#define glUniform4i pdc_glUniform4i
#define glUniform1f pdc_glUniform1f
#define glUniform2f pdc_glUniform2f
#define glUniform3f pdc_glUniform3f
#define glUniform4f pdc_glUniform4f
#define glGenFramebuffers pdc_glGenFramebuffers
#define glBindFramebuffer pdc_glBindFramebuffer
#define glDeleteFramebuffers pdc_glDeleteFramebuffers
#define glDeleteVertexArrays pdc_glDeleteVertexArrays
#define glDeleteBuffers pdc_glDeleteBuffers
#define glDeleteProgram pdc_glDeleteProgram
#define glCreateShader pdc_glCreateShader
#define glShaderSource pdc_glShaderSource
#define glCompileShader pdc_glCompileShader
#define glGetShaderiv pdc_glGetShaderiv
#define glGetShaderInfoLog pdc_glGetShaderInfoLog
#define glAttachShader pdc_glAttachShader
#define glDeleteShader pdc_glDeleteShader
#define glLinkProgram pdc_glLinkProgram
#define glGetProgramiv pdc_glGetProgramiv
#define glGetProgramInfoLog pdc_glGetProgramInfoLog
#define glFramebufferTexture pdc_glFramebufferTexture
#define glClearBufferfv pdc_glClearBufferfv
#define glUseProgram pdc_glUseProgram
#define glBufferData pdc_glBufferData
#define glDrawArraysInstanced pdc_glDrawArraysInstanced
#define glBlitFramebuffer pdc_glBlitFramebuffer
#define glClear pdc_glClear
#define glClearColor pdc_glClearColor
#define glViewport pdc_glViewport
#define glDeleteTextures pdc_glDeleteTextures
#define glEnable pdc_glEnable
#define glDisable pdc_glDisable
#define glDrawBuffer pdc_glDrawBuffer
#define glReadBuffer pdc_glReadBuffer
#define glBlendFunc pdc_glBlendFunc
#define glGetIntegerv pdc_glGetIntegerv
#define glGenTextures pdc_glGenTextures
#define glBindTexture pdc_glBindTexture
#define glTexParameteri pdc_glTexParameteri
#define glTexImage2D pdc_glTexImage2D
#define glTexSubImage2D pdc_glTexSubImage2D
#define glCopyTexSubImage2D pdc_glCopyTexSubImage2D
#define glPixelStorei pdc_glPixelStorei

#define GLFUNCS \
    GLFUNC(CREATEPROGRAM, CreateProgram) \
    GLFUNC(GENVERTEXARRAYS, GenVertexArrays) \
    GLFUNC(BINDVERTEXARRAY, BindVertexArray) \
    GLFUNC(GENBUFFERS, GenBuffers) \
    GLFUNC(BINDBUFFER, BindBuffer) \
    GLFUNC(ENABLEVERTEXATTRIBARRAY, EnableVertexAttribArray) \
    GLFUNC(VERTEXATTRIBIPOINTER, VertexAttribIPointer) \
    GLFUNC(VERTEXATTRIBDIVISOR, VertexAttribDivisor) \
    GLFUNC(GETUNIFORMLOCATION, GetUniformLocation) \
    GLFUNC(UNIFORM1I, Uniform1i) \
    GLFUNC(UNIFORM2I, Uniform2i) \
    GLFUNC(UNIFORM3I, Uniform3i) \
    GLFUNC(UNIFORM4I, Uniform4i) \
    GLFUNC(UNIFORM1F, Uniform1f) \
    GLFUNC(UNIFORM2F, Uniform2f) \
    GLFUNC(UNIFORM3F, Uniform3f) \
    GLFUNC(UNIFORM4F, Uniform4f) \
    GLFUNC(GENFRAMEBUFFERS, GenFramebuffers) \
    GLFUNC(BINDFRAMEBUFFER, BindFramebuffer) \
    GLFUNC(DELETEFRAMEBUFFERS, DeleteFramebuffers) \
    GLFUNC(DELETEVERTEXARRAYS, DeleteVertexArrays) \
    GLFUNC(DELETEBUFFERS, DeleteBuffers) \
    GLFUNC(DELETEPROGRAM, DeleteProgram) \
    GLFUNC(CREATESHADER, CreateShader) \
    GLFUNC(SHADERSOURCE, ShaderSource) \
    GLFUNC(COMPILESHADER, CompileShader) \
    GLFUNC(GETSHADERIV, GetShaderiv) \
    GLFUNC(GETSHADERINFOLOG, GetShaderInfoLog) \
    GLFUNC(ATTACHSHADER, AttachShader) \
    GLFUNC(DELETESHADER, DeleteShader) \
    GLFUNC(LINKPROGRAM, LinkProgram) \
    GLFUNC(GETPROGRAMIV, GetProgramiv) \
    GLFUNC(GETPROGRAMINFOLOG, GetProgramInfoLog) \
    GLFUNC(FRAMEBUFFERTEXTURE, FramebufferTexture) \
    GLFUNC(CLEARBUFFERFV, ClearBufferfv) \
    GLFUNC(USEPROGRAM, UseProgram) \
    GLFUNC(BUFFERDATA, BufferData) \
    GLFUNC(DRAWARRAYSINSTANCED, DrawArraysInstanced) \
    GLFUNC(BLITFRAMEBUFFER, BlitFramebuffer) \
    GLFUNCPROTO(CLEAR, Clear, (GLbitfield)) \
    GLFUNCPROTO(CLEARCOLOR, ClearColor, (GLclampf, GLclampf, GLclampf, GLclampf)) \
    GLFUNCPROTO(VIEWPORT, Viewport, (GLint, GLint, GLsizei, GLsizei)) \
    GLFUNCPROTO(DELETETEXTURES, DeleteTextures, (GLsizei, const GLuint*)) \
    GLFUNCPROTO(ENABLE, Enable, (GLenum)) \
    GLFUNCPROTO(DISABLE, Disable, (GLenum)) \
    GLFUNCPROTO(DRAWBUFFER, DrawBuffer, (GLenum)) \
    GLFUNCPROTO(READBUFFER, ReadBuffer, (GLenum)) \
    GLFUNCPROTO(BLENDFUNC, BlendFunc, (GLenum, GLenum)) \
    GLFUNCPROTO(GETINTEGERV, GetIntegerv, (GLenum, GLint *)) \
    GLFUNCPROTO(GENTEXTURES, GenTextures, (GLsizei, GLuint *)) \
    GLFUNCPROTO(BINDTEXTURE, BindTexture, (GLenum, GLuint)) \
    GLFUNCPROTO(TEXPARAMETERI, TexParameteri, (GLenum, GLenum, GLint)) \
    GLFUNCPROTO(TEXIMAGE2D, TexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *)) \
    GLFUNCPROTO(TEXSUBIMAGE2D, TexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *)) \
    GLFUNCPROTO(COPYTEXSUBIMAGE2D, CopyTexSubImage2D, (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)) \
    GLFUNCPROTO(PIXELSTOREI, PixelStorei, (GLenum, GLint)) \

#ifdef GLFUNC_IMPL

#define GLFUNC(sig, name) PFNGL##sig##PROC pdc_gl##name = NULL;
#define GLFUNCPROTO(sig, name, params) \
    typedef void (*PFNGL##sig##PROC) params; GLFUNC(sig, name)
GLFUNCS
#undef GLFUNCPROTO
#undef GLFUNC

static void load_gl_funcs()
{
    void (*(*loader)(const char*))(void) = (void (*(*)(const char*))(void))SDL_GL_GetProcAddress;
#define GLFUNC(sig, name) pdc_gl##name = (PFNGL##sig##PROC)loader("gl" #name);
#define GLFUNCPROTO(sig, name, params) GLFUNC(sig, name)
    GLFUNCS
#undef GLFUNCPROTO
#undef GLFUNC
}

#else

#define GLFUNC(sig, name) extern PFNGL##sig##PROC pdc_gl##name;
#define GLFUNCPROTO(sig, name, params) \
    typedef void (*PFNGL##sig##PROC) params; GLFUNC(sig, name)

GLFUNCS
#undef GLFUNCPROTO
#undef GLFUNC

#endif
