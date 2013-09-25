/*
 * libtdn: Functions to manage the tuple description nodes.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 *
 * $Revision: 1.21 $
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libctf.h"
#include "libtokens.h"
#include "libtdn.h"
#include "crc32.h"

#define BITSINTABLE     24	/* 24-bit table */
#define TABLE_SIZE (1 << BITSINTABLE)

TDNgrp *tdngrplist[TABLE_SIZE];	/* Array of TDNgrp list heads */

/* Initialise the TDN global variables */
void init_libtdn(Ctfparam * p)
{
  /* Nothing to do */
}

/* Reinitialise the global variables */
void reinit_libtdn(void)
{
  TDNgrp *node, *next;
  int i;

  /* Walk the tdngrplist, find any lists and free all the TDNgrp
   * nodes on the list.
   */
  for (i=0; i < TABLE_SIZE; i++) {
    if (tdngrplist[i]==NULL) continue;

    for (node=tdngrplist[i]; node != NULL; node= next) {
      next= node->next;
      free(node->node);
      free(node);
    }

    /* And clear the head of the list */
    tdngrplist[i]= NULL;
  }
}


/* Return a pointer to a malloc'd TDN which contains the next tuple
 * description from the given Ctfhandle. The caller assumes responsibilty
 * for the allocated memory. NULL is return if the Ctfhandle is invalid,
 * or if there are no more tuples in the CTF file. id is the id of the
 * CTF file in the database.
 */
TDN *get_next_tdn(Ctfhandle * ctf, int fileid, Ctfparam * p)
{
  /* NOTE: We actually search for matching tuples of size p->tuple_size-1.
   * We compensate for this in print_listrun() where we only print out
   * runs of size p->tuple_size. The reason for searching for tuples of
   * size p->tuple_size-1 is as follows: We are using hash values to find
   * matching tuples, but this brings a risk of collisions which leads to
   * false positives. If the minimum run length is 16 and we use tuples of
   * size 15, then this means that two consecutive tuples must match, not
   * just a single tuple. This significantly reduces the number of false
   * positives.
   */
  int Tuple_size = p->tuple_size-1;
  int do_isomorph_comparison = p->flags & CTP_ISOMORPHIC;
  int do_heuristics = p->flags & CTP_COMPHEUR;

  /* Note: the tuple[] array actually contains the space for the Tuple_size
   * tokens, plus room for the Tuple_size hashed identifiers/literals that
   * occur with the tokens. The valhash pointer points directly at the
   * first hashed identifiers/literal.
   */
  uint8_t tuple[Tuple_size + Tuple_size * sizeof(uint16_t)];
  uint16_t *valhash;		/* List of id hash values */
  uint8_t *posn;		/* Position we are currently looking at */
  uint8_t token;		/* Token at that position */
  uint8_t ptok, pptok;		/* Previous and previous-previous token */
  uint16_t idvalue;
  uint32_t linenum, ourlinenum = 0;
  uint32_t offset = 0;		/* Offset of this tuple */
  TDN *tdn;
  int i;			/* Index into the tuple array */

  /* Error if inputs are bad */
  if ((fileid < 1) || (ctf == NULL)) return (NULL);

  /* Error if EOF */
  if (ctf->cursor >= ctf->end) return (NULL);

  /* Initialise vars for this tuple */
  valhash = (uint16_t *) & tuple[Tuple_size];
  posn = ctf->cursor;
  linenum = ctf->linenum;
  i = 0;
  ptok= pptok = 0;

  /* Read Tuple_size tokens into the tuple array */
  while ((i < Tuple_size) && (posn < ctf->end)) {
    /* Get the token */
    token = *posn;

    /* Deal with the token */
    switch (token) {
    case FILENAME:
      /* A new file, so discard any tokens we have so far 
       * and start with this as the file for this tuple.
       */
      i = 0;
      ctf->name_offset = (uint32_t) (posn - ctf->start);
      linenum = ctf->linenum = 1;
      posn += 5;		/* Skip the token & the timestamp */

      /* Move the position up past the name */
      while (((token = *(posn++)) != '\0') && (posn < ctf->end));
      break;

    case LINE:
      linenum++; posn++;
      break;

    case STRINGLIT:
    case CHARCONST:
    case LABEL:
    case IDENTIFIER:
    case INTVAL:
      /* Save our offset and line number */
      if (i == 0) {
	offset = (uint32_t) (posn - ctf->start);
	ourlinenum = linenum;
      }
      /* Save the cursor and line number for next time */
      if (i == 1) {
	ctf->cursor = posn;
	ctf->linenum = linenum;
      }
      /* Read in 2 bytes to get the id value */
      posn++;
      idvalue = *(posn++) << 8;
      idvalue += *(posn++);

      /* Heuristic: prevent comparisons on num,num,num,num,num */
      if (do_heuristics && (pptok==INTVAL)
			&& (ptok==COMMA) && (token==INTVAL)) {
	idvalue = rand();
      }

      valhash[i] = idvalue;
      tuple[i] = token;
      i++;
      pptok= ptok; ptok= token;
      break;

    default:
      /* Save our offset and line number */
      if (i == 0) {
	offset = (uint32_t) (posn - ctf->start);
	ourlinenum = linenum;
      }

      /* Save the cursor and line number for next time */
      if (i == 1) {
	ctf->cursor = posn;
	ctf->linenum = linenum;
      }
      valhash[i] = 0;
      tuple[i] = token;
      posn++; i++;
      pptok= ptok; ptok= token;
    }
  }

  /* We now have Tuple_size tokens, or ran out of input */
  if (i < Tuple_size) return (NULL);

  /* Build and populate the TDN */
  tdn = (TDN *) malloc(sizeof(TDN));
  if (tdn == NULL) return (NULL);

  /* Make the checksums */
  if (do_isomorph_comparison)
    tdn->tuple_crc = crc32(tuple, Tuple_size);
  else
    tdn->tuple_crc = crc32(tuple, Tuple_size + Tuple_size * sizeof(uint16_t));

  /* Fill in the rest of the TDN */
  tdn->offset = offset;
  tdn->name_offset = ctf->name_offset;
  tdn->file_line_id = (fileid << 20) | (ourlinenum & 0xfffff);
  tdn->previnctf=NULL;

  return (tdn);
}


