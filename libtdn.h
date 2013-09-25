/*
 * libtdn: Definition of the tuple definition node, and function prototypes.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.8 $
 */

/* We have a 2^24 bit array of pointers to the head of a 
 * chain of TDNs. Each node in the chain looks like the following.
 * This collects all the TDNs which share the same top 24 bits of CRC
 * into a linked list.
 */
#ifndef LIBTDN_H
#define LIBTDN_H

typedef struct tdngrp
{
  uint8_t crcbot;		/* Bottom 8-bits of 32-bit CRC */
  uint16_t ctfid;		/* Cached copy of the node's CTF file-id */
  TDN *node;			/* Pointer to the TDN node */
  struct tdngrp *next;		/* Next node in the list */
} TDNgrp;

void init_libtdn(Ctfparam * p);
TDN *get_next_tdn(Ctfhandle * ctf, int fileid, Ctfparam * p);
TDNgrp *get_tdngrp_for(TDN * tdn, Ctfparam * p);
int append_tdn(TDN * tdn, TDNgrp * grp, Ctfparam * p);

#endif /* LIBTDN_H */

