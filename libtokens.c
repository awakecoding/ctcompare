/*
 * libtokens.c: Functions dealing with the token stream in a CTF file.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.32 $
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "libctf.h"
#include "libtokens.h"

/** Functions dealing with the token stream stored in a CTF file.
 *
 * ctfopen(): open the named CTF file for reading, checking the header
 * as well. Returns the Ctfhandle handle to the open file, or sets errno
 * and returns NULL on error.
 */
Ctfhandle *ctfopen(char *name)
{
  Ctfhandle *ctf;
  struct stat sb;

  if ((ctf = malloc(sizeof(*ctf))) == NULL) 
    return(NULL);

  if ((ctf->fd = open(name, O_RDONLY)) == -1)
    return(NULL);

  fstat(ctf->fd, &sb);
  ctf->start = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, ctf->fd, 0);
  if ((ctf->start == NULL) || (ctf->start == MAP_FAILED))
    return(NULL);

  /* Initialise ctf */
  ctf->cursor = ctf->start;
  ctf->end = ctf->start + sb.st_size;
  ctf->linenum = 1;

  /* Check the ctf header */
  if ((*(ctf->cursor++) != 'c') || (*(ctf->cursor++) != 't') ||
      (*(ctf->cursor++) != 'f') || (*(ctf->cursor++) != '2') ||
      (*(ctf->cursor++) != '.') || (*(ctf->cursor++) != '1')) {
    errno= EINVAL;
    return(NULL);
  }
  return (ctf);
}

/** ctfclose(): close an open Ctfhandle and free the Ctfhandle's memory.
 * Returns 0 if OK, -1 on error.
 */
int ctfclose(Ctfhandle * ctf)
{
  if (ctf == NULL) return (-1);
  int fd= ctf->fd;
  if (munmap(ctf->start, ctf->end - ctf->start) < 0) return (-1);
  free(ctf);
  return (close(fd));
}


/** get_token(): given a Ctfhandle and a file offset, return the next
 * token from the file at the given offset. The offset is updated to point
 * at the next token. Any id-value associated with the the token is
 * returned in the id parameter, or 0 if there is no value. Any filename
 * associated with a FILENAME token is returned in name, and the id
 * parameter is used to return the timestamp. On any error, -1 is returned.
 * The space for the filename is malloc'd here; the caller takes
 * responsibility for freeing it.
 */
int get_token(Ctfhandle * ctf, uint32_t * offset, uint32_t * id, char **name)
{
  unsigned int ch, token;
  uint32_t idvalue;

  /* Give up if ctf, ctf->start or offset don't exist */
  if ((ctf == NULL) || (ctf->start == NULL) || (offset == NULL))
    return (-1);

  /* EOF handling. */
  ctf->cursor = ctf->start + *offset;
  if (ctf->cursor >= ctf->end) return (-1);

  /* Get the token */
  token = *(ctf->cursor++);

  /* Set a default id value */
  if (id) *id = 0;

  /*
   * Deal with filenames and the id numbers for certain tokens
   */
  switch (token) {
  case STRINGLIT:
  case CHARCONST:
  case LABEL:
  case IDENTIFIER:
  case INTVAL:
    /* Read in 2 bytes to get the id value */
    idvalue = *(ctf->cursor++) << 8;
    idvalue += *(ctf->cursor++);
    if (id)
      *id = idvalue;
    break;

  case FILENAME:
    /* Read in 4 bytes to get the timestamp value */
    idvalue = *(ctf->cursor++) << 24;
    idvalue += *(ctf->cursor++) << 16;
    idvalue += *(ctf->cursor++) << 8;
    idvalue += *(ctf->cursor++);
    if (id)
      *id = idvalue;

    /* Pass back the pointer to the filename, and move the cursor up */
    if (name)
      *name = (char *) ctf->cursor;
    while (((ch = *(ctf->cursor++)) != '\0') && (ctf->cursor < ctf->end));
  }

  *offset = ctf->cursor - ctf->start;
  return (token);
}

