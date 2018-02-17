/* COPYRIGHT (C) 2018 Julian Offenhäuser
 *
 * Sources of my first demoscene production
 */


#ifndef DEBUG_BUILD


// Implemented crt functions
extern "C"
{
	void* _fltused = 0;
	void* _chkstk = 0;
#pragma function(memset)
	void *memset(void *dest, int c, size_t count)
	{
		char *bytes = (char *)dest;
		while (count--)
			*bytes++ = (char)c;
		return dest;
	}

#pragma function(memcpy)
	void *memcpy(void *dest, const void *src, size_t count)
	{
		char *dest8 = (char *)dest;
		const char *src8 = (const char *)src;
		while (count--)
			*dest8++ = *src8++;
		return dest;
	}
}

#endif


/* Functions used in main and fn.hh */


#ifdef DEBUG_BUILD

#include "librocket/sync.h"
#include "bass/c/bass.h"

#include <math.h>

// Start demo playback
void __fastcall Play();
// Init rocket tracks
void __fastcall InitRocket();
// Rocket callback
void PauseRocket(void *d, int flag);
void SetRocketTime(void *d, int row);
int IsRocketRunning(void *d);
// Bass
double bass_get_row(HSTREAM h);

#endif


#include "clinkster.h"
#include "fn.hh"


#ifdef DEBUG_BUILD


static const float bpm = 210.0f;
static const int rpb = 8;
static const double row_rate = (double(bpm) / 60) * rpb;


void __fastcall Play()
{
	Init();

	// Play track from hd
	BASS_Start();
	BASS_ChannelPlay(BASS::stream, false);
}

void __fastcall InitRocket()
{
	using namespace ROCKET;

	r = sync_get_track(rocket, "r");
	g = sync_get_track(rocket, "g");
	b = sync_get_track(rocket, "b");

	// Init callback
	ROCKET::cb =
	{
		PauseRocket,
		SetRocketTime,
		IsRocketRunning,
	};
}

void PauseRocket(void *d, int flag)
{
	HSTREAM h = *((HSTREAM *)d);
	if (flag) BASS_ChannelPause(h);
	else BASS_ChannelPlay(h, false);
}

void SetRocketTime(void *d, int row)
{
	HSTREAM h = *((HSTREAM *)d);
	QWORD pos = BASS_ChannelSeconds2Bytes(h, row / row_rate);
	BASS_ChannelSetPosition(h, pos, BASS_POS_BYTE);
}

int IsRocketRunning(void *d)
{
	HSTREAM h = *((HSTREAM *)d);
	return BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
}

static double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	return time * row_rate;
}

void main()
{
	// Init clinkster
	Clinkster_GenerateMusic();

	//Init bass
	if (!BASS_Init(-1, 44100, 0, 0, 0))
		DEMO::Die(ERR_INIT_BASS);

	// Write music to hd
	FILE* outfile = fopen(EXPORT_TRACK_NAME, "wb");
	fwrite(Clinkster_WavFileHeader, 1, sizeof(Clinkster_WavFileHeader), outfile);
	fwrite(Clinkster_MusicBuffer, 1, Clinkster_WavFileHeader[10], outfile);
	fclose(outfile);

	BASS::stream = BASS_StreamCreateFile(false, EXPORT_TRACK_NAME, 0, 0, BASS_STREAM_PRESCAN);
	if (!BASS::stream)
		DEMO::Die(ERR_INIT_BASS);

	// Init rocket device
	ROCKET::rocket = sync_create_device("s");
	if (!ROCKET::rocket)
		DEMO::Die(ERR_INIT_SYNC);
	if (sync_connect(ROCKET::rocket, "localhost", SYNC_DEFAULT_PORT))
		DEMO::Die(ERR_INIT_SYNC);
	
	InitRocket();
	Play();

	// Timer driven message loop
	SetTimer(WINDOW::win_handle, 0, 10, NULL);
	MSG message;
	while (GetMessageW(&message, WINDOW::win_handle, 0, 0) != 0)
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

#else

// In release mode, include auto-generated stuff

void __stdcall WinMainCRTStartup()
{
	Init();
	Clinkster_GenerateMusic();
	Clinkster_StartMusic();

	SetTimer(WINDOW::win_handle, 0, 10, NULL);
	MSG message;
	while (GetMessageW(&message, WINDOW::win_handle, 0, 0) != 0)
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

#endif