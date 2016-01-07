import sys
def getchar():
  ch=sys.stdin.read(1)
  if not len(ch):  return -1
  return ord(ch)
D=None
constW=[16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
constU=[3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258]
constP=[0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0]
constQ=[1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577]
constL=[0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13]
C=None
T=None
J=None
Y=None
Z=[None]*320
B=[None]*17
G=[None]*17
N=[None]*1998
S=[None]*32768
def fvRead(arg):
  global Y
  global J
  oo=None
  f=None
  if arg:
    if Y+7<arg:
      J=J+((((((getchar()))&255))<<(Y)))
      Y=Y+(8)
    if Y<arg:
      f=(((getchar()))&255)
      if arg==16:
        oo=J+((((f)&((1<<(16-Y))-1)))<<(Y))
        J=((f)>>(16-Y))
      else:
        oo=((J+((f)<<(Y)))&((1<<(arg))-1))
        J=((f)>>(arg-Y))
      Y=Y+(8-arg)
    else:
      oo=((J)&((1<<(arg))-1))
      Y=Y-(arg)
      J=((((J)>>(arg)))&((1<<(Y))-1))
  else:
    oo=0
  return oo
def fvNalloc():
  global N
  global D
  o=None
  o=D
  D=N[o]
  N[o]=0
  return o
def fvFree(arg):
  global N
  global D
  if arg!=0:
    fvFree(N[arg])
    fvFree(N[arg+1])
    N[arg+1]=0
    N[arg+2]=0
    N[arg]=D
    D=arg
def fvDescend(arg):
  global N
  while N[arg]!=0:
    if fvRead(1):
      arg=N[arg+1]
    else:
      arg=N[arg]
  return N[arg+2]
def fvMktree(arg):
  global B
  global G
  global N
  global Z
  oo=None
  q=None
  o=None
  f=None
  B[0]=0; B[1]=0; B[2]=0; B[3]=0; B[4]=0; B[5]=0; B[6]=0; B[7]=0; B[8]=0
  B[9]=0; B[10]=0; B[11]=0; B[12]=0; B[13]=0; B[14]=0; B[15]=0; B[16]=0
  oo=0
  while oo<arg: B[Z[oo]]=B[Z[oo]]+1; oo+=1
  B[0]=0
  G[0]=0
  G[1]=((G[0]+B[0])<<1); G[2]=((G[1]+B[1])<<1)
  G[3]=((G[2]+B[2])<<1); G[4]=((G[3]+B[3])<<1)
  G[5]=((G[4]+B[4])<<1); G[6]=((G[5]+B[5])<<1)
  G[7]=((G[6]+B[6])<<1); G[8]=((G[7]+B[7])<<1)
  G[9]=((G[8]+B[8])<<1); G[10]=((G[9]+B[9])<<1)
  G[11]=((G[10]+B[10])<<1); G[12]=((G[11]+B[11])<<1)
  G[13]=((G[12]+B[12])<<1); G[14]=((G[13]+B[13])<<1)
  G[15]=((G[14]+B[14])<<1); G[16]=((G[15]+B[15])<<1)
  N[3]=0
  oo=0
  while oo<arg:
    if Z[oo]:
      q=G[Z[oo]]
      G[Z[oo]]=G[Z[oo]]+1
      f=3
      o=Z[oo]
      while o:
        o-=1
        if N[f]==0:
          N[f]=fvNalloc()
        if (0!=((q)&(((1)<<(o))))):
          f=N[f]+1
        else:
          f=N[f]+0
      N[f]=fvNalloc()
      N[N[f]+2]=oo
    oo+=1
  return N[3]
def fvWrite(arg):
  global T
  global S
  global C
  S[T]=arg
  T=(T+1)&32767
  if T==C:
    (sys.stdout.write(chr(S[C])))
    C=(C+1)&32767
def fvMain():
  global J
  global Y
  global C
  global T
  global N
  global D
  o=None
  q=None
  ty=None
  oo=None
  ooo=None
  oooo=None
  f=None
  p=None
  x=None
  v=None
  h=None
  ty=3
  while ty!=4:
    oo=0; ooo=0
    J=0; Y=0; C=0; T=0
    v=0; h=0
    N[0]=0; N[1]=0; N[2]=0
    N[3]=0; N[4]=0; N[5]=0
    D=6
    o=D
    while o<1998:
      N[o]=o+3; o+=1
      N[o]=0; o+=1
      N[o]=0; o+=1
    ty=getchar()
    if (0!=((512+ty)&(256))):
      ty=4
    elif ty==120:
      fvRead(8)
    elif ty==80:
      fvRead(8)
      o=fvRead(8)
      ty=3
      if o==3:
        fvRead(8)
        fvRead(16)
        fvRead(16)
        ty=fvRead(8)
        fvRead(8)
        fvRead(16); fvRead(16)
        fvRead(16); fvRead(16)
        oo=fvRead(8); oo=oo+(((fvRead(8))<<(8)))
        ooo=fvRead(8); ooo=ooo+(((fvRead(8))<<(8)))
        fvRead(16); fvRead(16)
        f=fvRead(8); f=f+(((fvRead(8))<<(8)))
        q=fvRead(8); q=q+(((fvRead(8))<<(8)))
        while f: fvRead(8); f-=1
        while q: fvRead(8); q-=1
      elif o==7:
        o=0
        while o<13: fvRead(8); o+=1
      elif o==5:
        o=0
        while o<17: fvRead(8); o+=1
        o=fvRead(8); o=o+(((fvRead(8))<<(8)))
        while o: fvRead(8); o-=1
      elif o==1:
        oo=0
        while oo<25: fvRead(8); oo+=1
        f=fvRead(8); f=f+(((fvRead(8))<<(8)))
        o=fvRead(8); o=o+(((fvRead(8))<<(8)))
        q=fvRead(8); q=q+(((fvRead(8))<<(8)))
        oo=0
        while oo<12: fvRead(8); oo+=1
        while f: fvRead(8); f-=1
        while o: fvRead(8); o-=1
        while q: fvRead(8); q-=1
    elif ty==31:
      fvRead(16)
      o=fvRead(8)
      fvRead(16); fvRead(16)
      fvRead(16)
      if (0!=((o)&(2))):
        fvRead(16)
      if (0!=((o)&(4))):
        q=fvRead(16)
        while q: fvRead(8); q-=1
      if (0!=((o)&(8))):
        while fvRead(8):
          pass
      if (0!=((o)&(16))):
        while fvRead(8):
          pass
      if (0!=((o)&(32))):
        f=0
        while f<12: fvRead(8); f+=1
    if ty==0:
      while oo: (sys.stdout.write(chr((getchar())))); oo-=1
      while ooo:
        oo=65536
        while oo:
          (sys.stdout.write(chr((getchar()))))
          oo-=1
        ooo-=1
    elif ty==4:
      pass
    elif ty!=3:
      o=0
      while o==0:
        o=fvRead(1)
        q=fvRead(2)
        if q:
          if q==1:
            oo=288
            while oo:
              oo-=1
              if oo<144:
                Z[oo]=8
              elif oo<256:
                Z[oo]=9
              elif oo<280:
                Z[oo]=7
              else:
                Z[oo]=8
            v=fvMktree(288)
            Z[0]=5; Z[1]=5; Z[2]=5; Z[3]=5; Z[4]=5; Z[5]=5; Z[6]=5; Z[7]=5
            Z[8]=5; Z[9]=5; Z[10]=5; Z[11]=5; Z[12]=5; Z[13]=5; Z[14]=5; Z[15]=5
            Z[16]=5; Z[17]=5; Z[18]=5; Z[19]=5; Z[20]=5; Z[21]=5; Z[22]=5; Z[23]=5
            Z[24]=5; Z[25]=5; Z[26]=5; Z[27]=5; Z[28]=5; Z[29]=5; Z[30]=5; Z[31]=5
            h=fvMktree(32)
          else:
            p=fvRead(5)+257
            x=fvRead(5)+1
            v=fvRead(4)+4
            oo=0
            while oo<v: Z[constW[oo]]=fvRead(3); oo+=1
            while oo<19: Z[constW[oo]]=0; oo+=1
            v=fvMktree(19)
            ooo=0
            oo=0
            while oo<p+x:
              oooo=fvDescend(v)
              if oooo==16:
                oooo=ooo; f=3+fvRead(2)
              elif oooo==17:
                oooo=0; f=3+fvRead(3)
              elif oooo==18:
                oooo=0; f=11+fvRead(7)
              else:
                ooo=oooo; f=1
              q=f
              while q: Z[oo]=oooo; oo+=1; q-=1
            fvFree(v)
            v=fvMktree(p)
            oo=x
            while oo: oo-=1; Z[oo]=Z[oo+p]
            h=fvMktree(x)
          oo=fvDescend(v)
          while oo!=256:
            if oo<257:
              fvWrite(oo)
            else:
              oo=oo-(257)
              f=constU[oo]+fvRead(constP[oo])
              oo=fvDescend(h)
              oo=constQ[oo]+fvRead(constL[oo])
              if T<oo:
                oo=32768-oo+T
              else:
                oo=T-oo
              while f:
                fvWrite(S[oo])
                oo=(oo+1)&32767
                f-=1
            oo=fvDescend(v)
          fvFree(v)
          fvFree(h)
        else:
          fvRead(((Y)&7))
          oo=fvRead(16)
          fvRead(16)
          while oo: fvWrite(fvRead(8)); oo-=1
      while C!=T:
        (sys.stdout.write(chr(S[C])))
        C=(C+1)&32767
    fvRead(((Y)&7))
    if ty==31:
      fvRead(16); fvRead(16); fvRead(16); fvRead(16)
    elif ty==120:
      fvRead(16); fvRead(16)
fvMain()
