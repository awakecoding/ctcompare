/*
 lexer.c - functions and variables shared between several lexer implementations 
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * Copyright (c) Red Hat, Inc - author Florian Festi <ffesti@redhat.com>
 * Much of this file comes from cslang.l, a part of the CSlang program
 * which is Copyright (C) 1995 Tudor Hulubei under the GPL.
 *
 * $Revision: 1.7 $
 */

#include <stdlib.h>
#include <time.h>
#include "liblexer.h"

void py_restart(FILE * input_file);
int py_lex(void);
void c_restart(FILE * input_file);
int c_lex(void);
void j_restart(FILE * input_file);
int j_lex(void);
void hex_restart(FILE * input_file);
int hex_lex(void);
void txt_restart(FILE * input_file);
int txt_lex(void);
void asm_restart(FILE * input_file);
int asm_lex(void);
void perl_restart(FILE * input_file);
int perl_lex(void);

void reset_lexer(void);
void output_filename(char *name);

int inside_comment = 0;
int indent = 0;

int numericvalcount=0;
int lastnumericvalue=0;

#define endswith(suff)	!strcmp(suff, file + strlen(file) - strlen(suff))

/* See if this is a text file */
int is_textfile(char *file)
{
  int len;
  char c1='x', c2, c3;

  if (endswith(".txt") || endswith(".TXT") || endswith(".tex") ||
      endswith(".mm")  || endswith(".me")  || endswith(".ms")  ||
      endswith(".mc")  || endswith(".el")  || endswith(".t"))
    return(1);

  /* See if it ends with .[0-9] or .[0-9][a-z] */
  len= strlen(file);
  if (len<2) return(0);
  if (len>2) c1= file[len-3];
  c2= file[len-2];
  c3= file[len-1];

  if (c1 == '.' && c2 >= 'a' && c2 <= 'z' && c3 >= '0' && c3 <= '9') return(1);
  if (c2 == '.' && c3 >= '0' && c3 <= '9') return(1);
  return(0);
}

void tokenize(char *file)
{
  FILE *zin;

  /* fast check for the most common suffixes
   * This code comes from Eric Raymond's comparator 1.0 source code,
   * which is (c) Eric Raymond under the GPL.
   */

  /* Seed the random number generator */
  srand(time(NULL));

  if (endswith(".o") || endswith("~") || endswith(".pyc"))
    return;
  else if (strstr(file, "CVS") || strstr(file, "RCS") ||
	   strstr(file, "SCCS") || strstr(file, ".git") ||
	   strstr(file, ".hg"))
    return;

  zin = fopen(file, "r");
  if (zin == NULL)
    return;

  reset_lexer();

  if (endswith(".c") || endswith(".h") ||
      endswith(".C") || endswith(".H") ||
      endswith(".C++") || endswith(".Cpp")) {
    output_filename(file);
    c_restart(zin);
    c_lex();
  } else if (endswith(".s") || endswith(".S")) {
    output_filename(file);
    asm_restart(zin);
    asm_lex();
  } else if (endswith(".py") || endswith(".PY")) {
    output_filename(file);
    py_restart(zin);
    py_lex();
  } else if (endswith(".java") || endswith(".JAVA")) {
    output_filename(file);
    j_restart(zin);
    j_lex();
  } else if (endswith(".hex") || endswith(".HEX")) {
    output_filename(file);
    hex_restart(zin);
    hex_lex();
  } else if (endswith(".pl") || endswith(".pm") || endswith(".perl")) {
    output_filename(file);
    perl_restart(zin);
    perl_lex();
  } else if (is_textfile(file)) {
    output_filename(file);
    txt_restart(zin);
    txt_lex();
  }
  fclose(zin);
}

#undef endswith

void myputc(char ch, FILE * f)
{
  if (inside_comment && (ch != LINE))
    return;
  putc(ch, f);
}

void myputindent(size_t depth, FILE * f)
{
  size_t i;
  /* printf("INDENT %i %i", depth, indent); */
  if (inside_comment)
    return;
  for (i = indent; i < depth; i++)
    myputc(INDENT, f);
  for (i = indent; i > depth; i--)
    myputc(OUTDENT, f);
  indent = depth;
}

/* CCITT CRC-16 hash:
 * returns a 16-bit number
 * for the input string.
 */
unsigned int get_hashval(char *t)
{
  unsigned int crc = 0;

  while (*t != '\0') {
    crc = (unsigned char) (crc >> 8) | (crc << 8);
    crc ^= *t;
    crc ^= (unsigned char) (crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;
    t++;
  }
  /* printf("Got %d for %s\n", h, oldt); */
  return (crc & 0xffff);
}


/* This function called at the beginning of each input file */
void reset_lexer(void)
{
  inside_comment = 0;
  indent = 0;
  numericvalcount=0;
  lastnumericvalue=0;
}


/* Given a STRINGLIT, CHARCONST, INTVAL, IDENTIFIER or LABEL,
 * output the token followed by a 16-bit hash of the value.
 */
void myputtokhash(char ch, char *text, FILE * f)
{
  unsigned int hash;
  char *pos;

  /* Output the token to start with */
  if (inside_comment)
    return;
  putc(ch, f);

  hash = get_hashval(text);

  /* Break runs of 15 or more literal ints with the same value */
  if (ch==INTVAL)
  {
    if (hash==lastnumericvalue) /* Increment if the same litval, else reset */
      numericvalcount++;
    else
      numericvalcount=0;

    lastnumericvalue= hash;     /* Save the litval for next time */

    /* If we've seen 15 or more of them, throw in a random hashval */
    if (numericvalcount>=15) {
      /* printf("15 in a row for %s %d\n", text, hash); */
      hash= rand();
      numericvalcount=0;
    }
  }

  /* printf("Hash %04x %s\n", hash, text); */
  putc(hash / 256, f);
  putc(hash & 255, f);
  for (pos = strchr(text, '\n'); pos != NULL; pos = strchr(++pos, '\n')) {
    /* printf("NEWLINE in COMMENT: %s", text); */
    putc(LINE, f);
  }
}
