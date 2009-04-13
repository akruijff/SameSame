
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <fcntl.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>

#include "configure.h"
#include "toolkit.h"

#ifdef WITH_MMAP
#include <sys/mman.h>
#endif

static char decimal_point = localeconv()->decimal_point[0];

static struct Buffer
{
	size_t size;
	char *buf1, *buf2;

	Buffer();
	~Buffer() throw();
} buffer;

Buffer::Buffer()
{
#ifdef WITH_MMAP
	size = (size_t)sysconf(_SC_PAGESIZE);
#else
	size = 2^15;
#endif
	buf1 = new char[size];
	buf2 = new char[size];
}

Buffer::~Buffer() throw()
{
	delete[] buf1;
	delete[] buf2;
}

int fcmp(int fd1, int fd2, const struct stat &s1, const struct stat &s2) throw()
{
	if (fd1 <= 0) return FILE_OPEN1_ERROR;
	if (fd2 <= 0) return FILE_OPEN2_ERROR;

	if (s1.st_size != s2.st_size) // diffent sizes means different
		return FILE_DIFFERENT;
	else if (s1.st_size == 0) // identical files if both file sizes are 0
		return FILE_IDENTICAL;

	int len = buffer.size;
	off_t offset = 0;
	int previous;
	do
	{
#ifdef WITH_MMAP
		void *m1 = mmap (0, buffer.size, PROT_READ, MAP_SHARED, fd1, offset);
		void *m2 = mmap (0, buffer.size, PROT_READ, MAP_SHARED, fd2, offset);

		if (m1 != MAP_FAILED && m2 != MAP_FAILED)
			if(memcmp(m1, m2, buffer.size))
			{
				munmap(m1, buffer.size);
				munmap(m2, buffer.size);
				return FILE_DIFFERENT;
			}
			else
			{
				munmap(m1, buffer.size);
				munmap(m2, buffer.size);
			}
		else
		{
			if (m1 == MAP_FAILED)
				munmap(m1, buffer.size);
			else
				munmap(m2, buffer.size);
#endif // WITH_MMAP
			// Read bytes from first files and check if we could.
			if ((previous = pread(fd1, buffer.buf1, buffer.size, offset)) < 0)
				return FILE_READ1_ERROR;

			// Read bytes from the second file and check if we read
			// the same amount.
			if (previous != pread(fd2, buffer.buf2, buffer.size, offset))
				return FILE_READ2_ERROR;

			if (memcmp(buffer.buf1, buffer.buf2, len) != 0)
				return FILE_DIFFERENT;
#ifdef WITH_MMAP
		}
#endif // WITH_MMAP
		offset += len;
	} while(offset < s1.st_size);
	return FILE_IDENTICAL;
}

int fcmp(const char *f1, const char *f2,
	const struct stat &s1, const struct stat &s2) throw()
{
	if (s1.st_size != s2.st_size) // diffent sizes means different
		return FILE_DIFFERENT;
	else if (s1.st_size == 0) // identical files if both file sizes are 0
		return FILE_IDENTICAL;

	// open files f1 and f2
	int fd1 = open(f1, O_RDONLY | O_SHLOCK | O_DIRECT);
	if (fd1 < 0)
		return FILE_OPEN1_ERROR;
	int fd2 = open(f2, O_RDONLY | O_SHLOCK | O_DIRECT);
	if (fd2 < 0)
	{
		close(fd1);
		return FILE_OPEN2_ERROR;
	}

	// check file input
	int status = fcmp(fd1, fd2, s1, s2);

	// Files are identical.
	close(fd1);
	close(fd2);
	return status ? status : FILE_IDENTICAL;
}

char *fgetline(char *&str, size_t &size, FILE *file, int eol, char *ptr)
{
	if (&size == 0)
		return NULL;
	if (ptr == NULL)
		ptr = str;
	int input = fgetc(file);
	if (input == EOF)
		return NULL;
	size_t n = 0;
	char *end = str + size - 1;

#ifdef DEBUG
	if (ptr - str >= size)
	{
		fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG

	while(input != eol && input != EOF)
	{
		*ptr = input;
		if (++ptr == end)
		{
			char *tmp = new char[size << 1];
			memcpy(tmp, str, size);
			delete[] str;
			ptr = (str = tmp) + size - 1;
			end = ptr + size;
			size <<= 1;
		}
#ifdef DEBUG
		if (ptr - str >= size)
		{
			fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		input = fgetc(file);
	}
	*ptr = 0;
#ifdef DEBUG
	if (ptr - str >= size)
	{
		fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG
	return str;
}

int digits(unsigned long number) throw()
{
	int counter = 0;
	while(number)
	{
		++counter;
		number /= 10;
	}
	return counter;
}

void fprintsize(FILE *out, unsigned long size) throw()
{
	int i = 0;
	while(size > 2048)
	{
		size >>= 10;
		++i;
	}
	switch(i)
	{
		case 0: fprintf(out, "%lu Bytes", size);		break;
		case 1: fprintf(out, "%lu KB", size);			break;
		case 2: fprintf(out, "%lu MB", size);			break;
		case 3: fprintf(out, "%lu GB", size);			break;
		case 4: fprintf(out, "%lu TB", size);			break;
		default: fprintf(out, "%lu ?B", size);
	}
}

static timeval tmpTime;

void fprinttime(FILE *out, struct timeval &after, struct timeval &before,
	int humanReadble) throw()
{
	tmpTime.tv_sec = after.tv_sec - before.tv_sec;
	tmpTime.tv_usec = after.tv_usec - before.tv_usec;
	if (tmpTime.tv_usec < 0)
		tmpTime.tv_sec--, tmpTime.tv_usec += 1000000;
	tmpTime.tv_usec /= 10000;
	if (humanReadble)
	{
		unsigned int days, hours, mins;
		days = tmpTime.tv_sec / 86400;
		tmpTime.tv_sec %= 86400;
		hours = tmpTime.tv_sec / 3600;
		tmpTime.tv_sec %= 3600;
		mins = tmpTime.tv_sec / 60;
		tmpTime.tv_sec %= 60;

		fprintf(out, "\t");
		if (days)
			fprintf(out, "%dd", days);
		if (hours)
			fprintf(out, "%dh", hours);
		if (mins)
			fprintf(out, "%dm", mins);
		fprintf(out, "%u%c%02lds", tmpTime.tv_sec,
			decimal_point, tmpTime.tv_usec);
	}
	else
		fprintf(out, "%9jd%c%02ld", (intmax_t)tmpTime.tv_sec,
			decimal_point, tmpTime.tv_usec);
}
