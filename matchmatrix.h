
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_MATCHMATRIX_H
#define AK_MATCHMATRIX_H

#include "configure.h"

#ifdef WITH_LOGIC
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

class MatchMatrix
{
	size_t n;
	signed char **arr;

public:
	/**
	 * Creates a MatchMatrix that is able to store the match results of
	 * n elements.
	 */
	MatchMatrix(size_t n);

	~MatchMatrix() throw();

#ifndef DEBUG
	int get(size_t i, size_t j) const throw()
	{ return arr[i][j - i]; }
#else
	int get(size_t i, size_t j) const throw();
#endif // DEBUG

#ifndef DEBUG
	void set(size_t i, size_t j, int result) throw()
	{ arr[i][j - i] = result; }
#else
	void set(size_t i, size_t j, int result) throw();
#endif // DEBUG


#ifndef DEBUG
	/**
	 * Reset a number of elements to 0.
	 * @param i - the row to be reset
	 */
	void reset(size_t i) throw()
	{ memset(arr[i], 0, n - i); }
#else
	void reset(size_t i) throw();
#endif // DEBUG
};
#endif // WITH_LOGIC
#endif // AK_MATCHMATRIX_H
