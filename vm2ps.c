/* vm2.c -- a simple bytecode interpreter (virtual machine)
 * original author of august.c: Ron McFarland <rcm@one.net>, 1996
 * restructured, commented by pts@fazekas.hu at Sat Jul 28 00:24:36 CEST 2001
 * Sat Jul 28 15:13:02 CEST 2001
 */
#include <stdio.h> /* getchar(), putchar() */

#define RAMSIZE 48000
int ram[RAMSIZE];
/* Memory model:
 * 0..codelen-1: program bytecode, microinstructions; 0<=ip && ip<codelen
 * codelen..top-1: data segment
 * top..frame-19: operand stack of current function
 * frame-18..frame-4: local vars of current function
 * frame-3: number of arguments of current function
 * frame-2: frame of caller
 * frame-1: ip of caller
 * frame..frame+nargs-1: arguments of current function
 * frame+nargs..RAMSIZE-1: stack for caller and upper functions
 */
int main() {
  int     *q,
          top, /* top of the stack, inside ram. Push: ram[--top]=val. Pop: ram[top++]. */
          /* frame, address of local data frame */
          r,
          o,
          par, /* value, parameter of the current microinstruction */
          ip, /* Instruction Pointer, inside ram */
          codelen, /* length of program code */
          b;
  char   *d;
  d = " \t\n;{}\0AÊP€³²‰Š@ÉµgEÏÍ±°Œ0Ž‹q";
  b = 256;
  q = ram;
  getchar();
  o = getchar();
  codelen = getchar();
  codelen = codelen + getchar() * b;
  ip = codelen;
  if (o) { /* uncompress compressed bytecode */
    while (ip) {
      o = getchar();
      if (o == 59) {
        r = 2;
      } else if (o == '{') {
        r = 5;
      } else if (o == '}') {
        r = 8;
      } else
        r = 0;
      if (r) {
        r = r + getchar() % 4;
        par = 1 - getchar() - r;
        while (r--) {
          *q = q[par];
          q++;
          ip--;
        }
      } else {
        if ((o == 32) + (o == 9) + (o == 10)) {
          r = getchar();
          o = o % 4 + (r + (r == 123)) % 8 * 3;
          if (8 < o)
            o = o + getchar() % 4 * 9;
          o = d[o] + b;
        }
        *q++ = o % b;
        ip--;
      }
    }
    while (getchar() != 12);
    getchar();
  } else {
    while (ip--)
      *q++ = getchar();
  }
  top = RAMSIZE;
  ip = 0;
  while (ip<codelen) {
    printf("@%9d: ", ip);
    o = ram[ip++];
           if (o == 17) { /* NOT */
      printf("NOT\n");
      /* ram[top] = !ram[top]; */
    } else if (o == 18) { /* NEG */
      printf("NEG\n");
      /* ram[top] = -ram[top]; */
    } else if (o == 12) { /* ADD */
      printf("ADD\n");
      /* r = ram[top++];
      ram[top] = ram[top] + r; */
    } else if (o == 13) { /* SUB */
      printf("SUB\n");
      /* r = ram[top++];
      ram[top] = ram[top] - r; */
    } else if (o == 14) { /* IMUL -- signed ints */
      printf("IMUL\n");
      /* r = ram[top++];
      ram[top] = ram[top] * r; */
    } else if (o == 15) { /* IDIV -- signed ints */
      printf("IDIV\n");
      /* r = ram[top++];
      ram[top] = ram[top] / r; */
    } else if (o == 16) { /* IREM -- signed ints */
      printf("IREM\n");
      /* r = ram[top++];
      ram[top] = ram[top] % r; */
    } else if (o ==  6) { /* EQ */
      printf("EQ\n");
      /* r = ram[top++];
      ram[top] = ram[top] == r; */
    } else if (o ==  7) { /* NE */
      printf("NE\n");
      /* r = ram[top++];
      ram[top] = ram[top] != r; */
    } else if (o ==  8) { /* LT */
      printf("LT\n");
      /* r = ram[top++];
      ram[top] = ram[top] < r; */
    } else if (o == 1) {  /* ptr newval GETSET newval */
      if (ram[ip]==22) { /* GETSET POP ==> SET */
        printf("SET\n");
        ip++;
      } else {
        printf("GETSET\n");
      }
      /* r = ram[top++];
      ram[ram[top]] = r;
      ram[top] = r; */
    } else if (o == 9) {  /* ptr newval SET - */
      printf("SET\n");
      /* r = ram[top++];
      ram[ram[top]] = r;
      top++; */
    } else if (o == 10) { /* RET -- return from subroutine */
      /* there may be same stale jumps to these RETs :-( */
      if (ram[ip]==10) ip++; /* RET RET ==> RET */
      printf("RET\n");
#if 0
      r = ram[top]; /* remember function return value */
      top = frame;
      ip = ram[top - 1];    /* restore old context */
      frame = ram[top - 2]; /* restore old context */
      par = ram[top - 3];   /* restore old context */
      top = top + par / 2;  /* pop arguments */
      ram[top] = r;         /* push return value */
      if (par % 2)          /* optionally ignore return value */
        top++;
#endif
    } else if (o == 22) { /* val POP - */
      printf("POP\n");
      /* top++; */
    } else if (o == 27) { /* HALT -- exit from main program */
      printf("HALT\n");
      /* return ram[top]; */
    } else if (o == 21) { /* val DUP val val */
      printf("DUP\n");
      /* r = ram[top];
      top--;
      ram[top] = r; */
    } else if (o == 23) { /* ptr GET val */
      printf("GET\n");
      /* ram[top] = ram[ram[top]]; */
    } else if (o == 19) { /* ptr GETINC oldval */
      printf("GETINC\n");
      /* ram[top] = ram[ram[top]]++; */
    } else if (o == 25) { /* ptr GETDEC oldval */
      printf("GETDEC\n");
      /* ram[top] = ram[ram[top]]--; */
    } else { /* a long instruction */
      par = o % 16;
      o = o / 16;
      if (par == 7) { /* decode large parameter (0..65535) */
        par = ram[ip++];
        par = par + ram[ip++] * b;
      } else if (7 < par) /* decode small parameter (0..2047) */
        par = par + ram[ip++] * 8 - 1;
             if (o ==  2) { /* PUSHAADR -- push address of function argument */
        printf("%d PUSHAADR\n", par);
        /* top--;
        ram[top] = frame + par; */
      } else if (o ==  4) { /* PUSHLADR -- push address of local var */
        printf("%d PUSHLADR\n", par);
        /* top--;
        ram[top] = frame - par - 4; */
      } else if (o ==  3) { /* PUSHAVAL -- push value of function argument */
        printf("%d PUSHAVAL\n", par);
        /* top--;
        ram[top] = ram[frame + par]; */
      } else if (o ==  5) { /* PUSHLVAL -- push value of local var */
        printf("%d PUSHLVAL\n", par);
        /* top--;
        ram[top] = ram[frame - par - 4]; */
      } else if (o ==  6) { /* PUSHGADR -- push address of global var */
        printf("%d PUSHGADR\n", par);
        /* top--;
        ram[top] = codelen + par; */
      } else if (o ==  7) { /* PUSHGVAL -- push value of global var */
        printf("%d PUSHGVAL\n", par);
        /* top--;
        ram[top] = ram[codelen + par]; */
      } else if (o == 12) { /* PUSH par (constant) */
        printf("%d PUSH\n", par);
        /* top--;
        ram[top] = par; */
      } else if (o == 8) { /* PUSH par (constant) */
        printf("%d PUSH\n", par);
        /* top--;
        ram[top] = par; */
      } else if (o == 9) { /* RCALL IP+val -- relative subroutine call. Unused! */
        printf("%d RCALL\n", par);
        /* top--;
        ram[top] = ip;
        ip = ip + par; */
      } else if (o == 11) { /* faddr arg1 ... nargs-FUNCALL */
        printf("%d FUNCALL\n", par);
#if 0
        r = ram[top - -par / 2];
        if (r < 3) { /* built-in function */
          if (r)
            r = putchar(ram[top]);
          else
            r = getchar();
          top = top + par / 2;
          ram[top] = r;
          if (par % 2) /* optionally ignore return value */
            top++;
        } else { /* user-defined function */
          ram[top - 1] = ip;    /* save context */
          ram[top - 2] = frame; /* save context */
          ram[top - 3] = par;   /* save context -- number of arguments */
          frame = top;          /* establish new local frame */
          top = top - 19;       /* give space for local vars */
          ip = r;               /* JMP */
        };
#endif
      } else if (o == 13) { /* JMP -- unconditional jump */
        printf("%d JMP\n", par);
        /* ip = par; */
      } else if (o == 15) { /* JNZ -- jump if popped parue is non-zero */
        printf("%d JNZ\n", par);
        /* if (ram[top++])
          ip = par; */
      } else if (o == 14) { /* JZ -- jump if popped parue is zero */
        printf("%d JZ\n", par);
        /* if (!ram[top++])
          ip = par; */
      } /* IF long instruction sub-type */
    } /* IF long instruction? */
  } /* WHILE(1) */
  return 0;
} /* main() */

