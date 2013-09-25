/* pylexer.l: Lex code to help build a Python token file.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * Copyright (c) Red Hat, Inc  - author Florian Festi <ffesti@redhat.com>
 * Much of this file comes from cslang.l, a part of the CSlang program
 * which is Copyright (C) 1995 Tudor Hulubei under the GPL.
 *
 * $Revision: 1.4 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%

and		myputc(ANDAND, zout);
as		myputc(AS, zout);
assert		myputc(ASSERT, zout);
break		myputc(BREAK, zout);
class		myputc(CLASS, zout);
continue	myputc(CONTINUE, zout);
def		myputc(FUNCTION, zout);
del		myputc(DEL, zout);
elif		myputc(ELIF, zout);
else		myputc(ELSE, zout);
except		myputc(EXCEPT, zout);
exec		myputc(EXEC, zout);
finally		myputc(FINALLY, zout);
for		myputc(FOR, zout);
from		myputc(FROM, zout);
global		myputc(GLOBAL, zout);
if		myputc(IF, zout);
import 		myputc(IMPORT, zout);
in		myputc(IN, zout);
is		myputc(IS, zout);
lambda		myputc(LAMBDA, zout);
not		myputc(NOT, zout);
or		myputc(OROR, zout);
pass		myputc(PASS, zout);
print		myputc(PRINT, zout);
raise		myputc(THROW, zout);
return		myputc(RETURN, zout);
try		myputc(TRY, zout);
while		myputc(WHILE, zout);
with		myputc(WITH, zout);
yield		myputc(YIELD, zout);

int		myputc(INT, zout);
long		myputc(LONG, zout);
float		myputc(FLOAT, zout);
None		myputc(NONE, zout);
type		myputc(TYPE, zout);

"+="		myputc(PLUSassign, zout);
"+"		myputc(PLUS, zout);
"-="		myputc(MINUSassign, zout);
"-"		myputc(MINUS, zout);
"*="		myputc(MULTassign, zout);
"*"		myputc(MULT, zout);
"=**"		myputc(EXPassign, zout);
"**"		myputc(EXP, zout);
"/="		myputc(DIVassign, zout);
"/"		myputc(DIV, zout);
"//="		myputc(INTDIVassign, zout);
"//"		myputc(INTDIV, zout);
"%="		myputc(MODassign, zout);
"&="		myputc(ANDassign, zout);
"|="		myputc(ORassign, zout);
"^="		myputc(XORassign, zout);
"^"		myputc(XOR, zout);
"=="		myputc(EQ, zout);
"!="		myputc(NE, zout);
"<>"		myputc(NE, zout);
"<="		myputc(LE, zout);
">="		myputc(GE, zout);
"<<"		myputc(LS, zout);
">>"		myputc(RS, zout);
"<<="		myputc(LSassign, zout);
">>="		myputc(RSassign, zout);
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
"~"		myputc(NOT, zout);
":"		myputc(COLON, zout);
"."		myputc(DOT, zout);
"%"		myputc(MOD, zout);
"|"		myputc(OR, zout);
"\\"		myputc(BACKSLASH, zout);
@		myputc(DECORATOR, zout);

[a-zA-Z_][a-zA-Z_0-9]*		myputtokhash(IDENTIFIER, yytext, zout);

0[xX][0-9a-fA-F]+[uUlL]* |
[1-9][0-9]*[uUlL]*	 |
0[0-7]*[uUlL]*			myputtokhash(INTVAL, yytext, zout);

'\\x[0-9a-fA-F]+' |
'\\[0-7]+' 			myputtokhash(CHARCONST, yytext, zout);

\#.*$				;

[uU]?[rR]?'''('{0,2}[^']|\\')*''' |
[uU]?[rR]?'([^'\n]|\\')*'	          |
[uU]?[rR]?\"{3}(["]{0,2}[^"]|\\\")*\"{3} |
[uU]?[rR]?\"([^\"\n]|\\\")*\"	myputtokhash(STRINGLIT, yytext, zout); /* single and tripple quoted strings */

^[ ]*				myputindent((strlen(yytext))/4, zout);
^\t* 				myputindent(strlen(yytext), zout);

\n				myputc(LINE, zout);

[ \t]*				;       /* Ignore tabs and spaces.  */

. 				;	/* Ignore all unrecognised tokens */

%%

int yywrap()
{
    return 1;
}

void py_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
