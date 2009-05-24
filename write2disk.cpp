
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"

#ifdef WITH_DISK_STORAGE

#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"
#include "holder.h"
#include "write2disk.h"


int Write2Disk::visit(Holder &holder) { return 0; }

int Write2Disk::visit(SizeGroup &sizegroup)
{
	if ((fileSize = sizegroup.getFileSize()) < max)
		counter += sizegroup.diskWrite(storage);
	return 1;
}

int Write2Disk::visit(FileGroup &filegroup) { return 1; }
void Write2Disk::visit(Filename &filename) {}

#endif // WITH_DISK_STORAGE
