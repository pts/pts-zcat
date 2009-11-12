/* muzcat.c -- a portable and minimalistic zcat (gzip -cd) implementation
 * originally rcm.c
 * original author: Ron McFarland <rcm@one.net>, 1996
 * rewritten, restructured and extended by pts@fazekas.hu at
 * Fri Jun 29 23:42:40 CEST 2001
 * Sat Jun 30 09:49:56 CEST 2001
 * Sat Jun 30 16:01:11 CEST 2001 C, Java
 * Sat Jul 21 10:43:19 CEST 2001
 * Sat Jul 21 16:16:45 CEST 2001 CFG_LANG_OC, timings
 * Tue Jul 24 20:59:35 CEST 2001 CFG_LANG_KNRC, CFG_LANG_PERL
 * Thu Jul 26 12:14:40 CEST 2001 added CFG_INPUT_*, CFG_MULTIPLE
 * Fri Jul 27 22:20:21 CEST 2001 cleanups, comments, CFG_LANG_ANSIC_SYSCALL, CFG_LANG_ANSIC_LINUX
 * Sat Jul 28 13:25:38 CEST 2001 WHILE etc, CFG_LANG_RUBY, CFG_LANG_PYTHON, muzcat3.ps
 * Sun Jul 29 12:22:59 CEST 2001 cleanups, CFG_LANG_ISOPASCAL
 * Sat Jun 29 22:30:11 CEST 2002 Makefile, bugfixes, CFG_LANG_GAWK, CFG_LANG_FREEPASCAL, CFG_LANG_XPASCAL
 * Sun Jun 30 23:12:36 CEST 2002 CFG_LANG_XPASCAL now works with Turbo Pascal >=5.5
 *
 *
 * SUXX: we don't check for CRC
 * SUXX: we don't validate file
 * SUXX: we don't check for EOF
 * SUXX: may segfault on invalid file
 *
 * Imp: !! fixup 16-bit arithmetic CFG_LANG_XPASCAL-style
 * Imp: CFG_LANG_TURBOPASCAL
 * Imp: buffering for Turbo Pascal (CFG_LANG_XPASCAL)
 * Imp: import CFG_OLD_BUFFERING from statecat.c
 * Imp: remove SHL(fvRead(),8) for ZIP
 * Imp: CFG_LANG_OC NE(...,NULL) -> NZ(...)
 * Imp: is NODESIZE large enough??
 * Imp: write error-tolelarant, partial zip decompressor
 * OK : Linux kernel version
 * Imp: PHP
 * OK : multiple ZIP files (Thu Jul 26 19:25:22 CEST 2001)
 * OK : STORED for ZIP files (Thu Jul 26 19:06:28 CEST 2001)
 * OK : Perl (before Thu Jul 26 10:55:57 CEST 2001)
 * OK : eliminate malloc(), only static storage in var `N'
 * OK : eliminate pointers (`*')
 * OK : works if WORD can hold a 16 bit signed integer
 * OK : works with WORD defined as either `signed short' or `unsigned short'
 *      The problems with %, / and >> have been solved. This means that
 *      it should be easy to port to PostScript.
 * OK : works with BYTE defined as either `signed char'  or `unsigned char'
 * OK : eliminate DWORDs (32 bit integers)
 * OK : add initialization of global variables
 * OK : convert short, fixed loops to series of instructions
 * OK : simplify if's
 * OK : `for' -> `while'
 *      This means that it should be easy to port to Ruby.
 * OK : no `for', no `goto', no `break', no `continue'
 * OK : unobfuscate [] handling
 * OK : use ++, --, =, += etc. as a standalone instruction, not an operator.
 *      This means that it should be easy to port to Python.
 * OK : separate, named fv*() functions instead of B()
 * OK : remove commas in favour of semicolons
 * OK : eliminate operator `?:'
 * OK : eliminate var `I'
 * OK : boolean handling is like in Java: comparison and logical operators are
 *      assumed to return a boolean value (which cannot be converted to/from
 *      int), `if' and `while' test only boolean, not int.
 * OK : eliminate logical operators: `&&', '||'
 * OK : runs fine with checkergcc, even in OC mode
 * Dat: no signed integers, just unsigned
 * Dat: no var `l' anymore
 * Dat: no var `O' anymore
 * Dat: global variables: one letter, upper case
 * Dat: no octal and hexadecimal constants in the source
 * Dat: no multiplication or division, just bit shifts
 * Dat: compiles with: gcc -s -O3 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations rcm.c
 * Dat: dependency of functions:
 *	# `f g' means: g calls f.
 *	fvMain main
 *	fvRead fvMain
 *	fvMktree fvMain
 *	fvDescend fvMain
 *	fvFree fvMain
 *	fvWrite fvMain
 *	fvRead fvDescend
 *	fvNalloc fvMktree
 * Dat: functions are listed avoiding forward dependency
 * Dat: no division and multiplication
 * Dat: no initialized var decls: `int a=42;'
 * Dat: no inner var decls: `if (1) { int a; }'
 * Dat: no multiple var decls: `int a, b;'
 * Dat: function local (i.e automatic) var decls are prefixed by LOCAL_VAR
 * Dat: bodies for while() and if() are all {}-ed
 * Dat: while and if bodies are in seperate line
 * Dat: 2nd argument of EQ(), NE(), LT() is constant if possible
 * Dat: RETURN/RETURN_VOID is the last instruction of all functions, no other
 *      RETURNs
 */

/* The user can effect the compilation by #defining a couple of control
 * macros.
 *
 * To choose the language, #define _one_ of the following:
 *
 * #define CFG_LANG_JAVA 1
 * #define CFG_LANG_OC 1
 * #define CFG_LANG_ANSIC_SIGNED 1
 * #define CFG_LANG_PERL 1
 * #define CFG_LANG_KNRC 1
 * #define CFG_LANG_ANSIC 1
 *
 * To affect how input is treated, #define _one_ of the following:
 *
 * #define CFG_INPUT_RAW  1  -- raw, RFC 1951
 * #define CFG_INPUT_FILE 1  -- autodetect: gzip or RFC 1950 or ZIP
 * #define CFG_INPUT_RFC  1  -- autodetect: RFC 1952 or RFC 1950 or ZIP
 *
 * Please use CFG_INPUT_FILE instead of CFG_INPUT_RFC, so it will be
 * compatible with existing utilities.
 *
 * To affect how multiple files are treated, do _one_ of the following:
 *
 * #define CFG_MULTIPLE 1    -- concatenate multiple input files
 * #undef  CFG_MULTIPLE      -- stop after first input file
 *
 * Note that reading after the first ZIP file is not supported.
 */

#ifdef MUZCAT_NO_DEFAULT
#else
#define CFG_INPUT_FILE 1
#define CFG_MULTIPLE 1
#endif

/* Imp: #undef more */
#undef my_getchar
#undef my_putchar
#undef BYTE
#undef WORD
#undef ENDDOT
#undef ARRAY
#undef SHR
#undef SHL
#undef ADD
#undef SUB
#undef HAVE_ARRAY_INIT
#undef DEFUN_0
#undef DEFUN_1
#undef DEFUN_0_VOID
#undef DEFUN_1_VOID
#undef ARY
#undef YRA
#undef GLOBAL_INIT_ARRAY
#undef GLOBAL_ARRAY
#undef AREF
#undef GLOBAL_VAR
#undef LOCAL_VAR
#undef ENDDOT2
#undef SMALLEST_CODE
#undef IF
#undef ELSE
#undef ELSE_IF
#undef ENDIF
#undef WHILE
#undef ENDWHILE
#undef ENDFUN
#undef PASS
#undef GLOBAL_REF
#undef AFTER_GLOBALS
#define AFTER_GLOBALS
#undef ALLOW_LOCALS
#undef RETURN
#undef RETURN_VOID
#undef FUNCODE
#undef EQ
#undef NE
#undef SET
#undef ASET
#undef NZ
#undef NOARGS
#undef IGNORE
#undef GETCHAR_NEGATIVE
#undef HAVE_NO_MODIFIED_OPS
#undef NEED_ASCII85DECODE
#undef HAVE_SPECIAL_DIVISION


#if CFG_LANG_JAVA
  /* Any javac and JVM will suffice, JRE >=1.1 is required.
   *
   * .class file size: 5076 bytes
   * Test Bash:    5590ms user
   * Test Random:  1440ms user
   * Test Misc:    6490ms user
   *
   */
  #define FUNCODE
  /* ^^^ marks the end of LOCAL_VAR(...)s, beginning of function code */
  #define RETURN(fname,value) return value;
  /* ^^^ auto `;', RETURN/RETURN_VOID must be just before ENDFUN. No RETURN
   *     and RETURN_VOID anywhere else in the function.
   */
  #define IGNORE /* ignore retval of function */
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a!=0 /* non-zero. Java is strictly boolean typed. */
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static protected void name() {
  #define DEFUN_1_VOID(name,atype) static protected void name(atype arg) {
  #define DEFUN_0(rtype,name) static protected rtype name() {
  #define DEFUN_1(rtype,name,atype) static protected rtype name(atype arg) {
  #define BYTE int
  #define WORD int
  #define CONST final
  #define GLOBAL_VAR(type,name) static protected type name;
  #define LOCAL_VAR(type,name) type name;
  #define LOCAL_VARS(x) x
  #define ENDDOT }
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[]=new type[length];
  #define HAVE_ARRAY_INIT 1
  #define SHL(a,b) ((a)<<(b))
  /* ^^^ SHL(a,b): Precondition: 0<=b && b<=15 */
  #define SHR(a,b) ((a)>>(b)) /* even >>> works OK */
  /* ^^^ SHR(a,b): Precondition: 0<=b && b<=14 (!!) */
  #define ADD(r,a) r+=(a) /* stupid Java, disallows extra parens */
  #define SUB(r,a) r-=(a)
  #define SLIDE(r) r++; r&=32767 /* stupid Java, disallows extra parens */
  /* ^^^ SLIDE(r): increments the var `r', modulo 32768. Precondition:
   *     0<=r<32768. Postcondition: 0<=r<32768.
   */
  #define TAND_1(a) (0!=((a)&1))
  /* ^^^ TAND_1(a) is true iff a is odd */
  #define TAND_P(a,p) (0!=((a)&(p)))
  /* ^^^ TAND_P(a,p) is true iff 0!=(a&p). Precondition: p must be a power
   *     of two (!), such as: 1, 2, 4, 8, 16.
   */
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) r++ /* stupid Java, disallows extra parens */
  #define DECR(r) r-- /* stupid Java, disallows extra parens */
  #if 0 /* unused */
    #define LAND(a,b) ((a) && (b))
    /* ^^^ returns true (testable by `if') iff both `a' and `b' are true */
  #endif
  import java.io.IOException;
  public class ZCat {
    public static void main(String[] args) {
      fvMain();
      System.out.flush();
    }
    public static WORD my_getchar() {
      try {
        int i=System.in.read();
        if (i<-1) System.exit(2); /* -1 is OK, it means EOF */
        return i;
        /* return (WORD)(i&255); */
      } catch (IOException e) {
        System.exit(3);
      }
      return 0;
    }
    public static void my_putchar(WORD b) {
      /* try { */
        System.out.write((byte)b);
      /* } catch (IOException e) {} */
    }
#elif CFG_LANG_OC
  /* This works since Sat Jul 21 15:57:03 CEST 2001.
   *
   * OC (Obfuscated C) is a simplistic, C-like programming language invented
   * and implemented by Lennart Augustsson <augustss@carlstedt.se>. OC's most
   * remarkable property is that the overall size of OC runtime environment
   * and the OC compiler is less than 3400 bytes (!) (measuring the C
   * source code).
   *
   * You can dowload OC from http://www.ioccc.org. Search for the 1996 contest
   * entries, and download `august' (Lennart Augostsson's entry).
   * august.c and august.txt is shipped with muzcat.c for your convenience.
   *
   * OC bytecode size: 4872 bytes (should be hard to reduce to 4000)
   * Required memory: 48000 bytes (august.c compiled with -DZ=48000)
   * Test Bash:   22720ms user
   * Test Random:  3250ms user
   * Test Misc:   20900ms user
   *
   * Special care was taken to make this program work in OC because OC has a
   * rather minimal instruction set (see below), and both the runtime
   * environment and the compile impose limitations on code size and memory
   * usage.
   */
  /* code OK: int main() { int a; (a)=(65); putchar((a)); } */
  /* code OK: void main(void) { int a; (a)=(65); putchar((a)); } */
  /* OK : functions may have at most 16 local variables! */
  /* OK : no += etc.; but ++ and -- is allowed (see parse.oc, fac.oc) */
  /* OK : no bitwise operations: `&', `|', `^', `~' */
  /* OK : no logical operators: `&&', '||' */
  /* OK : no bit shift operators; `<<', '>>' */
  /* OK : no array initializers */
  /* OK : no initialized var decls: `int a=42;' */
  /* OK : no inner var decls; `if (1) { int a; }' */
  /* OK : no argc, argv in main */
  /* OK : ensure powtwo[16] unused */
  /* OK : function return types cannot be void */
  /* OK : bytecode size reduced <5000, so it compiles with august.c */
  /* OK : variable names must be /[a-z]+/ */
  /* Imp: eliminate 32768 */
  /* OK : no comparison operators `>', `>=', `<=' */
  /* #define AFTER_GLOBALS int dummy; */
  /* Dat: memory requirement: 35278 words */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 0
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #define SMALLEST_CODE 1 /* optimize for smallest code size, so <5000 */
  #define ENDDOT2 enddot2
  #undef GLOBAL_INIT_ARRAY /* N/A */
  #define AREF(aname,idx) aname[idx]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) int name() {
  #define DEFUN_1_VOID(name,atype) int name(atype arg) {
  #define DEFUN_0(rtype,name) rtype name() {
  #define DEFUN_1(rtype,name,atype) rtype name(atype arg) {
  #define BYTE char
  #define WORD int
  #define GLOBAL_VAR(type,name) type name;
  #define LOCAL_VAR(type,name) type name;
  #define LOCAL_VARS(x) x
  #define CONST
  #define my_putchar putchar
  #define my_getchar getchar
  #define RETURN_VOID(fname) return 0; /* might have been omitted... */
  #define GLOBAL_ARRAY(type,name,length) type name[length];
  #define HAVE_ARRAY_INIT 0
  #define ADD(r,a) ((r)=(r)+(a))
  #define SUB(r,a) ((r)=(r)-(a))
  #define SLIDE(r) if (r==32767) r=0; else INCR(r)
  #define TAND_1(a) ((a)%2)
  #define TAND_P(a,p) ((a)/(p)%2)
  #define TNAND_P(a,p) (0==((a)/(p)%2))
  #define AND_255(a) ((a)%256)
  #define AND_7(a) ((a)%8)
  #define AND_LOW(a,b) ((a)%powtwo[b])
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)*2)
  #define SHL(a,b) ((a)*powtwo[b])
  #define SHR(a,b) ((a)/powtwo[b])
  #if 1 /* works! */
    #define INCR(r) ((r)++)
    #define DECR(r) ((r)--)
  #else
    #define INCR(r) ((r)=(r)+1)
    #define DECR(r) ((r)=(r)-1)
  #endif
  
  #define N n
  #define D d
  #define T t
  #define C c
  #define J j
  #define Y y
  #define Z z
  #define B b
  #define G g
  #define S s
  #define constW cw
  #define constU cu
  #define constP cp
  #define constQ cq
  #define constL cl
  #define fvMain fvmain
  #define fvNalloc fvnalloc
  #define fvFree fvfree
  #define fvDescend fvdescend
  #define fvWrite fvwrite
  #define fvMktree fvmktree
  #define fvRead fvread
  GLOBAL_ARRAY(WORD,powtwo,16); /* WORD powtwo[16]; */
  /* void ocinit(NO_ARGS) { */
  /* } */
  int io; /* global var defined early for main() */
  int iq; /* global var defined early for main() */
  #define ENDDOT \
    int main() { \
      /* dummy; */ \
      /* powtwo[0)=1; powtwo[1)=2; powtwo[2)=4; powtwo[3)=8; powtwo[4)=16; powtwo[5)=32; \
      powtwo[6)=64; powtwo[7)=128; powtwo[8)=256; powtwo[9)=512; powtwo[10)=1024; \
      powtwo[11)=2048; powtwo[12)=4096; powtwo[13)=8192; powtwo[14)=16384; \
      powtwo[15)=32768; */ \
      /* powtwo[16)=65536; */ \
      /* AREF(constQ,0)=1; AREF(constQ,1)=2; AREF(constQ,2)=3; AREF(constQ,3)=4; AREF(constQ,4)=5; AREF(constQ,5)=7; AREF(constQ,6)=9; AREF(constQ,7)=13; AREF(constQ,8)=17; AREF(constQ,9)=25; AREF(constQ,10)=33; AREF(constQ,11)=49; AREF(constQ,12)=65; AREF(constQ,13)=97; AREF(constQ,14)=129; AREF(constQ,15)=193; AREF(constQ,16)=257; AREF(constQ,17)=385; AREF(constQ,18)=513; AREF(constQ,19)=769; AREF(constQ,20)=1025; AREF(constQ,21)=1537; AREF(constQ,22)=2049; AREF(constQ,23)=3073; AREF(constQ,24)=4097; AREF(constQ,25)=6145; AREF(constQ,26)=8193; AREF(constQ,27)=12289; AREF(constQ,28)=16385; AREF(constQ,29)=24577; */ \
      io=1; iq=0; while (iq<30) { AREF(constQ,iq)=io+1; INCR(iq); AREF(constQ,iq)=io*3/2+1; INCR(iq); io=TWICE(io); /* !! op* op/ */ } \
      AREF(constQ,0)=1; \
      io=1; iq=0; while (iq<16) { AREF(powtwo,iq)=io; INCR(iq); io=TWICE(io); } \
      iq=0; while (iq<30) { AREF(constL,iq)=iq/2-1; /* !! op/ */ INCR(iq); } /* used bool as int */ \
      AREF(constL,0)=0; AREF(constL,1)=0; \
      /* AREF(constL,0)=0; AREF(constL,1)=0; AREF(constL,2)=0; AREF(constL,3)=0; AREF(constL,4)=1; AREF(constL,5)=1; AREF(constL,6)=2; AREF(constL,7)=2; AREF(constL,8)=3; AREF(constL,9)=3; AREF(constL,10)=4; AREF(constL,11)=4; AREF(constL,12)=5; AREF(constL,13)=5; AREF(constL,14)=6; AREF(constL,15)=6; AREF(constL,16)=7; AREF(constL,17)=7; AREF(constL,18)=8; AREF(constL,19)=8; AREF(constL,20)=9; AREF(constL,21)=9; AREF(constL,22)=10; AREF(constL,23)=10; AREF(constL,24)=11; AREF(constL,25)=11; AREF(constL,26)=12; AREF(constL,27)=12; AREF(constL,28)=13; AREF(constL,29)=13; */ \
      AREF(constU,0)=3; AREF(constU,1)=4; AREF(constU,2)=5; AREF(constU,3)=6; AREF(constU,4)=7; AREF(constU,5)=8; AREF(constU,6)=9; AREF(constU,7)=10; AREF(constU,8)=11; AREF(constU,9)=13; AREF(constU,10)=15; AREF(constU,11)=17; AREF(constU,12)=19; AREF(constU,13)=23; AREF(constU,14)=27; AREF(constU,15)=31; AREF(constU,16)=35; AREF(constU,17)=43; AREF(constU,18)=51; AREF(constU,19)=59; AREF(constU,20)=67; AREF(constU,21)=83; AREF(constU,22)=99; AREF(constU,23)=115; AREF(constU,24)=131; AREF(constU,25)=163; AREF(constU,26)=195; AREF(constU,27)=227; AREF(constU,28)=258; \
      AREF(constW,0)=16; AREF(constW,1)=17; AREF(constW,2)=18; AREF(constW,3)=0; AREF(constW,4)=8; AREF(constW,5)=7; AREF(constW,6)=9; AREF(constW,7)=6; AREF(constW,8)=10; AREF(constW,9)=5; AREF(constW,10)=11; AREF(constW,11)=4; AREF(constW,12)=12; AREF(constW,13)=3; AREF(constW,14)=13; AREF(constW,15)=2; AREF(constW,16)=14; AREF(constW,17)=1; AREF(constW,18)=15; \
      iq=0; io=0; while (iq<28) { if (iq==4) io=1; AREF(constP,iq)=iq/4-io; /* !! op/ */ INCR(iq); } /* used bool as int */ \
      AREF(constP,28)=0; \
      /* AREF(constP,0)=0; AREF(constP,1)=0; AREF(constP,2)=0; AREF(constP,3)=0; AREF(constP,4)=0; AREF(constP,5)=0; AREF(constP,6)=0; AREF(constP,7)=0; AREF(constP,8)=1; AREF(constP,9)=1; AREF(constP,10)=1; AREF(constP,11)=1; AREF(constP,12)=2; AREF(constP,13)=2; AREF(constP,14)=2; AREF(constP,15)=2; AREF(constP,16)=3; AREF(constP,17)=3; AREF(constP,18)=3; AREF(constP,19)=3; AREF(constP,20)=4; AREF(constP,21)=4; AREF(constP,22)=4; AREF(constP,23)=4; AREF(constP,24)=5; AREF(constP,25)=5; AREF(constP,26)=5; AREF(constP,27)=5; AREF(constP,28)=0; */ \
      fvMain(); \
      return 0; }
