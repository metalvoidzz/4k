/* Implement missing c runtime functions */


#pragma once


#include <cinttypes>


extern "C"
{
	const void* _fltused = 0;
	const void* _chkstk = 0;

	extern "C" size_t __cdecl strlen(const char*);
	size_t __cdecl strlen(const char* ch)
	{
		const char *p = ch;
		while (*p) ++p;
		return p - ch;
	}
}