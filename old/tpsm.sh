#!/bin/bash --
#
# tpsm.sh
# by pts@fazekas.hu at Thu Dec 27 11:31:58 CET 2001
#

# CPPFLAGS=
CPPFLAGS=-DNDEBUG=1
# CPPFLAGS=-DDEBUGMSG=1

set -ex
<statecat.psm cpp $CPPFLAGS | perl -ne's@/\s+(?=\w)@/@g;print if!/^#/&&!/^\s*$/' |
perl -x ./pscompress.pl >statecat.ps
CMD="gs -q -dNODISPLAY -dSAFER -dBATCH statecat.ps"

rm -f test test2
cat random.file    random.file random.file >test2
time cat random.file.zip random.file.zip random.file.gz |
$CMD >test
cmp test test2

rm -f test; time $CMD <pts2.pbm.gz >test; cmp pts2.pbm test
rm -f test; time $CMD <bash.file.gz >test; cmp bash.file test


rm -f test; time $CMD <misc.file.gz >test; cmp misc.file test

# rm -f test; time $CMD <flate.txt.gz >test; cmp flate.txt test

echo "All OK."

