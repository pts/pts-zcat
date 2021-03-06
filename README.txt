muZCat: portable and minimalistic Flate decompression filter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

FAQ
~~~
1. What's muZCat?
"""""""""""""""""
muZCat is Flate decompression filter that can decompress GZIP, ZIP and ZLIB
(PS /FlateEncode-d) files and concatenations of those. muZCat is implemented
in ANSI C, K&R C, Java, OC, Perl, Ruby, Python, ISO Standard Pascal and
PostScript. No external libraries (such as zlib) are required.

2a. What are the advantages of muZCat?
""""""""""""""""""""""""""""""""""""""
-- Small code size (both source and compiled).

-- Uses small data memory (<10000 bytes).

-- Small stack space required (<1000 bytes).

-- No external libraries required. Everything is contained in muzcat.c.

-- Available in multiple programming languages: C, Java, Perl and more.
   (The main file, muzcat.c supports multiple languages via CPP #ifdef's.)

-- Uses only system calls getchar() and putchar() (or their equivalents if
   language is not C).

-- Doesn't use pointers.

-- Doesn't use dynamic memory allocation.

-- Operates with numbers 0..65535 or -32768..32767. It is
   signedness-independent.

2b. What are the drawbacks of muZCat?
"""""""""""""""""""""""""""""""""""""
-- It is quite slow, around 7.35 times slower than GNU gunzip. See FAQ
   answer 3 for details.

-- There is no error reporting. The result is undefined on I/O errors.
   (Even an infinite loop and a segfault is possible.)

-- If the input is invalid, the result is undefined.
   (Even an infinite loop and a segfault is possible.)

-- CRC checks are skipped. Files are assumed to be correct and to have
   correct CRCs.

-- Although muZCat can decompress multiple files, file names are not
   preserved. Not even file boundaries are preserved: the output, STDOUT is
   the silent concatenation of the output files. (Improving this
   should be fairly easy, see muzcat.c.)

2c. How is muZCat different from other Flate decompressors?
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
Flate decompressors known to the author:

-- gunzip: GNU gunzip: the gunzip(1) utility, `gunzip -c', `gzip -cd', `zcat'.
-- unzip: Info-Zip's unzip: the unzip(1) utility.
-- funzip: Info-Zip's filter unzip: the funzip(1) utility.
-- pkunzip: PkWare's PkUnzip: pkunzip.exe.
-- FlateDecode: PostScript LanguageLevel3 /FlateDecode, in GNU Ghostscript
   (also in PDF)
-- rcm: Ron McFarland's IOCCC entry, rcm.c
-- miniunzip: ZIP uncopressor example program from zlib
-- gzinflate: PHP function gzinflate(), uses zlib
-- gzuncompress: PHP function gzuncompress(), uses zlib
-- muzcat: as obtained from `muzcat' muZCat

Compared features:

-- gz: can decompress gzip .gz files
-- zip1: can decompress the first file of the .zip file
-- zipall: can decompress all files of the .zip file
-- zipname: multiple files decompressed from .zip, with correct file names
-- fe: can decompress ZLIB (PostScript /FlateEncode-d) files
-- concat: can decompress the concatenation of multiple compressed files

Programs and features joined, grouped by program:

-- gunzip: gz, zip1
-- unzip: zip1, zipall, zipname
-- funzip: gz, zip1
-- pkunzip: zip1, zipall, zipname
-- FlateDecode: fe
-- rcm: gz
-- miniunzip: dip1, zipall, zipname
-- gzinflate: gz
-- gzuncompress: fe
-- muzcat: gz, zip1, zipall, fe, concat

3. How slow is muZCat?
""""""""""""""""""""""
Measured on an Intel Celeron 333MHz, with 128MB of memory, STDOUT redirected
to /dev/null, on the file

	linux-2.2.8.tar.gz
	compressed size:   13 808 890 bytes
	uncompressed size: 58 388 480 bytes
        compression ratio: 23.65%

