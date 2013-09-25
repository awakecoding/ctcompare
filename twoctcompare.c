/* Specialised version of ctcompare. Tokenise two individual files,
 * compare them, and print out the results.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.5 $
 */

#include <sys/types.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>		/* For setrlimit */
#include <sys/resource.h>
#include "libctf.h"
#include "liblexer.h"
#include "libtokens.h"

extern FILE *zout;

int main(int argc, char *argv[])
{
  FILE *zin;
  char outname1[300];
  char outname2[300];
  int i, numctf;
  Ctfhandle *C;
  Ctfparam *p;
  Run *foundruns = NULL;	/* Matching runs of code that were found */

  if (argc != 3) {
    fprintf(stderr, "Usage: twoctcompare file1 file2\n");
    exit(1);
  }

  struct rlimit R;		/* Give us 400M of memory only */
  R.rlim_cur= 400 * 1024 * 1024;
  R.rlim_max= 400 * 1024 * 1024;
  setrlimit(RLIMIT_AS, &R);

  R.rlim_cur= 10;
  R.rlim_max= 10;
  setrlimit(RLIMIT_CPU, &R);	/* and 10 seconds to run */

  /* Create two temp filenames */
  snprintf(outname1, 300, "/tmp/ctc1_%d.ctf", getpid());
  snprintf(outname2, 300, "/tmp/ctc2_%d.ctf", getpid());

  /* Tokenise the first file */
  zin = fopen(argv[1], "r");
  if (zin == NULL) {
    fprintf(stderr, "Cannot open %s\n", argv[1]); exit(1);
  }
  fclose(zin);
  zout = fopen(outname1, "w");
  if (zout == NULL) {
    fprintf(stderr, "Cannot write %s\n", outname1); exit(1);
  }

  /* Output the ctf header and version 2.1 */
  fputs("ctf2.1", zout);
  tokenize(argv[1]);
  putc(EOFTOKEN, zout);
  fclose(zout);

  /* Tokenise the second file */
  zin = fopen(argv[2], "r");
  if (zin == NULL) {
    fprintf(stderr, "Cannot open %s\n", argv[2]); exit(1);
  }
  fclose(zin);
  zout = fopen(outname2, "w");
  if (zout == NULL) {
    fprintf(stderr, "Cannot write %s\n", outname2); exit(1);
  }

  /* Output the ctf header and version 2.1 */
  fputs("ctf2.1", zout);
  tokenize(argv[2]);
  putc(EOFTOKEN, zout);
  fclose(zout);

  /* Initialise the params structure */
  p = init_ctfparams(NULL);
  if (p == NULL) {
    fprintf(stderr, "Unable to initialise ctfparams structure\n"); exit(1);
  }

  /* Make sure that we do the heuristics to remove unwanted matches */
  p->flags |= CTP_COMPHEUR;

#if 0
  /* Because we are doing only 2 files, we can afford to turn on
   * isomorphic comparisons.
   */
  p->flags |= CTP_ISOMORPHIC;
  p->isomorph_count_threshold = 3;
#endif

  add_ctffile(outname1, 0);
  add_ctffile(outname2, 0);

  /* Get the number of CTF files: second time around there is no loading! */
  numctf = load_ctflist();

  /* Process each CTF file in the list */
  for (i = 1; i < numctf; i++) {

    C = ctfopen(get_ctfname(i));
    if (C == NULL) {
      fprintf(stderr, "Can't open CTF file %s\n", get_ctfname(i)); exit(1);
    }

    foundruns = find_runs_from_ctf(i, p);
    ctfclose(C);
  }

  print_listruns(foundruns, p);
  unlink(outname1);
  unlink(outname2);
  exit(0);
}
