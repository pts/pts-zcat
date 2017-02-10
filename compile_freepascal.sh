#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:13:51 CET 2016
set -xe
cpp -DCFG_LANG_FREEPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas
ppc386 -O2 -vewnhi -Xs -oa.out zcat.pas
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
