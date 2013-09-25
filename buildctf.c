/*
 * buildctf: Build a C token file given a directory containing C code.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.29 $
 */

#include <sys/types.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "libctf.h"

void usage(void)
{
  fprintf(stderr, "Usage: buildctf [-s size_in_bytes] [-d] directory outputfile\n"); exit(1);
}

int main(int argc, char *argv[])
{
  int write_to_db=0;
  int ssize=0;
  int err;

  /* Get the optional argument */
  if (argc < 3) usage();

  if (!strcmp(argv[1], "-s")) {
    ssize=atoi(argv[2]);
    argc-=2; argv+=2;
  }

  if (!strcmp(argv[1], "-d")) {
    write_to_db=1;
    argc--; argv++;
  }

  /* Check the mandatory arguments */
  if (argc != 3) usage();

  /* Do the tokenising */
  err = tokenise_tree(argv[1], argv[2], write_to_db,ssize);
  if (err == -1) {
    fprintf(stderr, "Error tokenising %s to %s: %s\n", argv[1], argv[2],
	    strerror(errno));
    exit(1);
  }
  exit(0);
}
