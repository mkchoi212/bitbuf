#include "bitbuf.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int TEST_CNT = 0;

void success(char *fname) { printf("%-20sOK\n", fname); }

int assert_str(char *res, char *expected, char *fname) {
  if (strcmp(expected, res) == 0) {
    ++TEST_CNT;
    return 1;
  } else {
    fprintf(stderr, "%s FAILED\n\t\tExpected %s\t\t got %s\n", fname, expected,
            res);
    exit(EXIT_FAILURE);
    return 0;
  }
}

int assert_num(int expected, int res, char *fname) {
  if (expected == res) {
    ++TEST_CNT;
    return 1;
  } else {
    fprintf(stderr, "%s FAILED\n\t\tExpected %d\t\t got %d\n", fname, expected,
            res);
    exit(EXIT_FAILURE);
    return 0;
  }
}

void test_hexstr() {
  char str[20];
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeefe");
  bitbuf_hex(&bb, str);

  assert_str(str, "deadbeefe", "hexstr");
  success("hexstr");
  bitbuf_release(&bb);
}

void test_binstr() {
  char str[33];
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0x1234abcd");
  bitbuf_bin(&bb, str);

  assert_str(str, "00010010001101001010101111001101", "binstr");
  success("binstr");
  bitbuf_release(&bb);
}

void test_ascii() {
  char str[11];
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0x68656c6c6f776f726c64");
  bitbuf_ascii(&bb, str);

  assert_str(str, "helloworld", "ascii");
  success("ascii");
  bitbuf_release(&bb);
}

void test_copy() {
  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0xdeadbeef");
  bitbuf b2 = BITBUF_INIT;
  bitbuf_copy(&b2, &b1);

  if (bitbuf_cmp(&b1, &b2) == 0) success("copy");
  bitbuf_release(&b1);
  bitbuf_release(&b2);
}

void test_addbyte() {
  char str[14];

  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0b000100010010");
  bitbuf_addbyte(&b1, 0x34);
  bitbuf_addbyte(&b1, 0x56);
  bitbuf_addbyte(&b1, 0x78);
  bitbuf_addstr_hex(&b1, "9");
  bitbuf_addbyte(&b1, 0xab);

  bitbuf_hex(&b1, str);

  assert_str(str, "1123456789ab", "addbyte");
  success("addbyte");
  bitbuf_release(&b1);
}

void test_addbit() {
  char str[5];
  char binstr[] = "0001001000110100";

  bitbuf bb = BITBUF_INIT;
  size_t i;
  for (i = 0; i < strlen(binstr); ++i) {
    bitbuf_addbit(&bb, binstr[i] - 48);
  }

  bitbuf_hex(&bb, str);
  assert_str(str, "1234", "addbit");
  success("addbit");
  bitbuf_release(&bb);
}

void test_insert() {
  char str[20];

  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0b10101000");
  bitbuf_insert_bit(&b1, 1, 3);
  bitbuf_insert_bit(&b1, 1, b1.len);
  bitbuf_bin(&b1, str);
  bitbuf_release(&b1);
  assert_str(str, "1011010001", "insert");
  success("insert");
}

void test_initstr() {
  char str[20];
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xf 0b0001001001001000 0x0deadf 0b");
  bitbuf_hex(&bb, str);
  assert_str(str, "f12480deadf", "initstr");
  assert_num(bb.buf[0], 0xf1, "initstr-1");
  assert_num(bb.buf[1], 0x24, "initstr-2");
  assert_num(bb.buf[4], 0xad, "initstr-3");

  bitbuf_reset(&bb);
  memset(str, '\0', 20);

  bitbuf_init_str(&bb, "0xdeadbeefffeeee");
  bitbuf_hex(&bb, str);
  assert_str(str, "deadbeefffeeee", "initstr-4");
  success("initstr");

  bitbuf_release(&bb);
}

void test_getbit() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeef");

  size_t i;
  unsigned char bit;
  char res[] = "11011110101011011011111011101111";

  for (i = 0; i < bb.len; ++i) {
    bit = bitbuf_getbit(&bb, i);
    assert_num(res[i] - 48, bit, "getbit");
  }

  assert_num(i, bb.len, "getbit");
  success("getbit");
  bitbuf_release(&bb);
}

void test_setbit() {
  char str[10];
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_zero(&bb, 16);

  bitbuf_setbit(&bb, 3, 1);
  bitbuf_setbit(&bb, 6, 1);
  bitbuf_setbit(&bb, 10, 1);
  bitbuf_setbit(&bb, 11, 1);
  bitbuf_setbit(&bb, 13, 1);

  bitbuf_hex(&bb, str);
  assert_str(str, "1234", "setbit");
  success("setbit");
  bitbuf_release(&bb);
}