#elif CFG_LANG_OCPS
  /* This works since Sun Jul 29 11:28:43 CEST 2001
   *
   * We use OC to compile to PostScript, see compile_ps1.sh.
   *
   * To gain performance, we abuse OC operators %, * and /. Operator % is
   * used as bitwise and (&), operator * is used as bitwise left shift (<<),
   * operator / undefined. The shift count of the bitwise left shift can be
   * negative (such as in PostScript's bitshift operator) to mean a right
   * shift. So we can avoid the
   * powtwo[] array. Modification isn't sign-safe (we never get negative
   * result with binary operators), but we can ignore this...
   *   
   */
  #define HAVE_SPECIAL_DIVISION 1
  #define HAVE_NO_MODIFIED_OPS 0
  #define GETCHAR_NEGATIVE 1
  #define NEED_ASCII85DECODE 0
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a!=0 /* PostScript is strictly boolean typed. */
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 0
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #define SMALLEST_CODE 1 /* optimize for smallest code size, so <5000 */
  #define ENDDOT2 enddot2
  #undef GLOBAL_INIT_ARRAY /* N/A */
  #define AREF(aname,idx) aname[idx]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) int name() {
  #define DEFUN_1_VOID(name,atype) int name(atype arg) {
  #define DEFUN_0(rtype,name) rtype name() {
  #define DEFUN_1(rtype,name,atype) rtype name(atype arg) {
  #define BYTE char
  #define WORD int
  #define GLOBAL_VAR(type,name) type name;
  #define LOCAL_VAR(type,name) type name;
  #define LOCAL_VARS(x) x
  #define CONST
  #define my_putchar putchar
  #define my_getchar getchar
  #define RETURN_VOID(fname) return 0; /* might have been omitted... */
  #define GLOBAL_ARRAY(type,name,length) type name[length];
  #define HAVE_ARRAY_INIT 0
  /* If P is a power of 2, then a%b==a&(b-1) */
  #if HAVE_NO_MODIFIED_OPS /* operators *, /, % have their original meaning */
    #define SHL(a,b) ((a)*powtwo[b])
    #define SHR(a,b) ((a)/powtwo[b])
    #define TAND_1(a) (0!=((a)%2))        /* PostScript is strictly boolean typed. */
    #define TAND_P(a,p) (0!=((a)/(p)%2))  /* PostScript is strictly boolean typed. */
    #define TNAND_P(a,p) (0==((a)/(p)%2)) /* PostScript is strictly boolean typed. */
    #define AND_255(a) ((a)%256)
    #define AND_7(a) ((a)%8)
    #define AND_LOW(a,b) ((a)%powtwo[b])
    #define TWICE(a) ((a)*2)
  #else /* operators *, /, % have modified meaning */
    #define SHL(a,b) ((a)*(b)) /* Fortunately parsee.c doesn't assume a*b == b*a */
    #define SHR(a,b) ((a)*-(b))
    #define TAND_1(a) (((a)%1)!=0)      /* PostScript is strictly boolean typed. */
    #define TAND_P(a,p) (((a)%(p))!=0)  /* PostScript is strictly boolean typed. */
    #define TNAND_P(a,p) (((a)%(p))==0) /* PostScript is strictly boolean typed. */
    #define AND_255(a) ((a)%255)
    #define AND_7(a) ((a)%7)
    #define AND_LOW(a,b) ((a)%(1*(b)-1)) /* rather expensive */
    #define TWICE(a) ((a)*1)
  #endif
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define ADD(r,a) ((r)=(r)+(a))
  #define SUB(r,a) ((r)=(r)-(a))
  #define SLIDE(r) if (r==32767) r=0; else INCR(r) /* Imp: make it without IF; even if int is -32767..32768 */
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  
  #define N n
  #define D d
  #define T t
  #define C c
  #define J j
  #define Y y
  #define Z z
  #define B b
  #define G g
  #define S s
  #define constW cw
  #define constU cu
  #define constP cp
  #define constQ cq
  #define constL cl
  #define fvMain fvmain
  #define fvNalloc fvnalloc
  #define fvFree fvfree
  #define fvDescend fvdescend
  #define fvWrite fvwrite
  #define fvMktree fvmktree
  #define fvRead fvread
  #if HAVE_NO_MODIFIED_OPS
    WORD powtwo[16];
    #define INIT_POWTWO io=1; iq=0; while (iq<16) { AREF(powtwo,iq)=io; INCR(iq); io=TWICE(io); }
  #else
    #define INIT_POWTWO
  #endif
  /* void ocinit(NO_ARGS) { */
  /* } */
  int io; /* global var defined early for main() */
  int iq; /* global var defined early for main() */
  #define ENDDOT \
    int main() { \
      INIT_POWTWO \
      io=1; iq=0; while (iq<30) { AREF(constQ,iq)=io+1; INCR(iq); AREF(constQ,iq)=SHR(TWICE(io)+io,1)+1; INCR(iq); io=TWICE(io); /* !! op*, op/ */ } \
      AREF(constQ,0)=1; \
      iq=0; while (iq<30) { AREF(constL,iq)=SHR(iq,1)-1; /* !! op/ */ INCR(iq); } /* used bool as int */ \
      AREF(constL,0)=0; AREF(constL,1)=0; \
      AREF(constU,0)=3; AREF(constU,1)=4; AREF(constU,2)=5; AREF(constU,3)=6; AREF(constU,4)=7; AREF(constU,5)=8; AREF(constU,6)=9; AREF(constU,7)=10; AREF(constU,8)=11; AREF(constU,9)=13; AREF(constU,10)=15; AREF(constU,11)=17; AREF(constU,12)=19; AREF(constU,13)=23; AREF(constU,14)=27; AREF(constU,15)=31; AREF(constU,16)=35; AREF(constU,17)=43; AREF(constU,18)=51; AREF(constU,19)=59; AREF(constU,20)=67; AREF(constU,21)=83; AREF(constU,22)=99; AREF(constU,23)=115; AREF(constU,24)=131; AREF(constU,25)=163; AREF(constU,26)=195; AREF(constU,27)=227; AREF(constU,28)=258; \
      AREF(constW,0)=16; AREF(constW,1)=17; AREF(constW,2)=18; AREF(constW,3)=0; AREF(constW,4)=8; AREF(constW,5)=7; AREF(constW,6)=9; AREF(constW,7)=6; AREF(constW,8)=10; AREF(constW,9)=5; AREF(constW,10)=11; AREF(constW,11)=4; AREF(constW,12)=12; AREF(constW,13)=3; AREF(constW,14)=13; AREF(constW,15)=2; AREF(constW,16)=14; AREF(constW,17)=1; AREF(constW,18)=15; \
      iq=0; io=0; while (iq<28) { if (iq==4) io=1; AREF(constP,iq)=SHR(iq,2)-io; /* !! op/ */ INCR(iq); } /* used bool as int */ \
      AREF(constP,28)=0; \
      fvMain(); \
      return 0; }
#elif CFG_LANG_OCPSAD
  /* This is obsolete since Sun Nov 11 12:01:59 CET 2001, because I've managed
   * to implement to /ASCII85Decode filter in pure, LanguageLevel 1 PostScript.
   *
   * We use OC to compile to PostScript, with /ASCII85Decode capabilities,
   * see compile_ps1.sh. We assume a PostScript interpreter having >=32 bit
   * signed integers (like GNU Ghostscript). We must also avoid numeric constants
   * in the range 20000..27000.
   *
   * To gain performance, we abuse OC operators %, * and /. Operator % is
   * used as bitwise and (&), operator * is used as bitwise left shift (<<),
   * operator / undefined. The shift count of the bitwise left shift can be
   * negative (such as in PostScript's bitshift operator) to mean a right
   * shift. So we can avoid the
   * powtwo[] array. Modification isn't sign-safe (we never get negative
   * result with binary operators), but we can ignore this...
   */
  #define HAVE_NO_MODIFIED_OPS 0
  #define GETCHAR_NEGATIVE 1
  #define NEED_ASCII85DECODE 1
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a!=0 /* PostScript is strictly boolean typed. */
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 0
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #define SMALLEST_CODE 1 /* optimize for smallest code size, so <5000 */
  #define ENDDOT2 enddot2
  #undef GLOBAL_INIT_ARRAY /* N/A */
  #define AREF(aname,idx) aname[idx]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) int name() {
  #define DEFUN_1_VOID(name,atype) int name(atype arg) {
  #define DEFUN_0(rtype,name) rtype name() {
  #define DEFUN_1(rtype,name,atype) rtype name(atype arg) {
  #define BYTE char
  #define WORD int
  #define GLOBAL_VAR(type,name) type name;
  #define LOCAL_VAR(type,name) type name;
  #define LOCAL_VARS(x) x
  #define CONST
  #define my_putchar putchar
  #define my_getchar getchar
  #define RETURN_VOID(fname) return 0; /* might have been omitted... */
  #define GLOBAL_ARRAY(type,name,length) type name[length];
  #define HAVE_ARRAY_INIT 0
  /* If P is a power of 2, then a%b==a&(b-1) */
  #if HAVE_NO_MODIFIED_OPS /* operators *, /, % have their original meaning */
    #define SHL(a,b) ((a)*powtwo[b])
    #define SHR(a,b) ((a)/powtwo[b])
    #define TAND_1(a) (0!=((a)%2))        /* PostScript is strictly boolean typed. */
    #define TAND_P(a,p) (0!=((a)/(p)%2))  /* PostScript is strictly boolean typed. */
    #define TNAND_P(a,p) (0==((a)/(p)%2)) /* PostScript is strictly boolean typed. */
    #define AND_255(a) ((a)%256)
    #define AND_7(a) ((a)%8)
    #define AND_LOW(a,b) ((a)%powtwo[b])
    #define TWICE(a) ((a)*2)
  #else /* operators *, /, % have modified meaning */
    #define SHL(a,b) ((a)*(b)) /* Fortunately parsee.c doesn't assume a*b == b*a */
    #define SHR(a,b) ((a)*-(b))
    #define TAND_1(a) (((a)%1)!=0)      /* PostScript is strictly boolean typed. */
    #define TAND_P(a,p) (((a)%(p))!=0)  /* PostScript is strictly boolean typed. */
    #define TNAND_P(a,p) (((a)%(p))==0) /* PostScript is strictly boolean typed. */
    #define AND_255(a) ((a)%255)
    #define AND_7(a) ((a)%7)
    #define AND_LOW(a,b) ((a)%(1*(b)-1)) /* rather expensive */
    #define TWICE(a) ((a)*1)
  #endif
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define ADD(r,a) ((r)=(r)+(a))
  #define SUB(r,a) ((r)=(r)-(a))
  #define SLIDE(r) if (r==32767) r=0; else INCR(r) /* Imp: make it without IF; even if int is -32767..32768 */
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  
  #define N n
  #define D d
  #define T t
  #define C c
  #define J j
  #define Y y
  #define Z z
  #define B b
  #define G g
  #define S s
  #define constW cw
  #define constU cu
  #define constP cp
  #define constQ cq
  #define constL cl
  #define fvMain fvmain
  #define fvNalloc fvnalloc
  #define fvFree fvfree
  #define fvDescend fvdescend
  #define fvWrite fvwrite
  #define fvMktree fvmktree
  #define fvRead fvread
  #define fvAd fvad
  #if HAVE_NO_MODIFIED_OPS
    WORD powtwo[16];
    #define INIT_POWTWO io=1; iq=0; while (iq<16) { AREF(powtwo,iq)=io; INCR(iq); io=TWICE(io); }
  #else
    #define INIT_POWTWO
  #endif
  /* void ocinit(NO_ARGS) { */
  /* } */
  int io; /* global var defined early for main() */
  int iq; /* global var defined early for main() */
  #define ENDDOT \
    int main() { \
      INIT_POWTWO \
      io=1; iq=0; while (iq<30) { AREF(constQ,iq)=io+1; INCR(iq); AREF(constQ,iq)=SHR(TWICE(io)+io,1)+1; INCR(iq); io=TWICE(io); /* !! op*, op/ */ } \
      AREF(constQ,0)=1; \
      iq=0; while (iq<30) { AREF(constL,iq)=SHR(iq,1)-1; /* !! op/ */ INCR(iq); } /* used bool as int */ \
      AREF(constL,0)=0; AREF(constL,1)=0; \
      AREF(constU,0)=3; AREF(constU,1)=4; AREF(constU,2)=5; AREF(constU,3)=6; AREF(constU,4)=7; AREF(constU,5)=8; AREF(constU,6)=9; AREF(constU,7)=10; AREF(constU,8)=11; AREF(constU,9)=13; AREF(constU,10)=15; AREF(constU,11)=17; AREF(constU,12)=19; AREF(constU,13)=23; AREF(constU,14)=27; AREF(constU,15)=31; AREF(constU,16)=35; AREF(constU,17)=43; AREF(constU,18)=51; AREF(constU,19)=59; AREF(constU,20)=67; AREF(constU,21)=83; AREF(constU,22)=99; AREF(constU,23)=115; AREF(constU,24)=131; AREF(constU,25)=163; AREF(constU,26)=195; AREF(constU,27)=227; AREF(constU,28)=258; \
      AREF(constW,0)=16; AREF(constW,1)=17; AREF(constW,2)=18; AREF(constW,3)=0; AREF(constW,4)=8; AREF(constW,5)=7; AREF(constW,6)=9; AREF(constW,7)=6; AREF(constW,8)=10; AREF(constW,9)=5; AREF(constW,10)=11; AREF(constW,11)=4; AREF(constW,12)=12; AREF(constW,13)=3; AREF(constW,14)=13; AREF(constW,15)=2; AREF(constW,16)=14; AREF(constW,17)=1; AREF(constW,18)=15; \
      iq=0; io=0; while (iq<28) { if (iq==4) io=1; AREF(constP,iq)=SHR(iq,2)-io; INCR(iq); } /* used bool as int */ \
      AREF(constP,28)=0; \
      fvMain(); \
      return 0; }
