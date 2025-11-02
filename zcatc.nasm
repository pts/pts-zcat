;
; zcatc.nasm: tiny stdin-to-stdout decompression filter implementation of the gzip, zlib, Deflate and Quasijarus strong file formats for DOS 8086 in NASM assembly
; by pts@fazekas.hu at Fri Sep 26 00:32:20 CEST 2025
;
; Usage: zcatc <file.gz >file
;
; Compile, for decompressing gzip and raw Deflate: nasm-0.98.39 -O0 -w+orphan-labels -f bin -o zcatc.com zcatc.nasm
; Compile, for decompressing gzip and zlib: nasm-0.98.39 -O0 -w+orphan-labels -DUSE_ZLIB -f bin -o zlcatc.com zcatc.nasm
; This program compiles identically with optimized (-O32700) and unoptimized (-0) NASM.
;
; Program size: The DOS 8086 .com program zcatc.com is 958 bytes. It's
; written in manually optimized 8086 assembly in the NASM syntax. The
; feature-equivalent C program, zcats.c can be compiled to DOS 8086 .com
; program of 1624 byes with OpenWatcom C compiler and a custom tiny libc.
; The reasons why the assembly implementation is >40% smaller are the heavy
; manual optimizations.
;
; Minimum DOS version needed: MS-DOS >=2.00, IBM PC DOS >=2.00 or a DOS
; emulator (such as DOSBox, emu2, kvikdos, NTVDM). There is no file size
; limit, the program works as long as DOS or the emulator can supply the
; compressed input and consume the uncompressed output.
;
; Memory requirements: <=0x9180 bytes, including 0x80 bytes of extra stack
; space for DOS and interrupt handlers. Of these bytes, the majority, 0x8000
; bytes are used by the ring buffer (dictionary), as mandated by the
; [Deflate specification](https://www.rfc-editor.org/rfc/rfc1951.txt).
; Actually, DOS will typically use 0x10000 bytes (64 KiB) plus a few dozen
; bytes for process overhead to run this DOS .com program.
;
; This program does the same as the zcat(1) program in gzip, except for these:
;
; * This program ignores its command-line arguments.
; * The error conditions are the same, but the error messages are different.
; * The set of supported input file formats are different (but both
;   programs support the gzip file format).
; * The set of gzip flags supported is different.
; * This program doesn't compute or check the CRC-32 and Adler-32 checksums
;   of the uncompressed output data.
; * This program doesn't flush the write buffer on error.
; * This program stops at the end of the first Deflate stream.
;
; This program does the same as
; https://github.com/pts/pts-zcat/blob/master/muzcat.c , except for the
; following functionality differences:
;
; * This program has stricter error checking and thus memory-safe operation.
; * The set of supported input file formats are different (but both
;   programs support the gzip file format).
; * The set of gzip flags supported is different.
; * This program stops reading its input after the end of the first Deflate
;   stream.
; * This program uses a bit less memory.
; * This program stops at the end of the first Deflate stream.
;
; NASM configuration options at compile time:
;
; * `-DUSE_DEFLATE` (default): As input, support the
;   [gzip](https://www.rfc-editor.org/rfc/rfc1952.txt), [raw
;   Deflate](https://www.rfc-editor.org/rfc/rfc1951.txt) and
;   [4.3BSD-Quasijarus strong
;   compression](http://fileformats.archiveteam.org/wiki/Quasijarus_Strong_Compression)
;   file formats. It doesn't support zlib, and conflicts with
;   `-DUSE_ZLIB`.
; * `-DUSE_ZLIB`: As input, support the
;   [gzip](https://www.rfc-editor.org/rfc/rfc1952.txt),
;   [zlib](https://www.rfc-editor.org/rfc/rfc1950.txt) and
;   [4.3BSD-Quasijarus strong
;   compression](http://fileformats.archiveteam.org/wiki/Quasijarus_Strong_Compression)
;   file formats. It doesn't support raw Deflate, and conflicts with
;   `-DUSE_DEFLATE`.
; * `-DUSE_NZTREELEAF`: Use a non-zero `TREE_LEAF` value; this helps
;   debugging of the Huffman tree builder.
; * `-DUSE_SCRAMBLECODE`: Use code to compute the bit count RLE code
;   scrambling order rather than a lookup table; this makes the program file
;   10 bytes longer.
; * `-DUSE_DEBUG`: Print lots of debug info to stdout, don't write
;   uncompressed data.
;
; Things to do before a release (all done):
;
; * Run a smoke test with small input files with and without -DUSE_ZLIB.
; * Compile with -O999999999 and check that they are bitwise identical. Also compile with newer NASM.
; * Run the big test with >42000 .gz files for both zcatc.nasm and zcats.c.
;
; TODO(pts): Add support for multiple gzip streams (such as in Alpine Linux APKINDEX.*.tar.gz); both gzip and muzcat support multiple streams; for that we need EOF detection.
;
; !! Calculate (1 << i) - 1 using a lookup table (lut_pow2_mask) in read_bits_max_8 (4-byte instruction with the right registers). Is it shorter? Probably not.
;

bits 16
cpu 8086
BASE equ 0x100
org BASE  ; For DOS .com program.

NUL equ 0  ; ASCII NUL == '\0'.
CR equ 13  ; ASCII CR  == '\r'.
LF equ 10  ; ASCII LF  == '\n'.

STDIN_FILENO equ 0
STDOUT_FILENO equ 1
STDERR_FILENO equ 2

EXIT_SUCCESS equ 0
EXIT_FAILURE equ 1

MAX_TMP_SIZE equ 19
MAX_INCOMPLETE_BRANCH_COUNT equ 7+15 +15  ; Because of DO_CHECK_HUFFMAN_TREE_100_PERCENT_COVERAGE == 1.
MAX_LITERAL_AND_LEN_SIZE equ 256+30
MAX_DISTANCE_SIZE equ 30
MAX_TREE_SIZE equ ((6-(3<<2))+((MAX_TMP_SIZE+MAX_LITERAL_AND_LEN_SIZE+MAX_DISTANCE_SIZE)<<1<<1)+(MAX_INCOMPLETE_BRANCH_COUNT<<2))  ; <<1 is for the max node_count/leaf_node_count for a tree. <<1 is for node_count/element_count.

%ifdef USE_NZTREELEAF  ; Enabling this makes the code longer, but sometims it makes it easier to debug.
  LEAF_PTR equ 0xface  ; Can be 0 or someting large (e.g. 0xffff or a bit less).
%else
  LEAF_PTR equ 0
%endif
BAD_LEAF_VALUE equ 0xffff

READ_BUFFER_SIZE equ 0x2000  ; Any postive value works, but values smaller than 0x1000 make the program slow because of increased system call overhead.
WRITE_RING_BUFFER_SIZE equ 0x8000  ; This is the maximum size (32 KiB) specified in https://www.rfc-editor.org/rfc/rfc1951.txt .

; Small (non-array) global variables are indexed using GSMALL(...) instead of [...] to save 1 byte per use.
%define GSMALL(label) [byte bp-global_uninitvars+label]
%define IS_HISTOGRAM_THE_FIRST_UNINITVAR 1

; --- Code.

_start:  ; Entry point of the DOS .com program.
		xor ax, ax  ; AX := 0.
		mov di, global_initvars
		stosw  ; bitbuf_bits_remaining := bitbuf := 0.
		stosw  ; match_distance_limit := 0.
		; Now DI == global_uninitvars.
		mov bp, di  ; BP := global_uninitvars. BP remains the same throughout the program, and can be used for GSMALL(...) addressing.

.try_gzip:  ; gzip: https://www.rfc-editor.org/rfc/rfc1952.txt
		call clear_read_buffer_and_read_byte  ; gzip ID1 byte.
		cmp al, 0x1f
		jne short .not_gzip_id1  ; ID1 byte, must be 0x1f for gzip.
		call read_byte  ; ID2 byte.
		cmp al, 0xa1
		je short .decompress_deflate  ; 4.3BSD-Quasijarus strong compression (compress -s) produces the 0x1f 0xa1 header and then raw Deflate.
		cmp al, 0x8b
		jne short .jmp_fatal_corrupted_input1  ; ID2 byte, must be 0x8b.
		call read_byte
		cmp al, 8  ; CM byte.
		jne short .jmp_fatal_corrupted_input1  ; CM byte, must be 8.
		call read_byte  ; FLG byte.
		xchg bx, ax  ; BL := AL (FLG byte); BH := junk; AX := junk.
		mov cx, 6
.next_ignore1:
		call read_byte  ; Ignore MTIME (4 bytes), XFL (1 byte) and OS (1 byte).
		loop .next_ignore1
		test bl, 2  ; BL is the FLG byte.
		jnz short .jmp_fatal_corrupted_input1  ; The FHCRC enables GCONT (2-byte continuation part number) here for gzip(1), and header CRC16 later for https://www.rfc-editor.org/rfc/rfc1950.txt . We just fail if it's set.
		test bl, 4
		jz short .after_fextra  ; FEXTRA. Ignore the extra data. Little-endian. It's OK to overlap bit reads with byte reads here, because we remain on byte boundary.
		call read_bits_16
		xchg cx, ax  ; CX := FEXTRA size; AX := junk.
		jcxz .after_fextra
.next_ignore2:
		call read_byte
		loop .next_ignore2
.after_fextra:
		test bl, 8
		jz short .after_fname  ; FNAME. Ignore the filename.
.next_ignore3:
		call read_byte
		test al, al
		jnz short .next_ignore3
.after_fname:
		test bl, 0x10
		jz short .after_fcomment  ; FCOMMENT. Ignore the comment.
.next_ignore4:
		call read_byte
		test al, al
		jnz short .next_ignore4
.after_fcomment:
		; Now we could ignore the 2-byte CRC16 if (flg & 2), but we've disallowed it above.
		test bl, 0x20
		jz short .after_enchdr  ; Ignore the gzip(1) encryption header.
		mov cl, 12  ; CX := 12. CH was already 0.
.next_ignore5:
		call read_byte  ; Ignore MTIME (4 bytes), XFL (1 byte) and OS (1 byte).
		loop .next_ignore5
%ifdef USE_ZLIB  ; zlib: https://www.rfc-editor.org/rfc/rfc1950.txt
.after_enchdr:
		jmp short .decompress_deflate
.not_gzip_id1:
		cmp al, 0x80  ; CM byte. Check that CINFO <= 7, otherwise ignore CINFO (sliding window size).
		jae short .jmp_fatal_corrupted_input1
		and al, 0xf
		cmp al, 8
		jne short .jmp_fatal_corrupted_input1  ; Check that CM == 8.
		call read_byte
		test al, 0x20
		jnz short .jmp_fatal_corrupted_input1  ; FLG byte. Check that FDICT == 0. Ignore FLEVEL and FCHECK.
%else  ; Use raw Deflate: https://www.rfc-editor.org/rfc/rfc1951.txt
.not_gzip_id1:
		dec word GSMALL(read_ptr)
  %if 1  ; We have to put back the byte in AL to the beginning of global_read_buffer. But it's already there, so we on't do anything.
  %elif 0  ; This is 1 byte shorter, but it works only for the first byte, i.e. if read_ptr == global.read_buffer.
		mov [global.read_buffer], al  ; Put the just-read byte back to the read buffer so that the Deflate decompressor (.decompress_deflate below) can read it.
  %elif 0
		mov di, GSMALL(read_ptr)
		stosb  ; Put the just-read byte back to the read buffer so that the Deflate decompressor (.decompress_deflate below) can read it.
  %endif
.after_enchdr:
%endif
.decompress_deflate:  ; Raw Deflate: https://www.rfc-editor.org/rfc/rfc1951.txt
		mov di, global.write_buffer  ; Will be used as the `out' variable through the Deflate decompression except when starting a compressed block.
.end_of_block:
		clc  ; The opcode of `stc' is just 1 larger than of `clc'. This is self-modifying code, this instruction will be modified to `stc' upon BFINAL == 1.
		jnc short .next_block
		; Now we want to flush the write buffer, i.e. the bytes
		; between global.write_buffer and DI, and then exit
		; successfully (exit(EXIT_SUCCESS)). We do all this a tricky
		; indirect way to save a few (4) bytes of code size.
		;
		; To flush, we call flush_write_buffer_and_write_byte, which
		; appends an extra junk byte in AL to the write buffer after
		; flushing. That's not a problem, because this extra junk
		; byte will never get flushed.
		;
		; To exit, we execute `int 0x20', which is a DOS syscall for
		; exit(EXIT_SUCCESS). To do so, we jump to address 0, where
		; DOS put the `int 0x20' instruction bytes when loading the
		; program. The `ret' flush_write_buffer_and_write_byte does
		; the jump to address 0, because DOS has also pushed the
		; word 0 to the stack when loading the .com program.
		jmp near flush_write_buffer_and_write_byte
		; Not reached.
.jmp_fatal_corrupted_input1:
		; This a construct of a jump jumping to another jump. It is
		; used at multiple places in this program to convert most of
		; the jumps to `jmp short', which is 1 byte shorter than a
		; `jmp near'.
		;
		; This particular jump is a `jmp near', and the code byte is
		; saved in the other nearby jumps jumping here instead of
		; jumping to the far-away fatal_corrupted_input directly.
		jmp near fatal_corrupted_input
.next_block:  ; Decompress next block.
		mov al, 1
		call read_bits_max_8  ; BFINAL.
		add [.end_of_block], al  ; Self-modifying code: This changes `clc' to `stc' at the beginning of .end_of_block upon BFINAL == 1.
		mov al, 2
		call read_bits_max_8  ; BTYPE.
		dec ax
		jns .not_uncompressed_block
.uncompressed_block:  ; Uncompressed block.
		mov byte GSMALL(bitbuf_bits_remaining), 0  ; Discard partially read byte.
		call read_bits_16  ; LEN.
		xchg cx, ax ; CX := LEN value; AX := junk.
		call read_bits_16  ; NLEN.
		xor ax, cx  ; AX := LEN^NLEN.
		inc ax
.jnz_fatal_corrupted_input5:
		jnz short .jmp_fatal_corrupted_input1  ; Jump iff (LEN^NLEN) != 0xffff.
		test cx, cx
		jz short .end_of_block
		js short .saturate_match_distance_limit1
		add GSMALL(match_distance_limit), cx
		jns short .next_uncompressed_byte
.saturate_match_distance_limit1:
		mov word GSMALL(match_distance_limit), 0x8000  ; Very large limit which won't ever be reached.
.next_uncompressed_byte:
		call read_byte
		call write_byte  ; Write byte in AL to DI, update DI.
		loop .next_uncompressed_byte
		jmp short .end_of_block
.not_uncompressed_block:  ; BTYPE == 1 || BTYPE == 2 || BTYPE == 3.
		; Optimization bug (?) in NASM 0.98.39 -O0: without this `byte', it emits a word-size displacement for the bp+... .
		push di  ; Save the `out' value. Will be restored to DI in .common_compressed_block just above .next_token.
		jnz short .btype_2_or_3  ; This still uses the FLAGS of the `dec ax' operation near the end of .next_block.
.block_compressed_with_fixed_huffman:  ; BTYPE == 1: Block compressed with fixed Huffman codes.
		mov di, global.huffman_bit_count_ary
		mov cx, 144
		mov al, 8
		rep stosb
		mov cl, 256-144  ; CX := 256-144.
		inc ax  ; AL := 9.
		rep stosb
		mov cl, 280-256  ; CX := 280-256.
		mov al, 7
		rep stosb
		; The tokens 286 and and 287 are invalid, but we still add them here because their presence affects the Huffman codes for 9-bit tokens 144..255.
		mov cl, 288-280  ; CX := 288-280.
		inc ax  ; AL := 8.
		rep stosb
		;mov di, global.huffman_bit_count_ary+...  ; Not needed, DI is already correct.
		; The distance values 30 and 31 are invalid, and there is no need to add them here, because their presence doesn't affect the Huffman codes of other distance values.
		mov cl, 30  ; CX := 30.
		mov al, 5
		rep stosb
		mov dx, 288  ; DX (literal_and_len_size) := 288.
		mov cl, 30  ; CX (distance_size) := 30.
		mov di, global.huffman_trees_ary
		jmp near .common_compressed_block
.btype_2_or_3:
		dec ax
		jnz short .jnz_fatal_corrupted_input5  ; Jump iff BTYPE == 3: Reserved block type (error).
.block_compressed_with_dynamic_huffman:  ; BTYPE == 2: Block compressed with dynamic Huffman codes.
		mov al, 5
		call read_bits_max_8
		cmp al, 286-257
.ja_fatal_corrupted_input4:
		ja short .jmp_fatal_corrupted_input1
		add ax, strict word 257
		push ax  ; Save literal_and_len_size.
		mov al, 5
		call read_bits_max_8
		inc ax
		cmp al, 30
		ja short .ja_fatal_corrupted_input4
		push ax  ; Save distance_size.
		mov al, 4
		call read_bits_max_8
		add al, 4  ; After this, 4 <= AX (tmp_size) <= 19.
		push ax  ; Save tmp_size.
		mov di, global.huffman_bit_count_ary  ; DI := tmp_bit_count_ary. We'll use only the first 20 bytes of this temporarily, and only right after the first call to build_huffman_tree_RUINS below.
		mov cx, 20>>1
		xor ax, ax
		rep stosw  ; Overwrites tmp_bit_count_ary (19 bytes used + 1 byte of padding) for a fast `stosw'.
		pop cx  ; Restore CX := tmp_size.
%ifdef USE_SCRAMBLECODE  ; This is 10 bytes longer and may also be slower than the lookup table.
		xor si, si  ; SI (size_i) := 0.
.next_tmp_bit_count_idx:
		call read_bits_3
		xchg dx, ax  ; DX (bit_count) := AX (bit_count); AX := junk.
		mov ax, si  ; AX := SI (size_i). This also sets AH := 0. After this, 0 <= AX <= 7.
		cmp al, 3
		jnb short .nb_3
		add al, 16
		jmp short .tmp_bit_count_idx_ok
.nb_3:
		jne short .a_3
		mov al, 0
		jmp short .tmp_bit_count_idx_ok
.a_3:
		shr al, 1
		jnc short .add
		sub al, 9
		neg al
		jmp short .tmp_bit_count_idx_ok
.add:
		add al, 6
.tmp_bit_count_idx_ok:
		mov bx, ax  ; BX := (size_i < 3 ? size_i + 16 : size_i == 3 ? 0 : ((size_i & 1) != 0 ? 19 - size_i : size_i + 12) >> 1).
		mov [di-20+bx], dl  ; tmp_bit_count_ary[BX] = bits_read.
		inc si  ; SI (size_i) += 1.
%else
		mov si, lut_deflate_bit_count_scramble
.next_tmp_bit_count_idx:
		lodsb  ; AH was already 0.
		xchg bx, ax  ; BX := (size_i < 3 ? size_i + 16 : size_i == 3 ? 0 : ((size_i & 1) != 0 ? 19 - size_i : size_i + 12) >> 1); AX := junk.
		call read_bits_3  ; Also sets AH := 0.
		mov [di-20+bx], al  ; tmp_bit_count_ary[BX] = bits_read.
%endif
		loop .next_tmp_bit_count_idx  ; Repeat tmp_size times.
		mov si, global.huffman_bit_count_ary  ; SI (build_bit_count_ary_ptr) := tmp_bit_count_ary. We'll use only the first 20 bytes of this temporarily, and only right after the first call to build_huffman_tree_RUINS below.
		mov dx, 19  ; DX (distance_size) := 19.
		mov di, global.huffman_trees_ary
		call build_huffman_tree_RUINS  ; build_huffman_tree(tmp_bit_count_ary, 19, literal_and_len_root_ptr);  ; All tmp_bit_count_ary[...] elements are in 0..7. Also sets DI := new tree_free_ptr.
		pop cx  ; Restore CX := distance_size.
		pop dx  ; Restore DX := literal_and_len_size.
		push di ; Save tree_free_ptr. Will be popped below, just above .common_compressed_block.
		push cx  ; Save distance_size.
		add dx, cx  ; DX (orig_size) := literal_and_len_size+distance_size.
		mov di, global.huffman_bit_count_ary  ; DI (bit_count_ary_ptr) := global.huffman_bit_count_ary.
		; We must read the two bit count arrays together, because it's OK for them to overlap during RLE decompression.
		; read_bit_count_ary(global.huffman_bit_count_ary, literal_and_len_size + distance_size); After this, all elements are in 0..15.

		; Input: DI is a pointer to the beginning of the output array of bit counts (unsigned char *bit_count_ary_ptr); DX is the uncompressed number of the RLE-compressed input values (size, 1 <= size <= 318).
		; Ruins AX, BX, CX, DX, SI, DI, FLAGS; keeps all other registers (such as CX) intact.
;read_bit_count_ary_RUINS:
		mov si, dx  ; SI (size) := DX (orig_size).
.next_rle_value:
		mov bx, global.huffman_trees_ary  ; The root node of the output of the build_huffman_tree_RUINS above.
		call read_using_huffman_tree  ; AX (value) := read_using_huffman_tree(tmp_root_ptr). Also sets AH := 0.
		cmp al, 16
		jnb short .not_rle_literal
		stosb
		dec si
		jmp short .maybe_next_rle_value
.not_rle_literal:
		jne short .not_16
		cmp si, dx
		je short .jmp_fatal_corrupted_input2  ; Jump iff size == orig_size.
		mov cl, [di-1]  ; CL (output_value) := bit_count_ary_ptr[-1].
		mov al, 2
		call read_bits_max_8
		;add al, 3  ; AX (count) := read_bits_max_8(2) + 3 - 3.  ; We will add the +3 later.
.append_many:
		add al, 3  ; Add the missing +3.
		sub si, ax  ; size -= count.
		jc short .jmp_fatal_corrupted_input2  ; Jump iff count > old_size.
		xchg ax, cx
		rep stosb
		jmp short .maybe_next_rle_value
.not_16:
		cmp al, 18  ; We only compare the low byte of BAD_LEAF_VALUE, but that's fine, because regular values are in 0..18.
		jb short .is_17  ; This can be converted to ja ....ja_fatal_corrupted_input if needed.
		je short .is_18
.jmp_fatal_corrupted_input2:
		; Changing this to `jmp short .ja_fatal_corrupted_input3' would introduce a bug for code jumping to here (.jmp_fatal_corrupted_input2), because the jump at  .ja_fatal_corrupted_input3 is a `ja' rather than a `jmp', so it wouldn't always jump.
		jmp near fatal_corrupted_input
.is_17:
		call read_bits_3
		;add al, 3  ; AX (count) := read_bits_max_8(3) + 3 - 3. We will add the +3 later.
.set_value_to_missing:
		mov cl, 0  ; CL (output_value) := 0.
		jmp short .append_many
.is_18:
		; Now: AL (value) is 18.
		mov al, 7
		call read_bits_max_8
		add al, 11-3  ; AX (count) := read_bits_max_8(7) + 11 - 3. We will add the +3 later.
		jmp short .set_value_to_missing
.maybe_next_rle_value:
		test si, si
		jnz short .next_rle_value  ; Jump iff remaining size is nonzero.

		pop cx  ; Restore CX := distance_size.
		sub dx, cx  ; DX := literal_and_len_size.
		pop di  ; Restore DI := tree_free_ptr.
		db 0xa9  ; This is the opcode byte of `test ax, strict word ...'. We use this overlapping code to skip over the 2 bytes of .ja_fatal_corrupted_input3.
.ja_fatal_corrupted_input3:
		jmp short .ja_fatal_corrupted_input2  ; Unfortunately we need this, because .ja_fatal_corrupted_input2 below is too far away for the jumpers above.
.common_compressed_block:
%if .common_compressed_block!=.ja_fatal_corrupted_input3+2
  %error ERROR_JUMP_JA3_MUST_BE_2_BYTES_FOR_SKIP  ; The db `0xa9' above needs it.
%endif
		; Now DX is literal_and_len size; CX is distance_size; DI is pointer to the first free node location in global.huffman_trees_ary.
		push cx  ; Save distance_size.
		mov si, global.huffman_bit_count_ary  ; SI (build_bit_count_ary_ptr) := global.huffman_bit_count_ary.
		; Now: DX (build_size) is literal_and_len_size.
		mov GSMALL(literal_and_len_root_ptr), di
		call build_huffman_tree_RUINS  ; build_huffman_tree(global.huffman_bit_count_ary, literal_and_len_size, literal_and_len_root_ptr).  ; All literal_and_len elements are in 0..15. Also sets DI := new tree_free_ptr.
		; Now: SI (build_bit_count_ary_ptr) is global.huffman_bit_count_ary+literal_and_len_size, as returned by the build_huffman_tree_RUINS above.
		pop dx  ; Restore DX (build_size) := distance_size.
		mov GSMALL(distance_root_ptr), di
		call build_huffman_tree_RUINS  ; build_huffman_tree(global.huffman_bit_count_ary + literal_and_len_size, distance_size, distance_root_ptr).  ; All distance elements are in 0..15. Also sets DI := new tree_free_ptr.
		pop di  ; Restore DI := out.
.next_token:
		mov bx, GSMALL(literal_and_len_root_ptr)
		call read_using_huffman_tree  ; AX (token) := read_using_huffman_tree(literal_and_len_root_ptr);
		sub ax, strict word 256
		jb short .literal_token
		jnz short .match_token
		jmp near .end_of_block
.literal_token:  ; LZ literal token.
		cmp byte GSMALL(match_distance_limit+1), 0x8000>>8
		jnb short .match_distance_limit_ok_for_literal  ; Jump iff match_distance_limit >= 0x8000.
		inc word GSMALL(match_distance_limit)
.match_distance_limit_ok_for_literal:
		call write_byte  ; Write byte in AL to DI, update DI.
%ifdef USE_DEBUG
		call debug_lz_literal
%endif
.jmp_next_token1:
		jmp short .next_token
.match_token:
		dec ax
		; Now we have the LZ match token with the some info about the length in AL == AX <= 29.
		cmp al, 28
		jb short .b_28
.ja_fatal_corrupted_input2:
		ja short .ja_fatal_corrupted_input1  ; Old token was BAD_LEAF_VALUE. This can also happen for fixed Huffman codes.
		mov ax, 258
		jmp short .match_length_ok
.b_28:
		cmp al, 8
		jnb short .nb_8
		add al, 3
		jmp short .match_length_ok
.nb_8:
		; Now: 8 <= AX <= 27.
		mov cl, al
		shr cl, 1
		shr cl, 1
		dec cx  ; After this, 1 <= CL <= 5.
		and al, 3
		add al, 4
		shl al, cl  ; After this, 8 <= AX <= 224.
		add al, 3  ; After this, 11 <= AX <= 227.
		xchg ax, cx
		call read_bits_max_8  ; Read 1..5 bits to AX.
		add ax, cx  ; After this, 11 <= AX <= 258.
.match_length_ok:
		; Now we have the final value of match_length in AX: 1..258.
		xchg dx, ax  ; DX (match_length) := AX (match_length); AX := junk.
		mov bx, GSMALL(distance_root_ptr)
		call read_using_huffman_tree  ; AX (match_distance) := read_using_huffman_tree(distance_root_ptr);
		cmp al, 29
.ja_fatal_corrupted_input1:
		ja short build_huffman_tree_RUINS.ja_fatal_corrupted_input4  ; match_distance == BAD_LEAF_VALUE. This can also happen for fixed Huffman codes.
		cmp al, 4
		jb .match_distance_ok
		mov cl, al
		shr cl, 1
		dec cx  ; CL (match_distance_extra_bits) := (match_distance >> 1) - 1. After this, 1 <= CL <= 13.
		and al, 1
		add al, 2  ; AX := (match_distance & 1) + 2. After this, 2 <= AX <= 3.
		shl ax, cl  ; After this, 4 <= AX <= 24576.
		xchg ax, cx
		call read_bits_max_16  ; Read 1..13 bits to AX.
		add ax, cx  ; After this, 4 <= AX <= 32767.
.match_distance_ok:
		; Now we have the final value of match_distance in AX: 0..32767 == 0..0x7fff.
		; Emit an LZ match string with length in DX (1..258) and distance in AX (0..32767).
%ifdef USE_DEBUG
		call debug_lz_match
%endif
		inc ax
		cmp ax, GSMALL(match_distance_limit)
		ja short build_huffman_tree_RUINS.ja_fatal_corrupted_input4
		add GSMALL(match_distance_limit), dx
		jns short .match_distance_limit_ok_for_match
		mov word GSMALL(match_distance_limit), 0x8000  ; Very large limit which won't ever be reached.
.match_distance_limit_ok_for_match:
		lea bx, [word di-(global.write_buffer-$$)-BASE]  ; BX := (DI-global.write_buffer)-1.
		sub bx, ax  ; BX -= AX (match_distance).
		; Emit an LZ match string, copying CX bytes from global.write_buffer+(BX&0x7fff) to DI, and then make sure that global.write_buffer is not full.
.next_match_byte:
		and bh, 0x7f  ; This is correct iff the size of the ring buffer (global.write_buffer) is 0x8000.
		mov al, [global.write_buffer+bx]
		call write_byte  ; Write byte in AL to DI, update DI.
		inc bx
		dec dx
		jnz short .next_match_byte
		jmp short .jmp_next_token1

; Builds a Huffman tree from bit counts
; build_bit_count_ary_ptr[:build_size], and adds it to
; global.huffman_trees_ary, starting at address tree_free_ptr. For the
; structure of global.huffman_trees_ary, see it comment below. For using
; this Huffman tree to read a value bit-by-by from stdin, see the function
; read_using_huffman_tree.
;
; Input: SI is build_bit_count_ary_ptr; DX is build_size (1 <= build_size <= 288); DI is tree_free_ptr, also the build_root_ptr value for the new tree.
; Output: DI is the new tree_free_ptr (larger than the input in DI); BX is the original build_root_ptr; CX is build_size; DX is 0; SI is build_bit_count_ary_ptr+build_size.
; Ruins AX, FLAGS; keeps all other registers intact.
build_huffman_tree_RUINS:
		push si  ; Save build_bit_count_ary_ptr.
		push di  ; Save build_root_ptr == tree_free_ptr.
%if LEAF_PTR  ; Usually false.
		mov ax, LEAF_PTR
%else
		xor ax, ax  ; Used by both `stosw' and `rep stosw' below.
%endif
		stosw  ; build_root_ptr[0] = LEAF_PTR. Also DI += 2.
		stosw  ; build_root_ptr[1] = arbitrary. Also DI += 2. We do it for the DI += 2 only.
		push di  ; Save tree_free_ptr.
%if LEAF_PTR  ; For the `rep stosw' below.
		xor ax, ax
%endif
%if IS_HISTOGRAM_THE_FIRST_UNINITVAR  ; This is true, and it makes the code shorter.
		mov di, bp  ; Short version of DI := address of build_bit_count_histogram_and_g_ary.
%else
		lea di, GSMALL(build_bit_count_histogram_and_g_ary)
%endif
		push di   ; Save address of bit_count_histogram_and_g_ary.
		mov cx, 16
		rep stosw  ; for (unsigned i = 0; i < 16; ++i) { bit_count_histogram_and_g_ary[i] = 0; }  ; Also sets CX := 0.
		pop di  ; Restore DI := bit_count_histogram_and_g_ary.
		mov cx, dx  ; CX := DX (build_size).
.next_histogram_entry:
		lodsb  ; AX := build_bit_count_ary_ptr[size_i]. Also SI += 1. AH was already 0. After this, 0 <= AX <= 15.
		mov bx, ax  ; BX := AX. This also sets BH := 0.
		add bx, bx  ; After this, 0 <= BX <= 30, thus BH == 0.
		inc word [di+bx]  ; bit_count_histogram_and_g_ary[build_bit_count_ary_ptr[old_size_i]] += 1.
		loop .next_histogram_entry  ; Repeat build_size times.
.histogram_done:
		xor ax, ax  ; AX (total_g) := 0.
		; Now: DI is bit_count_histogram_and_g_ptr, with initial value bit_count_histogram_and_g_ary.
		stosw  ; *bit_count_histogram_and_g_ptr++ := total_g << 1 == 0.
		mov cx, 15
.next_total_g_iter:
		cmp ax, strict word 0x4000
.ja_fatal_corrupted_input4:
		ja short fatal_corrupted_input  ; Jump iff AX (total_g) > 0x4000.
		add ax, ax  ; AX (total_g) <<= 1.
		add ax, [di]  ; AX (total_g) += *DI (bit_count_histogram_and_g_ptr).
		add ax, ax  ; Store high bit of old AX in CF.
		stosw  ; *bit_count_histogram_and_g_ptr++ := total_g << 1.
		rcr ax, 1  ; Copy CF (output of `shl ax, 1' above, `stosw' doesn't change the flags) to the high bit of AX.
		loop .next_total_g_iter
		;xor cx, cx  ; CX (size_i) := 0. No need to set, CX was already 0 (set by the `loop' above).
		pop di  ; Restore DI := tree_free_ptr. DI will be increased below, and the final value returned.
		pop bx  ; Restore BX := build_root_ptr. Also initialize BX := build_bit_count_ary_ptr+size_i.
		pop si  ; Restore SI := build_bit_count_ary_ptr.
		; Now AX is junk (it will be used below); BX is build_root_ptr; CX is initial size_i, which is now 0; DX is initial build_size_remaining, which is now build_size; SI is initial build_bit_count_ary_ptr; DI is tree_free_ptr; BP is the address of global_uninitvars.
.next_insert:
		lodsb  ; AL (bit_count) := bit_count_ary_ptr[size_i]. SI (build_bit_count_ary_ptr+size_i) += 1.
		cbw  ; AH := 0, because 0 <= AL <= 15.
		dec ax  ; AX (bit_count) := bit_count-1.
		js short .maybe_next_insert  ; If bit_count was 0, then ignore this value (size_i), and proceed to the end of the loop body.
		push si  ; Save build_bit_count_ary_ptr+size_i+1.
		push dx  ; Save build_size_remaining.
		push cx  ; Save size_i. CL is needed for the `shl si, cl' shift below.
		mov cl, al  ; CL := AL (bit_count).
		xchg si, ax  ; SI := AX (bit_count); AX := junk. After this, 0 <= SI (bit_count) <= 28.
		add si, si
%if IS_HISTOGRAM_THE_FIRST_UNINITVAR  ; This is true, and it makes the code shorter.
		mov dx,  [bp+si]  ; DX (code) := bit_count_histogram_and_g_ary[bit_count].
		inc word [bp+si]  ; bit_count_histogram_and_g_ary[bit_count] += 1.
%else
		mov dx,  GSMALL(build_bit_count_histogram_and_g_ary+si)  ; DX (code) := bit_count_histogram_and_g_ary[bit_count].
		inc word GSMALL(build_bit_count_histogram_and_g_ary+si)  ; bit_count_histogram_and_g_ary[bit_count] += 1.
%endif
		mov si, 1  ; SI := 1.
		shl si, cl  ; SI (code_mask_i) := 1 << bit_count.
		pop cx  ; Restore CX := size_i.
		; Now AX is junk (it will be used below); BX is build_root_ptr; CX is size_i; DX is code; SI is code_mask_i; DI is tree_free_ptr; BP is the address of end of global_uninitvars.
%if LEAF_PTR
		mov ax, LEAF_PTR  ; AX := LEAF_PTR.
%else
		xor ax, ax  ; AX := LEAF_PTR.
%endif
		push bx  ; Save build_root_ptr.
.next_level_down:
		cmp [bx], ax  ; AX == LEAF_PTR.
		jne short .init_leaf_done  ; Jump iff node_ptr[0] != LEAF_PTR.
		mov [bx], di  ; node_ptr[0] = DI (tree_free_ptr).
		stosw  ; *tree_free_ptr++ = LEAF_PTR.
		times 2 inc di  ; Skip setting the value of child 0 to BAD_LEAF_VALUE, because the next iteration in the first descent will overwrite it anyway.
		;stosw  ; This is an intentially missed size optimization opportunity. Keeping the longer version above instead for speed. This code is shorter and slower than `times 2 inc di' above, and setting the value to anything doesn't hurt.
		mov [bx+2], di  ; node_ptr[1] = tree_free_ptr.
		stosw  ; *tree_free_ptr++ = LEAF_PTR.
%if ((LEAF_PTR-1)&0xffff)==(BAD_LEAF_VALUE&0xffff)  ; Usually true.
		dec ax
		stosw  ; *tree_free_ptr++ = BAD_LEAF_VALUE.
		inc ax  ; AX := LEAF_PTR.
%else
		mov ax, BAD_LEAF_VALUE
		stosw  ; *tree_free_ptr++ = BAD_LEAF_VALUE.
  %if LEAF_PTR
		mov ax, LEAF_PTR  ; AX := LEAF_PTR.
  %else
		xor ax, ax  ; AX := LEAF_PTR.
  %endif
%endif
.init_leaf_done:
		test dx, si
		jz short .node_ptr_ok
		times 2 inc bx  ; if ((code & code_mask_i) != 0) ++node_ptr;
		;lodsw  ; Shorter and slower than `times 2 inc si' above, and loading the value to AX doesn't hurt.
.node_ptr_ok:
		mov bx, [bx]
		shr si, 1  ; SI (code_mask_i) >>= 1.
		jnz short .next_level_down
		mov [bx+2], cx  ; node_ptr[1] := CX (size_i).
		pop bx  ; Restore BX := build_root_ptr.
		pop dx  ; Restore DX := build_size_remaining.
		pop si  ; Restore SI := build_bit_count_ary_ptr+size_i+1.
.maybe_next_insert:
		inc cx
		dec dx
		jnz short .next_insert
%if LEAF_PTR==0  ; Usually true.
		cmp [bx], dx
%elif (LEAF_PTR>=-0x80 && LEAF_PTR<=0x7f) || (LEAF_PTR>=0xff80 && LEAF_PTR<=0xffff)
		cmp word [bx], strict byte LEAF_PTR&0xff
%else
		cmp word [bx], strict word LEAF_PTR
%endif
		je short fatal_corrupted_input  ; Fail (and avoid infinite loop) if all bit_count values are 0.
		ret

; Reads bits from stdin using the Huffman tree specified in BX, and decodes
; a single value (nonnegative integer).
;
; Input: BX pointer to the root node of the Huffman tree.
; Output: AX: the Huffman code value.
; Ruins BX, FLAGS; keeps all other registers intact.
read_using_huffman_tree:
%define IS_ADJACENT_BITBUF 1
		xchg ax, GSMALL(bitbuf)  ; AL := bitbuf; AH := bitbuf_bits_remaining; save old AX to GSMALL(bitbuf).
.next_node:
%if (LEAF_PTR>=-0x80 && LEAF_PTR<=0x7f) || (LEAF_PTR>=0xff80 && LEAF_PTR<=0xffff)
		cmp word [bx], strict byte LEAF_PTR&0xff
%else
		cmp word [bx], strict word LEAF_PTR
%endif
		je short .done
		dec ah  ; bitbuf_bits_remaining -= 1.
		jns short .bit_ok
		call read_byte  ; AL (bitbuf) := byte_read; AH (bitbuf_bits_remaining) := 0.
		mov ah, 7  ; bitbuf_bits_remaining := 7.
.bit_ok:
		shr al, 1  ; bitbuf >>= 1.
		jnc short .next_ptr_ok
		times 2 inc bx
.next_ptr_ok:
		mov bx, [bx]
		jmp short .next_node
.done:
		xchg GSMALL(bitbuf), ax  ; Save to bitbuf and bitbuf_bits_remaining; restore AX from GSMALL(bitbuf).
		mov ax, [bx+2]
		ret

; Jump or call this function, it doesn't return.
fatal_corrupted_input:
		mov dx, fatal_msgs.corrupted_input
		mov cl, fatal_msgs.corrupted_input.end-fatal_msgs.corrupted_input
		; Fall through to fatal.

; Jump or call this function, it doesn't return.
; Input: DX is a pointer to the error message; CL is byte size of error message.
fatal:
		mov ch, 0
		; Now: DX is message data start; CX is message length.
		mov bx, STDERR_FILENO
		mov ah, 0x40  ; AH := DOS syscall number for write using handle.
		int 0x21  ; DOS syscall.
		; Ignore syscall error indication in CF.
		mov ax, 0x4c<<8|EXIT_FAILURE  ; AH := DOS syscall number exit with an exit code; AL := exit code.
		int 0x21  ; DOS syscall.
		; Not reached.

; Input: DI is the output pointer; AL is the byte to write.
; Output: DI is 1 byte after the byte just written.
; Ruins FLAGS; keeps all other registers intact.
write_byte:
		cmp di, strict word global.write_buffer+WRITE_RING_BUFFER_SIZE
		jne short flush_write_buffer_and_write_byte.append
		; Fall through to flush_write_buffer_and_write_byte.

; Input: DI points to the end (byte after) of global.write_buffer to write; AL is the byte to write.
; Output: DI is 1 byte after the byte just written.
; Ruins FLAGS; keeps all other registers intact.
flush_write_buffer_and_write_byte:
		push cx  ; Save.
		lea cx, [word di-(global.write_buffer-$$)-BASE]  ; CX := size == number of bytes to flush.
		mov di, global.write_buffer
%ifndef USE_DEBUG  ; Since debug messages go to stdout, don't write the regular output file.
		jcxz .done  ; Avoid writing 0 bytes. That would make DOS truncate the file instead of writing to it.
		push bx  ; Save.
		push ax  ; Save.
		push dx  ; Save.
		; DOS can write >=0x8000 (or even 0xffff) bytes at a time,
		; but when porting to other 16-bit systems we may want to
		; break the 0x8000-byte write to two 0x4000-byte writes to
		; avoid calling write(...) with a negative size (count).
		mov bx, STDOUT_FILENO
		mov dx, di  ; DX := global.write_buffer.
.write_more:
		mov ah, 0x40  ; DOS syscall number for write using handle.
		int 0x21  ; DOS syscall.
		jc short fatal_write
		test ax, ax
		jz short fatal_write
		sub cx, ax
		jnz short .write_more
		pop dx  ; Restore.
		pop ax  ; Restore.
		pop bx  ; Restore.
.done:
%endif
		pop cx  ; Restore.
.append:
		stosb
		ret

%ifndef USE_DEBUG
fatal_write:
		mov dx, fatal_msgs.write_error
		mov cl, fatal_msgs.write_error.end-fatal_msgs.write_error
		jmp short fatal
%endif

; Output: AL is byte read.
; Ruins AH, FLAGS; keeps all other registers intact.
read_byte:
		mov ax, GSMALL(read_ptr)
		cmp ax, GSMALL(read_limit)
		jne short clear_read_buffer_and_read_byte.read_from_buffer
		; Fall through to clear_read_buffer_and_read_byte.

; Output: AL is byte read.
; Ruins AH, FLAGS; keeps all other registers intact.
clear_read_buffer_and_read_byte:
		push bx  ; Save.
		push cx  ; Save.
		push dx  ; Save.
		mov ah, 0x3f  ; DOS syscall number for read using handle.
%if STDIN_FILENO  ; False, added for completeness only.
		mov bx, STDIN_FILENO
%else
		xor bx, bx  ; BX := STDIN_FILENO.
%endif
		mov cx, READ_BUFFER_SIZE
		mov dx, global.read_buffer
		int 0x21  ; DOS syscall.
		jnc short .read_ok  ; Jump iff there wasn't a syscall error above.
		mov dx, fatal_msgs.read_error
		mov cl, fatal_msgs.read_error.end-fatal_msgs.read_error
		jmp short fatal
.read_ok:	test ax, ax
		jz short fatal_corrupted_input  ; Unexpected EOF.
		xchg ax, dx  ; AX := global.read_buffer; DX := number of bytes read. .read_from_buffer below will set word GSMALL(read_ptr) := global.read_buffer+1.
		add dx, ax
		mov GSMALL(read_limit), dx
		pop dx  ; Restore.
		pop cx  ; Restore.
		pop bx  ; Restore.
.read_from_buffer:
		push si  ; Save.
		xchg si, ax  ; SI := read_ptr; AX := junk.
		lodsb  ; AL := read_ptr[0]; SI += 1.
		mov GSMALL(read_ptr), si
		pop si  ; Restore.
		ret

; Output: AX is unsigned integer with the bits read; AH is always 0.
; Ruins FLAGS; keeps all other registers intact.
read_bits_3:
		mov al, 3
		; Fall through to read_bits_max_8.

; Input: AL is the number of bits to read (bit_count), 0..8.
; Output: AX is unsigned integer with the bits read; AH is always 0.
; Ruins FLAGS; keeps all other registers intact.
read_bits_max_8:
%define IS_ADJACENT_BITBUF 1
		push bx  ; Save.
		push cx  ; Save.
		xchg cx, ax  ; CL := AL (bit_count); CH := junk; AX := junk.
		mov ax, GSMALL(bitbuf)  ; AL := old_bitbuf; AH := bitbuf_bits_remaining. This has the adjacent bitbuf requirement.
		mov bx, 1
		cmp ah, cl  ; AH (bitbuf_bits_remaining) < CL ?
		jnb short .read_from_bitbuf
		mov ch, al  ; CH := old_bitbuf.
		sub cl, ah  ; CL (bit_count) -= AH (bitbuf_bits_remaining).
		call read_byte  ; AL (new_bitbuf) := read_byte(); AH := junk.
		mov ah, al  ; AH := new_bitbuf.
		shr ah, cl  ; AH (new_bitbuf) >>= bit_count.
		mov GSMALL(bitbuf), ah
		shl bx, cl
		dec bx  ; BX := (1<<bit_count)-1. Also sets BH := 0.
		and ax, bx  ; AX := new_bitbuf&((1<<bit_count)-1). Also sets AH := 0 because BH == 0.
		mov bl, 8
		sub bl, cl  ; BL := 8-bit_count.
		mov cl, GSMALL(bitbuf_bits_remaining)
		shl al, cl  ; AL := (new_bitbuf&((1<<bit_count)-1))<<bitbuf_bits_remaining.
		or al, ch  ; AL (low byte of result) := old_bitbuf | (new_bitbuf&((1<<bit_count)-1))<<bitbuf_bits_remaining.
		mov GSMALL(bitbuf_bits_remaining), bl  ; bitbuf_bits_remaining := 8-bit_count.
		jmp short .done
.read_from_bitbuf:
		shl bx, cl
		dec bx  ; BX := (1<<bit_count)-1. Also sets BH := 0.
		and ax, bx  ; Also sets AH := 0 because BH == 0.
		shr byte GSMALL(bitbuf), cl
		sub GSMALL(bitbuf_bits_remaining), cl
.done:
		pop cx  ; Restore.
		pop bx  ; Restore.
		ret

; Output: AX is unsigned integer with the bits read.
; Ruins FLAGS; keeps all other registers intact.
read_bits_16:
		mov al, 16
		; Fall through to read_bits_max_16.

; Input: AL is the number of bits to read (bit_count), 0..8.
; Output: AX is unsigned integer with the bits read.
; Ruins FLAGS; keeps all other registers intact.
read_bits_max_16:
		cmp al, 8
		jna short read_bits_max_8
		push bx  ; Save.
		xchg ax, bx  ; AL := junk; AH := junk; BL := bit_count; BH := junk.
		mov al, 8
		call read_bits_max_8  ; AL := read_bits_max_8(8) (first value read); AH := 0.
		xchg ax, bx  ; AL := bit_count; AH := junk; BL := first value read; BH := 0.
		sub al, 8  ; AL := bit_count-8.
		call read_bits_max_8  ; AL := read_bits_max_8(bit_count-8) (second value read); AH := 0.
		xchg ax, bx  ; AL := first value read; AH := 0; BL := second value read; BH := 0.
		mov ah, bl
		pop bx  ; Restore.
		ret

%ifdef USE_DEBUG
; Input: AL is byte value.
; Ruins FLAGS; keeps all other registers intact.
debug_hex_byte:
		push ax
		aam 0x10
		xchg al, ah
		cmp al, 9
		jna short .digit1
		add al, 'a'-'0'-10
.digit1:
		add al, '0'
		int 0x29
		mov al, ah
		cmp al, 9
		jna short .digit2
		add al, 'a'-'0'-10
.digit2:
		add al, '0'
		int 0x29
		pop ax
		ret

; Input: AX is word value.
; Ruins FLAGS; keeps all other registers intact.
debug_hex_word:
		push ax
		xchg al, ah
		call debug_hex_byte
		mov al, ah
		call debug_hex_byte
		pop ax
		ret

; Input: AL is literal byte.
; Ruins FLAGS; keeps all other registers intact.
debug_lz_literal:
		push ax
		mov al, 'L'
		int 0x29  ; DOS syscall to write character in AL to stdout.
		pop ax
		call debug_hex_byte
		push ax
		mov al, LF
		int 0x29
		pop ax
		ret

; Input: DX is LZ match length (1..258); AX is LZ match distance (0..32767).
debug_lz_match:
		push ax
		mov al, 'M'
		int 0x29
		mov ax, dx  ; AX := LZ match length.
		call debug_hex_word
		mov al, '/'
		int 0x29
		pop ax
		call debug_hex_word
		push ax
		mov al, LF
		int 0x29
		pop ax
		ret
%endif  ; Of %ifdef DEBUG
		

; --- Read-only data.

%ifndef USE_SCRAMBLECODE
; lut_deflate_bit_count_scramble[size_i] == (size_i < 3 ? size_i + 16 : size_i == 3 ? 0 : ((size_i & 1) != 0 ? 19 - size_i : size_i + 12) >> 1).
lut_deflate_bit_count_scramble: db 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
%endif

fatal_msgs:
.read_error:	db 'read error', CR, LF
.read_error.end:
%ifndef USE_DEBUG
.write_error:	db 'write error', CR, LF
.write_error.end:
%endif
.corrupted_input: db 'corrupted input', CR, LF
.corrupted_input.end:

; -- Zero-initialized and uninitilized data. Part of the memory image, but missing from the end of the DOS .com program file.

absolute $
		resb ($$-$)&3  ; Align to dd.

global_initvars:  ; Global small (non-array) initialized variables, referenced using GSMALL(...) instead of [...] to save a byte per use.
bitbuf: resb 1  ; unsigned char. Must be the second variable. Initialized to 0.
bitbuf_bits_remaining: resb 1  ; unsigned char. Must be the first variable. Initialized to 0. Always 0..7. This must be exactly 1 byte after .bitbuf, for read_using_huffman_tree and read_bits_max_8.
match_distance_limit: resw 1  ; unsigned short. Initialized to 0.
global_uninitvars:  ; Global small (non-array) uninitialized variables, indexed using GSMALL(...) instead of [...] to save a byte per indexing.
build_bit_count_histogram_and_g_ary: resw 16  ; unsigned short[16]. Used by build_huffman_tree_RUINS as temporary storage. This must be the first variable in global_uninitvars.
read_ptr: resw 1  ; unsigned char*. Initial value arbitrary.
read_limit: resw 1  ; unsigned char*. Initial value arbitrary.
literal_and_len_root_ptr: resw 1  ; void**. Initial value arbitrary.
distance_root_ptr: resw 1  ; void**. Initial value arbitrary.

%if IS_HISTOGRAM_THE_FIRST_UNINITVAR && build_bit_count_histogram_and_g_ary!=global_uninitvars
  %error ERROR_HISTOGRAM_MUST_BE_THE_FIRST_UNINITVAR
  times -1 nop
%endif
%if IS_ADJACENT_BITBUF && bitbuf_bits_remaining!=bitbuf+1
  %error ERROR_BITBUF_VARS_MUST_BE_ADJACENT
  times -1 nop
%endif

; This array contains multiple Huffman trees (0..3). A Huffman tree is a
; binary tree (and also a trie) with values in its leaves. A non-leaf node
; always has 2 children: child 0 and child 1. The special value
; BAD_LEAF_VALUE in a leaf node indicates a bad Huffman code.
;
; To decode a value using a Huffman tree, start from the root node of the
; tree, read a single bit from the input, and for each bit read, follow the
; corresponding edge in the tree (either to child 0 or 1). Stop decoding
; when you reach a leaf node, and the result is the value in that leaf node.
; In Huffman trees for Deflate, the root node is never a leaf node, thus
; each Huffman code is at least 1 bit long.
;
; Each tree is identified by a pointer to its root node within the array.
; Each node occupies two slots (of word == unsigned short): node_ptr[0] and
; node_ptr[1]. If node_ptr[0] == LEAF_PTR, then this is a leaf node, and the
; value is in node_ptr[1], with special invalid value BAD_LEAF_VALUE
; indicating an invalid Huffman code. Otherwise, this is a non-leaf node
; with two children: node_ptr[0] is a pointer to child 0, and node_ptr[1] is
; a pointer to child 1.
global.huffman_trees_ary: resw MAX_TREE_SIZE
global.huffman_bit_count_ary: resb 256+32+30  ; unsigned char[256+32+30]. unsigned char[318].
global.write_buffer: resb WRITE_RING_BUFFER_SIZE
global.read_buffer: resb READ_BUFFER_SIZE
global.read_buffer.end:

; __END__
