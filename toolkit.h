
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_TOOLKIT_H
#define AK_TOOLKIT_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#define FILE_OPEN1_ERROR		-1
#define FILE_OPEN2_ERROR		-2
#define FILE_READ1_ERROR		-3
#define FILE_READ2_ERROR		-4
#define FILE_UNKOWN     		0
#define FILE_IDENTICAL  		1
#define FILE_DIFFERENT 			2

// for external use
#define FILE_USER1			4
#define FILE_USER2			8
#define FILE_USER3			16
#define FILE_USER4			32
#define FILE_USER5			64
#define FILE_USER6			128
#define FILE_USER7			256
#define FILE_USER8			512

/**
 * Compares two files using there meta data and then the regular data.
 *
 * @param	*fd1 - file discriptor 1
 * @param	*fd2 - file discriptor 2
 * @param	*s1 - meta data of file1
 * @param	*s2 - meta data of file2
 */
int fcmp(int fd1, int fd2, const struct stat &s1, const struct stat &s2) throw();

/**
 * Compares two files using there meta data and then the regular data.
 *
 * @param    f1 - file1
 * @param    f2 - file2
 * @param    s1 - meta data of file1
 * @param    s2 - meta data of file2
 * @returns  FILE_OPEN1_ERROR    when f1 couldn't be opend
 *           FILE_OPEN2_ERROR    when f2 couldn't be opend
 *           FILE_UNKOWN_ERROR   when some error accured.
 *           FILE_SAMESIZE_ERROR when the sizes are different.
 *           FILE_IDENTICAL      when the files are identical
 *           FILE_DIFFERENT      when the files are different
 */
int fcmp(const char *f1, const char *f2,
	const struct stat &s1, const struct stat &s2) throw();

/**
 * Returns the numberof digits the given number has.
 */
int digits(unsigned long number) throw();

/**
 * Reads a line from the input stream. If the capacity is to low the string
 * is enlarged and the old string is deleted.
 *
 * @param str - the location where the line goes
 * @param size - the capacity of the storage string
 * @param input - the stream to read from
 * @param eol - end of line
 */
char *fgetline(char *&str, size_t &size, FILE *file, int eol = '\n',
							char *pos = NULL);

/**
 * Writes the size in a human friendly way to the stream out.
 */
void fprintsize(FILE *out, unsigned long size) throw();

/**
 * Write the time difference between before and after to the stream out.
 * @param humanReable - do this in a human friendly way if non-0
 */
void fprinttime(FILE *out, struct timeval &after, struct timeval &before,
    int humanReadble = 0) throw();

inline void outputHardLinked(const char *f1, const char *f2,
	unsigned int links, size_t size, const char *sep) throw()
{
	printf("%u%s%s%s%s%s[%u]\n", (unsigned int)size, sep, f1, sep, f2,
		sep, links);
}

inline void outputSamefile(const char *f1, const char *f2,
	unsigned int l1, unsigned int l2,
	size_t size, int sameDevice, const char *sep) throw()
{
	printf("%u%s%s%s%s%s%c%s%u%s%u\n", (unsigned int)size, sep, f1, sep, f2,
		sep, sameDevice ? '=' : 'X', sep, l1, sep, l2);
}

inline int inputSamefile(const char *line, size_t &size, char *&f1, char *&f2,
	const char *sep, size_t len) throw()
{
	char *str, *tmp;
	if ((str = strstr(line, sep)) == NULL || sscanf(line, "%ui", &size) != 1)
		return 1;

	if ((str = strstr(tmp = str + len, sep)) == NULL)
		return 2;
	strncpy(f1, tmp, str - tmp);
	f1[str - tmp] = 0;

	if ((str = strstr(tmp = str + len, sep)) == NULL)
		return 3;
	strncpy(f2, tmp, str - tmp);
	f2[str - tmp] = 0;
	return 0;
}

#endif // AK_TOOLKIT_H
