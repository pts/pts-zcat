// by pts@fazekas.hu at 2002.02.20

import windows;

  extern (Windows) HANDLE GetStdHandle(DWORD nStdHandle);
  const DWORD STD_INPUT_HANDLE=(DWORD)(0xfffffff6),
            STD_OUTPUT_HANDLE=(DWORD)(0xfffffff5),
            STD_ERROR_HANDLE=(DWORD)(0xfffffff4);
  HANDLE hstdin, hstdout;

  static const int BUFSIZE=4096;
  static char rbuf[BUFSIZE];
  static char wbuf[BUFSIZE];
  static char *rend=(char*)rbuf+BUFSIZE;
  static char *rp=(char*)rbuf+BUFSIZE;
  static char *wend=(char*)wbuf+BUFSIZE;
  static char *wp=(char*)wbuf;
  static /*inline*/ void my_putchar(byte c) {
    if (wp==wend) flush_putchar(c); else *wp++=c;
  }
  static /*inline*/ short my_getchar() {
    return rp==rend?more_getchar():*rp++;
  }
  static void flush_putchar(byte c) {
    DWORD dummy;
    WriteFile(hstdout,wbuf,wp-(char*)wbuf,&dummy,null); /* assume no errors */
    *wbuf=c;
    wp=(char*)wbuf+1;
  }
  static int more_getchar() {
    DWORD got;
    if (rend==rbuf) return -1; /* previous read returned EOF */
    if (!ReadFile(hstdin,(char*)rbuf,BUFSIZE,&got,null)) got=0;
    if (got<1) { rend=rbuf; return -1; }
    rend=(char*)rbuf+got;
    rp=(char*)rbuf+1;
    return *rbuf;
  }

  int main(int argc, char **argv) {
    (void)argc; (void)argv;
    hstdin=GetStdHandle(STD_INPUT_HANDLE);
    hstdout=GetStdHandle(STD_OUTPUT_HANDLE);
    short  io, iq;
     io = 1  ;  iq = 0  ; while ( iq<30 ) {    constQ  [  iq  ] = io+1  ; ( iq ++) ;   constQ  [  iq  ] = io*3 / 2+1  ;   ( iq ++) ;  io = (( io )<<1)   ; } 
      constQ  [  0  ] = 1  ;
     iq = 0  ; while ( iq<30 ) {    constL  [  iq  ] = iq / 2-1  ; ( iq ++) ; }   
      constL  [  0  ] = 0  ;   constL  [  1  ] = 0  ;
      constU  [  0  ] = 3  ;   constU  [  1  ] = 4  ;   constU  [  2  ] = 5  ;   constU  [  3  ] = 6  ;   constU  [  4  ] = 7  ;   constU  [  5  ] = 8  ;   constU  [  6  ] = 9  ;   constU  [  7  ] = 10  ;   constU  [  8  ] = 11  ;   constU  [  9  ] = 13  ;   constU  [  10  ] = 15  ;   constU  [  11  ] = 17  ;   constU  [  12  ] = 19  ;   constU  [  13  ] = 23  ;   constU  [  14  ] = 27  ;   constU  [  15  ] = 31  ;   constU  [  16  ] = 35  ;   constU  [  17  ] = 43  ;   constU  [  18  ] = 51  ;   constU  [  19  ] = 59  ;   constU  [  20  ] = 67  ;   constU  [  21  ] = 83  ;   constU  [  22  ] = 99  ;   constU  [  23  ] = 115  ;   constU  [  24  ] = 131  ;   constU  [  25  ] = 163  ;   constU  [  26  ] = 195  ;   constU  [  27  ] = 227  ;   constU  [  28  ] = 258  ;
      constW  [  0  ] = 16  ;   constW  [  1  ] = 17  ;   constW  [  2  ] = 18  ;   constW  [  3  ] = 0  ;   constW  [  4  ] = 8  ;   constW  [  5  ] = 7  ;   constW  [  6  ] = 9  ;   constW  [  7  ] = 6  ;   constW  [  8  ] = 10  ;   constW  [  9  ] = 5  ;   constW  [  10  ] = 11  ;   constW  [  11  ] = 4  ;   constW  [  12  ] = 12  ;   constW  [  13  ] = 3  ;   constW  [  14  ] = 13  ;   constW  [  15  ] = 2  ;   constW  [  16  ] = 14  ;   constW  [  17  ] = 1  ;   constW  [  18  ] = 15  ;
     iq = 0  ;  io = 0  ; while ( iq<28 ) {  if (  iq == 4   ) {   io = 1  ; }    constP  [  iq  ] = iq / 4-io  ; ( iq ++) ; }   
      constP  [  28  ] = 0  ;
    fvMain();
    flush_putchar(0);
    return 0;
  }
