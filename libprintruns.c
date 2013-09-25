/*
 * printruns.c: Code to print out the runs once they are complete.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.29 $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "libctf.h"
#include "libtokens.h"

#undef NO_PRINTING		/* No printing for performance measurements */
#undef PRINTOFFSETS		/* Print token offsets, not line numbers */

extern Ctfhandle *ctf_handle[];	/* Array of CTF handles */


#ifdef DEBUG
/* Debug function: can be removed */
void print_tdn(TDN * tdn)
{
  int fileid = tdn->file_line_id >> 20;
  int linenum = tdn->file_line_id & 0xfffff;
  printf("crc %08x offset %04x name %04x file %02d line %03d\n",
	 tdn->tuple_crc, tdn->offset, tdn->name_offset,
	 fileid, linenum);
}
#endif

/* Given a TDN and a CTF file handle, return the line number
 * for the last token in the TDN. Returns the line number on
 * success, -1 on error.
 */
int last_linenum_for(TDN * tdn, Ctfhandle * ctf, Ctfparam * p)
{
  /* Error checking */
  if ((tdn == NULL) || (ctf == NULL)) return (-1);

  /* Get the line number of the first token and
   * the location where the token occurs. Make sure
   * that it lies in the mmap'd area.
   */
  int linenum = tdn->file_line_id & 0xfffff;
  uint8_t *posn = ctf->start + tdn->offset;
  if ((posn < ctf->start) || (posn >= ctf->end)) return (-1);

  /* Skip past tuple_size tokens, counting lines */
  /* We use -1 here, see the NOTE in libtdn.c. */
  int i = 0;
  while (i < p->tuple_size-1) {
    switch (*posn) {
    case STRINGLIT:
    case CHARCONST:
    case LABEL:
    case IDENTIFIER:
    case INTVAL:
      /* Skip the token + the 2-byte id value */
      posn += 3; i++;
      break;

    case FILENAME:
      /* We should never hit one of these! */
      return (-1);

    case LINE:
      /* Found a new line! */
      linenum++; posn++;
      break;

    default:
      /* Ordinary token, move up one */
      posn++; i++;
    }
  }
#ifdef PRINTOFFSETS
  return ((int) (posn - ctf->start) - 1);	/* -1 because posn++ above */
#else
  return (linenum);
#endif
}

/*
 * Given a run, print out the tokens in the run in much the same way as we do
 * in detok.c
 */
void print_tokens(Run * node)
{
  uint32_t val;
  unsigned int ch;
  int length = node->length;
  int fid = node->src_startnode->file_line_id >> 20;
  uint32_t offset = node->src_startnode->offset;
  uint32_t line = node->src_startnode->file_line_id & 0xfffff;

  printf("%5d:   ", line);
  while ((length > 0) &&
	 ((ch = get_token(ctf_handle[fid], &offset, &val, NULL)) != -1)) {
    switch (ch) {
    case FILENAME:
      line = 1;
      break;
    case LINE:
      line++;
      break;
    default:
      length--;
    }
    print_token(ch, line, val, "");
  }
  printf("\n\n");
}


/*
 * Print out lines of two files side-by-side. This is best viewed with a
 * terminal window 160 characters wide! If neither file exists, the function
 * simply returns with no error message.
 */
void paste_files(char *file1, char *file2, Run *node, int side_side,
		 Ctfparam * p)
{
  FILE *f1in, *f2in;
  char buf[1024];
  int count_to_eighty = 0;
  int i, bptr, maxlines;
  int numlines1, numlines2;
  int tab_upto = 0;
  int src_ctfid = node->src_startnode->file_line_id >> 20;
  int dst_ctfid = node->dst_startnode->file_line_id >> 20;
  char *err;

  int start1 = node->src_startnode->file_line_id & 0xfffff;
  int start2 = node->dst_startnode->file_line_id & 0xfffff;
  int end1 = last_linenum_for(node->src_endnode, ctf_handle[src_ctfid], p);
  int end2 = last_linenum_for(node->dst_endnode, ctf_handle[dst_ctfid], p);

  f1in = fopen(file1, "r");
  if (f1in == NULL) side_side = 0;

  f2in = fopen(file2, "r");
  if (f2in == NULL) side_side = 0;

  /* We can't display either file, simply show the tokens */
  if ((f1in == NULL) && (f2in == NULL)) {
    print_tokens(node); return;
  }
  /* Get maximum number of lines */
  numlines1 = end1 - start1 + 1;
  numlines2 = end2 - start2 + 1;
  maxlines = (numlines1 > numlines2) ? numlines1 : numlines2;
  start1--; start2--;

  /* Read up to line start1 in file1 */
  if (f1in != NULL)
    while (start1--) err = fgets(buf, 1020, f1in);

  /* Read up to line start2 in file2 */
  if (f2in != NULL)
    while (start2--) err = fgets(buf, 1020, f2in);

  if (side_side) {
    /* Now print out the lines */
    for (i = 0; i < maxlines; i++, numlines1--, numlines2--) {

      /* Print the left-hand line */
      if (numlines1 > 0) {
	err = fgets(buf, 1020, f1in);
	buf[strlen(buf) - 1] = '\0';	/* Remove newline */

	/*
	 * I used to printf("%-80s",buf); here, but it never did what I
	 * wanted, so I now print out exactly 80 chars by hand. This is
	 * because we have to deal with tabs, dammit!
	 */
	tab_upto = 0; bptr = 0;
	for (count_to_eighty = 0; count_to_eighty < 80; count_to_eighty++) {
	  if (tab_upto > count_to_eighty) {
	    fputc(' ', stdout); continue;
	  }
	  if (buf[bptr] == '\0') break;
	  if (buf[bptr] == '\t') {

	    /* Calculate the next highest multiple of 8 */
	    tab_upto = (count_to_eighty + 8) & ~7;
	    if (tab_upto >= 80) break;
	    fputc(' ', stdout);
	    bptr++;
	    continue;
	  }
	  fputc(buf[bptr++], stdout);
	}
	for (; count_to_eighty < 80; count_to_eighty++) {
	  fputc(' ', stdout);
	}
	printf(" ");
      } else
	printf("%81s", " ");

      /* Print the right-hand line */
      if (numlines2 > 0) {
	err = fgets(buf, 1020, f2in);
	fputs(buf, stdout);
      } else
	printf("\n");
    }
  } else {			/* Not side-by-side */
    if (f1in != NULL)
      for (i = 0; i < numlines1; i++) {
	err = fgets(buf, 1020, f1in);
	fputs(buf, stdout);
    } else
      print_tokens(node);
    printf("=====================================\n");
    if (f2in != NULL)
      for (i = 0; i < numlines2; i++) {
	err = fgets(buf, 1020, f2in);
	fputs(buf, stdout);
    } else
      print_tokens(node);
  }
  printf("\n");
  if (numlines1 > numlines2) printf("\n");
  if (f1in != NULL) fclose(f1in);
  if (f2in != NULL) fclose(f2in);
}


