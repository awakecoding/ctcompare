/* clexer.l: Lex code to help build a C token file.
 * Copyright (c) Warren Toomey, under the GPL3 license.
 *
 * Much of this file comes from cslang.l, a part of the CSlang program
 * which is Copyright (C) 1995 Tudor Hulubei under the GPL.
 *
 * $Revision: 1.21 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%

break		myputc(BREAK, zout);
case		myputc(CASE, zout);
char		myputc(CHAR, zout);
const		myputc(CONST, zout);
continue	myputc(CONTINUE, zout);
default		myputc(DEFAULT, zout);
do		myputc(DO, zout);
double		myputc(DOUBLE, zout);
else		myputc(ELSE, zout);
enum		myputc(ENUM, zout);
extern		myputc(EXTERN, zout);
float		myputc(FLOAT, zout);
for		myputc(FOR, zout);
goto		myputc(GOTO, zout);
if		myputc(IF, zout);
int		myputc(INT, zout);
long		myputc(LONG, zout);
register	myputc(REGISTER, zout);
return		myputc(RETURN, zout);
short		myputc(SHORT, zout);
signed		myputc(SIGNED, zout);
sizeof		myputc(SIZEOF, zout);
static		myputc(STATIC, zout);
struct		myputc(STRUCT, zout);
switch		myputc(SWITCH, zout);
typedef		myputc(TYPEDEF, zout);
union		myputc(UNION, zout);
unsigned	myputc(UNSIGNED, zout);
void		myputc(VOID, zout);
volatile	myputc(VOLATILE, zout);
while		myputc(WHILE, zout);

#[ \t]*define	myputc(HASHdefine, zout);
#[ \t]*elif	myputc(HASHelif, zout);
#[ \t]*else	myputc(HASHelse, zout);
#[ \t]*endif	myputc(HASHendif, zout);
#[ \t]*error	myputc(HASHerror, zout);
#[ \t]*ifdef	myputc(HASHifdef, zout);
#[ \t]*if	myputc(HASHif, zout);
#[ \t]*ifndef	myputc(HASHifndef, zout);
#[ \t]*include	myputc(HASHinclude, zout);
#[ \t]*line	myputc(HASHline, zout);
#[ \t]*pragma	myputc(HASHpragma, zout);
#[ \t]*undef	myputc(HASHundef, zout);
#[ \t]*warning	myputc(HASHwarning, zout);


"->"		myputc(ARROW, zout);
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
"<<="		myputc(LSassign, zout);
">>="		myputc(RSassign, zout);
"..."		myputc(ELLIPSIS, zout);
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

0[xX][0-9a-fA-F]+[uUlL]* |
[1-9][0-9]*[uUlL]*	 |
0[0-7]*[uUlL]*			myputtokhash(INTVAL, yytext, zout);

'\\x[0-9a-fA-F]+' |
'\\[0-7]+' 			myputtokhash(CHARCONST, yytext, zout);

'.'				myputtokhash(CHARCONST, yytext, zout);

'\\.' 				myputtokhash(CHARCONST, yytext, zout);

\"[^\"]*\"			myputtokhash(STRINGLIT, yytext, zout);

\n				myputc(LINE, zout);

[ \t]*		;	/* Ignore tabs and spaces.  */

.		;	/* Ignore all unrecognised tokens */

%%

int yywrap()
{
    return 1;
}

void c_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
