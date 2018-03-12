/* Functions */

#pragma once

#include "def.hh"
#include "visu.hh"
#include "sync.hh"


/* Demo functions */


namespace DEMO
{
#ifdef DEBUG_BUILD
	void __fastcall Die(int8_t cause)
	{
		if (cause != -1)
			MessageBox(NULL, error_msg[cause], "Error", MB_OK);

		ExitProcess(0);
	}
#else
	__forceinline void __fastcall Die()
	{
		ExitProcess(0);
	}
#endif
};	


/* Window functions */


LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_ESCAPE)
		{
			DEMO::Die();
		}
#ifdef DEBUG_BUILD
		else if (wParam == VK_SPACE) {
			init_gl();
		}
#endif
	}
	else if (uMsg == WM_QUIT)
		DEMO::Die();

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
static const WNDCLASSA wnd = {
	CS_OWNDC,
	MainWProc,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	" ",
};

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
	PFD_TYPE_RGBA,
	32,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};


__forceinline void __fastcall Init()
{
#if defined(SYNC_PRECALC_DATA)
	PrecalcSyncData();
#endif

	{
		using namespace WINDOW;

#ifdef DEBUG_BUILD

		if (!RegisterClassA(&wnd))
			DEMO::Die(ERR_INIT_WINAPI);

		hWnd = CreateWindowExA
		(
			WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			" ",
			"",
			WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU,
			0,
			0,
			WIDTH,
			HEIGHT,
			NULL,
			NULL,
			wnd.hInstance,
			NULL
		);

		if (!hWnd)
			DEMO::Die(ERR_OPEN_WIN);

#else
		
		RegisterClassA(&wnd);

		hWnd = CreateWindowA
		(
			" ",
			"",
			WS_POPUP | WS_VISIBLE | WS_SYSMENU,
			0,
			0,
			WIDTH,
			HEIGHT,
			NULL,
			NULL,
			NULL,
			NULL
		);
#endif

#ifndef DEBUG_BUILD
		ShowCursor(0);
#endif
		
		HDC hDC = GetDC(hWnd);

		int pf_handle = ChoosePixelFormat(hDC, &pfd);

#ifdef DEBUG_BUILD
		if (!pf_handle)
			DEMO::Die(ERR_OPEN_WIN);

		if (!SetPixelFormat(hDC, pf_handle, &pfd))
			DEMO::Die(ERR_OPEN_WIN);
#else
		SetPixelFormat(hDC, pf_handle, &pfd);
#endif

		DescribePixelFormat(hDC, pf_handle, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		ReleaseDC(hWnd, hDC);

		hDC = GetDC(hWnd);
		HGLRC hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);

#ifndef DEBUG_BUILD
		//SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
#endif
	}

	init_gl();
}


/* Render functions */


#ifdef DEBUG_BUILD
void __fastcall init_gl()
#else
__forceinline void __fastcall init_gl()
#endif
{
	using namespace RENDER;

	InitGLExt();
	
	unsigned short hPX = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef DEBUG_BUILD

	p_pshader = fopen(PIXEL_FILE, "r");

	fseek(p_pshader, 0L, SEEK_END);
	long fsize = ftell(p_pshader);
	rewind(p_pshader);
	pBuf = (char*)calloc(1, fsize + 1);
	fread(pBuf, fsize, 1, p_pshader);
	fclose(p_pshader);

	size_t pLen = strlen(pBuf);

	glShaderSource(hPX, 1, &pBuf, (const GLint*)&pLen);

#else
	
	glShaderSource(hPX, 1, &pshader_glsl, NULL);

#endif

	glCompileShader(hPX);

#ifdef DEBUG_BUILD

	GLint success;

	glGetShaderiv(hPX, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLint logSize = 0;
		GLchar* str = (GLchar*)malloc(logSize + 1);

		glGetShaderiv(hPX, GL_INFO_LOG_LENGTH, &logSize);
		str = (GLchar*)malloc(logSize + 1);
		glGetShaderInfoLog(hPX, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::hWnd, str, "Pixel shader output", MB_OK);
	}

#endif

	unsigned int hPr = glCreateProgram();

	glAttachShader(hPr, hPX);
	glLinkProgram(hPr);

#ifdef DEBUG_BUILD

	glGetShaderiv(hPX, GL_LINK_STATUS, &success);
	glValidateProgram(hPr);

#endif

	glUseProgram(hPr);

	uLoc = glGetUniformLocation(hPr, "un");
}

#ifdef DEBUG_BUILD
void __fastcall render_gl()
#else
__forceinline void __fastcall render_gl()
#endif
{
	using namespace RENDER;

	EVAL_UNIFORMS;

	glUniform1fv(uLoc, NUM_UNIF, uniforms);

	glBegin(GL_QUADS);

	glVertex2i(-1, 1);
	glVertex2i(1, 1);
	glVertex2i(1, -1);
	glVertex2i(-1, -1);

	glEnd();
	glFlush();
}
