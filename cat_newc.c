/* by pts@fazekas.hu at Wed Nov 22 14:29:08 CET 2017
 *
 * $ xstatic gcc -s -Os -ansi -pedantic -W -Wall -Wextra -Werror -o cat_newc cat_newc.c
 * (cat_newc is 12604 bytes)
 * $ ./cat_newc <testdata/misc.file >test.bin
 * $ cmp testdata/misc.file test.bin 
 */
#include <stdio.h>

int main(int argc, char **argv) {
  int c;
  (void)argc; (void)argv;
  for (; (c = getchar()) >= 0; putchar(c)) {}
  return 0;
}
