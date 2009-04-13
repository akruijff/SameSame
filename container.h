
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_CONTAINER_H
#define AK_CONTAINER_H

#define CONTAINER_HASH		1
#define CONTAINER_VECTOR	2

#include <stddef.h>

#include "configure.h"
#include "hash.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif


template <class T>
class Container
{
	int mode;
	T **arr;
	size_t size, capacity, factor;
	hash_t (*hashFunction)(const T &t);

public:
	Container(unsigned int capacity = 1, float factor = .8);
	~Container() throw();

private:
	/**
	 * Find the location of object t if it exists.
	 * Find a empty location for object t to go if it doesn't exists.
	 */
	size_t key(const T &t) const throw();

public:
	/**
	 * Returns the mode the container is in.
	 */
	int getMode() const throw()
	{ return mode; }

	/**
	 * Gets the size
	 */
	size_t getSize() const throw()
	{ return size; }

	/**
	 * Gets the size
	 */
	size_t getCapacity() const throw()
	{ return capacity; }

	/**
	 * Gets the boundry for the mode of the container.
	 */
	size_t getBoundry() const throw()
	{ return (mode == CONTAINER_VECTOR) ? size : capacity; }

	void setHashFunction(hash_t (*hashFunction)(const T &t)) throw()
	{ this->hashFunction = hashFunction; }

	/**
	 * Converts the internal working of the container to a hash or a vector.
	 */
	void convert(int mode) throw();

	/**
	 * Find the first element and returns is.
	 */
	T *first() const throw();

	/**
	 * Find the last element and returns is.
	 */
	T *last() const throw();

	/**
	 * Deletes a item. Only use this function if you are not interested in
	 * using the container as hash. Use delete (container -= object) if
	 * you do.
	 *
	 * @param index - the object at this index is deleted and replaced
	 *                with a NULL pointer.
	 * @see -=
	 */
	void deleteItem(size_t index) throw();

	/**
	 * Deletes all items
	 */
	void deleteItems() throw();

	/**
	 * Converts the array to a vector and sort it.
	 */
	void sort(int (&cmp)(const void *a, const void *b)) throw();

	/**
	 * Find object t and return a reference to it.
	 * This will be quite slow if the container is in the mode of a vector.
	 */
	T *operator[](const T &t) const throw();

	/**
	 * Returns a pointer to a object.
	 * @returns NULL if there is no element at the index.
	 */
	T *operator[](const size_t index) const throw()
#ifndef DEBUG
	{ return arr[index]; }
#else
	{
		if (index >= capacity)
		{
			fprintf(stderr, "%s:%u %u >= %u\n",
				__FILE__, __LINE__, index, capacity);
			exit(EXIT_FAILURE);
		}
		return arr[index];
	}
#endif

	/**
	 * Checks if object t is in the container.
	 * This will be quite slow if the container is in the mode of a vector.
	 */
	int operator!=(const T &t) const throw();
	int operator==(const T &t) const throw()
	{ return !operator!=(t); }

	/**
	 * Adds object t to the container. Operators with objects as paramaters
	 * will not function properly if objects are added that area equal (==)
	 * to another object in the container.
	 */
	void operator+=(T &t);

	/**
	 * Removes object t from the container. Operators with objects as
	 * paramatersw ill not function properly if objects are added that
	 * area equal (==) to another object in the container.
	 *
	 * Returns a pointer to the object that was removed;
	 */
	T *operator-=(T &t);
};

#endif // AK_CONTAINER_H
