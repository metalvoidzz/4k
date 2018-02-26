/* COPYRIGHT (C) 2018 Julian Offenhäuser
 *
 * Sources of my first demoscene production
 */

#ifndef DEBUG_BUILD

#pragma code_seg(".extern")
extern "C"
{
	void* _fltused = 0;
	void* _chkstk = 0;
	
	size_t __fastcall m_strlen(const char *str)
	{
		size_t length;
		for (length = 0; *str != '\0'; str++)
			length++;
		return length;
	}
}

#endif


#include "clinkster.h"
#include "fn.hh"
#include "sync.hh"


#ifdef DEBUG_BUILD

void __fastcall Quit()
{
	GenerateSyncData();
	WriteSyncData();
	DEMO::Die();
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

	Init();
	InitRocket();
	Play();

	// Timer driven message loop
	SetTimer(WINDOW::hWnd, 0, 10, NULL);
	MSG message;
	while (GetMessageA(&message, WINDOW::hWnd, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
		UpdateRocket();
	}
}

#else

// In release mode, no message loop to save space

#include "asmlib.hh"

void __stdcall WinMainCRTStartup()
{
	Init();
	Clinkster_GenerateMusic();
	Clinkster_StartMusic();

	while (1)
	{
		if (GetAsyncKeyState(VK_ESCAPE))
			DEMO::Die();

		if (Clinkster_GetPosition() > Clinkster_MusicLength) DEMO::Die();
		DEMO::time += 0.01;
		render_gl();
		Sleep(10);
	}
}

#endif
