/*
 * Find and report all code similarities in the given CTF files.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.31 $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "libctf.h"
#include "libtdn.h"

void usage(void)
{
  fprintf(stderr,
	  "Usage: ctcompare [-n nnn] [-rstxiaqp] [-I nnn] [CTF file] [CTF file...]\n");
  fprintf(stderr, "\t-n nnn: set the minimum matching run length to nnn\n");
  fprintf(stderr,
	  "\t-r:     print results sorted by run length descending\n");
  fprintf(stderr, "\t-s:     print results side by side on same line\n");
  fprintf(stderr,
	  "\t-x:     show matching source lines when match is found\n");
  fprintf(stderr, "\t-t:     show matching tokens when match is found\n");
  fprintf(stderr, "\t-i:     enable isomorphic code comparison\n");
  fprintf(stderr,
	  "\t-I nnn: limit the # isomorphic relations to nnn, implies -i\n");
  fprintf(stderr,
	  "\t-a      show all matches even if they are in the same tree\n");
  fprintf(stderr,
	  "\t-q      quiet, only print the number of matches found\n");
  fprintf(stderr,
	  "\t-p      print partial results, incompatible with -q -r\n");
  fprintf(stderr,
	  "\t-u      enable heuristics to reduce unwanted comparisons\n");
  fprintf(stderr, "    CTF file arguments augment those in the %s file\n",
	  CTFLIST_DB);
  exit(1);
}

int main(int argc, char *argv[])
{
  int numctf;			/* Number of CTF files to process */
  int i, ch;
  int quiet = 0;
  Ctfhandle *C;
  Ctfparam *p;
  Run *run, *foundruns = NULL;	/* Matching runs of code that were found */
  int runcount=0;

  /* Initialise the params structure */
  p = init_ctfparams(NULL);
  if (p == NULL) {
    fprintf(stderr, "Unable to initialise ctfparams structure\n"); exit(1);
  }

  /* Process options */
  while ((ch = getopt(argc, argv, "an:iI:rstxqpu")) != -1) {

    switch (ch) {
    case 'I':
      i = atoi(optarg);
      if (i < 2) {
	fprintf(stderr, "Bad value for -I, must be 1 or greater\n");
      } else {
	p->isomorph_count_threshold = i;
      }
    case 'i':
      p->flags |= CTP_ISOMORPHIC; break;
    case 'r':
      p->flags &= ~CTP_PARTPRINT;
      p->flags |= CTP_SORTRESULTS; break;
    case 't':
      p->flags |= CTP_PRINTTOKENS; break;
    case 's':
      p->flags |= CTP_SIDEBYSIDE; break;
    case 'x':
      p->flags |= CTP_PRINTCODE; break;
    case 'a':
      p->flags |= CTP_WITHINTREE; break;
    case 'n':
      i = atoi(optarg);
      if (i < 16) {
	fprintf(stderr, "Bad value for -n, must be 16 or greater\n");
      } else
	p->tuple_size = i; break;
    case 'q':
      p->flags &= ~CTP_PARTPRINT;
      quiet = 1; break;
    case 'p':
      p->flags |= CTP_PARTPRINT;
      quiet = 0; break;
    case 'u':
      p->flags |= CTP_COMPHEUR; break;
    default:
      usage();
    }
  }
  argc -= optind;
  argv += optind;

  /* Get the list of CTF files in the on-disk list */
  load_ctflist();

  /* Add on any extra CTF files from the command line */
  for (i = 0; i < argc; i++)
    add_ctffile(argv[i], 0);

  /* Get the number of CTF files: second time around there is no loading! */
  numctf = load_ctflist();
  if (numctf < 1) {
    fprintf(stderr, "No CTF files found as arguments or in %s\n", CTFLIST_DB);
    exit(1);
  }

  /* Initialise the TDN structures */
  init_libtdn(p);

  /* Process each CTF file in the list */
  for (i = 1; i < numctf; i++) {

    C = ctfopen(get_ctfname(i));
    if (C == NULL) {
      fprintf(stderr, "Can't open CTF file %s\n", get_ctfname(i)); exit(1);
    }

    /* Mark when we reach the last file */
    if (i== (numctf-1))
      p->flags |= CTP_LASTFILE;

    foundruns = find_runs_from_ctf(i, p);
    ctfclose(C);
  }

  if (quiet) {
    /* Count the number of runs ourselves */
    for (run= foundruns; run != NULL; run = run->next)
      if (run->length >= p->tuple_size) runcount++;
    printf("Number of runs found:       %d\n", runcount);
    printf("Number of TDNs used:        %d\n", p->tdncount);
    printf("Number of TDN comparisons:  %d\n", p->tdncmpcnt);
  } else
    print_listruns(foundruns, p);

#ifdef FREE_MEM
  init_ctfparams(p);		/* free() any malloc()d memory */
  free(p);
#endif
  exit(0);
}
