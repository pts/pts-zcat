/*
 * zcats.c: simple stdin-to-stdout decompression filter implementation of the gzip, zlib, Deflate and Quasijarus strong file formats file formats in ANSI C (C89) and C++98
 * by pts@fazekas.hu at Fri Sep 26 00:32:20 CEST 2025
 *
 * Compile for Unix with GCC, for decompressing gzip and raw Deflate: gcc -s -O2 -W -Wall -Werror -ansi -pedantic -o zcats zcats.c
 * Compile for Unix with GCC, for decompressing gzip and zlib: gcc -s -O2 -W -Wall -Werror -ansi -pedantic -DUSE_ZLIB -o zlcats zcats.c
 * Compile for Linux i386 and FreeBSD i386 with https://github.com/pts/minilibc686 : minicc -bfreebsdx -ansi -pedantic -o zcats.mini zcats.c
 * Compile to tiny DOS 8086 .com with OpenWatcom: wcc -q -bt=com -D_DOSCOMSTART -os -zl -j -ms -s -W -w4 -wx -we -wcd=201 -za -oi -0 -g=DGROUP -fo=.o zcats.c && wlink op q form dos com op d op nored op start=_comstart_ n zcatsc.com f zcats.o
 * Compile to DOS 8086 .com with OpenWatcom: owcc -bcom -s -Os -fno-stack-check -march=i086 -W -Wall -Wextra -Werror -std=c89 -o zcatsd.com zcats.c
 * Compile to DOS 8086 .exe with OpenWatcom: owcc -bdos -mcmodel=s -s -Os -mstack-size=0x300 -fno-stack-check -march=i086 -W -Wall -Wextra -Werror -std=c89 -o zcatsd.exe zcats.c
 * Compile to DOS 8086 .exe with Turbo C++ 1.x: tcc.exe -ms -O -f- -a -w -w-rch -ezcatsd.exe zcats.c
 * Compile for Win32: owcc -bwin32 -Wl,runtime -Wl,console=3.10 -s -Os -fno-stack-check -march=i386 -W -Wall -Wextra -Werror -Wno-n201 -std=c89 -o zcatsw.exe zcats.c
 * Compile for Win32 with https://github.com/pts/bakefat mmlibc386: mmlibcc.sh -bwin32 -o zcats.exe zcats.c
 * It can't be compiled with Microsoft C compiler 6.00a for DOS 8086, because identifier lengths are longer than 31 bytes.
 *
 * This program does the same as the zcat(1) program in gzip, except for these:
 *
 * * This program ignores its command-line arguments.
 * * The error conditions are the same, but the error messages are different.
 * * The set of supported input file formats are different (but both
 *   programs support the gzip file format).
 * * The set of gzip flags supported is different.
 * * This program doesn't compute or check the CRC-32 and Adler-32 checksums
 *   of the uncompressed output data.
 * * This program doesn't flush the write buffer on error.
 * * This program stops at the end of the first Deflate stream.
 *
 * This program does the same as
 * https://github.com/pts/pts-zcat/blob/master/muzcat.c , except for the
 * following functionality differences:
 *
 * * This program has stricter error checking and thus memory-safe operation.
 * * The set of supported input file formats are different (but both
 *   programs support the gzip file format).
 * * The set of gzip flags supported is different.
 * * This program stops reading its input after the end of the first Deflate
 *   stream.
 * * This program uses a bit less memory.
 * * This program stops at the end of the first Deflate stream.
 *
 * C compiler configuration options (`#ifdef`s):
 *
 * * `-DUSE_DEFLATE` (default): As input, support the
 *   [gzip](https://www.rfc-editor.org/rfc/rfc1952.txt), [raw
 *   Deflate](https://www.rfc-editor.org/rfc/rfc1951.txt) and
 *   [4.3BSD-Quasijarus strong
 *   compression](http://fileformats.archiveteam.org/wiki/Quasijarus_Strong_Compression)
 *   file formats. It doesn't support zlib, and conflicts with
 *   `-DUSE_ZLIB`.
 * * `-DUSE_ZLIB`: As input, support the
 *   [gzip](https://www.rfc-editor.org/rfc/rfc1952.txt),
 *   [zlib](https://www.rfc-editor.org/rfc/rfc1950.txt) and
 *   [4.3BSD-Quasijarus strong
 *   compression](http://fileformats.archiveteam.org/wiki/Quasijarus_Strong_Compression)
 *   file formats. It doesn't support raw Deflate, and conflicts with
 *   `-DUSE_DEFLATE`.
 * * `-DUSE_NZTREELEAF`: use a non-zero `TREE_LEAF` value; this helps
 *   debugging of the Huffman tree builder.
 */

