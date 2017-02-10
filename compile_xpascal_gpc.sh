#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:17:06 CET 2016
set -ex
cpp -DCFG_LANG_XPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas
# --extended-pascal almost works (instead of --borland-pascal), except that
# --extended-pascal doesn't support assign.
gpc zcat.pas -O3 -s -Wall -W -Wpointer-arith -Wmissing-prototypes -Wmissing-declarations -Winline --borland-pascal -oa.out
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
