
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_CONTAINER_CPP
#define AK_CONTAINER_CPP

#include <stdlib.h>

#include "container.h"
#include "toolkit.h"

template <class T>
Container<T>::Container(unsigned capacity, float factor)
{
#ifdef DEBUG
	if (capacity < 1)
	{
		fprintf(stderr, "%s:%d Capacity(%i) must be at least 1\n",
			__FILE__, __LINE__, capacity);
		exit(EXIT_FAILURE);
	}
	if (factor <= 0 || factor >= 1)
	{
		fprintf(stderr, "%s:%d factor(%f) must be between 0 and 1\n",
			__FILE__, __LINE__, factor);
		exit(EXIT_FAILURE);
	}
#endif
	mode = CONTAINER_HASH;
	size = 0;
	hashFunction = NULL;
	this->factor = 1024 * factor;
	arr = new T *[this->capacity = capacity];
	for (size_t i = 0; i < capacity; ++i)
		arr[i] = NULL;
}

template <class T>
Container<T>::~Container() throw()
{
	delete[] arr;
}

template <class T>
size_t Container<T>::key(const T &t) const throw()
{
	size_t key = (hashFunction ? hashFunction(t) : 0) % capacity;
	while(arr[key] != NULL && t != *arr[key])
		if (++key == capacity)
			key = 0;
#ifdef DEBUG
	if (key >= capacity)
	{
		fprintf(stderr, "%s:%d key(%i) must be between 0 <= key < %i\n",
			__FILE__, __LINE__, key, capacity);
		exit(EXIT_FAILURE);
	}
#endif
	return key;
}

template <class T>
T *Container<T>::first() const throw()
{
	switch(mode)
	{
		default:
		case CONTAINER_HASH:
			for (size_t i = 0; i < capacity; ++i)
				if (arr[i] != NULL)
					return arr[i];
		break;

		case CONTAINER_VECTOR:
			return arr[0];
		break;
	}
	return NULL;
}

template <class T>
T *Container<T>::last() const throw()
{
	switch(mode)
	{
		default:
		case CONTAINER_HASH:
			for (size_t i = capacity - 1; i > 0 && i < capacity; --i)
#ifdef DEBUG
				if (i >= capacity)
				{
					fprintf(stderr, "%s:%d index(%i) must be >= %i\n",
						__FILE__, __LINE__, i, capacity);
					exit(EXIT_FAILURE);
				} else
#endif
				if (arr[i] != NULL)
					return arr[i];

		case CONTAINER_VECTOR:
			return arr[size - 1];
	}
}

template <class T>
void Container<T>::deleteItem(size_t i) throw()
{
#ifdef DEBUG
	if (i >= capacity)
	{
		fprintf(stderr, "%s:%d %i should be less then %i\n",
			__FILE__, __LINE__, i, capacity);
	}
#endif
	delete arr[i];
	arr[i] = NULL;
	--size;
}

template <class T>
void Container<T>::deleteItems() throw()
{
	switch(mode)
	{
		default:
		case CONTAINER_HASH:
			for (size_t i = 0; i < capacity; ++i)
				if (arr[i] != NULL)
				{
					delete arr[i];
					arr[i] = NULL;
				}
		break;

		case CONTAINER_VECTOR:
			for (size_t i = 0; i < size; ++i)
			{
				delete arr[i];
				arr[i] = NULL;
			}
#ifndef FIXED
			for (size_t i = size; i < capacity; ++i)
				arr[i] = NULL;
#endif
		break;
	}
	size = 0;
}

