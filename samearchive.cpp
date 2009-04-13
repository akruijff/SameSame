
/* ************************************************************************ *
 * samearchive - Reads the paths from the input and output the identical    *
 *               files. This program is written for the special case        *
 *               where each directory acts as an archive or backup. The     *
 *               The output will only contain filename pairs that have the  *
 *               same relative path from the archive base.                  *
 *                                                                          *
 * Example:      find /backup/ | samearchive <dir1> <dir2> [<dir 3> ...]    *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

static const char version[] = "$Id: samearchive.cpp, v1.00 2009/04/14 00:00:00 akruijff Exp $\n";

#include "main.h"
#include "toolkit.h"
#include "stats.h"

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

static int
preCheck(const SizeGroup &p, const FileGroup &a, const FileGroup &b) throw()
{
	return 0;
}

static int selectResults(int flags, const char *sep)
{
	::flags = flags;
	::sep = sep;
	return FILE_IDENTICAL | FILE_BY_LOGIC;
}

static int printFileCompare(SizeGroup &parent, FileGroup &left,
	Filename &leftChild, FileGroup &right, Filename &rightChild,
	int result) throw()
{
	int pass = 1;
	const char *a = leftChild.data();
	const char *b = rightChild.data();
	size_t len1 = strlen(a);
	size_t len2 = strlen(b);
	for (size_t i = 0; i < argc && pass; ++i) if (length[i] <= len1)
		for (size_t j = i + 1; j < argc && pass; ++j) if (length[j] <= len2)
			if (!memcmp(argv[i], a, length[i]) &&
				!memcmp(argv[j], b, length[j]) &&
				!strcmp(a + length[i], b + length[j]))
				pass = 0;
	if (pass)
		return 0;

	switch(result)
	{
		case FILE_IDENTICAL:
		case FILE_IDENTICAL | FILE_BY_LOGIC:
			outputSamefile(a, b, left.stat().st_nlink, right.stat().st_nlink,
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
}
