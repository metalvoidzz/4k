#define _CRT_SECURE_NO_WARNINGS

#ifndef DEBUG_BUILD

#define SYNC_PRECALC_DATA
#include "auto_sync_data.h"
#include "impl.hh"

#else

#include <cstdio>

#endif


#include "clinkster/clinkster.h"
#include "fn.hh"
#include "def.hh"


#ifdef DEBUG_BUILD

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


	DEMO::hShader = CreateFile(PIXEL_FILE, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(DEMO::hShader, NULL, NULL, &DEMO::ftime);


	InitRocket();
	Init();
	Play();

	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	LARGE_INTEGER fps_to;
	QueryPerformanceCounter(&fps_to);
	int numframes = 0;

	MSG msg;
	SetTimer(WINDOW::hWnd, 0, 200, NULL);
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
				sprintf(buf, "%4.1f fps", numframes / dtime);
				SetWindowTextA(WINDOW::hWnd, buf);
				numframes = 0;
			}

			BASS_Update(0);
			Sleep(100);
		}
	}
}

#else

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

			DEMO::row = (int)((p / (float)Clinkster_TicksPerSecond) * row_rate);
			DEMO::time = DEMO::row * 0.01;

			//printf("Position(%f), Row(%i), Time(%f)\n", p, DEMO::row, DEMO::time);

			render_gl();

			Sleep(10);
		}
	}

	DEMO::Die();
}

void main()
{
	WinMainCRTStartup();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WinMainCRTStartup();
	return 0;
}

#endif
