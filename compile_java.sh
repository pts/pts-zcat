#!/bin/bash --
set -xe
cpp -DCFG_LANG_JAVA=1 -E muzcat.c | grep -v '^#' >ZCat.java
javac ZCat.java
rm -f test; time java ZCat <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time java ZCat <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time java ZCat <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
