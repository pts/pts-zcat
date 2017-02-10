#!/bin/sh --
set -ex
xstatic gcc -DCFG_LANG_ANSIC_SYSCALL -m32 -fno-stack-protector -fomit-frame-pointer -mpreferred-stack-boundary=2 -falign-functions=1 -falign-jumps=1 -falign-loops=1 -W -Wall -s -Os -o muzcat muzcat.c
cp -a muzcat a.out
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
#rm -f test; time ./a.out <random.file.gz >test ; cmp random.file test
#rm -f test; time ./a.out <misc.file.gz >test ; cmp misc.file test
: compile_xstatic.sh OK.
