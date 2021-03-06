#!/bin/bash --
# by pts@fazekas.hu at Sat Jul 28 00:57:26 CEST 2001
set -xe
# If your (old) gcc doesn't support -m32, then drop the flag.
gcc -m32 -static -nostdlib -nostdinc -Wl,-z,-norelro,--build-id=none -s -fno-stack-protector -fomit-frame-pointer -mpreferred-stack-boundary=2 -falign-functions=1 -falign-jumps=1 -falign-loops=1 -Os -DCFG_LANG_ANSIC_LINUX=1 -W -Wall -Wstrict-prototypes -ansi -pedantic -Wnested-externs -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wmissing-prototypes -Winline -Wmissing-declarations muzcat.c || exit 5
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