#ifdef USE_DEFLATE
#  ifdef USE_ZLIB
#    error Inconsistent use of -DUSE_ZLIB and -DUSE_DEFLATE.
#  endif
#else
#  ifndef USE_ZLIB
#    define USE_DEFLATE 1  /* Default. */
#  endif
#endif

#if defined(_DOSCOMSTART) && !defined(LIBC_PREINCLUDED)
  /* Tiny libc for OpenWatcom C compiler creating a DOS 8086 .com program.
   * For compilation instructions, see _DOSCOMSTART above.
   */
#  define LIBC_PREINCLUDED 1
  /* This must be put to a separate function than _comstart, otherwise wlink
   * wouldn't remove these 0x100 bytes from the beginning of the DOS .com
   * program.
   */
  __declspec(naked) void __watcall _nuls(void) { __asm { db 100h dup (?) } }

  extern char _edata[], _end[];
  extern void __watcall main0(void);
  /* It must not modify `bp', otherwise the OpenWatcom C compiler omits the
   * `pop bp' between the `mov sp, bp' and the `ret'.
   */
#  pragma aux main0 __modify [__ax __bx __cx __dx __si __di __es]

  /* This entry point function must be the first function defined in the .c
   * file, because the entry point of DOS .com programs is at the beginning.
   *
   * This function zero-initializes _BSS, then jumps to main0. When main0
   * returns, it will do a successful exit(0). If _BSS is missing (no matter
   * how long _DATA is), then both _end and _edata are NULL here, but that's
   * fine here.
   */
  __declspec(naked) void __watcall _comstart(void) {
      __asm { xor ax, ax }
      __asm { mov di, offset _edata } __asm { mov cx, offset _end }
      __asm { sub cx, di } __asm { rep stosb } __asm { jmp main0 } }

  /* At the end, the `ret' instruction will exit(0) the DOS .com program. */
#  define main0() void __watcall main0(void)
#  define main0_exit0() do {} while (0)
#  define main0_exit(exit_code) _exit(exit_code)

#  define LIBC_HAVE_WRITE_NONZERO_VOID 1
  /* Like write(...), but count must be nonzero, and it returns void. */
  static void write_nonzero_void(int fd, const void *buf, unsigned int count);
#  pragma aux write_nonzero_void = "mov ah, 40h"  "int 21h" \
      __parm [__bx] [__dx] [__cx] __modify [__ax]

#  define LIBC_HAVE_WRITE_NONZERO 1
  /* Like write(...), but count must be nonzero. */
  int write_nonzero(int fd, const void *buf, unsigned int count);
#  pragma aux write_nonzero = "mov ah, 40h"  "int 21h"  "jnc ok"  \
      "mov ax, -1"  "ok:"  \
      __value [__ax] __parm [__bx] [__dx] [__cx] __modify __exact [__ax]

  int write(int fd, const void *buf, unsigned count);
#  pragma aux write = "xor ax, ax"  "jcxz ok"  "mov ah, 40h"  "int 21h" \
      "jnc ok"  "mov ax, -1"  "ok:"  \
      __value [__ax] __parm [__bx] [__dx] [__cx] __modify __exact [__ax]

  int read(int fd, void *buf, unsigned int count);
