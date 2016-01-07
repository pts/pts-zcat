#!/bin/bash --
set -xe
cpp -DCFG_LANG_JAVA=1 -E muzcat.c | grep -v '^#' >ZCat.java
javac ZCat.java
rm -f test; time java ZCat <bash.file.gz >test ; cmp bash.file test
rm -f test; time java ZCat <random.file.gz >test ; cmp random.file test
rm -f test; time java ZCat <misc.file.gz >test ; cmp misc.file test
echo OK
