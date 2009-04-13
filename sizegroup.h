
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_SIZEGROUP_H
#define AK_SIZEGROUP_H

#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "configure.h"
#include "hash.h"
#include "container.h"
// #include "filename.h"
// #include "filegroup.h"
// #include "matchmatrix.h"

class Filename;
class FileGroup;
class MatchMatrix;
class SamefileVisitor;
class Storage;

#define FILE_BY_LOGIC		FILE_USER1

/**
 * A SizeGroup is a group of files (FileGroup(s)) that share the same file size.
 */
class SizeGroup
{
private:
	static FileGroup tmp;

	off_t fileSize;
	Container<FileGroup> hash;
	size_t nIdenticals;
	Storage *storage;

public:
	static hash_t hashFunction(const SizeGroup &obj) throw();

	/**
	 * The function compares two SizeGroup objects for reversed
	 * cronologicly order based on the file size.
	 */
	static int compare(const void *a, const void *b) throw();

	/**
	 * Creates a SizeGroup object without a preset shared file size.
	 */
	SizeGroup() { fileSize = 0; nIdenticals = 0; storage = NULL; }

	/**
	 * Creates a SizeGroup object with a preset shared file size.
	 * @param s - meta data with the size.
	 * @param capacity - the initial capacity of the group.
	 */
	SizeGroup(const struct stat &s, size_t capacity = 4);
	~SizeGroup() throw() { hash.deleteItems(); }

	/**
	 * Returns the number of FileGroup objects that are in this object.
	 */
	size_t getSize() const throw() { return hash.getSize(); }

	/**
	 * Returns the number of identical FileGroup objects that are in this
	 * object. (Waisted diskspace = getIdenticals() * getFileSize().)
	 */
	size_t getIdenticals() const throw() {return nIdenticals; }

	/**
	 * Returns the file size each file within this object have in common.
	 */
	off_t getFileSize() const throw() { return fileSize; }

	/**
	 * Writes all the FileGroup(s) on to the disk.
	 * @return the number of paths written to the disk or -1 on failure.
	 */
	size_t diskWrite(Storage &storage) throw();

	/**
	 * Read all the FileGroup(s) from the disk.
	 * @return the number of paths read to the disk or -1 on failure.
	 */
	int diskRead(
		int (&addingAllowed)(const char *, const struct stat &,
			const FileGroup &));

	/**
	 * Is the given SizeGroup the same as this one?
	 */
	int operator==(const SizeGroup &obj) const throw()
	{ return fileSize == obj.fileSize; }

	/**
	 * Is the given SizeGroup different as this one?
	 */
	int operator!=(const SizeGroup &obj) const throw()
	{ return fileSize != obj.fileSize; }

	/**
	 * Sets the file size
	 * @see tmp
	 */
	void operator=(const struct stat &s) throw()
	{ fileSize = s.st_size; }

	/**
	 * Selects the FileGroup for the given key. It is created if it
	 * doesn't exist jet.
	 */
	FileGroup &operator[](const struct stat &key);

	/**
	 * Sorts all the FileGroup objects in order depinding on the function
	 * compare.
	 */
	void sort(int (&compare)(const void *a, const void *b)) throw();

	/**
	 * Implements the visitor pattern.
	 */
	void accept(SamefileVisitor &v);

#ifdef WITH_LOGIC
	/**
	 * Compares each FileGroup against each other and passes the result to
	 * the given function
	 *
	 * @param match - the match matrix where we keep the temp results.
	 * @param f - the function that will be called for each match. When
	 *            it returns true the internal loop is finished, but the
	 *            external loop is skipped. (i.e. when two FileGroups match
	 *            and the first  call returns true, then the result would
	 *            be 1 : n instead of m : n.)
	 * @param flags	- the match result must have at least on bit in flags
	 *                set that is not the FILE_BY_LOGIC flag.
	 * @param preCheck - Checks whether or not this combination can be
	 *                   skipped based on the information that is know at
	 *                   this time. The combination is checks when this
	 *                   function is not set or it returns true.
	 */
	void compareFiles(MatchMatrix &match,
#else // WITH_LOGIC
	void compareFiles(
#endif // WITH_LOGIC
		int (&f)(SizeGroup &, FileGroup &, Filename &,
			FileGroup &, Filename &, int),
		int flags,
		int (*preCheck)(const SizeGroup &,
			const FileGroup &, const FileGroup &) = NULL
		) throw();
};

#endif // AK_SIZEGROUP_H
