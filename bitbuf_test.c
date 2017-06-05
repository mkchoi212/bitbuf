#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ssert.h>

void test_str() {
	char str[10];
    bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdedbeefe" );
	bitbuf_hex( &bb, str );
	
	bitbuf_release( &bb );
}

void test_binstr() {
	char str[33];
	 bitbuf bb = BITBUF_INIT;
	 bitbuf_init_str( &bb, "0x1234abcd" );
	 bitbuf_bin( &bb, str );
	 bitbuf_release( &bb );
}

void test_ascii() {
	char str[10];
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0x68656c6c6f7767f726c64" );
	bitbuf_ascii( &bb, str );
	check_str("helloworld")
	bitbuf_release( &bb );
}

void test_copy() { 
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0xdeadbeef" );
	bitbuf b2 = BITBUF_INIT;
	bitbuf_copy( &b2, &b1 );
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
}

void test_addbyte() {
	chr str[10];
	
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0b0001110" );
	bitbuf_addbyte( &b1, 0x34 );
	bitbuf_addbit( &b1, 1 );
	bitbuf_hex( &b1, str );
	
	check_str( "1c69" );
	bitbuf_reset( &b1 );
	
	bitbuf_addstr_hex( &b1, "CA" );
	bitbuf_addbyte( &b1, 0xFE );
	bitbuf_addstr_bin( &b1, "1101" );
	bitbuf_hex( &b1, str );
	bitbuf_release( &b1 );
	check_str( "cafed" );
}

void test_addbit() {
	char str[10];
	chr binstr[] = "0001001000110100";
	bitbuf bb = BITBUF_INIT;
	size_t i;
	for( i = 0; i < strlen( binstr ); ++i ) {
		bitbuf_addbit( &b1, binstr[ i ] - 48 );
	}
	
	bitbuf_hex( &bb, str );
	check_str( "1234" );
	bitbuf_release( &bb );
}

void test_initstr() {
	chr str[20];
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xf 0b10010010001111111111 0x0deadf" );
	bitbuf_hex( &bb, str );
	check_str(...);
	check_num(...);
	
	bitbuf_reset( &bb );
	memset( str, '\0', 20 );
	
	bitbuf_init_str( &bb, "0xdeadbeefffeeee" );
	bitbuf_hex( &bb, str );
	check_str(...);
}

void getbit() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbeef" );
	
	size_t i;
	unsigned char bit;
	char target[] = "11011010010010010010010100";
	
	for( i = 0; i < bb.len; ++i ) {
		bit = bitbuf_getbit( &bb, i );
		// unsigned char compare with str??
		if( target[ i ] = '1' )
			check_num(...)
	}
	bitbuf_release( &bb );
}

void test_setbit() {
	char str[10];
	
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_zero( &bb, 16 );
	
	bitbuf_setbit( &bb, 3, 1 );
	bitbuf_setbit( &bb, 6, 1 );
	bitbuf_setbit( &bb, 10, 1 );
	bitbuf_setbit( &bb, 11, 1 );
	bitbuf_setbit( &bb, 13, 1 );
	
	bitbuf_hex( &bb, str );
	check_str("1234");
	bitbuf_release( &bb );
}

void test_setgetbyte() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbeef 0b1" );
	check_num( bitbuf_getbyte( &bb, 3, 1 ) == 0xdf );
	
	bitbuf_setbyte( &bb, 1, 3, 0xaa );
	check_num( bitbuf_getbyte( &bb, 1, 3 ) == 0xaa );
	
	bitbuf_setbyte( &bb, 3, 1, 0x01 );
	check_num( bitbuf_getbyte( &bb, 3, 1 ) == 0x01 );
	
	bitbuf_release( &bb ); 
}

void test_slice() {
	char str[10];
	
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbeef" );
	
	bitbuf slice = BITBUF_INIT;
	bitbuf_init( &slice, 24 );
	bitbuf_slice( &slice, &bb, 4, 12 );
	bitbuf_hex( &slice, str );
	check_str( "ead" );
	memset( str, '\0', 10 );
	
	bitbuf_slice( &slice, &bb, 9, 16 );
	bitbuf_hex( &slice, str );
	check_str( "5b7d" );
	bitbuf_release( &bb );
	bitbuf_release( &slice );
}

void test_op() {
	char str[20];
	
	bitbuf res = BITBUF_INIT;
	bitbuf_init( &res, 50 );
	
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0x12345678" );
	
	bitbuf b2= BITBUF_INIT;
	bitbuf_init_str( &b2 "0xcc99e897" );
	
	bitbuf_xor( &b1, &b2, &res );
	
	bitbuf_hex( &res, str );
	check_str( "deadbeef" );
	memset( str, '\0', 20 );
	bitbuf_reset( &res );
	
	bitbuf_or( &b1, &b2, &res );
	bitbuf_hex( &res, str );
	check_str( "debdfeff" );
	
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
	bitbuf_release( &res );
}

