#!/bin/bash --
# by pts@fazekas.hu at Tue Jul 24 20:27:35 CEST 2001
set -xe
cpp -DCFG_LANG_PERL=1 -E muzcat.c | grep -v '^#' | grep -v '^ *$' >zcat.pl
# perl zcat.pl
rm -f test; time perl zcat.pl <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time perl zcat.pl <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time perl zcat.pl <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
