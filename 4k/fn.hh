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

	// Main loop //
	void __fastcall Loop()
	{
		// Handle window events
		MSG message;
		GetMessageW(&message, NULL, 0, 0);
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
};


/* Window functions */


// Window callback //
LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Make sure to exit process
	if (uMsg == WM_CHAR && wParam == VK_ESCAPE)
		DEMO::Die();
	else if (uMsg == WM_PAINT)
		render_gl();
	else if (uMsg == WM_DESTROY)
		DEMO::Die();
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
		if (!RegisterClassW(&wnd)) DEMO::Die(ERR_INIT_WINAPI);
#else
		RegisterClassW(&wnd);
#endif

		// Open
		HMONITOR hmon = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi = { sizeof(mi) };
#ifdef DEBUG_BUILD
		if (!GetMonitorInfo(hmon, &mi)) DEMO::Die(ERR_OPEN_WIN);
#else
		GetMonitorInfo(hmon, &mi);
#endif

		win_handle = CreateWindowW
		(
			L"H",
			L"",
			WS_POPUP | WS_VISIBLE | WS_SYSMENU,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
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
		SetWindowPos(win_handle, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
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

	// Init sound
	Clinkster_GenerateMusic();
}


/* Render functions */


// Init gl render stuff //
__forceinline void __fastcall init_gl()
{
	using namespace RENDER;

	if (!init_wrangler()) DEMO::Die();

	// Set aspect
	glViewport(0, 0, WIDTH, HEIGHT);

	// Create shaders
	hVS = glCreateShader(GL_VERTEX_SHADER);
	hPX = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vSrc = shaders[SHADER_VERTEX];
	const char* pSrc = shaders[SHADER_PIXEL];

	size_t vLen = sizeof(*vSrc);
	size_t pLen = sizeof(*pSrc);

	glShaderSource(hVS, 1, &vSrc, (const GLint*)&vLen);
	glShaderSource(hPX, 1, &pSrc, (const GLint*)&pLen);

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
		GLchar str[1024];

		// Vertex shader
		glGetShaderiv(hVS, GL_INFO_LOG_LENGTH, &logSize);
		glGetShaderInfoLog(hVS, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::win_handle, str, "Vertex shader output", MB_OK);
		memset(str, NULL, sizeof(str));

		// Pixel shader
		glGetShaderiv(hPX, GL_INFO_LOG_LENGTH, &logSize);
		glGetShaderInfoLog(hPX, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::win_handle, str, "Pixel shader output", MB_OK);

		DEMO::Die();
	}

#endif

	// Link shaders
	hPr = glCreateProgram();

	glAttachShader(hPr, hVS);
	glAttachShader(hPr, hPX);

	glLinkProgram(hPr);

#ifdef DEBUG_BUILD
	// Check if linking was successful
	glGetShaderiv(hVS, GL_LINK_STATUS, &success[0]);
	glGetShaderiv(hPX, GL_LINK_STATUS, &success[1]);

	if (!success[0] || !success[1])
		DEMO::Die(ERR_SHADER_LNK);

#endif

	// Uniforms
	DEMO::uLoc[0] = glGetUniformLocation(hPr, "u_time");

	glUniform1f(hPr, DEMO::uLoc[0]);
}

// Render a frame //
void __fastcall render_gl()
{
	glClear(GL_COLOR_BUFFER_BIT);
	// Render fullscreen quad

	// Use program
	glUseProgram(RENDER::hPr);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
	glPopMatrix();
}