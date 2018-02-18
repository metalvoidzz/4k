/* Defines related to visuals */

#pragma once

#include "def.hh"

#include <GL/gl.h>
#include <math.h>


// Indices //
#define SHADER_VERTEX	0
#define SHADER_PIXEL	1



/* If debug, load shaders from hd */
#ifdef DEBUG_BUILD

#include <stdio.h>

#define VERTEX_FILE		"vshader.glsl"
FILE* p_vshader;
char* vBuf;
#define PIXEL_FILE		"pshader.glsl"
FILE* p_pshader;
char* pBuf;

/* If release, use static shader array */
#else

// Shader array //
const char* shaders[] =
{
	// Vertex shader //



	"#version 120 \n" \

	"attribute vec2 vertexIn;" \
	"varying vec2 textureCoord;" \

	// Main
	"void main() {" \
	"	gl_Position = vec4(vertexIn.xy,0.0,1.0);" \
	"}",



	// Pixel shader //



	"#version 120 \n" \
	// Uniforms

	// Main
	"void main() {" \
	"	gl_FragColor = vec4(0.1,0.0,0.0,1.0);" \
	"}",
};

#endif




/* Minimalist extension wrangler
* See https://github.com/ApoorvaJ/Papaya
*/

#define GLDECL WINAPI
#define NOMINMAX

typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS					  0x8B82
#define GL_INFO_LOG_LENGTH				  0x8B84
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FUNC_ADD                       0x8006
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_STATIC_DRAW                    0x88E4
#define GL_STREAM_DRAW                    0x88E0
#define GL_TEXTURE0                       0x84C0
#define GL_VERTEX_SHADER				  0x8B31

#define GLE(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); extern name##proc * gl##name;

#define GL_LIST \
	GLE(void, AttachShader, GLuint program, GLuint shader) \
	GLE(void, BindBuffer, GLenum target, GLuint buffer) \
	GLE(void, BindFramebuffer, GLenum target, GLuint framebuffer) \
	GLE(void, BufferData, GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) \
	GLE(void, BufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) \
	GLE(GLenum, CheckFramebufferStatus, GLenum target) \
	GLE(void, ClearBufferfv, GLenum buffer, GLint drawbuffer, const GLfloat * value) \
	GLE(void, CompileShader, GLuint shader) \
	GLE(GLuint, CreateProgram, void) \
	GLE(GLuint, CreateShader, GLenum type) \
	GLE(void, DeleteBuffers, GLsizei n, const GLuint *buffers) \
	GLE(void, DeleteFramebuffers, GLsizei n, const GLuint *framebuffers) \
	GLE(void, EnableVertexAttribArray, GLuint index) \
	GLE(void, DrawBuffers, GLsizei n, const GLenum *bufs) \
	GLE(void, FramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
	GLE(void, GenBuffers, GLsizei n, GLuint *buffers) \
	GLE(void, GenFramebuffers, GLsizei n, GLuint * framebuffers) \
	GLE(GLint, GetAttribLocation, GLuint program, const GLchar *name) \
	GLE(void, BindAttribLocation, GLuint program, GLuint index, const GLchar* name) \
	GLE(void, ValidateProgram, GLuint program) \
	GLE(void, GetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
	GLE(void, GetShaderiv, GLuint shader, GLenum pname, GLint *params) \
	GLE(GLint, GetUniformLocation, GLuint program, const GLchar *name) \
	GLE(void, LinkProgram, GLuint program) \
	GLE(void, ShaderSource, GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
	GLE(void, Uniform1i, GLint location, GLint v0) \
	GLE(void, Uniform1f, GLint location, GLfloat v0) \
	GLE(void, Uniform2f, GLint location, GLfloat v0, GLfloat v1) \
	GLE(void, Uniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
	GLE(void, UniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	GLE(void, UseProgram, GLuint program) \
	GLE(void, VertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)

#define GLE(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); extern name##proc * gl##name;
GL_LIST
#undef GLE

#define GLE(ret, name, ...) name##proc * gl##name;
GL_LIST
#undef GLE

__forceinline bool init_wrangler()
{
	typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
	HINSTANCE dll = LoadLibraryA("opengl32.dll");
	if (!dll)
		return false;

	wglGetProcAddressproc* wglGetProcAddress = (wglGetProcAddressproc*)GetProcAddress(dll, "wglGetProcAddress");

#define GLE(ret, name, ...)																			\
            gl##name = (name##proc *)wglGetProcAddress("gl" #name);									\
            if (!gl##name) {																		\
                return false;																		\
            }
	GL_LIST
#undef GLE
		return true;
}