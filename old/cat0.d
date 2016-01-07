// cat0.d
// cannnot set binary mode :-(
import c.stdio;

extern (C) int setmode(int fd, int mode);
const int my_O_BINARY=0x8000;

extern (C) uint CRT_fmode = my_O_BINARY; // no effect
extern (C) uint _CRT_fmode = my_O_BINARY; // no effect
extern (C) uint _fmode = my_O_BINARY; // no effect
extern (C) uint fmode = my_O_BINARY; // no effect

int main (char[][] args) {
  int i;
  _CRT_fmode=_fmode=CRT_fmode=fmode=my_O_BINARY;
  setmode(0, my_O_BINARY); // no effect :-(
  setmode(1, my_O_BINARY); // no effect :-(
  while (0<=(i=fgetc(stdin))) fputc(i,stdout);
  return 0;
}
