// An unoptimized cat (stream copier) implementation in Google Go.
// by pts@fazekas.hu at Wed Nov 22 14:21:20 CET 2017
//
// $ go build cat.go && ./cat_new <testdata/misc.file >test.bin
// $ cmp testdata/misc.file test.bin
//
// $ GOOS=linux GOARCH=386 go build -ldflags '-s -w' cat_new.go
// (cat_new is 759584 bytes with go 1.9.2)
//

package main

import (
  "bufio";
  "io";
  "os";
)

// parts of stdio.h
var stdin  *bufio.Reader
var stdout *bufio.Writer
func init() {
  // glibc-2.3.6 has 8192-byte buffer as well
  stdin  = bufio.NewReaderSize(os.Stdin,  8192);
  stdout = bufio.NewWriterSize(os.Stdout, 8192);
}
func getchar() uint16 {
  c, err := stdin.ReadByte();
  if err == io.EOF { return 0xffff }
  if err != nil { panic("error reading stdin: " + err.Error()) }
  return uint16(c);
}
func putchar(b byte) {
  err := stdout.WriteByte(b);
  if err != nil { panic("error writing stdout: " + err.Error()) }
}

func main() {
  for {
    c := getchar()
    if c == 0xffff { break }
    putchar(byte(c));
  }
  stdout.Flush();
}
