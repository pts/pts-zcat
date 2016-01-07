/* vm2.c -- a simple bytecode interpreter (virtual machine)
 * original author of august.c: Ron McFarland <rcm@one.net>, 1996
 * restructured, commented by pts@fazekas.hu at Sat Jul 28 00:24:36 CEST 2001
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
          frame, /* address of local data frame */
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
  while (1) {
    o = ram[ip++];
           if (o == 17) { /* NOT */
      ram[top] = !ram[top];
    } else if (o == 18) { /* NEG */
      ram[top] = -ram[top];
    } else if (o == 12) { /* ADD */
      r = ram[top++];
      ram[top] = ram[top] + r;
    } else if (o == 13) { /* SUB */
      r = ram[top++];
      ram[top] = ram[top] - r;
    } else if (o == 14) { /* IMUL -- signed ints */
      r = ram[top++];
      ram[top] = ram[top] * r;
    } else if (o == 15) { /* IDIV -- signed ints */
      r = ram[top++];
      ram[top] = ram[top] / r;
    } else if (o == 16) { /* IREM -- signed ints */
      r = ram[top++];
      ram[top] = ram[top] % r;
    } else if (o ==  6) { /* EQ */
      r = ram[top++];
      ram[top] = ram[top] == r;
    } else if (o ==  7) { /* NE */
      r = ram[top++];
      ram[top] = ram[top] != r;
    } else if (o ==  8) { /* LT */
      r = ram[top++];
      ram[top] = ram[top] < r;
    } else if (o == 1) {  /* newval ptr GETSET oldval */
      r = ram[top++];
      ram[ram[top]] = r;
      ram[top] = r;
    } else if (o == 9) {  /* newval ptr SET - */
      r = ram[top++];
      ram[ram[top]] = r;
      top++;
    } else if (o == 10) { /* RET -- return from subroutine */
      r = ram[top]; /* remember function return value */
      top = frame;
      ip = ram[top - 1];    /* restore old context */
      frame = ram[top - 2]; /* restore old context */
      par = ram[top - 3];   /* restore old context */
      top = top + par / 2;  /* pop arguments */
      ram[top] = r;         /* push return value */
      if (par % 2)          /* optionally ignore return value */
        top++;
    } else if (o == 22) { /* val POP - */
      top++;
    } else if (o == 27) { /* HALT -- exit from main program */
      return ram[top];
    } else if (o == 21) { /* val DUP val val */
      r = ram[top];
      top--;
      ram[top] = r;
    } else if (o == 23) { /* ptr GET val */
      ram[top] = ram[ram[top]];
    } else if (o == 19) { /* ptr GETINCR oldval */
      ram[top] = ram[ram[top]]++;
    } else if (o == 25) { /* ptr GETDECR oldval */
      ram[top] = ram[ram[top]]--;
    } else { /* a long instruction */
      par = o % 16;
      o = o / 16;
      if (par == 7) { /* decode large parameter (0..65535) */
        par = ram[ip++];
        par = par + ram[ip++] * b;
      } else if (7 < par) /* decode small parameter (0..2047) */
        par = par + ram[ip++] * 8 - 1;
             if (o ==  2) { /* PUSHADR -- push address of function argument */
        top--;
        ram[top] = frame + par;
      } else if (o ==  4) { /* PUSHADR -- push address of local var */
        top--;
        ram[top] = frame - par - 4;
      } else if (o ==  3) { /* PUSHVAR -- push value of function argument */
        top--;
        ram[top] = ram[frame + par];
      } else if (o ==  5) { /* PUSHVAL -- push value of local var */
        top--;
        ram[top] = ram[frame - par - 4];
      } else if (o ==  6) { /* PUSHADR -- push address of global var */
        top--;
        ram[top] = codelen + par;
      } else if (o ==  7) { /* PUSHVAL -- push value of global var */
        top--;
        ram[top] = ram[codelen + par];
      } else if (o == 12) { /* PUSH par (constant) */
        top--;
        ram[top] = par;
      } else if (o == 8) { /* PUSH par (constant) */
        top--;
        ram[top] = par;
      } else if (o == 9) { /* CALL IP+par -- relative subroutine call */
        top--;
        ram[top] = ip;
        ip = ip + par;
      } else if (o == 11) { /* FUNCTION_CALL */
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
      } else if (o == 13) { /* JMP -- unconditional jump */
        ip = par;
      } else if (o == 15) { /* JNZ -- jump if popped parue is non-zero */
        if (ram[top++])
          ip = par;
      } else if (o == 14) { /* JZ -- jump if popped parue is zero */
        if (!ram[top++])
          ip = par;
      } /* IF long instruction sub-type */
    } /* IF long instruction? */
  } /* WHILE(1) */
} /* main() */
