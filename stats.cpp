
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"
#include "holder.h"
#include "stats.h"

int Stats::visit(Holder &holder) {return 0; }

int Stats::visit(SizeGroup &sizegroup)
{
	currentFileSize = sizegroup.getFileSize();
	if (minFileSize > currentFileSize)	minFileSize = currentFileSize;
	if (maxFileSize < currentFileSize)	maxFileSize = currentFileSize;
	totalSize += sizegroup.getSize() * currentFileSize;
	waisted += currentFileSize * sizegroup.getIdenticals();
	files += sizegroup.getSize();
	return 0;
}

int Stats::visit(FileGroup &filegroup)
{
	filenames += filegroup.getSize();
	return 1;
}

void Stats::visit(Filename &filename) {}