#  pragma aux read = "mov ah, 3fh"  "int 21h"  "jnc ok"  "mov ax, -1"  "ok:" \
      __value [__ax] __parm [__bx] [__dx] [__cx] __modify __exact [__ax]

  __declspec(noreturn) void exit(unsigned char exit_code);
#  pragma aux exit = "mov ah, 4ch"  "int 21h" \
      __parm [__al] __modify __exact []

  unsigned int strlen(const char *s);
  /* To prevent wlink: Error! E2028: strlen_ is an undefined reference */
#  pragma intrinsic(strlen)  /* !! Add shorter `#pragma aux' or static. */
  void *memset(void *s, int c, unsigned int n);
#  pragma intrinsic(memset)  /* !! Add shorter `#pragma aux' or static. */

#  define STRLIT_NL "\r\n"  /* Line ending for error messages on stderr. */
#endif

#ifndef LIBC_PREINCLUDED
#  ifdef __MMLIBC386__
#    include <mmlibc386.h>
#  else
#    include <stdlib.h>  /* exit(...). */
#    include <string.h>
    /* Turbo C++ 1.01 defines __MSDOS__. OpenWatcom C compiler defines __DOS__, __NT__ or __OS2__. */
#    if defined(MSDOS) || defined(_MSDOS) || defined(_WIN32) || defined(_WIN64) || defined(__DOS__) || defined(__COM__) || defined(__NT__) || defined(__MSDOS__) || defined(__OS2__)
#      include <fcntl.h>  /* O_BINARY. */
#      include <io.h>  /* read(...), write(...), setmode(...). */
#    else
#      include <unistd.h>  /* read(...), write(...). */
#    endif
#  endif
#  define STRLIT_NL "\n"  /* On DOS etc. write(STDERR_FILENO) will append an "\r" because the lack of O_BINARY. */
#endif

#if defined(__TURBOC__) && defined(__MSDOS__)
  unsigned _stklen = 0x300;  /* Global variable used by Turbo C++ 1.x libc _start. */
#endif

#ifndef   STDIN_FILENO
#  define STDIN_FILENO 0
#endif

#ifndef   STDOUT_FILENO
#  define STDOUT_FILENO 1
#endif

#ifndef   STDERR_FILENO
#  define STDERR_FILENO 2
#endif

#ifndef   EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#endif

#ifndef   EXIT_FAILURE
#  define EXIT_FAILURE 1
#endif

#ifdef __TURBOC__  /* Turbo C++ 1.01 emits a subspicious pointer conversion warning without the (char*) cast. */
#  define WRITE_PTR_ARG_CAST(arg) ((char*)(arg))
#else
#  define WRITE_PTR_ARG_CAST(arg) ((const char*)(arg))
#endif

#ifdef __noreturn
#else
#  ifdef __GNUC__
#    define __noreturn __attribute__((__noreturn__))
#  else
#    ifdef __WATCOMC__
#      define __noreturn __declspec(noreturn)
#    else
#      define __noreturn
#    endif
#  endif
#endif

#ifndef LIBC_HAVE_WRITE_NONZERO_VOID
#  define write_nonzero_void(fd, buf, count) (void)!write(fd, buf, count)
#endif

#ifndef LIBC_HAVE_WRITE_NONZERO
#  define write_nonzero(fd, buf, count) write(fd, buf, count)
#endif

/* --- */

#define FATAL_READ 0
#define FATAL_WRITE 1
#define FATAL_CORRUPTED_INPUT 2

/* We use `unsigned int' for a value which is at least 16 bits, and calculations are fast with it (i.e. faster than with unsigned short). */
typedef unsigned char uc8;  /* Always a single byte. For interfacing with read(2) and write(2). */
typedef unsigned char um8;  /* Unsigned integer type which is at least 8 bits, preferably 8 bits. */
typedef unsigned char ub8;  /* Unsigned integer type which is at least 1 bit (boolean), preferably 8 bits. */
typedef unsigned short um16;  /* Unsigned integer type which is at least 16 bits, preferably 16 bits. */
typedef char assert_sizeof_uc8[sizeof(uc8) == 1 ? 1 : -1];  /* We also rely on the implicit `& 0xff' behavior of global_bitbuf for USE_TREE. */
typedef char assert_sizeof_um16[sizeof(um16) >= 2 ? 1 : -1];
typedef um8 huffman_bit_count_t;

