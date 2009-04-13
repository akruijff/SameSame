
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_PRINTHARDLINKED_H
#define AK_PRINTHARDLINKED_H

#include "visitor.h"

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

/**
 * Prints all the filenames that are hard linked.
 */
class PrintHardLinked : public SamefileVisitor
{
	const char *sep;
public:
	PrintHardLinked(const char *sep) { this->sep = sep; }
	void visit(Holder &);
	void visit(SizeGroup &);
	void visit(FileGroup &);
	void visit(Filename &);
};

#endif // AK_PRINTHARDLINKED_H
