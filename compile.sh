#!/bin/sh --
set -ex
gcc -o muzcat -s -O3 -DNDEBUG=1 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations muzcat.c || exit 5
cp -a muzcat a.out
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
: OK, Created exacutable file: muzcat
