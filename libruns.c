/*
 * libruns.c: This file contains the functions to actually find runs of
 * similarity by reading in TDN nodes from the database and matching them
 * against the in-memory TDNs in the TDN groups.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.42 $
 */
#undef DEBUG

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "libctf.h"
#include "libtokens.h"
#include "libtdn.h"

extern Ctfhandle *ctf_handle[];	/* Array of CTF handles */
extern int ctflistnext;

/*
 * We have two Run linked lists: one is the list of incomplete runs.
 * The other is the list of completed runs.
 */
Run *inc_runlist = NULL;	/* Incomplete run list */
Run *done_runhead = NULL;	/* Complete run list */

int any_tdngrps = 0;		/* Have we got any tdngrps yet? */

#define BITSINTABLE     24	/* 24-bit table */
#define TABLE_SIZE (1 << BITSINTABLE)

/* We keep a lookup table to quickly search for runs which
 * can be extended when we find a tuple match on a TDN.
 */
Run *runLUT[TABLE_SIZE];

/* Isomorphic comparison tables.
 * We need two tables: one to record the match from a dst value
 * to a src value, and another to record the src to dst value match.
 * Here are some comments on the isomorph[] arrays. We would like to
 * identify code fragments that are similar, even if variables have been
 * renamed or even declared in a different order. For example, the
 * following two functions are essentially identical.
 * 
 * int maxofthree(int x, int y, int z)    int bigtriple(int a, int b, int c) {
 * if ((x>y) && (x>z)) return(x);       {  if ((a>b) && (a>c)) return(a);
 * if (y>z) return(y);                     if (b>c) return(b); return(z);
 * return(c); }                                      }
 * 
 * The tokenised forms of these functions are obviously different. However, it
 * is possible to compare these and show that they are identical. If we
 * start with the first "if" statement, a straight lexical comparison shows
 * that they are different. But at the first difference (id1123 cf. id456),
 * we put an entry into the isodtos[] array so that isodtos[456]= 123, i.e.
 * the identifier #456 on the RHS is isomorphic to identifier #123 on the
 * LHS, and vice versa. Thus when we get to the first "return" statement,
 * we can see that they are also isomorphic. We can thus conclude that
 * everything from the "if" down to the "}" is isomorphic and thus
 * identical.
 */
uint16_t isodtos[65536];	/* Destination to source isomorphism */
uint16_t isostod[65536];	/* Source to destination isomorphism */
uint16_t isoseen[65536];	/* =1 if we have seen this id value */
int max_isoseen = 0;		/* Number of relationships seen */

/* This function is used to create a hash value for two TDNs in
 * memory, so that we can quickly find any possible runs which
 * can be extended.
 */
static inline int runhash(TDN * a, TDN * b)
{
  return(((int)a >> 3 ^ (int)b) & 0xffffff);
}

void clear_isomorph_arrays(void)
{
  /*
   * Clear the identifer isomorph table for this run. I used to simply
   * memset() both isodtos[] and isostod[], but it is faster to track and
   * delete only those entries that we used.
   */
  while (--max_isoseen >= 0) {
    isodtos[isoseen[max_isoseen]] = 0;
    isostod[isoseen[max_isoseen]] = 0;
  }
  max_isoseen = 0;
}

void clear_donelist()
{
#ifdef FREE_MEM
  Run *run, *nextrun;
  for (run = done_runhead; run != NULL; run = nextrun) {
    nextrun = run->next; free(run);
  }
#endif
  done_runhead = NULL;
}

void clear_inclist()
{
#ifdef FREE_MEM
  Run *run, *nextrun;
  for (run = inc_runlist; run != NULL; run = nextrun) {
    nextrun = run->next; free(run);
  }
#endif
  inc_runlist = NULL;
}


/* Reinitialise the global variables */
void reinit_libruns(void)
{
  int i;

  /* Walk the runLUT table, freeing any Run nodes */
  for (i=0; i < TABLE_SIZE; i++)
    if (runLUT[i] != NULL) {
      free(runLUT[i]);
      runLUT[i]= NULL;
    }
 
  /* Clear the two linked lists */
  clear_donelist();
  clear_inclist();
  clear_isomorph_arrays();

  any_tdngrps = 0;
}

/* Starting at the offset in both ctf files, walk run_length tuples.
 * When we find identifiers, build up a mapping between each. Give up
 * if the mapping fails or if we exceed the number of permitted mappings.
 * Return 1 if the mappings were OK, or 0 if the mappings failed.
 */
