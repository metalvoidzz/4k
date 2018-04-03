/* Sync stuff */


#pragma once


static const float bpm = 104.0f;
static const int rpb = 8;
static const float row_rate = ((float)bpm / 60) * rpb;


#ifdef DEBUG_BUILD


/* Rocket stuff */


#include "librocket/sync.h"
#include "bass/c/bass.h"

#include "def.hh"


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

	rocket = sync_create_device("s");

	if (!rocket)
		DEMO::Die(ERR_INIT_SYNC);

	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
		DEMO::Die(ERR_INIT_SYNC);

	for (int i = 0; i < NUM_TRACKS; i++)
		tracks[i] = sync_get_track(rocket, trackNames[i]);

	ROCKET::cb = {
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
   
	ROCKET::up = BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
	return ROCKET::up;
}

void __fastcall Play()
{
	BASS_Start();
	BASS_ChannelPlay(BASS::stream, false);
}

double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	DEMO::time = time;
	return time * row_rate;
}

void __fastcall UpdateRocket()
{
	using namespace DEMO;

	row = bass_get_row(BASS::stream);
	if (sync_update(ROCKET::rocket, (int)floor(row), &ROCKET::cb, (void *)&BASS::stream))
		Die();
}

__forceinline float __fastcall GetSyncValue(unsigned char index)
{
	return sync_get_val(ROCKET::tracks[index], DEMO::row);
}

#else


namespace SYNC_DATA
{
	float data[NUM_TRACKS][NUM_ROWS];
}


/* Sync functions */


#include "def.hh"
#include "auto_sync_data.h"

#include <cmath>


using namespace SYNC_DATA;


#ifndef NO_INTER

// Precalculate single interpolation //
__forceinline void __fastcall inter_sync(uint16_t index)
{
	int i = index + 1;
	int inter = sync_data[index].inter;
	// If no value found, keep current one
	float next_val = sync_data[index].value;

	// Get next value
	while (i < NUM_EVENTS)
	{
		if (sync_data[i].time > sync_data[index].time && sync_data[i].value != 0 && sync_data[i].track == sync_data[index].track) {
			next_val = sync_data[i].value;
			break;
		}
		i++;
	}
	
	// Start at current row, but don't override its value
	float it = sync_data[index].time + 1;

	// Interpolate until row of next value reached or end was hit
	while (it < sync_data[i].time && it < NUM_ROWS)
	{
#ifdef USED_INTER_LINEAR
		if (inter == INTER_LINEAR) {
			float t = (it - sync_data[index].time) / (sync_data[i].time  - sync_data[index].time);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		} else
#endif
#ifdef USED_INTER_SMOOTH
		if (inter == INTER_SMOOTH) {
			float t = (it - sync_data[index].time) / (sync_data[i].time - sync_data[index].time);
			t = t * t * (3 - 2 * t);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		} else
#endif
#ifdef USED_INTER_RAMP
		if (inter == INTER_RAMP) {
			float t = (it - sync_data[index].time) / (sync_data[i].time - sync_data[index].time);
			t = pow(t, 2.0);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		} else
#endif
		{
			// No interpolation, keep value until next one
			data[sync_data[index].track][(int)it] = sync_data[index].value;
		}

		it++;
	}
}

#endif

__forceinline void __fastcall PrecalcSyncData()
{
	for (int i = 0; i < NUM_EVENTS; i++)
	{
		data[sync_data[i].track][sync_data[i].time] = sync_data[i].value;
#ifndef NO_INTER
		inter_sync(i);
#endif
	}
}

__forceinline float __fastcall GetSyncValue(uint16_t index)
{
	if (DEMO::row >= NUM_ROWS)
		return data[index][NUM_ROWS - 1];
	return data[index][DEMO::row];
}

#endif
