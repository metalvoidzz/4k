/* Defines */


#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <inttypes.h>

#ifdef DEBUG_BUILD
#include <cmath>
#endif


#ifdef DEBUG_BUILD

#define ERR_UNDEFINED	0
#define ERR_OPEN_WIN	1
#define ERR_INIT_WINAPI	2
#define ERR_INIT_SOUND	3
#define ERR_INIT_WR		4
#define ERR_SHADER_CMP	5
#define ERR_SHADER_LNK	6
#define ERR_INIT_SYNC	7
#define ERR_INIT_BASS	8

static const char* error_msg[] = {
	"An undefined error occured",
	"Unable to open window",
	"Unable to initialize WinApi",
	"Unable to initialize sound playback",
	"Unable to init opengl extension wrangler",
	"Unable to compile shaders",
	"Unable to link shaders",
	"Unable to create rocket device",
	"Unable to init bass",
};

#endif


#define WIDTH	1280
#define HEIGHT	720


namespace DEMO
{
#ifdef DEBUG_BUILD
	void __fastcall Die(int8_t cause = -1);
	HANDLE hShader;
	FILETIME ftime;
#else
	__forceinline void __fastcall Die();
#endif

	float time = 0.0;
	unsigned int row = 0;
	bool done = false;
};


#ifdef DEBUG_BUILD


#include "librocket/sync.h"
#include "bass/c/bass.h"


#define EXPORT_TRACK_NAME "track.wav"


namespace ROCKET
{
	sync_device* rocket;
	sync_cb cb;
	const char* trackNames[] = {
		"CamX",
		"CamY",
		"CamZ",
		"Alpha",
		"Scene",
	};
	const struct sync_track* tracks[sizeof(trackNames) / sizeof(trackNames[0])];

#define NUM_TRACKS sizeof(trackNames) / sizeof(trackNames[0])

	bool up = false;
};


namespace BASS
{
	HSTREAM stream;
};

#endif


#define NUM_UNIF	6

#define UNIF_UTIME	0
#define UNIF_ALPHA	1
#define UNIF_CAMX	2
#define UNIF_CAMY	3
#define UNIF_CAMZ	4
#define UNIF_SCENE	5

#define TRACK_CAMX	0
#define TRACK_CAMY	1
#define TRACK_CAMZ	2
#define TRACK_ALPHA 3
#define TRACK_SCENE 4

#define EVAL_UNIFORMS \
	uniforms[UNIF_UTIME] = DEMO::time; \
	uniforms[UNIF_ALPHA] = GetSyncValue(TRACK_ALPHA); \
	uniforms[UNIF_SCENE] = GetSyncValue(TRACK_SCENE); \
	uniforms[UNIF_CAMX] = GetSyncValue(TRACK_CAMX); \
	uniforms[UNIF_CAMY] = GetSyncValue(TRACK_CAMY); \
	uniforms[UNIF_CAMZ] = GetSyncValue(TRACK_CAMZ); \


namespace WINDOW
{
	HWND hWnd;
	HDC hDC;
};

namespace RENDER
{
	int uLoc;
	float uniforms[NUM_UNIF];
};


void __fastcall Init();
LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef DEBUG_BUILD
void __fastcall Quit();
#endif
void __fastcall init_gl();
void __fastcall render_gl();
