#!/bin/bash --
set -xe
echo '#include <stdio.h>' >zcat.c
cpp -DCFG_LANG_OC=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >>zcat.c
# enddot2: avoid nasty CPP/GCC bug
# gcc -s -O3 -DCFG_LANG_ANSIC=1 -ansi -pedantic -W -Wall -Wstrict-prototypes -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations zcat.c || exit 5
gcc -s -O3 -DCFG_LANG_OC=1 -ansi -pedantic -W -Wall -Wtraditional -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Winline zcat.c || exit 5
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
