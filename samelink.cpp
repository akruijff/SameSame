
/* ************************************************************************ *
 * samelink - Read the output from samefile output and outputs a list of    *
 *            identical files that it coudn't process or wasn't allowed     *
 *            to process.                                                   *
 *                                                                          *
 * Hard link identical files, keepin the last modified files:               *
 *            find / | samefile -iZt | samelink -Zt                         *
 *                                                                          *
 * Create symlinks to between files that are not on the same filesystem     *
 *            find / | samefile -i | samelink -s                            *
 *                                                                          *
 * Only create hard link if the files have the same extention:              *
 *            find / | samefile -i | samelink '[^/.]+$'                     *
 * ************************************************************************ *
 *             Written by Alex de Kruijff           21 May 2009             *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

static const char version[] = "$Id: samelink.cpp,v 1.01 2009/04/14 00:00:00 akruijff Exp $\n";

#include "configure.h"
#include "toolkit.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define TIME_SYNC	30

#define VERBOSE_LEVEL1		1
#define VERBOSE_LEVEL2		2
#define VERBOSE_LEVEL3		3
#define VERBOSE_MAX			3
#define VERBOSE_MASK		3
#define DRYRUN				4
#define HUMAN_READABLE		8
#define MATCH_LEFT			16
#define MATCH_RIGHT	 		32
#define MATCH_TIME			64
#define SYMLINK				128

#define MATCH_AUTO			(MATCH_LEFT | MATCH_RIGHT)
#define MATCH_MASK			(MATCH_LEFT | MATCH_RIGHT | MATCH_TIME)

#define S_DRYRUN(m)				((m) & DRYRUN)
#define S_HUMAN_READABLE(m)		((m) & HUMAN_READABLE)
#define S_MATCH(m)				((m) & MATCH_MASK)
#define S_MATCH_TIME(m)			((m) & MATCH_TIME)
#define S_SYMLINK(m)			((m) & SYMLINK)
#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

#define FAILED_LINK_LEFT	-4
#define FAILED_LINK_RIGHT	-3
#define FAILED_REMOVE_LEFT	-2
#define FAILED_REMOVE_RIGHT	-1
#define PRINT_AGAIN			0
#define SUCCES_LEFT			1
#define SUCCES_RIGHT		2

// Retrieved from processOptions
static char *program = NULL;
static const char *sep = "\t";
static size_t minSize = 0, maxSize = UINT_MAX;
static unsigned int flags = VERBOSE_LEVEL1 | MATCH_LEFT;
static int stopping = 0;

// Retrieved from processInput
static size_t processed = 0, untouched = 0;
#ifdef __LONG_LONG_SUPPORTED
static unsigned long long bytesSaved = 0;
#else // __LONG_LONG_SUPPORTED
static unsigned long bytesSaved = 0;
#endif // __LONG_LONG_SUPPORTED
static struct timeval time0, time1;

static void quit(int sig)
{
	stopping = 1;
}

/**
 * Prints the usage of this program.
 */
