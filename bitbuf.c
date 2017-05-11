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

int bitbuf_cmp( bitbuf *a, bitbuf *b ) {
    size_t clen = a->len > b->len ? b->len : a->len;
    return memcmp( a->buf, b->buf, BYTE_LEN( clen ) );
}

int bitbuf_fpat( bitbuf *src, bitbuf *pat, size_t garble, size_t offset ) {

    if( pat->len > src->len - offset )
        return -1;

    int hit = -1;
    size_t i, cursor, winSz, patSz, weight;
    patSz = BYTE_LEN( pat->len );
    winSz = patSz + 1;  // one byte larger to compensate for overflow during slicing

    unsigned char temp[ winSz ];
    memset( temp, 0, winSz );

    bitbuf window = BITBUF_INIT;
    bitbuf_attach( &window, temp, winSz );

    for( cursor = offset; cursor <= src->len - pat->len; ++cursor ) {
        
        weight = 0;
        bitbuf_slice( &window, src, cursor, pat->len );

        for( i = 0; i < patSz; ++i ) {
            weight += __builtin_popcount( window.buf[ i ] );

            if( weight > garble ) {
                bitbuf_resetlen( &window );
                break;
            }
        }

        if( i == patSz ) {
            hit = cursor;
            break;
        }
    }

    return hit;
}

void bitbuf_slice( bitbuf *dest, bitbuf *src, size_t start, size_t n ) {

    if( start + n > src->len ) 
        die( "slice: Out of bounds" );

    size_t width = BYTE_LEN( n + ( start % 8 ) ) * 8;
    if( width > dest->alloc )
        bitbuf_grow( dest, width - dest->alloc );

    memcpy( dest->buf, &src->buf[ start / 8 ], width / 8 );
    dest->len = width;

    /* Clean trash bits at the LSB */
    bitbuf_lsh( dest, start % 8 );

    /* Clean trash bits at the MSB */
    size_t trashShift = 8 - ( n % 8 );
    dest->buf[ n / 8 ] = dest->buf[ n / 8 ] >> trashShift << trashShift;
}

unsigned char bitbuf_getbit( const bitbuf *bb, size_t n ) {

    if( n > bb->len )
        die( "getbit: Out of bounds" );

    size_t byteLoc = n / 8;
    size_t bitLoc  = 7 - ( n % 8 );
    return getbit( bb->buf[ byteLoc ], bitLoc );
}

void bitbuf_setbit( bitbuf *bb, size_t n, int bit ) {
    if( n > bb->len )
        die( "setbit: Out of bounds" );

    size_t byteLoc = n / 8;
    size_t bitLoc  = 7 - ( n % 8 );

    if( bit )
        bb->buf[ byteLoc ] |= 0x1;
    else
        bb->buf[ byteLoc ] &= 0x0;
}

void bitbuf_addbyte( bitbuf *bb, unsigned char byte ) {

    if( bb->len + 8 > bb->alloc ) 
        bitbuf_grow( bb, bb->len * 2 );

    size_t pad = bb->len % 8;
    size_t bytePos = bb->len / 8;

    if( pad == 0 ) {
        bb->buf[ bytePos ] = byte;
    } else {
        bb->buf[ bytePos ] |= byte >> pad;
        bb->buf[ bytePos + 1 ] = byte << ( 8 - pad );
    }

    bb->len += 8;
}

void bitbuf_addbit( bitbuf *bb, int data ) {

    if( !bitbuf_avail( bb ) ) 
        bitbuf_grow( bb, bb->len * 2 + 1 );

    bitbuf_setbit( bb, bb->len, data );
    bb->len++;
}

void bitbuf_append_buf( bitbuf *dest, bitbuf *src ) {

    if( src->len > dest->alloc - dest->len )
        bitbuf_grow( dest, src->len );

    size_t pad, trash;
    pad   = dest->len % 8;
    trash = 8 - pad;
    
    /* Insert bits to dest and prepare src buf for copying */
    dest->buf[ dest->len / 8 ] |= src->buf[ 0 ] >> pad;

    size_t copyLen = src->len;
    if( pad != 0 ) {
        bitbuf_lsh( src, trash );
        copyLen -= trash;
    }

    memcpy( dest->buf + BYTE_LEN( dest->len ), src->buf, BYTE_LEN( copyLen ) );
    dest->len += src->len;
}

void bitbuf_dump( bitbuf *bb ) {

    /* Modify bufffer's length to unambiguate its length
     * while converting to hex
     */

    size_t bufLen = bb->len;
    bb->len = bufLen - ( bufLen % 4 );

    char hex[ BYTE_LEN( bb->len ) * 2 ];
    bitbuf_hex( bb, hex );
    printf( "0x%s", hex );
    bb->len = bufLen;

    /* Print leftovers in 0b format*/
    size_t binLen = bufLen % 4;
    if( binLen ) {
        char bin[ 4 ];
        bitbuf binBuf = BITBUF_INIT;
        bitbuf_slice( &binBuf, bb, bb->len - binLen, binLen );
        bitbuf_bin( &binBuf, bin );
        printf( " 0b%s", bin );
        bitbuf_release( &binBuf );
    }
    
    printf( "\n" );
}

