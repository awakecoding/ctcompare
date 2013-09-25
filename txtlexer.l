/* txtlexer.l: Lex code to parse text files and other files like nroff
 * input. Essentially, strip everything but sequences of alphanumeric
 * characters, and remove \.[A-Za-z]+ from the beginning of the line
 * so as to lose most of the nroff macros.
 *
 * Copyright (c) Warren Toomey, under the GPL3 license.
 *
 * $Revision: 1.1 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%

[a-zA-Z]+		myputtokhash(IDENTIFIER, yytext, zout);

[0-9]+			myputtokhash(INTVAL, yytext, zout);

^\.[a-zA-Z]+	;	/* Ignore nroff macros */


\n			myputc(LINE, zout);

.		;	/* Ignore all other characters */

%%

int yywrap()
{
    return 1;
}

void txt_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
