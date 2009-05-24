
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_MATCHMATRIX_H
#define AK_MATCHMATRIX_H

#include "configure.h"

#ifndef WITHOUT_LOGIC
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

#include <new>

class MatchMatrix
{
	size_t n;
	signed char **arr;

public:
	/**
	 * Creates a MatchMatrix that is able to store the match results of
	 * n elements.
	 */
	MatchMatrix(size_t n) throw (std::bad_alloc);

	~MatchMatrix() throw();

#ifndef DEBUG
	int get(size_t i, size_t j) const throw()
	{ return arr[i][j - i]; }

	void set(size_t i, size_t j, int result) throw()
	{ arr[i][j - i] = result; }

	/**
	 * Reset a number of elements to 0.
	 * @param i - the row to be reset
	 */
	void reset(size_t i) throw()
	{ memset(arr[i], 0, n - i); }
#else
	int get(size_t i, size_t j) const throw();
	void set(size_t i, size_t j, int result) throw();
	void reset(size_t i) throw();
#endif // DEBUG
};
#endif // WITHOUT_LOGIC
#endif // AK_MATCHMATRIX_H
