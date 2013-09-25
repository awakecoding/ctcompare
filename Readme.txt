Code Token Comparator Version 3.2
(c) 2007-2011, Warren Toomey wkt@tuhs.org
under the GPL v3 license
Introduction
The purpose of this set of programs is to allow you to compare several sets of C, Java, Python or Perl code trees on a token basis, rather than on a line by line basis. The programs help to identify similarities between snippets of code in the trees, even if the actual lines are dissimilar. For background on how the tools work, see the papers, slides and audio at this website:
http://minnie.tuhs.org/Programs/Ctcompare
Changelog
3.2: Memory and performance inprovements, up to 2 times faster than the 3.1 codebase. Several other lexers. New flags: -s to split large code trees, and -u to break up num,num,num,num runs in CTF files so that these runs of tokens are not compared.
3.1: A complete rewrite of the code, making it 5 to 10 times faster than the 2.x codebase. The code is now structured as a library with command-line front-end tools.
Building the Tools, Configuring the Makefile
This is still a research-oriented work in progress, so there's no ./configure; make install here. You will need: make, lex or flex, a C Compiler, the fts(3) function and a FreeBSD or Linux box.
The Makefile is set up to for Ubuntu and to do optimised compiling. However, read through the Makefile before you run 'make' to make the three programs: buildctf, detok and ctcompare.
If the make fails, you're on your own! The programs are small enough to make the chore of fixing them not too difficult. The code compiles and runs on FreeBSD 8.0, and Ubuntu Linux 10.04. At present, the code expects a 32-bit platform; it may not compile and run on a 64-bit platform (but patches to do so are most welcome!).
You can either leave the three programs here, or you can copy them into a directory in your $PATH, e.g. /usr/local/bin. I assume below that you are going to run them from this directory.
Tokenising a Code Tree
If you have a C, Java, Python or Perl code tree in a directory (and subdirectories therein) called /some/where/code and you want to tokenise the files in the tree and store the result in a "Code Token File" (CTF file) called mytree.ctf, you would do:
  $ ./buildctf  /some/where/code   mytree.ctf
