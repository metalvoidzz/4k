/* CRT function header */

#pragma once

extern "C"
{
	void CRT_memcpy(void *dest, void *src, unsigned long count);
	void CRT_memset(void *dest, int value, unsigned long count);
}
