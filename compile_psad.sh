#!/bin/bash --
# by pts@fazekas.hu at Sun Nov 11 12:06:10 CET 2001
set -xe
# august.c, the bytecode of the OC compiler isn't good for us because we need
# the parsee.c that adds 20000 to the appropriate function addresses.
gcc parsee.c -o parsee -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations
gcc vm2ps.c -o vm2ps -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations
cpp -DCFG_LANG_OCPS=1 -E muzcat.c | perl -pe "exit if/enddot2/" | grep -v "^#" | grep -v '^ *$' >zcatad.ec
<zcatad.ec ./parsee | ./vm2ps | perl ./vm2ps2.pl >zcatad.psp
<zcatad.psp >zcatad.psr perl ps_peep.pl
(
  echo '/Codes ['
  cat zcatad.psr
  echo '] bind def'
  cat vm.ps
) >zcatad.ps
rm -f test; time gs -dNODISPLAY -q zcatad.ps <a.file.gz.a85 >test ; cmp a.file test
exit
# vvv tests not appropriate (need further decoding)
rm -f test; time gs -dNODISPLAY -q zcatad.ps <random.file.gz >test ; cmp random.file test
rm -f test; time gs -dNODISPLAY -q zcatad.ps <bash.file.gz   >test ; cmp bash.file test
rm -f test; time gs -dNODISPLAY -q zcatad.ps <misc.file.gz   >test ; cmp misc.file test
echo OK
