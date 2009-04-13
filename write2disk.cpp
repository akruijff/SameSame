
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
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
#include "storage.h"
#include "write2disk.h"

void Write2Disk::visit(Holder &holder) {}

void Write2Disk::visit(SizeGroup &sizegroup)
{
	if ((fileSize = sizegroup.getFileSize()) < max)
		counter += sizegroup.diskWrite(storage);
}

void Write2Disk::visit(FileGroup &filegroup) {}
void Write2Disk::visit(Filename &filename) {}

#endif // WITH_DISK_STORAGE
