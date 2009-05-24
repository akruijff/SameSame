
/* ************************************************************************ *
 *             Written by Alex de Kruijff           21 May 2009             *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "debug.h"

// using std::new_handler;
using std::bad_alloc;

long varDynamic = 0;

void *operator new(size_t sz) throw (std::bad_alloc)
{
	if (sz == 0)
		sz = 1;
	void *ptr = (void *) malloc (sz);
	++varDynamic;

	while (ptr == 0)
	{
//		new_handler handler = __new_handler;
#ifdef __EXCEPTIONS
		throw bad_alloc();
#else
		std::abort();
#endif
		ptr = (void *) malloc (sz);
	}
	return ptr;
}

void operator delete(void *ptr)
{
	if (ptr)
	{
		if (--varDynamic == 0)
			fprintf(stderr, "debug: no leaks!\n");
		free(ptr);
	}
}

void checkDynamic()
{
	fprintf(stderr, "debug: leaking %u items\n", varDynamic);
}