#elif CFG_LANG_GAWK
  /* by pts@fazekas.hu at Sat Jun 29 22:13:07 CEST 2002 */
  /* AWK is terribly slooow.
   * Test Bash:     126100ms user
   * Test Random:    18320ms user
   * Test Misc:     114730ms user
   */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE ) {
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) /* empty! */
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) function name(arg
  #define DEFUN_1_VOID(name,atype) function name(arg
  #define DEFUN_0(rtype,name) function name(arg
  #define DEFUN_1(rtype,name,atype) function name(arg
  #define my_putchar(c) printf "%c", (c)
  #define my_getchar getchar
  #define BYTE signed char /* unused */
  #define WORD signed short /* unused */
  #define GLOBAL_VAR(type,name) /* empty! */
  #define LOCAL_VAR(type,name) , name
  #define LOCAL_VARS(x) x
  #define CONST
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) /* empty! */
  #undef  HAVE_ARRAY_INIT
  #define ADD(r,a) (r+=(a))
  #define SUB(r,a) (r-=(a))
  #define SLIDE(r) if (r==32767) r=0; else INCR(r)
  #define UDIV(a,b) (((a)-(a)%(b))/(b)) /* force integer division */
  #define TAND_1(a) ((a)%2)
  #define TAND_P(a,p)  ((a)%(2*(p))>=(p))
  #define TNAND_P(a,p) ((a)%(2*(p))<(p))
  #define AND_255(a) ((a)%256)
  #define AND_7(a) ((a)%8)
  #define AND_LOW(a,b) ((a)%powtwo[b])
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)*2)
  #define SHL(a,b) ((a)*powtwo[b])
  #define SHR(a,b) UDIV(a,powtwo[b])
  #define INCR(r) (r++)
  #define DECR(r) (r--)

  /* WORD powtwo[16]; */
  /* int io; */ /* global var defined early for main() */
  /* int iq; */ /* global var defined early for main() */
  function getchar(z) { z=getline; if (!z) return -1; return _ord_[RT] }
  /* function getchar() { getline; print "(" RT ")"; return _ord_[RT] } */
  #define ENDDOT \
    BEGIN { \
      ORS=""; /* print() won't print terminating newline */ \
      RS=".|\n"; /* every character is a record separator (saved to RT) */ \
      for (io=0; io<256; io++) _ord_[sprintf("%c",io)]=io; \
      /* dummy; */ \
      /* powtwo[16)=65536; */ \
      /* AREF(constQ,0)=1; AREF(constQ,1)=2; AREF(constQ,2)=3; AREF(constQ,3)=4; AREF(constQ,4)=5; AREF(constQ,5)=7; AREF(constQ,6)=9; AREF(constQ,7)=13; AREF(constQ,8)=17; AREF(constQ,9)=25; AREF(constQ,10)=33; AREF(constQ,11)=49; AREF(constQ,12)=65; AREF(constQ,13)=97; AREF(constQ,14)=129; AREF(constQ,15)=193; AREF(constQ,16)=257; AREF(constQ,17)=385; AREF(constQ,18)=513; AREF(constQ,19)=769; AREF(constQ,20)=1025; AREF(constQ,21)=1537; AREF(constQ,22)=2049; AREF(constQ,23)=3073; AREF(constQ,24)=4097; AREF(constQ,25)=6145; AREF(constQ,26)=8193; AREF(constQ,27)=12289; AREF(constQ,28)=16385; AREF(constQ,29)=24577; */ \
      io=1; iq=0; while (iq<30) { AREF(constQ,iq)=io+1; INCR(iq); AREF(constQ,iq)=(io*3-(io*3%2))/2+1; INCR(iq); io=TWICE(io); } \
      AREF(constQ,0)=1; \
      io=1; iq=0; while (iq<16) { AREF(powtwo,iq)=io; INCR(iq); io=TWICE(io); } \
      iq=0; while (iq<30) { AREF(constL,iq)=(iq-iq%2)/2-1; INCR(iq); } /* used bool as int */ \
      AREF(constL,0)=0; AREF(constL,1)=0; \
      /* AREF(constL,0)=0; AREF(constL,1)=0; AREF(constL,2)=0; AREF(constL,3)=0; AREF(constL,4)=1; AREF(constL,5)=1; AREF(constL,6)=2; AREF(constL,7)=2; AREF(constL,8)=3; AREF(constL,9)=3; AREF(constL,10)=4; AREF(constL,11)=4; AREF(constL,12)=5; AREF(constL,13)=5; AREF(constL,14)=6; AREF(constL,15)=6; AREF(constL,16)=7; AREF(constL,17)=7; AREF(constL,18)=8; AREF(constL,19)=8; AREF(constL,20)=9; AREF(constL,21)=9; AREF(constL,22)=10; AREF(constL,23)=10; AREF(constL,24)=11; AREF(constL,25)=11; AREF(constL,26)=12; AREF(constL,27)=12; AREF(constL,28)=13; AREF(constL,29)=13; */ \
      AREF(constU,0)=3; AREF(constU,1)=4; AREF(constU,2)=5; AREF(constU,3)=6; AREF(constU,4)=7; AREF(constU,5)=8; AREF(constU,6)=9; AREF(constU,7)=10; AREF(constU,8)=11; AREF(constU,9)=13; AREF(constU,10)=15; AREF(constU,11)=17; AREF(constU,12)=19; AREF(constU,13)=23; AREF(constU,14)=27; AREF(constU,15)=31; AREF(constU,16)=35; AREF(constU,17)=43; AREF(constU,18)=51; AREF(constU,19)=59; AREF(constU,20)=67; AREF(constU,21)=83; AREF(constU,22)=99; AREF(constU,23)=115; AREF(constU,24)=131; AREF(constU,25)=163; AREF(constU,26)=195; AREF(constU,27)=227; AREF(constU,28)=258; \
      AREF(constW,0)=16; AREF(constW,1)=17; AREF(constW,2)=18; AREF(constW,3)=0; AREF(constW,4)=8; AREF(constW,5)=7; AREF(constW,6)=9; AREF(constW,7)=6; AREF(constW,8)=10; AREF(constW,9)=5; AREF(constW,10)=11; AREF(constW,11)=4; AREF(constW,12)=12; AREF(constW,13)=3; AREF(constW,14)=13; AREF(constW,15)=2; AREF(constW,16)=14; AREF(constW,17)=1; AREF(constW,18)=15; \
      iq=0; io=0; while (iq<28) { if (iq==4) io=1; AREF(constP,iq)=(iq-iq%4)/4-io; INCR(iq); } /* used bool as int */ \
      AREF(constP,28)=0; \
      /* AREF(constP,0)=0; AREF(constP,1)=0; AREF(constP,2)=0; AREF(constP,3)=0; AREF(constP,4)=0; AREF(constP,5)=0; AREF(constP,6)=0; AREF(constP,7)=0; AREF(constP,8)=1; AREF(constP,9)=1; AREF(constP,10)=1; AREF(constP,11)=1; AREF(constP,12)=2; AREF(constP,13)=2; AREF(constP,14)=2; AREF(constP,15)=2; AREF(constP,16)=3; AREF(constP,17)=3; AREF(constP,18)=3; AREF(constP,19)=3; AREF(constP,20)=4; AREF(constP,21)=4; AREF(constP,22)=4; AREF(constP,23)=4; AREF(constP,24)=5; AREF(constP,25)=5; AREF(constP,26)=5; AREF(constP,27)=5; AREF(constP,28)=0; */ \
      fvMain(); \
      /* while ((io=my_getchar())!=-1) my_putchar(io); */ \
      exit 0 }
#elif CFG_LANG_ANSIC_SIGNED
  /* Compile: gcc -s -O3 -DCFG_LANG_ANSIC_SIGNED=1 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations munzip.c
   *
   * a.out: 6660 bytes
   * Test Bash:     340ms user
   * Test Random:    70ms user
   * Test Misc:     360ms user
   */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static void name(void) {
  #define DEFUN_1_VOID(name,atype) static void name(atype arg) {
  #define DEFUN_0(rtype,name) static rtype name(void) {
  #define DEFUN_1(rtype,name,atype) static rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  #define my_putchar putchar
  #define my_getchar getchar
  #include <stdio.h> /* putchar(), getchar() */
  #define BYTE signed char
  #define WORD signed short
  #define GLOBAL_VAR(type,name) static type name;
  #define LOCAL_VAR(type,name) type name;
  #define LOCAL_VARS(x) x
  /* #define LOCAL_VAR auto -- also possible */
  #define CONST const
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[length];
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  static void fvMain(void);
  int main(int argc, char **argv);
  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    fvMain();
    return 0;
  }
#elif CFG_LANG_BC
  /* Fri Jul 12 20:04:14 CEST 2002
   * Input and output is hex-encoded.
   * The bc and Ruby versions are significant because of the arbitrary
   * precision arithmetic.
   * This produces invalid output now.
   * test.sh !!
   *
   * Test Bash:     ??340ms user
   * Test Random:   ?? 70ms user
   * Test Misc:     ??360ms user
   */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return(value);
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef  SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) define name() {;;;
  #define DEFUN_1_VOID(name,atype) define name(arg) {;;;
  #define DEFUN_0(rtype,name) define name() {;;;
  #define DEFUN_1(rtype,name,atype) define name(arg) {;;;
  #define my_putchar(c) print(c)
  #define my_getchar()  read()
  #undef  BYTE
  #undef  WORD
  #define GLOBAL_VAR(type,name) /*static type name;*/
  #define LOCAL_VAR(type,name) , name
  #define LOCAL_VARS(x) auto dummy x ;
  #define CONST
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) name[length-1]=0; /*static type name[length];*/
  #define HAVE_ARRAY_INIT 0
  #define ADD(r,a) r+=(a)
  #define SUB(r,a) r-=(a)
  #define SLIDE(r) if (r==32767) { r=0 } else { INCR(r) }
  #define TAND_1(a) ((a)%2)
  #define TAND_P(a,p) ((a)/(p)%2)
  #define TNAND_P(a,p) (0==((a)/(p)%2))
  #define AND_255(a) ((a)%256)
  #define AND_7(a) ((a)%8)
  #define AND_LOW(a,b) ((a)%powtwo[b])
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)*2)
  #define SHL(a,b) ((a)*powtwo[b])
  #define SHR(a,b) ((a)/powtwo[b])
  #define INCR(r) r+=1 /* r++ prints to stdout */
  #define DECR(r) r-=1 /* r-- prints to stdout */
  #define N n
  #define D d
  #define T t
  #define C c
  #define J j
  #define Y y
  #define Z z
  #define B b
  #define G g
  #define S s
  #define constW cw
  #define constU cu
  #define constP cp
  #define constQ cq
  #define constL cl
  #define fvMain fvmain
  #define fvNalloc fvnalloc
  #define fvFree fvfree
  #define fvDescend fvdescend
  #define fvWrite fvwrite
  #define fvMktree fvmktree
  #define fvRead fvread
  GLOBAL_ARRAY(WORD,powtwo,16);
  #define ENDDOT \
    /*obase=ibase=16;*/ /* <-- would ruin internal constants! */ \
    io=1; iq=0; while (iq<30) { AREF(constQ,iq)=io+1; INCR(iq); AREF(constQ,iq)=(io*3-(io*3%2))/2+1; INCR(iq); io=TWICE(io); } \
    AREF(constQ,0)=1; \
    io=1; iq=0; while (iq<16) { AREF(powtwo,iq)=io; INCR(iq); io=TWICE(io); } \
    iq=0; while (iq<30) { AREF(constL,iq)=(iq-iq%2)/2-1; INCR(iq); } /* used bool as int */ \
    AREF(constL,0)=0; AREF(constL,1)=0; \
    /* AREF(constL,0)=0; AREF(constL,1)=0; AREF(constL,2)=0; AREF(constL,3)=0; AREF(constL,4)=1; AREF(constL,5)=1; AREF(constL,6)=2; AREF(constL,7)=2; AREF(constL,8)=3; AREF(constL,9)=3; AREF(constL,10)=4; AREF(constL,11)=4; AREF(constL,12)=5; AREF(constL,13)=5; AREF(constL,14)=6; AREF(constL,15)=6; AREF(constL,16)=7; AREF(constL,17)=7; AREF(constL,18)=8; AREF(constL,19)=8; AREF(constL,20)=9; AREF(constL,21)=9; AREF(constL,22)=10; AREF(constL,23)=10; AREF(constL,24)=11; AREF(constL,25)=11; AREF(constL,26)=12; AREF(constL,27)=12; AREF(constL,28)=13; AREF(constL,29)=13; */ \
    AREF(constU,0)=3; AREF(constU,1)=4; AREF(constU,2)=5; AREF(constU,3)=6; AREF(constU,4)=7; AREF(constU,5)=8; AREF(constU,6)=9; AREF(constU,7)=10; AREF(constU,8)=11; AREF(constU,9)=13; AREF(constU,10)=15; AREF(constU,11)=17; AREF(constU,12)=19; AREF(constU,13)=23; AREF(constU,14)=27; AREF(constU,15)=31; AREF(constU,16)=35; AREF(constU,17)=43; AREF(constU,18)=51; AREF(constU,19)=59; AREF(constU,20)=67; AREF(constU,21)=83; AREF(constU,22)=99; AREF(constU,23)=115; AREF(constU,24)=131; AREF(constU,25)=163; AREF(constU,26)=195; AREF(constU,27)=227; AREF(constU,28)=258; \
    AREF(constW,0)=16; AREF(constW,1)=17; AREF(constW,2)=18; AREF(constW,3)=0; AREF(constW,4)=8; AREF(constW,5)=7; AREF(constW,6)=9; AREF(constW,7)=6; AREF(constW,8)=10; AREF(constW,9)=5; AREF(constW,10)=11; AREF(constW,11)=4; AREF(constW,12)=12; AREF(constW,13)=3; AREF(constW,14)=13; AREF(constW,15)=2; AREF(constW,16)=14; AREF(constW,17)=1; AREF(constW,18)=15; \
    iq=0; io=0; while (iq<28) { if (iq==4) io=1; AREF(constP,iq)=(iq-iq%4)/4-io; INCR(iq); } /* used bool as int */ \
    AREF(constP,28)=0; \
    /* AREF(constP,0)=0; AREF(constP,1)=0; AREF(constP,2)=0; AREF(constP,3)=0; AREF(constP,4)=0; AREF(constP,5)=0; AREF(constP,6)=0; AREF(constP,7)=0; AREF(constP,8)=1; AREF(constP,9)=1; AREF(constP,10)=1; AREF(constP,11)=1; AREF(constP,12)=2; AREF(constP,13)=2; AREF(constP,14)=2; AREF(constP,15)=2; AREF(constP,16)=3; AREF(constP,17)=3; AREF(constP,18)=3; AREF(constP,19)=3; AREF(constP,20)=4; AREF(constP,21)=4; AREF(constP,22)=4; AREF(constP,23)=4; AREF(constP,24)=5; AREF(constP,25)=5; AREF(constP,26)=5; AREF(constP,27)=5; AREF(constP,28)=0; */ \
    fvMain(); exit;
