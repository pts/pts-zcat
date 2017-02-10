#!/bin/bash --
set -xe
gcc -ansi -O -DZ=48000 -D'T=m[s]' -D'P=m[s++]' \
    -D'L=m[p++]' -D'g=getchar()' -DE=else -DW=while -D'B=m[p++]' \
    -DI=if -DR='s=s+l/2;T=r;I(l%2)s++' -D'X=m[s-' \
    -D'D=Q(13,-)Q(14,*)Q(15,/)Q(16,%)Q(6,==)Q(7,!=)Q(8,<)C(1,r=P;m[T]=r;T=r)C(9,r=P;m[T]=r;s++)' \
    august.c -o august
cpp -DCFG_LANG_OC=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.oc
# enddot2: avoid nasty CPP/GCC bug
cat august.c zcat.oc | ./august >zcat.oo
# cat zcat.oc | ./parse >zcat.oo
rm -f test; time cat zcat.oo testdata/bash.file.gz   | ./august >test ; cmp testdata/bash.file test
rm -f test; time cat zcat.oo testdata/random.file.gz | ./august >test ; cmp testdata/random.file test
rm -f test; time cat zcat.oo testdata/misc.file.gz   | ./august >test ; cmp testdata/misc.file test
echo OK
