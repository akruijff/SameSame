
/* ************************************************************************ *
 *             Written by Alex de Kruijff           21 May 2009             *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "hash.h"
#include "toolkit.h"
#include "matchmatrix.h"
#include "sizegroup.h"
#include "holder.h"
#include "visitor.h"
#include "stats.h"

#include <new>

/* ************************************************************************ */
SizeGroup Holder::tmp;

Holder::Holder(size_t capacity) throw (std::bad_alloc) : hash(capacity)
{
	hash.setHashFunction(SizeGroup::hashFunction);
}

size_t Holder::remove(size_t min, size_t max) throw()
{
	size_t counter = 0, n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i) if (hash[i] != NULL)
		if (min <= hash[i]->getFileSize() && hash[i]->getFileSize() < max)
		{
			hash.deleteItem(i);
			++counter;
		}
	hash.fix();
	return counter;
}

SizeGroup &Holder::operator[](const struct stat &s) throw (std::bad_alloc)
{
	tmp = s;
	if (hash[tmp] != NULL)
		return *hash[tmp];

	SizeGroup *ptr = new SizeGroup(s); // throws bad_alloc
	try
	{
		hash += *ptr; // throws bad_alloc
		return *ptr;
	}
	catch(std::bad_alloc &e)
	{
		delete ptr;
		throw(e);
	}
}

void Holder::sort(int (&compare)(const void *a, const void *b)) throw()
{
	hash.sort(SizeGroup::compare); // converts the container to an array
	size_t n = hash.getSize();
	for (size_t i = 0; i < n; ++i)
		hash[i]->sort(compare);
}

void Holder::accept(SamefileVisitor &v)
{
	if (v.visit(*this))
		return;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->accept(v);
}

void Holder::compareFiles(Stats &stats,
	int (&func)(const SizeGroup &, const FileGroup &,
		const Filename &, const FileGroup &, const Filename &, int),
	int flags,
	int (&addingAllowed)(const char *, const struct stat &,
		const FileGroup &),
	int (*postAction)(SizeGroup &),
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) throw(std::bad_alloc)
{
	// Clean up SizeGroup(s) with just one FileGroup
	// And get the maximum FileGroup(s) within any one SizeGroup
	size_t max = 1, size, n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
	{
		if (hash[i] == NULL)
			continue;
		if ((size = hash[i]->getSize()) > max)
			max = size;
	}

	// Compare
#ifndef WITHOUT_LOGIC
	MatchMatrix match(max); // throws(bad_alloc)
#endif // WITHOUT_LOGIC
	for (size_t i = 0; i < n; ++i)
	{
		if (hash[i] == NULL)
			continue;
		SizeGroup &select = *hash[i];
		select.diskRead(addingAllowed);
#ifndef WITHOUT_LOGIC
		select.compareFiles(match, func, flags, preCheck);
#else // WITHOUT_LOGIC
		select.compareFiles(func, flags, preCheck);
#endif // WITHOUT_LOGIC
		hash[i]->accept(stats);
		if (postAction != NULL && postAction(select))
			delete (hash -= *hash[i]);
	}
}
