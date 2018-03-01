/* Sync stuff */


#pragma once


#ifdef DEBUG_BUILD


/* Rocket stuff */


#include "librocket/sync.h"
#include "bass/c/bass.h"

#include "def.hh"


static const float bpm = 210.0f;
static const int rpb = 8;
static const double row_rate = (double(bpm) / 60) * rpb;


void __fastcall InitRocket();
void PauseRocket(void *d, int flag);
void SetRocketTime(void *d, int row);
int IsRocketRunning(void *d);
void __fastcall Play();
double bass_get_row(HSTREAM h);
void __fastcall UpdateRocket();


void __fastcall InitRocket()
{
	using namespace ROCKET;

	// Init rocket device
	rocket = sync_create_device("s");

	if (!rocket)
		DEMO::Die(ERR_INIT_SYNC);

	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
		DEMO::Die(ERR_INIT_SYNC);

	for (int i = 0; i < NUM_TRACKS; i++)
		tracks[i] = sync_get_track(rocket, trackNames[i]);

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

void __fastcall Play()
{
	// Play track from hd
	BASS_Start();
	BASS_ChannelPlay(BASS::stream, false);
}

double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	return time * row_rate;
}

void __fastcall UpdateRocket()
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


/* Auto-generated sync header */


#include "auto_sync_data.h"


namespace SYNC_DATA
{
#ifdef SYNC_PRECALC_DATA
	float vals[NUM_ROWS * sizeof(float) * NUM_EVENTS];
#endif
	void* bindings[NUM_EVENTS];
}


/* Sync functions */


#include "def.hh"


#ifdef SYNC_PRECALC_DATA
// Pull data out of compressed sync file
// Increases performance and preload times
__forceinline void __fastcall PrecalcSyncData()
{
	using namespace SYNC_DATA;

	// Precalc tracks
	for (int t; t < NUM_TRACKS; t++)
	{
		// Precalc keys
		for (int i; i < NUM_EVENTS; i++)
		{
			SyncKey* k = &sync_data[i];
			vals[t * (k->time)] = k->value;
		}
	}
}
#endif

float __fastcall GetSyncValue(unsigned char index)
{
	return SYNC_DATA::vals[index * DEMO::row];
}

#endif
