
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <new>

#include <string.h>

#include "configure.h"
#include "matchmatrix.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef WITH_LOGIC
MatchMatrix::MatchMatrix(size_t number)
{
#ifdef DEBUG
	if (number <= 0)
	{
		fprintf(stderr, "%s:%u Number (%u) to low\n",
			__FILE__, __LINE__, number);
		exit(EXIT_FAILURE);
	}
#endif
	arr = new signed char*[n = number];
	size_t i = 0;
	try
	{
		for (; i < n; ++i)
		{
			arr[i] = new signed char[n - i];
			memset(arr[i], 0, n - i);
		}
	}
	catch(const std::bad_alloc &e)
	{
		for (--i; i > 0; --i)
			delete[] arr[i];
		delete[] arr[0];
		delete[] arr;
		throw(e);
	}
}

MatchMatrix::~MatchMatrix() throw()
{
	for (size_t i = 0; i < n; ++i)
		delete[] arr[i];
	delete[] arr;
}

#ifdef DEBUG
int MatchMatrix::get(size_t i, size_t j) const throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%u) must be smaller then %u\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	if (j < i)
	{
		fprintf(stderr, "%s:%u j (%u) must be larger or equal to i (%u)",
		__FILE__, __LINE__, j, i);
		exit(EXIT_FAILURE);
	}
	return arr[i][j - i];
}

void MatchMatrix::set(size_t i, size_t j, int result) throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%u) must be smaller then %u\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	if (j < i)
	{
		fprintf(stderr, "%s:%u j (%u) must be larger or equal to i (%u)",
			__FILE__, __LINE__, j, i);
		exit(EXIT_FAILURE);
	}
	arr[i][j - i] = result;
}

void MatchMatrix::reset(size_t i) throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%u) must be smaller then %u\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	memset(arr[i], 0, n - i);
}
#endif // DEBUG
#endif // WITH_LOGIC