#if 0
Imp: idiv -> bitshift
OK : don''t use boolean as integer
Dat: GETINC, GETDEC
OK : eliminate PUSHLADR
Dat: 0-FUNCALL: call with no arguments, preserve return value
Dat: 2-FUNCALL: call with one argument, preserve return value
OK : `0-FUNCALL'' -> `0 2-FUNCALL''
Imp: `0-PUSHAADR'' -> `0'' (the argument is in memory cell 0), others are shifted

ADD IDIV IMUL IREM LT EQ NE SUB
GET GETDEC GETINC SET
HALT
POP *-PUSH
RET 0-FUNCALL 2-FUNCALL
*-JMP *-JZ
0-PUSHAADR
*-PUSHGADR
0..10-PUSHLADR <-- elminated

Exit codes:
>=0: next instruction group
-1: HALT
-2: RET
<=-3: JMP

{ add						% 0: ADD
  idiv						% 1: IDIV
  mul						% 2: IMUL
  mod						% 3: IREM
  lt						% 4: LT
  eq						% 5: EQ
  ne						% 6: NE
  sub						% 7: SUB
  { Ram exch get }				% 8: GET
  { Ram exch dup Ram exch get 1 add put 0 }     % 9: GETINC
  { Ram exch dup Ram exch get 1 add sub 0 }     % 10: GETDEC
  { Ram 3 1 roll put }                          % 11: SET
  { -1 exit }                                   % 12: HALT
  pop						% 13: POP
  % *-PUSH: the literal number is inserted
  { -2 exit }                                   % 14: RET
  % 0-FUNCALL: changed
  { exch exit }                                 % 15: 2-FUNCALL, leaves `arg faddr''
  { * exit }                                    % 16: JMP
  { 0 eq {* exit}if }                           % 17: JZ
  % 0-PUSHAADR: changed
  % *-PUSHGADR: the literal number (+1) is inserted
} bind

#endif
