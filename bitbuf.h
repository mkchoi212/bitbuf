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

static void panic( const char *fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    fprintf( stderr, fmt, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
    exit( EXIT_FAILURE );
}