int check_isomorphic_run(Run * run, int isomorph_count_threshold)
{
  /* Get copies of the TDNs and CTF handles involved,
   * plus pointers to the start of the in-memory token runs.
   */
  TDN *src = run->src_startnode;
  TDN *dst = run->dst_startnode;
  int srcfile = src->file_line_id >> 20;
  int dstfile = dst->file_line_id >> 20;
  uint8_t *srcposn = ctf_handle[srcfile]->start + src->offset;
  uint8_t *dstposn = ctf_handle[dstfile]->start + dst->offset;
  int i = 0;
  uint16_t srcid, dstid;	/* The two identifiers to map */
  uint8_t token;

  clear_isomorph_arrays();

  /* Walk the run for its whole length */
  while (i < run->length) {

    /* Walk past any LINE tokens in either file */
    while (*srcposn == LINE) srcposn++;
    while (*dstposn == LINE) dstposn++;

    /* Fail on a token mismatch */
    if (*srcposn != *dstposn) return (0);

    /* Bypass most tokens, deal with identifiers */
    switch (*srcposn) {
    case STRINGLIT:
    case CHARCONST:
    case LABEL:
    case IDENTIFIER:
    case INTVAL:
      /* Skip the token and get the 2-byte id values */
      token= *srcposn;
      srcposn++; dstposn++; i++;
      srcid = *((uint16_t *) srcposn);
      dstid = *((uint16_t *) dstposn);
      srcposn += 2; dstposn += 2;

      /* If the identifiers are already equal, don't bother to map them */
      if (srcid == dstid) break;

      /* The identifiers are different. Only try to put in a mapping */
      /* for LABELs and IDENTIFIERs */
      if ((token != LABEL) && (token != IDENTIFIER)) return(0);

      /* First thing, record a mapping each way if there is none */
      if (isodtos[dstid] == 0) {
	isodtos[dstid] = srcid;
	isoseen[max_isoseen++] = dstid;
      }
      if (isostod[srcid] == 0) {
	isostod[srcid] = dstid;
	isoseen[max_isoseen++] = srcid;
      }
      /* Now reject if the mappings fail in either direction */
      if (isodtos[dstid] != srcid) return (0);
      if (isostod[srcid] != dstid) return (0);

      /*
       * Stop now if we have reach the threshold on the number of isomorphic
       * relations that we can have. isomorph_count_threshold is always
       * doubled because we always have a 2-way relation.
       */
      if (max_isoseen > isomorph_count_threshold)
	return (0);
      break;

    case LINE:
    case FILENAME:
      return (0);		/* We should never hit one of these! */

    default:
      srcposn++; dstposn++; i++;	/* Ordinary token, move up one */
    }
  }

  /* We got through the whole run without rejecting it, so it must be OK */
  return (1);
}

/*
 * We have compared the TDN against all in the group. Move any untouched
 * runs to the done list, so that we won't have to compare against them
 * in the future. If only_untouched==1, move the untouched runs.
 * If only_untouched==0, move all the runs. Returns # of runs moved.
 */
int move_nowcomplete_runs(int only_untouched, int do_isomorph_comparison,
			   int isomorph_count_threshold)
{
  Run *run, *lastrun, *nextcopy;
  int endhash;
  int count=0;

  /* Walk the list of runs in the incomplete list */
  for (lastrun = run = inc_runlist; run != NULL;) {
    /* Skip those which were touched, so may still be extended */
    if (only_untouched && (run->touched != 0)) {
      lastrun = run; run = run->next;
      continue;
    }

    /* We now have a run which must be removed from the incomplete list.
     * Copy the run's next pointer so we can iterate at the loop bottom.
     */
    nextcopy = run->next;

    /* Unlink the run from the list */
    if (lastrun != run)		/* Not the first run in the list */
      lastrun->next = nextcopy;
    else
      lastrun = nextcopy;

    /* Fix up the head of the incomplete list */
    if (inc_runlist == run)
      inc_runlist = nextcopy;

    /* Calculate the endhash for the run, and remove it from LUT */
    endhash = runhash(run->src_endnode, run->dst_endnode);
    runLUT[endhash] = NULL;

    /* Do an isomorphic check if required */
    if (do_isomorph_comparison) {
      /* Don't insert the run if it fails the isomorphic check */
      if (check_isomorphic_run(run, isomorph_count_threshold) == 0) {
	goto nextrun;	/* Yuk, a goto! */
      }
    }

    /* Insert the run into the completed list */
    run->next = done_runhead;
    done_runhead = run;
    count++;

  nextrun:
    /* Iterate to the next run in the list */
    run = nextcopy;
  }
  return(count);
}