gunzip:         4300 ms user time
muZCat 0.04:   31600 ms user time (  7.350 times slower than gunzip)
muZCat 0.06:   12900 ms user time (  3.000 times slower than gunzip)
statecat.ps: 1774510 ms user time (412.667 times slower than gunzip) (w/debug)
statecat.ps: 1071960 ms user time (249.293 times slower than gunzip)

It would be hard to make muZCat faster so it will be only around 1.5 times
slower than gunzip. The author of muZCat isn't planning to optimize it, but he
welcomes faster, working contributions.

4a. Who is responsible for muZCat?
""""""""""""""""""""""""""""""""""
muZCat was (re)written by Szab� P�ter <pts@fazekas.hu> in July 2001. He is
the author and the maintainer for muZCat. He welcomes e-mails about
improving the software. Please start the Subject: with `[muZCat] '.

The C code of muZCat and the Flate decompression routine is based on the work
of Ron McFarland <rcm@one.net>, in 1996. However, Ron's code has been highly
re-structured and rewritten.

There is no home page and no mailing list.

4b. Where can I download muZCat?
""""""""""""""""""""""""""""""""
The latest stable version is available from

	http://www.inf.bme.hu/~pts/muzcat-mini-latest.tar.gz

5. How to compile muZCat?
"""""""""""""""""""""""""
On MS-DOS or other exotic platforms you may have to enable binary file I/O
mode in C. This is left as a platform-specific exercise for you.

To compile muzcat in various languages, run the shell scripts:

	compile_ansic.sh	ANSI C
	compile_ansics.sh	ANSI C with signed integers
	compile_ansioc.sh	ANSI C, signed integers, simplest syntax
	compile_cpp.sh		standard C++
	compile_java.sh		Java
	compile_oc.sh		Obfuscated C (see also august.txt)
	compile_perl.sh		Perl5
	compile_ansicsys.sh	ANSI C, uses syscalls read(2) and write(2)
	compile_ansiclnx.sh	ANSI C, uses Linux syscalls, no libc!
	compile_ruby.sh		Ruby >=1.6.0
	compile_python.sh	Python >=1.5.1
	compile_isopascal.sh	ISO 7185:1991 Standard Pascal, doesn't work
	compile_freepascal.sh	Free Pascal 2.6, works.
	compile_turbopascal.sh  Turbo Pascal >=5.5.
	compile_xpascal.sh	For Turbo Pascal >=5.5.
	compile_xpascal_freepascal.sh	Doesn't work.
	compile_xpascal_gpc.sh	For GPC and Turbo Pascal >=5.5.
	compile_ps1.sh		PostScript LanguageLevel 1

5b. How to compile muZCat to be very small on Linux?
""""""""""""""""""""""""""""""""""""""""""""""""""""
Use compile_ansiclnx.sh, see FAQ answer 5 for more details.

Also see compile_xstatic.sh for compiling with pts-xstatic.

The resultant ./a.out is statically linked, but it is small (<9000 bytes),
because it doesn't use the libc _at_ _all_! (It calls the system calls
read(2) and write(2) with `int 80h' directly.) As a side effect, ./a.out is
libc-independent, so you don't have to be afraid of sudden segfaults
when you upgrade your libc :-).

Compress ./a.out with the UPX executable packer (available from
http://upx.sf.net/).

6. How to use muZCat?
"""""""""""""""""""""
See FAQ answer 5 first.

To use muZCat, see the shell scripts mentioned in FAQ answer 5, and
read the comments in muzcat.c.

Also try compile.sh, which creates the binary `muzcat'.

Examples:
	
	# create test_file
	gzip -c <test_file >test_file.gz
	zip -9 test_file.zip test_file
	gs -dNODISPLAY -q -c '/i(%stdin)(r)file def/o(%stdout)(w)file
          /FlateEncode filter def/s 4096 string def{i s readstring exch o
          exch writestring not{exit}if}loop o closefile
          quit' <test_file >test_file.flate
	muzcat <test_file.gz >test_file.out
	muzcat <test_file.zip >test_file.out
	muzcat <test_file.flate >test_file.out

