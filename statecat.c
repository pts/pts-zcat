/* statecat.c -- a stateful minimalistic zcat (gzip -cd) implementation
 * originally rcm.c
 * original author: Ron McFarland <rcm@one.net>, 1996
 * formally muzcat.c (at Tue Dec 25 11:07:47 CET 2001)
 * rewritten, restructured and extended by pts@fazekas.hu at
 * Tue Dec 25 11:08:11 CET 2001
 * (l228.tar.gz  4.21s user gunzip)
 * (l228.tar.gz 33.40s user muzcat)
 *  l228.tar.gz 28.50s user Tue Dec 25 15:47:09 CET 2001
 *  l228.tar.gz 26.02s user Tue Dec 25 19:47:28 CET 2001
 *  l228.tar.gz 21.62s user Tue Dec 25 20:13:27 CET 2001
 *  l228.tar.gz 12.94s user Tue Dec 25 20:13:27 CET 2001 change: putchar() -> fwrite()/write()
 *
 * Dat: my_getchar() returns 0..255 or -1 (or a very large integer) on EOF
 * Imp: fxRead8(); fxRead8(); instead of fxRead16(); ??
 * Imp: deeper testing of stored etc.
 * Imp: test case for no<NODESIZE
 * Imp: eliminate CR(AREF ??
 */

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
#undef HAVE_SPECIAL_DIVISION

#if CFG_LANG_ANSIC_SIGNED
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
  #include <unistd.h> /* write() */
  #include <assert.h>
  #define BYTE signed char
  #define WORD signed short
  #define GLOBAL_VAR(type,name) static type name;
  #define LOCAL_VAR(type,name) type name;
  /* #define LOCAL_VAR auto -- also possible */
  #define CONST const
  #define ENDDOT
  #define VOID void
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
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
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
  #include <unistd.h> /* write() */
  #include <assert.h>
  /* used integer types, except for main() */
  #define BYTE unsigned char /* 8 bits */
  #define WORD unsigned short /* >=16 bits; works either unsigned or signed */
  #define GLOBAL_VAR(type,name)  static type name;
  #define LOCAL_VAR(type,name) type name;
  /* #define LOCAL_VAR auto -- also possible */
  #define CONST const
  #define ENDDOT
  #define VOID void
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
  #define TWICE(a) ((a)<<1)
  #define INCR(r) ((r)++)
  #define DECR(r) ((r)--)
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


/**
 * Contains the code word lengths used for Huffman code generation in
 * fvMktree(). 320==288+32. 320 is large enough to hold both the code lengths
 * of the literal--length (288) and the distance (32) Huffman trees -- and
 * large enough to hold the auxilary Huffman tree (used for building the
 * real tree) of length 19.
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
  #define AREF_S(idx) (idx)/-1
  #define ASET_S(idx,val) (idx)/(val)
#else
  GLOBAL_ARRAY(BYTE,S,32768)
  #define AREF_S(idx) AREF(S,idx)
  #define ASET_S(idx,val) ASET(S,idx,val)
#endif

#define SLOCAL_VAR(t,n) static t n;

AFTER_GLOBALS

#if CFG_OLD_SLOW_FVREAD

/**
 * A buffer of 16 bits for storing bits read but not processed yet. Needed
 * because we can read only bytes, not bits with getchar(). Was 32 bits,
 * but could be reduced.
 * Used by only fvRead().
 */
GLOBAL_VAR(WORD,J)
/**
 * The number of bits in the buffer <code>J</code>. Used only in fvRead();
 */
GLOBAL_VAR(WORD,Y)

static void fxInit(void) {
  J=0; Y=0;
}

/** @return 0..255 -- or 511 on EOF */
DEFUN_0(WORD,fxRead8EOF)
  unsigned short i=my_getchar();
  assert(Y==0);
  return i&511;
ENDFUN
/** @return always 0..255 */
DEFUN_0(WORD,fxRead8)
  assert(Y==0);
  return (unsigned char)my_getchar();
ENDFUN
DEFUN_0(WORD,fxRead16)
  WORD w=(unsigned char)my_getchar();
  assert(Y==0);
  ADD(w, SHL((unsigned char)my_getchar(),8));
  return w;
ENDFUN

/**
 * Reads <code>param</code> bits from input.
 * @param param number of bits to read
 * @return WORD, consisting of the bits
 * @whichcode 1
 */
