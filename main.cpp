
/* ************************************************************************ *
 * This is samefile driver. Programs can use this by including this source  *
 * file and implement the following functions:                              *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <new>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "configure.h"
#include "toolkit.h"
// #include "visitor.h"
#include "storage.h"
#include "write2disk.h"
#include "printhardlinked.h"
#include "stats.h"
// #include "filename.h"
// #include "filegroup.h"
// #include "sizegroup.h"
#include "holder.h"
#include "main.h"

#define VERBOSE_LEVEL1		1
#define VERBOSE_LEVEL2		2
#define VERBOSE_LEVEL3		3
#define VERBOSE_MAX			3
#define VERBOSE_MASK		3
#define ADD_HARDLINKED		4
#define FULL_LIST			8
#define HUMAN_READABLE		16
#define MATCH_LEFT			32
#define MATCH_RIGHT			64
#define MATCH_TIME			128
#define REPORT_HARDLINKS	256
#define SKIP_SORT		512

#define MATCH_AUTO			(MATCH_LEFT | MATCH_RIGHT)
#define MATCH_MASK			(MATCH_LEFT | MATCH_RIGHT | MATCH_TIME)

#define S_ADD_HARDLINKED(m)		((m) & ADD_HARDLINKED)
#define S_FULL_LIST(m)			((m) & FULL_LIST)
#define S_HUMAN_READABLE(m)		((m) & HUMAN_READABLE)
#define S_MATCH(m)				((m) & (MATCH_LEFT | MATCH_RIGHT))
#define S_MATCH_LEFT(m)			((m) & MATCH_LEFT)
#define S_MATCH_RIGHT(m)		((m) & MATCH_RIGHT)
#define S_MATCH_TIME(m)			((m) & MATCH_TIME)
#define S_MATCH_MASK(m)			((m) & MATCH_MASK)
#define S_REPORT_HARDLINKS(m)	((m) & REPORT_HARDLINKS)
#define S_SKIP_SORT(m)			((m) & SKIP_SORT)
#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

static Stats stats;

// Retrieved from processOptions
static const char *program = NULL;
static const char *sep = "\t";
static size_t minSize = 0, maxDiskFileSize = 0, maxSize = UINT_MAX;
static unsigned int flags = VERBOSE_LEVEL1 | MATCH_LEFT;
static int eol = '\n';

// Retrieved from processInput
#ifdef DEBUG
static struct timeval time0, time1, time2, time3, time4;
#else
static struct timeval time0, time2, time3, time4;
#endif

int processOptions(
	int argc, char **argv,
	void (&usage)(const char *),
	const char *version) throw()
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	while((c = getopt(argc, argv, "h?g:m:s:aH0xALZtilrqVv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': usage(program);				break;

			case 'g':
				if (sscanf(optarg, "%u",  &minSize) !=  1)
					minSize = 0, fprintf(stderr,
						"warning: can't convert -g %s, using-g 0 instead\n",
						optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%u",  &maxSize) !=  1)
					maxSize = 0, fprintf(stderr,
						"warning: can't convert -m %s, using-g 0 instead\n",
						optarg);
				break;

			case 's':
				fprintf(stderr, "this option is obsolite use -S instead\n");
			case 'S': sep = optarg; 									break;
			case 'H': flags |= HUMAN_READABLE; 							break;
			case '0': eol = 0; 											break;

			case 'a': flags &= ~MATCH_MASK;		 						break;
			case 'A': flags &= ~MATCH_RIGHT;	flags |= MATCH_LEFT;	break;
			case 'L': flags &= ~MATCH_MASK; 	flags |= MATCH_AUTO; 	break;
			case 'Z': flags &= ~MATCH_LEFT;		flags |= MATCH_RIGHT;  	break;
			case 't': 							flags |= MATCH_TIME; 	break;

			case 'x': 						flags |= FULL_LIST; 		break;
			case 'i':						flags |= ADD_HARDLINKED;	break;
			case 'r':						flags |= REPORT_HARDLINKS;	break;
			case 'l': flags &= ~REPORT_HARDLINKS;						break;

			case 'q': flags &= ~VERBOSE_MASK; 							break;
			case 'V': printf(version); exit(EXIT_SUCCESS);			break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags; 	break;
		}
	return optind;
}

#ifdef WITH_DISK_STORAGE
static void
solveMemoryProblem(Holder &holder, Write2Disk &write2Disk, size_t flags)
#else // WITH_DISK_STORAGE
static void solveMemoryProblem(Holder &holder, size_t flags)
#endif // WITH_DISK_STORAGE
{
	holder.accept(stats.reset());
	size_t maxDiskFileSize = 0, max = stats.getMaxFileSize();

#ifdef WITH_DISK_STORAGE
	// try to write to disk
	if (flags & 1)
		switch(S_MATCH_MASK(flags))
		{
			default:
				do
				{
					holder.accept(write2Disk.reset(++maxDiskFileSize));
				}
				while(maxDiskFileSize < max && write2Disk.done() <= 0);
				break;
			case MATCH_LEFT: // would give problems with sorting.
			case MATCH_RIGHT: // would give problems with sorting.
				write2Disk.reset(0);
		}

	// try to remove from memory if we fail
	if (write2Disk.done() <= 0)
	{
#endif // WITH_DISK_STORAGE
		if (flags & 2)
			while(!holder.remove(0, ++minSize) && minSize < max);

		// abort if we fail
		if (minSize == max)
		{
			fprintf(stderr,
				"Aborting... the filelist was much to large.\n");
			fprintf(stderr, "Try using the options -g and -m\n");
		}
		else if (S_VERBOSE_LEVEL1(flags))
			fprintf(stderr,
				"warning: memory full - changed minimum file size to %u\n",
				minSize);
#ifdef WITH_DISK_STORAGE
	}
	else if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "written %u items to disk\n", write2Disk.done());
#endif // WITH_DISK_STORAGE
}

#ifdef WITH_DISK_STORAGE
static void readInput(Holder &holder, Write2Disk &write2Disk) throw()
#else // WITH_DISK_STORAGE
static void readInput(Holder &holder) throw()
#endif // WITH_DISK_STORAGE
{
	size_t capacity = PATH_INIT;
	int continueRoutine = 1;
	char *pos = NULL, *path = new char[capacity]; // intentional
	struct stat s;
	do
	{
		try
		{
			while(pos = fgetline(path, capacity, stdin, eol, pos))
			{
				// Skip file if its unlinkable, non-regular file,
				// to small or to big
				if (lstat(path, &s) < 0 || S_ISREG(s.st_mode)  ==  0 ||
					s.st_size <= minSize || maxSize && s.st_size > maxSize)
						continue;

				FileGroup &filegroup = holder[s][s];
				if (filegroup != path &&
					(S_ADD_HARDLINKED(flags) || filegroup.getSize() == 0))
					filegroup += path;
				pos = NULL;
			}
			continueRoutine = 0;
		}
		catch(const std::bad_alloc &e)
		{
			pos = path + ((pos == NULL) ? capacity : strlen(path));
#ifdef WITH_DISK_STORAGE
			solveMemoryProblem(holder, write2Disk, 3);
#else // WITH_DISK_STORAGE
			solveMemoryProblem(holder, 3);
#endif // WITH_DISK_STORAGE
		}
	} while(continueRoutine);
	delete[] path;
}

static int addingAllowed(const char *path, const struct stat &s,
	const FileGroup &obj)
{
	return obj != path && (S_ADD_HARDLINKED(flags) || obj.getSize() == 0);
}

static int deleteEarly(SizeGroup &obj)
{
	return 1;
}

void processInput(Stats &stats,
	int (&printFileCompare)(SizeGroup &parent, FileGroup &left,
		Filename &leftChild, FileGroup &right, Filename &rightChild,
		int result),
	int (&selectResults)(int flags, const char *sep),
	int (*preCheck)(const SizeGroup &,
		const FileGroup &, const FileGroup &))
{
	gettimeofday(&time0, (struct timezone *)NULL);
	size_t oldMinSize = minSize;
	Holder holder;

	// Stage1 - reading the input
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: reading input\n");
#ifdef WITH_DISK_STORAGE
	Storage storage(program);
	Write2Disk write2Disk(storage);
	readInput(holder, write2Disk);
#else // WITH_DISK_STORAGE
	readInput(holder);
#endif // WITH_DISK_STORAGE

#ifdef DEBUG
	gettimeofday(&time1, (struct timezone *)NULL);
#endif

	// Stage2 - mandatory sorting
#ifdef DEBUG
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: sorting\n");
#endif
	switch(S_MATCH_MASK(flags))
	{
		case MATCH_LEFT:								// -A
			holder.sort(FileGroup::compareFirst);
			break;
		case MATCH_LEFT | MATCH_TIME:					// -At
			holder.sort(FileGroup::compareOldest);
			break;
		case MATCH_RIGHT:								// -Z
			holder.sort(FileGroup::compareLast);
				break;
		case MATCH_RIGHT | MATCH_TIME:					// -Zt
			holder.sort(FileGroup::compareYoungest);
			break;
		case MATCH_LEFT | MATCH_RIGHT:					// -L
			holder.sort(FileGroup::compareNlink);
	}
	gettimeofday(&time2, (struct timezone *)NULL);

	// Stage3 - print hard linked files
	if (S_REPORT_HARDLINKS(flags))
	{
		if (S_VERBOSE_LEVEL2(flags))
			fprintf(stderr, "info: hard linked filenames\n");
		PrintHardLinked printHardLinked(sep);
		holder.accept(printHardLinked);
		gettimeofday(&time3, (struct timezone *)NULL);
	}
	else
		time3 = time2;

	// Stage4 - checkfiles & print identical
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: comparing files\n");
	try
	{
		holder.compareFiles(printFileCompare, selectResults(flags, sep),
			addingAllowed, deleteEarly, preCheck);
	}
	catch(const std::bad_alloc &e)
	{
#ifdef WITH_DISK_STORAGE
		solveMemoryProblem(holder, write2Disk, 2);
#else // WITH_DISK_STORAGE
		solveMemoryProblem(holder, 2);
#endif // WITH_DISK_STORAGE
	}
	gettimeofday(&time4, (struct timezone *)NULL);

	// Stage5 - finish stats
	if (S_VERBOSE_LEVEL2(flags))
		holder.accept(stats.reset());

	if (minSize != oldMinSize && S_VERBOSE_LEVEL1(flags))
	{
		fprintf(stderr,
			"Changed mimimum file size from %u to %u.\n",
			oldMinSize, minSize);
		fprintf(stderr, "Continue with: ... | %s -g %u -m %u\n",
			program, oldMinSize, minSize);
	}
}

void processStats(Stats &stats)
{
	size_t avg = stats.getTotalSize() / stats.getFiles();
	int percentage = 100 * stats.getWaisted() / stats.getTotalSize();
	int ndigits = digits(stats.getTotalSize());

	fprintf(stderr, "info: stats");
	fprintf(stderr, "\nNumber of i-nodes...........: ");
	if (S_HUMAN_READABLE(flags))	fprintsize(stderr, stats.getFiles());
	else
	{
		for(int i = digits(avg); i < ndigits; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "%u", stats.getFiles());
	}

	fprintf(stderr, "\nNumber of filenames.........: ");
	if (S_HUMAN_READABLE(flags))	fprintsize(stderr, stats.getFilenames());
	else
	{
		for(int i = digits(avg); i < ndigits; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "%u", stats.getFilenames());
	}


	fprintf(stderr, "\nAverage file size...........: ");
	if (S_HUMAN_READABLE(flags))	fprintsize(stderr, avg);
	else
	{
		for(int i = digits(avg); i < ndigits; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "%u", avg);
	}

	fprintf(stderr, "\nGrand total file size.......: ");
	if (S_HUMAN_READABLE(flags))	fprintsize(stderr, stats.getTotalSize());
	else							fprintf(stderr, "%llu", stats.getTotalSize());

	fprintf(stderr, "\nWaisted disk space..........: ");
	if (S_HUMAN_READABLE(flags))	fprintsize(stderr, stats.getWaisted());
	else
	{
		for(int i = digits(stats.getWaisted()); i < ndigits; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "%llu", stats.getWaisted());
	}
	fprintf(stderr, " (%u%%)", percentage);

	fprintf(stderr, "\nExecution time:");
#ifdef DEBUG
	if (time1.tv_sec !=  time0.tv_sec || time1.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n    reading input...........:");
		fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
	}
	if (time2.tv_sec !=  time1.tv_sec || time2.tv_usec !=  time1.tv_usec)
	{
		fprintf(stderr, "\n    sorting.................:");
		fprinttime(stderr, time2, time1, S_HUMAN_READABLE(flags));
	}
#else
	if (time2.tv_sec !=  time0.tv_sec || time2.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n    reading input...........:");
		fprinttime(stderr, time2, time0, S_HUMAN_READABLE(flags));
	}
#endif
	if (time3.tv_sec !=  time2.tv_sec || time3.tv_usec !=  time2.tv_usec)
	{
		fprintf(stderr, "\n    report hard linked files:");
		fprinttime(stderr, time3, time2, S_HUMAN_READABLE(flags));
	}
	if (time4.tv_sec !=  time3.tv_sec || time4.tv_usec !=  time3.tv_usec)
	{
		fprintf(stderr, "\n    report identical files..:");
		fprinttime(stderr, time4, time3, S_HUMAN_READABLE(flags));
	}
	if (time4.tv_sec !=  time0.tv_sec || time4.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n                              --------+");
		fprintf(stderr, "\n        Total execution time:");
			fprinttime(stderr, time4, time0, S_HUMAN_READABLE(flags));
		fprintf(stderr, "\n");
	}
	else
		fprintf(stderr, "0s\n");
}
