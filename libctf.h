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

/** Functions to tokenise a source code tree.
 *
 * tokenise_tree: given a directory name, open and tokenise all the
 * source files therein, including subdirectories. Save the tokenised result
 * as a CTF file called output_file. If ondisk is 1, add the name of the
 * CTF file to the ctflist.db file. The function returns 0 on success. or
 * If an error occurs, the function returns -1 and sets errno.
 *
 * The output_file name, if it does not end in ".ctf", will have ".ctf"
 * appended.
 *
 * If splitsize is non-zero, it represents a size in bytes. The output
 * consists of a number of outputfiles, each of size roughly splitsize,
 * with names based on output_file. For example, if output_file is
 * "abc.ctf", then the files will be "abc0001.ctf", "abc0002.ctf", etc.
 */
int tokenise_tree(char *directory_name, char *output_file, int ondisk, int splitsize);


/** Functions dealing with the token stream stored in a CTF file.
 *
 * ctfopen(): open the named CTF file for reading, checking the header
 * as well. Returns the Ctfhandle handle to the open file, or sets errno
 * and returns NULL on error.
 */
Ctfhandle *ctfopen(char *name);

/** ctfclose(): close an open Ctfhandle and free the Ctfhandle's memory.
 * Returns 0 if OK, -1 on error.
 */
int ctfclose(Ctfhandle * ctf);

/** get_token(): given a Ctfhandle and a file offset, return the next
 * token from the file at the given offset. The offset is updated to point
 * at the next token. Any id-value associated with the the token is
 * returned in the id parameter, or 0 if there is no value. Any filename
 * associated with a FILENAME token is returned in name, and the id
 * parameter is used to return the timestamp. On any error, -1 is returned.
 * The space for the filename is malloc'd here; the caller takes
 * responsibility for freeing it.
 */
int get_token(Ctfhandle * ctf, uint32_t * offset, uint32_t * id, char **name);

/** tok2str(): given a token value, return a pointer to a
 * string constant which represents that token value.
 * Returns NULL if the given token value does not exist.
 */
char *tok2str(unsigned int ch);

/** print_token(): given a token value, a line number, an identifier
 * value associated with the token, and a filename associated with the
 * token, print out a textual representation of that token. The function
 * performs no input error checking.
 */
void print_token(unsigned int ch, uint32_t linenum, uint32_t id, char *filename);


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
Run *find_runs_from_ctf(int ctfid, Ctfparam * p);


/** Functions to print out code similarity.
 *
 * print_listruns(): given the head of a singly-linked list of runs
 * an a pointer to a Ctfparam struct, print out the runs of code
 * similarity following the print options specified in the Ctfparam struct.
 */
void print_listruns(Run * run, Ctfparam * p);


/** Functions dealing with the on-disk list of CTF files: ctflist.db.
 *
 * load_ctflist(): load the details of the CTF list from ctflist.db into
 * an in-memory list. Returns the number of entries in the in-memory list on
 * success, or sets errno and returns -1 on failure. This function can be
 * called multiple times, and the list will only be loaded from disk once.
 * If the in-memory list is updated using other functions, the function will
 * always return the number of entries in the in-memory list, regardless of
 * the state of the on-disk list.
 */
int load_ctflist(void);

/** get_ctfname(): given a specific CTF file id (1 or greater), return a
 * pointer to the CTF file's name. Returns NULL if the ctflist is not loaded,
 * or there is no entry in the list with the given id.
 */
char *get_ctfname(int id);

/** id_of_ctffile(): given a CTF filename, find the CTF file id in the
 * ctflist. Returns a number greater than 0 on success, or -1 on failure:
 * either the ctflist is not loaded, or the CTF filename is not represented
 * in the ctflist.
 */
int id_of_ctffile(char *name);

/** add_ctffile(): given a CTF filename, add the CTF file to the ctflist in
 * memory and on disk if requested. The function ensures that a CTF filename
 * cannot be duplicated in the in-memory list. Returns the CTF file id for
 * the CTF file which is a number greater than 0 on success, or -1 on
 * failure: either the ctflist is not loaded, or the CTF filename is not
 * represented in the ctflist.
 */
int add_ctffile(char *name, int ondisk);

/** Functions to reset the state of the system to its initial value.
 *
 * init_ctfparams(): reset the state of the system to its initial value.
 * As well, create, initialise and return a pointer to a Ctfparam structure
 * with default values. On failure, NULL is returned. If an old
 * Ctfparam pointer is passed, this will be reinitialised, and all
 * the internal state variables will be reset to their initial values.
 * Note: the system state resetting may take a while.
 */
Ctfparam *init_ctfparams(Ctfparam * oldparams);



#endif /* LIBCTF_H */
