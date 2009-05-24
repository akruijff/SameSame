
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_STATS_H
#define AK_STATS_H

#include "visitor.h"

#include <limits.h>

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

/**
 * Collects stats from the holder graph
 */
class Stats : public SamefileVisitor
{
	size_t minFileSize, maxFileSize, currentFileSize, maxDiskFileSize;
	unsigned long files, filenames; // file : filename = 1 : n
#ifdef __LONG_LONG_SUPPORTED
	unsigned long long totalSize, waisted;
#else // __LONG_LONG_SUPPORTED
	unsigned long totalSize, waisted;
#endif // __LONG_LONG_SUPPORTED

public:
	Stats() : minFileSize(UINT_MAX), maxFileSize(0), maxDiskFileSize(0),
		files(0), filenames(0), totalSize(0), waisted(0) {}

	/**
	 * Reset the internal values.
	 */
	Stats &reset()
	{
		totalSize = waisted = files = filenames = 0;
		maxFileSize = maxDiskFileSize = 0;
		minFileSize = UINT_MAX;
		return *this;
	}

	size_t getMinFileSize() { return minFileSize; }
	size_t getMaxFileSize() { return maxFileSize; }
	size_t getMaxDiskFileSize() { return maxDiskFileSize; }

	unsigned long getFiles() { return files; }
	unsigned long getFilenames() { return filenames; }

#ifdef __LONG_LONG_SUPPORTED
	unsigned long long getTotalSize() { return totalSize; }
	unsigned long long getWaisted() { return waisted; }
#else // __LONG_LONG_SUPPORTED
	unsigned long getTotalSize() { return totalSize; }
	unsigned long getWaisted() { return waisted; }
#endif // __LONG_LONG_SUPPORTED

	int visit(Holder &);
	int visit(SizeGroup &);
	int visit(FileGroup &);
	void visit(Filename &);
};

#endif // AK_STATS_H
