
/* ************************************************************************ *
 * samefile - Read the paths from the input and outputs the identical       *
 *            files. This program has additional options and a lower        *
 *            memory foot print from the original program written by        *
 *            Jens Schweikhardt.                                            *
 *                                                                          *
 * To get a list for when you intent to keep the file last modified:        *
 *            find / | samefile-iZt                                         *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

static const char version[] = "$Id: samefile.cpp, v3.01 2009/04/14 00:00:00 akruijff Exp $\n";

#include "toolkit.h"
#include "stats.h"
#include "sizegroup.h"
#ifdef DEBUG
#include "debug.h"
#endif // DEBUG

// This file holds the engine code
#include "main.h"

static int flags;
static const char *sep;

/**
 * Prints the usage of this program.
 */
static void usage(const char *program) throw()
{
	fprintf(stderr, "\n%s: read alist of filenames from stdin and", program);
	fprintf(stderr, "\nwrites a list of identical files on stdout.\n");

	fprintf(stderr, "\nusage: %s [-a | -A | -L | -Z | -At | -Az] [-g size] \\", program);
	size_t len = strlen(program);
	fprintf(stderr, "\n        ");
	for (size_t i = 0; i < len; ++i)
		fprintf(stderr, " ");
	fprintf(stderr, "[-l | -r] [-m size] [-S sep] [-0HiqVvx]");

	fprintf(stderr, "\nexample: find <dir> | %s [options]", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "\n  options: -A match based on the first filename (default)");
	fprintf(stderr, "\n           -a don't sort files with same size alphabetically");
	fprintf(stderr, "\n           -g only output files greater than size (0)");
	fprintf(stderr, "\n           -H human readable statistics");
	fprintf(stderr, "\n           -i add files with identical inodes");
	fprintf(stderr, "\n           -L match based on the number of links");
	fprintf(stderr, "\n           -l don't report hard linked filenames (default)");
	fprintf(stderr, "\n           -m only output files less or equal than size (0)");
	fprintf(stderr, "\n           -q suppress non-error messages");
	fprintf(stderr, "\n           -r report hard linked filenames");
	fprintf(stderr, "\n           -S use sep as separator string for files (tab)");
	fprintf(stderr, "\n           -t match based on the modiciation time \\");
	fprintf(stderr, "\n              instead of the alphabethical order");
	fprintf(stderr, "\n           -V output version information and exit");
	fprintf(stderr, "\n           -v increase verbosity");
	fprintf(stderr, "\n           -x match all (n x n)");
	fprintf(stderr, "\n           -Z match based on the last filename");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

static int selectResults(int flags, const char *sep)
{
	::flags = flags;
	::sep = sep;
	return S_FULL_LIST(flags)	? FILE_IDENTICAL | FILE_BY_LOGIC
					: FILE_IDENTICAL;
}

static int printFileCompare(const SizeGroup &parent,
	const FileGroup &left, const Filename &leftChild,
	const FileGroup &right, const Filename &rightChild,
	int result) throw()
{
	switch(result)
	{
		case FILE_IDENTICAL:
		case FILE_IDENTICAL | FILE_BY_LOGIC:
			outputSamefile(leftChild.data(), rightChild.data(),
				left.stat().st_nlink, right.stat().st_nlink,
				parent.getFileSize(), left.isOnSameDevice(right), sep);
			break;
		case FILE_OPEN1_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				fprintf(stderr, "inaccessible: %s\n", leftChild.data());
			break;
		case FILE_OPEN2_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				fprintf(stderr, "inaccessible: %s\n", rightChild.data());
			break;
		case FILE_READ1_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				fprintf(stderr, "unreadable: %s\n", leftChild.data());
			break;
		case FILE_READ2_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				fprintf(stderr, "unreadable: %s\n", rightChild.data());
	}
	return !S_FULL_LIST(flags);
}

int main(int argc, char **argv)
{
	_malloc_options = "H";
	Stats stats;
	processOptions(argc, argv, usage, version);
	processInput(stats, printFileCompare, selectResults);
	if (S_VERBOSE_LEVEL2(flags))
		processStats(stats);
#ifdef DEBUG
	checkDynamic();
#endif // DEBUG
}
