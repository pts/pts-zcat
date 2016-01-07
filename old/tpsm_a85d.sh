#!/bin/bash --
#
# tpsm_a85d.sh
# by pts@fazekas.hu at Sat Feb 23 18:12:36 CET 2002
#

# CPPFLAGS=
CPPFLAGS="-DCFG_A85D=1 -DNDEBUG=1"
# CPPFLAGS=-DDEBUGMSG=1

ASCII85Encode() {
 gs -dNODISPLAY -q -c '/i(%stdin)(r)file def/o(%stdout)(w)file/ASCII85Encode filter def/s 4096 string def{i s readstring exch o exch writestring not{exit}if}loop o closefile quit' "$@"
}
        
set -ex
<statecat.psm cpp $CPPFLAGS | perl -ne's@/\s+(?=\w)@/@g;print if!/^#/&&!/^\s*$/' |
perl -x ./pscompress.pl >statecat.ps
CMD="gs -q -dNODISPLAY -dSAFER -dBATCH statecat.ps"

rm -f test test2
cat random.file    random.file random.file >test2
time cat random.file.zip random.file.zip random.file.gz |
ASCII85Encode |
$CMD >test
cmp test test2

rm -f test; time ( <pts2.pbm.gz  ASCII85Encode | $CMD >test ); cmp pts2.pbm  test
rm -f test; time ( <bash.file.gz ASCII85Encode | $CMD >test ); cmp bash.file test
rm -f test; time ( <misc.file.gz ASCII85Encode | $CMD >test ); cmp misc.file test

#rm -f test; time $CMD <flate.txt.gz >test; cmp flate.txt test

echo "All OK."

