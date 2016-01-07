#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:13:51 CET 2016
set -xe
cpp -DCFG_LANG_FREEPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas
ppc386 -O2 -vewnhi -Xs -oa.out zcat.pas
rm -f test; time ./a.out <bash.file.gz >test ; cmp bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
