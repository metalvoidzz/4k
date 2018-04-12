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

		exit(0);
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
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_ESCAPE)
			DEMO::Die();
#ifdef DEBUG_BUILD
		else if (wParam == VK_SPACE) {
			init_gl();
		}
#endif
	}
	else if (uMsg == WM_CLOSE)
		DEMO::Die();
#ifdef DEBUG_BUILD
	else if (uMsg == WM_TIMER)
	{
		// Check for updated shaders
		FILETIME f;
		GetFileTime(DEMO::hShader, NULL, NULL, &f);

		SYSTEMTIME t_old;
		SYSTEMTIME t_new;

		FileTimeToSystemTime(&DEMO::ftime, &t_old);
		FileTimeToSystemTime(&f, &t_new);

		if (t_old.wSecond != t_new.wSecond) {
			init_gl();
			DEMO::ftime = f;
			printf("Reloading\n");
		}
		else {
			DEMO::ftime = f;
		}
	}
#endif

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
	"c",
};

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	24,
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,  
	0, 0, 0, 0,
	32,
	0,
	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
};

static DEVMODE screenSettings = { { 0 },
#if _MSC_VER < 1400
	0,0,148,0,0x001c0000,{ 0 },0,0,0,0,0,0,0,0,0,{ 0 },0,32,XRES,YRES,0,0,
#else
	0,0,156,0,0x001c0000,{ 0 },0,0,0,0,0,{ 0 },0,32, WIDTH, HEIGHT,{ 0 }, 0,
#endif
#if(WINVER >= 0x0400)
	0,0,0,0,0,0,
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
	0,0
#endif
#endif
};

void __fastcall Init()
{
	{
		using namespace WINDOW;

		RegisterClassA(&wnd);

		DWORD dwStyle = WS_VISIBLE | WS_POPUP;

		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);

		hWnd = CreateWindowExA
		(
			WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			"c",
			" ",
			dwStyle,
			0,
			0,
			WIDTH,
			HEIGHT,
			NULL,
			NULL,
			wnd.hInstance,
			NULL
		);

#ifdef DEBUG_BUILD
		if (!hWnd) DEMO::Die(ERR_INIT_WINAPI);
#else
		ShowCursor(0);
#endif

		WINDOW::hDC = GetDC(hWnd);

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

		//SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
	}

	init_gl();

#if defined(SYNC_PRECALC_DATA)
	PrecalcSyncData();
#endif
}


/* Render functions */


#ifdef DEBUG_BUILD

#include <fstream>
#include <string>
#include <streambuf>

#endif
void __fastcall init_gl()
{
	using namespace RENDER;

	InitGLExt();

	unsigned short hPX = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef DEBUG_BUILD

	std::ifstream str(PIXEL_FILE, std::ios::in);
	std::string file((std::istreambuf_iterator<char>(str)), std::istreambuf_iterator<char>());
	str.close();

	size_t pLen = file.size();
	char* pBuf = (char*)file.c_str();

	glShaderSource(hPX, 1, &pBuf, (const GLint*)&pLen);

#else

	size_t pLen = strlen(pshader_glsl);

	glShaderSource(hPX, 1, &pshader_glsl, (const GLint*)&pLen);

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

		printf("%s\n", str);
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

void __fastcall render_gl()
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

	wglSwapLayerBuffers(WINDOW::hDC, WGL_SWAP_MAIN_PLANE);
}
