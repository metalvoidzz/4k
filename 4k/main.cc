/* COPYRIGHT (C) 2018 Julian Offenhäuser
 *
 * Sources of my first demoscene production
 */


extern "C"
{
	// Work around automatic c runtime linkage
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


#define DEBUG_BUILD

#include "fn.hh"
#include "clinkster.h"


void __stdcall WinMainCRTStartup()
{
	Init();
	Clinkster_GenerateMusic();
	Clinkster_StartMusic();

	while (1)
	{
		float pos = Clinkster_GetPosition();
		if (pos > Clinkster_MusicLength) DEMO::Die();
		DEMO::Loop();
		Sleep(10);
	}
}