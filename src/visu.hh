/* Defines related to visuals */

#pragma once

#include "def.hh"

#include <GL/gl.h>

#ifdef DEBUG_BUILD

#define PIXEL_FILE "pshader.glsl"

#else

#include "auto_pshader.h"

#endif


#define GL_ARRAY_BUFFER 0x8892
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FUNC_ADD 0x8006
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_STATIC_DRAW 0x88E4
#define GL_STREAM_DRAW 0x88E0
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_VERTEX_SHADER 0x8B31

typedef char GLchar;


#ifdef DEBUG_BUILD

#define GL_LIST \
	GLE(void, AttachShader, GLuint program, GLuint shader) \
	GLE(void, CompileShader, GLuint shader) \
	GLE(GLuint, CreateProgram, void) \
	GLE(GLuint, CreateShader, GLenum type) \
	GLE(void, BindAttribLocation, GLuint program, GLuint index, const GLchar* name) \
	GLE(void, GetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
	GLE(void, GetShaderiv, GLuint shader, GLenum pname, GLint *params) \
	GLE(GLint, GetUniformLocation, GLuint program, const GLchar *name) \
	GLE(void, LinkProgram, GLuint program) \
	GLE(void, ShaderSource, GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
	GLE(void, Uniform1fv, GLint location, GLsizei count, const GLfloat* value); \
	GLE(void, Uniform1i, GLint location, GLint value); \
	GLE(void, UseProgram, GLuint program) \
	GLE(void, ValidateProgram, GLuint program) \
	GLE(void, ActiveTexture, GLenum texture) \

#else

#define GL_LIST \
	GLE(void, AttachShader, GLuint program, GLuint shader) \
	GLE(void, CompileShader, GLuint shader) \
	GLE(GLuint, CreateProgram, void) \
	GLE(GLuint, CreateShader, GLenum type) \
	GLE(GLint, GetUniformLocation, GLuint program, const GLchar *name) \
	GLE(void, LinkProgram, GLuint program) \
	GLE(void, ShaderSource, GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
	GLE(void, Uniform1fv, GLint location, GLsizei count, const GLfloat* value); \
	GLE(void, UseProgram, GLuint program) \

#endif

#define GLE(ret, name, ...) typedef ret WINAPI name##proc(__VA_ARGS__); extern name##proc * gl##name; name##proc * gl##name;
GL_LIST
#undef GLE

#define GLE(ret, name, ...) gl##name = (name##proc *)wglGetProcAddress("gl" #name);
#define InitGLExt() GL_LIST


/*// Generate textures and push to shader //
void init_tex()
{
	using namespace RENDER;


	// Generate textures //

	
	// Generate texture names //


	GLuint pTexNames[NUM_TEX];
	glGenTextures(NUM_TEX, pTexNames);


	// Bind textures //


	for (uint16_t i = 0; i < NUM_TEX; i++)
	{
		glBindTexture(GL_TEXTURE_2D, pTexNames[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex[i].w, tex[i].h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex[i].data);
		glBindTexture(GL_TEXTURE_2D, pTexNames[i]);
	}


	// Push to uniforms //


	// t0
	uTex[0] = glGetUniformLocation(hPr, "t0");
	glUniform1i(uTex[0], 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, pTexNames[0]);
}*/




