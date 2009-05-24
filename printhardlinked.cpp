
/* ************************************************************************ *
 *            Written by Alex de Kruijff           21 April 2009            *
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

int PrintHardLinked::visit(Holder &holder) {return 0; }
int PrintHardLinked::visit(SizeGroup &sizegroup) {return 0; }
int PrintHardLinked::visit(FileGroup &filegroup)
{
	size_t n = filegroup.getBoundry();
	for (size_t i = 0;  i < n; ++i) if (filegroup[i] != NULL)
		for (size_t j = i + 1; j < n; ++j) if (filegroup[j] != NULL)
			outputHardLinked(filegroup[i]->data(), filegroup[j]->data(),
				filegroup.stat().st_nlink, filegroup.stat().st_size, sep);
	return 1;
}

void PrintHardLinked::visit(Filename &filename) {}