DEFUN_1(WORD,fvRead,WORD)
  GLOBAL_REF(Y)
  GLOBAL_REF(J)
  LOCAL_VAR(WORD,roo)
  LOCAL_VAR(WORD,rf)
  FUNCODE
  IF(NZ(arg))
    assert(0<=Y*2 && Y<=7);
    /* ^^^ Dat: Y*2: avoid GCC warning */
    assert(1<=arg && arg<=13);
    /* if (Y>7 || arg>16) abort(); */
    /* J can hold at most 16 bits, may be signed or unsigned */
    IF(LT(Y+7,arg))
      /* ^^^ `if' is ok instead of `while', because of the asserts above */
      ADD(J,SHL(AND_255(my_getchar()),Y));
      ADD(Y,8);
    ENDIF
    assert(arg <= Y+8);
    IF(LT(Y,arg)) /* Y < arg <= Y+8 */
      SET(rf,AND_255(my_getchar()));
      IF(EQ(arg,16))
        assert(8<=Y && Y<=15);
        SET(roo,J+SHL(AND_LOW(rf,16-Y),Y));
        SET(J,SHR(rf, 16-Y)); /* 1 <= 16-Y <= 7, it's safe to call SHR */
      ELSE
        SET(roo,AND_LOW(J+SHL(rf,Y), arg));
        SET(J,SHR(rf, arg-Y)); /* 0<=f && f<=255, no need to cast to unsigned */
        /* ^^^ 1<=arg-Y && arg-Y<=7, so it's safe to call SHR */
      ENDIF
      ADD(Y, 8-arg); /* 8-arg may be negative, but Y remains nonnegative */
    ELSE /* Y >= arg */
      assert(1<=Y && Y<=15);
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
  assert(0<=Y*2 && Y<=7);
  /* ^^^ Dat: Y*2: avoid GCC warning */
  #if NDEBUG
  #else
    fprintf(stderr, "fvRead(%d)=%d\n", arg, roo);
  #endif
  RETURN(fvRead,roo)
ENDFUN

static void fxSkip(void) {
  IGNORE fvRead(AND_7(Y));
}

#define fxRead1 fvRead(1)

#else

/* Operations:

-- fxRead8(): read a 8-bit byte at byte boundary
-- fxRead16z(): read a 16-bit word at byte boundary, MSB first (ZIP)
-- fxRead16(): read a 16-bit word at byte boundary, LSB first (normal)
-- fxSkip(): skip to next byte boundary
-- fxRead1(): read 1 bit (for Huffman)
-- fvRead(): read any amount of bits (0..13), LSB first
*/

/** char, 0..255 */
SLOCAL_VAR(WORD,preread)
/** 0..7: bits of bufread[pos] already read */
SLOCAL_VAR(WORD,past)

static void fxInit(void) {
  preread=my_getchar();
  past=0;
}

/** reads 0..13 bits */
static WORD fvRead(WORD arg) {
  WORD ret;
  WORD opast=past;
  #ifdef NDEBUG
  #else
    fprintf(stderr,"==> fvRead(%d)\n    past=%d preread=%d\n", arg, past, preread);
  #endif
  if (arg!=0) {
    assert((preread&~255)==0); /* Not EOF */
    ret=SHR(preread,opast);
    past+=arg;
    if (past==8) {
      preread=my_getchar(); 
      past=0;
    } else if (past>=9) {
      preread=my_getchar(); assert((preread&~255)==0); /* Not EOF */
      ret+=SHL(preread,8-opast);
      if (past<16) {
        past-=8;
      } else {
        preread=my_getchar();
        if (past>16) {
          assert(past<=7+13);
          assert((preread&~255)==0); /* Not EOF */
          ret+=SHL(preread,16-opast);
        }
        past-=16;
      }
    }
    ret&=((1<<arg)-1);
  } else ret=0;
  #if NDEBUG
  #else
    /* fprintf(stderr, "fvRead(%d)=%d\n", arg, ret); */
    fprintf(stderr, "<-- fvRead=%d\n", ret);
  #endif
  return ret;
}

static WORD fxRead1(void) {
  WORD ret;
  assert((preread&~255)==0); /* Not EOF */
  if (past==7) {
    ret=1&SHR(preread,7);
    preread=my_getchar(); 
    past=0;
  } else {
    ret=1&SHR(preread,past);
    past++;
  }
  return ret;
}

