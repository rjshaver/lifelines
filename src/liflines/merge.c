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
 * merge.c -- Merge persons and families
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 13 Dec 94
 *   3.0.3 - 21 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

#include "llinesi.h"

extern BOOLEAN traditional;
extern STRING iredit, cfpmrg, nopmrg, noqmrg, noxmrg, nofmrg;
extern STRING dhusb,  dwife,  cffmrg, fredit, badata, ronlym;

STRING mgsfam = (STRING) "These persons are children in different families.";
STRING mgconf = (STRING) "Are you sure you want to merge them?";

static void merge_fam_links(NODE, NODE, NODE, NODE, INT);
static NODE remove_dupes(NODE, NODE);
static NODE sort_children(NODE, NODE);

/*================================================================
 * merge_two_indis -- Merge first person to second; data from both
 *   are put in file that user edits; first person removed
 *---------------------------------------------------------------
 *   indi1 - person in database -- indi1 is merged into indi2
 *   indi2 - person in database -- indi1 is merged into this person
 *   indi3 - merged version of the two persons before editing
 *   indi4 - merged version of the two persons after editing
 *==============================================================*/
NODE
merge_two_indis (NODE indi1,    /* two persons to merge - can't be null */
                 NODE indi2,
                 BOOLEAN conf)          /* have user confirm change */
{
	NODE indi01, indi02;	/* original arguments */
	NODE name1, refn1, sex1, body1, famc1, fams1;
	NODE name2, refn2, sex2, body2, famc2, fams2;
	NODE indi3, name3, refn3, sex3, body3, famc3, fams3;
	NODE indi4;
	NODE fam, husb, wife, chil, rest, fref, keep=NULL;
	NODE this, that, prev, next, node, head;
	NODE fam12, name12, refn12;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	FILE *fp;
	INT sx2;
	STRING msg, key;
 	BOOLEAN emp;

/* Do start up checks */

	ASSERT(indi1 && indi2);
	ASSERT(eqstr("INDI", ntag(indi1)));
	ASSERT(eqstr("INDI", ntag(indi2)));
	if (readonly) {
		message(ronlym);
		return NULL;
	}
	if (indi1 == indi2) {
		message(nopmrg);
		return NULL;
	}

/* Check restrictions on persons */

	famc1 = FAMC(indi1);
	famc2 = FAMC(indi2);
/*LOOSEEND -- THIS CHECK IS NOT GOOD ENOUGH */
	if (traditional) {
		if (famc1 && famc2 && nestr(nval(famc1), nval(famc2))) {
			if (!ask_yes_or_no_msg(mgsfam, mgconf)) {
				message(noqmrg);
				return NULL;
			}
		}
	}
	fams1 = FAMS(indi1);
	fams2 = FAMS(indi2);
	if (fams1 && fams2 && SEX(indi1) != SEX(indi2)) {
		message(noxmrg);
		return NULL;
	}

/* Split original persons */

	/* If we are successful, the original indi1 will be deleted
 	 * and the indi2 will be updated with the new info.
   	 * However, if we do not merge then we must
	 * insure that indi1 and indi2 are in their original state.
	 * It seems safer to do this by only working with copies
    	 * of the originals.
	 */
	indi01 = indi1;	/* keep original indi1 for later delete */
	indi1 = copy_nodes(indi1, TRUE, TRUE);
	indi02 = indi2;	/* keep original indi2 for later update and return */
	indi2 = copy_nodes(indi2, TRUE, TRUE);

	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	split_indi(indi2, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	indi3 = indi2; 
	indi2 = copy_nodes(indi2, TRUE, TRUE);
	sx2 = SEX_UNKNOWN;
	if (fams1) sx2 = val_to_sex(sex1);
	if (fams2) sx2 = val_to_sex(sex2);

/*CONDITION: 1s, 2s - build first version of merged person */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	name3 = union_nodes(name1, name2, TRUE, TRUE);
	refn3 = union_nodes(refn1, refn2, TRUE, TRUE);
	sex3  = union_nodes(sex1,  sex2,  TRUE, TRUE);
	body3 = union_nodes(body1, body2, TRUE, TRUE);
	famc3 = union_nodes(famc1, famc2, TRUE, TRUE);
	fams3 = union_nodes(fams1, fams2, TRUE, TRUE);
	write_nodes(0, fp, tto, indi3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, name3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, refn3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, sex3,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, body3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, famc3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fams3, TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi3, name3, refn3, sex3, body3, famc3, fams3);

/*CONDITION 2 -- 3 (init combined) created and joined*/

/* Have user edit merged person */
	do_edit();
	while (TRUE) {
		indi4 = file_to_node(editfile, tti, &msg, &emp);
		if (!indi4 && !emp) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_indi(indi4, &msg, indi3)) {
			if (ask_yes_or_no_msg(msg, iredit)) {
				do_edit();
				continue;
			}
			free_nodes(indi4);
			indi4 = NULL;
			break;
		}
		break;
	}
	free_nodes(indi3);

/* Have user confirm changes */
	if (!indi4 || (conf && !ask_yes_or_no(cfpmrg))) {
		if (indi4) free_nodes(indi4);
		join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
		free_nodes(indi1);
		join_indi(indi2, name2, refn2, sex2, body2, famc2, fams2);
		free_nodes(indi2);
		/* originals (indi01 and indi02) have not been modified */
		return NULL;
	}

/* Modify families that have persons as children */

	classify_nodes(&famc1, &famc2, &fam12);

/* Both were children in same family; remove first as child */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				keep = nchild(that);
				free_node(that);
				if (!prev)
					chil = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		that = chil;
		while (keep && that) {
			if (eqstr(nval(that), nxref(indi2))) {
				nchild(that) = union_nodes(nchild(that),
					keep, TRUE, FALSE);
			}
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/*HERE*/
/* Only first was child; make family refer to second */

	this = famc1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	free_nodes(fam12);

/*HERE*/
/* Modify families that had persons as spouse */

	classify_nodes(&fams1, &fams2, &fam12);

/* Both were parents in same family; remove first as parent */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		if (sx2 == SEX_MALE)
			head = that = husb;
		else
			head = that = wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				free_nodes(that);
				if (!prev)
					prev = head = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		if (sx2 == SEX_MALE)
			husb = head;
		else
			wife = head;
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/*HERE*/
/* Only first was parent; make family refer to second */

	this = fams1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = (sx2 == SEX_MALE) ? husb : wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		join_fam(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	free_nodes(fam12);

/*HERE*/

	classify_nodes(&name1, &name2, &name12);
	classify_nodes(&refn1, &refn2, &refn12);

	key = rmvat(nxref(indi4));
	for (node = name1; node; node = nsibling(node))
		add_name(nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	resolve_links(indi4);
	indi_to_dbase(indi4);
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	free_nodes(indi1);
	join_indi(indi2, name2, refn2, sex2, body2, famc2, fams2);
	free_nodes(indi2);
	free_nodes(name12);
	free_nodes(refn12);

	/* update indi02 to contain info from new merged record in indi4 */

	split_indi(indi4, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	split_indi(indi02, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	join_indi(indi4, name2, refn2, sex2, body2, famc2, fams2);
	join_indi(indi02, name1, refn1, sex1, body1, famc1, fams1);
	free_nodes(indi4);

/* BUG!
  Nodes in indi02 that were edited out of indi4 are not deleted.
  For example, orphaned NAMEs from indi2 are now ghost names
  pointing at indi4. If indi4 is deleted, the ghost names will
  remain, and will point at whatever record is later put into
  the key# of indi4.
  Perry, 2000/11/16
*/

	delete_indi(indi01, FALSE);	/* this is the original indi1 */
	return indi02;			/* this is the updated indi2 */
}
/*=================================================================
 * merge_two_fams -- Merge first family into second; data from both
 *   are put in file that user edits; first family removed
 *===============================================================*/
NODE
merge_two_fams (NODE fam1,
                NODE fam2)
{
	NODE husb1, wife1, chil1, rest1, husb2, wife2, chil2, rest2;
	NODE fref1, fref2;
	NODE fam3, husb3, wife3, chil3, rest3, fref3;
	NODE fam4, husb4, wife4, chil4, rest4, fref4;
	TRANTABLE tti = tran_tables[MEDIN], tto = tran_tables[MINED];
	FILE *fp;
	STRING msg;
	BOOLEAN emp;

	if (readonly) {
		message(ronlym);
		return NULL;
	}
	ASSERT(fam1 && fam2);
	ASSERT(eqstr("FAM", ntag(fam1)));
	ASSERT(eqstr("FAM", ntag(fam2)));
	if (fam1 == fam2) {
		message(nofmrg);
		return NULL;
	}

/* Check restrictions on families */
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	split_fam(fam2, &fref2, &husb2, &wife2, &chil2, &rest2);
#if 0
	if (husb1 && husb2 && nestr(nval(husb1), nval(husb2))) {
		message(dhusb);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
	if (wife1 && wife2 && nestr(nval(wife1), nval(wife2))) {
		message(dwife);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
#endif

/* Create merged file with both families together */
	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	fam3 = copy_nodes(fam2, TRUE, TRUE);
	fref3 = union_nodes(fref1, fref2, TRUE, TRUE);
	husb3 = union_nodes(husb1, husb2, TRUE, TRUE);
	wife3 = union_nodes(wife1, wife2, TRUE, TRUE);
	rest3 = union_nodes(rest1, rest2, TRUE, TRUE);
	chil3 = sort_children(chil1, chil2);
	write_nodes(0, fp, tto, fam3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, fref3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, rest3, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, chil3, TRUE, TRUE, TRUE);
	fclose(fp);

/* Have user edit merged family */
	join_fam(fam3, fref3, husb3, wife3, rest3, chil3);
	do_edit();
	while (TRUE) {
		fam4 = file_to_node(editfile, tti, &msg, &emp);
		if (!fam4 && !emp) {
			if (ask_yes_or_no_msg(msg, fredit)) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_fam(fam4, &msg, fam3)) {
			if (ask_yes_or_no_msg(badata, iredit)) {
				do_edit();
				continue;
			}
			free_nodes(fam4);
			fam4 = NULL;
			break;
		}
		break;
	}
	free_nodes(fam3);

/* Have user confirm changes */

	if (!fam4 || !ask_yes_or_no(cffmrg)) {
		if (fam4) free_nodes(fam4);
		join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
		join_fam(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
	split_fam(fam4, &fref4, &husb4, &wife4, &chil4, &rest4);

 /* Modify links between persons and families */
#define CHUSB 1
#define CWIFE 2
#define CCHIL 3
	merge_fam_links (fam1, fam2, husb1, husb2, CHUSB);
	merge_fam_links (fam1, fam2, wife1, wife2, CWIFE);
	merge_fam_links (fam1, fam2, chil1, chil2, CCHIL);

/* Update database with second family; remove first */
	join_fam(fam4, fref2, husb2, wife2, chil2, rest2);
	free_nodes(fam4);
	nchild(fam1) = NULL;
	delete_fam(fam1);
	free_nodes(husb1);
	free_nodes(wife1);
	free_nodes(chil1);
	free_nodes(rest1);
	join_fam(fam2, fref4, husb4, wife4, chil4, rest4);
	resolve_links(fam2);
	fam_to_dbase(fam2);
	return fam2;
}
/*================================================================
 * merge_fam_links -- Shift links of persons in list1 from fam1 to
 *   fam2.  List1 holds the persons that refer to fam1, and list2
 *   holds the persons who refer to fam2.  If a person is on both
 *   lists, the reference in the person to the fam1 is removed from
 *   the person.  If a person is only on list1, the reference to fam1
 *   is changed to refer to fam2.  No changes are made for persons
 *   only on list2.  No changes are made to the references from the
 *   families to the persons.
 *================================================================*/
void
merge_fam_links (NODE fam1,
                 NODE fam2,
                 NODE list1,
                 NODE list2,
                 INT code)
{
	NODE curs1, curs2, prev, this, next, first, keep=NULL;
	NODE indi, name, refn, sex, body, famc, fams;

	curs1 = list1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		indi = key_to_indi(rmvat(nval(curs1)));
		split_indi(indi, &name, &refn, &sex, &body, &famc, &fams);
		prev = NULL;
		if (code == CHUSB || code == CWIFE)
			first = this = fams;
		else
			first = this = famc;

/* Both fams linked to this indi; remove link in indi to first */

		if (curs2) {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					next = nsibling(this);
					nsibling(this) = NULL;
					keep = nchild(this);
					free_node(this);
					if (!prev)
						first = next;
					else
						nsibling(prev) = next;
					this = next;
				} else {
					prev = this;
					this = nsibling(this);
				}
			}
			this = first;
			while (keep && this) {
				if (eqstr(nval(this), nxref(fam2))) {
					nchild(this) =
					    union_nodes(nchild(this), keep,
					    TRUE, FALSE);
/*HERE*/
				}
				this = nsibling(this);
			}

/* Only first fam linked with this indi; move link to second */

		} else {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					stdfree(nval(this));
					nval(this) = strsave(nxref(fam2));
				}
				prev = this;
				this = nsibling(this);
			}
		}
		if (code == CHUSB || code == CWIFE)
			fams = first;
		else
			famc = first;
		join_indi(indi, name, refn, sex, body, famc, fams);
		indi_to_dbase(indi);
		curs1 = nsibling(curs1);
	}
}
/*================================================
 * sort_children -- Return sorted list of children
 *==============================================*/
static NODE
sort_children (NODE chil1,
               NODE chil2)
{
	NODE copy1, copy2, chil3, prev, kid1, kid2;
	STRING year1, year2;
	INT int1, int2;
	copy1 = remove_dupes(chil1, chil2);
	copy2 = copy_nodes(chil2, TRUE, TRUE);
	int1 = int2 = 1;
	prev = chil3 = NULL;
	while (copy1 && copy2) {
		if (int1 == 1) {
			kid1 = key_to_indi(rmvat(nval(copy1)));
			year1 = event_to_date(BIRT(kid1), NULL, TRUE);
			if (!year1)
				year1 = event_to_date(BAPT(kid1), NULL, TRUE);
			int1 = year1 ? atoi(year1) : 0;
		}
		if (int2 == 1) {
			kid2 = key_to_indi(rmvat(nval(copy2)));
			year2 = event_to_date(BIRT(kid2), NULL, TRUE);
			if (!year2)
				year2 = event_to_date(BAPT(kid2), NULL, TRUE);
			int2 = year2 ? atoi(year2) : 0;
		}
		if (int1 < int2) {
			if (!prev)
				prev = chil3 = copy1;
			else
				prev = nsibling(prev) = copy1;
			copy1 = nsibling(copy1);
			int1 = 1;
		} else {
			if (!prev)
				prev = chil3 = copy2;
			else
				prev = nsibling(prev) = copy2;
			copy2 = nsibling(copy2);
			int2 = 1;
		}
	}
	if (copy1) {
		if (!prev)
			chil3 = copy1;
		else
			nsibling(prev) = copy1;
	}
	if (copy2) {
		if (!prev)
			chil3 = copy2;
		else
			nsibling(prev) = copy2;
	}
	return chil3;
}
/*=================================================
 * remove_dupes -- Return all in list1 not in list2
 *===============================================*/
static NODE
remove_dupes (NODE list1,
              NODE list2)
{
	NODE copy1 = copy_nodes(list1, TRUE, TRUE);
	NODE prev1, next1, curs1, curs2;
	prev1 = NULL;
	curs1 = copy1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		if (curs2) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			free_nodes(curs1);
			if (!prev1)
				copy1 = next1;
			else
				nsibling(prev1) = next1;
			curs1 = next1;
		} else {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
	}
	return copy1;
}
