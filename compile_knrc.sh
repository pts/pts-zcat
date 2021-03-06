#!/bin/bash --
set -xe
gcc -s -O3 -DCFG_LANG_KNRC=1 -traditional -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations muzcat.c || exit 5
# ^^^ GCC 2.7.2.3 (on Debian Slink) prints a warning in /usr/local/include/sys/cdefs.h
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