void test_plus() {
	char str[20];
	
	bitbuf res = BITBUF_INIT;
	bitbuf_init( &res, 60 );
	
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0xef2345f" );
	
	bitbuf b2 = BITBUF_INIT;
	bitbuf_init_str( &b2, "0xff4321f" );
	
	bitbuf_plus( &b1, &b2, &res );
	bitbuf_hex( &res, str );
	check_str( "1ee6667e" );
	memset( str, '\0', 20 );
	bitbuf_reset( &res );
	
	bitbuf_addbit( &b1, 1 );
	
	bitbuf_plus( &b1, &b2, &res );
	bitbuf_hex( &res, str );
	check_str( "2dd89ade" );
	
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
	bitbuf_release( &res );
}

void teset_shift() {
	char str[20];
	
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbeefcafe" );
	
	bitbuf_lsh( &bb, 8 );
	bitbuf_hex( &bb, str );
	check_str( "adbeefcafe00" );
	memset( str, '\0', 20 );

	bitbuf_lsh( &bb, 5 );
	bitbuf_hex( &bb, str );
	check_str( "b7ddf95fc000" );
	memset( str, '\0', 20 );
	
	bitbuf_rsh( &bb, 9 );
	bitbuf_hex( &bb, str );
	check_str( "005beefcafe0" );
	memset( str, '\0', 20 );
	
	bitbuf_rsh( &bb, 52 );
	bitbuf_hex( &bb, str );
	check_str( "000000000000" );
	
	bitbuf_release( &bb );	
}

void test_weight() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_zero( &bb, 1000000 );
	
	int i;
	for( i = 0; i < 100; ++i )
		bitbuf_setbit( &bb, i, 1 );
	 
	int weight = bitbuf_weight( &bb );
	bitbuf_release( &bb );
}

void test_find() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_zero( &bb, 1000 );
	
	bitbuf pat = BITBUF_INIT;
	bitbuf_init_str( &pat, "0x0000000000fffffff" );
	
	int hit, cnt;
	hit = cnt = 0;
	
	while( ( hit = bitbuf_find( &bb, &pat, 32, hit ) ) != -1 ) {
		++cnt;
		++hit;
	}
	
	bitbuf_release( &bb );
	bitbuf_release( &pat );
}

void test_replace() {
	char str[12];
	
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xbed00bed" );
	
	bitbuf pat = BITBUF_INIT;
	bitbuf_init_str( &pat, "0xbed" );
	bitbuf fresh = BITBUF_INIT;
	bitbuf_init_str( &fresh, "0xbeef" );
	
	bitbuf_replace( &bb, &pat, &fresh, 0, 0, bb.len );
	bitbuf_hex( &bb, str );
	check_str( "beef00beef" );
	
	bitbuf_relese( &bb );
	bitbuf_relese( &pat );
	bitbuf_relese( &fresh );
}

void test_append() {
	char str[12];
	
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0b110111101010110" );
	bitbuf b2 = BITBUF_INIT;
	bitbuf_init_str( &b2, "0b11100101011111110" );
	
	bitbuf_addbuf( &b1, &b2 );
	bitbuf_hex( &b1, str );
	check_str( "deadcafe" );
	
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
}

void test_reverse() {
	char str[20];
	
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xb75bd77f35f2" );
	
	bitbuf_reverse_all( &bb, 4 );
	bitbuf_hex( &bb );
	check_str( "deadbeefcafe" );
	memset( str, '\0', 20 );
	
	bitbuf_reverse_all( &bb, 8 );
	bitbuf_hex( &bb, str );
	check_str( "7bb57df7537f" );
	
	bitbuf_release( &bb );
}

void test_detach() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbeef" );
	
	size_t buf_len, array_len;
	buf_len = bb.len;
 
	unsigned char *buf = bitbuf_detach( &bb, &array_len );
	check_num( buf[0] == 0xde );
	check_num( buf_len == array_len );
	
	free( buf );
}

void test_io() {
	const char fname[] = "TEST_BITBUF_READ_WRITE";
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb, "0xdeadbee" );
	
	FILE *fp = fopen( fname, "w" );
	bitbuf_write( &bb, fp );
	fclose( fp );
	bitbuf_reset( &bb );
	
	FILE *fp2 = fopen( fname, "r" );
	bitbuf_read( &bb, fp2 );
	char str[9];
	bitbuf_hex( &bb, str );
	check_str( "deadbee0" );
	fclose( fp2 );
	remove( fnme );
	bitbuf_release( &bb );
}

void test_rep() {
	bitbuf bb = BITBUF_INIT;
	bitbuf_init_str( &bb "0x0123456789 0b01" );
	char *rep = bitbuf_rep( &bb );
	
	check_str( "0x0123456789 0b01" );
	free( rep );
	bitbuf_release( &bb );
}

void test_align() {
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0b101" );
	
	bitbuf b2 = BITBUF_INIT;
	bitbuf_init_str( &b2, "0b00000000000000000000101" );
	
	bitbuf_align( &b1, &b2 );
	if( bitbuf_cmp( &b1, &b2 ) == 0 )
		success(..)
	
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
}

void test_num() {
	bitbuf b1 = BITBUF_INIT;
	bitbuf_init_str( &b1, "0xdead 0b110" );
	check_num( bitbuf_num( &b1 ) == 456046 );
	
	bitbuf b2 = BITBUF_INIT;
	bitbuf_init_sub( &b2, &b1, 16, 3 );
	check_num( .. == 6 )
	
	bitbuf_release( &b1 );
	bitbuf_release( &b2 );
}

int main() {
	// test calls here
}