/* Make a new run */
void make_new_run(TDN * tdn, TDNgrp * grp, Ctfparam * p)
{
  Run *newrun;

  /* Create a new Run
   * node and set the src nodes to tdn and grp->node. Set the length to
   * tuple_size-1 (see the NOTE in libtdn.c for explanation of the -1).
   */
  newrun = (Run *) malloc(sizeof(Run));
  if (newrun == NULL) {
    fprintf(stderr, "Unable to malloc new run: %s\n", strerror(errno));
    exit(1);
  }
  newrun->src_startnode = tdn;
  newrun->dst_startnode = grp->node;
  newrun->src_endnode = tdn;
  newrun->dst_endnode = grp->node;
  newrun->length = p->tuple_size-1;
  newrun->touched = 1;
#ifdef DEBUG
  printf("New run  ");
  print_listrun(newrun);
#endif

  /* Calculate the endhash for the new run, and add it to the LUT */
  int endhash = runhash(newrun->src_endnode, newrun->dst_endnode);
  /* if (runLUT[endhash] != NULL) printf("HASHDUP\n"); */
  runLUT[endhash] = newrun;

  /* Insert the new run into the incomplete runlist */
  newrun->next = inc_runlist;
  inc_runlist = newrun;
}

/* Extend an existing run */
void extend_run(Run * run, TDN * tdn, TDNgrp * grp)
{
  /* Calculate the endhash for the old run, and remove it from LUT */
  int endhash = runhash(run->src_endnode, run->dst_endnode);
  runLUT[endhash] = NULL;

  /* Update the Run's endnodes to point at the new
   * TDN pair, and increment the run's length.
   */
  run->src_endnode = tdn;
  run->dst_endnode = grp->node;
  run->length++;
  run->touched = 1;
#ifdef DEBUG
  printf("Extended ");
  print_listrun(run);
#endif

  /* Calculate the endhash for the extended run, and add it to the LUT */
  endhash = runhash(run->src_endnode, run->dst_endnode);
  /* if (runLUT[endhash] != NULL) printf("HASHDUP\n"); */
  runLUT[endhash] = run;
}

/* We now have two TDNs showing code similarity.
 * Try to find an existing run whose endnodes
 * match the previnctf pointers in tdn and grp.
 * If a match, extend the run. If no match, make
 * a new run.
 */
void add_extend_runs(TDN * tdn, TDNgrp * grp, Ctfparam * p)
{
#ifdef DEBUG
  printf("Starting add_extend_runs, incomplete run list is:\n");
  for (run = inc_runlist; run != NULL; run = run->next) {
    printf("  start 0x%x end->next 0x%x\n",
	   (int) run->src_startnode, (int) run->src_endnode);
  }
#endif

  /* Shortcut: get our previnctfs as a hash, lookup the LUT.
   * If a match, it must be the run for us to extend.
   */
  int endhash = runhash(tdn->previnctf, grp->node->previnctf);
  if (runLUT[endhash] != NULL) {
    extend_run(runLUT[endhash], tdn, grp);
  } else {
    /* If we didn't extend the above run, it's a new run. */
    make_new_run(tdn, grp, p);
  }
}


/** Functions to find runs of code similarity.
 *
 * find_runs_from_ctf(): given the number of a CTF file in the ctflist.db,
 * and a pointer to a Ctfparam struct, build the TDNs from that CTF file
 * in memory. Compare the TDNs from the specified CTF file to the already
 * in-memory TDNs, find any similarities, and build & extend runs of code
 * similarity in the incomplete run list. Add the TDNs from the specified
 * CTF file to the in-memory TDNs. Return a pointer to the head of a
 * singly-linked list of runs that matched the search criteria given in
 * the Ctfparam struct, or NULL if no runs were found.
 *
 * If  p->flags has CTP_PARTPRINT set, then this function will print the
 * complete runs after each source file and always return NULL. Use this
 * to conserve memory somewhat. This option is incompatible with
 * CTP_SORTRESULTS.
 *
 * If p->flags has CTP_NOSEARCH set, only create and add the CTF file's
 * TDNs to the in-memory TDNs, do no perform the run search.
 */
