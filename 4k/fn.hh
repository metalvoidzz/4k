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

	__forceinline void __fastcall UpdateRocket()
	{
		using namespace DEMO;

		double row = bass_get_row(BASS::stream);
		if (sync_update(ROCKET::rocket, (int)floor(row), &ROCKET::cb, (void *)&BASS::stream))
			Die();

		time = row * 0.01;

		/* Update values */

		// Camera position
		RENDER::cx = sync_get_val(ROCKET::tracks[TRACK_CAMX], row);
		RENDER::cy = sync_get_val(ROCKET::tracks[TRACK_CAMY], row);
		RENDER::cz = sync_get_val(ROCKET::tracks[TRACK_CAMZ], row);
		
		// Alpha
		RENDER::alpha = sync_get_val(ROCKET::tracks[TRACK_APLHA], row);
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

	// Make sure to exit process
	if (uMsg == WM_CHAR)
	{
		if (wParam == VK_ESCAPE)
			DEMO::Die();
		// Recompile shaders
		else if (wParam == VK_SPACE)
			init_gl();
		else if (wParam == VK_UP)
		{
			//RENDER::camPos[0] += 0.1;
		} else if (wParam == VK_DOWN) {

		} else if (wParam == VK_LEFT) {

		} else if (wParam == VK_RIGHT) {

		}
	}
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
		DEMO::UpdateRocket();

		BASS_Update(0);
		Sleep(10);

		SendMessage(hWnd, WM_PAINT, 0, 0);
	}
	else return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
#else


LONG WINAPI MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProcW(hWnd, uMsg, wParam, lParam); }


#endif


// Save some bytes by using a const WNDCLASS
// todo: is hInstance = NULL valid everytime?
#ifndef DEBUG_BUILD
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
#endif


// Init demo //
__forceinline void __fastcall Init()
{
	// Init Window
	{
		using namespace WINDOW;

#ifdef DEBUG_BUILD

		// Debug mode //

		WNDCLASSA wnd;

		wnd.hInstance = GetModuleHandle(NULL);
		wnd.style = CS_OWNDC;
		wnd.lpfnWndProc = MainWProc;
		wnd.lpszClassName = " ";
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hIcon = NULL;
		wnd.hCursor = NULL;
		wnd.hbrBackground = NULL;

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
#endif

		// Context
		hDC = GetDC(hWnd);

		//memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		//pfd.nVersion = 1;
		//pfd.dwFlags = PFD_SUPPORT_OPENGL;
		//pfd.iPixelType = PFD_TYPE_RGBA;
		//pfd.cColorBits = 32;

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
		ReleaseDC(hWnd, hDC);

		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);

		// Show
#ifndef DEBUG_BUILD
		//SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
#endif
		//ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
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

#ifdef DEBUG_BUILD
	if (!init_wrangler())
		DEMO::Die();
#else
	init_wrangler();
#endif

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
	size_t vLen = m_strlen(vshader_glsl);
	size_t pLen = m_strlen(pshader_glsl);

	glShaderSource(hVS, 1, &vshader_glsl, (const GLint*)&vLen);
	glShaderSource(hPX, 1, &pshader_glsl, (const GLint*)&pLen);
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

		MessageBoxA(WINDOW::hWnd, str, "Vertex shader output", MB_OK);
		free(str);

		// Pixel shader
		glGetShaderiv(hPX, GL_INFO_LOG_LENGTH, &logSize);
		str = (GLchar*)malloc(logSize + 1);
		glGetShaderInfoLog(hPX, logSize, &logSize, &str[0]);

		MessageBoxA(WINDOW::hWnd, str, "Pixel shader output", MB_OK);
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

	if (!success[0] || !success[1])
		DEMO::Die(ERR_SHADER_LNK);

	// Validate
	glValidateProgram(hPr);
#endif

	// Bind
	glUseProgram(RENDER::hPr);

	// Add Uniforms
	uLoc[UNIF_UTIME] = glGetUniformLocation(hPr, "u_time");
	uLoc[UNIF_ALPHA] = glGetUniformLocation(hPr, "u_alpha");
	uLoc[2] = glGetUniformLocation(hPr, "u_x");
	uLoc[3] = glGetUniformLocation(hPr, "u_y");
	uLoc[4] = glGetUniformLocation(hPr, "u_z");
	//uLoc[UNIF_CAM] = glGetUniformLocation(hPr, "u_cam");
}

// Render a frame //
void __fastcall render_gl()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Uniforms
	glUniform1f(RENDER::uLoc[UNIF_UTIME], DEMO::time);
	glUniform1f(RENDER::uLoc[UNIF_ALPHA], RENDER::alpha);
	//glUniform3fv(RENDER::uLoc[UNIF_CAM], 3, RENDER::camPos);
	glUniform1f(RENDER::uLoc[2], RENDER::cx);
	glUniform1f(RENDER::uLoc[3], RENDER::cy);
	glUniform1f(RENDER::uLoc[4], RENDER::cz);

	// Render fullscreen quad
	glBegin(GL_QUADS);

	glVertex2f(-1, 1);
	glVertex2f(1, 1);
	glVertex2f(1, -1);
	glVertex2f(-1, -1);

	glEnd();
	glFlush();
}