/*
 * Given a TDN return a pointer to the
 * TDNgrp which match the top 24 bits of the CRC. Returns NULL on errors.
 */
TDNgrp *get_tdngrp_for(TDN * tdn, Ctfparam * p)
{
  if (tdn == NULL) return (NULL);

  /* Use the top 24 bits of the CRC to get the index into the tdngrplist */
  int index = tdn->tuple_crc >> (32-BITSINTABLE);
  return (tdngrplist[index]);
}

/*
 * Using this_tdn, insert it into the TDNgrp which matches the top 24 bits
 * of the CRC. The grp pointer points a node in the existing TDNgrp, but
 * not the head node. Returns 0 if ok, -1 on error.
 */
int append_tdn(TDN * tdn, TDNgrp * grp, Ctfparam * p)
{
  TDNgrp *newnode;

  /* Use the top 24 bits of the CRC to get the index into the tdngrplist */
  int index = tdn->tuple_crc >> (32-BITSINTABLE);

  /* Allocate & fill in the newnode to point to tdn */
  newnode = (TDNgrp *) malloc(sizeof(TDNgrp));
  if (newnode == NULL) {
    /* printf("Unable to malloc a TDNgrp: %s\n", strerror(errno)); */
    return (-1);
  }
  newnode->crcbot = tdn->tuple_crc & 0xff;
  newnode->ctfid = tdn->file_line_id >> 20;
  newnode->node = tdn;

  /* Store tdn into the group's linked list, near the end */
  if (grp != NULL) {
    newnode->next = grp->next;
    grp->next = newnode;
  } else {
    /* or if we are the first node, at the front  */
    newnode->next = tdngrplist[index];
    tdngrplist[index] = newnode;
  }
  p->tdncount++;
  return (0);
}
