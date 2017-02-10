#!/bin/bash --
# for the Digital Mars D language, signed
# by pts@fazekas.hu at Fri Feb 21 13:19:02 CET 2003
#
# Timings:
#   extracting pine.tar.gz (3784xxx bytes), user time:
#   Linux zcat              0.678 s
#   Linux muzcat            4.600 s
#   Win32 MinGW gcc muzcat  5.050 s
#   Digital Mars D 0.55     6.430 s

set -xe
gcc -E -DCFG_LANG_DMD_SIGNED=1 muzcat.c | grep -v '^#' | grep -v '^ *$' >zcat.d
# vvv untested
dmd -o a.out zcat.d
rm -f test; time ./a.out <testdata/bash.file.zip >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
