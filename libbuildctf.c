/*
 * libbuildctf: Build a C token file given a directory containing C code.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.33 $
 */

#include <sys/types.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fts.h>
#include <errno.h>
#include <sys/stat.h>
#include "liblexer.h"

FILE *zin, *zout;		/* XXX: Make these not globals */

void output_filename(char *name)
{
  struct stat sb;
  uint32_t timestamp = 0;

  /* Get the file's last modification time */
  if (stat(name, &sb) == 0)
    timestamp = sb.st_mtime;

  /* Output the FILENAME token, the timestamp, the filename and a NUL */
  fputc(FILENAME, zout);
  fputc((timestamp >> 24) & 0xff, zout);
  fputc((timestamp >> 16) & 0xff, zout);
  fputc((timestamp >> 8) & 0xff, zout);
  fputc(timestamp & 0xff, zout);
  fputs(name, zout);
  fputc('\0', zout);
}

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
int tokenise_tree(char *directory_name, char *output_file, int ondisk, int splitsize)
{
  struct stat sb;
  int err;
  char *dirlist[2];
  char outnamebuf[1024];	/* Used to generate output filenames */
  FTS *ftsptr;
  FTSENT *entry;
  int filenum=1;	/* Unique file number for each output file */
  int len;

  /* Check that we have an output file name */
  if (output_file == NULL) {
    errno = EINVAL; return (-1);
  }

  /* Find and remove any trailing .ctf */
  len= strlen(output_file);
  if (!strcmp(&(output_file[len-4]), ".ctf"))
    output_file[len-4]= '\0';

  /* Check that the directory exists and is a directory */
  if (directory_name == NULL) {
    errno = EINVAL; return (-1);
  }

  err = stat(directory_name, &sb);
  if (err < 0) return (-1);
  if (!S_ISDIR(sb.st_mode)) {
    errno = ENOTDIR; return (-1);
  }

  /* Search for files to tokenise */
  dirlist[0] = directory_name;
  dirlist[1] = NULL;
  ftsptr = fts_open(dirlist, FTS_LOGICAL, NULL);
  zout=NULL;

  /* Process each entry that is a file */
  while (1) {
    entry = fts_read(ftsptr);
    if (entry == NULL) break;
    if (entry->fts_info != FTS_F) continue;

    /* If we have reached or exceeded the splitsize for the
     * current output file, then close it.
     */
    if (splitsize>0 && zout!=NULL && (ftell(zout) >= splitsize)) {
      putc(EOFTOKEN, zout);
      fclose(zout);
      if (ondisk==1) add_ctffile(outnamebuf, 1);
      zout=NULL;
    }

    /* Open up the output file if we need to */
    if (zout==NULL) {
      if (splitsize>0)
        snprintf(outnamebuf, 1024, "%s%04d.ctf", output_file, filenum++);
      else
        snprintf(outnamebuf, 1024, "%s.ctf", output_file);

      zout = fopen(outnamebuf, "w");
      if (zout == NULL) return (-1);

      /* Output the ctf header and version 2.1 */
      fputs("ctf2.1", zout);
    }

    /* After all that rigmarole, now tokenise the source file found. */
    tokenize(entry->fts_accpath);
  }

  /* No source files left, so close the last output file */
  fts_close(ftsptr);
  putc(EOFTOKEN, zout);
  fclose(zout);

  if (ondisk==1) add_ctffile(outnamebuf, 1);

  return (0);
}
