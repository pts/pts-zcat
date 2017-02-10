#!/bin/bash --
# by pts@fazekas.hu at Sun Jul 29 11:29:49 CEST 2001
# compression by pts@fazekas.hu at Sat Nov 10 12:53:14 CET 2001
# tiny by pts@fazekas.hu at Sun Nov 11 23:40:13 CET 2001
set -xe
# august.c, the bytecode of the OC compiler isn't good for us because we need
# the parsee.c that adds 20000 to the appropriate function addresses.
gcc parsee.c -o parsee -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations
gcc vm2ps.c -o vm2ps -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations
# cpp -DCFG_LANG_OCPS=1 -E muzcat.c.29 | perl -pe "exit if/enddot2/" | grep -v "^#" | grep -v '^ *$' >zcat.ec1
cpp -DCFG_LANG_OCPS=1 -E muzcat.c | perl -pe "exit if/enddot2/" | grep -v "^#" | grep -v '^ *$' >zcat.ec
<zcat.ec ./parsee | ./vm2ps | perl ./vm2ps2.pl >zcat.psp
# vvv minimal compression
# <zcat.psp >zcat.psr perl -e '$_=join"",<>;s/%.*//g;s/\s+/ /g;s/ *([{}]) */$1/g;print'
# vvv ps_peep.pl is mandatory because it handles emudiv
<zcat.psp >zcat.psr perl ps_peep.pl --tiny
(
  echo -n '['
  cat zcat.psr
  echo -n ']'
  cat vmtiny.ps
) >zcattiny.ps
rm -f test; time gs -dNODISPLAY -q zcattiny.ps <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time gs -dNODISPLAY -q zcattiny.ps <testdata/bash.file.gz   >test ; cmp testdata/bash.file test
rm -f test; time gs -dNODISPLAY -q zcattiny.ps <testdata/misc.file.gz   >test ; cmp testdata/misc.file test
echo OK