char *tokstring[] = {
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 0 */
  "ERR ", "ERR ", "\n", "ERR ", "ERR ", ">>= ", "ERR ", "ERR ",	/* 8 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 16 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 24 */
  "/= ", "! ", "\"string\"", "->", "++ ", "% ", "& ", "'c'",	/* 32 */
  "( ", ") ", "* ", "+ ", ", ", "- ", ". ", "/ ",	/* 40 */
  "-- ", "&& ", "|| ", "++= ", "%= ", "-= ", "&= ", "*= ",	/* 48 */
  "|= ", "NUM", ": ", "; ", "< ", "= ", "> ", "? ",	/* 56 */
  "!= ", "<= ", "case ", "char ", "const ", "continue ", "default ", "do ",	/* 64 */
  "...", "double ", "else ", "enum ", "extern ", "float ", "for ", "goto ",	/* 72 */
  "if ", "int ", "long ", "register ", "return ", "short ", "signed ", "sizeof ",	/* 80 */
  "static ", "struct ", "switch ", "[ ", "\\", "] ", "^ ", "id",	/* 88 */
  "label: ", "typedef ", "union ", "unsigned ", "void ", "volatile ", "while ", "#define ",	/* 96 */
  "#elif ", "#else ", "#endif ", "#error ", "#ifdef ", "#if ", "#ifndef ", "#include ",	/* 104 */
  "#line ", "#pragma ", "#undef ", "#warning ", "~= ", "== ", "break ", ">= ",	/* 112 */
  "<< ", ">> ", "<<= ", "{ ", "| ", "} ", "~ ", "abstract ",	/* 120 */
  "boolean ", "byte ", "extends ", "final ", "finally ", "implements ", "import ", "instanceof ",	/* 128 */
  "interface ", "native ", "new ", "null ", "package ", "private ", "protected ", "public ",	/* 136 */
  "strictfp ", "super ", "synchronized ", "this ", "throw ", "throws ", "transient ", "try",	/* 144 */
  ">>> ", ">>>= ", "INDENT ", "OUTDENT ", "^ ", "class ", "def ", "is ",	/* 152 */
  "None ", "except ", "as ", "assert ", "del ", "elif ", "exec ", "from ",	/* 160 */
  "global ", "in ", "lambda ", "pass ", "print ", "with ", "yield ", "type ",	/* 168 */
  "**= ", "**", "//= ", "// ", "@ ", "$", "@", "=~ ",	/* 176 */
  "`", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 184 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 192 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 200 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 208 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 216 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 224 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 232 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ",	/* 240 */
  "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR ", "ERR "	/* 248 */
};

/** tok2str(): given a token value, return a pointer to a
 * string constant which represents that token value.
 * Returns NULL if the given token value does not exist.
 */
char *tok2str(unsigned int ch)
{
  if (ch >= 256) return(NULL);
  return (tokstring[ch]);
}

/** print_token(): given a token value, a line number, an identifier
 * value associated with the token, and a filename associated with the
 * token, print out a textual representation of that token. The function
 * performs no input error checking.
 */
void print_token(unsigned int ch, uint32_t linenum, uint32_t id, char *filename)
{
  time_t time;
  switch (ch) {
  case FILENAME:
    time = id;
    printf("\n\n%s:\t%s\n%5d:   ", filename, ctime(&time), linenum);
    break;
  case LINE:
    printf("\n%5d:   ", linenum);
    break;
  case IDENTIFIER:
  case INTVAL:
    printf("%s%d ", tokstring[ch], id);
    break;
  case STRINGLIT:
    printf("\"str%d\" ", id);
    break;
  case LABEL:
    printf("L%d: ", id);
    break;
  case CHARCONST:
    printf("'c%d' ", id);
    break;
  default:
    fputs(tokstring[ch], stdout);
  }
}