6b. How to use muzcat3.ps?
""""""""""""""""""""""""""
muzcat3.ps is a pre-compiled PostScript implementation of muZCat. See the
leading comments in the file for more details.

7. Is muZCat free software?
"""""""""""""""""""""""""""
Yes, it is licensed under the GNU GPL.

8a. Can I concatenate multiple compressed input files to get a concatenated
output?
"""""""
Yes. For example:

	gzip -c <a.file >a.file.gz
	zip -9 a.file.zip a.file
	cat a.file.gz a.file.zip a.file.gz >aaa.zzz
	muzcat <aaa.zzz >aaa.file
	# Now: filesize("aaa.file")==3*filesize("a.file")

8b. How can I prevent concatenation?
""""""""""""""""""""""""""""""""""""
Just instert the character 4 (Ctrl-D) into the input stream. For example:

	gzip -c <a.file >a.file.gz
	zip -9 a.file.zip a.file
	perl -e 'print chr(4)' >ctrl_d
	cat a.file.gz a.file.zip ctrl_d a.file.gz >aa.zz
	muzcat <aa.zz >aa.file
	# Now: filesize("aa.file")==2*filesize("a.file")

9. Why did you use `36000 array' in vm.ps?
""""""""""""""""""""""""""""""""""""""""""
Because the decompressor bytecode needs an array of >=16-bit (signed or
unsigned) integers of length 36000. Actually `20000 array' would have been
enough because it needs around 32000 8-bit bytes and 4000 16-bit values.
But, to make it faster and simpler, I've done it this way.

Some comparison benchmarks for integer array memory implementations:

	%            time    vm usage
	% string:  14,50s  85540bytes
	% array:    6,53s 287900bytes
	% array2:   6,07s 287900bytes
	% dict:     7,26s 635440bytes
	% string1:  6,07s  42708bytes
	% 0.05:     6s     54000bytes (??)

Note that method `string1' allows only values 0..255 (and not 0..65535) so
it is incapable for running the muZCat decompression algorithm.

muZCat 0.05 is trickier: the buffer `char S[32768]' is stored in a string,
and others are stored in an array2. So we get both superior speed and
quite good memory usage.

10a. Does muZCat support Sed?
"""""""""""""""""""""""""""""
No, beacuse Sed (even GNU sed) cannot read or write binary files. For real
Sed hacking, try the file dc.sed distributed along GNU sed.

10b. Does muZCat support dc?
""""""""""""""""""""""""""""
No, beacuse dc (even GNU dc) cannot read or write binary files. For some dc
hacking, try feeding the following two-liner to the STDIN of _GNU_ dc:
 
	[/dlflg/=u]dZ[lalbdlp*lqlg*+lpla*lqlf*+lflgsbsasfsglex]dscZ[r%O*]sdzzKsa
	[nlbdlaldxlglfldxsfdsalex]dsuZ[.]zsqsssgsbnsfspselsn[lcxsslqdlp+spz+sqdx]dx

10c. Does muZCat support AWK?
"""""""""""""""""""""""""""""
gawk (GNU AWK) is supported, please try `./compile_gawk.sh'. Original AWK, as
specified in (The AWK Programming Language, Addison-Wesley, 1988) cannot
read binary files (even length("ab\0cd")==2 in some implementations), so it
cannot be supported.

10d. Does muZCat support shell scripts?
"""""""""""""""""""""""""""""""""""""""
Not yet. It wouldn't be very hard to implement a GNU Bash version, but shell
script syntax differs considerably from C syntax, so a compiler would be
necessary. The running program would be even slower than AWK.

10e. Does muZCat support Pascal?
""""""""""""""""""""""""""""""""
muZCat supports several favors of Pascal implementations: gpc (GNU Pascal
Compiler) >=19991030, ppc386 (Free Pascal) and Turbo Pascal >=5.5 (including
Borland Pascal 7.0). Just try ./compile_freepascal.sh,
./compule_turbopascal.sh (just generates zcat.pas), ./compile_isopascal.c
(dosn't work now) or ./compile_xpascal_gpc.sh .

In ISO 7185:1991 Standard Pascal it is impossible to
read binary characters (0..255) from the standard input. (Fortunately,
writing is easy: `write(chr(...));' works.) The most common
problems are:

