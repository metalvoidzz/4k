/* COPYRIGHT (C) 2018 Julian Offenhäuser
 *
 * Sources of my first demoscene production
 */

#ifndef DEBUG_BUILD

extern "C"
{
	void* _fltused = 0;
	void* _chkstk = 0;

	void m_memcpy(void *dest, void *src, size_t n)
	{
		char *csrc = (char *)src;
		char *cdest = (char *)dest;

		for (int i = 0; i<n; i++)
			cdest[i] = csrc[i];
	}

	void _ftol2()
	{
		__asm
		{
			fistp qword ptr[esp - 8]
			mov edx, [esp - 4]
			mov eax, [esp - 8]
			ret
		}
	}
}

#endif


#ifndef DEBUG_BUILD
#include "auto_sync_data.h"
#endif


#include "clinkster.h"
//#define SYNC_PRECALC_DATA
#include "fn.hh"


#ifdef DEBUG_BUILD

void __fastcall Quit()
{
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

	InitRocket();
	Init();
	Play();

	SetTimer(WINDOW::hWnd, 0, 10, NULL);
	MSG message;
	while (GetMessageA(&message, WINDOW::hWnd, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
}

#else

#include "asmlib.hh"

void __stdcall WinMainCRTStartup()
{
	Init();
	Clinkster_GenerateMusic();
	Clinkster_StartMusic();

	MSG message;
	while (GetMessageA(&message, WINDOW::hWnd, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
		
		DEMO::time = Clinkster_GetPosition() / Clinkster_TicksPerSecond;
		DEMO::row = DEMO::time / 0.01;

		if (Clinkster_GetPosition() > Clinkster_MusicLength) break;
		
		render_gl();

		Sleep(10);
	}

	DEMO::Die();
}

#endif
