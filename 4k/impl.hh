/* Implement missing c runtime functions */

#pragma once

#include <cinttypes>

extern "C"
{
	const void* _fltused = 0;
	const void* _chkstk = 0;

	extern "C" void* __cdecl memcpy(void *, void const *, size_t);
#pragma intrinsic(memcpy)
#pragma function(memcpy)
	void* __cdecl memcpy(void *dest, void const *src, size_t n)
	{
		char *csrc = (char *)src;
		char *cdest = (char *)dest;

		for (int i = 0; i<n; i++)
			cdest[i] = csrc[i];

		return cdest;
	}

	/*void _ftol2()
	{
		__asm
		{
			fistp qword ptr[esp - 8]
			mov edx, [esp - 4]
			mov eax, [esp - 8]
			ret
		}
	}*/

	/*extern "C" void * __cdecl memset(void*, int, size_t);
#pragma intrinsic(memset)
#pragma function(memset)
	void* __cdecl memset(void* b, int c, size_t len)
	{
		int i;
		unsigned char* p = (unsigned char*)b;
		i = 0;
		while (len > 0)
		{
			*p = c;
			p++;
			len--;
		}
		return(b);
	}*/

	extern "C" size_t __cdecl strlen(const char*);
#pragma intrinsic(strlen)
#pragma function(strlen)
	size_t __cdecl strlen(const char* ch)
	{
		size_t length = 0;
		if (ch != nullptr) while (ch[length] != '\0') ++length;
		return length;
	}

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
		eco.n.i = 184 *(y)+(16249);
		eco.n.j = 0;
		return eco.d;
	}

	float LOG(float y)
	{
		int * nTemp = (int*)&y;
		y = (*nTemp) >> 16;
		return (y - 16249) / 184;
	}

	extern "C" double __cdecl pow(double, double);
#pragma intrinsic(pow)
#pragma function(pow)
	double pow(double b, double p)
	{
		return EXP(LOG(b) * p);
	}

	extern "C" int __cdecl abs(int);
#pragma intrinsic(abs)
#pragma function(abs)
	int abs(int n)
	{
		const int ret[2] = { n, -n };
		return ret[n<0];
	}
}