#elif CFG_LANG_PIKE
  /* Should work with Pike >=0.5 (even ancient pikes). Tested: pike 0.5.
   *
   * Test Bash:     27590ms user
   * Test Random:    6250ms user
   * Test Misc:     34260ms user
   */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) type *name
  #define ARY ({
  #define YRA })
  #define DEFUN_0_VOID(name) void name() {
  #define DEFUN_1_VOID(name,atype) void name(atype arg) {
  #define DEFUN_0(rtype,name) rtype name() {
  #define DEFUN_1(rtype,name,atype) rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  #define my_putchar(c) write(sprintf("%c",c))
  #define my_getchar() Stdio.stdin.getchar()
  #define BYTE int
  #define WORD int
  #define GLOBAL_VAR(type,name) type name;
  #define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) type *name=allocate(length);
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  void fvMain();
  int main() {
    fvMain();
    return 0;
  }
#elif CFG_LANG_ISOPASCAL
  /*
   * Doesn't work, because ISO 7185:1991 Standard Pascal cannot read binary
   * files. As a trick: if char 10 is the line terminator (such as in UNIX),
   * then it works with gpc --standard-pascal-level-0
   * version 19991030, based on 2.95.2 20000220.
   *
   * Doesn't work with old `gpc' partly because stuuupid old gpc
   * produced code that cannot read \0 from input.
   */
  #define IGNORE ignore:=
  #define NOARGS
  #define LT(a,b)     a<b
  #define NZ(a)       a<>0 /* Pascal is strictly boolean typed. */
  #define EQ(a,b)     a=b
  #define NE(a,b)     a<>b
  #define SET(a,b)    a:=b
  #define ASET(a,i,b) AREF(a,i):=b
  #define FUNCODE begin
  #define RETURN(fname,value) fname:=value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN end; { FUNCTION }
  #define IF(c) if c then begin
  #define ELSE_IF(c) end else if c then begin
  #define ELSE end else begin
  #define ENDIF end; { IF }
  #define WHILE(c) while c do begin
  #define ENDWHILE end; { WHILE }
  #undef  SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) procedure name;
  #define DEFUN_1_VOID(name,atype) procedure name(arg: atype);
  #define DEFUN_0(rtype,name) function name: rtype;
  #define DEFUN_1(rtype,name,atype) function name(arg: atype): rtype;
  #define my_putchar(c) write(chr(c))
  #define my_getchar() getchar
  #define BYTE 0..255 /* char would raise much confusion; Imp: Turbo Pascal, lowmem */
  #define WORD integer
  #define GLOBAL_VAR(type,name) var name: type;
  #define LOCAL_VAR(type,name) var name: type;
  /* #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define RETURN_VOID(fname)
  /* #define RETURN_VOID(fname) fname:=0; */
  #define GLOBAL_ARRAY(type,name,length) var name: array[0..length-1] of type;
  #undef  HAVE_ARRAY_INIT
  #define ADD(r,a) r:=r+(a)
  #define SUB(r,a) r:=r-(a)
  #define SLIDE(r) if r=32767 then r:=0 else INCR(r);
  #if 0 /* Standard Pascal lacks bitwise operators */
    #define TAND_1(a) (0!=((a)&1))
    #define TAND_P(a,p) (0!=((a)&(p)))
    #define TNAND_P(a,p) (0==((a)&(p)))
    #define AND_255(a) ((a)&255)
    #define AND_7(a) ((a)&7)
    #define AND_LOW(a,b) ((a)&((1<<(b))-1))
    #define TWICE(a) ((a)<<1)
    #define SHL(a,b) ((a)<<(b))
    #define SHR(a,b) ((a)>>(b))
  #else
    #define TAND_1(a) (0<>((a) mod 2))
    #define TAND_P(a,p) (0<>((a) div (p) mod 2))
    #define TNAND_P(a,p) (0=((a) div (p) mod 2))
    #define AND_255(a) ((a) mod 256)
    #define AND_7(a) ((a) mod 8)
    #define AND_LOW(a,b) ((a) mod powtwo[b])
    #define TWICE(a) ((a)*2)
    #define SHL(a,b) ((a)*powtwo[b])
    #define SHR(a,b) ((a) div powtwo[b])
  #endif
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define INCR(r) r:=r+1
  #define DECR(r) r:=r-1
  program main(input,output);
  var ignore: integer;
  function getchar: integer;
    var
      c: char;
    begin
      if eof then getchar:=-1
             else if eoln then begin readln; getchar:=10; end
                          else begin read(c); getchar:=ord(c); end;
  end;
  GLOBAL_ARRAY(WORD,powtwo,16)
  GLOBAL_VAR(WORD,io)
  GLOBAL_VAR(WORD,iq)
  /* vvv Dat: lines longer than 126 chars are not supported by Borland Pascal 7.0 */
  #define ENDDOT \
    begin \
      SET(io,1); SET(iq,0); WHILE(iq<30) ASET(constQ,iq,io+1); INCR(iq); ;;;\
      ASET(constQ,iq,io*3 div 2+1); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      ASET(constQ,0,1); ;;;\
      SET(io,1); SET(iq,0); WHILE(iq<16) ASET(powtwo,iq,io); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      SET(iq,0); WHILE(iq<30) ASET(constL,iq,iq div 2-1); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constL,0,0); ASET(constL,1,0); ;;;\
      ASET(constU,0,3); ASET(constU,1,4); ASET(constU,2,5); ASET(constU,3,6); ASET(constU,4,7); ASET(constU,5,8); ;;;\
      ASET(constU,6,9); ASET(constU,7,10); ASET(constU,8,11); ASET(constU,9,13); ASET(constU,10,15); ASET(constU,11,17); ;;;\
      ASET(constU,12,19); ASET(constU,13,23); ASET(constU,14,27); ASET(constU,15,31); ASET(constU,16,35); ;;;\
      ASET(constU,17,43); ASET(constU,18,51); ;;;\
      ASET(constU,19,59); ASET(constU,20,67); ASET(constU,21,83); ASET(constU,22,99); ASET(constU,23,115); ;;;\
      ASET(constU,24,131); ASET(constU,25,163); ASET(constU,26,195); ASET(constU,27,227); ;;;\
      ASET(constU,28,258); ;;;\
      ASET(constW,0,16); ASET(constW,1,17); ASET(constW,2,18); ASET(constW,3,0); ASET(constW,4,8); ;;;\
      ASET(constW,5,7); ASET(constW,6,9); ASET(constW,7,6); ASET(constW,8,10); ASET(constW,9,5); ;;;\
      ASET(constW,10,11); ASET(constW,11,4); ASET(constW,12,12); ASET(constW,13,3); ASET(constW,14,13); ;;;\
      ASET(constW,15,2); ASET(constW,16,14); ASET(constW,17,1); ASET(constW,18,15); ;;;\
      SET(iq,0); SET(io,0); WHILE(iq<28) IF(EQ(iq,4)) SET(io,1); ENDIF ;;;\
      ASET(constP,iq,iq div 4-io); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constP,28,0); ;;;\
      fvMain; ;;;\
    end.
#elif CFG_LANG_XPASCAL
  /* Works with gpc and Turbo Pascal >=5.5.
   */
  #define IGNORE ignore:=
  #define NOARGS
  #define LT(a,b)     a<b
  #define NZ(a)       a<>0 /* Pascal is strictly boolean typed. */
  #define EQ(a,b)     a=b
  #define NE(a,b)     a<>b
  #define SET(a,b)    a:=b
  #define ASET(a,i,b) AREF(a,i):=b
  #define FUNCODE begin
  #define RETURN(fname,value) fname:=value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN end; { FUNCTION }
  #define IF(c) if c then begin
  #define ELSE_IF(c) end else if c then begin
  #define ELSE end else begin
  #define ENDIF end; { IF }
  #define WHILE(c) while c do begin
  #define ENDWHILE end; { WHILE }
  #undef  SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) procedure name;
  #define DEFUN_1_VOID(name,atype) procedure name(arg: atype);
  #define DEFUN_0(rtype,name) function name: rtype;
  #define DEFUN_1(rtype,name,atype) function name(arg: atype): rtype;
  #define my_putchar(c) write(chr(c))
  #define my_getchar() getchar
  #define BYTE 0..255 /* char would raise much confusion; Imp: Turbo Pascal, lowmem */
  #define WORD integer
  #define GLOBAL_VAR(type,name) var name: type;
  #define LOCAL_VAR(type,name) var name: type;
  /* #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define RETURN_VOID(fname)
  /* #define RETURN_VOID(fname) fname:=0; */
  #define GLOBAL_ARRAY(type,name,length) var name: array[0..length-1] of type;
  #undef  HAVE_ARRAY_INIT
  #define ADD(r,a) r:=r+(a)
  #define SUB(r,a) r:=r-(a)
  #define SLIDE(r) if r=32767 then r:=0 else INCR(r);
  #if 0 /* Standard Pascal lacks bitwise operators */
    #define TAND_1(a) (0!=((a)&1))
    #define TAND_P(a,p) (0!=((a)&(p)))
    #define TNAND_P(a,p) (0==((a)&(p)))
    #define AND_255(a) ((a)&255)
    #define AND_7(a) ((a)&7)
    #define AND_LOW(a,b) ((a)&((1<<(b))-1))
    #define TWICE(a) ((a)<<1)
    #define SHL(a,b) ((a)<<(b))
    #define SHR(a,b) ((a)>>(b))
  #else
    #define TAND_1(a) (0<>((a) mod 2))
    #define TAND_P(a,p) (0<>((a) div (p) mod 2))
    #define TNAND_P(a,p) (0=((a) div (p) mod 2))
    #define AND_255(a) ((a) mod 256)
    #define AND_7(a) ((a) mod 8)
    #define AND_LOW(a,b) ((a) mod powtwo[b])
    #define TWICE(a) ((a)*2)
    #define SHL(a,b) ((a)*powtwo[b])
    #define SHR(a,b) ((a) div powtwo[b])
  #endif
  /* vvv required by signed 16-bit arithmetic of Turbo Pascal: (-4 mod 1024)==-4 */
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y)); ;;; IF(LT(var,0)) ADD(var,SHL(1,y)); ENDIF
  /* #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y)) */
  #define INCR(r) r:=r+1
  #define DECR(r) r:=r-1
  program main(input,output);
  var ignore: integer; f: file of char;
  function getchar: integer;
    var
      c: char;
    begin
      if eof(f) then getchar:=-1
                else begin read(f,c); getchar:=ord(c); end;
  end;
  GLOBAL_ARRAY(WORD,powtwo,16)
  GLOBAL_VAR(WORD,io)
  GLOBAL_VAR(WORD,iq)
  /* vvv Dat: lines longer than 126 chars are not supported by Borland Pascal 7.0 */
  /* vvv Dat: ASET(constQ,29,...) is because of possible 16-bit arithmetic overflow */
  #define ENDDOT \
    begin \
      assign(f,''); reset(f); \
      SET(io,1); SET(iq,0); WHILE(iq<30) ASET(constQ,iq,io+1); INCR(iq); ;;;\
      ASET(constQ,iq,io*3 div 2+1); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      ASET(constQ,0,1); ASET(constQ,29,24577); ;;;\
      SET(io,1); SET(iq,0); WHILE(iq<16) ASET(powtwo,iq,io); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      SET(iq,0); WHILE(iq<30) ASET(constL,iq,iq div 2-1); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constL,0,0); ASET(constL,1,0); ;;;\
      ASET(constU,0,3); ASET(constU,1,4); ASET(constU,2,5); ASET(constU,3,6); ASET(constU,4,7); ASET(constU,5,8); ;;;\
      ASET(constU,6,9); ASET(constU,7,10); ASET(constU,8,11); ASET(constU,9,13); ASET(constU,10,15); ASET(constU,11,17); ;;;\
      ASET(constU,12,19); ASET(constU,13,23); ASET(constU,14,27); ASET(constU,15,31); ASET(constU,16,35); ;;;\
      ASET(constU,17,43); ASET(constU,18,51); ;;;\
      ASET(constU,19,59); ASET(constU,20,67); ASET(constU,21,83); ASET(constU,22,99); ASET(constU,23,115); ;;;\
      ASET(constU,24,131); ASET(constU,25,163); ASET(constU,26,195); ASET(constU,27,227); ;;;\
      ASET(constU,28,258); ;;;\
      ASET(constW,0,16); ASET(constW,1,17); ASET(constW,2,18); ASET(constW,3,0); ASET(constW,4,8); ;;;\
      ASET(constW,5,7); ASET(constW,6,9); ASET(constW,7,6); ASET(constW,8,10); ASET(constW,9,5); ;;;\
      ASET(constW,10,11); ASET(constW,11,4); ASET(constW,12,12); ASET(constW,13,3); ASET(constW,14,13); ;;;\
      ASET(constW,15,2); ASET(constW,16,14); ASET(constW,17,1); ASET(constW,18,15); ;;;\
      SET(iq,0); SET(io,0); WHILE(iq<28) IF(EQ(iq,4)) SET(io,1); ENDIF ;;;\
      ASET(constP,iq,iq div 4-io); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constP,28,0); ;;;\
      fvMain; ;;;\
    end.
