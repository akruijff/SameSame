
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_STORAGE_H
#define AK_STORAGE_H

#include "configure.h"
// #include "hash.h"
// #include "container.h"
// #include "filename.h"
#include "visitor.h"

#include <stdio.h>
#include <sys/stat.h>

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

/**
 * Utility that allows for storing paths temporarily to disk in order to
 * save memory. To write paths to the disk this class must be passed as
 * an visitor. followed by passing this class as an visitor. To read
 * paths from the disk the member function open must be called followed
 * by the member function read.
 */
class Storage : public SamefileVisitor
{
	FILE *stream;
	const struct stat *s;
	char *storageFile, *line;
	size_t storageBase, lineCapacity, counter;

public:
	Storage(const char *program) throw (std::bad_alloc);
	~Storage() throw();

	int open(size_t filesize) throw();
	const char *read(struct stat &s) throw (std::bad_alloc);
	size_t close() throw();
	void clean() const throw();

	int visit(Holder &);
	int visit(SizeGroup &);
	int visit(FileGroup &);
	void visit(Filename &);
};

#endif // AK_STORAGE_H
