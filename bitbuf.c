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

void bitbuf_init_file( bitbuf *bb, const char *name ) {

    FILE *fp = fopen( name, "rb" );
    if( fp == NULL )
        die( "init_file: %s does not exist", name );

    bitbuf_read_file( bb, fp );
    fclose( fp );
}

void bitbuf_init_str( bitbuf *bb, const char *str ) {
    char *in = strdup( str );
    char *org = in;
    char *token;

    while( ( token = strsep( &in, " " ) ) != NULL ) {
        if( strlen( token ) <= 2 )
            die( "init_str: Please provide the string in 0b... | 0x... format" );
        else if( token[ 1 ] == 'x' )
            bitbuf_add_str_hex( bb, token + 2 );
        else if( token[ 1 ] == 'b' )
            bitbuf_add_str_bin( bb, token + 2 );
    }
}

void bitbuf_release( bitbuf *bb ) {

    if( bb->alloc) {
        free( bb->buf );
        bitbuf_init( bb, 0 );
    }
}

void bitbuf_copy( bitbuf *dest, bitbuf *src ) { 
    
    bitbuf_reset( dest );
    bitbuf_append_buf( dest, src );
}

unsigned char* bitbuf_detach( bitbuf *bb, size_t *sz ) {

    unsigned char *res = bb->buf;

    if( sz )
        *sz = bb->len;

    bb->buf = NULL;
    bb->len = bb->alloc = 0;
    return res;
}

void bitbuf_attach( bitbuf *bb, void *data, size_t len ) {
    if( bb->alloc ) 
        bitbuf_release( bb );

    size_t sz = len * 8;
    bb->buf   = data;
    bb->len   = sz;
    bb->alloc = sz;
}

void bitbuf_grow( bitbuf *bb, size_t extra ) {

    size_t new = BYTE_LEN( bb->alloc + extra );
    size_t old = BYTE_LEN( bb->alloc );

    int isNew = !( bb->alloc );
    if( isNew )
        bb->buf = NULL;

    if( ( bb->buf = ( unsigned char * )realloc( bb->buf, new ) ) == NULL ) {
        die( "grow: Could not allocate more buffer space" );
    } else {
        memset( bb->buf + old, 0, new - old );
        bb->alloc = new * 8;
    }
}
