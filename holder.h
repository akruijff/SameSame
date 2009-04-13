
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_HOLDER_H
#define AK_HOLDER_H

#include "configure.h"
#include "container.h"
#include "filegroup.h"
#include "sizegroup.h"

class SamefileVisitor;

/**
 * A Holder holds all the filenames with there meta data within a structure of
 * Container<SizeGroup> -> Container<FileGroup> -> Container<Filename>
 * @see SizeGroup, FileGroup, Filename, Container
 */
class Holder
{
	static SizeGroup tmp;

	Container<SizeGroup> hash;

public:
	/**
	 * Creates a Holder with the given capacity to store SizeGroup objects
	 */
	Holder(size_t capacity = 64);

	~Holder() throw() { hash.deleteItems();}

	/**
	 * Removes those SizeGroup who are within
	 * min <= sizeGroup.fileSize < max.
	 */
	size_t remove(size_t min, size_t max) throw();

	/**
	 * Selects the SizeGroup for the geven key. It is created if it doesn't
	 * exist jet.
	 */
	SizeGroup &operator[](const struct stat &key);

	/**
	 * Sorts all the SizeGroup objects in reversed cronologic order based
	 * on the file size, and sorts all the FileGroup objects within each
	 * SizeGroup depending on the function compare.
	 */
	void sort(int (&compare)(const void *a, const void *b)) throw();

	/**
	 * Implements the visitor pattern.
	 */
	void accept(SamefileVisitor &v);

	/**
	 * Compares each FilegGroup that live within the SizeGroup objects
	 *
	 * @param f - the function that will be called for each match. When
	 *            it returns true the internal loop is finished, but the
	 *            external loop is skipped. (i.e. when two FileGroups match
	 *            and the first  call returns true, then the result would
	 *            be 1 : n instead of m : n.)
	 * @param flags - the match result must have at least on bit in flags
	 *                set that is not the FILE_BY_LOGIC flag.
	 * @param postAction - Called after all FileGroup(s) are compared. The
	 *                     SizeGroup is removed and deleted when this
	 *                     function returns true.
	 * @param preCheck - Checks whether or not this combination can be
	 *                   skipped based on the information that is know at
	 *                   this time. The combination is checks when this
	 * @param readMaxFileSize - the max file size of paths written to disk
	 */
	void compareFiles(
		int (&f)(SizeGroup &, FileGroup &, Filename &,
			FileGroup &, Filename &, int),
		int flags,
		int (&addingAllowed)(const char *, const struct stat &,
			const FileGroup &),
		int (*postAction)(SizeGroup &) = NULL,
		int (*preCheck)(const SizeGroup &,
			const FileGroup &, const FileGroup &) = NULL) throw();
};

#endif // AK_HOLDER_H
