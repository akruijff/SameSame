
/* ************************************************************************ *
 *             Written by Alex de Kruijff           21 May 2009             *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_FILEGROUP_H
#define AK_FILEGROUP_H

#include "configure.h"
#include "hash.h"
#include "container.h"
// #include "storage.h"
// #include "visitor.h"
#include "filename.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#include <sys/stat.h>

#include <new>

class SizeGroup;
class SamefileVisitor;

/**
 * A FileGroup is a group of Filename(s) that share the same device and inode
 * numers.
 * @see Filename, Container
 */
class FileGroup
{
#ifdef EXPERIMENTAL
	static XFilenameWrapper tmp;
#else // EXPERIMENTAL
	static Filename tmp;
#endif // EXPERIMENTAL

	struct stat s;
#ifdef EXPERIMENTAL
	Container<XFilename> hash;
#else // EXPERIMENTAL
	Container<Filename> hash;
#endif // EXPERIMENTAL

	friend class SizeGroup;

public:
	static hash_t hashFunction(const FileGroup &obj) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them on the
	 * number of links. If those are equal then the first Filename is
	 * used for comparison.
	 */
	static int compareNlink(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them
	 * alphabetically using the first Filename.
	 * In order for this to work the function sort must be called.
	 * @see sort
	 */
	static int compareFirst(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them
	 * reversed alphabetically using the last Filename.
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareLast(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them
	 * chronologically using the modication time.
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareOldest(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them
	 * reversed chronologically using the modication time.
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareYoungest(const void *a, const void *b) throw();

	/**
	 * Creates a FileGroup object without a preset shared metadata.
	 */
	FileGroup() throw (std::bad_alloc);

	/**
	 * Creates a FileGroup object with a preset shared meta data.
	 * @param s - shared meta data
	 * @param capacity - the initial capacity of the group.
	 */
	FileGroup(const struct stat &s, size_t capacity = 1)
	throw (std::bad_alloc);

	~FileGroup() throw() { hash.deleteItems(); }

	/**
	 *
	 */
	void empty() throw() { hash.empty(1); }

#ifdef DEBUG
	size_t getCapacity() const throw() { return hash.getCapacity(); }
#endif
	/**
	 * Return the number of Filenames within this group.
	 */
	size_t getSize() const throw() { return hash.getSize(); }

        /**
         * Gets the boundry for the mode of the container.
         */
        size_t getBoundry() const throw() { return hash.getBoundry(); }

	/**
	 * Returns that meta data.
	 */
	const struct stat &stat() const throw() { return s; }

	/**
	 * Implements the visitor pattern.
	 */
	void accept(SamefileVisitor &v);

	/**
	 * Checks if both files are on the same device.
	 */
	int isOnSameDevice(const FileGroup &obj) const throw()
	{ return s.st_dev == obj.s.st_dev; }

	/**
	 * Sort all the Filename on alphabethical order.
	 */
	void sort() throw()
#ifdef EXPERIMENTAL
	{ hash.sort(XFilename::compare); };
#else // EXPERIMENTAL
	{ hash.sort(Filename::compare); };
#endif // EXPERIMENTAL

	/**
	 * Open the file using any of Filename(s) within this group.
	 */
	int open(int flags) const throw();

	/**
	 * Compares if this FileGroup has the same contence of the given
	 * FileGroup.
	 * @returns see ::fcmp()
	 */
	int fcmp(const FileGroup &obj) const throw();

	/**
	 * Finds the Filename at the given index.
	 * @return NULL if the above is not true.
	 */
#ifdef EXPERIMENTAL
	const XFilename *operator[](int index) const
#else // EXPERIMENTAL
	const Filename *operator[](int index) const
#endif // EXPERIMENTAL
	{ return hash[index]; }

	/**
	 * Is this the same dev, inode pair?
	 */
	int operator==(const FileGroup &obj) const throw()
	{ return s.st_ino == obj.s.st_ino && s.st_dev == obj.s.st_dev; }

	/**
	 * Is this the same dev, inode pair?
	 */
	int operator!=(const FileGroup &obj) const throw()
	{ return s.st_ino != obj.s.st_ino || s.st_dev != obj.s.st_dev; }

	/**
	 * Doesn't str live in this object?
	 */
	int operator!=(const char *str) const throw (std::bad_alloc);

	/**
	 * Does str live in this object?
	 */
	int operator==(const char *str) const throw (std::bad_alloc)
	{ return !operator!=(str); }

	/**
	 * Sets the meta data of this FileGroup to s.
	 */
	void operator=(const struct stat &s) throw()
	{ this->s = s; }

	/**
	 * Adds a Filename to the FileGroup.
	 */
	void operator+=(const char *path) throw (std::bad_alloc);
};

#endif // AK_FILEGROUP_H
