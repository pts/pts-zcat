#!/bin/bash --
#
# gen_seps.sh
# by pts@fazekas.hu at Fri Dec 28 00:08:01 CET 2001
#

# CPPFLAGS=
CPPFLAGS=-DNDEBUG=1
# CPPFLAGS=-DDEBUGMSG=1

set -ex
<statecat.psm cpp -DCFG_A85D=1 $CPPFLAGS |
perl -ne's@/\s+(?=\w)@/@g;print if!/^#/&&!/^\s*$/' |
perl -x ./pscompress.pl >seps.ps
CMD="gs -q -dNODISPLAY -dSAFER -dBATCH seps.ps"

rm -f test; time $CMD <pts2.pbm.gz.a85 >test; cmp pts2.pbm test