static void fxSkip(void) {
  if (past!=0) {
    assert((preread&~255)==0); /* Not EOF */
    preread=my_getchar(); 
    past=0;
  }
}

static WORD fxRead16(void) {
  WORD ret;
  assert(past==0);
  assert((preread&~255)==0); /* Not EOF */
  ret=preread;
  preread=my_getchar(); assert((preread&~255)==0); /* Not EOF */
  ret+=preread<<8;
  preread=my_getchar();
  return ret;
}

static WORD fxRead8(void) {
  WORD ret;
  assert(past==0);
  assert((preread&~255)==0); /* Not EOF */
  ret=preread;
  preread=my_getchar();
  return ret;
}

static WORD fxRead8EOF(void) {
  WORD ret;
  assert(past==0);
  ret=preread&511;
  preread=my_getchar();
  return ret;
}

#endif

#define fvReadq fvRead
/** Reads an uint16 from the ZIP file */
#define fxRead16z fxRead16



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
  LOCAL_VAR(NODEN,no) /* the first/next free node */
  FUNCODE
  SET(no,D);
  assert(no<NODESIZE); /* abort if out of memory */
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
  #ifdef NDEBUG
  #else
    fprintf(stderr, "==> fvDescend(%d)\n", arg);
  #endif
  WHILE(NE(NULL,AREF(N,arg)))
    SET(arg,AREF(N,arg+fxRead1()));
  ENDWHILE
  #ifdef NDEBUG
  #else
    fprintf(stderr, "<-- fvDescend=%d\n", AREF(N,arg+2));
  #endif
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
  LOCAL_VAR(WORD,moo)
  LOCAL_VAR(WORD,mq)
  LOCAL_VAR(WORD,mo)
  LOCAL_VAR(WORD,mf)
  FUNCODE
  /* NODEN anode; */
#if SMALLEST_CODE
  SET(moo,0); WHILE(LT(moo,17)) ASET(B,moo,0); INCR(moo); ENDWHILE
#else
  ASET(B,0,0); ASET(B,1,0); ASET(B,2,0); ASET(B,3,0); ASET(B,4,0); ASET(B,5,0); ASET(B,6,0); ASET(B,7,0); ASET(B,8,0);
  ASET(B,9,0); ASET(B,10,0); ASET(B,11,0); ASET(B,12,0); ASET(B,13,0); ASET(B,14,0); ASET(B,15,0); ASET(B,16,0);
#endif
  SET(moo,0);
  WHILE(LT(moo,arg)) INCR(AREF(B,AREF(Z,moo))); INCR(moo); ENDWHILE /* ! CR(AREF */
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
      INCR(AREF(G,AREF(Z,moo))); /* ! CR(AREF */
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

GLOBAL_VAR(WORD,T) /* index (in S) of the first free position, [0..32767] */

#define FMT_ZIP_STORED   0 /* file is stored, not Deflated in ZIP file */
#define FMT_ZLIB         120
#define FMT_GZIP         31
#define FMT_NONE         3
#define FMT_STOP         4 /* stop processing and flush STDIN */
#define FMT_ZIP_DEFLATED 8

#define MODE_BOS   0 /* at beginning of (sub)file */
#define MODE_MOB   1 /* at middle of a DEFLATE block */
#define MODE_REP   2 /* printing a repeating string in a DEFLATE block */
#define MODE_BOB   3 /* at the beginning of a block, just before BFINAL and BTYPE */
#define MODE_COPY1 4 /* copying from input to output (a stored block) */
#define MODE_ZSTO  5 /* copying from input to output (FMT_ZIP_STORED) */

#define SLOCAL_VAR(t,n) static t n;
SLOCAL_VAR(WORD,mode) /* MODE_* */
SLOCAL_VAR(WORD,o)
SLOCAL_VAR(WORD,q)
SLOCAL_VAR(WORD,ty) /* type of input file: FMT_* */
/* vvv sub-locals */
SLOCAL_VAR(WORD,oo)
SLOCAL_VAR(WORD,ooo)
SLOCAL_VAR(WORD,oooo)
SLOCAL_VAR(WORD,f)
SLOCAL_VAR(WORD,p)
SLOCAL_VAR(WORD,x)
/**
 * Root of the literal--length Huffman tree in <code>N</code>
 * (see RFC 1951). Its length is at most 288 with values 0..287. Values
 * 286 and 287 are invalid. A value in 0..255 means a literal byte, a
 * value of 256 indicates end-of-block, and a
 * value in 257..285 is a prefix (with 29 possible values) of a length
 * in 3..258. See subsubsection 3.2.5 of RFC 1951.
 */
