#ifndef _BITBUF_H
#define _BITBUF_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _bitbuf {
  size_t alloc;
  size_t len;
  unsigned char *buf;
} bitbuf;

/* Used as default ->buf vlue so people can always assume
 * there is something that acts as a buffer
 */
extern unsigned char bitbuf_slopbuf[];

/* Macro used to initialize the variables in the bitbuf struct */
#define BITBUF_INIT \
  { 0, 0, bitbuf_slopbuf }

/* Least number of bytes required to fill `n` bits */
#define BYTE_LEN(n) (n + 7) / 8

/* Buffer size when `fread`ing  */
#define MAX_BUF 4096

/* popcount support for Visual Studio */
#ifdef __MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt
#endif

/**
 * Initializers
 * ______________________________________
 */

/* Initialize the structure. You can allocate as much as you want to
 * prevent future reallocs, hence improving performance
 */
void bitbuf_init(bitbuf *, size_t);

/* Initialize the structure filled with zeros */
void bitbuf_init_zero(bitbuf *, size_t);

/* Initialize the structure directly from a file name */
void bitbuf_init_file(bitbuf *, const char *);

/* Initialize the structure with a provided string constant
 * Allowed prefixs are "0b" and "0x"
 */
void bitbuf_init_str(bitbuf *, const char *);

/* Initialize from a part of another buffer
 */
void bitbuf_init_sub(bitbuf *dest, const bitbuf *src, size_t start, size_t n);

/**
 *  Memory Management
 * ______________________________________
 */

/* Set all values to 0 and reset the length while maintaining originally
 * allocated memory
 * Useful when re-using existing buffers
 */
void bitbuf_reset(bitbuf *);

/* Release the byte buffer from bitbuf and the memory it used.
 * You should __not__ use this buffer after using this function unless you
 * reinitialize it
 */
void bitbuf_release(bitbuf *);

/* Attach a byte array to the buffer. Specify the array to attach and the
 * current length
 * of the array in __BYTES__ and the amount of malloc()ed memory.
 * The array __must__ have been malloc()ed before being attached and can't be
 * free()ed directly
 */
void bitbuf_attach(bitbuf *, const void *, size_t len, size_t alloc);

/* Detach the bits from the structure and return it while also getting its size
 * You now own the storage the bit occupies and is your responsibility to free()
 * it
 */
unsigned char *bitbuf_detach(bitbuf *, size_t *);

/* Copy contents of the buffer */
void bitbuf_copy(bitbuf *dest, const bitbuf *src);

/* Swap the contents */
static inline void bitbuf_swap(bitbuf *a, bitbuf *b) {
  bitbuf *tmp = a;
  *a = *b;
  *b = *tmp;
}

/* Determine the amount of allocated but unused memory */
static inline size_t bitbuf_avail(bitbuf *bb) {
  return bb->alloc > bb->len ? bb->alloc - bb->len : 0;
}

/* Ensure that at least this amount of memory is available
 * Used when you typically know the size of data and want to avoid repetitive
 * realloc()s
 * Should not be used often but can be useful for performance
 */
void bitbuf_grow(bitbuf *, size_t);

/* Set the length of buffer
 * This does *NOT* allocate new memory and should not perform a
 * `bitbuf_setlen()` to a
 * length biffer than `len` + `bitbuf_avail()`
 * This is just meant for `please fix invariants from this bitbuf I messed with`
 */
void bitbuf_setlen(bitbuf *bb, size_t len);

/* Empty the buffer by setting the size to zero */
#define bitbuf_resetlen(bb) bitbuf_setlen(bb, 0)

/**
 * Buffer Contents
 * ______________________________________
 */

/* Get the number of 1's (Hamming Weight) */
size_t bitbuf_weight(const bitbuf *);

/* Find a pattern within the src buffer and return the matching index
 * If no patterns are found, return -1
 */
int bitbuf_find(const bitbuf *src, const bitbuf *pat, size_t garble,
                size_t offset);

/* Replace the first occurence of `old` with `fresh`
 * Returns the number of patterns replaced
 */
int bitbuf_replace(bitbuf *src, const bitbuf *old, const bitbuf *fresh,
                   size_t garble, size_t start, size_t end);

/* Compare two buffers. Return an integer less than, equal to or greater than
 * zero if the first
 * buffer is found to be respectively less than, equal to or greater than the
 * second buffer
 */
int bitbuf_cmp(const bitbuf *, const bitbuf *);

/* Slice `n` elements to `dest` starting from the provided index */
void bitbuf_slice(bitbuf *dest, const bitbuf *src, size_t start, size_t n);

/* Zero is added as padding to the shorter buffer to match the length of the
 * longer one */
void bitbuf_align(bitbuf *, bitbuf *);

