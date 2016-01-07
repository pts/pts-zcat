#! /bin/bash --
#
# test.sh
# by pts@fazekas.hu at Sat Jun 29 17:49:44 CEST 2002
#

TMP=test.tmp
trap 'rm -f "$TMP"' EXIT

if [ "$#" != 1 ]; then
  echo "Usage: $0 <command-line>"
  exit 3
fi

BAD=0
for FGZ in tests/random.file.gz; do
#for FGZ in tests/*.gz; do
  F="${FGZ%.gz}"
  rm -f "$TMP"
  echo "<$FGZ $1 >$TMP"
  time eval "$1" <"$FGZ" >"$TMP" || BAD=1
  # ^^^ Dat: eval allows pipes etc.
  cmp "$F" "$TMP" || BAD=1
done

[ "$BAD" = 0 ] || trap '' EXIT
exit "$BAD"