#define MAX_TMP_SIZE 19
#define MAX_INCOMPLETE_BRANCH_COUNT (7 + 15 + 15)  /* Because of DO_CHECK_HUFFMAN_TREE_100_PERCENT_COVERAGE == 1. */
#define MAX_LITERAL_AND_LEN_SIZE (0x100 + 32)
#define MAX_DISTANCE_SIZE 30
#define MAX_TREE_SIZE ((6U - (3 << 2)) + ((MAX_TMP_SIZE + MAX_LITERAL_AND_LEN_SIZE + MAX_DISTANCE_SIZE) << 1 << 1) + (MAX_INCOMPLETE_BRANCH_COUNT << 2))  /* <<1 is for the max node_count/leaf_node_count for a tree. <<1 is for node_count/element_count. */

#define TREE_TMP_ROOT_IDX 0
#define TREE_LITERAL_AND_LEN_ROOT_IDX 2
#define TREE_DISTANCE_ROOT_IDX 4
#define TREE_FIRST_FREE_IDX 6
#if USE_NZTREELEAF
#  define LEAF_IDX 0xfaceU  /* Can be 0 or someting large (e.g. 0xffffU or a bit less). */
#else
#  define LEAF_IDX 0U
#endif
#define BAD_LEAF_VALUE ((um16)-1)

static const char * const global_error_msgs[3] = { "read error" STRLIT_NL, "write error" STRLIT_NL, "corrupted input" STRLIT_NL };

static uc8 global_write_buffer[0x8000U];
static uc8 global_read_buffer[0x2000];
/* Each node in this Huffman tree occupies two slots:
 * global_huffman_trees_ary[node_idx] and global_huffman_trees_ary[node_idx
 * + 1]. If global_huffman_trees_ary[node_idx] == LEAF_IDX, then this is a
 * leaf node, and the value is in global_huffman_trees_ary[node_idx + 1]
 * (with special invalid value BAD_LEAF_VALUE) indicating an invalud Huffman
 * code. Otherwise this is a non-leaf node with two children:
 * global_huffman_trees_ary[node_idx] is the new node_idx of child 0, and
 * global_huffman_trees_ary[node_idx + 1] is the new node_idx of child 1.
 */
static um16 global_huffman_trees_ary[MAX_TREE_SIZE];
static huffman_bit_count_t global_huffman_bit_count_ary[0x100 + 32 + 30];  /* [288]. The tokens 286 and 287 are placeholders needed by correct fixed Huffman codes. */

static unsigned int global_read_idx;
static unsigned int global_read_buffer_remaining;
static um8 global_bitbuf_bits_remaining;
static uc8 global_bitbuf;
static unsigned int global_tree_free_idx;

static __noreturn void fatal(int error_code) {
  const char *msg = global_error_msgs[error_code];
  write_nonzero_void(STDERR_FILENO, WRITE_PTR_ARG_CAST(msg), strlen(msg));
  exit(EXIT_FAILURE);
}

static um8 read_byte(void) {
  int got;
  if (global_read_buffer_remaining-- == 0) {
    if ((got = read(STDIN_FILENO, (char*)global_read_buffer, (int)sizeof(global_read_buffer))) < 0) fatal(FATAL_READ);
    if (got == 0) {
      fatal(FATAL_CORRUPTED_INPUT);
    }
    global_read_buffer_remaining = --got;
    global_read_idx = 0;
  }
  return global_read_buffer[global_read_idx++];
}

