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
#define ERR_INIT_SYNC	7
#define ERR_INIT_BASS	8

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
	// Rocket creation error
	"Unable to create rocket device",
	// Init bass error
	"Unable to init bass",
};

#endif


/* Demo defines */


#define WIDTH	800
#define HEIGHT	600


namespace DEMO
{
	void __fastcall Loop();
#ifdef DEBUG_BUILD
	void Die(int8_t cause = -1);
	__forceinline void __fastcall UpdateRocket();
#else
	__forceinline void __fastcall Die();
#endif

	float time = 0.0;
};


#ifdef DEBUG_BUILD


#define EXPORT_TRACK_NAME "track.wav"


#define NUM_TRACKS		4

#define TRACK_CAMX		0
#define TRACK_CAMY		1
#define TRACK_CAMZ		2
#define TRACK_APLHA		3
namespace ROCKET
{
	// Sync device
	sync_device* rocket;
	// Sync callback
	sync_cb cb;
	// Sync tracks
	const struct sync_track* tracks[NUM_TRACKS];
	const char* trackNames[NUM_TRACKS] = {
		"CamX",
		"CamY",
		"CamZ",
		"Alpha",
	};
};

namespace BASS
{
	// Bass stream
	HSTREAM stream;
};

#endif


/* Window defines */


namespace WINDOW
{
	HWND hWnd;
};


/* Render defines */


namespace RENDER
{
	// Uniforms

#define NUM_UNIF	5

#define UNIF_UTIME	0
#define UNIF_ALPHA	1
#define UNIF_CAM	2


#define ADD_UNIFORMS \
	uLoc[UNIF_UTIME] = glGetUniformLocation(hPr, "u_time"); \
	uLoc[UNIF_ALPHA] = glGetUniformLocation(hPr, "u_alpha"); \
	uLoc[2] = glGetUniformLocation(hPr, "u_x"); \
	uLoc[3] = glGetUniformLocation(hPr, "u_y"); \
	uLoc[4] = glGetUniformLocation(hPr, "u_z"); \
	//uLoc[UNIF_CAM] = glGetUniformLocation(hPr, "u_cam"); \

#define EVAL_UNIFORMS \
	glUniform1f(RENDER::uLoc[UNIF_UTIME], DEMO::time); \
	glUniform1f(RENDER::uLoc[UNIF_ALPHA], RENDER::alpha); \
	glUniform1i(RENDER::uLoc[2], RENDER::cx); \
	glUniform1i(RENDER::uLoc[3], RENDER::cy); \
	glUniform1i(RENDER::uLoc[4], RENDER::cz); \


	int uLoc[NUM_UNIF]; //Uniform locations

	float alpha = 0.0;
	float camPos[3];
	int cx, cy, cz;
};


/* Global namespace functions */


__forceinline void __fastcall Init();
void __fastcall render_gl();
void __fastcall init_gl();
LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
