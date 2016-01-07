#!/bin/bash --
# by pts@fazekas.hu at Sat Jul 28 00:57:26 CEST 2001
set -xe
# checkergcc -g
# no -ansi, allow `inline' in unistd.h
# no -pedantic, because a lot of `inline' warnings
gcc -static -nostdlib -nostdinc -Wl,-z,-norelro,--build-id=none -s -fno-stack-protector -fomit-frame-pointer -mpreferred-stack-boundary=2 -falign-functions=1 -falign-jumps=1 -falign-loops=1 -Os -DCFG_LANG_ANSIC_LINUX=1 -W -Wall -Wstrict-prototypes -ansi -pedantic -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations muzcat.c || exit 5
rm -f test; time ./a.out <bash.file.gz >test ; cmp bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
echo OK
