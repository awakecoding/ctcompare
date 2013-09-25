/* hexlexer.l: Lex code to tokenise a file created by the hd(1) or hexdump(1)
 * command. This allows binary files to be compared, once they are converted
 * to hex.
 *
 * It's ugly, but we simply use 16 existing tokens to match the hex
 * digits. We can't use the 256 token values for a hex pair, because
 * some tokens have special meaning, e.g. EOFTOKEN, FILENAME and LINE
 *
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * $Revision: 1.3 $
 */
%{
#include "liblexer.h"
#define YY_NO_INPUT
%}

%%


[ \t]*		;	/* Ignore tabs and spaces.  */

^[0-9a-f]{8}	;	/* Skip the leading hex address */

\|.*$		;	/* Skip stuff after a | symbol */

0   myputc(DECR, zout);
1   myputc(ANDAND, zout);
2   myputc(OROR, zout);
3   myputc(PLUSassign, zout);
4   myputc(MODassign, zout);
5   myputc(MINUSassign, zout);
6   myputc(ANDassign, zout);
7   myputc(MULTassign, zout);
8   myputc(ORassign, zout);
9   myputc(ORassign, zout);
a   myputc(TYPEDEF, zout);
b   myputc(UNION, zout);
c   myputc(UNSIGNED, zout);
d   myputc(VOID, zout);
e   myputc(VOLATILE, zout);
f   myputc(WHILE, zout);

\n		myputc(LINE, zout);	/* Newline characters */

.		;	/* Ignore all unrecognised tokens */




%%

int yywrap()
{
    return 1;
}

void hex_dummy()
{
    /* Get rid of the " `yyunput' defined but not used " warning. */
    unput(' ');
}