/* 0 <= bit_count <= 8. */
static unsigned int read_bits_max_8(um8 bit_count) {
  unsigned int result;
  if (global_bitbuf_bits_remaining < bit_count) {
    result = global_bitbuf;
    global_bitbuf = read_byte();
    bit_count -= global_bitbuf_bits_remaining;
    result |= (global_bitbuf & ((1U << bit_count) - 1)) << global_bitbuf_bits_remaining;
    global_bitbuf_bits_remaining = 8 - bit_count;
    global_bitbuf >>= bit_count;
  } else {
    result = global_bitbuf & ((1U << bit_count) - 1);
    global_bitbuf >>= bit_count;
    global_bitbuf_bits_remaining -= bit_count;
  }
  return result;
}

/* 0 <= bit_count <= 16. */
static unsigned int read_bits_max_16(um8 bit_count) {
  unsigned int result;
  if (bit_count > 8) {
    result = read_bits_max_8(8);
    result += read_bits_max_8(bit_count - 8) << 8;
  } else {
    result = read_bits_max_8(bit_count);
  }
  return result;
}

static void flush_write_buffer(unsigned int size) {
  unsigned int size_i;
  int got;
  for (size_i = 0; size_i < size; ) {
    got = (int)(size - size_i);
    if (sizeof(int) == 2 && got < 0) got = 0x4000;
    if ((got = write_nonzero(STDOUT_FILENO, WRITE_PTR_ARG_CAST(global_write_buffer + size_i), got)) <= 0) fatal(FATAL_WRITE);
    size_i += got;
  }
}

static uc8 *write_byte(uc8 *out, uc8 b) {
  if (out == global_write_buffer + sizeof(global_write_buffer)) {
    flush_write_buffer(sizeof(global_write_buffer));
    out = global_write_buffer;
  }
  *out++ = b;
  return out;
}

static um16 read_using_huffman_tree(unsigned int node_idx) {
  while (global_huffman_trees_ary[node_idx] != LEAF_IDX) {
    if (global_bitbuf_bits_remaining-- == 0) {
      global_bitbuf = read_byte();
      global_bitbuf_bits_remaining = 7;
    }
    if ((global_bitbuf & 1) != 0) ++node_idx;
    global_bitbuf >>= 1;
    node_idx = global_huffman_trees_ary[node_idx];
  }
  return global_huffman_trees_ary[node_idx + 1];
}

/* 1 <= size <= 318. */
static void read_bit_count_ary(huffman_bit_count_t *bit_count_ary_ptr, unsigned int size) {
  unsigned int count;
  unsigned int value;
  const unsigned int orig_size = size;
  while (size) {
    value = read_using_huffman_tree(TREE_TMP_ROOT_IDX);
    if (value > 18) goto corrupted_input;  /* For value == BAD_LEAF_VALUE. */
    if (value < 16) {
      *bit_count_ary_ptr++ = value;
      --size;
    } else if (value == 16) {
      if (size == orig_size) { corrupted_input: fatal(FATAL_CORRUPTED_INPUT); }
      value = bit_count_ary_ptr[-1];
      count = read_bits_max_8(2) + 3;
     append_many:
      if (count > size) goto corrupted_input;
      size -= count;
      while (count-- != 0) {
        *bit_count_ary_ptr++ = value;
      }
    } else if (value == 17) {
      count = read_bits_max_8(3) + 3;
     set_value_to_missing:
      value = 0;
      goto append_many;
    } else {
      count = read_bits_max_8(7) + 11;
      goto set_value_to_missing;
    }
  }
}

