/* Header to the auto-generated sync data */

#pragma once

typedef struct
{
	int row;
	float value;
	int interp;
}SyncTrack;

float GetSyncValue(int row, char* track);
