
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"
#include "holder.h"
#include "stats.h"

void Stats::visit(Holder &holder) {}

void Stats::visit(SizeGroup &sizegroup)
{
	currentFileSize = sizegroup.getFileSize();
	if (minFileSize > currentFileSize)	minFileSize = currentFileSize;
	if (maxFileSize < currentFileSize)	maxFileSize = currentFileSize;
	totalSize += currentFileSize;
	waisted += currentFileSize * sizegroup.getIdenticals();
	files += sizegroup.getSize();
}

void Stats::visit(FileGroup &filegroup)
{
	filenames += filegroup.getSize();
}

void Stats::visit(Filename &filename) {}