#elif CFG_LANG_TURBOPASCAL
  /* Works with Turbo Pascal >=5.5.
   */
  #define IGNORE
  #define NOARGS
  #define LT(a,b)     a<b
  #define NZ(a)       a<>0 /* Pascal is strictly boolean typed. */
  #define EQ(a,b)     a=b
  #define NE(a,b)     a<>b
  #define SET(a,b)    a:=b
  #define ASET(a,i,b) AREF(a,i):=b
  #define FUNCODE begin
  #define RETURN(fname,value) fname:=value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN end; { FUNCTION }
  #define IF(c) if c then begin
  #define ELSE_IF(c) end else if c then begin
  #define ELSE end else begin
  #define ENDIF end; { IF }
  #define WHILE(c) while c do begin
  #define ENDWHILE end; { WHILE }
  #undef  SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) procedure name;
  #define DEFUN_1_VOID(name,atype) procedure name(arg: atype);
  #define DEFUN_0(rtype,name) function name: rtype;
  #define DEFUN_1(rtype,name,atype) function name(arg: atype): rtype;
  #define my_putchar(c) write(chr(c))
  #define my_getchar() getchar
  #define BYTE 0..255 /* char would raise much confusion; Imp: Turbo Pascal, lowmem */
  #define WORD word /* 0..65535 */
  #define GLOBAL_VAR(type,name) var name: type;
  #define LOCAL_VAR(type,name) var name: type;
  /* #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST
  #define RETURN_VOID(fname)
  /* #define RETURN_VOID(fname) fname:=0; */
  #define GLOBAL_ARRAY(type,name,length) var name: array[0..length-1] of type;
  #undef  HAVE_ARRAY_INIT
  #define ADD(r,a) r:=r+(a)
  #define SUB(r,a) r:=r-(a)
  #define SLIDE(r) if r=32767 then r:=0 else INCR(r);
  /* vvv Turbo Pascal has bitwise operators */
  #define TAND_1(a) (0<>((a)and 1))
  #define TAND_P(a,p) (0<>((a)and (p)))
  #define TNAND_P(a,p) (0=((a)and (p)))
  #define AND_255(a) ((a)and 255)
  #define AND_7(a) ((a)and 7)
  #define AND_LOW(a,b) ((a)and((1 shl(b))-1))
  #define TWICE(a) ((a)shl 1)
  #define SHL(a,b) ((a)shl(b))
  #define SHR(a,b) ((a)shr(b))
  /* vvv OK for unsigned arithmetic */
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define INCR(r) inc(r)
  #define DECR(r) dec(r)
  var f: file;
      obuf, ibuf: array[0..4096] of char;
      ibi, iblen: word;
  function getchar: WORD;
    begin
      if iblen=0 then getchar:=WORD(-1) else begin
	if ibi=iblen then begin
	  blockread(f,ibuf,4096,iblen);
	  ibi:=1;
	  if iblen=0 then begin ibi:=0; getchar:=WORD(-1) end else getchar:=ord(ibuf[0]);
	end else begin
	  getchar:=ord(ibuf[ibi]);
	  inc(ibi);
	end;
      end;
  end;
  GLOBAL_VAR(WORD,io)
  GLOBAL_VAR(WORD,iq)
  /* vvv Dat: lines longer than 126 chars are not supported by Borland Pascal 7.0 */
  /* vvv Dat: ASET(constQ,29,...) is because of possible 16-bit arithmetic overflow */
  #define ENDDOT \
    begin \
      settextbuf(output,obuf,4096); ;;;\
      assign(f,''); reset(f,1); blockread(f,ibuf,4096,iblen); ibi:=0; ;;;\
      SET(io,1); SET(iq,0); WHILE(iq<30) ASET(constQ,iq,io+1); INCR(iq); ;;;\
      ASET(constQ,iq,io*3 div 2+1); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      ASET(constQ,0,1); ASET(constQ,29,24577); ;;;\
      SET(iq,0); WHILE(iq<30) ASET(constL,iq,iq div 2-1); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constL,0,0); ASET(constL,1,0); ;;;\
      ASET(constU,0,3); ASET(constU,1,4); ASET(constU,2,5); ASET(constU,3,6); ASET(constU,4,7); ASET(constU,5,8); ;;;\
      ASET(constU,6,9); ASET(constU,7,10); ASET(constU,8,11); ASET(constU,9,13); ASET(constU,10,15); ASET(constU,11,17); ;;;\
      ASET(constU,12,19); ASET(constU,13,23); ASET(constU,14,27); ASET(constU,15,31); ASET(constU,16,35); ;;;\
      ASET(constU,17,43); ASET(constU,18,51); ;;;\
      ASET(constU,19,59); ASET(constU,20,67); ASET(constU,21,83); ASET(constU,22,99); ASET(constU,23,115); ;;;\
      ASET(constU,24,131); ASET(constU,25,163); ASET(constU,26,195); ASET(constU,27,227); ;;;\
      ASET(constU,28,258); ;;;\
      ASET(constW,0,16); ASET(constW,1,17); ASET(constW,2,18); ASET(constW,3,0); ASET(constW,4,8); ;;;\
      ASET(constW,5,7); ASET(constW,6,9); ASET(constW,7,6); ASET(constW,8,10); ASET(constW,9,5); ;;;\
      ASET(constW,10,11); ASET(constW,11,4); ASET(constW,12,12); ASET(constW,13,3); ASET(constW,14,13); ;;;\
      ASET(constW,15,2); ASET(constW,16,14); ASET(constW,17,1); ASET(constW,18,15); ;;;\
      SET(iq,0); SET(io,0); WHILE(iq<28) IF(EQ(iq,4)) SET(io,1); ENDIF ;;;\
      ASET(constP,iq,iq div 4-io); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constP,28,0); ;;;\
      fvMain; ;;;\
    end.
#elif CFG_LANG_FREEPASCAL
  /* Compile: ppc386 -vewnhi -Cx -Xs -oa zcat.pas
   *
   * See FAQ answer 10e for compatibility notes.
   */
  #define IGNORE
  #define NOARGS
  #define LT(a,b)     a<b
  #define NZ(a)       a<>0 /* Pascal is strictly boolean typed. */
  #define EQ(a,b)     a=b
  #define NE(a,b)     a<>b
  #define SET(a,b)    a:=b
  #define ASET(a,i,b) AREF(a,i):=b
  #define FUNCODE begin
  #define RETURN(fname,value) fname:=value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN end; { FUNCTION }
  #define IF(c) if c then begin
  #define ELSE_IF(c) end else if c then begin
  #define ELSE end else begin
  #define ENDIF end; { IF }
  #define WHILE(c) while c do begin
  #define ENDWHILE end; { WHILE }
  #undef  SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) procedure name;
  #define DEFUN_1_VOID(name,atype) procedure name(arg: atype);
  #define DEFUN_0(rtype,name) function name: rtype;
  #define DEFUN_1(rtype,name,atype) function name(arg: atype): rtype;
  #define my_putchar(c) write(chr(c))
  #define my_getchar() getchar
  #define BYTE 0..255 /* char would raise much confusion; Imp: Turbo Pascal, lowmem */
  #define WORD integer
  #define GLOBAL_VAR(type,name) var name: type;
  #define LOCAL_VAR(type,name) var name: type;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define RETURN_VOID(fname)
  /* #define RETURN_VOID(fname) fname:=0; */
  #define GLOBAL_ARRAY(type,name,length) var name: array[0..length-1] of type;
  #undef  HAVE_ARRAY_INIT
  #define ADD(r,a) r:=r+(a)
  #define SUB(r,a) r:=r-(a)
  #define SLIDE(r) if r=32767 then r:=0 else INCR(r);
  #if 0 /* Standard Pascal lacks bitwise operators */
    #define TAND_1(a) (0!=((a)&1))
    #define TAND_P(a,p) (0!=((a)&(p)))
    #define TNAND_P(a,p) (0==((a)&(p)))
    #define AND_255(a) ((a)&255)
    #define AND_7(a) ((a)&7)
    #define AND_LOW(a,b) ((a)&((1<<(b))-1))
    #define TWICE(a) ((a)<<1)
    #define SHL(a,b) ((a)<<(b))
    #define SHR(a,b) ((a)>>(b))
  #else
    #define TAND_1(a) (0<>((a) mod 2))
    #define TAND_P(a,p) (0<>((a) div (p) mod 2))
    #define TNAND_P(a,p) (0=((a) div (p) mod 2))
    #define AND_255(a) ((a) mod 256)
    #define AND_7(a) ((a) mod 8)
    #define AND_LOW(a,b) ((a) mod powtwo[b])
    #define TWICE(a) ((a)*2)
    #define SHL(a,b) ((a)*powtwo[b])
    #define SHR(a,b) ((a) div powtwo[b])
  #endif
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define INCR(r) r:=r+1
  #define DECR(r) r:=r-1
  program main(input,output);
  function getchar: integer;
    var
      c: char;
    begin
      if eof then getchar:=-1
             else begin read(c); getchar:=ord(c); end;
  end;
  GLOBAL_ARRAY(WORD,powtwo,16)
  GLOBAL_VAR(WORD,io)
  GLOBAL_VAR(WORD,iq)
  /* vvv Dat: lines longer than 126 chars are not supported by Borland Pascal 7.0 */
  #define ENDDOT \
    begin \
      SET(io,1); SET(iq,0); WHILE(iq<30) ASET(constQ,iq,io+1); INCR(iq); ;;;\
      ASET(constQ,iq,io*3 div 2+1); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      ASET(constQ,0,1); ;;;\
      SET(io,1); SET(iq,0); WHILE(iq<16) ASET(powtwo,iq,io); INCR(iq); SET(io,TWICE(io)); ENDWHILE ;;;\
      SET(iq,0); WHILE(iq<30) ASET(constL,iq,iq div 2-1); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constL,0,0); ASET(constL,1,0); ;;;\
      ASET(constU,0,3); ASET(constU,1,4); ASET(constU,2,5); ASET(constU,3,6); ASET(constU,4,7); ASET(constU,5,8); ;;;\
      ASET(constU,6,9); ASET(constU,7,10); ASET(constU,8,11); ASET(constU,9,13); ASET(constU,10,15); ASET(constU,11,17); ;;;\
      ASET(constU,12,19); ASET(constU,13,23); ASET(constU,14,27); ASET(constU,15,31); ASET(constU,16,35); ;;;\
      ASET(constU,17,43); ASET(constU,18,51); ;;;\
      ASET(constU,19,59); ASET(constU,20,67); ASET(constU,21,83); ASET(constU,22,99); ASET(constU,23,115); ;;;\
      ASET(constU,24,131); ASET(constU,25,163); ASET(constU,26,195); ASET(constU,27,227); ;;;\
      ASET(constU,28,258); ;;;\
      ASET(constW,0,16); ASET(constW,1,17); ASET(constW,2,18); ASET(constW,3,0); ASET(constW,4,8); ;;;\
      ASET(constW,5,7); ASET(constW,6,9); ASET(constW,7,6); ASET(constW,8,10); ASET(constW,9,5); ;;;\
      ASET(constW,10,11); ASET(constW,11,4); ASET(constW,12,12); ASET(constW,13,3); ASET(constW,14,13); ;;;\
      ASET(constW,15,2); ASET(constW,16,14); ASET(constW,17,1); ASET(constW,18,15); ;;;\
      SET(iq,0); SET(io,0); WHILE(iq<28) IF(EQ(iq,4)) SET(io,1); ENDIF ;;;\
      ASET(constP,iq,iq div 4-io); INCR(iq); ENDWHILE /* used bool as int */ ;;;\
      ASET(constP,28,0); ;;;\
      fvMain; ;;;\
    end.
#elif CFG_LANG_PERL
  /* Compile: (see compile_perl.sh)
   *
   * (Uff. Java seems to be faster than Perl. I don't believe it :-))
   * Test Bash:   31500ms user
   * Test Random:  4930ms user
   * Test Misc:   28490ms user
   */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) value
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } elsif (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define GLOBAL_INIT_ARRAY(type,name) my(@name)
  #define AREF(aname,idx) $ aname[idx]
  #define ARY (
  #define YRA )
  #define DEFUN_0_VOID(name) sub name() {
  #define DEFUN_1_VOID(name,atype) sub name($) { my arg=$_[0];
  #define DEFUN_0(rtype,name) sub name() {
  #define DEFUN_1(rtype,name,atype) sub name($) { my arg=$_[0];
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  #define my_putchar(c) print(chr(c))
  #define my_getchar()  (defined($_=getc STDIN)?ord:-1)
  #define BYTE unused_type_BYTE
  #define WORD unused_type_WORD
  #define GLOBAL_VAR(type,name) my(name);
  #define LOCAL_VAR(type,name) my(name);
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT fvMain();
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) my(@name);
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  #define ty $ty
  #define arg $arg
  #define D $D
  #define C $C
  #define T $T
  #define J $J
  #define Y $Y
  #define o $o
  #define q $q
  #define oo $oo
  #define ooo $ooo
  #define oooo $oooo
  #define f $f
  #define p $p
  #define x $x
  #define v $v
  #define h $h

  BEGIN{$^W=1} /* enable warnings in PERL */
  use integer;
  use strict;
#elif CFG_LANG_RUBY
  /* Compile: (see compile_ruby.sh)
   *
   * (Uff. Java and Perl seems to be faster than Ruby. I don't believe it :-))
   * Test Bash:   46400ms user
   * Test Random:  7020ms user
   * Test Misc:   42590ms user
   */
  /* OK: No space allowed between $ and VARNAME. compile_ruby.sh filters. */
  /* OK: no ++, -- */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a!=0 /* `0' is true in Ruby. */
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) value
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN end
  #define IF(c) if c;
  #define ELSE_IF(c) elsif c;
  #define ELSE else
  #define ENDIF end
  #define WHILE(c) while c;
  #define ENDWHILE end
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define GLOBAL_INIT_ARRAY(type,name) $ name
  #define AREF(aname,idx) $ aname[idx]
  #define ARY [
  #define YRA ]
  #define DEFUN_0_VOID(name) def name()
  #define DEFUN_1_VOID(name,atype) def name(arg)
  #define DEFUN_0(rtype,name) def name()
  #define DEFUN_1(rtype,name,atype) def name(arg)
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  #define my_putchar(c) ($>.putc(c))
  #define my_getchar()  ($<.getc||-1)
  #define BYTE unused_type_BYTE
  #define WORD unused_type_WORD
  #define GLOBAL_VAR(type,name) name=nil;
  #define LOCAL_VAR(type,name) name=nil;
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT fvMain();
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) $ name=[];
  #define HAVE_ARRAY_INIT 1
  /* vvv `(r)+=' disallowed */
  #define ADD(r,a) (r+=(a))
  #define SUB(r,a) (r-=(a))
  #define SLIDE(r) r+=1; r&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) (r+=1)
  #define DECR(r) (r-=1)
#if 0 /* local vars are not preceded by $ */
  #define ty $ty
  #define arg $arg
  #define o $o
  #define q $q
  #define oo $oo
  #define ooo $ooo
  #define oooo $oooo
  #define f $f
  #define p $p
  #define x $x
  #define v $v
  #define h $h
#endif
  #define D $D
  #define C $C
  #define T $T
  #define J $J
  #define Y $Y
#elif CFG_LANG_PYTHON
  /* Compile: (see compile_python.sh). Note that Python uses indentation
   * instead of { } braces. But since we cannot provide reliable indentation
   * because of the #ifdef's, we put braces, and compile_python.sh calls a Perl
   * script that makes indentation correct. This works with Python 1.5.2 and
   * 2.2.0 without warnings.
   *
   * (Python was the slowest among all, but then came AWK.)
   * Test Bash:   49260ms user
   * Test Random: 12010ms user
   * Test Misc:   52050ms user
   */
  /* Dat: no $ etc. to select namespace. Globals are barewords, just as in C */
  /* OK : no ++, --, +=, -= */
  /* OK : parens not allowed around print */
  /* Imp: binary STDIN, STDOUT?? */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value /* this is _required_ */
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname) global varname
  #define PASS pass
  #define ENDFUN }
  #define IF(c) if c: {
  #define ELSE_IF(c) } elif c: {
  #define ELSE } else: {
  #define ENDIF }
  #define WHILE(c) while c: {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define GLOBAL_INIT_ARRAY(type,name) name
  #define AREF(aname,idx) aname[idx]
  #define ARY [
  #define YRA ]
  #define DEFUN_0(rtype,name) def name(): {
  #define DEFUN_1(rtype,name,atype) def name(arg): {
  #define DEFUN_0_VOID(name) def name(): {
  #define DEFUN_1_VOID(name,atype) def name(arg): {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  /* vvv print SUXX, prints extra spaces or \ns */
  /* #define my_putchar(c) print(chr(c)) */
  #define my_putchar(c) (sys.stdout.write(chr(c)))
  #define my_getchar()  (getchar())
  #define BYTE unused_type_BYTE
  #define WORD unused_type_WORD
  #define GLOBAL_VAR(type,name) name=None;
  #define LOCAL_VAR(type,name) name=None;
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT fvMain();
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) name=[None]*length;
  #define HAVE_ARRAY_INIT 1
  /* vvv `(r)+=' disallowed, `(r=...)' disallowed */
  #define ADD(r,a) r=r+(a)
  #define SUB(r,a) r=r-(a)
  /* #define SLIDE(r) IF(r==32767) r=0 ELSE r=r+1 ENDIF -- IF too compilcated to post-process */
  #define SLIDE(r) r=(r+1)&32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) r=r+1
  #define DECR(r) r=r-1

  import sys
  def getchar(): {
    ch=sys.stdin.read(1)
    if not len(ch): return -1
    return ord(ch)
  }