void test_setgetbyte() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeef 0b1");
  assert_num(0xdf, bitbuf_getbyte(&bb, 3, 1), "getbyte");
  assert_num(0xdf, bitbuf_getbyte(&bb, 3, 1), "getbyte");
  success("getbyte");

  bitbuf_setbyte(&bb, 1, 3, 0xaa);
  assert_num(0xaa, bitbuf_getbyte(&bb, 1, 3), "getbyte");
  assert_num(0x55, bitbuf_getbyte(&bb, 1, 4), "getbyte");
  success("setbyte");
  bitbuf_release(&bb);
}

void test_slice() {
  char str[10];

  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeef");

  bitbuf slice = BITBUF_INIT;
  bitbuf_init(&slice, 24);
  bitbuf_slice(&slice, &bb, 4, 12);
  bitbuf_hex(&slice, str);
  assert_str(str, "ead", "slice-1");
  memset(str, '\0', 10);

  bitbuf_slice(&slice, &bb, 9, 16);
  bitbuf_hex(&slice, str);

  assert_str(str, "5b7d", "slice-2");
  success("slice");
  bitbuf_release(&bb);
  bitbuf_release(&slice);
}

void test_op() {
  char str[20];

  bitbuf res = BITBUF_INIT;
  bitbuf_init(&res, 10);

  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0x12345678");

  bitbuf b2 = BITBUF_INIT;
  bitbuf_init_str(&b2, "0xcc99e897");
  {
    bitbuf_xor(&b1, &b2, &res);
    bitbuf_hex(&res, str);
    assert_str(str, "deadbeef", "op-xor");
    success("xor");
    memset(str, '\0', 20);
    bitbuf_reset(&res);
  }
  {
    bitbuf_or(&b1, &b2, &res);
    bitbuf_hex(&res, str);
    assert_str(str, "debdfeff", "op-or");
    success("or");
    memset(str, '\0', 20);
    bitbuf_reset(&res);
  }
  {
    bitbuf_and(&b1, &b2, &res);
    bitbuf_hex(&res, str);
    assert_str(str, "00104010", "op-and");
    success("and");
    memset(str, '\0', 20);
    bitbuf_reset(&res);
  }

  bitbuf_release(&b1);
  bitbuf_release(&b2);
  bitbuf_release(&res);
}

void test_plus() {
  char str[20];

  bitbuf res = BITBUF_INIT;
  bitbuf_init(&res, 60);

  bitbuf b1 = BITBUF_INIT;
  bitbuf b2 = BITBUF_INIT;

  {
    bitbuf_init_str(&b1, "0xef2345f");
    bitbuf_init_str(&b2, "0xff4321f");
    bitbuf_plus(&b1, &b2, &res);
    bitbuf_hex(&res, str);
    assert_str(str, "1ee6667e", "plus");
    bitbuf_reset(&b1);
    bitbuf_reset(&b2);
    bitbuf_reset(&res);
  }
  {
    bitbuf_init_str(&b1, "0x12345");
    bitbuf_init_str(&b2, "0xfeedbeef");
    bitbuf_plus(&b1, &b2, &res);
    bitbuf_hex(&res, str);
    assert_str(str, "feeee234", "plus");
  }

  success("plus");
  bitbuf_release(&b1);
  bitbuf_release(&b2);
  bitbuf_release(&res);
}

void test_shift() {
  char str[20];

  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeefcafe");

  {
    bitbuf_lsh(&bb, 8);
    bitbuf_hex(&bb, str);
    assert_str(str, "adbeefcafe00", "lsh-1");
    memset(str, '\0', 20);

    bitbuf_lsh(&bb, 5);
    bitbuf_hex(&bb, str);
    assert_str(str, "b7ddf95fc000", "lsh-2");
    memset(str, '\0', 20);
  }

  {
    bitbuf_rsh(&bb, 9);
    bitbuf_hex(&bb, str);
    assert_str(str, "005beefcafe0", "rsh-1");
    memset(str, '\0', 20);

    bitbuf_rsh(&bb, 52);
    bitbuf_hex(&bb, str);
    assert_str(str, "000000000000", "rsh-2");
  }

  bitbuf_release(&bb);
}

void test_weight() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_zero(&bb, 1000000);

  int i;
  for (i = 0; i < 100; ++i) bitbuf_setbit(&bb, i, 1);

  int weight = bitbuf_weight(&bb);
  assert_num(100, weight, "weight");
  success("weight");
  bitbuf_release(&bb);
}