The directory name can be relative or absolute (i.e. it doesn't have to start with a /). However note that other tools like ctcompare may need to open the source files to print out snippets of code. If you choose a relative directory name, you will need to run ctcompare in the same directory that you ran buildctf.
What Does a CTF File Reveal About the Source Code?
The aim of the CTF file format is to allow a compact representation of a code tree to be exported in a way that allows similarities to be found, but in such a way that the complete source code is not revealed. This should allow proprietary code trees to be exported in CTF format.
A CTF file will reveal this about your source code tree:
the name of every source file in your tree
the last modification time for each source file
a syntactical representation of each source file, i.e. syntax elements such as () [] {} -> == += #include etc.
where literal elements occur in the file's syntax; this includes labels, string literals, character literals, numeric literals, and identifiers
a 16-bit hashed value of each literal element
Note that the actual values for literal constants and identifiers are not revealed, i.e. if you use the variable foobar, then this will not be revealed. However, a 16-bit hash of the word "foobar" is revealed, so a dictionary attack will provide a list of possible variable names that produce the same hash value. A 16-bit hash was purposefully chosen to increase the likelihood of collisions, thus reducing the effectiveness of a dictionary attack. Full details of the CTF file format are given below.
Viewing the Tokenised Results File
To view the contents of the resulting CTF file, use the detok command:
  $ ./detok mytree.ctf | less
This shows you the basic code tokens for each file in your source tree. An example CTF file is provided in the ctcompare tarball.
Keeping a List of CTF Files
The tool to find code similarities, ctcompare, selects which CTF files to cross-compare as follows:
the text file ctflist.db in the current directory is loaded: each line contains the relative or absolute pathname of a CTF file which is to be compared. No error will occur if the ctflist.db file does not exist.
extra CTF files not named in the ctflist.db file can be named as command-line arguments to ctcompare.
You can hand-edit the ctflist.db file; alternatively, when you tokenise a source tree using buildctf, you can specify the -d flag to append the name of the created CTF file to the ctflist.db file.
Comparing Source Trees
With all the source tree you want tokenised into CTF files, and the list of CTF files stored in ctflist.db, you can now search for code similarities. This is done with the ctcompare tool:
  $ ./ctcompare | less
will compare all the trees named in the ctflist.db file, and print out the source files from different trees which have 16 or more matching tokens, and literal elements (e.g. identifiers) with the same hash value. The result has 3 tab-separated columns:
#tokens_in_common filename:start-end_line filename:start-end_line
The 1st column is the number of source code tokens found to be consecutive. The 2nd column names a file, start and end line of the run of matching tokens. The 3rd column names a file, start and end line of the run of matching tokens.
Options to Ctcompare
Ctcompare has several options:
Usage: ctcompare [-n nnn] [-rstxiaq] [-I nnn] [CTF file] [CTF file...] 
-n nnn: set the minimum matching run length to nnn
-r: print results sorted by run length descending
-t: show matching tokens when a match is found
-x: show matching source lines when a match is found
-s: similar to -x, but print results side by side on the same line
-i: enable isomorphic code comparison, see below
-I nnn: limit the # of isomorphic relations to nnn, implies -i
-a: show all matches even if they are in the same source tree
-q: quiet, only print the number of matches found
-u: break up num,num,num,num runs in CTF files so that these runs of tokens are not compared
CTF file arguments augment those in the ctflist.db file
Isomorphic Code Comparison
The default code comparison is an exact comparison: not only must lexical elements (such as () {} [] ++ += etc.) match, but variable names must also match. Ctcompare also supports "isomorphic" code comparison with the -i and -I nnn options.
Code that is isomorphic can be detected if there is a 1-to-1 relationship between identifiers. For example, the following two functions perform the same action although the variable names are different.
   int maxofthree(int x, int y, int z)
   {
     if ((x>y) && (x>z)) return(x);
     if (y>z) return(y);
     return(z);
   }          

   int bigtriple(int b, int a, int c)
   {
     if ((b>a) && (b>c)) return(b);
     if (a>c) return(a);
     return(c);
   }
Ctcompare, with the -i option, will perform isomorphic code comparison and will find code similarities such as the one above. Note, however, that this does slow the operation down significantly, and there will be many false positives.
By default, ctcompare will give up on a potential run when the number of isomorphic relations exceeds 3, e.g. in the above example there are 3 relations: x <=> b, y <=> a and z <=> c. If a new variable called "fred" appeared in the top function that was isomorphic to a new variable "mary" in the bottom function, ctcompare would give up as that would be a 4th isomorphic relation. You can control the number of isomorphic relations with the -I nnn option, e.g.
  $ ./ctcompare -i | less       # isomorphic comparison with <=3 relations
  $ ./ctcompare -I 10 | less    # isomorphic comparison with <=10 relations
With high -I values (10 or more), you will start to see lots of false positives. I recommend that you start with a high token threshold such as -n 50 and the default -I 3 to find the largest matches with few isomorphic relations, and then iteratively lower -n and/or raise -I until you start to see lots of false positives.
Memory Issues
Ctcompare trades increased memory usage for faster results. When running, the memory usage will be 128 Mbytes + 20 bytes per token + 28 bytes per run found. To reduce runtime, allocated memory is not freed. To compare code trees totalling a million lines of code, for example, you will probably need a Gigabyte of free RAM or more.
Other Scripts
There are a couple of Perl scripts that help you deal with the output from ctcompare. Assume that you have done the following:
  $ ./ctcompare -i -n 30 -x > output
but the output has thousands of results, and many are ignorable, e.g.
41  Linux0.96c/chr_drv/keyboard.c:1083-1088  /Net2/sys/kern/tty_conf.c:63-69
        do_self,do_self,do_self,do_self,        /* 04-07 3 4 5 6 */
        do_self,do_self,do_self,do_self,        /* 08-0B 7 8 9 0 */
        do_self,do_self,do_self,do_self,        /* 0C-0F + ' bs tab */
        do_self,do_self,do_self,do_self,        /* 10-13 q w e r */
        do_self,do_self,do_self,do_self,        /* 14-17 t y u i */
=====================================
        enodev, enodev, enodev, enodev, enodev,         /* 1- defunct */
        enodev, enodev, enodev, enodev, enodev,
        enodev, enodev, enodev, enodev, enodev,         /* 2- defunct */
        enodev, enodev, enodev, enodev, enodev,
The Perl script filter_result can be used to filter out unwanted results from the output file. Usage is:
  $ ./filter_result filename minimum maximum [exclude regex pattern ...]
minimum is the minimum run size to show, default 16. maximum is the maximum run size to show, default infinity. If you set maximum to 0, it is interpreted as infinity. Exclude patterns apply only to lines in the output file which have source file called filename.
Now you can eliminate keyboard.c from the output, and show runs with 40 or more tokens:
  $ ./filter_result output 40 0 keyboard.c | less
A second Perl script takes the output from ctcompare, and shows the file pairs in the output that are most related, i.e. have the largest number of token runs. You can do this as follows:
  $ ./sum_filepairs output | less
to see output similar to this:
14600: Src/32V/sys/sys/tty.c  Src/V7/dev/tty.c
3630: Src/32V/sys/sys/mx2.c  Src/V7/dev/mx2.c
2573: Src/32V/sys/sys/sys1.c  Src/V7/sys/sys1.c
2511: Src/32V/sys/sys/bio.c  Src/V7/dev/bio.c
2300: Src/32V/sys/sys/mx1.c  Src/V7/dev/mx1.c
1939: Src/32V/sys/sys/sys4.c  Src/V7/sys/sys4.c
1938: Src/32V/sys/sys/prim.c  Src/V7/sys/prim.c
1926: Src/32V/sys/sys/sys2.c  Src/V7/sys/sys2.c
Note: both filter_result and sum_filepairs can read from standard input if "-" is given as the filename, so you can even do this if you wish:
  $ ./ctcompare -i -n 30 -x | ./filter_result - 40 0 keyboard.c | \
                                                    ./sum_filepairs - | less
Comparing Other Languages
At present ctcompare supports 4 languages: C, Java, Python and Perl. To add support for another language: choose up to 255 tokens (remember, each token is an octet), copy one of the existing lexers and modify it to perform the lexical analysis for the new language. Then modify lexer.c and the Makefile to use it.
Known Bugs
Internally, ctcompare uses hashing to speed the lookups of code similarities. This may lead to a very small number of false positives. In practice, you would have to wade through hundreds of thousands of real code similarities to find an incorrect code similarity reported by ctcompare.
Feedback and Support
I probably can't offer much in the way of support, as this is a tool to scratch my itch and to do some academic research. However, I am interested in your feedback and bug fixes :-) Please e-mail me with any comments at wkt@tuhs.org.
I am especially interested in CTF files of proprietary UNIX source trees, especially for SysVR4 onwards. If you can provide any of these, I wouldbe most grateful.
Related Programs
Eric Raymond has written a tool called comparator, available at http://www.catb.org/~esr/comparator/. It works as follows:
[comparator] works by first chopping the specified trees into overlapping shreds (by default 3 lines long) and computing a hash of each shred. The resulting list of shreds is examined and all unique hashes are thrown out. Then comparator generates a report listing all cliques of lines with duplicate hashes, with overlapping ranges merged. ... A consequence of the method is that comparator will find common code segments wherever they are. It is insensitive to rearrangements of the source trees.
Dick Grune has written a token-based tool called SIM, available at http://www.cs.vu.nl/~dick/sim.html. It uses tokens in the same way as ctcompare, but does not seem to use literal or identifer values in the code comparison.
If anybody knows of any other freely available tools dealing with code comparison, I would be grateful if you e-mailed me the details.
Using libctf.a
The 3.x codebase is now organised as a library libctf.a and header file libctf.h, with the front-end programs buildctf, detok and ctcompare. I haven't written any good documentation for the library yet, for for now please read the header file and look at the source code for the front-end programs to give you ideas on how to use the library. Please bug me with question, as that will encourage me to write the doc file.
CTF File Format
The aim of the CTF file format is to allow a representation of a code tree to be exported in a way that allows similarities to be found, but in such a way that the complete source code is not revealed. This should allow proprietary code trees to be exported in CTF format.
A CTF file contains:
strings, which occur in normal text format and are terminated with a 0x00 octet, as per C,
single-octet tokens, and
multi-octet unsigned integer (16- or 32-bit) values in big-endian format.
Each CTF file begins with a string header indicating the CTF version. Currently, this is "CTF2.1" followed by a 0x00 octet.
After the CTF header, the CTF file is composed of a set of variable-length records, one for each file which has been tokenised. Each file record starts with the octet 0x09. This is followed by the file's 4-octet last modification timestamp (seconds since 1970) in big-endian format. This is followed by the filename in ASCII, terminated by a 0x00 octet. This is then followed by a set of tokens for the file.
Most tokens are 1 octet long, and are listed in libtokens.h. However, the following token values represent literal elements with values:
IDENTIFIER: 0x5f
INTVAL: 0x39
CHARCONST: 0x27
STRINGLIT: 0x22
LABEL: 0x60 
Each of these tokens is followed by 2 octets which represent, in big-endian format, the hashed value of that specific literal element in the file. For example, take the short C code:
        void print_word(char *str) {
          char *c;
          
          c=str;
          while ((c!='\0') && (c!=' ')) putchar(c++);
          putchar('\n');
        }
There are several identifiers and character constants in the code. As buildctf parses the file, it creates a 16-bit hash for each one:
Literal Value
Hash Value
print_word
13043
str
6928
c
23749
putchar
32886
Thus, the code would be represented in the CTF file as follows:
  void id13043 ( char * id6928 ) { 
    char * id23749; 

    id23749 = id6928; 
    while ((id23749 != 'c7388') && (id23749 != 'c20789')) id32886(id23749++); 
    id32886 ('c12652'); 
  } 
The set of tokens representing a single source file in the CTF file is terminated either by 0x09, i.e. the beginning of a new file, or the end of file token (0x00). It goes without saying that the values 0x09 and 0x00 do not represent actual source tokens. Similarly, the value 0x0A represents a newline in the source file, and does not represent an actual source token.