static  short     D ; 
  static  short     constW [ 19 ]; 
  static  short     constU [ 29 ]; 
  static  short     constP [ 29 ]; 
  static  short     constQ [ 30 ]; 
  static  short     constL [ 30 ]; 
static  short    C ;   
static  short    T ;   
static  short    J ; 
static  short    Y ; 
static  short    Z [ 320 ]; 
static  short    B [ 17 ]; 
static  short    G [ 17 ]; 
static  short    N [ 1998  ]; 
  static  byte    S [ 32768 ]; 
static  short    fvRead ( short   arg) { 
   short    oo  ; 
   short    f  ; 
  if (  arg   ) { 
    if (  Y+7 < arg   ) { 
      (( J )+=( (( (( my_getchar  () )&255)  )<<( Y ))  )) ;
      (( Y )+=( 8 )) ;
    } 
    if (  Y < arg   ) {   
       f  = (( my_getchar  () )&255)   ;
      if (  arg == 16   ) { 
         oo  = J+ (( ( f  &((1<<( 16-Y ))-1))  )<<( Y ))   ;
         J = (( f  )>>(  16-Y ))   ;  
      } else { 
         oo  = (( J+ (( f  )<<( Y ))  )&((1<<(  arg ))-1))   ;
         J = (( f  )>>(  arg-Y ))   ;  
      } 
      (( Y )+=(  8-arg )) ;  
    } else {   
       oo  = ( J &((1<<(  arg ))-1))   ;
      (( Y )-=( arg )) ;
       J = (( (( J )>>(  arg ))  )&((1<<(  Y ))-1))   ;  
    } 
  } else { 
     oo  = 0  ;
  } 
  return  oo  ; 
} 
static  short     fvNalloc () { 
   short     o  ;   
   o  = D  ;
   D =  N [ o  ]   ;  
    N  [  o   ] = 0   ;  
  return  o  ; 
} 
static void  fvFree ( short    arg) { 
  if (  arg != 0    ) { 
    fvFree( N [ arg ] );
    fvFree( N [ arg+1 ] );
      N  [  arg+1  ] = 0   ;  
      N  [  arg+2  ] = 0  ;  
      N  [  arg  ] = D  ;  
     D = arg  ;  
  } 
} 
static  short    fvDescend ( short    arg) { 
  while (  0  !=  N [ arg ]    ) { 
    if (  fvRead(1)   ) { 
       arg =  N [ arg+1 ]   ;
    } else { 
       arg =  N [ arg ]   ;
    } 
  } 
  return   N [ arg+2 ]  ;   
} 
static  short     fvMktree ( short   arg) { 
   short    oo  ; 
   short    q  ; 
   short    o  ; 
   short    f  ; 
    B  [  0  ] = 0  ;   B  [  1  ] = 0  ;   B  [  2  ] = 0  ;   B  [  3  ] = 0  ;   B  [  4  ] = 0  ;   B  [  5  ] = 0  ;   B  [  6  ] = 0  ;   B  [  7  ] = 0  ;   B  [  8  ] = 0  ;
    B  [  9  ] = 0  ;   B  [  10  ] = 0  ;   B  [  11  ] = 0  ;   B  [  12  ] = 0  ;   B  [  13  ] = 0  ;   B  [  14  ] = 0  ;   B  [  15  ] = 0  ;   B  [  16  ] = 0  ;
   oo  = 0  ;
  while (  oo  < arg   ) {  (  B [  Z [ oo  ]  ]  ++) ; ( oo  ++) ; }   
    B  [  0  ] = 0  ;
    G  [  0  ] = 0  ;
    G  [   1  ] =  ((  G [  0 ] +  B [  0 ]  )<<1)   ;   G  [   2  ] =  ((  G [  1 ] +  B [  1 ]  )<<1)   ;
    G  [   3  ] =  ((  G [  2 ] +  B [  2 ]  )<<1)   ;   G  [   4  ] =  ((  G [  3 ] +  B [  3 ]  )<<1)   ;
    G  [   5  ] =  ((  G [  4 ] +  B [  4 ]  )<<1)   ;   G  [   6  ] =  ((  G [  5 ] +  B [  5 ]  )<<1)   ;
    G  [   7  ] =  ((  G [  6 ] +  B [  6 ]  )<<1)   ;   G  [   8  ] =  ((  G [  7 ] +  B [  7 ]  )<<1)   ;
    G  [   9  ] =  ((  G [  8 ] +  B [  8 ]  )<<1)   ;   G  [  10  ] =  ((  G [  9 ] +  B [  9 ]  )<<1)   ;
    G  [  11  ] =  ((  G [ 10 ] +  B [ 10 ]  )<<1)   ;   G  [  12  ] =  ((  G [ 11 ] +  B [ 11 ]  )<<1)   ;
    G  [  13  ] =  ((  G [ 12 ] +  B [ 12 ]  )<<1)   ;   G  [  14  ] =  ((  G [ 13 ] +  B [ 13 ]  )<<1)   ;
    G  [  15  ] =  ((  G [ 14 ] +  B [ 14 ]  )<<1)   ;   G  [  16  ] =  ((  G [ 15 ] +  B [ 15 ]  )<<1)   ;
    N  [  3  ] = 0   ;  
   oo  = 0  ;
  while (  oo  < arg   ) { 
    if (   Z [ oo  ]    ) { 
       q  =  G [  Z [ oo  ]  ]   ;
      (  G [  Z [ oo  ]  ]  ++) ;
       f  = 3  ;  
       o  =  Z [ oo  ]   ;
      while (  o    ) { 
        ( o  --) ;
        if (   N [ f  ]  == 0    ) { 
            N  [  f   ] = fvNalloc ()   ;
        } 
        if ( (0!=( q  &( (( 1 )<<( o  ))  )))  ) { 
           f  =  N [ f  ] +1  ;
        } else { 
           f  =  N [ f  ] +0  ;
        } 
      } 
        N  [  f   ] = fvNalloc ()   ;
        N  [   N [ f  ] +2  ] = oo   ;  
    } 
    ( oo  ++) ;
  } 
  return   N [ 3 ]  ; 
} 
static void  fvWrite ( short   arg) { 
    S  [   T   ] =  arg    ;
   T ++; ( T )&=32767 ;  
  if (  T == C   ) { 
    my_putchar ( S [  C  ]  );
     C ++; ( C )&=32767 ;  
  } 
} 
static void  fvMain () { 
   short    o ; 
   short    q ; 
   short    ty ;   
   short    oo ; 
   short    ooo ; 
   short    oooo ; 
   short    f ; 
   short    p ; 
   short    x ; 
   short     v ; 
   short     h ; 
   ty = 3   ;
  while (  ty != 4    ) {   
   oo = 0  ;  ooo = 0  ;  
   J = 0  ;  Y = 0  ;  C = 0  ;  T = 0  ;
   v = 0   ;  h = 0   ;
    N  [  0  ] = 0   ;   N  [  1  ] = 0   ;   N  [  2  ] = 0  ;  
    N  [  3  ] = 0   ;   N  [  4  ] = 0   ;   N  [  5  ] = 0  ;  
   D = 6  ;  
   o = D  ;
  while (  o < 1998    ) { 
      N  [  o  ] = o+3  ;  ( o ++) ;  
      N  [  o  ] = 0   ; ( o ++) ;  
      N  [  o  ] = 0  ;    ( o ++) ;  
  } 
     ty = my_getchar  ()  ;  
    if ( (0!=(( 512+ty )&( 256 )))  ) {   
       ty = 4   ;
    } else if (  ty == 120    ) {   
        fvRead(8);  
    } else if (  ty == 80   ) {   
        fvRead(8);  
       o = fvRead(8)  ;
       ty = 3   ;
      if (  o == 3   ) {   
          fvRead(8);  
          fvRead(16);  
          fvRead(16);  
         ty = fvRead(8)  ;  
          fvRead(8);  
          fvRead(16);   fvRead(16);  
          fvRead(16);   fvRead(16);  
         oo =  fvRead(8)  ; (( oo )+=(   (( fvRead(8) )<<( 8 ))  )) ;  
         ooo = fvRead(8)  ; (( ooo )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
          fvRead(16);   fvRead(16);  
         f = fvRead(8)  ; (( f )+=(  (( fvRead(8) )<<( 8 ))  )) ;        
         q = fvRead(8)  ; (( q )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
        while (  f   ) {    fvRead(8); ( f --) ; }   
        while (  q   ) {    fvRead(8); ( q --) ; }   
      } else if (  o == 7   ) {   
         o = 0  ; while (  o < 13   ) {    fvRead(8); ( o ++) ; }   
      } else if (  o == 5   ) {   
         o = 0  ; while (  o < 17   ) {    fvRead(8); ( o ++) ; } 
         o = fvRead(8)  ; (( o )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
        while (  o   ) {    fvRead(8); ( o --) ; } 
      } else if (  o == 1   ) {   
         oo = 0  ; while (  oo < 25   ) {    fvRead(8); ( oo ++) ; } 
         f = fvRead(8)  ; (( f )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
         o = fvRead(8)  ; (( o )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
         q = fvRead(8)  ; (( q )+=(  (( fvRead(8) )<<( 8 ))  )) ;  
         oo = 0  ; while (  oo < 12   ) {    fvRead(8); ( oo ++) ; } 
        while (  f   ) {    fvRead(8); ( f --) ; }   
        while (  o   ) {    fvRead(8); ( o --) ; }   
        while (  q   ) {    fvRead(8); ( q --) ; }   
      }   
    } else if (  ty == 31   ) {   
        fvRead(16);  
       o = fvRead(8)  ;  
          fvRead(16);   fvRead(16);  
          fvRead(16);  
      if ( (0!=( o &( 2 )))  ) {   
          fvRead(16);
      } 
      if ( (0!=( o &( 4 )))  ) {   
         q = fvRead(16)  ;
        while (  q   ) {    fvRead(8); ( q --) ; } 
      } 
      if ( (0!=( o &( 8 )))  ) { 
        while (  fvRead(8)   ) { 
           ;
        } 
      }   
      if ( (0!=( o &( 16 )))  ) { 
        while (  fvRead(8)   ) { 
           ;
        } 
      }   
      if ( (0!=( o &( 32 )))  ) {   
         f = 0  ; while (  f < 12   ) {    fvRead(8); ( f ++) ; } 
      } 
    }   
    if (  ty == 0    ) { 
      while (  oo   ) {  my_putchar (my_getchar  ()); ( oo --) ; } 
      while (  ooo   ) { 
        my_putchar (my_getchar  ());
        my_putchar (my_getchar  ());
         oo = 32767  ;
        while (  oo   ) { 
          my_putchar (my_getchar  ());
          my_putchar (my_getchar  ());
          ( oo --) ;
        } 
        ( ooo --) ;
      } 
    } else if (  ty == 4    ) { 
    } else if (  ty != 3    ) { 
   o = 0  ;
  while (  o == 0   ) { 
     o = fvRead(1)  ;  
     q = fvRead(2)  ;  
    if (  q   ) { 
      if (  q == 1   ) { 
         oo = 288  ;
        while (  oo   ) { 
          ( oo --) ;
            if (  oo < 144   ) { 
                Z  [  oo  ] = 8  ;
            } else if (  oo < 256   ) { 
                Z  [  oo  ] = 9  ;
            } else if (  oo < 280   ) { 
                Z  [  oo  ] = 7  ;
            } else { 
                Z  [  oo  ] = 8  ;
            } 
        } 
         v = fvMktree(288)  ;
          Z  [   0  ] = 5  ;   Z  [   1  ] = 5  ;   Z  [   2  ] = 5  ;   Z  [   3  ] = 5  ;   Z  [   4  ] = 5  ;   Z  [   5  ] = 5  ;   Z  [   6  ] = 5  ;   Z  [   7  ] = 5  ;
          Z  [   8  ] = 5  ;   Z  [   9  ] = 5  ;   Z  [  10  ] = 5  ;   Z  [  11  ] = 5  ;   Z  [  12  ] = 5  ;   Z  [  13  ] = 5  ;   Z  [  14  ] = 5  ;   Z  [  15  ] = 5  ;
          Z  [  16  ] = 5  ;   Z  [  17  ] = 5  ;   Z  [  18  ] = 5  ;   Z  [  19  ] = 5  ;   Z  [  20  ] = 5  ;   Z  [  21  ] = 5  ;   Z  [  22  ] = 5  ;   Z  [  23  ] = 5  ;
          Z  [  24  ] = 5  ;   Z  [  25  ] = 5  ;   Z  [  26  ] = 5  ;   Z  [  27  ] = 5  ;   Z  [  28  ] = 5  ;   Z  [  29  ] = 5  ;   Z  [  30  ] = 5  ;   Z  [  31  ] = 5  ;
         h = fvMktree(32)  ;
      } else { 
         p =  fvRead(5) + 257  ;  
         x =  fvRead(5) + 1  ;  
         v =  fvRead(4) + 4  ;    
         oo =  0  ;
        while (  oo < v   ) {     Z  [   constW [ oo ]   ] =  fvRead(3)  ; ( oo ++) ; }   
        while (  oo < 19   ) {    Z  [   constW [ oo ]   ] =  0  ; ( oo ++) ; } 
         v = fvMktree(19)  ;
         ooo = 0  ;
         oo = 0  ;
        while (  oo < p + x   ) { 
           oooo = fvDescend(v)  ;
          if (  oooo == 16   ) { 
             oooo = ooo  ;  f = 3+fvRead(2)  ;
          } else if (  oooo == 17   ) { 
             oooo = 0  ;  f = 3+fvRead(3)  ;
          } else if (  oooo == 18   ) { 
             oooo = 0  ;  f = 11+fvRead(7)  ;
          } else { 
             ooo = oooo  ;  f = 1  ;
          } 
           q = f  ;
          // printf("oo=%d\n", oo);
          while (  q   ) {    Z  [  oo  ] = oooo  ; ( oo ++) ; ( q --) ; } 
        } 
        fvFree(v);
         v = fvMktree(p)  ;
         oo = x  ;
        while (  oo   ) {  ( oo --) ;   Z  [  oo  ] =  Z [ oo + p ]   ; } 
         h = fvMktree(x)  ;
      } 
       oo = fvDescend(v)  ;
      while (  oo !=  256   ) { 
        if (  oo < 257   ) { 
          fvWrite(oo);
        } else { 
          (( oo )-=( 257 )) ;
           f =  constU [ oo ]  + fvRead( constP [ oo ] )  ;  
           oo = fvDescend(h)  ;
           oo =  constQ [ oo ]  + fvRead( constL [ oo ] )  ;  
          if (  T < oo   ) { 
             oo = 32768-oo+T  ;
          } else { 
             oo = T-oo  ;
          } 
          while (  f   ) { 
            fvWrite( S [  oo  ]  );
             oo ++; ( oo )&=32767 ;    
            ( f --) ;
          } 
        } 
         oo = fvDescend(v)  ;
      } 
      fvFree(v);
      fvFree(h);
    } else {   
        fvRead(( Y &7) );
       oo = fvRead(16)  ;  
        fvRead(16);  
      while (  oo   ) {  fvWrite(fvRead(8)); ( oo --) ; } 
    } 
  } 
  while (  C != T   ) { 
    my_putchar ( S [  C  ]  );
     C ++; ( C )&=32767 ;    
  } 
  }   
    fvRead(( Y &7) );
  if (  ty == 31    ) { 
      fvRead(16);   fvRead(16);   fvRead(16);   fvRead(16);
  } else if (  ty == 120    ) { 
      fvRead(16);   fvRead(16);
  } 
  }   
} 
