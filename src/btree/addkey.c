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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * addkey.c -- Adds a new key to a BTREE
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    3.0.0 - 24 Sep 94
 *===========================================================*/

#include "standard.h"
#include "btree.h"

/*===========================
 * addkey -- Add key to BTREE
 *=========================*/
addkey (btree, ikey, rkey, fkey)
BTREE btree;  /*btree handle*/
FKEY ikey;    /*index file key -- index to add key to*/
RKEY rkey;    /*record key of new entry*/
FKEY fkey;    /*file key of new entry*/
{
	INDEX index;
	SHORT lo, hi;

/*wrintf("ADDKEY: ikey, rkey = %s, %s; ", fkey2path(ikey), rkey2str(rkey));
wrintf("fkey = %s\n", fkey2path(fkey));/*DEBUG*/
   /* Validate the operation */
	if (!bwrite(btree)) return;
	if ((index = getindex(btree, ikey)) == NULL)
		FATAL();
	if (nkeys(index) >= NOENTS - 1) FATAL();

   /* Search for record key in index -- shouldn't be there */
	lo = 1;
	hi = nkeys(index);
	while (lo <= hi) {
		SHORT md = (lo + hi)/2;
		INT rel = ll_strncmp(rkey.r_rkey, rkeys(index, md).r_rkey, 8);
		if (rel < 0)
			hi = --md;
		else if (rel > 0)
			lo = ++md;
		else
			FATAL();
	}

   /* Add new RKEY, FKEY pair to index */
	for (hi = nkeys(index); hi >= lo; hi--) {
		rkeys(index, hi+1) = rkeys(index, hi);
		fkeys(index, hi+1) = fkeys(index, hi);
	}
	rkeys(index, lo) = rkey;
	fkeys(index, lo) = fkey;
	nkeys(index)++;

   /* If index is now full split it */
	if (nkeys(index) >= NOENTS - 1) {
		INDEX newdex = crtindex(btree);
		SHORT n = NOENTS/2 - 1;
		nkeys(newdex) = nkeys(index) - n - 1;
		nkeys(index) = n;
		putindex(btree, index);
		for (lo = 0, hi = n + 1; hi < NOENTS; lo++, hi++) {
			rkeys(newdex, lo) = rkeys(index, hi);
			fkeys(newdex, lo) = fkeys(index, hi);
		}
		putindex(btree, newdex);

   /* Special case -- split requires new master index */
		if (iself(index) == iself(bmaster(btree))) {
			INDEX master = crtindex(btree);
			nkeys(master) = 1;
			fkeys(master, 0) = ikey;
			rkeys(master, 1) = rkeys(newdex, 0);
			fkeys(master, 1) = iself(newdex);
			newmaster(btree, master);
			writeindex(bbasedir(btree), master);
		} else	
			addkey(btree, iparent(index), rkeys(newdex, 0),
				iself(newdex));
	} else
		putindex(btree, index);
}