void print_listrun(Run * run, Ctfparam * p)
{
  uint32_t off;
  int src_lastline, dst_lastline;
  char *sname, *dname;

  /* Do nothing if the run length < the tuple size */
  if (run->length < p->tuple_size)
    return;

#ifndef NO_PRINTING
  int src_ctfid = run->src_startnode->file_line_id >> 20;
  int dst_ctfid = run->dst_startnode->file_line_id >> 20;

  /* Find where the filenames actually start: base + offset + skip the token
   * + skip the 4-byte timestamp
   */
  off = run->src_startnode->name_offset;
  sname= (char *)(ctf_handle[src_ctfid]->start + off + 1 + sizeof(uint32_t));
  off = run->dst_startnode->name_offset;
  dname= (char *)(ctf_handle[dst_ctfid]->start + off + 1 + sizeof(uint32_t));
  
  /*
   * The two TDNs are the last where all tuple_size tokens match, but the
   * line number in the TDN is for the first token, not the last token. We
   * need to manually walk another tuple_size TDNs to get the real end line
   * numbers.
   */
  src_lastline = last_linenum_for(run->src_endnode, ctf_handle[src_ctfid], p);
  dst_lastline = last_linenum_for(run->dst_endnode, ctf_handle[dst_ctfid], p);

#ifdef PRINTOFFSETS
  printf("%d  %s:%d-%d  %s:%d-%d\n",
	 run->length,
	 sname, (int) run->src_startnode->offset, src_lastline,
	 dname, (int) run->dst_startnode->offset, dst_lastline);
#else
  printf("%d  %s:%d-%d  %s:%d-%d\n",
	 run->length,
	 sname, run->src_startnode->file_line_id & 0xfffff, src_lastline,
	 dname, run->dst_startnode->file_line_id & 0xfffff, dst_lastline);
#endif

  /* Now print out more detailed results as required */
  if (p->flags & CTP_SIDEBYSIDE) paste_files(sname, dname, run, 1, p);
  else if (p->flags & CTP_PRINTCODE) paste_files(sname, dname, run,0, p);
  else if (p->flags & CTP_PRINTTOKENS) print_tokens(run);

  if ((p->flags & CTP_PRINTTOKENS) || (p->flags & CTP_SIDEBYSIDE))
    printf("=====================================\n");

#endif /* NO_PRINTING */
}

/* Comparison function used by qsort below */
int runlen_compare(const void *aa, const void *bb)
{
  Run *a = *((Run **) aa);
  Run *b = *((Run **) bb);
  if (b->length < a->length) return (-1);
  if (b->length == a->length) return (0);
  return (1);
}

/* Print out all the runs from the runlist in descending runlength order. */
void print_sorted_listruns(Run * origrun, Ctfparam * p)
{
  int i, count = 0;
  Run *run, **runarray;
  /* This is going to be ugly */

  /* Count all the runs in the list */
  for (run = origrun; run != NULL; run = run->next) count++;

  /* Allocate an array to hold all the pointers */
  runarray = (Run **) malloc(count * sizeof(Run *));
  if (runarray == NULL) return;		/* Should we return an error ? */

  /* Fill the array with the Run pointers */
  for (i = 0, run = origrun; run != NULL; i++, run = run->next)
    runarray[i] = run;

  /* Quicksort the array */
  qsort(runarray, count, sizeof(Run *), runlen_compare);

  /* Now print out the runs */
  for (i = 0; i < count; i++)
    print_listrun(runarray[i], p);
}

/** Functions to print out code similarity.
 *
 * print_listruns(): given the head of a singly-linked list of runs
 * an a pointer to a Ctfparam struct, print out the runs of code
 * similarity following the print options specified in the Ctfparam struct.
 */
void print_listruns(Run * run, Ctfparam * p)
{
  if (p == NULL) return;

  if (p->flags & CTP_SORTRESULTS) {
    print_sorted_listruns(run, p); return;
  }

  for (; run != NULL; run = run->next)
    print_listrun(run, p);
}
