/*
 * lexer.h - functions & variables for several lexer implementations 
 *
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * Copyright (c) Red Hat, Inc - author Florian Festi <ffesti@redhat.com>
 * Much of this file comes from cslang.l, a part of the CSlang program
 * which is Copyright (C) 1995 Tudor Hulubei under the GPL2.
 *
 * $Revision: 1.4 $
 */

#ifndef LIBLEXER_H
#define LIBLEXER_H

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "libctf.h"
#include "libtokens.h"

void tokenize(char *filename);
void myputc(char ch, FILE * f);
void myputid(char ch, char *text, FILE * f);
void myputtokhash(char ch, char *text, FILE * f);
void myputindent(size_t depth, FILE * f);
extern FILE *zout;
extern int inside_comment;
extern int indent;

#endif /* LIBLEXER_H */
