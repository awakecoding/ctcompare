/* perlexer.l: Lex code to help build a Perl token file.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 *
 * I recognise most of the operators, but I don't look for keywords,
 * these are recognised as identifiers instead.
 *
 * $Revision: 1.1 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%

"++"		myputc(INCR, zout);
"--"		myputc(DECR, zout);
"&&"		myputc(ANDAND, zout);
"||"		myputc(OROR, zout);
"+="		myputc(PLUSassign, zout);
"+"		myputc(PLUS, zout);
"-="		myputc(MINUSassign, zout);
"-"		myputc(MINUS, zout);
"*="		myputc(MULTassign, zout);
"*"		myputc(MULT, zout);
"/="		myputc(DIVassign, zout);
"/"		myputc(DIV, zout);
"%="		myputc(MODassign, zout);
"&="		myputc(ANDassign, zout);
"|="		myputc(ORassign, zout);
"^="		myputc(XORassign, zout);
"=="		myputc(EQ, zout);
"!="		myputc(NE, zout);
"<="		myputc(LE, zout);
">="		myputc(GE, zout);
"<<"		myputc(LS, zout);
">>"		myputc(RS, zout);
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
"->"            myputc(ARROW, zout);
"$"             myputc(DOLLAR, zout);
"@"             myputc(ATSIGN, zout);
"=~"            myputc(EQTILDE, zout);
"`"             myputc(BACKTICK, zout);
"\\"		myputc(BACKSLASH, zout);
"#"		{ if (inside_comment==0) inside_comment=1; }

[a-zA-Z_][a-zA-Z_0-9]*		myputtokhash(IDENTIFIER, yytext, zout);

[a-zA-Z_][a-zA-Z_0-9]*:		myputtokhash(LABEL, yytext, zout);

0[xX][0-9a-fA-F]+ |
[1-9][0-9]*	  |
0[0-7]*				myputtokhash(INTVAL, yytext, zout);

\"[^\"]*\"			myputtokhash(STRINGLIT, yytext, zout);

\'[^\']*\'			myputtokhash(STRINGLIT, yytext, zout);

\n			{ myputc(LINE, zout);
			  if (inside_comment==1) inside_comment=0;
			}

[ \t]*		;	/* Ignore tabs and spaces.  */

.		;	/* Ignore all unrecognised tokens */
			{ fprintf(stderr, "unrecognised token %s\n",yytext);
			}

%%

int yywrap()
{
    return 1;
}

void perl_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