template <class T>
void Container<T>::convert(int mode) throw()
{
	if (this->mode == mode)
		return;
	switch(mode)
	{
		case CONTAINER_HASH:
		break;

		case CONTAINER_VECTOR:
		{
			this->mode = CONTAINER_VECTOR;
			size_t i = 0, j = capacity - 1;
			while(i < size && j >= size)
			{
				// find the first NULL pointer
				while(i < j && arr[i] != NULL)
					++i;

				// find the first non-NULL pointer
				while(i < j && arr[j] == NULL)
					--j;

				arr[i] = arr[j];
#ifdef DEBUG
				arr[j] = NULL;
#endif // DEBUG
				--j;
			}
#ifdef DEBUG
			for (i = 0; i < size; ++i)
				if (arr[i] == NULL)
				{
					fprintf(stderr,
						"%s:%d This line should have some value.\n",
						__FILE__, __LINE__);
					printf("i=%i size=%i cap=%i\n", i, size, capacity);
					exit(EXIT_SUCCESS);
				}
			for (i = size; i < capacity; ++i)
				if (arr[i] != NULL)
				{
					fprintf(stderr,
						"%s:%d A This line should not have some value.\n"
						__FILE__, __LINE__);
					printf("i=%i size=%i cap=%i\n", i, size, capacity);
					exit(EXIT_SUCCESS);
				}
#endif // DEBUG
		}
		break;

#ifdef DEBUG
		default:
			fprintf(stderr,
				"%s:%d wrong mode value: %i\n",
				__FILE__, __LINE__, mode);
		break;
#endif // DEBUG
	}
}

template <class T>
void Container<T>::sort(int (&cmp)(const void *a, const void *b)) throw()
{
	if (mode != CONTAINER_VECTOR)
		convert(CONTAINER_VECTOR);
#ifdef DEBUG
	for (size_t i = 0; i < size; ++i)
		if (arr[i] == NULL)
		{
			fprintf(stderr,
				"%s:%d B This line should have some value.\n",
				__FILE__, __LINE__);
			printf("i=%i size=%i cap=%i\n", i, size, capacity);
			exit(EXIT_SUCCESS);
		}
#endif // DEBUG
	qsort(arr, size, sizeof(T *), cmp);
}

template <class T>
T *Container<T>::operator[](const T &t) const throw()
{
	switch(mode)
	{
		case CONTAINER_HASH:
			return arr[key(t)];

		case CONTAINER_VECTOR:
		default:
			for (size_t i = 0; i < size; ++i)
				if (arr[i] != NULL && t == *arr[i])
					return arr[i];
	}
	return NULL;
}

template <class T> inline
int Container<T>::operator!=(const T &t) const throw()
{
	switch(mode)
	{
		case CONTAINER_HASH:
			return arr[key(t)] == NULL;

		case CONTAINER_VECTOR:
		default:
			for (size_t i = 0; i < size; ++i)
				if (arr[i] != NULL && t == *arr[i])
					return 0;
			return 1;
	}
}

template <class T>
void Container<T>::operator+=(T &t)
{
	// Resize if needed
	switch(mode)
	{
		case CONTAINER_HASH:
		{
			if (size < ((size_t)capacity * factor) >> 10)
				break;
			size_t n = capacity;
			T **tmp = new T *[capacity <<= 1];
			for (size_t i = 0; i < capacity; ++i)
				tmp[i] = NULL;
			T **swap = tmp;
			tmp = arr;
			arr = swap;
			for (size_t i = 0; i < n; ++i)
				if (tmp[i] != NULL)
					arr[key(*tmp[i])] = tmp[i];
			delete[] tmp;
		}
		break;

		case CONTAINER_VECTOR:
		{
			if (size < capacity)
				break;
			T **tmp = new T *[capacity <<= 1];
#ifdef DEBUG
			for (size_t i = 0; i < size; ++i)
				tmp[i] = NULL;
#endif
#ifndef FIXED
			for (size_t i = size; i < capacity; ++i)
				tmp[i] = NULL;
#endif
			memcpy(tmp, arr, size * sizeof(T **));
			delete[] arr;
			arr = tmp;
		}
		break;
	}

	// Adding object t to the hash
	switch(mode)
	{
		case CONTAINER_HASH:
			arr[key(t)] = &t;
			++size;
		break;

		case CONTAINER_VECTOR:
			arr[size] = &t;
			++size;
		break;
	}
}

template <class T>
T *Container<T>::operator-=(T &t)
{
	size_t k = key(t);
	if (arr[k] == NULL)
		return NULL;
	T *ptr = arr[k];
	arr[k] = NULL;
	--size;
	if (++k == capacity)
		k = 0;
	while(arr[k] != NULL)
	{
		size_t i = key(*arr[k]);
		if (k != i)
		{
			arr[i] = arr[k];
			arr[k] = NULL;
		}
		if (++k == capacity)
			k = 0;
	}
	return ptr;
}
#endif // AK_CONTAINER_CPP
