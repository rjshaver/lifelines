/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * strcvt.c -- string conversion functions
 *===========================================================*/

#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include "bfs.h"
#include "icvt.h"

/*===================================================
 * makewide -- convert internal to wchar_t
 * handling annoyance that MS-Windows wchar_t is small
 * This only succeeds if internal charset is UTF-8
 * Either returns 0 if fails, or a bfstr which actually
 * contains wchar_t characters.
 *=================================================*/
bfptr
makewide (const char *str)
{
	bfptr bfs=0;
#ifdef HAVE_WCSCOLL
	if (uu8) {
#ifdef _WIN32
		/* MS-Windows can't handle UCS-4; could we use UTF-16 ? */
		CNSTRING dest = "UCS-2-INTERNAL";
#else
		CNSTRING dest = "UCS-4-INTERNAL";
#endif
		BOOLEAN success;
		bfs = bfNew(strlen(str)*4+3);
		bfCpy(bfs, str);
		bfs = iconv_trans("UTF-8", dest, bfs, "?", &success);
		if (!success) {
			bfDelete(bfs);
			bfs = 0;
		}

		success = TRUE;
	}
#else
	str=str; /* unused */
#endif /* HAVE_WCSCOLL */

	return bfs;
}
/*=========================================
 * isnumeric -- Check string for all digits
 * TODO: convert to Unicode -- but must find where we make
 * numeric equivalent & convert it as well
 *=======================================*/
BOOLEAN
isnumeric (STRING str)
{
	INT c;
	if (!str) return FALSE;
	while ((c = (uchar)*str++)) {
#ifndef OS_NOCTYPE
		if (chartype(c) != DIGIT) return FALSE;
#else
		if (!isdigit(c)) return FALSE;
#endif
	}
	return TRUE;
}
/*======================================
 * lower -- Convert string to lower case
 *  returns static buffer
 *  TODO: convert to Unicode
 *====================================*/
STRING
lower (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
	while ((c = (uchar)*str++) && (++i < MAXLINELEN+1))
		*p++ = ll_tolower(c);
	*p = '\0';
	return scratch;
}
/*======================================
 * upper -- Convert string to upper case
 *  returns static buffer
 *  TODO: convert to Unicode
 *====================================*/
STRING
upper (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
	while ((c = (uchar)*str++) && (++i < MAXLINELEN+1))
		*p++ = ll_toupper(c);
	*p = '\0';
	return scratch;
}
/*================================
 * capitalize -- Capitalize string
 *  returns static buffer (borrowed from lower)
 *  TODO: convert to Unicode
 *==============================*/
STRING
capitalize (STRING str)
{
	STRING p = lower(str);
	*p = ll_toupper((uchar)*p);
	return p;
}
/*================================
 * titlecase -- Titlecase string
 * Created: 2001/12/30 (Perry Rapp)
 *  returns static buffer (borrowed from lower)
 *  TODO: convert to Unicode
 *==============================*/
STRING
titlecase (STRING str)
{
	/* % sequences aren't a problem, as % isn't lower */
	STRING p = lower(str), buf=p;
	if (!p[0]) return p;
	while (1) {
		/* capitalize first letter of word */
		*p = ll_toupper((uchar)*p);
		/* skip to end of word */
		while (*p && !iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
		/* skip to start of next word */
		while (*p && iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
	}
}