static void usage()
{
	fprintf(stderr, "\n%s reads the samefile output and links", program);
	fprintf(stderr, "\nidentical files together.\n");
	fprintf(stderr, "\nusage: samelink [-A  |  -At  |  -L  |  -Z | -Zt] [-g size] \\");
	size_t len = strlen(program);
	fprintf(stderr, "\n        ");
	for (size_t i = 0; i < len; ++i)
		fprintf(stderr, " ");
	fprintf(stderr, "[-m size] [-S sep] [-HnqstVv]\n");

	fprintf(stderr, "example: find <dir> | samefile | %s\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "\n  options: -A relink based on the first filename");
	fprintf(stderr, "\n           -g only process files greater than size (0)");
//	fprintf(stderr, "\n           -H human readable statistics");
	fprintf(stderr, "\n           -L relink based on the number of links (default)");
	fprintf(stderr, "\n           -m only process files less or equal than size (0)");
	fprintf(stderr, "\n           -n perform a trial run with no changes made");
	fprintf(stderr, "\n           -q suppress non-error messages");
	fprintf(stderr, "\n           -S use sep as separator string for files (tab)");
	fprintf(stderr, "\n           -s symlink files if its not posible to hard the files");
	fprintf(stderr, "\n           -t match based on the modiciation time instead of the path");
	fprintf(stderr, "\n           -V output version information and exit");
	fprintf(stderr, "\n           -v increase verbosity");
	fprintf(stderr, "\n           -Z relink based on the last filename");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

static int processOptions(int argc, char **argv)
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	while ((c = getopt(argc, argv, "h?g:S:aHALZtisnqVv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': usage();						break;

			case 'g':
				if (sscanf(optarg, "%u", &minSize) != 1)
					minSize = 0, fprintf(stderr,
						"warning: can't convert -g %s, using -g 0 instead\n",
						optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%u", &maxSize) !=  1)
					maxSize = 0, fprintf(stderr,
						"warning: can't convert -m %s, using-g 0 instead\n",
						optarg);
				break;

			case 'S': sep = optarg;										break;
			case 'H': flags |= HUMAN_READABLE;							break;

			case 'A': flags |=  MATCH_LEFT;		flags &= ~MATCH_RIGHT;	break;
			case 'L': flags |=  MATCH_LEFT;		flags |=  MATCH_RIGHT;  break;
			case 'Z': flags &= ~MATCH_LEFT;		flags |=  MATCH_RIGHT;	break;
			case 't': flags |= MATCH_TIME;								break;

			case 's': flags |= SYMLINK;									break;
			case 'n': flags |= DRYRUN;									break;

			case 'q': flags &= ~VERBOSE_MASK;							break;
			case 'V': printf(version); exit(EXIT_SUCCESS);				break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags;	break;
		}
	return optind;
}

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns FAILED_REMOVE_LEFT  - coudn't remove the the left file
 *          FAILED_REMOVE_RIGHT - coudn't remove the the rigth file
 *          FAILED_LINK_LEFT    - coudn't link to the left file!
 *          FAILED_LINK_RIGHT   - coudn't link to the right file!
 *          PRINT_AGAIN         - files didn't meet the requirements
 *          SUCCES_LEFT         - on succes and the left file was removed
 *          SUCCES_RIGHT        - on succes and the right file was removed
 */
inline int relink(char *f1, char *f2, struct stat &s1, struct stat &s2)
{
	int (*linkf)(const char *dst, const char *src) =
		s1.st_dev == s2.st_dev ? link :
			S_SYMLINK(flags) ? symlink : NULL;

	// check if files can be processed
	if (linkf == NULL)
		return PRINT_AGAIN;

	char *dst, *src;
	switch(S_MATCH(flags))
	{
		default:
		case MATCH_LEFT:					// -A
			if (strcmp(f1, f2) < 0)			dst = f1, src = f2;
			else							dst = f2, src = f1;
			break;
		case MATCH_LEFT | MATCH_TIME:		// -At
			if (s1.st_mtime < s2.st_mtime)	dst = f1, src = f2;
			else							dst = f2, src = f1;
			break;
		case MATCH_RIGHT:					// -Z
			if (strcmp(f1, f2) < 0)			dst = f2, src = f1;
			else							dst = f1, src = f2;
			break;
		case MATCH_RIGHT | MATCH_TIME:		// -Zt
			if (s1.st_mtime < s2.st_mtime)	dst = f2, src = f1;
			else							dst = f1, src = f2;
			break;
		case MATCH_LEFT | MATCH_RIGHT:		// -L
			if (s1.st_nlink <= s2.st_nlink)	dst = f2, src = f1;
			else							dst = f1, src = f2;
			break;
	}

	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "%s %c> %s\n", src,
			s1.st_dev == s2.st_dev ? '=' : '-', dst);

	// I assume dst exists so no test for that
	if (!S_DRYRUN(flags))
		if (unlink(src) && S_VERBOSE(flags))
			return src == f1 ? FAILED_REMOVE_LEFT : FAILED_REMOVE_RIGHT;
		else if (link(dst, src) && S_VERBOSE(flags))
			return src == f1 ? FAILED_LINK_LEFT : FAILED_LINK_RIGHT;
	return src == f1 ? SUCCES_LEFT : SUCCES_RIGHT;
}

/**
 * Read each line from stdin and relink
 */
static void processInput(int argc, char **argv)
{
	gettimeofday(&time0, (struct timezone *)NULL);
	int len = strlen(sep);
	size_t fs = 1024, capacity = 1024;
	char *line = new char[capacity];

	size_t size = 0;
	int time2sync = time(NULL) + TIME_SYNC;
	struct stat s1, s2;
	char *f1 = new char[fs], *f2 = new char[fs];
	while(fgetline(line, capacity, stdin) != NULL)
	{
		// Is it time to sync?
		if (time(NULL) < time2sync && !S_DRYRUN(flags))
		{
			sync();
			time2sync = time(NULL) + TIME_SYNC;
		}
		if (stopping)
			break;

		// Is f1 and f2 to small?
		if (strlen(line) > fs)
		{
			delete[] f1;
			delete[] f2;
			while(strlen(line) > fs)
				fs <<= 1;
			f1 = new char[fs];
			f2 = new char[fs];
		}

		// read input
		if (inputSamefile(line, size, f1, f2, sep, len))
		{
			printf("%s\n", line);
			++untouched;
			continue;
		}

		// get meta data
		if (lstat(f1, &s1)  || lstat(f2, &s2))
		{
			printf("%s\n", line);
			++untouched;
			continue;
		}

		// check if files are already hard linked
		if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
		{
			outputHardLinked(f1, f2, s1.st_nlink, s1.st_size, sep);
			++untouched;
			continue;
		}

		// check the file size
		if (s1.st_size <= minSize || s2.st_size <= minSize ||
			s1.st_size > maxSize || s2.st_size > maxSize ||
			!S_SYMLINK(flags) && s1.st_dev != s2.st_dev)
		{
			outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
				s1.st_dev == s2.st_dev, sep);
			++untouched;
			continue;
		}

		switch(relink(f1, f2, s1, s2))
		{
			case FAILED_REMOVE_LEFT:
				outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
					s1.st_dev == s2.st_dev, sep);
				++untouched;
				if (S_VERBOSE_LEVEL1(flags))
					fprintf(stderr, "failed to remove %s\n",  f1);
			break;

			case FAILED_REMOVE_RIGHT:
				outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
					s1.st_dev == s2.st_dev, sep);
				++untouched;
				if (S_VERBOSE_LEVEL1(flags))
					fprintf(stderr, "failed to remove %s\n",  f2);
			break;

			case FAILED_LINK_LEFT:
				outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
					s1.st_dev == s2.st_dev, sep);
				++untouched;
				if (S_VERBOSE_LEVEL1(flags))
					fprintf(stderr, "failed to link %s -> %s\n",  f1, f2);
			break;

			case FAILED_LINK_RIGHT:
				outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
					s1.st_dev == s2.st_dev, sep);
				++untouched;
				if (S_VERBOSE_LEVEL1(flags))
					fprintf(stderr, "failed to link %s -> %s\n",  f2, f1);
			break;

			case SUCCES_LEFT:
				if (s1.st_nlink == 1)
					bytesSaved += s1.st_size;
				++processed;
			break;

			case SUCCES_RIGHT:
				if (s2.st_nlink == 1)
					bytesSaved += s2.st_size;
				++processed;
			break;

			default:
				++untouched;
				outputSamefile(f1, f2, s1.st_nlink, s2.st_nlink, s1.st_size,
					s1.st_dev == s2.st_dev, sep);
			break;
		}
	}
	delete[] line;
	delete[] f1;
	delete[] f2;
	gettimeofday(&time1, (struct timezone *)NULL);
}

static void processStats()
{
	fprintf(stderr, "\nProcessed files: %u", (unsigned)processed);
	fprintf(stderr, "\nUntouched files: %u", (unsigned)untouched);
	fprintf(stderr, "\nBytes saved....: %llu", bytesSaved);
	fprintf(stderr, "\nExecution time:");
	fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	signal(SIGHUP, quit);
	signal(SIGINT, quit);
	signal(SIGALRM, quit);
	signal(SIGTERM, quit);
	signal(SIGXCPU, quit);
	signal(SIGXFSZ, quit);
	signal(SIGVTALRM, quit);
	signal(SIGPROF, quit);
	signal(SIGUSR1, quit);
	signal(SIGUSR2, quit);
	signal(SIGTHR, quit);
	int offset = processOptions(argc, argv);
	processInput(argc - offset, argv + offset);
	if (S_VERBOSE_LEVEL3(flags))
		processStats();
}
