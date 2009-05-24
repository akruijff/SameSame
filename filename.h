
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_FILENAME_H
#define AK_FILENAME_H

#include "configure.h"

#include <string.h>

#include <new>

class Filename
{
	char *str;

public:
	/**
	 * Assumes a and b are of type Filename ** and compares the strings.
	 */
	static int compare(const void *a, const void *b) throw()
	{ return (*(Filename **)a)->compare(**(Filename **)b); }

	/**
	 * Creates a capacity for a sting of the given size.
	 */
	Filename(size_t capacity = 0) throw (std::bad_alloc);

	/**
	 * Copies the string str in to the object.
	 */
	Filename(const char *str) throw (std::bad_alloc);

	~Filename() throw() { delete[] str; }

	/**
	 * Resize the capacity to hold a string of a length of capacity.
	 * @see renew
	 */
	void resize(size_t capacity) throw (std::bad_alloc);

	/**
	 * Renew the capacity to hold a string of a length of capacity.
	 * @see resize
	 */
	void renew(size_t capacity) throw (std::bad_alloc);

	/**
	 * Returns the filename in a string
	 */
	const char *data() const throw() { return str; }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns 0 on succes
	 */
	int compare(const Filename &obj) const throw()
	{ return strcmp(str, obj.str); }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns true - if both filenames are equal.
	 */
	int operator==(const Filename &obj) const throw()
	{ return !compare(obj); }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns true - if both filenames are not equal.
	 */
	int operator!=(const Filename &obj) const throw()
	{ return compare(obj); }

	/**
	 * Repleaces the current filename with a copy of the given string.
	 * Note: You need to know for sure there is enove space for str.
	 */
	void operator=(const char *str) throw() { strcpy(this->str, str); }
};

#endif // AK_FILENAME_H
