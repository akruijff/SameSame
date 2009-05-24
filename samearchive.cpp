
/* ************************************************************************ *
 * samearchive - Reads the paths from the input and output the identical    *
 *               files. This program is written for the special case        *
 *               where each directory acts as an archive or backup. The     *
 *               The output will only contain filename pairs that have the  *
 *               same relative path from the archive base.                  *
 *                                                                          *
 * Example:      find /backup/ | samearchive <dir1> <dir2> [<dir 3> ...]    *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

static const char version[] = "$Id: samearchive.cpp, v1.01 2009/04/14 00:00:00 akruijff Exp $\n";

#include "toolkit.h"
#include "stats.h"
#ifdef DEBUG
#include "debug.h"
#endif // DEBUG

// This file holds the engine code
#include "main.h"

static int argc;
static size_t *length;
static char **argv;
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
	fprintf(stderr, "[-l | -r] [-m size] [-S sep] [-0HiqVv] <dir1> <dir2> [...]");

	fprintf(stderr, "\nexample: find <dir> | %s [options] <dir1> <dir2> [...]", program);
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
	fprintf(stderr, "\n           -Z match based on the last filename");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

inline int passCombination(const char *a, const char *b,
	size_t len1, size_t len2)
{
	for (size_t i = 0; i < argc; ++i) if (length[i] <= len1)
		for (size_t j = i + 1; j < argc; ++j) if (length[j] <= len2)
			if (!memcmp(argv[i], a, length[i]) &&
				!memcmp(argv[j], b, length[j]) &&
				!strcmp(a + length[i], b + length[j]))
				return 0;
	return 1;
}

static int preCheck(const SizeGroup &parent,
const FileGroup &left, const FileGroup &right) throw()
{
	size_t leftBoundry = left.getBoundry();
	size_t rightBoundry = right.getBoundry();
	int output = 1;
	for (size_t i = 0; i < leftBoundry; ++i) if (left[i] != NULL)
	{
		const char *a = left[i]->data();
		size_t len1 = strlen(a);
		for (size_t j = 0; j < rightBoundry; ++j) if (right[j] != NULL)
			if (!passCombination(a, right[j]->data(),
				len1, strlen(right[j]->data())))
				return 0;
	}
	return 1;
}

static int selectResults(int flags, const char *sep)
{
	::flags = flags;
	::sep = sep;
	return FILE_IDENTICAL | FILE_BY_LOGIC;
}

static int printFileCompare(const SizeGroup &parent,
	const FileGroup &left, const Filename &leftChild,
	const FileGroup &right, const Filename &rightChild,
	int result) throw()
{
	if (passCombination(leftChild.data(), rightChild.data(),
		strlen(leftChild.data()), strlen(rightChild.data())))
		return 0;

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
	return 0;
}

int main(int argc, char **argv)
{
	_malloc_options = "H";
	Stats stats;
	int offset = processOptions(argc, argv, usage, version);
	::argc = argc - offset;
	::argv = argv + offset;
	length = new size_t[::argc];
	for (size_t i = 0; i < ::argc; ++i)
		length[i] = strlen(::argv[i]);

	processInput(stats, printFileCompare, selectResults, preCheck);
	if (S_VERBOSE_LEVEL2(flags))
		processStats(stats);
#ifdef DEBUG
	checkDynamic();
#endif // DEBUG

}
