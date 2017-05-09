#ifndef BITBUF_H
#define BITBUF_H

#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

typedef struct _bitbuf {

    size_t alloc;
    size_t len;
    unsigned char *buf;
} bitbuf;

#define BITBUF_INIT { 0, 0, bitbuf_slopbuf }
#define BYTE_LEN( n ) ( n + 7 ) / 8

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
 * Attach a byte array to the structure. Provide the array to be attached,
 * its length and the amount of malloc()ed memory.
 * The array __must__ have been malloc()ed before being attached and can't
 * be free()ed directly
 */
extern void bitbuf_attach( bitbuf *, void *buf, size_t len, size_t alloc );

/**
 * Detach the buffer from the structure and return it while getting its size/
 * You now own the bit array and its your responsibility to __release__ it
 * with free()
 */
extern unsigned char * bitbuf_detach( bitbuf *, size_t );

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
    *b = *temp;
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
static inline size_t bitbuf_avail( bitbuf *b ) {
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
        panic( "bitbuf_setlen() beyond buffer" );

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
extern void bitbuf_setbit( bitbuf *, int, size_t );


static void panic( const char *fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    fprintf( stderr, fmt, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
    exit( EXIT_FAILURE );
}
