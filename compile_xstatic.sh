#!/bin/sh --
set -ex
xstatic gcc -DCFG_LANG_ANSIC_SYSCALL -m32 -fno-stack-protector -fomit-frame-pointer -mpreferred-stack-boundary=2 -falign-functions=1 -falign-jumps=1 -falign-loops=1 -W -Wall -s -Os -o muzcat muzcat.c
cp -a muzcat a.out
rm -f test; time ./a.out <testdata/bash.file.zip >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
: compile_xstatic.sh OK.
