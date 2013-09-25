/*
 * libctflist: Functions to keep track of what CTF files to compare.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.14 $
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libctf.h"


/* We keep the CTF list in memory in the following array
 * along with the next free slot in the list, and an 
 * array of CTF handles.
 */
int ctflistnext = 1;
char *ctflist[NUMCTFFILES];
Ctfhandle *ctf_handle[NUMCTFFILES];

/* Reinitialise the global variables */
void reinit_libctflist(void)
{
  int i;
  for (i = 1; i < ctflistnext; i++) {
    free(ctflist[i]);
    ctflist[i]=NULL;
    if (ctf_handle[i]) {
      ctfclose(ctf_handle[i]);
      ctf_handle[i]=NULL;
    }
  }
  ctflistnext = 1;
  return;
}

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
int load_ctflist(void)
{
  char buffer[MAXCTFNAME + 1];	/* Buffer for file input */
  FILE *cin;
  int i;

  /* Try to open any unopened ctf handles */
  for (i = 1; i < ctflistnext; i++)
    if (ctf_handle[i]==NULL)
      ctf_handle[i]= ctfopen(ctflist[i]);

  /* Don't re-read the ctflist file if the list is populated */
  if (ctflistnext > 1) return (ctflistnext);

  /* Open the ctflist file */
  cin = fopen(CTFLIST_DB, "r");
  if (cin == NULL) return (-1);

  /* Read in each entry into the list */
  while (ctflistnext < NUMCTFFILES) {
    if (fgets(buffer, MAXCTFNAME, cin) == NULL) break;

    /* Remove the newline on the end */
    buffer[strlen(buffer) - 1] = '\0';
    ctflist[ctflistnext++] = strdup(buffer);
  }

  /* Close the input file and return the number of entries */
  fclose(cin);
  return (ctflistnext);
}

/** get_ctfname(): given a specific CTF file id (1 or greater), return a
 * pointer to the CTF file's name. Returns NULL if the ctflist is not loaded,
 * or there is no entry in the list with the given id.
 */
char *get_ctfname(int id)
{
  if (id < 1 || id >= ctflistnext) return (NULL);
  return (ctflist[id]);
}

/** id_of_ctffile(): given a CTF filename, find the CTF file id in the
 * ctflist. Returns a number greater than 0 on success, or -1 on failure:
 * either the ctflist is not loaded, or the CTF filename is not represented
 * in the ctflist.
 */
int id_of_ctffile(char *name)
{
  /* Walk the ctflist and return the position of the matching string */
  int i;

  if (name == NULL) return (-1);
  for (i = 1; i < ctflistnext; i++)
    if (!strcmp(ctflist[i], name))
      return (i);

  return (-1);
}

/** add_ctffile(): given a CTF filename, add the CTF file to the ctflist in
 * memory and on disk if requested. The function ensures that a CTF filename
 * cannot be duplicated in the in-memory list. Returns the CTF file id for
 * the CTF file which is a number greater than 0 on success, or -1 on
 * failure: either the ctflist is not loaded, or the CTF filename is not
 * represented in the ctflist.
 */
int add_ctffile(char *name, int ondisk)
{
  FILE *cout;
  int id;

  /* Load the ctflist just in case */
  load_ctflist();

  /* Return the id if it's already in the list */
  if (name == NULL) return (-1);
  if ((id = id_of_ctffile(name)) != -1) return (id);

  /* Try to save it in the on-disk file */
  if (ondisk == 1) {
    cout = fopen(CTFLIST_DB, "a");
    if (cout == NULL) return (-1);
    fprintf(cout, "%s\n", name);
    fclose(cout);
  }

  /* Not in the list, so try to copy it into the list */
  if (ctflistnext == NUMCTFFILES) return (-1);

  ctflist[ctflistnext++] = strdup(name);
  return (ctflistnext);
}

/* This doesn't belong here, but there is no other good place to put it. */
extern void reinit_libruns(void);
extern void reinit_libtdn(void);

/** Functions to reset the state of the system to its initial value.
 *
 * init_ctfparams(): reset the state of the system to its initial value.
 * As well, create, initialise and return a pointer to a Ctfparam structure
 * with default values. On failure, NULL is returned. If an old
 * Ctfparam pointer is passed, this will be reinitialised, and all
 * the internal state variables will be reset to their initial values.
 * Note: the system state resetting may take a while.
 */
Ctfparam *init_ctfparams(Ctfparam * oldparams)
{
  Ctfparam *p;

  if (oldparams != NULL) {
    p = oldparams;

    /* Reset the system state */
    reinit_libctflist();
    reinit_libtdn();
    reinit_libruns();
  } else {
    p = (Ctfparam *) malloc(sizeof(Ctfparam));
    if (p == NULL) return (NULL);
  }

  /* Fill in default values */
  p->tuple_size = TUPLE_SIZE;
  p->dbname = CTFLIST_DB;
  p->isomorph_count_threshold = 3;
  p->flags = 0;
  p->runcount = 0;
  p->tdncount = 0;
  p->tdncmpcnt = 0;
  return (p);
}
