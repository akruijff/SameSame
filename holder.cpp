
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
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

/* ************************************************************************ */
SizeGroup Holder::tmp;

Holder::Holder(size_t capacity) : hash(capacity)
{
	hash.setHashFunction(SizeGroup::hashFunction);
}

size_t Holder::remove(size_t min, size_t max) throw()
{
	size_t counter = 0, n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i) if (hash[i] != NULL)
		if (min <= hash[i]->getFileSize() && hash[i]->getFileSize() < max)
		{
			hash -= *hash[i];
			++counter;
		}
	return counter;
}

SizeGroup &Holder::operator[](const struct stat &s)
{
	tmp = s;
	if (hash[tmp])
		return *hash[tmp];

	SizeGroup *ptr = new SizeGroup(s);
	hash += *ptr;
	return *ptr;
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
	v.visit(*this);
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->accept(v);
}

void Holder::compareFiles(int (&func)(SizeGroup &, FileGroup &,
		Filename &, FileGroup &, Filename &, int),
	int flags,
	int (&addingAllowed)(const char *, const struct stat &,
		const FileGroup &),
	int (*postAction)(SizeGroup &),
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) throw()
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
#ifdef WITH_LOGIC
	MatchMatrix match(max);
#endif // WITH_LOGIC
	for (size_t i = 0; i < n; ++i)
	{
		if (hash[i] == NULL)
			continue;
		SizeGroup &select = *hash[i];
		select.diskRead(addingAllowed);
#ifdef WITH_LOGIC
		select.compareFiles(match, func, flags, preCheck);
#else // WITH_LOGIC
		select.compareFiles(func, flags, preCheck);
#endif // WITH_LOGIC
		if (postAction != NULL && postAction(select))
			delete (hash -= *hash[i]);
	}
}