Run *find_runs_from_ctf(int ctfid, Ctfparam * p)
{
  Run *run;
  TDN *tdn, *lastdn = NULL;	/* Next TDN obtained from the CTF file */
  TDNgrp *grp, *lastgrp;	/* Matching tdngrp for the TDN */
  uint32_t name_offset = 0;
  int wecan_freetdn;

  /* Cache copies of some of the params from p, as we won't have
   * to follow pointer and will make the code faster. Note that
   * isomorph_count_threshold is always doubled because we
   * always have a 2-way relation.
   */
  int all_matches = p->flags & CTP_WITHINTREE;
  int do_isomorph_comparison = p->flags & CTP_ISOMORPHIC;
  int isomorph_count_threshold = 2 * p->isomorph_count_threshold;
  int lastfile= p->flags & CTP_LASTFILE;
  int partprint= p->flags & CTP_PARTPRINT;

  /* Check for illegal arguments */
  if ((ctfid < 1) || (ctfid >= ctflistnext) || (p == NULL)) return (NULL);

  clear_inclist();		/* Set the incomplete list empty */

  /* Only create/insert the TDNs if TP_NOSEARCH is set */
  if (p->flags & CTP_NOSEARCH) {
    all_matches= 0; any_tdngrps = 0;
  }

  /* If this is the first CTF file and we are not going to do an in-tree
   * search for runs, don't look for runs. Instead, simply insert the
   * TDNs into the tdngrps.
   */
  if ((all_matches == 0) && (any_tdngrps == 0)) {
    while ((tdn = get_next_tdn(ctf_handle[ctfid], ctfid, p)) != NULL) {
      grp = get_tdngrp_for(tdn, p);

      /* Point each TDN at the next one from the CTF file.
       * Strictly speaking, we should not point a TDN at a TDN from a
       * different source code file, but in practice this causes no issues.
       */
      if (lastdn != NULL) tdn->previnctf = lastdn;
      lastdn = tdn;

      /* Append tdn at the end of the grp matching the top 24 bits of CRC */
      append_tdn(tdn, grp, p);
    }
    any_tdngrps = 1;
    return (NULL);
  }

  /* We do have existing TDNgrps, so now we can look for matching runs */
  while ((tdn = get_next_tdn(ctf_handle[ctfid], ctfid, p)) != NULL) {

    /* Update the previnctf before we search for a run,
     * to ensure that any run found is able to point at this tdn.
     * Strictly speaking, we should not point a TDN at a TDN from a
     * different source code file, but in practice this causes no issues.
     */
    if (lastdn != NULL) tdn->previnctf = lastdn;
    lastdn = tdn;

    /*
     * If the name offsets between the adjacent TDNs are different, we have
     * moved to a new source file in the CTF tree. Any incomplete runs are
     * now complete, so move them to the done list.
     */
    if (name_offset != tdn->name_offset) {
      p->runcount+= move_nowcomplete_runs(0, do_isomorph_comparison,
			    isomorph_count_threshold);
#if 0
      printf("End of source file\n");
#endif
      if (partprint) {
        print_listruns(done_runhead, p);
        clear_donelist();
      }
      clear_inclist();          /* Set the incomplete list empty */
      name_offset = tdn->name_offset;
    }

    /* Mark all the runs as untouched before we work on this TDN */
    for (run = inc_runlist; run != NULL; run = run->next)
      run->touched = 0;

#ifdef DEBUG
    /* Print out the token and offset which starts this TDN */
    uint32_t o = tdn->offset;
    int tok = get_token(C, &o, NULL, NULL);
    printf("Token %s at 0x%x line %d\n", tok2str(tok), tdn->offset,
	   tdn->file_line_id & 0xfffff);

#endif

    /*
     * Get the TDN group associated with this TDN and walk all of the TDNs in
     * the group
     */
    wecan_freetdn= lastfile;	/* Only try to free TDNs on the last file */
    for (lastgrp = NULL, grp = get_tdngrp_for(tdn, p);
	 grp != NULL; lastgrp = grp, grp = grp->next) {
      /*
       * Skip TDNs that don't match: low crc mismatch, both TDNs are from the
       * same file.
       */
      uint8_t our_crcbot = tdn->tuple_crc & 0xff;

      /* Stop if from the same file, when not doing an in-tree search */
      if ((all_matches == 0) && (grp->ctfid == ctfid)) break;

      /* Skip if the full checksums don't match */
      if (grp->crcbot != our_crcbot) continue;

      /* Skip if the grp comes from the same source file as the tdn */
      if ((all_matches != 0) && (tdn->name_offset == grp->node->name_offset))
	continue;

      /* We now have two TDNs showing code similarity. Add the TDN
       * as the beginning of a new run, or extend an existing run.
       */
      add_extend_runs(tdn, grp, p);
      wecan_freetdn=0;
      p->tdncmpcnt++;
    }

#ifdef FREE_MEM
    /* Free the TDN under these circumstances: we are the last CTF file,
     * the TDN has not been put into a run, and we are not comparing
     * against ourself.
     */
    if ((!all_matches) && wecan_freetdn)
      free(tdn);
#endif

    /*
     * We have compared the TDN against all in the group. Move any untouched
     * runs to the done list, so that we won't have to compare against them
     * in the future.
     */
    p->runcount+= move_nowcomplete_runs(1, do_isomorph_comparison,
			  isomorph_count_threshold);

    /* Append the TDN at the end of the grp matching the top 24 bits of CRC.
     * Do this if we are looking for all matches (i.e. within CTF trees), or 
     * if there will be future CTF files that want to compare against us.
     */
    if (all_matches || (!lastfile))
      append_tdn(tdn, lastgrp, p);
  }

  /* Move any incomplete runs to the done list before returning it. */
  p->runcount+= move_nowcomplete_runs(0, do_isomorph_comparison,
						isomorph_count_threshold);
  if (partprint) {
    print_listruns(done_runhead, p);
    clear_donelist();
  }
  return (done_runhead);
}
