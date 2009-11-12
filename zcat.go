// An unoptimized zcat (GZip uncompressor) implementation in Google Go.
// by pts@fazekas.hu at Thu Nov 12 23:57:34 CET 2009
//
// 8g zcat.go && 8l zcat.8 && ./8.out <test.gz >test.bin
//
// kat@pax:~/d$ time ./ansic.bin <../anyremote-4.11.tar.gz >/dev/null
// real    0m0.433s
// user    0m0.428s
// sys     0m0.008s
// kat@pax:~/d$ time ./8.out <../anyremote-4.11.tar.gz >/dev/null
// real    0m1.050s
// user    0m1.048s
// sys     0m0.000s
kat@pax:~/d$ time gzip -cd <../anyremote-4.11.tar.gz >/dev/null
// 
// real    0m0.084s
// user    0m0.084s
// sys     0m0.000s

package main

import (
  "bufio";
  "os";
)

// parts of stdio.h
var stdin  *bufio.Reader
var stdout *bufio.Writer
func init() {
  // glibc-2.3.6 has 8192-byte buffer as well
  stdin,  _ = bufio.NewReaderSize(os.Stdin,  8192);
  stdout, _ = bufio.NewWriterSize(os.Stdout, 8192);
}
func getchar() uint16 {
  c, err := stdin.ReadByte();
  if err == os.EOF { return 0xffff }
  if err != nil { panic("error reading stdin: " + err.String()) }
  return uint16(c);
}
func putchar(b byte) {
  err := stdout.WriteByte(b);
  if err != nil { panic("error writing stdout: " + err.String()) }
}

var W = [...]uint16{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 }
var U = [...]uint16{ 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 }
var P = [...]uint16{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 }
var Q = [...]uint16{ 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 }
var L = [...]uint16{ 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 }
var C, T, J, Y, D uint16
var Z[320]uint16
var B, G [17]uint16
var N[1998]uint16
var S [32768]byte