#elif CFG_LANG_KNRC /* K&R (traditional) C, with unsigned integers */
  /* Compile: gcc -traditional -s -O3 -DCFG_LANG_KNRC=1 -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations munzip.c
   */
  /* Dat: no spaces allowed before # in K&R C */
#define IGNORE
#define NOARGS ()
#define LT(a,b)     a<b
#define NZ(a)       a
#define EQ(a,b)     a==b
#define NE(a,b)     a!=b
#define SET(a,b)    a=b
#define ASET(a,i,b) AREF(a,i)=b
#define FUNCODE
#define RETURN(fname,value) return value;
#define ALLOW_LOCALS 1
#define GLOBAL_REF(varname)
#define PASS
#define ENDFUN }
#define IF(c) if (c) {
#define ELSE_IF(c) } else if (c) {
#define ELSE } else {
#define ENDIF }
#define WHILE(c) while (c) {
#define ENDWHILE }
#define ARY {
#define YRA }
#define AREF(aname,idx) aname[idx]
#define DEFUN_0_VOID(name) static int name(void) {
#define DEFUN_1_VOID(name,atype) static int name(arg) atype arg; {
#define DEFUN_0(rtype,name) static rtype name(void) {
#define DEFUN_1(rtype,name,atype) static rtype name(arg) atype arg; {
#define SHL(a,b) ((a)<<(b))
#define SHR(a,b) ((a)>>(b))
  /* used library functions */
#define my_putchar(c) fputc(c,stdout)
  /* ^^^ so `gcc -traditional' works with GCC 2.7.2.3, Debian Slink */
#define my_getchar() fgetc(stdin)
#include <stdio.h> /* putchar(), getchar() */
  /* used integer types, except for main() */
#define BYTE unsigned char /* 8 bits */
#define WORD unsigned short /* >=16 bits; works either unsigned or signed */
#define GLOBAL_VAR(type,name)  static type name;
#define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
#define LOCAL_VARS(x) x
#define CONST const
#define ENDDOT
   /* ^^^ Imp: do we need this? does K&R C have `void'? as return type? */
#define RETURN_VOID(fname) return 0;
#define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
#define GLOBAL_ARRAY(type,name,length) static type name[length];
#define HAVE_ARRAY_INIT 1
#define ADD(r,a) ((r)+=(a))
#define SUB(r,a) ((r)-=(a))
#define SLIDE(r) (r)++; (r)&=32767
#define TAND_1(a) (0!=((a)&1))
#define TAND_P(a,p) (0!=((a)&(p)))
#define TNAND_P(a,p) (0==((a)&(p)))
#define AND_255(a) ((a)&255)
#define AND_7(a) ((a)&7)
#define AND_LOW(a,b) ((a)&((1<<(b))-1))
#define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
#define TWICE(a) ((a)<<1)
#define INCR(r) ((r)++)
#define DECR(r) ((r)--)
#define ENDDOT2
  static VOID fvMain(void);
  int main(int argc, char **argv);
  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    fvMain();
    return 0;
  }
  int x;
#elif CFG_LANG_ANSIC_SYSCALL /* ANSI C, with syscalls read(2) and write(2) */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static void name(void) {
  #define DEFUN_1_VOID(name,atype) static void name(atype arg) {
  #define DEFUN_0(rtype,name) static rtype name(void) {
  #define DEFUN_1(rtype,name,atype) static rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  /* used integer types, except for main() */
  #define BYTE unsigned char /* 8 bits */
  #define WORD unsigned short /* >=16 bits; works either unsigned or signed */
  #define GLOBAL_VAR(type,name)  static type name;
  #define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[length];
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  
  #include <unistd.h> /* read(2) and write(2) */
  #define BUFSIZE 4096
  static char rbuf[BUFSIZE], *rend=rbuf+BUFSIZE, *rp=rbuf+BUFSIZE;
  static char wbuf[BUFSIZE], *wend=wbuf+BUFSIZE, *wp=wbuf;
  /* (wp==wend?flush_putchar(  S [ C ]  ):*wp++=5) ; */
  #define my_putchar(c) (wp==wend?flush_putchar(c):(*wp++=c))
  #define my_getchar()  (rp==rend?more_getchar():*rp++)
  static int flush_putchar(char c) {
    write(1, wbuf, wp-wbuf); /* assume no errors */
    *wbuf=c;
    wp=wbuf+1;
    return 0; /* dummy */
  }
  static char more_getchar(void) {
    int got;
    if (rend==rbuf) return -1; /* previous read returned EOF */
    got=read(0, rbuf, BUFSIZE);
    if (got<1) { rend=rbuf; return -1; }
    rend=rbuf+got;
    rp=rbuf+1;
    return *rbuf;
  }

  static void fvMain(void);
  int main(int argc, char **argv);
  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    fvMain();
    flush_putchar(0); /* flush remaining bytes to STDOUT */
    return 0;
  }
#elif CFG_LANG_ANSIC_LINUX /* ANSI C, with Linux kernel syscalls read(2) and write(2) */
  /* please link me with `ld -e _niam_' */
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static void name(void) {
  #define DEFUN_1_VOID(name,atype) static void name(atype arg) {
  #define DEFUN_0(rtype,name) static rtype name(void) {
  #define DEFUN_1(rtype,name,atype) static rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  /* used integer types, except for main() */
  #define BYTE unsigned char /* 8 bits */
  #define WORD unsigned short /* >=16 bits; works either unsigned or signed */
  #define GLOBAL_VAR(type,name)  static type name;
  #define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[length];
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)

  /* vvv Linux kernel magic */  
  #define __KERNEL_SYSCALLS__
  #include <linux/types.h>   
  #include <linux/unistd.h>
  int errno; /* not extern! */

  #define BUFSIZE 4096
  static char rbuf[BUFSIZE], *rend=rbuf+BUFSIZE, *rp=rbuf+BUFSIZE;
  static char wbuf[BUFSIZE], *wend=wbuf+BUFSIZE, *wp=wbuf;
  /* (wp==wend?flush_putchar(  S [ C ]  ):*wp++=5) ; */
  #define my_putchar(c) (wp==wend?flush_putchar(c):(*wp++=c))
  #define my_getchar()  (rp==rend?more_getchar():*rp++)
  static int flush_putchar(char c) {
    write(1, wbuf, wp-wbuf); /* assume no errors */
    *wbuf=c;
    wp=wbuf+1;
    return 0; /* dummy */
  }
  static char more_getchar(void) {
    int got;
    if (rend==rbuf) return -1; /* previous read returned EOF */
    got=read(0, rbuf, BUFSIZE);
    if (got<1) { rend=rbuf; return -1; }
    rend=rbuf+got;
    rp=rbuf+1;
    return *rbuf;
  }

  static void fvMain(void);
  int _niam_(int argc, char **argv);
  int _niam_(int argc, char **argv) {
    (void)argc; (void)argv;
    fvMain();
    flush_putchar(0); /* flush remaining bytes to STDOUT */
    _exit(0);
    return 0;
  }
#elif CFG_LANG_ANSICAD
  /* Sun Nov 11 12:19:08 CET 2001
   */
  #undef  CFG_LANG_ANSIC
  #define CFG_LANG_ANSIC 1
  #define NEED_ASCII85DECODE 1
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static void name(void) {
  #define DEFUN_1_VOID(name,atype) static void name(atype arg) {
  #define DEFUN_0(rtype,name) static rtype name(void) {
  #define DEFUN_1(rtype,name,atype) static rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  /* used library functions */
  #define my_putchar putchar
  #define my_getchar getchar
  #include <stdio.h> /* putchar(), getchar() */
  /* used integer types, except for main() */
  #define BYTE unsigned char /* 8 bits */
  #define WORD unsigned short /* >=16 bits; works either unsigned or signed */
  #define GLOBAL_VAR(type,name)  static type name;
  #define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[length];
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  static void fvMain(void);
  int main(int argc, char **argv);
  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    fvMain();
    return 0;
  }
#else /* ANSI C, with unsigned integers */
  /* Compile: gcc -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations munzip.c
   *
   * a.out: 6660 bytes
   * Test Bash:     350ms user
   * Test Random:    70ms user
   * Test Misc:     370ms user
   */
  #undef  CFG_LANG_ANSIC
  #define CFG_LANG_ANSIC 1
  #define IGNORE
  #define NOARGS ()
  #define LT(a,b)     a<b
  #define NZ(a)       a
  #define EQ(a,b)     a==b
  #define NE(a,b)     a!=b
  #define SET(a,b)    a=b
  #define ASET(a,i,b) AREF(a,i)=b
  #define FUNCODE
  #define RETURN(fname,value) return value;
  #define ALLOW_LOCALS 1
  #define GLOBAL_REF(varname)
  #define PASS
  #define ENDFUN }
  #define IF(c) if (c) {
  #define ELSE_IF(c) } else if (c) {
  #define ELSE } else {
  #define ENDIF }
  #define WHILE(c) while (c) {
  #define ENDWHILE }
  #undef SMALLEST_CODE /* do not optimize for smallest code size */
  #define ENDDOT2
  #define AREF(aname,idx) aname[idx]
  #define GLOBAL_INIT_ARRAY(type,name) static CONST type name[]
  #define ARY {
  #define YRA }
  #define DEFUN_0_VOID(name) static void name(void) {
  #define DEFUN_1_VOID(name,atype) static void name(atype arg) {
  #define DEFUN_0(rtype,name) static rtype name(void) {
  #define DEFUN_1(rtype,name,atype) static rtype name(atype arg) {
  #define SHL(a,b) ((a)<<(b))
  #define SHR(a,b) ((a)>>(b))
  /* used library functions */
  #define my_putchar putchar
  #define my_getchar getchar
  #include <stdio.h> /* putchar(), getchar() */
  /* used integer types, except for main() */
  #define BYTE unsigned char /* 8 bits */
  #define WORD unsigned short /* >=16 bits; works either unsigned or signed */
  #define GLOBAL_VAR(type,name)  static type name;
  #define LOCAL_VAR(type,name) type name;
  /* ^^^ #define LOCAL_VAR auto -- also possible */
  #define LOCAL_VARS(x) x
  #define CONST const
  #define ENDDOT
  #define RETURN_VOID(fname)
  #define GLOBAL_ARRAY(type,name,length) static type name[length];
  #define HAVE_ARRAY_INIT 1
  #define ADD(r,a) ((r)+=(a))
  #define SUB(r,a) ((r)-=(a))
  #define SLIDE(r) (r)++; (r)&=32767
  #define TAND_1(a) (0!=((a)&1))
  #define TAND_P(a,p) (0!=((a)&(p)))
  #define TNAND_P(a,p) (0==((a)&(p)))
  #define AND_255(a) ((a)&255)
  #define AND_7(a) ((a)&7)
  #define AND_LOW(a,b) ((a)&((1<<(b))-1))
  #define SET_AND_LOW(var,x,y) SET(var,AND_LOW(x,y))
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
  static void fvMain(void);
  int main(int argc, char **argv);
  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    /* freopen("/tmp/t.in","rb",stdin); */
    fvMain();
    return 0;
  }
#endif

/* datatype declarations
 * It would be simple to define a struct for the binary Huffmann tree we
 * need for decompression as:
 * 
 *      struct node {
 *        struct node *left, *right;
 *        WORD value;
 *      };
 * 
 * But this approach would mean that we would need `pointer's. Many programming
 * languages don't have pointers, so the program would be less portable (but
 * the source would be more readable). So we use the array `N' for representing
 * nodes (one WORD for left, one WORD for right, one WORD for value). Nodes
 * are addressed with their node number (NODEN) in the array N. Every NODEN is
 * divisible by 3. `F' points to the first free node. Free nodes have their
 * left pointer pointing to the next free node. There is always a `next' free
 * node (i.e we don't have to deal with out-of-memory errors) beacuse the gzip
 * algorithm guarantees that the array `N' never exhausts.
 *
 * The following formula is true but not sharp: 1998 > 3*2*(286+2+30+2), one
 * has too examine the RFC again :-(. Empirically: 1996 and 1997 are not enough,
 * 1998 is. 1998 == 3*2*(286+2+30+2+13).
 */
#undef NULL
#define NULL 0
#undef NODEN
#define NODEN WORD
#define NODESIZE 1998 /* NODESIZE%3==0 */
/**
 * Index of the first free node in <code>N</code>.
 */
GLOBAL_VAR(NODEN,D)
#define LEFT(noden)  (AREF(N,(noden)+0)) /* unused */
#define RIGHT(noden) (AREF(N,(noden)+1)) /* unused */
#define NVAL(noden)  (AREF(N,(noden)+2)) /* unused */

#if 0
/* function declarations -- not needed at all (not even in C), because all
 * functions are static, and they are defined in non-forward-referencing
 * order.
 */
STATIC_FUNC VOID fvMain(void);
STATIC_FUNC NODEN fvNalloc(void);
STATIC_FUNC VOID fvFree(NODEN root);
STATIC_FUNC WORD fvDescend(NODEN root);
STATIC_FUNC VOID fvWrite(WORD param);
STATIC_FUNC NODEN fvMktree(WORD param);
STATIC_FUNC WORD fvRead(WORD param);
#endif

/* constant arrays (lookup tables) */
#if HAVE_ARRAY_INIT
  GLOBAL_INIT_ARRAY(WORD,constW) = ARY 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 YRA;
  GLOBAL_INIT_ARRAY(WORD,constU) = ARY 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 YRA;
  GLOBAL_INIT_ARRAY(WORD,constP) = ARY 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 YRA;
  GLOBAL_INIT_ARRAY(WORD,constQ) = ARY 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 YRA;
  GLOBAL_INIT_ARRAY(WORD,constL) = ARY 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 YRA;
#else
  GLOBAL_ARRAY(WORD, constW,19)
  GLOBAL_ARRAY(WORD, constU,29)
  GLOBAL_ARRAY(WORD, constP,29)
  GLOBAL_ARRAY(WORD, constQ,30)
  GLOBAL_ARRAY(WORD, constL,30)
#endif

GLOBAL_VAR(WORD,C) /* index (in S) of the oldest char, [0..32767] */
GLOBAL_VAR(WORD,T) /* index (in S) of the first free position, [0..32767] */

/**
 * A buffer of 16 bits for storing bits read but not processed yet. Needed
 * because we can read only bytes, not bits with getchar(). Was 32 bits,
 * but could be reduced.
 * Used by only fvRead().
 */
GLOBAL_VAR(WORD,J)
/**
 * The number of bits in the buffer <code>J</code>. Used only in fvRead();
 * Between 0..7 outside fvRead().
 */
GLOBAL_VAR(WORD,Y)

/**
 * Contains the code word lengths used for Huffman code generation in
 * fvMktree(). 320==288+32. 320 is large enough to hold both the code lengths
 * of the literal--length (288) and the distance (32) Huffman trees.
 */
GLOBAL_ARRAY(WORD,Z,320)

GLOBAL_ARRAY(WORD,B,17)
GLOBAL_ARRAY(WORD,G,17)

