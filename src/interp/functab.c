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
 * functab.c -- Table of builtin functions
 * Copyright(c) 1993-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 26 Sep 93
 *   2.3.6 - 01 Jan 94    3.0.0 - 11 Oct 94
 *   3.0.3 - 02 Jul 96
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-01-28 J.F.Chandler */

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"

/*
 * The entries of builtins MUST be in alphabetic order (because an
 * alphabetic binary sort is performed directly on this list).
 */
BUILTINS builtins[] = {
	{"add",             2,	32,	__add},
	{"addnode",         3,	3,	__addnode},
	{"addtoset",        3,	3,	__addtoset},
	{"alpha",           1,	1,	___alpha},
	{"ancestorset",     1,	1,	__ancestorset},
	{"and",             2,	32,	__and},
	{"atoi",            1,	1,	__strtoint},
	{"baptism",         1,	1,	__bapt},
	{"birth",           1,	1,	__birt},
	{"burial",          1,	1,	__buri},
	{"capitalize",      1,	1,	__capitalize},
	{"card",            1,	1,	__card},
	{"child",           1,	1,	__child},
	{"childset",        1,	1,	__childset},
	{"choosechild",     1,	1,	__choosechild},
	{"choosefam",       1,	1,	__choosefam},
	{"chooseindi",      1,	1,	__chooseindi},
	{"choosespouse",    1,	1,	__choosespouse},
	{"choosesubset",    1,	1,	__choosesubset},
	{"col",             1,	1,	__col},
	{"complexdate",	    1,	1,	__complexdate},
	{"concat",          1,	32,	__concat},
	{"copyfile",        1,	1,	__copyfile},
	{"createnode",      2,	2,	__createnode},
	{"d",               1,	1,	__d},
	{"database",        0,	1,	__database},
	{"date",            1,	1,	__date},
	{"dateformat",      1,	1,	__dateformat},
	{"dayformat",       1,	1,	__dayformat},
	{"death",           1,	1,	__deat},
	{"debug",           1,	1,	__debug},
	{"decr",            1,	1,	__decr},
	{"deletefromset",   3,	3,	__deletefromset},
	{"deletenode",      1,	1,	__deletenode},
	{"dequeue",         1,	1,	__dequeue},
	{"dereference",	    1,	1,	__getrecord},
	{"descendantset",   1,	1,	__descendentset},
	{"descendentset",   1,	1,	__descendentset},
	{"difference",      2,	2,	__difference},
	{"div",             2,	2,	__div},
	{"empty",           1,	1,	__empty},
	{"enqueue",         2,	2,	__enqueue},
	{"eq",              2,	2,	__eq},
	{"eqstr",           2,	2,	__eqstr},
	{"exp",             2,	2,	__exp},
	{"extractdate",     4,	4,	__extractdate},
	{"extractdatestr",  5,	6,	__extractdatestr},
	{"extractnames",    4,	4,	__extractnames},
	{"extractplaces",   3,	3,	__extractplaces},
	{"extracttokens",   4,	4,	__extracttokens},
	{"f",               1,	2,	__f},
	{"fam",             1,	1,	__fam},
	{"father",          1,	1,	__fath},
	{"female",          1,	1,	__female},
	{"firstchild",      1,	1,	__firstchild},
	{"firstfam",        0,	0,	__firstfam},
	{"firstindi",       0,	0,	__firstindi},
	{"fnode",           1,	1,	__fnode},
	{"free",            1,	1,	__free},
	{"fullname",        4,	4,	__fullname},
	{"ge",              2,	2,	__ge},
	{"gengedcom",       1,	1,	__gengedcom},
	{"gengedcomstrong", 1,	1,	__gengedcomstrong},
	{"gengedcomweak",   1,	1,	__gengedcomweak},
	{"genindiset",      2,	2,	__genindiset},
	{"getcol",          0,	0,	__getcol},
	{"getel",           2,	2,	__getel},
	{"getfam",          1,	1,	__getfam},
	{"getindi",         1,	2,	__getindi},
	{"getindimsg",      2,	2,	__getindi},
	{"getindiset",      1,	2,	__getindiset},
	{"getint",          1,	2,	__getint},
	{"getintmsg",       2,	2,	__getint},
	{"getproperty",     1,	1,	__getproperty},
	{"getrecord",       1,	1,	__getrecord},
	{"getstr",          1,	2,	__getstr},
	{"getstrmsg",       2,	2,	__getstr},
	{"gettoday",        0,	0,	__gettoday},
	{"givens",          1,	1,	__givens},
	{"gt",              2,	2,	__gt},
	{"heapused",        0,	0,	__heapused},
	{"husband",         1,	1,	__husband},
	{"incr",            1,	1,	__incr},
	{"index",           3,	3,	__index},
	{"indi",            1,	1,	__indi},
	{"indiset",         1,	1,	__indiset},
	{"inlist",          2,	2,	__inlist},
	{"inode",           1,	1,	__inode},
	{"insert",          3,	3,	__insert},
	{"inset",           2,	2,	__inset},
	{"intersect",       2,	2,	__intersect},
	{"key",             1,	2,	__key},
	{"keysort",         1,	1,	__keysort},
	{"lastchild",       1,	1,	__lastchild},
	{"lastfam",         0,	0,	__lastfam},
	{"lastindi",        0,	0,	__lastindi},
	{"le",              2,	2,	__le},
	{"length",          1,	1,	__length},
	{"lengthset",       1,	1,	__lengthset},
	{"level",           1,	1,	__level},
	{"linemode",        0,	0,	__linemode},
	{"list",            1,	1,	__list},
	{"lock",            1,	1,	__lock},
	{"long",            1,	1,	__long},
	{"lookup",          2,	2,	__lookup},
	{"lower",           1,	1,	__lower},
	{"lt",              2,	2,	__lt},
	{"male",            1,	1,	__male},
	{"marriage",        1,	1,	__marr},
	{"menuchoose",      1,	2,	__menuchoose},
	{"mod",             2,	2,	__mod},
	{"monthformat",     1,	1,	__monthformat},
	{"mother",          1,	1,	__moth},
	{"mul",             2,	32,	__mul},
	{"name",            1,	2,	__name},
	{"namesort",        1,	1,	__namesort},
	{"nchildren",       1,	1,	__nchildren},
	{"ne",              2,	2,	__ne},
	{"neg",             1,	1,	__neg},
	{"nestr",           2,	2,	__nestr},
	{"newfile",         2,	2,	__newfile},
	{"nextfam",         1,	1,	__nextfam},
	{"nextindi",        1,	1,	__nextindi},
	{"nextsib",         1,	1,	__nextsib},
	{"nfamilies",       1,	1,	__nfamilies},
	{"nl",              0,	0,	__nl},
	{"not",             1,	1,	__not},
	{"nspouses",        1,	1,	__nspouses},
	{"or",              2,	32,	__or},
	{"ord",             1,	1,	__ord},
	{"originformat",    1,	1,	__originformat},
	{"outfile",         0,	0,	__outfile},
	{"pagemode",        2,	2,	__pagemode},
	{"pageout",         0,	0,	__pageout},
	{"parent",          1,	1,	__parent},
	{"parents",         1,	1,	__parents},
	{"parentset",       1,	1,	__parentset},
	{"place",           1,	1,	__place},
	{"pn",              2,	2,	__pn},
	{"pop",             1,	1,	__pop},
	{"pos",             2,	2,	__pos},
	{"prevfam",         1,	1,	__prevfam},
	{"previndi",        1,	1,	__previndi},
	{"prevsib",         1,	1,	__prevsib},
	{"print",           1,	32,	__print},
	{"program",         0,	0,	__program},
	{"push",            2,	2,	__push},
	{"pvalue",          1,	1,	__pvalue},
	{"qt",              0,	0,	__qt},
	{"reference",       1,	1,	__reference},
	{"requeue",         2,	2,	__requeue},
	{"rjustify",        2,	2,	__rjustify},
	{"roman",           1,	1,	__roman},
	{"root",            1,	1,	__rot},
	{"row",             1,	1,	__row},
	{"save",            1,	1,	__save},
	{"savenode",        1,	1,	__savenode},
	{"set",             2,	2,	__set},
	{"setel",           3,	3,	__setel},
	{"sex",             1,	1,	__sex},
	{"short",           1,	1,	__short},
	{"sibling",         1,	1,	__sibling},
	{"siblingset",      1,	1,	__siblingset},
	{"soundex",         1,	1,	__soundex},
	{"sp",              0,	0,	__space},
	{"spouseset",       1,	1,	__spouseset},
	{"stddate",         1,	1,	__stddate},
	{"strcmp",          2,	2,	__strcmp},
	{"strconcat",       1,	32,	__concat},
	{"strlen",          1,	1,	__strlen},
	{"strsave",         1,	1,	__save},
	{"strsoundex",      1,	1,	__strsoundex},
	{"strtoint",        1,	1,	__strtoint},
	{"sub",             2,	2,	__sub},
	{"substring",       3,	3,	__substring},
	{"surname",         1,	1,	__surname},
	{"system",          1,	1,	__runsystem},
	{"table",           1,	1,	__table},
	{"tag",             1,	1,	__tag},
	{"title",           1,	1,	__titl},
	{"trim",            2,	2,	__trim},
	{"trimname",        2,	2,	__trimname},
	{"union",           2,	2,	__union},
	{"uniqueset",       1,	1,	__uniqueset},
	{"unlock",          1,	1,	__unlock},
	{"upper",           1,	1,	__upper},
	{"value",           1,	1,	__value},
	{"valuesort",       1,	1,	__valuesort},
	{"version",         0,	0,	__version},
	{"wife",            1,	1,	__wife},
	{"writefam",        1,	1,	__writefam},
	{"writeindi",       1,	1,	__writeindi},
	{"xref",            1,	1,	__xref},
	{"year",            1,	1,	__year},
	{"yearformat",      1,	1,	__yearformat},
};

INT nobuiltins = ARRSIZE(builtins);
