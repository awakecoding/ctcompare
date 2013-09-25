/*
 * Header file for the CTF source code similarity library. For details,
 * see http://minnie.tuhs.org/Programs/Ctcompare/
 *
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * $Revision: 1.25 $
 */

#ifndef LIBCTF_H
#define LIBCTF_H

/*** Constants defined by the library ***/

#define CTFLIST_DB "ctflist.db"	/* Name of the Ctf list created */
#define MAXCTFNAME 1024		/* Maximum size of any CTF filename */
#define TUPLE_SIZE 16	   /* By default, each tuple has TUPLE_SIZE tokens */


/*** Structures defined by the library ***/

/* List of parameters passed to various functions */
typedef struct _ctfparam
{
  /* Parameters that affect the determination of code similarity */
  int tuple_size;		/* Number of tokens in each tuple */
  char *dbname;			/* Name of disk file with list of CTF files */
  int isomorph_count_threshold;	/* Maximum # of isomorphic relations */
  int flags;			/* Search & printing flags; see below */

  /* Statistics counters */
  int runcount;			/* Number of runs of similarity found */
  int tdncount;			/* Number of TDNs used to find similarities */
  int tdncmpcnt;		/* Number of TDN comparisons made */
} Ctfparam;

				/* Available flag bits & their meaning */
#define CTP_ISOMORPHIC  0x01	/* Do an isomorphic comparison on the code */
#define CTP_WITHINTREE  0x02	/* Search for runs within each tree */
#define CTP_PRINTTOKENS 0x04	/* Print out tokens in a run */
#define CTP_PRINTCODE   0x08	/* Print out the code in a run */
#define CTP_SIDEBYSIDE  0x10	/* Print out the code side by side */
#define CTP_SORTRESULTS 0x20	/* Print results sorted by run length */
#define CTP_NOSEARCH    0x40	/* Don't search for runs, just create */
				/* token tuples in find_runs_from_ctf() */
				/* Note that CTP_PRINTTOKENS, CTP_PRINTCODE */
				/* and CTP_SIDEBYSIDE are mutually exclusive */
#define CTP_LASTFILE	0x80	/* If this is set, it indicates that the */
				/* last CTF file in the list is being */
				/* compared for similarities. We can avoid */
				/* adding its TDNs to the in-memory table. */
#define CTP_PARTPRINT	0x100	/* If set, find_runs_from_ctf() prints out */
				/* the runs itself and returns NULL */
#define CTP_COMPHEUR	0x200	/* Enable some heuristics which remove */
				/* certain unwanted matches: see the Readme */


/* Handle to an open and mmap()d CTF file */
typedef struct _ctfhandle
{
  int fd;		/* File descriptor used to mmap() the file */
  uint8_t *start;	/* Starting address of the mmap */
  uint8_t *end;		/* End address of the mmap +1 (i.e 1st outside) */
  uint8_t *cursor;	/* Current position in the map, used internally */
  uint32_t name_offset;	/* Offset of the last filename found */
  uint32_t linenum;	/* Linenumber of the last line found */
} Ctfhandle;


/* The TDN represents the details of one tuple of TUPLE_SIZE tokens from
 * a CTF file: which TDN preceded this one, the tuple's CRC, the offset
 * of the first token in the CTF file, and the offset of the source code
 * filename which contains the tokens in this tuple.
 *
 * The top 12 bits of file_line_id represent the CTF file identity, i.e.
 * there can be up to 4096 CTF files in the database. The bottom 20 bits
 * represent the line number where the first token in the TDN occurred, i.e.
 * source code files can be up to 1,048,576 lines long.
 */
typedef struct _tdn
{
  struct _tdn *previnctf;     /* Previous TDN from the CTF file */
  uint32_t tuple_crc;	      /* CRC of the tokens in this tuple */
  uint32_t offset;	      /* Offset where this tuple of tokens occurs */
  uint32_t name_offset;	      /* Offset of the filename which has this tuple */
  uint32_t file_line_id;    /* Two bitfields containing CTF file id & line # */
} TDN;

#define NUMCTFFILES (1 << 12)
#define MAXSRCLINES (1 << 20)


/*
 * We build up runs of code similarity by walking the TDNs of a single CTF
 * tree. This means that the number of incomplete runs is small, and one
 * "side" of the code similarity comes from the same CTF tree. We will
 * represent each run with the Run node. The fields are the starting and
 * ending TDNs from each tree, the length of the run in tokens, and a
 * next pointer used to build a singly-linked list of runs found. The
 * touched flag is used internally and should not be modified.
 */

typedef struct _run
{
  TDN *src_startnode;		/* 1st TDN from tree we are walking */
  TDN *dst_startnode;		/* 1st TDN from the other tree */
  TDN *src_endnode;		/* Currently last TDN from walking tree */
  TDN *dst_endnode;		/* Currently last TDN from other tree */
  uint32_t length;		/* Length of the run so far */
  struct _run *next;		/* Linked list of all incomplete runs */
  uint32_t touched;		/* Flag to indicate if the run was touched */
} Run;


/*** Functions exported by the library ***/

#endif /* LIBCTF_H */
