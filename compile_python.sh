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
rm -f test; time python zcat.py <bash.file.gz >test ; cmp bash.file test
rm -f test; time python zcat.py <random.file.gz >test ; cmp random.file test
rm -f test; time python zcat.py <misc.file.gz >test ; cmp misc.file test
echo OK
