#!/bin/bash --
# by pts@fazekas.hu at Sat Jul 28 13:32:03 CEST 2001
set -xe
cpp -DCFG_LANG_PYTHON=1 -E muzcat.c | grep -v '^#' | grep -v '^ *$' |
  perl -pe 's/;+\s*while/\nwhile/' |
  perl -ne ' s/^[ \t]*(\}[ \t]*)?/if(length$1){substr($D,-2,2)=""};$D/e;
  # s/^\s*/$D/;
  s/\s*([{}])[ \t]*/if($1eq"{"){$D.="  "}else{
  substr($D,-2,2)=""}""/ge; next if /\A[\s;]*\Z/;
  print
  ' >zcat.py
#  perl -pe 's/^\s+/$D/; s/([{}])[ \t]*/if($1eq"{"){$D.=".:"}else{
#  substr($D,-2,2)=""}""/ge' >zcat.py
# python zcat.py
# exit
rm -f test; time python zcat.py <testdata/bash.file.gz >test ; cmp testdata/bash.file test
rm -f test; time python zcat.py <testdata/random.file.gz >test ; cmp testdata/random.file test
rm -f test; time python zcat.py <testdata/misc.file.gz >test ; cmp testdata/misc.file test
echo OK