#if ALLOW_LOCALS
  #define no o
  #define rf f
  #define roo oo
  #define moo oo
  #define mq q
  #define mo o
  #define mf f
#else
  GLOBAL_VAR(NODEN,no)
  GLOBAL_VAR(WORD,roo)
  GLOBAL_VAR(WORD,rf)
  GLOBAL_VAR(WORD,moo)
  GLOBAL_VAR(WORD,mq)
  GLOBAL_VAR(WORD,mo)
  GLOBAL_VAR(WORD,mf)
  GLOBAL_VAR(WORD,o)
  GLOBAL_VAR(WORD,q)
  GLOBAL_VAR(WORD,ty)
  GLOBAL_VAR(WORD,oo)
  GLOBAL_VAR(WORD,ooo)
  GLOBAL_VAR(WORD,oooo)
  GLOBAL_VAR(WORD,f)
  GLOBAL_VAR(WORD,p)
  GLOBAL_VAR(WORD,v)
  GLOBAL_VAR(WORD,h)
  GLOBAL_VAR(WORD,x)
  #undef LOCAL_VAR
  #define LOCAL_VAR(a,b)
#endif

GLOBAL_ARRAY(WORD,N,NODESIZE)
#if HAVE_SPECIAL_DIVISION
  /* Special division trick to emulate the long char array, for CFG_LANG_OCPS */
#  define AREF_S(idx) (idx)/-1
#  define ASET_S(idx,val) (idx)/(val)
#else
  GLOBAL_ARRAY(BYTE,S,32768)
#  define AREF_S(idx) AREF(S,idx)
#  define ASET_S(idx,val) ASET(S,idx,val)
#endif

AFTER_GLOBALS

#if NEED_ASCII85DECODE
  /**
   * Reads a character from the PostScript LanguageLevel 2 /ASCII85Encoded-d
   * input stream STDIN.
   * <p>Sun Nov 11 11:53:35 CET 2001
   * @return the character read or -1
   */
  DEFUN_0(WORD,fvAd)
    FUNCODE 
  ENDFUN
# define my_fvad_getchar fvAd
# define my_pipe_getchar my_getchar
#else
# define my_fvad_getchar my_getchar
# define my_pipe_getchar my_getchar
#endif

/**
 * Reads <code>param</code> bits from input.
 * @param param number of bits to read
 * @return WORD, consisting of the bits
 * @whichcode 1
 */
DEFUN_1(WORD,fvRead,WORD)
  GLOBAL_REF(Y)
  GLOBAL_REF(J)
  LOCAL_VARS(
    LOCAL_VAR(WORD,roo)
    LOCAL_VAR(WORD,rf)
  )
  FUNCODE
  /* Assert: 0<=J && J<=32767 */
  IF(NZ(arg))
    /* Assert: 0<=Y && Y<=7; */
    /* Assert: 1<=arg && arg<=16; */
    /* if (Y>7 || arg>16) abort(); */
    /* J can hold at most 16 bits, may be signed or unsigned */
    IF(LT(Y+7,arg))
      /* ^^^ `if' is ok instead of `while', because of the asserts above */
      ADD(J,SHL(AND_255(my_fvad_getchar()),Y));
      ADD(Y,8);
    ENDIF
    IF(LT(Y,arg)) /* Y < arg <= Y+8 */
      SET(rf,AND_255(my_fvad_getchar()));
      IF(EQ(arg,16))
        /* Assert: 8<=Y && Y<=15 */
        SET(roo,J+SHL(AND_LOW(rf,16-Y),Y));
        SET(J,SHR(rf, 16-Y)); /* 1 <= 16-Y <= 7, it's safe to call SHR */
      ELSE
        SET_AND_LOW(roo, J+SHL(rf,Y), arg);
        SET(J,SHR(rf, arg-Y)); /* 0<=f && f<=255, no need to cast to unsigned */
        /* ^^^ 1<=arg-Y && arg-Y<=7, so it's safe to call SHR */
      ENDIF
      ADD(Y, 8-arg); /* 8-arg may be negative, but Y remains nonnegative */
    ELSE /* Y >= arg */
      /* Assert: 1<=Y && Y<=15; */
      SET(roo,AND_LOW(J, arg));
      SUB(Y,arg);
      SET(J,AND_LOW(SHR(J, arg), Y)); /* J >>= arg; */
      /* ^^^ this has been corrected to be signedness-independent */
      /* ^^^ 1<=arg && arg<=8, so it's safe to call SHR */
    ENDIF
    /* fprintf(stderr, "read %d\n", roo&0xffff); */
  ELSE
    SET(roo,0);
  ENDIF
  /* Assert: 0<=J && J<=32767 */
  RETURN(fvRead,roo)
ENDFUN

/** 
 * Allocates a new node and initializes it with empty content. This function
 * lets us get rid of the global variable F (which was buggily uninitialized
 * in the original scm.c).
 * @return pointer to the newly allocated node
 * @whichcode -1
 */
DEFUN_0(NODEN,fvNalloc)
  GLOBAL_REF(N)
  GLOBAL_REF(D)
  LOCAL_VARS(
    LOCAL_VAR(NODEN,no) /* the first/next free node */
  )
  FUNCODE
  SET(no,D);
#if 0
  if (no>NODESIZE) abort(); /* abort if out of memory */
#endif  
  SET(D,AREF(N,no)); /* not free anymore */
  ASET(N,no,NULL); /* clear; the other two fields are already cleared */
  /* N[n+1)=NULL; N[n+2)=0; */
  /* fprintf(stderr, "alloc %d\n", n); */
  RETURN(fvNalloc,no)
ENDFUN

/**
 * Frees the Huffman tree originating from <code>root</code> recursively.
 * Used <code>I</code> as input only.
 * Moved <code>I</code> into a argeter.
 * @param arg root node index in N
 */
DEFUN_1_VOID(fvFree,NODEN)
  GLOBAL_REF(N)
  GLOBAL_REF(D)
  FUNCODE
  IF(NE(arg,NULL))
    fvFree(AREF(N,arg));
    fvFree(AREF(N,arg+1));
    /* fprintf(stderr, "free %d\n", arg); */
    ASET(N,arg+1,NULL); /* clear */
    ASET(N,arg+2,0); /* clear */
    ASET(N,arg,D); /* link to the beginning of the free list */
    SET(D,arg); /* set it free */
  ENDIF
  RETURN_VOID(fvFree)
ENDFUN

/**
 * Goes down (descends) enough levels in the binary tree of (struct node)s.
 * Reads numbers from input encoded as a Huffman tree represented by the
 * (struct node)s.
 * Called only from 2 places.
 * Used <code>I</code> as both input and output, but the output is never used
 * in the program. So I've put <code>I</code> to a argument.
 * @param arg root is only for input
 * @whichcode 3
 */
DEFUN_1(WORD,fvDescend,NODEN)
  GLOBAL_REF(N)
  FUNCODE
  WHILE(NE(AREF(N,arg),NULL))
    IF(NZ(fvRead(1)))
      SET(arg,AREF(N,arg+1));
    ELSE
      SET(arg,AREF(N,arg));
    ENDIF
  ENDWHILE
  RETURN(fvDescend,AREF(N,arg+2)) /* arg->value; */
ENDFUN

/**
 * Allocates new (struct node)s. This is the Huffman tree
 * builder. It reads the global array <code>Z</code>: the code lengths have
 * been already stored there.
 * Used <code>I</code> as output only, so I moved it to the return value.
 * @param arg the number of entries (codes) of the Huffman tree to be built
 * @return the root of the Huffman tree just built
 * @whichcode 5
 */
DEFUN_1(NODEN,fvMktree,WORD)
  GLOBAL_REF(B)
  GLOBAL_REF(G)
  GLOBAL_REF(N)
  GLOBAL_REF(Z)
  LOCAL_VARS(
    LOCAL_VAR(WORD,moo)
    LOCAL_VAR(WORD,mq)
    LOCAL_VAR(WORD,mo)
    LOCAL_VAR(WORD,mf)
  )
  FUNCODE
  /* NODEN anode; */
#if SMALLEST_CODE
  SET(moo,0); WHILE(LT(moo,17)) ASET(B,moo,0); INCR(moo); ENDWHILE
#else
  ASET(B,0,0); ASET(B,1,0); ASET(B,2,0); ASET(B,3,0); ASET(B,4,0); ASET(B,5,0); ASET(B,6,0); ASET(B,7,0); ASET(B,8,0);
  ASET(B,9,0); ASET(B,10,0); ASET(B,11,0); ASET(B,12,0); ASET(B,13,0); ASET(B,14,0); ASET(B,15,0); ASET(B,16,0);
#endif
  SET(moo,0);
  WHILE(LT(moo,arg)) INCR(AREF(B,AREF(Z,moo))); INCR(moo); ENDWHILE /* ! */
  ASET(B,0,0);
  ASET(G,0,0);
#if SMALLEST_CODE
  SET(moo,0);
  WHILE(LT(moo,16))
    ASET(G, moo+1,TWICE(AREF(G,moo)+AREF(B,moo)));
    INCR(moo);
  ENDWHILE
#else
  ASET(G, 1, TWICE(AREF(G, 0)+AREF(B, 0))); ASET(G, 2, TWICE(AREF(G, 1)+AREF(B, 1)));
  ASET(G, 3, TWICE(AREF(G, 2)+AREF(B, 2))); ASET(G, 4, TWICE(AREF(G, 3)+AREF(B, 3)));
  ASET(G, 5, TWICE(AREF(G, 4)+AREF(B, 4))); ASET(G, 6, TWICE(AREF(G, 5)+AREF(B, 5)));
  ASET(G, 7, TWICE(AREF(G, 6)+AREF(B, 6))); ASET(G, 8, TWICE(AREF(G, 7)+AREF(B, 7)));
  ASET(G, 9, TWICE(AREF(G, 8)+AREF(B, 8))); ASET(G,10, TWICE(AREF(G, 9)+AREF(B, 9)));
  ASET(G,11, TWICE(AREF(G,10)+AREF(B,10))); ASET(G,12, TWICE(AREF(G,11)+AREF(B,11)));
  ASET(G,13, TWICE(AREF(G,12)+AREF(B,12))); ASET(G,14, TWICE(AREF(G,13)+AREF(B,13)));
  ASET(G,15, TWICE(AREF(G,14)+AREF(B,14))); ASET(G,16, TWICE(AREF(G,15)+AREF(B,15)));
#endif
  /* Dat: anode is the ->left pointer of the Sentinel node */
  ASET(N,3,NULL); /* anode=NULL; */
  SET(moo,0);
  WHILE(LT(moo,arg))
    IF(NZ(AREF(Z,moo)))
      /* WORD o, f; */ /* struct node **f; */
      SET(mq,AREF(G,AREF(Z,moo)));
      INCR(AREF(G,AREF(Z,moo)));
      SET(mf,3); /* mf=&anode; */
      SET(mo,AREF(Z,moo));
      WHILE(NZ(mo))
        DECR(mo);
        IF(EQ(AREF(N,mf),NULL))
          ASET(N,mf,fvNalloc NOARGS);
        ENDIF
        /* if (0!=((mq>>o)&1)) f=&(*f)->right; else f=&(*f)->left; */
        IF(TAND_P(mq,SHL(1,mo)))
          SET(mf,AREF(N,mf)+1);
        ELSE
          SET(mf,AREF(N,mf)+0);
        ENDIF
      ENDWHILE
      ASET(N,mf,fvNalloc NOARGS);
      ASET(N,AREF(N,mf)+2,moo); /* (*f)->value=moo; */
    ENDIF
    INCR(moo);
  ENDWHILE
  RETURN(fvMktree,AREF(N,3))
ENDFUN

/**
 * Writes out a character if available.
 * @whichcode 2
 */
DEFUN_1_VOID(fvWrite,WORD)
  GLOBAL_REF(T)
  GLOBAL_REF(S)
  GLOBAL_REF(C)
  FUNCODE
  /* fprintf(stderr, "write %d\n", arg&0xffff); */
#if 0
    /* we don't need this because T and C are already in [0..32767) */
    T&=32767;
#endif
  ASET_S(T,arg);
  SLIDE(T); /* T++; T&=32767; */
  IF(EQ(T,C))
    my_putchar(AREF_S(C));
    SLIDE(C); /* C++; C&=32767; */
  ENDIF
  RETURN_VOID(fvWrite)
ENDFUN

#define FMT_ZIP_STORED   0 /* file is stored, not Deflated in ZIP file */
#define FMT_ZLIB         120
#define FMT_GZIP         31
#define FMT_NONE         3
#define FMT_STOP         4 /* stop processing and flush STDIN */
#define FMT_ZIP_DEFLATED 8 

/**
 * @whichcode 0, 6
 */
DEFUN_0_VOID(fvMain)
  GLOBAL_REF(J)
  GLOBAL_REF(Y)
  GLOBAL_REF(C)
  GLOBAL_REF(T)
  GLOBAL_REF(N)
  GLOBAL_REF(D)
  LOCAL_VARS(
    LOCAL_VAR(WORD,o)
    LOCAL_VAR(WORD,q)
    LOCAL_VAR(WORD,ty) /* type of input file */
    /* vvv sub-locals */
    LOCAL_VAR(WORD,oo)
    LOCAL_VAR(WORD,ooo)
    LOCAL_VAR(WORD,oooo)
    LOCAL_VAR(WORD,f)
    LOCAL_VAR(WORD,p)
    LOCAL_VAR(WORD,x)
    /**
     * Root of the literal--length Huffman tree in <code>N</code>
     * (see RFC 1951). Its length is at most 288 with values 0..287. Values
     * 286 and 287 are invalid. A value in 0..255 means a literal byte, a
     * value of 256 indicates end-of-block, and a
     * value in 257..285 is a prefix (with 29 possible values) of a length
     * in 3..258. See subsubsection 3.2.5 of RFC 1951.
     */
    LOCAL_VAR(NODEN,v)
    /**
     * Root of the distance Huffman tree in <code>N</code> (see RFC 1951).
     * Its length is at most 32 with values 0..31. Values 30 and 31 are invalid.
     * A value of 0..29 is a prefix (with 30 possible values) of a distance in
     * 1..32768. See subsubsection 3.2.5 of RFC 1951.
     */
    LOCAL_VAR(NODEN,h)
  )
  
  FUNCODE
  SET(ty,FMT_NONE);
#ifdef CFG_MULTIPLE
  WHILE(NE(ty,FMT_STOP)) /* outermost while loop */
#endif
  SET(oo,0); SET(ooo,0); /* avoid GCC warnings */
  SET(J,0); SET(Y,0); SET(C,0); SET(T,0);
  SET(v,NULL); SET(h,NULL);
  ASET(N,0,NULL); ASET(N,1,NULL); ASET(N,2,0); /* the NULL node is initially empty */
  ASET(N,3,NULL); ASET(N,4,NULL); ASET(N,5,0); /* the Sentinel node is initially empty */
  SET(D,6); /* first free node is 6. `0' is NULL, `3' is Sentinel */
  SET(o,D);
  WHILE (LT(o,NODESIZE))
    ASET(N,o,o+3);  INCR(o); /* next free node is next node */
    ASET(N,o,NULL); INCR(o); /* empty RIGHT */
    ASET(N,o,0);    INCR(o); /* empty NVAL */
  ENDWHILE
#ifdef CFG_INPUT_RAW
#else
    SET(ty,my_fvad_getchar()); /* read first byte of the header */
    /* fprintf(stderr,"ty=%d %d!\n",ty, TAND_P(512+ty,256)); */
#if GETCHAR_NEGATIVE
    IF(LT(ty,0))
#else
    IF(TAND_P(512+ty,256)) /* EOF */
