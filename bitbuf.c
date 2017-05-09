#include "bitbuf.h"

unsigned char bitbuf_slopbuf[ 10 ];
unsigned char getbit( unsigned long int, size_t );

void bitbuf_init( bitbuf *bb, size_t s ) {
    
    bb->buf = bitbuf_slopbuf;
    bb->len = bb->alloc = 0;

    if( s )
        bitbuf_grow( bb, s );
}

void bitbuf_zeros( bitbuf *bb, size_t s ) {

    size_t n  = BYTE_LEN( s );
    bb->buf   = ( unsigned char * )calloc( n , sizeof ( unsigned char ) );
    bb->len   = s;
    bb->alloc = n * 8;
}

void bitbuf_grow( bitbuf *bb, size_t extra ) {

    size_t new = BYTE_LEN( bb->alloc + extra );
    size_t old = BYTE_LEN( bb->alloc );

    int isNew = !( bb->alloc );
    if( isNew )
        bb->buf = NULL;

    if( ( bb->buf = ( unsigned char * )realloc( bb->buf, new ) ) == NULL ) {
        die( "bitbuf_grow: Could not allocate more buffer space" );
    } else {
        memset( bb->buf + old, 0, new - old );
        bb->alloc = new * 8;
    }
}
