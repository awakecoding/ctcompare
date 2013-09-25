/* asmlexer.l: Lex code to parse assembly files.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 *
 * Basically, skip comments and keep the rest. We treat
 * # and / as "comments to end of line", otherwise C comments.
 *
 * $Revision: 1.1 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%

"+"             myputc(PLUS, zout);
"-"             myputc(MINUS, zout);
"*"             myputc(MULT, zout);
"("		myputc(OPENPAREN, zout);
")"		myputc(CLOSEPAREN, zout);
"{"		myputc(OPENCURLY, zout);
"}"		myputc(CLOSECURLY, zout);
"["		myputc(OPENBRACKET, zout);
"]"		myputc(CLOSEBRACKET, zout);
"&"		myputc(AND, zout);
","		myputc(COMMA, zout);
";"		myputc(SEMICOLON, zout);
"<"		myputc(LT, zout);
"="		myputc(EQUALS, zout);
">"		myputc(GT, zout);
"?"		myputc(QUESTION, zout);
"!"		myputc(NOT, zout);
":"		myputc(COLON, zout);
"."		myputc(DOT, zout);
"%"		myputc(MOD, zout);
"^"		myputc(CARET, zout);
"|"		myputc(OR, zout);
"~"		myputc(TILDE, zout);
"\\"		myputc(BACKSLASH, zout);
"/*"		inside_comment=1;
"*/"		inside_comment=0;

[a-zA-Z_][a-zA-Z_0-9]*		myputtokhash(IDENTIFIER, yytext, zout);

[a-zA-Z_][a-zA-Z_0-9]*:		myputtokhash(LABEL, yytext, zout);

[0-9]+				myputtokhash(INTVAL, yytext, zout);

'.'                             myputtokhash(CHARCONST, yytext, zout);

\"[^\"]*\"                      myputtokhash(STRINGLIT, yytext, zout);

\n				myputc(LINE, zout);


#.*$		;	/* Comment to end of line, ignore */

\/.*$		;	/* Comment to end of line, ignore */

[ \t]*		;	/* Ignore tabs and spaces.  */

.		;	/* Ignore all unrecognised tokens */

%%

int yywrap()
{
    return 1;
}

void asm_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