-- reading char 0
-- reading char 10
-- reading char 13
-- reading char 26
-- reading char 255

As an example, I present a working implementation of the `cat' command
(a program that copies stdin to stdout in an 8-bit safe way):

ANSI C:

	#include <stdio.h>
	int main(int i) {
	  while ((i=getchar())!=-1) putchar(i);
	  return 0;
	}

ANSI C++ iostream:

	#include <iostream>
	int main(int i) {
          while ((i=cin.get())!=-1) cout.put(i);
	  return 0;
	} 

Turbo Pascal 5.5, gpc version 19991030, based on 2.95.2 20000220:

	program t(input,output);
	var f: file of char;  c: char;
	begin
	  assign(f,'');
	  reset(f);
	  while not eof(f) do begin read(f,c); write(c); end;
	end.

ppc386 Free Pascal Compiler version 0.99.10 [1999/08/23] -- 0.99.13:

	var c: char;
	begin
	  while not eof do begin read(c); write(c); end;
	end.

There isn't any code that works with gpc version 19990118, based on
egcs-2.91.60.

There isn't any code that works with `gpc --standard-pascal' or `gpc
--standard-pascal-level-0'.

11a. What is the most simple gzip(1) header?
""""""""""""""""""""""""""""""""""""""""""""
The most simple gzip header (10 bytes):

ID1   hex 0x1f == dec 31 == FMT_GZIP
ID2   hex 0x8b
CM    hex 0x08
FLG   hex 0x00
MTIME hex 0x00, 0x00, 0x00, 0x00 (1 Jan 1970, UNIX epoch)
XFL   hex 0x00
OS    hex 0xff
After that comes the compressed data stream.
After that comes the CRC32 and ISIZE (byte aligned? ?)

11b. What is the most simple zlib(1) header?
""""""""""""""""""""""""""""""""""""""""""""
0x78 0x9c

(There are 3 other possibilities.)

12. Can muZCat decompress .bz2 (BZIP2) or .rar files?
"""""""""""""""""""""""""""""""""""""""""""""""""""""
No, these files use a different compression algorithm.

12. Can muZCat decompress .7z, .xz or .lzma files?
""""""""""""""""""""""""""""""""""""""""""""""""""
No, these files use a different compression algorithm.

See https://github.com/pts/muxzcat for a tiny decompressor of .xz
and .lzma files implemented in C.

See https://github.com/pts/pts-tiny-7z-sfx for a tiny extractor and
self-extractor for .7z archives.

99. ChangeLog, CHANGES, Revision history
""""""""""""""""""""""""""""""""""""""""
0.04 stable, working
0.05 test_vm_eps.eps
0.06 Tue Dec 25 20:53:24 CET 2001
     faster statecat.c (see FAQ answer 3 for speed details)
0.07 a much faster PostScript version (statecat.ps)
     GAWK
     multiple Pascal implementations

Technical notes
~~~~~~~~~~~~~~~
Imp: update muzcat3.c
Imp: restore array in muzcat3.ps 
Imp: check for double PostScript string get
Imp: read till EOD or EOF?
Imp: protect dict begin..end
Imp: `20000 array' instead of `36000 array'.
Imp: PS1 maximum length of a string (should be 65535), 32768 is enough for us
Imp: PS1 32 bit signed integer arithmetic, needed for manual /ASCII85Decode
Imp: let bind do its job... in onechar_*
Imp: closefile in filters...
Imp: remove PBM header...
Dat: fxRead8(); fxRead8(); instead of fxRead16(); ?
     don't care because it is called so rarely...

__END__
