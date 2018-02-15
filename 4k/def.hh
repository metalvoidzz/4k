/* Defines */

#pragma once

#include <windows.h>
#include <inttypes.h>


/* Error messages */


#ifdef DEBUG_BUILD

// Indices //
#define ERR_UNDEFINED	0
#define ERR_OPEN_WIN	1
#define ERR_INIT_WINAPI	2
#define ERR_INIT_SOUND	3
#define ERR_INIT_WR		4
#define ERR_SHADER_CMP	5
#define ERR_SHADER_LNK	6

static const char* error_msg[] =
{
	// Undefined error
	"An undefined error occured",
	// Open window error
	"Unable to open window",
	// Init WinApi error
	"Unable to initialize WinApi",
	// Init sound playback error
	"Unable to initialize sound playback",
	// Init wrangler error
	"Unable to init opengl extension wrangler",
	// Shader compile error
	"Unable to compile shaders",
	// Shader link error
	"Unable to link shaders",
};

#endif


/* Demo defines */


#define WIDTH	1920
#define HEIGHT	1080


// Uniform bindings
#define UNIF_FLOAT_TIME 0

namespace DEMO
{
	// Uniforms
	unsigned short uLoc[1]; //Uniform locations

	void __fastcall Loop();
#ifdef DEBUG_BUILD
	void Die(int8_t cause = -1);
#else
	__forceinline void __fastcall Die();
#endif
};


/* Window defines */


namespace WINDOW
{
	HWND win_handle;
	HGLRC hRC;
	PIXELFORMATDESCRIPTOR pfd;
	int pf_handle;
	HDC hDC;
};


/* Render defines */


namespace RENDER
{
	unsigned short hVS; //vertex shader handle
	unsigned short hPX; //pixel shader handle
	unsigned short hPr; //shader program handle
};


/* Global namespace functions */


LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
__forceinline void Init();
__forceinline void __fastcall init_gl();
void __fastcall render_gl();