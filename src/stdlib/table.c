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
 * table.c -- Hash table operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 04 Sep 94    3.0.2 - 22 Dec 94
 *   3.0.3 - 19 Sep 95
 *===========================================================*/

#include "standard.h"
#include "table.h"

/*======================
 * hash -- Hash function
 *====================*/
static INT hash (key)
STRING key;
{
	INT hval = 0;
	while (*key)
		hval += *key++;
	hval %= MAXHASH;
	if(hval < 0) FATAL();
	if(hval >= MAXHASH) FATAL();
	return hval;
}
/*================================
 * fndentry -- Find entry in table
 *==============================*/
static ENTRY fndentry (tab, key)
TABLE tab;
STRING key;
{
	ENTRY entry;
	if (!tab || !key) return NULL;
	entry = tab[hash(key)];
	while (entry) {
		if (eqstr(key, (char*)entry->ekey)) return entry;
		entry = entry->enext;
	}
	return NULL;
}
/*=============================
 * create_table -- Create table
 *===========================*/
TABLE create_table (void)
{
	TABLE tab = (TABLE) stdalloc(MAXHASH*sizeof(ENTRY));
	INT i;
	for (i = 0; i < MAXHASH; i++)
		tab[i] = NULL;
	return tab;
}
/*======================================
 * insert_table -- Insert entry in table
 *====================================*/
void insert_table (tab, key, val)
TABLE tab;
STRING key;
WORD val;
{
	ENTRY entry = fndentry(tab, key);
	if (entry)
		entry->evalue = val;
	else {
		INT hval = hash(key);
		entry = (ENTRY) stdalloc(sizeof(*entry));
		entry->ekey = key;
		entry->evalue = val;
		entry->enext = tab[hval];
		tab[hval] = entry;
	}
}
/*==========================================
 * delete_table -- Remove element from table
 *========================================*/
void delete_table (tab, key)
TABLE tab;
STRING key;
{
	INT hval = hash(key);
	ENTRY prev = NULL;
	ENTRY this = tab[hval];
	while (this && nestr(key, this->ekey)) {
		prev = this;
		this = this->enext;
	}
	if (!this) return;
	if (prev)
		prev->enext = this->enext;
	else
		tab[hval] = this->enext;
	stdfree(this);
}
/*======================================
 * in_table() - Check for entry in table
 *====================================*/
BOOLEAN in_table (tab, key)
TABLE tab;
STRING key;
{
	return fndentry(tab, key) != NULL;
}
/*===============================
 * valueof -- Find value of entry
 *=============================*/
WORD valueof (tab, key)
TABLE tab;
STRING key;
{
	ENTRY entry;
	if (!key) return NULL;
	if ((entry = fndentry(tab, key)))
		return entry->evalue;
	else
		return NULL;
}
/*===================================
 * valueofbool -- Find value of entry
 *=================================*/
WORD valueofbool (tab, key, there)
TABLE tab;
STRING key;
BOOLEAN *there;
{
	ENTRY entry;
	*there = FALSE;
	if (!key) return NULL;
	if ((entry = fndentry(tab, key))) {
		*there = TRUE;
		return entry->evalue;
	}
	return NULL;
}
/*=============================
 * remove_table -- Remove table
 *===========================*/
void remove_table (tab, rcode)
TABLE tab;
INT rcode;
{
	INT i;
	ENTRY ent, nxt;
	if (!tab) return;
	for (i = 0; i < MAXHASH; i++) {
		nxt = tab[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			switch (rcode) {
			case FREEBOTH:
				stdfree(ent->evalue);
			case FREEKEY:
				stdfree(ent->ekey);
				break;
			case FREEVALUE:
				stdfree(ent->evalue);
				break;
			}
			stdfree(ent);
		}
	}
	stdfree(tab);
}
/*=================================================
 * traverse_table -- Traverse table doing something
 *===============================================*/
void traverse_table (tab, tproc)
TABLE tab;
INT (*tproc)();
{
	INT i;
	ENTRY ent, nxt;
	if (!tab || !tproc) return;
	for (i = 0; i < MAXHASH; i++) {
		nxt = tab[i];
		while ((ent = nxt)) {
			nxt = ent->enext;
			(*tproc)(ent);
		}
	}
}
