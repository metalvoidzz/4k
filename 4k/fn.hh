/* Functions */

#pragma once

#include "def.hh"
#include "visu.hh"


/* Demo functions */


namespace DEMO
{
	// Exit gracefully (i.e. die) //
#ifdef DEBUG_BUILD
	void Die(int8_t cause)
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


// Window callback //
LONG WINAPI MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT ps;

	// Make sure to exit process
#ifdef DEBUG_BUILD
	if (uMsg == WM_CHAR)
	{
		if (wParam == VK_ESCAPE)
			DEMO::Die();
		// Recompile shaders
		else if (wParam == VK_SPACE)
		{
			init_gl();
			//render_gl();
		}
	}
#else
	if (uMsg == WM_CHAR && wParam == VK_ESCAPE)
		DEMO::Die();
#endif
	else if (uMsg == WM_PAINT)
	{
		render_gl();
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	else if (uMsg == WM_DESTROY)
		DEMO::Die();
	// Loop code
	else if (uMsg == WM_TIMER)
	{
#ifdef DEBUG_BUILD
		double row = bass_get_row(BASS::stream);
		if (sync_update(ROCKET::rocket, (int)floor(row), &ROCKET::cb, (void *)&BASS::stream))
			DEMO::Die();
		//row to time
		DEMO::time = row * 0.01;
		BASS_Update(0);
		Sleep(10);
#else
		float pos = Clinkster_GetPosition();
		if (pos > Clinkster_MusicLength) DEMO::Die();
		DEMO::time += 0.01;
		Sleep(10);
#endif

		SendMessage(hWnd, WM_PAINT, 0, 0);
	}
	else return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// Init demo //
__forceinline void Init()
{
	// Init Window
	{
		using namespace WINDOW;

		WNDCLASSW wnd;

		wnd.hInstance = GetModuleHandle(NULL);
		wnd.style = CS_OWNDC;
		wnd.lpfnWndProc = MainWProc;
		wnd.lpszClassName = L"H";
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hIcon = NULL;
		wnd.hCursor = NULL;
		wnd.hbrBackground = NULL;
		wnd.lpszMenuName = NULL;

#ifdef DEBUG_BUILD
		if (!RegisterClassW(&wnd))
			DEMO::Die(ERR_INIT_WINAPI);
#else
		RegisterClassW(&wnd);
#endif

		// Open
/*		HMONITOR hmon = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi = { sizeof(mi) };
#ifdef DEBUG_BUILD
		if (!GetMonitorInfo(hmon, &mi)) DEMO::Die(ERR_OPEN_WIN);
#else
		GetMonitorInfo(hmon, &mi);
#endif*/

		win_handle = CreateWindowW
		(
			L"H",
			L"",
			WS_POPUP | WS_VISIBLE | WS_SYSMENU,
			0,
			0,
			WIDTH,
			HEIGHT,
			(HWND)NULL,
			(HMENU)NULL,
			wnd.hInstance,
			NULL
		);

#ifdef DEBUG_BUILD
		if (!win_handle)
			DEMO::Die(ERR_OPEN_WIN);
#endif

		// Context
		hDC = GetDC(win_handle);

		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;

		pf_handle = ChoosePixelFormat(hDC, &pfd);

#ifdef DEBUG_BUILD
		if (!pf_handle)
			DEMO::Die(ERR_OPEN_WIN);

		if (!SetPixelFormat(hDC, pf_handle, &pfd))
			DEMO::Die(ERR_OPEN_WIN);
#else
		SetPixelFormat(hDC, pf_handle, &pfd);
#endif

		DescribePixelFormat(hDC, pf_handle, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		ReleaseDC(win_handle, hDC);

		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);

		// Show
#ifndef DEBUG_BUILD
		SetWindowPos(win_handle, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
#endif
		// No borders
		//LONG lStyle = GetWindowLong(win_handle, GWL_STYLE);
		//lStyle &= ~(WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE);
		//SetWindowLong(win_handle, GWL_STYLE, lStyle);
		ShowWindow(win_handle, SW_SHOW);
#ifdef DEBUG_BUILD
		SetForegroundWindow(win_handle);
		SetFocus(win_handle);
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

	if (!init_wrangler())
		DEMO::Die();

	// Set aspect
	glViewport(0, 0, WIDTH, HEIGHT);

	// Create shaders
	hVS = glCreateShader(GL_VERTEX_SHADER);
	hPX = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef DEBUG_BUILD
	p_vshader = fopen(VERTEX_FILE, "r");
	p_pshader = fopen(PIXEL_FILE, "r");
	
	if (p_vshader == NULL || p_pshader == NULL)
		DEMO::Die(ERR_UNDEFINED);

	// Vertex shader
	fseek(p_vshader, 0L, SEEK_END);
	long fsize = ftell(p_vshader);
	rewind(p_vshader);
	vBuf = (char*)calloc(1, fsize + 1);
	fread(vBuf, fsize, 1, p_vshader);
	fclose(p_vshader);

	// Pixel shader
	fseek(p_pshader, 0L, SEEK_END);
	fsize = ftell(p_pshader);
	rewind(p_pshader);
	pBuf = (char*)calloc(1, fsize + 1);
	fread(pBuf, fsize, 1, p_pshader);
	fclose(p_pshader);

	size_t vLen = strlen(vBuf);
	size_t pLen = strlen(pBuf);

	glShaderSource(hVS, 1, &vBuf, (const GLint*)&vLen);
	glShaderSource(hPX, 1, &pBuf, (const GLint*)&pLen);
#else
	const char* vSrc = shaders[SHADER_VERTEX];
	const char* pSrc = shaders[SHADER_PIXEL];

	size_t vLen = sizeof(*vSrc);
	size_t pLen = sizeof(*pSrc);

	glShaderSource(hVS, 1, &vSrc, (const GLint*)&vLen);
	glShaderSource(hPX, 1, &pSrc, (const GLint*)&pLen);
#endif

	// Compile shaders
	glCompileShader(hVS);
	glCompileShader(hPX);

#ifdef DEBUG_BUILD
	GLint success[2];

	// Check if successful
	glGetShaderiv(hVS, GL_COMPILE_STATUS, &success[0]);
	glGetShaderiv(hPX, GL_COMPILE_STATUS, &success[1]);

	if (!success[0] || !success[1])
	{
		// Get gl output
		GLint logSize = 0;
		GLchar* str = (GLchar*)malloc(logSize + 1);

		// Vertex shader
		glGetShaderiv(hVS, GL_INFO_LOG_LENGTH, &logSize);
		glGetShaderInfoLog(hVS, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::win_handle, str, "Vertex shader output", MB_OK);
		free(str);

		// Pixel shader
		glGetShaderiv(hPX, GL_INFO_LOG_LENGTH, &logSize);
		str = (GLchar*)malloc(logSize + 1);
		glGetShaderInfoLog(hPX, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::win_handle, str, "Pixel shader output", MB_OK);

		//DEMO::Die();
	}

#endif

	// Link shaders
	hPr = glCreateProgram();

	glAttachShader(hPr, hVS);
	glAttachShader(hPr, hPX);

	// Attributes
	glBindAttribLocation(hPr, 0, "position");

	glLinkProgram(hPr);

#ifdef DEBUG_BUILD
	// Check if linking was successful
	glGetShaderiv(hVS, GL_LINK_STATUS, &success[0]);
	glGetShaderiv(hPX, GL_LINK_STATUS, &success[1]);

	//if (!success[0] || !success[1])
	//	DEMO::Die(ERR_SHADER_LNK);

#endif

	// Validate
	glValidateProgram(hPr);

	// Add Uniforms
	uLoc[0] = glGetUniformLocation(hPr, "u_time");
	uLoc[1] = glGetUniformLocation(hPr, "alpha");
}

// Render a frame //
void __fastcall render_gl()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind shader
	glUseProgram(RENDER::hPr);
	glUniform1f(RENDER::uLoc[0], DEMO::time);

	// Render fullscreen quad
	glBegin(GL_TRIANGLES);

	glVertex2i(-1, -1);
	glVertex2i(1, -1);
	glVertex2i(-1, 1);

	glVertex2i(1, 1);
	glVertex2i(-1, 1);
	glVertex2i(1, -1);

	glEnd();
	glFlush();
}