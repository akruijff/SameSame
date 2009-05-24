
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <unistd.h>

#include "configure.h"
#include "toolkit.h"
#include "filename.h"
#include "filegroup.h"
#include "storage.h"

#include <new>

/* ************************************************************************ */

void removeDirectoryContence(char *storageFile)
{
	// Remove any regular files that may exist in the directory
	char *const paths[2] = {storageFile, NULL};
	FTS *ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
	FTSENT *ftsent;
	while((ftsent = fts_read(ftsp)) != NULL)
		if (ftsent->fts_info & FTS_F)
			unlink(ftsent->fts_accpath);
	fts_close(ftsp);
}

Storage::Storage(const char *program) throw (std::bad_alloc)
: stream(NULL), s(NULL)
{
	// Initial saving space
	int error;
	pid_t storagePid = getpid();
	storageBase = strlen(TEMP_STORAGE_DIR);
	storageBase += strlen(program);
	storageBase += digits(storagePid);
	storageBase += 2;
	storageFile = new char[storageBase + 11]; // throws bad_alloc
	try
	{
		line = new char[lineCapacity = PATH_INIT]; // throws bad_alloc
	}
	catch(const std::bad_alloc &e)
	{
		delete[] storageFile;
		throw(e);
	}

	// Populating storageFile
	sprintf(storageFile, "%s/%s", TEMP_STORAGE_DIR, program);
	mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO);
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/%u", storagePid);
	if (mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO))
		removeDirectoryContence(storageFile);
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/");
	storageBase = strlen(storageFile);
}

Storage::~Storage() throw()
{
	storageFile[storageBase] = 0;
	removeDirectoryContence(storageFile);
	rmdir(storageFile);
	delete[] storageFile;
	delete[] line;
}

int Storage::open(size_t filesize) throw()
{
	size_t n = digits(filesize);
	char *end = storageFile + storageBase;
	char *ptr = end + n;
#ifdef DEBUG
	if (ptr >= storageFile + storageBase + 11)
	{
		fprintf(stderr, "%s:%d pointer out of range\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif

	*ptr = 0;
	while(end != ptr)
	{
		*--ptr = filesize % 10;
		filesize /= 10;
	}
	if (stream != NULL)
		close();
	else
		counter = 0;
	stream = fopen(storageFile, "r");
}

const char *Storage::read(struct stat &s) throw (std::bad_alloc)
{
	fgetline(line, lineCapacity, stream); // throws bad_alloc

	if (sscanf(line, "%u", s.st_dev) == EOF)
		return NULL;
	char *str = strstr(str, " ");

	if (sscanf(++str, "%u", s.st_ino) == EOF)
		return NULL;
	str = strstr(str, " ");
	++counter;
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

int Storage::visit(Holder &holder) { return 0; }

int Storage::visit(SizeGroup &sizegroup)
{
	sprintf(storageFile + storageBase, "%u", sizegroup.getFileSize());
	if (stream != NULL)
		close();
	else
		counter = 0;
	stream = fopen(storageFile, "a");
	return 0;
}

int Storage::visit(FileGroup &filegroup)
{
	s = &filegroup.stat();
	return 0;
}

void Storage::visit(Filename &filename)
{
	fprintf(stream, "%u %u %s\n", s->st_dev, s->st_ino, filename.data());
	++counter;
}
