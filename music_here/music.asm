; Clinkster music converted from ..\music_here\music.xrns 2018-02-15 20:55:22

%define USES_SINE 1
%define USES_SAWTOOTH 0
%define USES_SQUARE 1
%define USES_PARABOLA 0
%define USES_TRIANGLE 1
%define USES_NOISE 0
%define USES_VELOCITY 0
%define USES_LONG_NOTES 0
%define USES_DELAY 0
%define USES_PANNING 0
%define USES_INDEXDECAY 0
%define USES_GAIN 1

%define SUBSAMPLES_PER_TICK 12600
%define MAX_INSTRUMENT_SUBSAMPLES 1376256
%define MAX_TOTAL_INSTRUMENT_SAMPLES 589824
%define MAX_RELEASE_SUBSAMPLES 524288
%define TOTAL_SAMPLES 1376256
%define MAX_TRACK_INSTRUMENT_RENDERS 3

%define MAX_DELAY_LENGTH 0
%define LEFT_DELAY_LENGTH 0
%define RIGHT_DELAY_LENGTH 0
%define DELAY_STRENGTH 0.00000000

%define NUMTRACKS 5
%define LOGNUMTICKS 9
%define MUSIC_LENGTH 384
%define TICKS_PER_SECOND 14.00000000


	section instdata data align=1

_InstrumentData:
	; 00:  Track 01 / 01|String
	db	0,0,8,5,0,27,3,12,9,39,-93,0,-58,0,0,-3,19,29
	db	36,34,0,-1
	; 01:  Track 02 / 01|String
	db	0,0,8,5,0,27,3,12,9,45,-93,0,-58,0,0,-3,19,29
	db	28,32,13,0,11,31,0,-1
	; 02:  Track 03 / 02|Basstromme
	db	0,0,0,0,0,0,1,12,15,45,32,13,-44,-34,29,-27,-6,-1
	db	26,12,8,4,0,-1
	; 03:  Track 04 / 04|Drone
	db	2,1,7,7,5,5,5,5,13,45,0,0,0,0,0,-53,-18,21
	db	36,17,15,0,-1
	; 04:  Track 04 / 02|Basstromme
	db	0,0,0,0,0,0,1,12,15,45,32,13,-44,-34,29,-27,-6,-1
	db	26,8,0,-1
	db	-1

	section notepos data align=1

_NotePositions:
	; 00:  Track 01 / 01|String
	; position 0 - pattern 0
	db	0
	; position 1 - pattern 0
	db	64
	; position 2 - pattern 1
	db	64
	; position 3 - pattern 2
	db	64

	; 01:  Track 02 / 01|String
	; position 0 - pattern 0
	db	31
	; position 1 - pattern 0
	db	64
	; position 2 - pattern 1
	db	64
	; position 3 - pattern 2
	db	64
	; position 4 - pattern 3
	db	33,32
	; position 5 - pattern 4
	db	32,32

	; 02:  Track 03 / 02|Basstromme
	; position 4 - pattern 3
	db	-2,0,12,4,8,8,12,4,8
	; position 5 - pattern 4
	db	8,12,4,8,8,12,4,8

	; 03:  Track 04 / 04|Drone
	; position 2 - pattern 1
	db	-1,128
	; position 3 - pattern 2
	db	64

	; 04:  Track 04 / 02|Basstromme
	; position 3 - pattern 2
	db	-1,248


	section notesamp data align=1

_NoteSamples:
	; 00:  Track 01 / 01|String
	; position 0 - pattern 0
	db	0
	; position 1 - pattern 0
	db	0
	; position 2 - pattern 1
	db	0
	; position 3 - pattern 2
	db	0
	db	-1

	; 01:  Track 02 / 01|String
	; position 0 - pattern 0
	db	2
	; position 1 - pattern 0
	db	2
	; position 2 - pattern 1
	db	2
	; position 3 - pattern 2
	db	2
	; position 4 - pattern 3
	db	0,1
	; position 5 - pattern 4
	db	0,1
	db	-1

	; 02:  Track 03 / 02|Basstromme
	; position 4 - pattern 3
	db	0,2,1,1,0,2,1,1
	; position 5 - pattern 4
	db	0,2,1,1,0,2,1,1
	db	-1

	; 03:  Track 04 / 04|Drone
	; position 2 - pattern 1
	db	1
	; position 3 - pattern 2
	db	0
	db	-1

	; 04:  Track 04 / 02|Basstromme
	; position 3 - pattern 2
	db	0
	db	-1