func fvRead(arg uint16) uint16 {
  var oo, f uint16;
  if arg != 0 {
    if Y+7<arg {
      J+=((((getchar())&255))<<(Y));
      Y+=8;
    }
    if Y<arg {
      f=((getchar())&255);
      if arg==16 {
        oo=J+((((f)&((1<<(16-Y))-1)))<<(Y));
        J=((f)>>(16-Y));
      } else {
        oo=((J+((f)<<(Y)))&((1<<(arg))-1));
        J=((f)>>(arg-Y));
      }
      Y+=8-arg;
    } else {
      oo=((J)&((1<<(arg))-1));
      Y-=arg;
      J=((((J)>>(arg)))&((1<<(Y))-1));
    }
  } else {
    oo=0;
  }
  return oo;
}
func fvNalloc() uint16 {
  var o uint16;
  o=D;
  D=N[o];
  N[o]=0;
  return o;
}
func fvFree(arg uint16) {
  if arg!=0 {
    fvFree(N[arg]);
    fvFree(N[arg+1]);
    N[arg+1]=0;
    N[arg+2]=0;
    N[arg]=D;
    D=arg;
  }
}
func fvDescend(arg uint16) uint16 {
  for N[arg]!=0 {
    if fvRead(1) != 0 {
      arg=N[arg+1];
    } else {
      arg=N[arg];
    }
  }
  return N[arg+2];
}
func fvMktree(arg uint16) uint16 {
  var oo, q, o, f uint16;
  B[0]=0; B[1]=0; B[2]=0; B[3]=0; B[4]=0; B[5]=0; B[6]=0; B[7]=0; B[8]=0;
  B[9]=0; B[10]=0; B[11]=0; B[12]=0; B[13]=0; B[14]=0; B[15]=0; B[16]=0;
  oo=0;
  for oo<arg { B[Z[oo]]++; oo++; }
  B[0]=0;
  G[0]=0;
  G[1]=((G[0]+B[0])<<1); G[2]=((G[1]+B[1])<<1);
  G[3]=((G[2]+B[2])<<1); G[4]=((G[3]+B[3])<<1);
  G[5]=((G[4]+B[4])<<1); G[6]=((G[5]+B[5])<<1);
  G[7]=((G[6]+B[6])<<1); G[8]=((G[7]+B[7])<<1);
  G[9]=((G[8]+B[8])<<1); G[10]=((G[9]+B[9])<<1);
  G[11]=((G[10]+B[10])<<1); G[12]=((G[11]+B[11])<<1);
  G[13]=((G[12]+B[12])<<1); G[14]=((G[13]+B[13])<<1);
  G[15]=((G[14]+B[14])<<1); G[16]=((G[15]+B[15])<<1);
  N[3]=0;
  oo=0;
  for oo<arg {
    if Z[oo] != 0 {
      q=G[Z[oo]];
      G[Z[oo]]++;
      f=3;
      o=Z[oo];
      for o != 0 {
        o--;
        if N[f]==0 {
          N[f]=fvNalloc ();
        }
        if (0!=((q&(((1)<<(o)))))) {
          f=N[f]+1;
        } else {
          f=N[f]+0;
        }
      }
      N[f]=fvNalloc ();
      N[N[f]+2]=oo;
    }
    oo++;
  }
  return N[3];
}
func fvWrite(arg uint16) {
  S[T]=byte(arg);
  (T)++; (T)&=32767;
  if T==C {
    putchar(S[C]);
    (C)++; (C)&=32767;
  }
}
func main() {
  var o, q, ty, oo, ooo, oooo, f, p, x, v, h uint16;
  ty=3;
  for ty!=4 {
    oo=0; ooo=0;
    J=0; Y=0; C=0; T=0;
    v=0; h=0;
    N[0]=0; N[1]=0; N[2]=0;
    N[3]=0; N[4]=0; N[5]=0;
    D=6;
    o=D;
    for o<1998 {
      N[o]=o+3; o++;
      N[o]=0; o++;
      N[o]=0; o++;
    }
    ty=getchar();
    if (0!=(ty & 256)) {
      ty=4;
    } else if ty==120 {
      fvRead(8);
    } else if ty==80 {
      fvRead(8);
      o=fvRead(8);
      ty=3;
      if o==3 {
        fvRead(8);
        fvRead(16);
        fvRead(16);
        ty=fvRead(8);
        fvRead(8);
        fvRead(16); fvRead(16);
        fvRead(16); fvRead(16);
        oo=fvRead(8); oo+=((fvRead(8))<<(8));
        ooo=fvRead(8); ooo+=((fvRead(8))<<(8));
        fvRead(16); fvRead(16);
        f=fvRead(8); f+=((fvRead(8))<<(8));
        q=fvRead(8); q+=((fvRead(8))<<(8));
        for f != 0 { fvRead(8); f--; }
        for q != 0 { fvRead(8); q--; }
      } else if o==7 {
        o=0; for o<13 { fvRead(8); o++; }
      } else if o==5 {
        o=0; for o<17 { fvRead(8); o++; }
        o=fvRead(8); o+=((fvRead(8))<<(8));
        for o != 0 { fvRead(8); o--; }
      } else if o==1 {
        oo=0; for oo<25 { fvRead(8); oo++; }
        f=fvRead(8); f+=((fvRead(8))<<(8));
        o=fvRead(8); o+=((fvRead(8))<<(8));
        q=fvRead(8); q+=((fvRead(8))<<(8));
        oo=0; for oo<12 { fvRead(8); oo++; }
        for f != 0 { fvRead(8); f--; }
        for o != 0 { fvRead(8); o--; }
        for q != 0 { fvRead(8); q--; }
      }
    } else if ty==31 {
      fvRead(16);
      o=fvRead(8);
        fvRead(16); fvRead(16);
        fvRead(16);
      if (0!=((o&(2)))) {
        fvRead(16);
      }
      if (0!=((o&(4)))) {
        q=fvRead(16);
        for q != 0 { fvRead(8); q--; }
      }
      if (0!=((o&(8)))) {
        for fvRead(8) != 0 {}
      }
      if (0!=((o&(16)))) {
        for fvRead(8) != 0 {}
      }
      if (0!=((o&(32)))) {
        f=0; for f<12 { fvRead(8); f++; }
      }
    }
    if ty==0 {
      for oo != 0 { putchar(byte(getchar())); oo--; }
      for ooo != 0 {
        putchar(byte(getchar()));
        putchar(byte(getchar()));
        oo=32767;
        for oo != 0 {
          putchar(byte(getchar()));
          putchar(byte(getchar()));
          oo--;
        }
        ooo--;
      }
    } else if ty==4 {
    } else if ty!=3 {
      o=0;
      for o==0 {
        o=fvRead(1);
        q=fvRead(2);
        if q != 0 {
          if q==1 {
            oo=288;
            for oo != 0 {
              oo--;
              if oo<144 {
                Z[oo]=8;
              } else if oo<256 {
                Z[oo]=9;
              } else if oo<280 {
                Z[oo]=7;
              } else {
                Z[oo]=8;
              }
            }
            v=fvMktree(288);
            Z[0]=5; Z[1]=5; Z[2]=5; Z[3]=5; Z[4]=5; Z[5]=5; Z[6]=5; Z[7]=5;
            Z[8]=5; Z[9]=5; Z[10]=5; Z[11]=5; Z[12]=5; Z[13]=5; Z[14]=5; Z[15]=5;
            Z[16]=5; Z[17]=5; Z[18]=5; Z[19]=5; Z[20]=5; Z[21]=5; Z[22]=5; Z[23]=5;
            Z[24]=5; Z[25]=5; Z[26]=5; Z[27]=5; Z[28]=5; Z[29]=5; Z[30]=5; Z[31]=5;
            h=fvMktree(32);
          } else {
            p=fvRead(5) + 257;
            x=fvRead(5) + 1;
            v=fvRead(4) + 4;
            oo=0;
            for oo<v { Z[W[oo]]=fvRead(3); oo++; }
            for oo<19 { Z[W[oo]]=0; oo++; }
            v=fvMktree(19);
            ooo=0;
            oo=0;
            for oo<p + x {
              oooo=fvDescend(v);
              if oooo==16 {
                oooo=ooo; f=3+fvRead(2);
              } else if oooo==17 {
                oooo=0; f=3+fvRead(3);
              } else if oooo==18 {
                oooo=0; f=11+fvRead(7);
              } else {
                ooo=oooo; f=1;
              }
              q=f;
              for q != 0 { Z[oo]=oooo; oo++; q--; }
            }
            fvFree(v);
            v=fvMktree(p);
            oo=x;
            for oo != 0 { oo--; Z[oo]=Z[oo + p]; }
            h=fvMktree(x);
          }
          oo=fvDescend(v);
          for oo!=256 {
            if oo<257 {
              fvWrite(oo);
            } else {
              oo-=257;
              f=U[oo] + fvRead(P[oo]);
              oo=fvDescend(h);
              oo=Q[oo] + fvRead(L[oo]);
              if T<oo {
                oo=32768-oo+T;
              } else {
                oo=T-oo;
              }
              for f != 0 {
                fvWrite(uint16(S[oo]));
                (oo)++; (oo)&=32767;
                f--;
              }
            }
            oo=fvDescend(v);
          }
          fvFree(v);
          fvFree(h);
        } else {
          fvRead(((Y)&7));
          oo=fvRead(16);
          fvRead(16);
          for oo != 0 { fvWrite(fvRead(8)); oo--; }
        }
      }
      for C!=T {
        putchar(S[C]);
        (C)++; (C)&=32767;
      }
    }
    fvRead(((Y)&7));
    if ty==31 {
      fvRead(16); fvRead(16); fvRead(16); fvRead(16);
    } else if ty==120 {
      fvRead(16); fvRead(16);
    }
  }
  stdout.Flush();
}
