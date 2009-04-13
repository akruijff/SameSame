
/* ************************************************************************ *
 * samearchive-lite - Reads the paths from the input and output the         *
 *                    identical files. This program is written for the      *
 *                    special case where each directory acts as an archive  *
 *                    or backup. This program is written for the special    *
 *                    case where each directory acts as an archive or       *
 *                    backup. The output will only contain filename pairs   *
 *                    that have the same relative path from the archive     *
 *                    base.                                                 *
 *                                                                          *
 *                    This version uses a lot less memory then samefile and *
 *                    is faster, but only find a partial set of identical   *
 *                    files. It basicaly does 80% of the job, but does this *
 *                    in 50% of the time while using 10% of the resources   *
 *                    compared to samearchive.                              *
 *                                                                          *
 * Example:           find <dir1> | samearchive-lite <dir1> <dir2> [...]    *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

static const char version[] = "$Id: samearchive-lite.cpp, v1.00 2009/04/14 00:00:00 akruijff Exp $\n";

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "configure.h"
#include "toolkit.h"

#ifdef STATIC_CACHE_CAPACITY
#include "hash.h"
#endif

typedef struct match_t {
	struct stat s1;
	struct stat s2;
	int result;

	// make sure match_t don't get out of scope!
	match_t &operator=(match_t &t)
	{
		s1.st_ino = t.s1.st_ino, s1.st_dev = t.s1.st_dev;
		s2.st_ino = t.s2.st_ino, s2.st_dev = t.s2.st_dev;
		return *this;
	}
} match_t;

#ifdef STATIC_CACHE_CAPACITY
static match_t cache[STATIC_CACHE_CAPACITY];
#else // STATIC_CACHE_CAPACITY
static match_t previous;
#endif // STATIC_CACHE_CAPACITY

#define VERBOSE_LEVEL1	1
#define VERBOSE_LEVEL2	2
#define VERBOSE_LEVEL3	3
#define VERBOSE_MAX		3
#define VERBOSE_MASK	3

#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

// Retrieved from processOptions
static char *program = NULL;
static const char *sep = "\t";
static size_t minSize = 0, maxSize = UINT_MAX;
static unsigned int flags = VERBOSE_LEVEL1;
static int eol = '\n';

/**
 * Prints the usage of this program.
 */
