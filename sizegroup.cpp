
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <stdio.h>

#include "configure.h"
#include "hash.h"
#include "toolkit.h"
#include "visitor.h"
#include "storage.h"
#include "filegroup.h"
#include "matchmatrix.h"
#include "sizegroup.h"

#ifndef FILE_BY_LOGIC
#define FILE_BY_LOGIC FILE_USER1
#endif

FileGroup SizeGroup::tmp;

int SizeGroup::compare(const void *a, const void *b) throw()
{
	SizeGroup *sa = *(SizeGroup **)a;
	SizeGroup *sb = *(SizeGroup **)b;
	return sb->fileSize - sa->fileSize;
}

/* ************************************************************************ */

SizeGroup::SizeGroup(const struct stat &s, size_t capacity) : hash(capacity)
{
	fileSize = s.st_size;
	nIdenticals = 0;
	storage = NULL;
	hash.setHashFunction(FileGroup::hashFunction);
}

hash_t SizeGroup::hashFunction(const SizeGroup &obj) throw()
{
	return hashword((hash_t *)&obj.fileSize, sizeof(off_t) / sizeof(hash_t));
}

FileGroup &SizeGroup::operator[](const struct stat &key)
{
	tmp = key;
	if (hash[tmp])
		return *hash[tmp];
	FileGroup *ptr = new FileGroup(key);
	hash += *ptr;
	return *ptr;
}

void SizeGroup::accept(SamefileVisitor &v)
{
	v.visit(*this);
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->accept(v);
}

void SizeGroup::sort(int (&compare)(const void *a, const void *b)) throw()
{
	size_t n = hash.getSize();
	hash.convert(CONTAINER_VECTOR);
	for (size_t i = 0; i < n; ++i)
		hash[i]->sort();
	hash.sort(compare);
}

size_t SizeGroup::diskWrite(Storage &storage) throw()
{
	accept(storage);
	this->storage = &storage;

	size_t counter = 0;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->empty();
	return storage.close();
}

int SizeGroup::diskRead(
	int (&addingAllowed)(const char *, const struct stat &, const FileGroup &))
{
	if (storage == NULL)
		return 0;
	if (storage->open(fileSize) < 0)
		return -1;
	const char *path;
	while((path = storage->read(tmp.s)) != NULL)
		if (addingAllowed(path, hash[tmp]->stat(), *hash[tmp]))
			*hash[tmp] += path;
	size_t counter = storage->close();
	storage->clean();
	return counter;
}

/* ************************************************************************ */

#ifdef WITH_LOGIC
void SizeGroup::compareFiles(MatchMatrix &match,
#else // WITH_LOGIC
void SizeGroup::compareFiles(
#endif // WITH_LOGIC
	int (&f)(SizeGroup &, FileGroup &, Filename &,
		FileGroup &, Filename &, int),
	int flags,
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) throw()
{
	size_t n = hash.getSize();
	if (n < 2)
		return;

	// Make sure the container is a vector
	hash.convert(CONTAINER_VECTOR);

	// Compare the files.
	int skipLogic = !(flags & FILE_BY_LOGIC);
	for (size_t i = 0; i < n; ++i)
	{
#ifdef DEBUG
		if (hash[i] == NULL)
		{
			fprintf(stderr, "%s:%d left filegroup doesn't exist\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		FileGroup &left = *hash[i];
#ifdef DEBUG
		if (!left.getSize())
		{
			fprintf(stderr, "%s:%d left filegroup is empty\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		size_t nn = hash[i]->getSize();
		for (size_t j = i + 1; j < n; ++j)
		{
#ifdef DEBUG
			if (hash[j] == NULL)
			{
				fprintf(stderr, "%s:%d right filegroup doesn't exist\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
#endif // DEBUG
			FileGroup &right = *hash[j];
#ifdef DEBUG
			if (!right.getSize())
			{
				fprintf(stderr, "%s:%d right filegroup is empty\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
#endif // DEBUG

			// Check so we can save on hard disk access.
			if (preCheck != NULL && preCheck(*this, left, right))
				continue;

			// Do a physically check if we couldn't use logic
#ifdef WITH_LOGIC
			int result = match.get(i, j);
			if (!(result))
			{
				result = hash[i]->fcmp(*hash[j]);
#else // WITH_LOGIC
				int result = hash[i]->fcmp(*hash[j]);
#endif // WITH_LOGIC
				if (result > 0 && result & FILE_IDENTICAL)
					++nIdenticals;
				else if (result < 0)
					f(*this, left, *left[0], right, *right[0], result);
#ifdef WITH_LOGIC
				match.set(i, j, result);
			}
#endif // WITH_LOGIC

			// do something with the result.
			switch(result)
			{
				case FILE_IDENTICAL | FILE_BY_LOGIC:
				case FILE_DIFFERENT | FILE_BY_LOGIC:
				case FILE_IDENTICAL:
				case FILE_DIFFERENT:
				{
					if ((flags & FILE_BY_LOGIC) && (result & FILE_BY_LOGIC) ||
						!(result & FILE_BY_LOGIC))
					{
						int skip = 0;
						size_t mm = hash[j]->getSize();
						for (size_t x = 0; x < nn; ++x)
						{
							for (size_t y = 0; y < mm; ++y)
								if (f(*this, left, *left[x], right, *right[y],
									result))
									skip = 1;
							if (skip == 1)
								break;
						}
					}
#ifdef WITH_LOGIC
					// a==b (00) a==c (01) => b==c (10)  | rowSize = 10
					// a!=d (02) => b!=d (12) c!=d (22)
					for (size_t k = i + 1; k <= j; ++k)
						if (j != k)
							switch(result | match.get(i, k))
							{
								case FILE_IDENTICAL:
									match.set(k, j,
										FILE_IDENTICAL | FILE_BY_LOGIC);
								break;

								case FILE_IDENTICAL | FILE_DIFFERENT:
									match.set(k, j,
										FILE_DIFFERENT | FILE_BY_LOGIC);
								break;
							}
#endif // WITH_LOGIC
				}
				break;

#ifdef WITH_LOGIC
				case FILE_OPEN1_ERROR:
				case FILE_READ1_ERROR:
						j = n;
				break;

				case FILE_OPEN2_ERROR:
				case FILE_READ2_ERROR:
					for (size_t k = i + 1; k < j; ++k)
						match.set(k, j, result);
				break;
#endif // WITH_LOGIC
			}
		}
#ifdef WITH_LOGIC
		match.reset(i);
#endif // WITH_LOGIC
	}
}