/* 1 <= size <= 288. */
static void build_huffman_tree(const huffman_bit_count_t *bit_count_ary_ptr, unsigned int size, unsigned int root_idx) {
  unsigned int size_i, bit_count;
  um16 bit_count_histogram_and_g_ary[16];
  um16 *bit_count_histogram_and_g_ptr;
  unsigned int total_g;
  unsigned int node_idx;
  um16 code, code_mask_i;
  memset(bit_count_histogram_and_g_ary, 0, sizeof(bit_count_histogram_and_g_ary));
  for (size_i = size; size_i-- != 0; ) {
    ++bit_count_histogram_and_g_ary[bit_count_ary_ptr[size_i]];
  }
  bit_count_histogram_and_g_ptr = bit_count_histogram_and_g_ary;
  *bit_count_histogram_and_g_ptr++ = total_g = 0;
  do {
    if (total_g > 0x4000U) { corrupted_input: fatal(FATAL_CORRUPTED_INPUT); }
    total_g <<= 1;
    total_g += *bit_count_histogram_and_g_ptr;
    *bit_count_histogram_and_g_ptr++ = total_g << 1;
  } while (bit_count_histogram_and_g_ptr != bit_count_histogram_and_g_ary + 16);
  for (size_i = 0; size_i < size; ++size_i) {
    if ((bit_count = bit_count_ary_ptr[size_i]) == 0) continue;
    node_idx = root_idx;
    --bit_count;
    code = bit_count_histogram_and_g_ary[bit_count]++;
    code_mask_i = 1U << bit_count;
    do {
      if (global_huffman_trees_ary[node_idx] == LEAF_IDX) {
        global_huffman_trees_ary[node_idx] = global_tree_free_idx;
        global_huffman_trees_ary[global_tree_free_idx++] = LEAF_IDX;
        ++global_tree_free_idx;  /* Skip setting the value of child 0 to BAD_LEAF_VALUE, because the next iteration in the first descent will overwrite it anyway. */
        global_huffman_trees_ary[node_idx + 1] = global_tree_free_idx;
        global_huffman_trees_ary[global_tree_free_idx++] = LEAF_IDX;
        global_huffman_trees_ary[global_tree_free_idx++] = BAD_LEAF_VALUE;
      }
      if ((code & code_mask_i) != 0) ++node_idx;
      node_idx = global_huffman_trees_ary[node_idx];
    } while ((code_mask_i >>= 1) != 0);
    global_huffman_trees_ary[node_idx + 1] = size_i;
  }
  if (global_huffman_trees_ary[root_idx] == LEAF_IDX) goto corrupted_input;  /* Fail (and avoid infinite loop) if all bit_count values are 0. */
}

#ifndef main0
#  define main0() int main(void)
#  define main0_exit0() return EXIT_SUCCESS
#endif

