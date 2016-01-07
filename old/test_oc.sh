#!/bin/bash
set -xe
# cat august.c zcat.oc | ./august >zcat.oo
cat zcat.oc | ./parse >zcat.oo
# cat zcat.oo bash.gz | ./august >test
cat zcat.oo | ./august >test
ls -l test
hexdump -e '"%08_ax  " 4/1 "%02X " "  " 4/1 "%02X " "  " 4/1 "%02X " "  " 4/1 "%02X "' -e '"  " 16/1 "%_p" "\n"' test
