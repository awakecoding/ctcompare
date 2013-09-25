# Compiler flags: debugging & profiling
#CFLAGS=-g -Wall -pg
#LDFLAGS=-pg
#
# Compiler flags: debugging
#CFLAGS=-g -Wall
#LDFLAGS=-m32 -g
#
# Compiler flags: optimised
CFLAGS=-O2 -Wall
LDFLAGS=-m32

# Uncomment this if you want the programs to
# free() memory: this will slow them down but
# it reduces the overall memory footprint
#CFLAGS+= -DFREE_MEM

# Lexers for C, Java and Python are now built in
LEXEROBJS = clexer.o jlexer.o pylexer.o hexlexer.o txtlexer.o asmlexer.o \
		perllexer.o
LEXERSRCS = clexer.c jlexer.c pylexer.c hexlexer.c txtlexer.c asmlexer.c \
		perllexer.c
LIBOBJS = libbuildctf.o libctflist.o liblexer.o libprintruns.o \
		libruns.o libtdn.o libtokens.o

CC=cc
VERS=3.2

all: buildctf detok ctcompare twoctcompare

libctf.a: Makefile $(LIBOBJS) $(LEXEROBJS)
	ar -rs libctf.a $(LIBOBJS) $(LEXEROBJS)

buildctf: Makefile buildctf.o libctf.a
	$(CC) -o buildctf $(LDFLAGS) buildctf.o libctf.a

ctcompare: Makefile ctcompare.o libctf.a
	$(CC) -o ctcompare $(LDFLAGS) ctcompare.o libctf.a

twoctcompare: Makefile twoctcompare.o libctf.a
	$(CC) -o twoctcompare $(LDFLAGS) twoctcompare.o libctf.a

detok: Makefile detok.o libctf.a
	$(CC) -o detok $(LDFLAGS) detok.o libctf.a

clexer.c: clexer.l
	lex -o$@ -Pc_ $<

jlexer.c: jlexer.l
	lex -o$@ -Pj_ $<

pylexer.c: pylexer.l
	lex -o$@ -Ppy_ $<

perllexer.c: perllexer.l
	lex -o$@ -Pperl_ $<

hexlexer.c: hexlexer.l
	lex -o$@ -Phex_ $<

txtlexer.c: txtlexer.l
	lex -o$@ -Ptxt_ $<

asmlexer.c: asmlexer.l
	lex -o$@ -Pasm_ $<

clean:
	rm -f buildctf detok ctcompare enhashctf showkeys ctcompare \
		twoctcompare *~ *.o *.a $(LEXERSRCS)

realclean: clean
	rm -f *.db

libctf.h: lctf.h hdr_doc.pl libbuildctf.c libtokens.c libruns.c
	./hdr_doc.pl lctf.h libctf.h libbuildctf.c libtokens.c libruns.c \
		libprintruns.c libctflist.c
