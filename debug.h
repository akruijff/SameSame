
#ifndef AK_DEBUG_H
#define AK_DEBUG_H
#define AK_DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#include <cstddef>
#include <new>

#define MAGIC_CODE 0x5AA55AA5
#ifndef DEBUG
#define DEBUG
#endif // DEBUG

inline void checkMagic(size_t magic, char *file, size_t line)
{
	if (magic != MAGIC_CODE)
	{
		fprintf(stderr, "%s:%d magic code fault.\n", file, line);
		exit(EXIT_FAILURE);
	}
}

void *operator new(std::size_t sz) throw (std::bad_alloc);

inline void *operator new[](std::size_t sz) throw (std::bad_alloc)
{ return operator new(sz); }

void operator delete(void *ptr);

inline void operator delete[](void *ptr)
{ operator delete(ptr); }

void checkDynamic();
#endif // AK_DEBUG_H
