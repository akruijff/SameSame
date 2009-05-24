
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "matchmatrix.h"

#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

#include <new>

#ifndef WITHOUT_LOGIC
MatchMatrix::MatchMatrix(size_t number) throw (std::bad_alloc)
{
#ifdef DEBUG
	if (number <= 0)
	{
		fprintf(stderr, "%s:%u Number (%u) to low\n",
			__FILE__, __LINE__, number);
		exit(EXIT_FAILURE);
	}
#endif
	n = number;
	signed char *tmp = new signed char[n * sizeof(char **) + (n + 1) * n / 2];
	arr = (signed char **)tmp;
	arr[0] = ((signed char *)tmp) + n * sizeof(char **);
	for (size_t i = 1; i < n; ++i)
		arr[i] = arr[i - 1] + n - i + 1;
#ifdef DEBUG
	if (tmp + n * sizeof(char **) + (n + 1) * n / 2 <= arr[n - 1])
	{
		fprintf(stderr, "%s:%u Array out of bounds\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
/*
	arr = new signed char*[n = number]; // throws bad_alloc
	try
	{
		arr[0] = new signed char[(n + 1) * n / 2]; // throws bad_alloc
		for (size_t i = 1; i < n; ++i)
			arr[i] = arr[i - 1] + n - i + 1;
	}
	catch(std::bad_alloc &e)
	{
		delete[] arr;
		throw(e);
	}
*/
}

MatchMatrix::~MatchMatrix() throw()
{
	signed char *tmp = (signed char *)arr;
	delete tmp;
//	delete[] arr[0];
//	delete[] arr;
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
#endif // WITHOUT_LOGIC
