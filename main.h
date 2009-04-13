
/* ************************************************************************ *
 * This is samefile driver. Programs can use this by including this source  *
 * file and implement the following functions:                              *
 * ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_MAIN_H
#define AK_MAIN_H

#include <new>

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <limits.h>
// #include <unistd.h>

// #include "configure.h"
// #include "toolkit.h"
// #include "visitor.h"
// #include "storage.h"
// #include "write2disk.h"
// #include "printhardlinked.h"
// #include "stats.h"
// #include "filename.h"
// #include "filegroup.h"
// #include "sizegroup.h"
#include "holder.h"

#define VERBOSE_LEVEL1		1
#define VERBOSE_LEVEL2		2
#define VERBOSE_LEVEL3		3
#define VERBOSE_MAX			3
#define VERBOSE_MASK		3
#define ADD_HARDLINKED		4
#define FULL_LIST			8
#define HUMAN_READABLE		16
#define MATCH_LEFT			32
#define MATCH_RIGHT			64
#define MATCH_TIME			128
#define REPORT_HARDLINKS	256
#define SKIP_SORT		512

#define MATCH_AUTO			(MATCH_LEFT | MATCH_RIGHT)
#define MATCH_MASK			(MATCH_LEFT | MATCH_RIGHT | MATCH_TIME)

#define S_ADD_HARDLINKED(m)		((m) & ADD_HARDLINKED)
#define S_FULL_LIST(m)			((m) & FULL_LIST)
#define S_HUMAN_READABLE(m)		((m) & HUMAN_READABLE)
#define S_MATCH(m)				((m) & (MATCH_LEFT | MATCH_RIGHT))
#define S_MATCH_LEFT(m)			((m) & MATCH_LEFT)
#define S_MATCH_RIGHT(m)		((m) & MATCH_RIGHT)
#define S_MATCH_TIME(m)			((m) & MATCH_TIME)
#define S_MATCH_MASK(m)			((m) & MATCH_MASK)
#define S_REPORT_HARDLINKS(m)	((m) & REPORT_HARDLINKS)
#define S_SKIP_SORT(m)			((m) & SKIP_SORT)
#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

class Filename;
class FileGroup;
class SizeGroup;
class Stats;

int processOptions(
	int argc, char **argv,
	void (&usage)(const char *),
	const char *version) throw();

void processInput(Stats &stats,
	int (&printFileCompare)(SizeGroup &parent, FileGroup &left,
		Filename &leftChild, FileGroup &right, Filename &rightChild,
		int result),
	int (&selectResults)(int flags, const char *sep),
	int (*preCheck)(const SizeGroup &,
		const FileGroup &, const FileGroup &) = NULL);

void processStats(Stats &stats);

#endif // AK_MAIN_H
