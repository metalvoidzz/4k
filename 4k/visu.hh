/* Defines related to visuals */

#pragma once

#include "def.hh"

#include <GL/gl.h>

#ifdef DEBUG_BUILD

#include <stdio.h>

#define VERTEX_FILE "vshader.glsl"
FILE* p_vshader;
char* vBuf;
#define PIXEL_FILE "pshader.glsl"
FILE* p_pshader;
char* pBuf;

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
	GLE(void, Uniform1f, GLint location, GLfloat v0) \
	GLE(void, Uniform1i, GLint location, GLint v0); \
	GLE(void, UseProgram, GLuint program) \
	GLE(void, ValidateProgram, GLuint program)
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
	GLE(void, UseProgram, GLuint program)
#endif

#define GLE(ret, name, ...) typedef ret WINAPI name##proc(__VA_ARGS__); extern name##proc * gl##name; name##proc * gl##name;
GL_LIST
#undef GLE

#define GLE(ret, name, ...) gl##name = (name##proc *)wglGetProcAddress("gl" #name);
#define InitGLExt() GL_LIST