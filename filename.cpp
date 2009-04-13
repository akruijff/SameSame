
/* ************************************************************************ *
 *            Written by Alex de Kruijff           14 April 2009            *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include <string.h>

#include "filename.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef WITH_REGEX
static const char *empty = "";
#endif

Filename::Filename(size_t size)
{
	this->str = new char[++size];
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	this->str[0] = 0;
}

Filename::Filename(const char *str)
{
#ifdef DEBUG
	if (str == NULL)
	{
		fprintf(stderr, "%s:%d str shouldn't be NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	size_t len = strlen(str);
	this->str = new char[++len];
#ifdef DEBUG
	if (len == 0)
	{
		fprintf(stderr, "%s:%d len shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	memcpy(this->str, str, len);
}

void Filename::resize(size_t size)
{
	size_t len = strlen(str);
	if (size < len)
		size = len;
	char *tmp = new char[++size];
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	memcpy(tmp, str, ++len);
	delete[] str;
	str = tmp;
}

void Filename::renew(size_t size)
{
	char *tmp = new char[++size];
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	delete[] str;
	str = tmp;
	str[0] = 0;
}