/* Get / set a single bit at a specific zero-indexed position */
unsigned char bitbuf_getbit(const bitbuf *, size_t);
void bitbuf_setbit(bitbuf *, size_t, int);

/* Get / set a single byte with an offset going from left to right (MSB) */
unsigned char bitbuf_getbyte(const bitbuf *, size_t pos, size_t offset);
void bitbuf_setbyte(bitbuf *, size_t pos, size_t offset, unsigned char byte);

/**
 * Adding data
 * ______________________________________
 *
 * __NOTE__ that all functions here will grow the buffer as needed
 * If it fails other than lack of memory, it will be free()ed
 */

/* Add a single bit / byte to the end of the buffer */
void bitbuf_addbit(bitbuf *, int);
void bitbuf_addbyte(bitbuf *, unsigned char);

/* Append the `dest` buffer to the end of the `src` buffer */
void bitbuf_addbuf(bitbuf *dest, const bitbuf *src);

/* Convert string data to bits and add to buffer
 * Provided base must be between 2 and 36 inclusive
 * `base` is the base in which the string is in
 * `unit` is the number of bits a single character of provided string requires
 */
void bitbuf_addstr(bitbuf *, const char *, size_t base, size_t unit);
void bitbuf_addstr_bin(bitbuf *, const char *);
void bitbuf_addstr_hex(bitbuf *, const char *);

/* Insert buffer or bit after the specified index */
void bitbuf_insert(bitbuf *dest, const bitbuf *src, size_t idx);
static inline void bitbuf_insert_bit(bitbuf *dest, const int bit, size_t idx) {
  bitbuf src = BITBUF_INIT;
  bitbuf_init(&src, 1);
  bitbuf_addbit(&src, bit);
  bitbuf_insert(dest, &src, idx);
  bitbuf_release(&src);
}

/* Append buffer at the beginning of the `dest` buffer
 * Equivalent to `bitbuf_insert(dest, src, 0)`
 */
void bitbuf_prependbuf(bitbuf *dest, bitbuf *src);

/**
 * Operations
 * ______________________________________
 */

/* Used for passing in function pointers that represent operators to bitbuf_op()
 * Function must take in two seperate bytes and yield a single byte
 */
typedef unsigned char (*OperatorPtr)(unsigned char, unsigned char);

/* Pass in a function pointer of siginture `OperatorPtr` for two bitbuf's to be
 * evaluated */
void bitbuf_op(const bitbuf *, const bitbuf *, bitbuf *res, OperatorPtr op);

/* Basic operations built with bitbuf_op */
void bitbuf_and(const bitbuf *, const bitbuf *, bitbuf *res);
void bitbuf_or(const bitbuf *, const bitbuf *, bitbuf *res);
void bitbuf_xor(const bitbuf *, const bitbuf *, bitbuf *res);

/* Add the two buffers in the numeric sense */
void bitbuf_plus(const bitbuf *, const bitbuf *, bitbuf *res);

/* Reverse `n` number of bits from the provided index */
void bitbuf_reverse(bitbuf *, size_t start, size_t n);

/* Reverse all bits in the buffer by `unit` bits */
void bitbuf_reverse_all(bitbuf *, size_t unit);

/* Left and right shift */
void bitbuf_lsh(bitbuf *, size_t);
void bitbuf_rsh(bitbuf *, size_t);

/**
 * Conversions
 * ______________________________________
 *
 * __NOTE__ Make sure to make a big enough char array to these functions
 * Use `BYTE_LEN( bb.len ) + 1` as a reference
 */
void bitbuf_bin(const bitbuf *, char *);
void bitbuf_hex(const bitbuf *, char *);
void bitbuf_ascii(const bitbuf *, char *);

/* Convert buffer into a human readable format
 * The char array is allocated dynamically and must be free()ed
 */
char *bitbuf_rep(bitbuf *);

/* TODO REFER TO INT_MAX
 * Numeric representation of the buffer
 * Maximum number supported is 4 bytes or 4,294,967,295
 */
unsigned long bitbuf_num(const bitbuf *);

/**
 * I / O
 * ______________________________________
 */

/* Dump `bitbuf_rep` to stdout
 * Useful when debugging
 */
void bitbuf_dump(bitbuf *);

/* Read binary file into the buffer and
 * return the number of BYTES read */
size_t bitbuf_read(bitbuf *, FILE *);

/* Write the buffer to a file and
 * return the number of BYTES written
 */
size_t bitbuf_write(bitbuf *, FILE *);

/**
 * Utilities
 * ______________________________________
 */
static inline int max(int a, int b) { return a > b ? a : b; }

static inline int min(int a, int b) { return a > b ? b : a; }

static inline unsigned char getbit(unsigned long data, size_t n) {
  return ((data >> n) & 0x01) == 1;
}

#endif
