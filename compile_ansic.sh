#!/bin/bash --
set -xe
gcc -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations muzcat.c || exit 5
rm -f test; time ./a.out <bash.file.zip >test ; cmp bash.file test
rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
