#!/bin/bash --
# by pts@fazekas.hu at Sat Jul 28 13:03:09 CEST 2001
set -xe
cpp -DCFG_LANG_RUBY=1 -E muzcat.c | grep -v '^#' | grep -v '^ *$' |
  ruby -pe '$_.gsub!(/\$ */,"\$")' >zcat.rb
# ruby -w zcat.rb
rm -f test; time ruby -w zcat.rb <bash.file.gz >test ; cmp bash.file test
rm -f test; time ruby -w zcat.rb <random.file.gz >test ; cmp random.file test
rm -f test; time ruby -w zcat.rb <misc.file.gz >test ; cmp misc.file test
echo OK