static void usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s read a list of filenames from stdin and\n", program);
	fprintf(stderr, "archives (directories) from the paramters and\n");
	fprintf(stderr, "writes a list of identical files on stdout.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "usage: %s [-g size] [-s sep] [-aqVv] <dir1> <dir2> [...]\n", program);
	fprintf(stderr, "exampe: find <dir1> | %s [options] <dir1> <dir2> [...]\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "  Options: -g : only output files greater than size (0)\n");
	fprintf(stderr, "           -m : only output files less or equal than size (0)\n");
	fprintf(stderr, "           -q : suppress non-error messages\n");
	fprintf(stderr, "           -S : use sep as separator string for files (tab)\n");
	fprintf(stderr, "           -V : output version information and exit\n");
	fprintf(stderr, "           -v : more verbose output on stderr\n");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

static int processOptions(int argc, char **argv)
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	while((c = getopt(argc, argv, "h?g:S:qVv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': usage();						break;
			case '0': eol = 0;											break;
			case 'g':
				if (sscanf(optarg, "%u", &minSize) != 1)
					minSize = 0, fprintf(stderr,
						"warning: can't convert -g %s, using -g 0 instead",
						optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%u", &maxSize) != 1)
					minSize = 0, fprintf(stderr,
						"warning: can't convert -m %s, using -g 0 instead",
						optarg);
				break;

			case 'S':sep = optarg;										break;

			case 'q': flags &= ~VERBOSE_MASK;							break;
			case 'V': printf(version); exit(EXIT_SUCCESS);				break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags;	break;
		}
	return optind;
}

#ifdef STATIC_CACHE_CAPACITY
static hash_t key(const match_t &m)
{
	return hashword((hash_t *)&m.s1.st_ino, sizeof(ino_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s2.st_ino, sizeof(ino_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s1.st_dev, sizeof(dev_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s2.st_dev, sizeof(dev_t) / sizeof(hash_t)
		))));
}
#endif

/**
 * Checks all files on the standerd input with there counter parts in the other
 * archives.
 */
static void processInput(int argc, char **argv)
{
	if (argc < 2)
		usage();
	size_t len0 = strlen(argv[0]);
	if (argv[0][len0 - 1] == '/')
		argv[0][--len0] = 0;
	if (argv[1][strlen(argv[1]) - 1] == '/')
		argv[1][strlen(argv[1]) - 1] = 0;

	// filther out doubles
	int *skip = new int[argc];
	{
		int counter = 0;
		for (int i = 1; i < argc; ++i)
		{
			skip[i] = 0;
			for (int j = 0; j < i; ++j)
				if (!strcmp(argv[i], argv[j]))
				{
					skip[i] = 1;
					++counter;
					continue;
				}
		}

		// Print usage if there are not at least two archives remaining
		if (argc < 2)
		{
			usage();
			delete[] skip;
			exit(EXIT_SUCCESS);
		}
	}

	// How much extra space do we need in f2 compaired to f1?
	size_t diff = strlen(argv[1]) - strlen(argv[0]);
	for (int i = 0; i < argc; ++i)
	{
		if (argv[i][strlen(argv[i]) - 1] == '/')
			argv[i][strlen(argv[i])] = 0;
		if (diff < strlen(argv[i]) - strlen(argv[0]))
			 diff = strlen(argv[i]) - strlen(argv[0]);
	}

#ifdef STATIC_CACHE_CAPACITY
	memset(cache, 0, STATIC_CACHE_CAPACITY * sizeof(match_t));
#else // STATIC_CACHE_CAPACITY
	memset(&previous, 0, sizeof(match_t));
#endif // STATIC_CACHE_CAPACITY

	int len;
	size_t f1n = 256, f2n = 256;
	char *f1 = new char[f1n], *f2 = new char[f2n];

	// Shortcut: the first part of f2 is constant if argc == 2
	if (argc == 2)
		memcpy(f2, argv[1], len = strlen(argv[1]));

	struct match_t m;
	while(fgetline(f1, f1n, stdin, eol) != NULL)
	{
		// Skip unlinkble lines or do not start with argv[0]
		if (lstat(f1, &m.s1) < 0 || (m.s1.st_mode & S_IFREG) == 0 ||
			m.s1.st_size <= minSize || m.s1.st_size > maxSize)
				continue;

		// Skip lines that do not start with argv[0]
		if (strlen(f1) < len0 || memcmp(f1, argv[0], len0))
		{
			fprintf(stderr, "error: line didn't start with %s\n", argv[0]);
			fprintf(stderr, "%s %i < %i\n", f1, strlen(f1), len0);
			continue;
		}

		// check on all other archives
		char *f = f1 + strlen(argv[0]);
		for (int i = 1; i < argc; ++i)
		{
			// filther out doubles
			if (skip[i])
				continue;

			// enlage f2 if needed
			if (f2n < strlen(f1) + diff)
			{
				while ((f2n <<= 1) < strlen(f1) + diff);
				delete[] f2;
				f2 = new char[f2n];
				if (argc == 2)
					memcpy(f2, argv[1], len);
			}

			// f2 = argv[i] + f
			if (argc > 2)
				memcpy(f2, argv[i], len = strlen(argv[i]));
			strcpy(f2 + len, f);

			// Skip unlinkble lines, diffent sizes or same file
			if (lstat(f2, &m.s2) || (m.s2.st_mode & S_IFREG == 0) ||
				m.s1.st_dev == m.s2.st_dev && m.s1.st_ino == m.s2.st_ino ||
				m.s1.st_size != m.s2.st_size)
			{
				continue;
			}

			// check f1 with f2
#ifdef STATIC_CACHE_CAPACITY
			hash_t k = key(m) % STATIC_CACHE_CAPACITY;
			switch(cache[k].s1.st_ino == m.s1.st_ino &&
				cache[k].s2.st_ino == m.s2.st_ino &&
				cache[k].s1.st_dev == m.s1.st_dev &&
				cache[k].s2.st_dev == m.s2.st_dev &&
				cache[k].s1.st_ino != 0 ?  cache[k].result :
					(cache[k] = m).result = fcmp(f1, f2, m.s1, m.s2))
#else // STATIC_CACHE_CAPACITY
			switch(previous.s1.st_ino == m.s1.st_ino &&
				previous.s2.st_ino == m.s2.st_ino &&
				previous.s1.st_dev == m.s1.st_dev &&
				previous.s2.st_dev == m.s2.st_dev &&
				previous.s1.st_ino != 0 ? previous.result :
					(previous = m).result = fcmp(f1, f2, m.s1, m.s2))

#endif // STATIC_CACHE_CAPACITY
			{
				case FILE_OPEN1_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						fprintf(stderr, "inaccessible: %s\n", f1);
					break;
				case FILE_OPEN2_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						fprintf(stderr, "inaccessible: %s\n", f2);
					break;
				case FILE_READ1_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						fprintf(stderr, "unreadable: %s\n", f1);
					break;
				case FILE_READ2_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						fprintf(stderr, "unreadable: %s\n", f2);
					break;
				case FILE_IDENTICAL:
					outputSamefile(f1, f2, m.s1.st_nlink, m.s2.st_nlink,
						m.s1.st_size, m.s1.st_dev == m.s2.st_dev, sep);
					break;
			}
		}
	}
	delete[] skip;
	delete[] f1;
	delete[] f2;
}

int main(int argc, char **argv)
{
	int offset = processOptions(argc, argv);
	processInput(argc - offset, argv + offset);
	return 0;
}
