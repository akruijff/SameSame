
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "toolkit.h"
#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"
#include "holder.h"
#include "printhardlinked.h"

void PrintHardLinked::visit(Holder &holder) {}
void PrintHardLinked::visit(SizeGroup &sizegroup) {}
void PrintHardLinked::visit(FileGroup &filegroup)
{
	size_t size = filegroup.getSize();
	for (size_t i = 0;  i <size; ++i)
		for (size_t j = i + 1; j<size; ++j)
			outputHardLinked(filegroup[i]->data(), filegroup[j]->data(),
				filegroup.stat().st_nlink, filegroup.stat().st_size, sep);
}

void PrintHardLinked::visit(Filename &filename) {}