SLOCAL_VAR(NODEN,v)
/**
 * Root of the distance Huffman tree in <code>N</code> (see RFC 1951).
 * Its length is at most 32 with values 0..31. Values 30 and 31 are invalid.
 * A value of 0..29 is a prefix (with 30 possible values) of a distance in
 * 1..32768. See subsubsection 3.2.5 of RFC 1951.
 */
SLOCAL_VAR(NODEN,h)

/** @return length to be written from S */
static WORD fvWork(void) {
  GLOBAL_REF(J)
  GLOBAL_REF(Y)
  GLOBAL_REF(T)
  GLOBAL_REF(N)
  GLOBAL_REF(D)
  FUNCODE

  IF(EQ(mode,MODE_REP))
    assert(T==0);
    assert(f<=258);
    /* f==0 is OK now */
    WHILE(NZ(f))
      ASET_S(T,AREF_S(oo));
      SLIDE(oo);
      INCR(T);
      DECR(f);
    ENDWHILE
    SET(mode,MODE_MOB);
  ELSE_IF(EQ(mode,MODE_COPY1))
    /* Now: f: copy count unsinged word16 */
    assert(T==0);
    /* f==0 is OK now */
    IF((unsigned short)f>=32768)
      WHILE(NE((unsigned short)T,32768))
        ASET_S(T,fxRead8());
        INCR(T);
        DECR(f);
      ENDWHILE
      SET(T,0);
      assert(mode==MODE_COPY1);
      return (WORD)32768;
    ELSE
      WHILE(NZ(f))
        ASET_S(T,fxRead8());
        INCR(T);
        DECR(f);
      ENDWHILE
      IF(NZ(o)) /* BUG fixed at Thu Dec 27 14:15:36 CET 2001 */
        goto on_bos;
      ELSE
        SET(mode,MODE_BOB);
      ENDIF
    ENDIF
  ELSE_IF(EQ(mode,MODE_ZSTO))
    /* Now: f: copy count low unsinged word16, ooo: high unsigned word16 */
    /* Now: oo: 0..1 flag for decrementing ooo */
    assert(T==0);
    /* f==0 is OK now */
    IF((unsigned short)f>=32768)
      /* WHILE(NE(f,oo))  Ruining oo here is a bad idea since we'll need it later
        ASET_S(T,fxRead8());
        INCR(T);
        DECR(f);
      ENDWHILE */

      WHILE(NE((unsigned short)T,32768))
        ASET_S(T,fxRead8());
        INCR(T);
        DECR(f);
      ENDWHILE

      SET(T,0);
      assert(mode==MODE_ZSTO);
      return (WORD)32768;
    ELSE_IF(NZ(ooo)) 
      WHILE(NE((unsigned short)T,32768))
        ASET_S(T,fxRead8());
        INCR(T);
      ENDWHILE
      IF(NZ(oo)) /* BUGFIX by pts@fazekas.hu at Wed Dec 26 22:56:19 CET 2001 */
        DECR(ooo);
        SET(oo,0);
      ELSE
        SET(oo,1);
      ENDIF
      SET(T,0);
      assert(mode==MODE_ZSTO);
      return (WORD)32768;
    ELSE
      WHILE(NZ(f))
        ASET_S(T,fxRead8());
        INCR(T);
        DECR(f);
      ENDWHILE
      SET(mode,MODE_BOS);
    ENDIF
  ENDIF
  IF(EQ(mode,MODE_MOB))
   mode_mob:
    /**
     * Uncompresses the data block with the Huffman codes set up.
     *
     * Reads at most 33 bits per entry. Unfortunately the block can be of
     * arbitrary length (terminated by an entry of 256).
     */
    SET(oo,fvDescend(v));
    /* ^^^ reads 0..15 bits, see subsubsection 3.2.7 of RFC 1951 */
    WHILE(NE(oo, 256))
      IF(LT(oo,257))
        /* ^^^ BUG corrected */
        ASET_S(T,oo);
        SLIDE(T); /* T++; T&=32767; */
        if (T==0) return (WORD)32768; /* remain in MODE_MOB */
      ELSE
        SUB(oo,257);
        SET(f,AREF(constU,oo) + fvRead(AREF(constP,oo))); /* fvRead! 0..5 bits */
        assert(3<=f && f<=258);
        SET(oo,fvDescend(h));
        SET(oo,AREF(constQ,oo) + fvRead(AREF(constL,oo))); /* fvRead! 0..13 bits */
        /* oo = oo <= T ? T - oo : 32768 - oo + T; */
        IF(LT(T,oo))
          SET(oo,32768-oo+T);
        ELSE
          SET(oo,T-oo);
        ENDIF
        IF((unsigned short)f<32768-(unsigned short)T)
          WHILE(NZ(f))
            ASET_S(T,AREF_S(oo));
            SLIDE(oo);
            INCR(T);
            DECR(f);
          ENDWHILE
          assert((unsigned short)T<32768);
        ELSE
          SUB(f,32768-T);
          WHILE((unsigned short)T!=32768)
            ASET_S(T,AREF_S(oo));
            SLIDE(oo);
            INCR(T);
          ENDWHILE
          SET(T,0);
          mode=MODE_REP;
          return (WORD)32768;
        ENDIF
      ENDIF
      SET(oo,fvDescend(v));
      /* ^^^ reads 0..15 bits, see subsubsection 3.2.7 of RFC 1951 */
    ENDWHILE
    fvFree(v);
    fvFree(h);
    IF(NZ(o))
      goto on_bos;
    ELSE
      SET(mode,MODE_BOB);
    ENDIF
  ENDIF
  WHILE(EQ(mode,MODE_BOB))
   mode_bob:
    assert(mode==MODE_BOB);
    SET(o,fxRead1()); /* BFINAL: 1 iff this is the last data block */
    SET(q,fvRead(2)); /* BTYPE: block type; 0=stored, 1=fixed Huff, 2=dyn Huff */
   #if NDEBUG
   #else
    fprintf(stderr, "MODE:BOB o=%d q=%d %ld\n", o, q, ftell(stdin));
   #endif
    assert(q==0 || q==1 || q==2);
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
         *
         * Maximum read: 5+5+4+3*19+7*289 == 2094 bits
         */
        assert(q==2);
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
          /* SET(q,f); */ /* Imp muzcat: WHILE(NZ(f)) */
          WHILE(NZ(f)) ASET(Z,oo,oooo); INCR(oo); DECR(f); ENDWHILE
        ENDWHILE
        fvFree(v);
        SET(v,fvMktree(p));
        SET(oo,x);
        /* vvv No need for array copy, just change/pass the pointer to Z... */
        WHILE(NZ(oo)) DECR(oo); ASET(Z,oo,AREF(Z,oo + p)); ENDWHILE
        SET(h,fvMktree(x));
      ENDIF
      SET(mode,MODE_MOB);
      goto mode_mob;
    ELSE /* inline: fv(..., 7); */
      /**
       * Copies a block of input to output (mostly) verbatim. This is for
       * BTYPE==0, non-compressed block, subsubsection 3.2.4 of RFC 1951.
       * (We need non-compressed because
       * some blocks cannot be compressed effectively, so gzip inserts them
       * as is.)
       * @whichcode 7
       */
      fxSkip();
      /* ^^^ skip till we reach byte boundary. fvRead! 0..7 bits */
      SET(f,fxRead16()); /* length of block: 0..65535 */
     #if NDEBUG
     #else
      fprintf(stderr, "COPY1_BLK=%d T=%d\n", f, T);
     #endif
      IGNORE fxRead16(); /* one's complement of length; ignored */
      IF((unsigned short)f<32768-(unsigned short)T)
        WHILE(NZ(f))
          ASET_S(T,fxRead8());
          INCR(T);
          DECR(f);
        ENDWHILE
        assert((unsigned short)T<32768);
        assert(mode==MODE_BOB);
      ELSE
        /* Even if f>=32768 */
        SUB(f,32768-T);
        WHILE((unsigned short)T!=32768)
          ASET_S(T,fxRead8());
          INCR(T);
        ENDWHILE
        SET(T,0);
        mode=MODE_COPY1;
        return (WORD)32768;
      ENDIF
    ENDIF
    IF(NZ(o))
     on_bos:
      SET(mode,MODE_BOS);
      fxSkip();
      /* ^^^ skip till we reach byte boundary. fvRead! 0..7 bits */
     #if NDEBUG
     #else
      fprintf(stderr,"done! %d\n", ty);
     #endif
      IF(EQ(ty,FMT_GZIP))
        /* CRC32 and ISIZE remaining */
        IGNORE fxRead16(); IGNORE fxRead16(); IGNORE fxRead16(); IGNORE fxRead16();
      ELSE_IF(EQ(ty,FMT_ZLIB))
        /* ADLER32 remaining */
        IGNORE fxRead16(); IGNORE fxRead16();
      ENDIF
    ENDIF
  ENDWHILE /* MODE_BOB */
  assert(mode==MODE_BOS);
    
  WHILE(NE(ty,FMT_STOP))
    /* SET(oo,0); SET(ooo,0); */ /* avoid GCC warnings */
    /* fxInit(); */ /* assert(Y==0); SET(J,0); SET(Y,0); */
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
    SET(ty,fxRead8EOF()); /* read first byte of the header */
    IF(EQ(ty,511)) /* EOF */
      SET(ty,FMT_STOP);
    ELSE_IF(EQ(ty,FMT_ZLIB)) /* ZLIB format */
      IGNORE fxRead8(); /* skip second header byte: 0x01 or 0x5e or 0x9c or 0xda */
      /* SET(ty,FMT_ZLIB); */
    ELSE_IF(EQ(ty,80)) /* ZIP format */
      IGNORE fxRead8(); /* skip second header byte: 0x48 */
      SET(o,fxRead8());
      SET(ty,FMT_NONE);
      IF(EQ(o,3)) /* Local file header */
        IGNORE fxRead8(); /* skip: 0x04 */
        IGNORE fxRead16(); /* skip: version needed to extract file (0x0020) */
        IGNORE fxRead16(); /* LOCFLG flags */
        SET(ty,fxRead8()); /* lower half of compression method */
        IGNORE fxRead8(); /* upper half of compression method */
        IGNORE fxRead16(); IGNORE fxRead16(); /* file modification time in MS-DOS format */
        IGNORE fxRead16(); IGNORE fxRead16(); /* some kind of CRC-32 */
        SET(f,  fxRead16z());  /* lower compressed file size */
        SET(ooo,fxRead16z()); /* higher compressed file size */
        IGNORE fxRead16(); IGNORE fxRead16(); /* uncompressed file size */
        SET(oo,fxRead16z()); /* length of filename */      
        SET(q,fxRead16z()); /* length of extra field */
        #if NDEBUG
          WHILE(NZ(oo)) IGNORE fxRead8(); DECR(oo); ENDWHILE /* file name */
        #else
          WHILE(NZ(oo)) fputc(fxRead8(),stderr); DECR(oo); ENDWHILE /* file name */
          fprintf(stderr,".\n");
        #endif
        WHILE(NZ(q)) IGNORE fxRead8(); DECR(q); ENDWHILE /* extra field */
        assert(ty==FMT_ZIP_STORED || ty==FMT_ZIP_DEFLATED);
      ELSE_IF(EQ(o,7)) /* Extended local header of previous file in ZIP */
        SET(o,0); WHILE(LT(o,13)) IGNORE fxRead8(); INCR(o); ENDWHILE /* BUGFIX: was 15 */
      ELSE_IF(EQ(o,5)) /* End of Central Directory Structure in ZIP */
        /* fprintf(stderr,"EOCDS\n"); */
        SET(o,0); WHILE(LT(o,17)) IGNORE fxRead8(); INCR(o); ENDWHILE
        SET(o,fxRead16z()); /* CML: archive comment length */
        WHILE (NZ(o)) IGNORE fxRead8(); DECR(o); ENDWHILE
      ELSE_IF(EQ(o,1)) /* Central Directory Structure */
        /* fprintf(stderr,"CDS\n"); */
        SET(oo,0); WHILE(LT(oo,25)) IGNORE fxRead8(); INCR(oo); ENDWHILE
        SET(f,fxRead16z()); /* LEN: length of file name */
        SET(o,fxRead16z()); /* XLN: length of extra field */
        SET(q,fxRead16z()); /* CML: length of file comment */
        SET(oo,0); WHILE(LT(oo,12)) IGNORE fxRead8(); INCR(oo); ENDWHILE
        WHILE(NZ(f)) IGNORE fxRead8(); DECR(f); ENDWHILE /* file name */
        WHILE(NZ(o)) IGNORE fxRead8(); DECR(o); ENDWHILE /* extra field */
        WHILE(NZ(q)) IGNORE fxRead8(); DECR(q); ENDWHILE /* file comment */
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
      IGNORE fxRead16(); /* ignore ID2 and CM */
      SET(o,fxRead8()); /* FLG */
      IGNORE fxRead16(); IGNORE fxRead16(); /* ignore MTIME */
      IGNORE fxRead16(); /* ignore XFL, OS */
#ifdef CFG_INPUT_RFC
#else
      IF(TAND_P(o,2)) /* GCONT: skip part number of continuation */
        IGNORE fxRead16();
      ENDIF
#endif
      IF(TAND_P(o,4)) /* ignore FEXTRA */
        SET(q,fxRead16());
        WHILE(NZ(q)) IGNORE fxRead8(); DECR(q); ENDWHILE
      ENDIF
      IF(TAND_P(o,8))
        WHILE(NZ(fxRead8()))
          PASS;
        ENDWHILE
      ENDIF /* ignore FNAME */
      IF(TAND_P(o,16))
        WHILE(NZ(fxRead8()))
          PASS;
        ENDWHILE
      ENDIF /* ignore FCOMMENT */
#ifdef CFG_INPUT_RFC 
      IF(TAND_P(o,2)) IGNORE fxRead16(); ENDIF /* ignore FHCRC */
#else /* CFG_INPUT_FILE */
      IF(TAND_P(o,32)) /* skip encryption header */
        SET(f,0); WHILE(LT(f,12)) IGNORE fxRead8(); INCR(f); ENDWHILE
      ENDIF
#endif
    ENDIF /* IF file format */
    /* fprintf(stderr,"ty=%d\n", ty); */
    IF(EQ(ty,FMT_ZIP_STORED))
      /* fprintf(stderr,"ZIP_STORED oo=%d ooo=%d\n", oo, ooo); */
      SET(oo,0);
      IF((unsigned short)f<32768-(unsigned short)T)
        WHILE(NZ(f))
          ASET_S(T,fxRead8());
          INCR(T);
          DECR(f);
        ENDWHILE
        assert((unsigned short)T<32768);
        IF(NZ(ooo))
          mode=MODE_ZSTO;
          return T;
        ENDIF
        assert(mode==MODE_BOS);
      ELSE
        /* Even if f>=32768 */
        SUB(f,32768-T);
        WHILE((unsigned short)T!=32768)
          ASET_S(T,fxRead8());
          INCR(T);
        ENDWHILE
        SET(T,0);
        mode=MODE_ZSTO;
        return (WORD)32768;
      ENDIF
    ELSE_IF(EQ(ty,FMT_STOP))
      PASS
    ELSE_IF(NE(ty,FMT_NONE))
      SET(mode,MODE_BOB);
      goto mode_bob;
    ENDIF
  ENDWHILE /* outermost WHILE(NE(ty,FMT_STOP)) */
  IF(NZ(T))
    /* Flush unwritten buffer. */
    SET(oo,T);
    SET(T,0);
    return oo;
  ENDIF
  return 0;
ENDFUN

int main(int argc, char **argv);
int main(int argc, char **argv) {
  WORD howmuch;
  (void)argc; (void)argv;
  /* freopen("bash.file.zip","rb",stdin); */
  fxInit();
  SET(ty,FMT_NONE);
  SET(mode,MODE_BOS);
  SET(T,0); /* make the round buffer empty */
  WHILE(0!=(howmuch=fvWork()))
   #if NDEBUG
   #else
    fprintf(stderr,"written=%d ty=%d mode=%d\n", howmuch, ty, mode);
   #endif
   #if CFG_OLD_SLOW_WRITE
    { WORD i;
      SET(i,0);
      WHILE(NZ(howmuch))
        my_putchar(AREF_S(i));
        DECR(howmuch);
        INCR(i);
      ENDWHILE
    }
   #else
    write(1,S,(unsigned short)howmuch);
    /* fwrite(S,1,(unsigned short)howmuch,stdout); -- _not_ slower than write() */
   #endif
  ENDWHILE
  return 0;
}

/* __END__ */
