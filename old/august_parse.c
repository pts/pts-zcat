int curloc;  
int lexval;  
int token;  
int pusharg;  
int thechar;  
int nglob;  
int nlocal;  
int nfun;  
int strsize;  
int narg;  
int curgloboffs;  
int nsym;  
int pushop;  
int argids[50 ];
int globids[300 ];
int globoffs[300 ];
int globscalar[300 ];
int funids[300 ];
int funoffs[300 ];
int localids[50 ];
char symbol[5000 ];
char code[10000 ];
char names[5000 ];
int
digit(int c)
{
    return (( '/' < c ) * (  c < ':' )) ;
}
int
letter(int c)
{
    return (( '`' < c ) * (  c < '{' )) 
        ;
}
int
eqstr(char *p, char *q)
{
    while(*p) {
	if (*p++ != *q++)
	    return 0;
    }
    return !*q;
}
int
lookup(char *name)
{
    int i;
    char *ns;
    ns = names;
    i = 0;
    while(i < nsym) {
	if (eqstr(ns, name)) {
	    return i;
	}
	while(*ns++)
	    ;
	i++;
    }
    while(*ns++ = *name++)
	;
    return nsym++;
}
int
next()
{
    int r;
    r = thechar;
    thechar = getchar();
    return r;
}
int
gobble(int t, int rr, int r)
{
    if (thechar == t) {
	next();
	return rr;
    }
    return r;
}
int
getstring(int delim)
{
    int c;
    strsize = 0;
    while((c = next()) != delim) {
	if (c == '\\') {
	    if ( (c = next()) == 'n') c = '\n';
	      if (c == 't') c = '\t';
	      if (c == '0') c = 0;
	}
	symbol[strsize++] = c;
    }
    symbol[strsize++] = 0;
}
int
getlex()
{
    int c;
    char *p;
    while( (( 0 < (c = next()) ) * (  c < '!' )) )	 
	;
    if ((( c == -1 ) + ( 
	(( c == '(' ) + ( 
	(( c == ')' ) + ( 
        (( c == '[' ) + ( 
        (( c == ']' ) + ( 
        (( c == '{' ) + ( 
        (( c == '}' ) + ( 
        (( c == ',' ) + ( 
            c == ';' ))  ))  ))  ))  ))  ))  ))  )) )
	return c;
    if (c == '/') {
	if (thechar == '*') {
	    while((( next() != '*' ) + (  thechar != '/' )) )
		;
	    next();
	    return getlex();
	} else
	    return 1115 ;
    }
    if (c == '*') return 1114 ;
    if (c == '%') return 1116 ;
    if (c == '-')
	return gobble(c,1225 ,1013 );
    if (c == '<')
	return 808 ;
    if (c == '=')
	return gobble(c,806 ,101 );
    if (c == '+')
	return gobble(c,1219 ,1012 );
    if (c == '!')
	return gobble('=',807 ,1217 );
    if (c == '\'') {
	getstring(c);
	lexval = symbol[0];
	return 257 ;
    }
    if (c == '"') {
	getstring(c);
	return 258 ;
    }
    if (digit(c)) {
	lexval = c - '0';
	while(digit(thechar)) {
	    lexval = lexval * 10 + next() - '0';
	}
	return 257 ;
    }
	p = symbol;
	*p++ = c;
	while(letter(thechar))
	    *p++ = next();
	*p = 0;
	if ( (lexval = lookup(symbol)) < 7 ) {
	    if (lexval == 6 )
		return 517 ;
	    return lexval + 512 ;
	}
	return 256 ;
}
int 
istoken(int t)
{
    if (token == t) {
	token = getlex();
	return 1;
    }
    return 0;
}
int
type()
{
    istoken( 517  ) ;
    while(istoken(1114 ))
	;
}
int
name()
{
    int r;
    r = lexval;
    token = getlex();
    return r;
}
int
emit(int opc)
{
    code[curloc++] = opc;
}
int
emitat(int a, int c)
{
    code[a++] = c;
    code[a] = c/256;
}
int
emitop(int rator, int rand)
{
    int r;
    emit(rator+ 7 );
    r = curloc;
    emit(rand);
    emit(rand/256);
    return r;
}
int
pushloop(int puop, int max, int *arr)
{
    int i;
    i = 0;
    pushop = puop;
    while (i < max) {
	if (arr[i] == lexval) {
	    pusharg = i;
	    return 1;
	}
	i++;
    }
    return 0;
}
int
pushval()
{
    int lval;
    lval = 1;
    if (pushloop(64 , nlocal, localids)) {
    } else if (pushloop(32 , narg, argids)) {
	pusharg = narg-pusharg-1;
    } else if (pushloop(96 , nglob, globids)) {
	lval = globscalar[pusharg];
	pusharg = globoffs[pusharg];
    } else {
	lval = 0;
	if (pushloop(192 , nfun, funids)) {
	    pusharg = funoffs[pusharg];
	} else if (lexval < 9 ) {
	    pusharg = lexval- 7 ;
	}
    }
    emitop(pushop, pusharg);
    return lval;
}
int
pderef(int l)
{
    if (l)
	emit(23 );
}
int
expr(int needval, int prec)
{
    int na;
    int islval;
    int jdst;
    int op;
    int any;
    int opprec;
    islval = 0;
    if (istoken(257 )) {
	emitop(128 , lexval);
    } else if (istoken(258 )) {
	emitop(144 , strsize);
	any = 0;
	while(any < strsize)
	    emit(symbol[any++]);
    } else if (istoken(256 )) {
	islval = pushval();
    } else if (istoken('(')) {
	islval = expr(0, 0 );
	istoken( ')' ) ;
    } else if (istoken(1217 )) {
	expr(1, 100 );
	emit(17 );
    } else if (istoken(1013 )) {
	expr(1, 100 );
	emit(18 );
    } else if (istoken(1114 )) {
	expr(1, 100 );
	islval = 1;
    } 
    any = 1;
    while(any) {
	op = token % 100 ;
	if (istoken('(')) {
	    pderef(islval);
	    na = 0;
	    if (!istoken(')')) {
		do {
		    expr(1, 0 );
		    na++;
		} while(istoken(','));
		istoken( ')' ) ;
	    }
	    emitop(176 , na*2);
	    islval = 0;
	} else if (istoken('[')) {
	    pderef(islval);
	    expr(1, 0 );
	    emit(12 );
	    istoken( ']' ) ;
	    islval = 1;
	} else if (istoken(1219 )) {
	    emit(op);
	    islval = 0;
	} else if (istoken(1225 )) {
	    emit(op);
	    islval = 0;
	} else
	    any = 0;
    }
    opprec = token / 100 ;
    while (prec < opprec) {
	if ( (op = token % 100 ) != 1 ) {
	    pderef(islval);
	}
	{
	    token = getlex();
	    expr(1, opprec);
	    emit(op);
	}
	islval = 0;
	opprec = token / 100 ;
    }
    if (needval) {
	pderef(islval);
	islval = 0;
    }
    return islval;
}
int
pexpr()
{
    istoken( '(' ) ;
    expr(1, 0 );
    istoken( ')' ) ;
}
int
stmt()
{
    int jdest;
    int tst;
    if (istoken('{')) {
	while(!istoken('}'))
	    stmt();
    } else if (istoken(513 )) {
	pexpr();
	jdest = emitop (224 , 0);
	stmt();
	if (istoken(514 )) {
	    tst = emitop (208 , 0);
	    emitat(jdest, curloc);
	    stmt();
	    emitat(tst, curloc);
	} else {
	    emitat(jdest, curloc);
	}
    } else if (istoken(515 )) {
	tst = curloc;
	pexpr();
	jdest = emitop (224 , 0);
	stmt();
	emitop (208 , tst);
	emitat(jdest, curloc);
    } else if (istoken(516 )) {
	jdest = curloc;
	stmt();
	istoken( 515  ) ;
	pexpr();
	emit(17 );
	emitop (224 , jdest);
    } else if (istoken(512 )) {
	expr(1, 0 );
	istoken( ';' ) ;
	emit(10 );
    } else if (istoken(';')) {
    } else {
	expr(1, 0 );
	emit(22 );
	istoken( ';' ) ;
    }
}
int
parse()
{
    int objid;
    token = getlex();
    while(1) {
	if (token < 0)
	    return 1;
	type();			 
	objid = name();		 
	if (istoken('(')) {
	    funids[nfun] = objid;
	    funoffs[nfun++] = curloc;
	    narg = 0;
	    if (!istoken(')')) {
		do {
		    type();
		    argids[narg++] = name();
		} while(istoken(','));
		istoken( ')' ) ;
	    }
	    istoken( '{' ) ;
	    nlocal = 0;
	    while(token == 517 ) {
		type();
		do {
		    localids[nlocal++] = name();
		} while (istoken(','));
		istoken( ';' ) ;
	    }
	    while(!istoken('}'))
		stmt();
	    emit(10 );
	} else {
	    globoffs[nglob] = curgloboffs;
	    if (istoken('[')) {
		istoken( 257  ) ;
		curgloboffs = curgloboffs + lexval;
		istoken( ']' ) ;
		globscalar[nglob] = 0;
	    } else {
		curgloboffs++;
		globscalar[nglob] = 1;
	    }
	    globids[nglob++] = objid;
	    istoken( ';' ) ;
	}
    }
}
int
main(
    )
{
    int n;
    char *p;
    char *q;
    nsym = 9 ;
    p = names;
    q = "return\0if\0else\0while\0do\0int\0char\0getchar\0putchar" ;
    n = 48 ;
    do
	*p++ = *q++;
    while(n--);
    curloc = 7;	 
    thechar = getchar();
    parse();
    n = curloc;
    curloc = 0;
    lexval = lookup("main");
    pushval();
    emitop(176 , 0);
    emit(27 );
    putchar(0);
    putchar(0);
    putchar(n);
    putchar(n/256);
    p = code;
    while(n--) {
	putchar(*p++);
    }
    return 0;
}
