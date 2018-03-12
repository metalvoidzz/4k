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

#define EXP_A 184
#define EXP_C 16249 

	float EXP(float y)
	{
		union
		{
			float d;
			struct
			{
#ifdef LITTLE_ENDIAN
				short j, i;
#else
				short i, j;
#endif
			} n;
		} eco;
		eco.n.i = EXP_A*(y)+(EXP_C);
		eco.n.j = 0;
		return eco.d;
	}

	float LOG(float y)
	{
		int * nTemp = (int*)&y;
		y = (*nTemp) >> 16;
		return (y - EXP_C) / EXP_A;
	}

	float POW(float b, float p)
	{
		return EXP(LOG(b) * p);
	}
}

#endif


#ifndef DEBUG_BUILD

#include "auto_sync_data.h"
#define SYNC_PRECALC_DATA

#endif


#include "clinkster.h"
#include "fn.hh"


#ifdef DEBUG_BUILD

#include <stdio.h>

void __fastcall Quit()
{
	DEMO::Die();
}

void main()
{
	Clinkster_GenerateMusic();

	if (!BASS_Init(-1, 44100, 0, 0, 0))
		DEMO::Die(ERR_INIT_BASS);

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

	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	LARGE_INTEGER fps_to;
	QueryPerformanceCounter(&fps_to);
	int numframes = 0;

	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			UpdateRocket();

			render_gl();

			LARGE_INTEGER time;
			QueryPerformanceCounter(&time);
			numframes++;

			const float dtime = (time.QuadPart - fps_to.QuadPart) / (float)tps.QuadPart;
			if (dtime > 0.2f)
			{
				fps_to.QuadPart = time.QuadPart;
				char buf[64];
				printf("%4.1f fps\n", numframes / dtime);
				numframes = 0;
			}

			BASS_Update(0);
			Sleep(30);
		}
	}
}

#else

#include "asmlib.hh"

void __stdcall WinMainCRTStartup()
{
	Init();
	Clinkster_GenerateMusic();
	Clinkster_StartMusic();

	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			float p = Clinkster_GetPosition();

			if (p > Clinkster_MusicLength) break;

			DEMO::time = p / Clinkster_TicksPerSecond;
			DEMO::row = DEMO::time * row_rate;

			render_gl();

			Sleep(10);
		}
	}

	DEMO::Die();
}

#endif