main0() {
  uc8 b, flg;
  unsigned int i;
  uc8 *out;
  ub8 bfinal;
  unsigned int uncompressed_block_size;
  unsigned int literal_and_len_size;
  unsigned int distance_size;
  unsigned int tmp_size;
  unsigned int size_i;
  unsigned int token;
  unsigned int match_length;
  unsigned int match_distance_extra_bits;
  unsigned int match_distance;
  unsigned int match_distance_limit;
  unsigned int prev_idx;
  huffman_bit_count_t tmp_bit_count_ary[19];

#if O_BINARY  /* For DOS, Windows (Win32 and Win64) and OS/2. */
  setmode(STDIN_FILENO, O_BINARY);
  setmode(STDOUT_FILENO, O_BINARY);  /* Prevent writing (in write(2)) LF as CRLF on DOS, Windows (Win32) and OS/2. */
#endif
  if ((b = read_byte()) == 0x1f) {  /* gzip: https://www.rfc-editor.org/rfc/rfc1952.txt */
    /* First byte (b) is ID1, must be 0x1f. */
    if ((b = read_byte()) == 0xa1) goto end_of_header;  /* 4.3BSD-Quasijarus strong compression (compress -s) produces the 0x1f 0xa1 header and then raw Deflate. */
    if (b != 0x8b) fatal(FATAL_CORRUPTED_INPUT);  /* ID2 byte, must be 0x8b. */
    if ((b = read_byte()) != 8) fatal(FATAL_CORRUPTED_INPUT);  /* CM byte. Check that CM == 8. */
    flg = read_byte();  /* FLG byte. */
    for (i = 6; i-- != 0; ) {
      read_byte();  /* Ignore MTIME (4 bytes), XFL (1 byte) and OS (1 byte). */
    }
    if ((flg & 2) != 0) fatal(FATAL_CORRUPTED_INPUT);  /* The FHCRC enables GCONT (2-byte continuation part number) here for gzip(1), and header CRC16 later for https://www.rfc-editor.org/rfc/rfc1950.txt . We just fail if it's set. */
    if ((flg & 4) != 0) {  /* FEXTRA. Ignore the extra data. */
      for (i = read_bits_max_16(16); i-- != 0; ) {  /* Little-endian. It's OK to overlap bit reads with byte reads here, because we remain on byte boundary. */
        read_byte();
      }
    }
    if ((flg & 8) != 0) {  /* FNAME. Ignore the filename. */
      while (read_byte() != 0) {}
    }
    if ((flg & 16) != 0) {  /* FCOMMENT. Ignore the comment. */
      while (read_byte() != 0) {}
    }
    /* Now we could ignore the 2-byte CRC16 if (flg & 2), but we've disallowed it above. */
    if ((flg & 32) != 0) {  /* Ignore gzip(1) the encryption header. */
      for (i = 12; i-- != 0; ) {
        read_byte();
      }
    }
   end_of_header: ;
  } else {
#ifdef USE_ZLIB  /* zlib: https://www.rfc-editor.org/rfc/rfc1950.txt */
    if ((b & 0xf) != 8 || (b >> 4) > 7) fatal(FATAL_CORRUPTED_INPUT);  /* CM byte. Check that CM == 8, check that CINFO <= 7, otherwise ignore CINFO (sliding window size). */
    if (((b = read_byte()) & 0x20)) fatal(FATAL_CORRUPTED_INPUT);  /* FLG byte. Check that FDICT == 0. Ignore FLEVEL and FCHECK. */
#else
    ++global_read_buffer_remaining;
#  if 1  /* Shorter, b is already in global_read_buffer. */
    --global_read_idx;
#  else
    global_read_buffer[--global_read_idx] = b;  /* Put it back (ungetc(b)) for the Deflate decompressor. */
#  endif
#endif
  }
  out = global_write_buffer;
  match_distance_limit = 0;
  do {  /* Decompress next block. */
    bfinal = read_bits_max_8(1);  /* BFINAL. */  /* This would be the only caller of prog_decompress_read_bit(). We don't waste code size on the short implementation. */
    switch (read_bits_max_8(2)) {  /* BTYPE. */
     case 0:  /* Uncompressed block. */
      global_bitbuf_bits_remaining = 0;  /* Discard partially read byte. */
      uncompressed_block_size = read_bits_max_16(16);  /* LEN. */
      if (read_bits_max_16(16) != ((um16)~uncompressed_block_size & 0xffffU)) { corrupted_input: fatal(FATAL_CORRUPTED_INPUT); }  /* NLEN. */
      if (uncompressed_block_size >= 0x8000U || ((match_distance_limit += uncompressed_block_size) >= 0x8000U)) match_distance_limit = 0x8000U;
      while (uncompressed_block_size-- != 0) {
        out = write_byte(out, read_byte());
      }
      break;
     case 1:  /* Block compressed with fixed Huffman codes. */
      literal_and_len_size = 288;
      for (size_i = 0; size_i < 144; global_huffman_bit_count_ary[size_i++] = 8) {}
      for (; size_i < 256; global_huffman_bit_count_ary[size_i++] = 9) {}
      for (; size_i < 280; global_huffman_bit_count_ary[size_i++] = 7) {}
      /* The tokens 286 and and 287 are invalid, but we still add them here because their presence affects the Huffman codes for 9-bit tokens 144..255. */
      for (; size_i < 288; global_huffman_bit_count_ary[size_i++] = 8) {}
      distance_size = 30;
      /* The distance values 30 and 31 are invalid, and there is no need to add them here, because their presence doesn't affect the Huffman codes of other distance values. */
      for (; size_i < 288 + 30; global_huffman_bit_count_ary[size_i++] = 5) {}
      global_tree_free_idx = TREE_FIRST_FREE_IDX;
      goto common_compressed_block;
     case 2:  /* Block compressed with dynamic Huffman codes. */
      literal_and_len_size = read_bits_max_8(5) + 257;
      if (literal_and_len_size > 286) goto corrupted_input;
      distance_size = read_bits_max_8(5) + 1;
      if (distance_size > 30) goto corrupted_input;
      tmp_size = read_bits_max_8(4) + 4;
      memset(tmp_bit_count_ary, 0, sizeof(tmp_bit_count_ary));
      for (size_i = 0; size_i < tmp_size; ++size_i) {
        tmp_bit_count_ary[(size_i < 3 ? size_i + 16 : size_i == 3 ? 0 : ((size_i & 1) != 0 ? 19 - size_i : size_i + 12) >> 1)] = read_bits_max_8(3);
      }
      global_tree_free_idx = TREE_FIRST_FREE_IDX;
      global_huffman_trees_ary[TREE_TMP_ROOT_IDX] = LEAF_IDX;
      build_huffman_tree(tmp_bit_count_ary, 19, TREE_TMP_ROOT_IDX);  /* All tmp_bit_count_ary[...] elements are in 0..7. */
      /* We must read the two bit count arrays together, because it's OK for them to overlap during RLE decompression. */
      /* After this, all elements are in 0..15. */
      read_bit_count_ary(global_huffman_bit_count_ary, literal_and_len_size + distance_size);
     common_compressed_block:
      global_huffman_trees_ary[TREE_LITERAL_AND_LEN_ROOT_IDX] = global_huffman_trees_ary[TREE_DISTANCE_ROOT_IDX] = LEAF_IDX;
      build_huffman_tree(global_huffman_bit_count_ary, literal_and_len_size, TREE_LITERAL_AND_LEN_ROOT_IDX);  /* All literal_and_len elements are in 0..15. */
      build_huffman_tree(global_huffman_bit_count_ary + literal_and_len_size, distance_size, TREE_DISTANCE_ROOT_IDX);  /* All distance elements are in 0..15. */
      for (;;) {
        token = read_using_huffman_tree(TREE_LITERAL_AND_LEN_ROOT_IDX);
        if (token < 0x100) { /* LZ literal token. */
          if (match_distance_limit < 0x8000U) ++match_distance_limit;
          out = write_byte(out, token);
          continue;
        }
        token -= 0x101;
        if ((int)token < 0) break;  /* End-of-block token (old value 256). Go to next block. */
        /* Now we have the LZ match token with the some info about the length in token. */
        if (token >= 28) {
          if (token > 28) goto corrupted_input;  /* Old token was BAD_LEAF_VALUE. This can also happen for tokens 286 and 287 in fixed Huffman codes. */
          match_length = 0x102;
        } else if (token < 8) {
          match_length = 3 + token;
        } else {
          match_length = 3 + ((4 + (token & 3)) << ((token >> 2) - 1)) + read_bits_max_8((token >> 2) - 1);
        }
        /* Now we have the final value of match_length: 1..258. */
        match_distance = read_using_huffman_tree(TREE_DISTANCE_ROOT_IDX);
        if (match_distance > 29) goto corrupted_input;  /* match_distance == BAD_LEAF_VALUE. This can also happen for distances 30 and 31 in fixed Huffman codes. */
        if (match_distance >= 4) {
          match_distance_extra_bits = (match_distance >> 1) - 1;
          match_distance = read_bits_max_16(match_distance_extra_bits) + (((match_distance & 1) + 2) << match_distance_extra_bits);
        }
        /* Now we have the final value of match_distance: 0..32767 == 0..0x7fff. */
        if (match_distance >= match_distance_limit) goto corrupted_input;
        if ((match_distance_limit += match_length) >= 0x8000U) match_distance_limit = 0x8000U;
        prev_idx = (out - global_write_buffer - 1 - match_distance);
        do {
          out = write_byte(out, global_write_buffer[prev_idx++ & (0x8000U - 1U)]);
        } while (--match_length != 0);
      }
      break;
     default:  /* case 3. Reserved block type (error). */
      goto corrupted_input;
    }
  } while (!bfinal);
  flush_write_buffer(out - global_write_buffer);
  main0_exit0();  /* return EXIT_SUCCESS; */
}
