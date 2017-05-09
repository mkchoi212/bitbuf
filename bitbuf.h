#ifndef BITBUF_H
#define BITBUF_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef struct _bitbuf {

    size_t alloc;
    size_t len;
    unsigned char *buf;
} bitbuf;

#define BITBUF_INIT { 0, 0, bitbuf_slopbuf }
#define BYTE_LEN( n ) ( n + 7 ) / 8

static void die( const char *fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    fprintf( stderr, fmt, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
    exit( EXIT_FAILURE );
}


/**
 * Used as desfualt ->buf value to ensure
 * there is always something that acts as ->buf
 */
extern unsigned char bitbuf_slopbuf[];

/**
 * Function pointer passed into 'bitbuf_op'
 * Function _must_ take two bytes and yield a single byte
 */
typedef unsigned char (*OperatorFunc)(unsigned char, unsigned char);

/**
 * Life-cycle Functions
 * ____________________________________________
 */

/**
 * Initialize the structure. You can allocate as much as 
 * you wish to prevent future reallocs
 */
extern void bitbuf_init( bitbuf *, size_t );

/**
 * Initialize buffer with content from the provided
 * file name
 */
extern void bitbuf_init_file( bitbuf *, const char * );

/**
 * Create buffer with all zeroes
 */
extern void bitbuf_zeros( bitbuf *, size_t );
/**
 * Set all values to 0 and reset the length while maintaining
 * originally allocated storage.
 * !!! Useful when reusing existing buffers
 */
extern void bitbuf_reset( bitbuf * );

/**
 * Release the byte array from bitbuf and the memory it used.
 * You should not use the structure after calling this function.
 * unless you reinitialzie it
 */
extern void bitbuf_release( bitbuf * );

/**
 * Attach a byte array to the structure.
 * The array __must__ have been malloc()ed before being attached and can't
 * be free()ed directly
 */
extern void bitbuf_attach( bitbuf *, void *buf, size_t size );

/**
 * Detach the buffer from the structure and return it while getting its size/
 * You now own the bit array and its your responsibility to __release__ it
 * with free()
 */
extern unsigned char * bitbuf_detach( bitbuf *, size_t * );

/**
 * Basic operations
 * ____________________________________________
 */

/**
 * Copy the structure 
 */
extern void bitbuf_copy( bitbuf *dest, bitbuf *src );

/**
 * Swap contents of two buffers
 */
static inline void bitbuf_swap( bitbuf *a, bitbuf *b ) {
    bitbuf *tmp = a;
    *a = *b;
    *b = *tmp;
}

/**
 * Buffer size
 * ____________________________________________
 */

/**
 * Ensure that a provided amount of memery is available.
 * Typically used when the size of the data is known before-hand
 * and wish to repetitive realloc()s for performance
 */
extern void bitbuf_grow( bitbuf *, size_t );

/**
 * Determine amount of allocated but unused memory
 */
static inline size_t bitbuf_avail( bitbuf *bb ) {
    return bb->alloc > bb->len ? bb->alloc - bb->len : 0;
}

/**
 * Sets the length of the structure to a given value.
 * This does __NOT__ allocate or free memory and should not set the length
 * to a value biffer than `len` + `bitbuf_avail()`.
 * 
 * Can be used to adjust buffer length while retaining all buffer data
 */
static inline void bitbuf_setlen( bitbuf *b, size_t len ) {
    if( len > ( b->alloc ? b->alloc : 0 ) )
        die( "bitbuf_setlen() beyond buffer" );

    b->len = len;
}

#define bitbuf_resetlen( b ) bitbuf_setlen( b, 0 )

/**
 * Buffer content
 * ____________________________________________
 */

/**
 * Get the Hamming weight (number of 1's ) of the structure
 */
static inline size_t bitbuf_weight( bitbuf *b ) {
    size_t i, cnt = 0;

    for( i = 0; i < BYTE_LEN( b->len ); ++i ) 
        cnt += __builtin_popcount( b->buf[ i ] );

    return cnt;
}

/**
 * Locate a bit needle in the  bit haystack starting from the provided offset
 * Set garble value for permitted bit error count
 * Return -1 if nothing is found
 */
extern int bitbuf_fpat( bitbuf *src, bitbuf *pat, size_t garble, size_t offset );

/**
 * Compare two buffers
 * Returns zero if two buffers are identical, otherwise returns the difference
 * between the first two differing bytes
 */
extern int bitbuf_cmp( bitbuf *, bitbuf * );

/**
 * Slice n elements of the src buffer and store in dest buffer
 * starting from the provided index
 */
extern void bitbuf_slice( bitbuf *dest, bitbuf *src, size_t start, size_t n );

/**
 * Align two buffers by padding the shorter buffer with zeros
 */
extern void bitbuf_align( bitbuf *, bitbuf * );

/**
 * Get / set a signle bit at a specific zero-indexed bit position
 */
extern unsigned char bitbuf_getbit( const bitbuf *, size_t );

extern void bitbuf_setbit( bitbuf *b, int bit, size_t idx );

/**
 * Adding data
 * ____________________________________________
 * __NOTE__ that functions here will grow the buffer as needed
 * If it fails other than lack of memory, it will be free()ed
 */

/**
 * Add a single bit / byte to the end of the buffer
 */
extern void bitbuf_addbit( bitbuf *, int );

extern void bitbuf_addbyte( bitbuf *, unsigned char );

/**
 * Append contents of src buffer to the dest buffer
 */
extern void bitbuf_append_buf( bitbuf *dest, bitbuf *src );

/**
 * I/O Functions
 * ____________________________________________
 */

/**
 * Print the buffer to stdout
 * Useful for inspecting the buffer while debugging
 *
 * Format: 0xDATA 0bLEFT_OVER_BITS
 */
extern void bitbuf_dump( bitbuf * );

/**
 * Read file and store all data in buffer and return number of bits read
 */
extern size_t bitbuf_read_file( bitbuf *, FILE * );

/**
 * Write buffer to file and return number of bits written
 * Returns value less than `len` only if a write error occured
 */
extern size_t bitbuf_write_file( bitbuf *, FILE * );

/**
 * Modifying the buffer
 * ____________________________________________
 */

/**
 * Reverse n number of bits at the provided index __in place__
 */
extern void bitbuf_reverse( bitbuf *, size_t start, size_t n );

/**
 * Reverse all bits in the buffer by n bits __in place__
 */
extern void bitbuf_reverse_all( bitbuf *, size_t n );

/**
 * Left / right shift
 */
extern void bitbuf_lsh( bitbuf *, size_t );

extern void bitbuf_rsh( bitbuf *, size_t );

/**
 * Apply `OperationFunc` to two buffers and store the result in `res`
 */
extern void bitbuf_op( bitbuf *, bitbuf *, bitbuf *res, OperatorFunc op );

/**
 * Operations built on top of `bitbuf_op()`
 *
 * Addition | And | Or | Xor 
 */
extern void bitbuf_plus( bitbuf *, bitbuf *, bitbuf *res );
extern void bitbuf_and( bitbuf *, bitbuf *, bitbuf *res );
extern void bitbuf_or( bitbuf *, bitbuf *, bitbuf *res );
extern void bitbuf_xor( bitbuf *, bitbuf *, bitbuf *res );

/**
 * String to bitbuf
 * ____________________________________________
 */

/**
 * Convert string input to bitbuf
 * Provided base must be between 2 and 36 inclusive
 *
 * `dataSize` is the number of bits a single character of data signifies
 */

extern void bitbuf_append_str( bitbuf *, const char *, size_t base, size_t dataSize );

static inline void bitbuf_add_str_hex( bitbuf *b, const char *hexStr ) { 
    /* `base` is 16 for hexadecimal data
     * `dataSize` is 4 as one character of hex (nibble) is 4 bits
     */

    size_t i, c;
    for( i = 0; i < strlen( hexStr ); ++i ) {
        c = hexStr[ i ];

        if( !( ( c>=48 && c<=57 ) || ( c>=97 && c<=102 ) || ( c>=65 && c<=70 ) ) )
            die( "Non-hexadecimal digit %c found at position %i", c, i );
    }
    bitbuf_append_str( b, hexStr, 16, 4 );
}

static inline void bitbuf_add_str_bin( bitbuf *b, const char *binStr ) { 
    size_t i, c;
    for( i = 0; i < strlen( binStr ); ++i ) {
        c = binStr[ i ];

        if( !( c == 48 || c == 49 ) )
            die( "Non-binary digit %c found at position %i", c, i );
    }
    bitbuf_append_str( b, binStr, 2, 1 );
}

/**
 * Bitbuf to string
 * ____________________________________________
 * __NOTE__ Make sure big enough character buffers are provided to the following functions
 */

extern void bitbuf_bin( bitbuf *, char * );
extern void bitbuf_hex( bitbuf *, char * );
extern void bitbuf_ascii( bitbuf *, char * );

#endif