#endif
      /* RETURN_FORCE_VOID; */
      SET(ty,FMT_STOP);
    ELSE_IF(EQ(ty,FMT_ZLIB)) /* ZLIB format */
      IGNORE fvRead(8); /* skip second header byte: 0x01 or 0x5e or 0x9c or 0xda */
      /* SET(ty,FMT_ZLIB); */
    ELSE_IF(EQ(ty,80)) /* ZIP format */
      /* False note: that fvRead(16) reads in the wrong byte order, so we must
       * ignore it. Constants are presented in the right byte order (LSB
       * first).
       *   Truth: fvRead(16) reads in the correct byte order (LSB first),
       * which is correct for both ZIP and DEFLATE. So SHL(fvRead(),8) is
       * absolutely unnecessary.
       */
      IGNORE fvRead(8); /* skip second header byte: 0x48 */
      SET(o,fvRead(8));
      SET(ty,FMT_NONE);
      IF(EQ(o,3)) /* Local file header */
        IGNORE fvRead(8); /* skip: 0x04 */
        IGNORE fvRead(16); /* skip: version needed to extract file (0x0020) */
        IGNORE fvRead(16); /* LOCFLG flags */
        SET(ty,fvRead(8)); /* lower half of compression method */
        IGNORE fvRead(8); /* upper half of compression method */
        IGNORE fvRead(16); IGNORE fvRead(16); /* file modification time in MS-DOS format */
        IGNORE fvRead(16); IGNORE fvRead(16); /* some kind of CRC-32 */
        SET(oo, fvRead(8)); ADD(oo,  SHL(fvRead(8),8)); /* lower compressed file size */
        SET(ooo,fvRead(8)); ADD(ooo, SHL(fvRead(8),8)); /* higher compressed file size */
        IGNORE fvRead(16); IGNORE fvRead(16); /* uncompressed file size */
        SET(f,fvRead(8)); ADD(f, SHL(fvRead(8),8)); /* length of filename */      
        SET(q,fvRead(8)); ADD(q, SHL(fvRead(8),8)); /* length of extra field */
        WHILE(NZ(f)) IGNORE fvRead(8); DECR(f); ENDWHILE /* file name */
        WHILE(NZ(q)) IGNORE fvRead(8); DECR(q); ENDWHILE /* extra field */
        /* now: ty is either of FMT_ZIP_STORED or FMT_ZIP_DEFLATED */
      ELSE_IF(EQ(o,7)) /* Extended local header of previous file in ZIP */
        SET(o,0); WHILE(LT(o,13)) IGNORE fvRead(8); INCR(o); ENDWHILE /* BUGFIX: was 15 */
        /* SET(o,fvRead(8)); */
      ELSE_IF(EQ(o,5)) /* End of Central Directory Structure in ZIP */
        /* fprintf(stderr,"EOCDS\n"); */
        SET(o,0); WHILE(LT(o,17)) IGNORE fvRead(8); INCR(o); ENDWHILE
        SET(o,fvRead(8)); ADD(o, SHL(fvRead(8),8)); /* CML: archive comment length */
        WHILE (NZ(o)) IGNORE fvRead(8); DECR(o); ENDWHILE
      ELSE_IF(EQ(o,1)) /* Central Directory Structure */
        /* fprintf(stderr,"CDS\n"); */
        SET(oo,0); WHILE(LT(oo,25)) IGNORE fvRead(8); INCR(oo); ENDWHILE
        SET(f,fvRead(8)); ADD(f, SHL(fvRead(8),8)); /* LEN: length of file name */
        SET(o,fvRead(8)); ADD(o, SHL(fvRead(8),8)); /* XLN: length of extra field */
        SET(q,fvRead(8)); ADD(q, SHL(fvRead(8),8)); /* CML: length of file comment */
        SET(oo,0); WHILE(LT(oo,12)) IGNORE fvRead(8); INCR(oo); ENDWHILE
        WHILE(NZ(f)) IGNORE fvRead(8); DECR(f); ENDWHILE /* file name */
        WHILE(NZ(o)) IGNORE fvRead(8); DECR(o); ENDWHILE /* extra field */
        WHILE(NZ(q)) IGNORE fvRead(8); DECR(q); ENDWHILE /* file comment */
      ENDIF /* IF ZIP structure sub-type */
    ELSE_IF(EQ(ty,31)) /* gzip/RFC 1952 format */
      /* fprintf(stderr,"gzip!\n"); */
      /* SET(ty,FMT_GZIP); */
      /* The most simple gzip header (10 bytes):
       * ID1   hex 0x1f == dec 31 == FMT_GZIP
       * ID2   hex 0x8b
       * CM    hex 0x08
       * FLG   hex 0x00
       * MTIME hex 0x00, 0x00, 0x00, 0x00 (1 Jan 1970, UNIX epoch)
       * XFL   hex 0x00
       * OS    hex 0xff
       * After that comes the compressed data stream.
       * After that comes the CRC32 and ISIZE (byte aligned? ?)
       */
      IGNORE fvRead(16); /* ignore ID2 and CM */
      SET(o,fvRead(8)); /* FLG */
#if 0
        fvRead(8); fvRead(16); /* fvRead(24); */
        fvRead(8); fvRead(16); /* fvRead(24); */
#else
        IGNORE fvRead(16); IGNORE fvRead(16); /* ignore MTIME */
        IGNORE fvRead(16); /* ignore XFL, OS */
#endif
#ifndef CFG_INPUT_RFC /* CFG_INPUT_FILE */
      IF(TAND_P(o,2)) /* GCONT: skip part number of continuation */
        IGNORE fvRead(16);
      ENDIF
#endif      
      IF(TAND_P(o,4)) /* ignore FEXTRA */
        SET(q,fvRead(16));
        WHILE(NZ(q)) IGNORE fvRead(8); DECR(q); ENDWHILE
      ENDIF
      IF(TAND_P(o,8))
        WHILE(NZ(fvRead(8)))
          PASS;
        ENDWHILE
      ENDIF /* ignore FNAME */
      IF(TAND_P(o,16))
        WHILE(NZ(fvRead(8)))
          PASS;
        ENDWHILE
      ENDIF /* ignore FCOMMENT */
#ifdef CFG_INPUT_RFC 
      IF(TAND_P(o,2)) IGNORE fvRead(16); ENDIF /* ignore FHCRC */
#else /* CFG_INPUT_FILE */
      IF(TAND_P(o,32)) /* skip encryption header */
        SET(f,0); WHILE(LT(f,12)) IGNORE fvRead(8); INCR(f); ENDWHILE
      ENDIF
#endif
    ENDIF /* IF file format */
    /* fprintf(stderr,"ty=%d\n", ty); */
    IF(EQ(ty,FMT_ZIP_STORED))
      /* fprintf(stderr,"oo=%d ooo=%d\n", oo, ooo); */
      WHILE(NZ(oo)) my_putchar(my_fvad_getchar()); DECR(oo); ENDWHILE
      WHILE(NZ(ooo))
#if 0 /* In PostScript, this would mean an infinite loop.. */
        my_putchar(my_fvad_getchar());
        SET(oo,16384*4-1); /* ! op*, op/ */ /* avoid conflict with large numerical constants */
        WHILE(NZ(oo)) my_putchar(my_fvad_getchar()); DECR(oo); ENDWHILE
#else
        my_putchar(my_fvad_getchar());
        my_putchar(my_fvad_getchar());
        SET(oo,32767);
        WHILE(NZ(oo))
          my_putchar(my_fvad_getchar());
          my_putchar(my_fvad_getchar());
          DECR(oo);
        ENDWHILE
#endif        
        DECR(ooo);
      ENDWHILE
    ELSE_IF(EQ(ty,FMT_STOP))
      PASS
    ELSE_IF(NE(ty,FMT_NONE))
#endif
  SET(o,0);
  WHILE(EQ(o,0))
    SET(o,fvRead(1)); /* BFINAL: 1 iff this is the last data block */
    SET(q,fvRead(2)); /* BTYPE: block type; 0=stored, 1=fixed Huff, 2=dyn Huff */
    /* fprintf(stderr, "MODE_BOB o=%d q=%d %ld\n", o, q, ftell(stdin)); */
    IF(NZ(q))
     /**
      * Now: q==BTYPE, q==1 or q==2. (BTYPE==3 means error). Handles
      * compressed data blocks, see subsubsection 3.2.5 of RFC 1951.
      */
#if 0 /* obsolete */
      IF(TAND_1(q)) /* inline: fv(..., 8); */
#else
      IF(EQ(q,1))
#endif      
      /**
       * Initializes the fixed Huffman codes for BTYPE==1.
       */
        /* WORD oo; */
        SET(oo,288);
        WHILE(NZ(oo))
          DECR(oo);
          /* AREF(Z,oo) = oo < 144 ? 8
                : oo < 256 ? 9
                : oo < 280 ? 7
                : 8; */
#if 0
            if (144<=oo && oo<256) AREF(Z,oo)=9;
            else if (256<=oo && oo<280) AREF(Z,oo)=7;
            else AREF(Z,oo)=8;
#else /* this doesn't require &&, <= etc */
            IF(LT(oo,144))
              ASET(Z,oo,8);
            ELSE_IF(LT(oo,256))
              ASET(Z,oo,9);
            ELSE_IF(LT(oo,280))
              ASET(Z,oo,7);
            ELSE
              ASET(Z,oo,8);
            ENDIF
#endif
        ENDWHILE
        SET(v,fvMktree(288));
#if SMALLEST_CODE
        SET(f,0); WHILE(LT(f,32)) ASET(Z,f,5); INCR(f); ENDWHILE
#else
        ASET(Z, 0,5); ASET(Z, 1,5); ASET(Z, 2,5); ASET(Z, 3,5); ASET(Z, 4,5); ASET(Z, 5,5); ASET(Z, 6,5); ASET(Z, 7,5);
        ASET(Z, 8,5); ASET(Z, 9,5); ASET(Z,10,5); ASET(Z,11,5); ASET(Z,12,5); ASET(Z,13,5); ASET(Z,14,5); ASET(Z,15,5);
        ASET(Z,16,5); ASET(Z,17,5); ASET(Z,18,5); ASET(Z,19,5); ASET(Z,20,5); ASET(Z,21,5); ASET(Z,22,5); ASET(Z,23,5);
        ASET(Z,24,5); ASET(Z,25,5); ASET(Z,26,5); ASET(Z,27,5); ASET(Z,28,5); ASET(Z,29,5); ASET(Z,30,5); ASET(Z,31,5);
#endif
        SET(h,fvMktree(32));
#if 0 /* obsolete */
      ENDIF IF(TAND_P(q,2)) /* inline: fv(..., 9); */
#else
      ELSE
#endif            
      /**
       * Reads dynamic Huffman codes for BTYPE==2.
       */
        /* WORD oo, ooo, oooo, f, p, x, v; */
        SET(p, fvRead(5) + 257); /* HLIT: 257..286 (287--289 are unused) */
        SET(x, fvRead(5) + 1); /* HDIST: 1..32 */
        SET(v, fvRead(4) + 4); /* HCLEN: 4..19 */ /* small v */
        SET(oo, 0);
        WHILE(LT(oo,v))  ASET(Z,AREF(constW,oo), fvRead(3)); INCR(oo); ENDWHILE /* small v */
        WHILE(LT(oo,19)) ASET(Z,AREF(constW,oo), 0); INCR(oo); ENDWHILE
        SET(v,fvMktree(19));
        SET(ooo,0);
        SET(oo,0);
        WHILE(LT(oo,p + x))
          SET(oooo,fvDescend(v));
          /* f = o == 16 ? ((o = ooo), 3 + fvRead(2))
               : o == 17 ? ((o = 0), 3 + fvRead(3))
               : o == 18 ? ((o = 0), 11 + fvRead(7))
               : ((ooo = o), 1); */
          IF(EQ(oooo,16))
            SET(oooo,ooo); SET(f,3+fvRead(2));
          ELSE_IF(EQ(oooo,17))
            SET(oooo,0); SET(f,3+fvRead(3));
          ELSE_IF(EQ(oooo,18))
            SET(oooo,0); SET(f,11+fvRead(7));
          ELSE
            SET(ooo,oooo); SET(f,1);
          ENDIF
          SET(q,f);
          WHILE(NZ(q)) ASET(Z,oo,oooo); INCR(oo); DECR(q); ENDWHILE
        ENDWHILE
        fvFree(v);
        SET(v,fvMktree(p));
        SET(oo,x);
        WHILE(NZ(oo)) DECR(oo); ASET(Z,oo,AREF(Z,oo + p)); ENDWHILE
        SET(h,fvMktree(x));
      ENDIF
    /**
     * Uncompresses the data block with the Huffman codes set up.
     */
      /* inline: fv(..., 6); */
      /* WORD oo, f; */
      SET(oo,fvDescend(v));
      WHILE(NE(oo, 256))
        IF(LT(oo,257))
          /* ^^^ BUG corrected */
          fvWrite(oo);
        ELSE
          SUB(oo,257);
          SET(f,AREF(constU,oo) + fvRead(AREF(constP,oo))); /* fvRead! 0..5 bits */
          SET(oo,fvDescend(h));
          SET(oo,AREF(constQ,oo) + fvRead(AREF(constL,oo))); /* fvRead! 0..13 bits */
          /* oo = oo <= T ? T - oo : 32768 - oo + T; */
          IF(LT(T,oo))
            SET(oo,32768-oo+T);
          ELSE
            SET(oo,T-oo);
          ENDIF
          WHILE(NZ(f))
            fvWrite(AREF_S(oo));
            SLIDE(oo); /* oo++; oo&=32767; */ /* oo %= 32768; */
            DECR(f);
          ENDWHILE
        ENDIF
        SET(oo,fvDescend(v));
      ENDWHILE
      fvFree(v);
      fvFree(h);
    ELSE /* inline: fv(..., 7); */
    /**
     * Copies a block of input to output (mostly) verbatim. This is for
     * BTYPE==0, non-compressed block, subsubsection 3.2.4 of RFC 1951.
     * (We need non-compressed because
     * some blocks cannot be compressed effectively, so gzip inserts them
     * as is.)
     * @whichcode 7
     */
      /* WORD oo; */
      IGNORE fvRead(AND_7(Y));
      /* ^^^ skip till we reach byte boundary. fvRead! 0..7 bits */
      SET(oo,fvRead(16)); /* length of block */
      /* fprintf(stderr, "BLK %d\n", oo&0xffff); */
      IGNORE fvRead(16); /* one's complement of length; ignored */
      WHILE(NZ(oo)) fvWrite(fvRead(8)); DECR(oo); ENDWHILE
    ENDIF
  ENDWHILE
#if 0
    /* we don't need this because T and C are already in [0..32767) */
    T&=32767; C&=32767; /* T%=32768; C%=32768; */
#endif
  /* vvv Flush unwritten buffer. */
  WHILE(NE(C,T))
    my_putchar(AREF_S(C));
    SLIDE(C); /* C++; C&=32767; */ /* C%=32768; */
  ENDWHILE
#ifdef CFG_INPUT_RAW
#else
  ENDIF /* IF */
#endif
#ifdef CFG_MULTIPLE
#ifdef CFG_INPUT_RAW
#else
  IGNORE fvRead(AND_7(Y));
  /* ^^^ BUG fixed at Tue Dec 25 12:25:15 CET 2001
   * skip till we reach byte boundary. fvRead! 0..7 bits
   */
  IF(EQ(ty,FMT_GZIP))
    /* CRC32 and ISIZE remaining */
    IGNORE fvRead(16); IGNORE fvRead(16); IGNORE fvRead(16); IGNORE fvRead(16);
  ELSE_IF(EQ(ty,FMT_ZLIB))
    /* ADLER32 remaining */
    IGNORE fvRead(16); IGNORE fvRead(16);
  ENDIF
#endif
  ENDWHILE /* outermost WHILE loop */
#else
  /* This is executed only if CFG_MULTIPLE is false */
#if GETCHAR_NEGATIVE
  WHILE(LT(-1,my_pipe_getchar())) ENDWHILE
#else
  /* vvv this works for both signed and unsigned, 16 bit and 32 bit */
  WHILE(TNAND_P(512+my_pipe_getchar(),256)) ENDWHILE /* read whole input to avoid `Broken pipe' messages */
  /* ^^^ corrected bug: [0..255] */
#endif
#endif
  RETURN_VOID(fvMain)
ENDFUN

ENDDOT
ENDDOT2
