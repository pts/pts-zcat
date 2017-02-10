#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:17:06 CET 2016
set -ex
cpp -DCFG_LANG_XPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas
# Doesn't work, creates empty output file (`test').
ppc386 -O2 -vewnhi -Xs -oa.out zcat.pas
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
