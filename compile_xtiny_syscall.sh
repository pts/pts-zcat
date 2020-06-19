#!/bin/sh --
set -ex
# Use -DCFG_LANG_ANSIC_LINUX for an even smaller binary output.
xtiny gcc -DCFG_LANG_ANSIC_SYSCALL -s -Os -W -Wall -Wextra -Werror -Wstrict-prototypes -o muzcat.xtiny muzcat.c
ls -ld muzcat.xtiny
cp -a muzcat.xtiny a.out
rm -f test; time ./a.out <testdata/bash.file.zip >test ; cmp testdata/bash.file test
rm -f test; time ./a.out <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time ./a.out <testdata/misc.file.gz >test ; cmp testdata/misc.file test
: compile_xtiny.sh OK.
