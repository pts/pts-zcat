#!/bin/sh --
# by pts@fazekas.hu at Thu Jan  7 12:17:06 CET 2016
set -ex
cpp -DCFG_LANG_TURBOPASCAL=1 -E muzcat.c | perl -pe 'exit if/enddot2/' | grep -v '^#' | grep -v '^ *$' >zcat.pas
echo OK
