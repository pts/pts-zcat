#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:17:06 CET 2016
set -ex
cpp -DCFG_LANG_ISOPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas

# Compiles fine but doesn't work because of reading \0
# gpc zcat.pas -O3 -s -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Winline --pedantic --standard-pascal-level-0 -oa.out

# This also doesn't work with Free Pascal 2.6, because it can't read binary
# data from stdin like this:
#   if eoln then begin readln; getchar:=10; end
# Use compile_freepascal.sh to make it work with Free Pascal.
# /usr/lib/fpc/0.99.10/
ppc386 -O2 -vewnhi -Xs -oa.out zcat.pas
rm -f test; time ./a.out <bash.file.gz >test ; cmp bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
