
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "toolkit.h"
#include "filename.h"
#include "filegroup.h"
#include "storage.h"

/* ************************************************************************ */

Storage::Storage(const char *program) : stream(NULL), s(NULL)
{
	// Initial saving space
	int error;
	pid_t storagePid = getpid();
	storageBase = strlen(TEMP_STORAGE_DIR);
	storageBase += strlen(program);
	storageBase += digits(storagePid);
	storageBase += 2;
	storageFile = new char[storageBase + 11];
	line = new char[lineCapacity = PATH_INIT];

	// Populating storageFile
	sprintf(storageFile, "%s/%s", TEMP_STORAGE_DIR, program);
	mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO);
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/%u", storagePid);
	mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO);
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/");
	storageBase = strlen(storageFile);
}

Storage::~Storage() throw()
{
	storageFile[storageBase] = 0;
	rmdir(storageFile);
	delete[] storageFile;
	delete[] line;
}

int Storage::open(size_t filesize) throw()
{
	size_t n = digits(filesize);
	char *end = storageFile + storageBase;
	char *ptr = end + n;
	*ptr = 0;
	while(end != ptr)
	{
		*--ptr = filesize % 10;
		filesize /= 10;
	}
	stream = fopen(storageFile, "r");
}

const char *Storage::read(struct stat &s)
{
	fgetline(line, lineCapacity, stream);

	if (sscanf(line, "%u", s.st_dev) == EOF)
		return NULL;
	char *str = strstr(str, " ");

	if (sscanf(++str, "%u", s.st_ino) == EOF)
		return NULL;
	str = strstr(str, " ");
	counter = 0;
	return ++str;
}

size_t Storage::close() throw()
{
	fclose(stream);
	stream = NULL;
	return counter;
}

void Storage::clean() const throw()
{
	unlink(storageFile);
}

void Storage::visit(Holder &holder) {}

void Storage::visit(SizeGroup &sizegroup)
{
	if (stream != NULL)
		close();
	sprintf(storageFile + storageBase, "%u", sizegroup.getFileSize());
	stream = fopen(storageFile, "a");
	counter = 0;
}

void Storage::visit(FileGroup &filegroup)
{
	s = &filegroup.stat();
}

void Storage::visit(Filename &filename)
{
	fprintf(stream, "%u %u %s\n", s->st_dev, s->st_ino, filename.data());
	++counter;
}
