
/* ************************************************************************ *
 *             Written by Alex de Kruijff           21 May 2009             *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "hash.h"
#include "toolkit.h"
#include "visitor.h"
#include "filename.h"
#include "filegroup.h"

#include <fcntl.h>
#include <unistd.h>

#include <new>

size_t tmpCapacity = 256;
#ifdef EXPERIMENTAL
XFilenameWrapper FileGroup::tmp(tmpCapacity);
#else // EXPERIMENTAL
Filename FileGroup::tmp(tmpCapacity);
#endif // EXPERIMENTAL

hash_t FileGroup::hashFunction(const FileGroup &obj) throw()
{
	return hashword(&obj.s.st_dev, 1, hashword(&obj.s.st_ino, 1));
}

int FileGroup::compareNlink(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fb.s.st_nlink - fa.s.st_nlink)
		return fb.s.st_nlink - fa.s.st_nlink;
	if (fa.s.st_dev - fb.s.st_dev)
		return fa.s.st_dev - fb.s.st_dev;
	return fa.s.st_ino - fb.s.st_ino;
}

int FileGroup::compareFirst(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
#ifdef DEBUG
	if (fa.hash.first() == NULL || fb.hash.first() == NULL)
	{
		fprintf(stderr, "%s:%d can not compare NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	return fa.hash.first()->compare(*fb.hash.first());
}

int FileGroup::compareLast(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
#ifdef DEBUG
	if (fa.hash.last() == NULL || fb.hash.last() == NULL)
	{
		fprintf(stderr, "%s:%d can not compare NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	return fb.hash.last()->compare(*fa.hash.last());
}

int FileGroup::compareOldest(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fa.s.st_mtime - fb.s.st_mtime)
		return fa.s.st_mtime - fb.s.st_mtime;
	if (fa.s.st_dev - fb.s.st_dev)
		return fa.s.st_dev - fb.s.st_dev;
	return fa.s.st_ino - fb.s.st_ino;
}

int FileGroup::compareYoungest(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fb.s.st_mtime - fa.s.st_mtime)
		return fb.s.st_mtime - fa.s.st_mtime;
	if (fa.s.st_dev - fb.s.st_dev)
		return fa.s.st_dev - fb.s.st_dev;
	return fa.s.st_ino - fb.s.st_ino;
}

/* ************************************************************************ */

FileGroup::FileGroup() throw (std::bad_alloc) : hash(1)
{
}

FileGroup::FileGroup(const struct stat &s, size_t capacity)
throw (std::bad_alloc) : hash(capacity)
{
	this->s = s;
	hash.convert(CONTAINER_VECTOR);
}

void FileGroup::accept(SamefileVisitor &v)
{
	if (v.visit(*this))
		return;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			v.visit(*hash[i]);
}


int FileGroup::open(int flags) const throw()
{
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
		{
			int fd = ::open(hash[i]->data(), flags);
			if (fd >= 0)
				return fd;
		}
		return -1;
}

int FileGroup::fcmp(const FileGroup &obj) const throw()
{
	// Different sizes means different files
	if (s.st_size != obj.s.st_size)
		return FILE_DIFFERENT;

	// Open the two files
	int fd1 = open(O_RDONLY);
	if (fd1 < 0)
		return FILE_OPEN1_ERROR;

	int fd2 = obj.open(O_RDONLY);
	if (fd2 < 0)
	{
		close(fd1);
		return FILE_OPEN2_ERROR;
	}

	int status = ::fcmp(fd1, fd2, s, obj.s);
	close(fd1);
	close(fd2);
	return status;
}

int FileGroup::operator!=(const char *path) const throw (std::bad_alloc)
{
	size_t len = strlen(path);
	if (tmpCapacity < len)
	{
		while(tmpCapacity < len)
			tmpCapacity <<= 1;
		tmp.renew(tmpCapacity);
	}
	tmp = path;
#ifdef EXPERIMENTAL
	return hash != tmp.getFilename();
#else // EXPERIMENTAL
	return hash != tmp;
#endif // EXPERIMENTAL
}

void FileGroup::operator+=(const char *path) throw (std::bad_alloc)
{
#ifdef EXPERIMENTAL
	size_t len = strlen(path);
	char *str = new char[++len];
	memcpy(str, path, len);
	try
	{
		hash += (XFilename *)str;
	}
#else // EXPERIMENTAL
	Filename *ptr = new Filename(path); // throws bad_alloc
	try
	{
		hash += *ptr; // throws bad_alloc
	}
#endif // EXPERIMENTAL
	catch(std::bad_alloc &e)
	{
		delete ptr;
		throw(e);
	}
}
