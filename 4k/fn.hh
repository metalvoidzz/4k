/* Functions */

#pragma once

#include "def.hh"
#include "visu.hh"


/* Demo functions */


namespace DEMO
{
	// Exit gracefully (i.e. die) //
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


#ifdef DEBUG_BUILD
// Window callback //
LONG WINAPI MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 	static PAINTSTRUCT ps;

	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_ESCAPE)
		{
			Quit();
		}
		// Recompile shaders
		else if (wParam == VK_SPACE)
		{
			init_gl();
		}
		// W
		else if (wParam == 0x57)
		{
			RENDER::cz += 1;
		}
		// A
		else if (wParam == 0x41)
		{
			RENDER::cy -= 1;
		}
		// S
		else if (wParam == 0x53)
		{
			RENDER::cz -= 1;
		}
		// D
		else if (wParam == 0x44)
		{
			RENDER::cy += 1;
		}
	}
	else if (uMsg == WM_PAINT)
	{
		render_gl();
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	else if (uMsg == WM_CLOSE)
	{
		Quit();
	}
	// Loop code
	else if (uMsg == WM_TIMER)
	{
		BASS_Update(0);
		Sleep(10);

		SendMessage(hWnd, WM_PAINT, 0, 0);
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
#else


LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 1;
}


#endif

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

static const PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_SUPPORT_OPENGL,
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



// Init demo //
__forceinline void __fastcall Init()
{
	// Init Window
	{
		using namespace WINDOW;

#ifdef DEBUG_BUILD

		// Debug mode //

		if (!RegisterClassA(&wnd))
			DEMO::Die(ERR_INIT_WINAPI);

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
			wnd.hInstance,
			NULL
		);

		if (!hWnd)
			DEMO::Die(ERR_OPEN_WIN);
#else
		// Release mode //
		
		RegisterClassA(&wnd);

		HWND hWnd = CreateWindowA
		(
			" ",
			"",
			WS_VISIBLE | WS_POPUP | WS_SYSMENU,
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

		// Context
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

		wglMakeCurrent(hDC, wglCreateContext(hDC));

		// Show
#ifndef DEBUG_BUILD
		SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
#endif
	}

	// Init OpenGL
	init_gl();
}


/* Render functions */


// Init gl render stuff //
#ifdef DEBUG_BUILD
void __fastcall init_gl()
#else
__forceinline void __fastcall init_gl()
#endif
{
	using namespace RENDER;

	init_wrangler();

	// Create shaders
	//unsigned short hVS = glCreateShader(GL_VERTEX_SHADER);
	unsigned short hPX = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef DEBUG_BUILD
	//p_vshader = fopen(VERTEX_FILE, "r");
	p_pshader = fopen(PIXEL_FILE, "r");
	
	//if (p_vshader == NULL || p_pshader == NULL)
	//	DEMO::Die(ERR_UNDEFINED);

	// Vertex shader
	/*fseek(p_vshader, 0L, SEEK_END);
	long fsize = ftell(p_vshader);
	rewind(p_vshader);
	vBuf = (char*)calloc(1, fsize + 1);
	fread(vBuf, fsize, 1, p_vshader);
	fclose(p_vshader);*/

	// Pixel shader
	fseek(p_pshader, 0L, SEEK_END);
	long fsize = ftell(p_pshader);
	rewind(p_pshader);
	pBuf = (char*)calloc(1, fsize + 1);
	fread(pBuf, fsize, 1, p_pshader);
	fclose(p_pshader);

	//size_t vLen = strlen(vBuf);
	size_t pLen = strlen(pBuf);

	//glShaderSource(hVS, 1, &vBuf, (const GLint*)&vLen);
	glShaderSource(hPX, 1, &pBuf, (const GLint*)&pLen);
#else
	
	//glShaderSource(hVS, 1, &vshader_glsl, NULL);
	glShaderSource(hPX, 1, &pshader_glsl, NULL);

#endif

	// Compile shaders
	//glCompileShader(hVS);
	glCompileShader(hPX);

#ifdef DEBUG_BUILD
	GLint success;

	// Check if successful
	glGetShaderiv(hPX, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		// Get gl output
		GLint logSize = 0;
		GLchar* str = (GLchar*)malloc(logSize + 1);

		// Pixel shader
		glGetShaderiv(hPX, GL_INFO_LOG_LENGTH, &logSize);
		str = (GLchar*)malloc(logSize + 1);
		glGetShaderInfoLog(hPX, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::hWnd, str, "Pixel shader output", MB_OK);
	}

#endif
	// Link shaders
	unsigned short hPr = glCreateProgram();

	glAttachShader(hPr, hPX);

	// Attributes
	glBindAttribLocation(hPr, 0, "position");

	glLinkProgram(hPr);

#ifdef DEBUG_BUILD
	// Check if linking was successful
	//glGetShaderiv(hVS, GL_LINK_STATUS, &success[0]);
	glGetShaderiv(hPX, GL_LINK_STATUS, &success);

	//if (!success[0] || !success[1])
	//	DEMO::Die(ERR_SHADER_LNK);

	// Validate
	glValidateProgram(hPr);
#endif

	// Bind
	glUseProgram(hPr);

	// Set aspect
	// glViewport(0, 0, WIDTH, HEIGHT);

	ADD_UNIFORMS
}

// Render a frame //
#ifdef DEBUG_BUILD
void __fastcall render_gl()
#else
__forceinline void __fastcall render_gl()
#endif
{
	EVAL_UNIFORMS

	// Render fullscreen quad
	glBegin(GL_QUADS);
	
	glVertex2i(-1, 1);
	glVertex2i(1, 1);
	glVertex2i(1, -1);
	glVertex2i(-1, -1);

	glEnd();
	glFlush();
}