void test_find() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_zero(&bb, 1000);

  bitbuf pat = BITBUF_INIT;
  bitbuf_init_str(&pat, "0x0000000000fffffff");

  int hit, cnt;
  hit = cnt = 0;

  while ((hit = bitbuf_find(&bb, &pat, 32, hit)) != -1) {
    ++cnt;
    ++hit;
  }

  assert_num(933, cnt, "find");
  bitbuf_release(&bb);
  bitbuf_release(&pat);
}

void test_replace() {
  char str[12];

  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xbed00bed");

  bitbuf pat = BITBUF_INIT;
  bitbuf_init_str(&pat, "0xbed");
  bitbuf fresh = BITBUF_INIT;
  bitbuf_init_str(&fresh, "0xbeef");

  bitbuf_replace(&bb, &pat, &fresh, 0, 0, bb.len);
  bitbuf_hex(&bb, str);
  assert_str(str, "beef00beef", "replace");
  success("replace");

  bitbuf_release(&bb);
  bitbuf_release(&pat);
  bitbuf_release(&fresh);
}

void test_append() {
  char str[12];

  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0b110111101010110");
  bitbuf b2 = BITBUF_INIT;
  bitbuf_init_str(&b2, "0b11100101011111110");

  bitbuf_addbuf(&b1, &b2);
  bitbuf_hex(&b1, str);
  assert_str(str, "deadcafe", "append");

  bitbuf_release(&b1);
  bitbuf_release(&b2);
}

void test_reverse() {
  char str[20];

  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xb75bd77f35f7");

  bitbuf_reverse_all(&bb, 4);
  bitbuf_hex(&bb, str);
  assert_str(str, "deadbeefcafe", "reverse");
  memset(str, '\0', 20);

  bitbuf_reverse_all(&bb, 8);
  bitbuf_hex(&bb, str);
  assert_str(str, "7bb57df7537f", "reverse");
  success("reverse");
  bitbuf_release(&bb);
}

void test_detach() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbeef");

  size_t buf_len, array_len;
  buf_len = bb.len;

  unsigned char *buf = bitbuf_detach(&bb, &array_len);
  assert_num(0xde, buf[0], "detach");
  assert_num(buf_len, array_len, "detach");
  success("detach");

  free(buf);
}

void test_io() {
  const char fname[] = "TEST_BITBUF_READ_WRITE";
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0xdeadbee");

  FILE *fp = fopen(fname, "w");
  bitbuf_write(&bb, fp);
  fclose(fp);
  bitbuf_reset(&bb);

  bitbuf_init_file(&bb, fname);
  char str[9];
  bitbuf_hex(&bb, str);

  assert_str(str, "deadbee0", "read");
  remove(fname);
  bitbuf_release(&bb);
  success("read");
}

void test_rep() {
  bitbuf bb = BITBUF_INIT;
  bitbuf_init_str(&bb, "0x0123456789 0b01");
  char *rep = bitbuf_rep(&bb);

  assert_str(rep, "0x0123456789 0b01", "rep");
  free(rep);
  bitbuf_release(&bb);
  success("rep");
}

void test_align() {
  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0b101");

  bitbuf b2 = BITBUF_INIT;
  bitbuf_init_str(&b2, "0b00000000000000000000101");

  bitbuf_align(&b1, &b2);
  assert_num(0, bitbuf_cmp(&b1, &b2), "align");
  success("align");

  bitbuf_release(&b1);
  bitbuf_release(&b2);
}

void test_num() {
  bitbuf b1 = BITBUF_INIT;
  bitbuf_init_str(&b1, "0xdead 0b110");
  assert_num(456046, bitbuf_num(&b1), "num-1");

  bitbuf b2 = BITBUF_INIT;
  bitbuf_init_sub(&b2, &b1, 16, 3);
  assert_num(6, bitbuf_num(&b2), "num-2");

  success("num");
  bitbuf_release(&b1);
  bitbuf_release(&b2);
}

int main() {
  test_hexstr();
  test_binstr();
  test_ascii();
  test_copy();
  test_addbyte();
  test_addbit();
  test_insert();
  test_initstr();
  test_getbit();
  test_setbit();
  test_setgetbyte();
  test_slice();
  test_op();
  test_plus();
  test_shift();
  test_weight();
  test_find();
  test_replace();
  test_append();
  test_reverse();
  test_detach();
  test_io();
  test_rep();
  test_align();
  test_num();

  printf("--------------------------\n");
  printf("%i tests passed\n", TEST_CNT);
  return EXIT_SUCCESS;
}
