#!/bin/bash --
set -xe
pts-tcc -Ipts-tcc-include -s -O2 -W -Wall -o muzcat muzcat_simple.c
rm -f test; time ./a.out <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