size_t bitbuf_read_file( bitbuf *bb, FILE *fp ) {
    fseek( fp, 0, SEEK_END );
    size_t fsize = ftell( fp );
    size_t bitLen = fsize * 8;
    rewind( fp );
    
    if( bitbuf_avail( bb ) < bitLen )
        bitbuf_grow( bb, bitLen - bitbuf_avail( bb ) );

    /* Read in large blocks when buffer is aligned */
    if( bb->len % 8 == 0 ) {
        fread( bb->buf + BYTE_LEN( bb->len ), fsize, 8, fp );
        bb->len = bb->len + bitLen;
    } else {
        int data;
        while( ( data = fgetc( fp ) ) != EOF ) 
            bitbuf_addbyte( bb, data );
    }

    return bitLen;
}

size_t bitbuf_write_file( bitbuf *bb, FILE *fp ) {
    return bb->len ? fwrite( bb->buf, 1, BYTE_LEN( bb->len ), fp ) : 0;
}

// TODO
void bitbuf_reverse( bitbuf *bb, size_t start, size_t n ) {
    size_t i, j;
    unsigned char head, tail;

    for( i = start, j = start + n - 1; i < bb->len; i += n ) 
        bitbuf_reverse( bb, i, n);
}

/* TODO MEH?? */
void bitbuf_lsh( bitbuf *bb, size_t n ) { 
    
    if( n > bb->len ) {
        memset( bb->buf, 0, BYTE_LEN( bb->len ) );
        n = 0;
    }
    
    int skip = n / 8;
    if( skip ) {
        memmove( bb->buf, bb->buf + skip, BYTE_LEN( bb->len ) - skip );
        memset( bb->buf + ( BYTE_LEN( bb->len ) - skip ), 0, skip ); 
    }

    size_t left = n % 8;
    if( left ) {
        size_t trash = 8 - left;
        unsigned char trail;
    }
}

void bitbuf_align( bitbuf *b1, bitbuf *b2 ) {
    
    bitbuf *lg = b1->len > b2->len ? b1 : b2;
    bitbuf *sm = b1->len > b2->len ? b2 : b1;
   
    size_t diff = lg->len - sm->len;
    bitbuf_grow( sm, diff );
    bitbuf_setlen( sm, lg->len );
    bitbuf_rsh( sm, diff );
}

void bitbuf_op( bitbuf *b1, bitbuf *b2, bitbuf *res, OperatorFunc op ) {
    if( b1->len != b2->len )
        die( "op: Buffers should be of same length to perform operations" );

    if( res->alloc < b1->len + 1 ) 
        bitbuf_grow( res, b1->len - res->alloc + 8 );

    size_t i;
    //meh
}

unsigned char xor( unsigned char a, unsigned char b ) { return a ^ b; }
void bitbuf_xor( bitbuf *b1, bitbuf *b2, bitbuf *res ) {
    OperatorFunc xorFunc = &xor;
    bitbuf_op( b1, b2, res, xorFunc );
}

unsigned char or( unsigned char a, unsigned char b )  { return a | b; }
void bitbuf_or( bitbuf *b1, bitbuf *b2, bitbuf *res ) {
    OperatorFunc orFunc = &or;
    bitbuf_op( b1, b2, res, orFunc );
}

unsigned char and( unsigned char a, unsigned char b ) { return a & b; }
void bitbuf_and( bitbuf *b1, bitbuf *b2, bitbuf *res ) {
    OperatorFunc andFunc = &and;
    bitbuf_op( b1, b2, res, andFunc );
}

void bitbuf_plus( bitbuf *b1, bitbuf *b2, bitbuf *res ) {
    //TODO
}


void bitbuf_append_str( bitbuf *bb, const char *data, size_t base, size_t unitLen ) {
    size_t MAX_STR_SZ = sizeof( unsigned long int ) * 8 / unitLen;

    char strSeg[ MAX_STR_SZ + 1 ];
}

void bitbuf_bin( bitbuf *bb, char *buf ) {
    size_t i;
    unsigned char bit;

    for( i = 0; i< bb->len; ++i ){
        bit = bitbuf_getbit( bb, i );

        if( bit == 0x01 )
            buf[ i ] = '1';
        else
            buf[ i ] = '0';
    }

    buf[ i ] = '\0';
}

void bitbuf_hex( bitbuf *bb, char *str ) {
    size_t i;
    for( i = 0; i < bb->len; ++i )
        sprintf( str + ( i * 2 ), "%02x", bb->buf[ i ] );
    
    str[ BYTE_LEN( bb->len ) * 2 ] = '\0';
}

void bitbuf_ascii( bitbuf *bb, char *str ) {
    memcpy( str, bb->buf, BYTE_LEN( bb->len ) );
    str[ BYTE_LEN( bb->len ) ] = '\0';
}

unsigned char getbit( unsigned long int data, size_t n ) {
    return ( ( data >> n ) & 0x01 ) == 1;
}
