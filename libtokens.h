/*
 * libtokens.h: Tokens from all languages for which we have parsers.
 *
 * Copyright (c) Warren Toomey, under the GPL3 license.
 * 
 * $Revision: 1.27 $
 */

#ifndef LIBTOKENS_H
#define LIBTOKENS_H

/* File structure tokens */
#define EOFTOKEN	0
#define FILENAME	9
#define LINE		10

/* C tokens */
#define RSassign	13
#define DIVassign	32
#define NOT		33
#define STRINGLIT	34
#define ARROW		35
#define INCR		36
#define MOD		37
#define AND		38
#define CHARCONST	39
#define OPENPAREN	40
#define CLOSEPAREN	41
#define MULT		42
#define PLUS		43
#define COMMA		44
#define MINUS		45
#define DOT		46
#define DIV		47
#define DECR		48
#define ANDAND		49
#define OROR		50
#define PLUSassign	51
#define MODassign	52
#define MINUSassign	53
#define ANDassign	54
#define MULTassign	55
#define ORassign	56
#define INTVAL		57
#define COLON		58
#define SEMICOLON	59
#define LT		60
#define EQUALS		61
#define GT		62
#define QUESTION	63
#define NE		64
#define LE		65
#define CASE		66
#define CHAR		67
#define CONST		68
#define CONTINUE	69
#define DEFAULT		70
#define DO		71
#define ELLIPSIS	72
#define DOUBLE		73
#define ELSE		74
#define ENUM		75
#define EXTERN		76
#define FLOAT		77
#define FOR		78
#define GOTO		79
#define IF		80
#define INT		81
#define LONG		82
#define REGISTER	83
#define RETURN		84
#define SHORT		85
#define SIGNED		86
#define SIZEOF		87
#define STATIC		88
#define STRUCT		89
#define SWITCH		90
#define OPENBRACKET	91
#define BACKSLASH	92
#define CLOSEBRACKET	93
#define CARET		94
#define IDENTIFIER	95
#define LABEL		96
#define TYPEDEF		97
#define UNION		98
#define UNSIGNED	99
#define VOID		100
#define VOLATILE	101
#define WHILE		102
#define HASHdefine	103
#define HASHelif	104
#define HASHelse	105
#define HASHendif	106
#define HASHerror	107
#define HASHifdef	108
#define HASHif		109
#define HASHifndef	110
#define HASHinclude	111
#define HASHline	112
#define HASHpragma	113
#define HASHundef	114
#define HASHwarning	115
#define XORassign	116
#define EQ		117
#define BREAK		118
#define GE		119
#define LS		120
#define RS		121
#define LSassign	122
#define OPENCURLY	123
#define OR		124
#define CLOSECURLY	125
#define TILDE		126

/* Java tokens */
#define ABSTRACT	127
#define BOOLEAN		128
#define BYTE		129
#define EXTENDS		130
#define FINAL		131
#define FINALLY		132
#define IMPLEMENTS	133
#define IMPORT		134
#define INSTANCEOF	135
#define INTERFACE	136
#define NATIVE		137
#define NEW		138
#define JAVANULL	139
#define PACKAGE		140
#define PRIVATE		141
#define PROTECTED	142
#define PUBLIC		143
#define STRICTFP	144
#define SUPER		145
#define SYNCHRONIZED	146
#define THIS		147
#define THROW		148
#define THROWS		149
#define TRANSIENT	150
#define TRY		151
#define URS		152
#define URSassign	153

/* Python tokens */
#define INDENT          154
#define OUTDENT         155
#define XOR             156
#define CLASS           157
#define FUNCTION        158
#define IS              159
#define NONE            160
#define EXCEPT          161
#define AS              162
#define ASSERT          163
#define DEL             164
#define ELIF            165
#define EXEC            166
#define FROM            167
#define GLOBAL          168
#define IN              169
#define LAMBDA          170
#define PASS            171
#define PRINT           172
#define WITH            173
#define YIELD           174
#define TYPE            175
#define EXPassign       176
#define EXP             177
#define INTDIVassign    178
#define INTDIV          179
#define DECORATOR       180

/* Perl tokens */
#define DOLLAR		181
#define ATSIGN		182
#define EQTILDE		183
#define BACKTICK	184


/* Inline functions */
static inline uint32_t get_ctf_offset(Ctfhandle * ctf)
{
  return (ctf->cursor - ctf->start);
}

#endif /* LIBTOKENS_H */
