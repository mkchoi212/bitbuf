#include "bitbuf.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

unsigned char bitbuf_slopbuf[1];

static void die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(EXIT_FAILURE);
}

void bitbuf_init(bitbuf *bb, size_t s) {
  bb->buf = bitbuf_slopbuf;
  bb->len = bb->alloc = 0;

  if (s) bitbuf_grow(bb, s);
}

void bitbuf_init_zero(bitbuf *bb, size_t s) {
  size_t n = BYTE_LEN(s);
  bb->buf = (unsigned char *)calloc(n, sizeof(unsigned char));
  bb->len = s;
  bb->alloc = n * 8;
}

void bitbuf_init_file(bitbuf *bb, const char *fname) {
  FILE *fp = fopen(fname, "rb");
  if (fp == NULL) die("init_file: %s does not exist", fname);

  bitbuf_read(bb, fp);
  fclose(fp);
}

void bitbuf_init_str(bitbuf *bb, const char *str) {
  char *in = strdup(str);
  char *org = in;
  char *tok;

  while ((tok = strsep(&in, " ")) != NULL) {
    if (strlen(tok) <= 2) continue;

    if (tok[0] != '0')
      die("init_str: Please provide the initializing string with either 0b | "
          "0x prefixes");
    if (tok[1] == 'b')
      bitbuf_addstr_bin(bb, tok + 2);
    else if (tok[1] == 'x')
      bitbuf_addstr_hex(bb, tok + 2);
    else
      die("init_str: %c is not a supported data format token", tok[1]);
  }

  free(org);
}

void bitbuf_init_sub(bitbuf *dest, const bitbuf *src, size_t start, size_t n) {
  bitbuf_init(dest, src->len);
  bitbuf_slice(dest, src, start, n);
}

void bitbuf_reset(bitbuf *bb) {
  if (bb->alloc) memset(bb->buf, 0, BYTE_LEN(bb->len));
  bb->len = 0;
}

void bitbuf_release(bitbuf *bb) {
  if (bb->alloc) {
    free(bb->buf);
    bitbuf_init(bb, 0);
  }
}

void bitbuf_attach(bitbuf *bb, const void *data, size_t alloc, size_t len) {
  if (bb->alloc) bitbuf_release(bb);

  bb->buf = (unsigned char *)data;
  bb->len = len * 8;
  bb->alloc = alloc * 8;
}

unsigned char *bitbuf_detach(bitbuf *bb, size_t *len) {
  unsigned char *res;
  res = bb->buf;

  if (len) *len = bb->len;

  bb->buf = NULL;
  bb->len = bb->alloc = 0;
  return res;
}

void bitbuf_copy(bitbuf *dest, const bitbuf *src) {
  bitbuf_reset(dest);
  bitbuf_addbuf(dest, src);
}

void bitbuf_grow(bitbuf *bb, size_t extra) {
  size_t newlen = BYTE_LEN(bb->alloc + extra);
  size_t oldlen = BYTE_LEN(bb->alloc);

  int new_buf = !(bb->alloc);
  if (new_buf) bb->buf = NULL;

  if ((bb->buf = (unsigned char *)realloc(bb->buf, newlen)) == NULL) {
    die("grow: Could not allocate more buffer space");
  } else {
    memset(bb->buf + oldlen, 0, newlen - oldlen);
    bb->alloc = newlen * 8;
  }
}

void bitbuf_setlen(bitbuf *bb, size_t len) {
  if (len > (bb->alloc ? bb->alloc : 0))
    die("setlen: Length beyond allocated buffer");
  bb->len = len;
}

static size_t popcnt(const unsigned char c) {
  size_t res;
  res = __builtin_popcount(c);
  return res;
}

size_t bitbuf_weight(const bitbuf *bb) {
  size_t i, cnt;
  cnt = 0;
  for (i = 0; i < BYTE_LEN(bb->len); ++i) cnt += popcnt(bb->buf[i]);

  return cnt;
}

int bitbuf_find(const bitbuf *src, const bitbuf *pat, size_t garble,
                size_t offset) {
  if (pat->len > src->len - offset || pat->len <= 8 || garble >= pat->len)
    return -1;

  int hit = -1;
  size_t i, cur, width, patlen, weight;
  weight = 0;
  patlen = BYTE_LEN(pat->len);
  width = patlen + 1;

  unsigned char temp[width];
  memset(temp, 0, width);
  bitbuf win = BITBUF_INIT;
  bitbuf_attach(&win, temp, width, width);

  for (cur = offset; cur <= src->len - pat->len; ++cur) {
    weight = 0;
    bitbuf_slice(&win, src, cur, pat->len);
    for (i = 0; i < patlen; ++i) {
      weight += popcnt(win.buf[i] ^ pat->buf[i]);
      if (weight > garble) {
        bitbuf_resetlen(&win);
        break;
      }
    }
    if (i == patlen) {
      hit = cur;
      break;
    }
  }

  return hit;
}

int bitbuf_replace(bitbuf *src, const bitbuf *old, const bitbuf *fresh,
                   size_t garble, size_t start, size_t end) {
  if (old->len == 0) die("replace: Cannot replace an empty buffer");

  int hit, cur, cnt;
  cur = cnt = 0;

  bitbuf res = BITBUF_INIT;
  bitbuf win = BITBUF_INIT;
  bitbuf_init(&res, src->len);

  while ((hit = bitbuf_find(src, old, garble, start)) != -1) {
    if ((size_t)hit > end) break;

    bitbuf_slice(&win, src, cur, hit - cur);
    bitbuf_addbuf(&res, &win);
    bitbuf_addbuf(&res, fresh);

    cur = hit + old->len;
    start += cur;
    ++cnt;
  }
  bitbuf_release(&win);
  bitbuf_release(src);
  bitbuf_swap(src, &res);
  return cnt;
}

int bitbuf_cmp(const bitbuf *a, const bitbuf *b) {
  if (a->len != b->len) die("cmp: Buffers should be the same length");
  return memcmp(a->buf, b->buf, BYTE_LEN(a->len));
}

void bitbuf_slice(bitbuf *dest, const bitbuf *src, size_t start, size_t n) {
  if (start + n > src->len) die("slice: Out of bounds");

  size_t width = BYTE_LEN(n + start % 8) * 8;
  if (width > dest->alloc) bitbuf_grow(dest, width - dest->alloc);

  memcpy(dest->buf, &src->buf[start / 8], width / 8);
  dest->len = width;

  /* clean trash at left */
  bitbuf_lsh(dest, start % 8);
  /* to the right */
  size_t trash = 8 - n % 8;
  dest->buf[n / 8] = dest->buf[n / 8] >> trash << trash;
  dest->len = n;
}

unsigned char bitbuf_getbit(const bitbuf *bb, size_t n) {
  if (n > bb->len) die("getbit: Out of bounds");

  size_t bytepos = n / 8;
  size_t bitpos = 7 - n % 8;
  return getbit(bb->buf[bytepos], bitpos);
}

void bitbuf_setbit(bitbuf *bb, size_t n, int bit) {
  if (n > bb->len) die("setbit: Out of bounds");

  size_t bytepos = n / 8;
  size_t bitpos = 7 - n % 8;
  unsigned char mask = 0x1 << bitpos;

  if (bit)
    bb->buf[bytepos] |= mask;
  else
    bb->buf[bytepos] &= ~mask;
}

unsigned char bitbuf_getbyte(const bitbuf *bb, size_t pos, size_t offset) {
  if ((pos + 1) * 8 + offset > bb->len) die("getbyte: Out of bounds");

  unsigned char win[2];
  size_t trash = 8 - offset;

  win[0] = bb->buf[pos] << offset;
  win[1] = bb->buf[pos + 1] >> trash;
  return win[0] | win[1];
}

void bitbuf_setbyte(bitbuf *bb, size_t pos, size_t offset, unsigned char byte) {
  if ((pos + 1) * 8 + offset > bb->len) die("setbyte: Out of bounds");

  size_t trash = 8 - offset;

  bb->buf[pos] &= ~0 << trash;
  bb->buf[pos] |= byte >> offset;
  bb->buf[pos + 1] &= ~(~0 << trash);
  bb->buf[pos + 1] |= byte << trash;
}

void bitbuf_addbyte(bitbuf *bb, unsigned char byte) {
  if (bb->len + 8 > bb->alloc) bitbuf_grow(bb, bb->len * 2);

  size_t pad = bb->len % 8;
  size_t bytepos = bb->len / 8;

  if (!pad) {
    bb->buf[bytepos] = byte;
  } else {
    bb->buf[bytepos] |= byte >> pad;
    bb->buf[bytepos + 1] = byte << (8 - pad);
  }

  bb->len += 8;
}

void bitbuf_addbit(bitbuf *bb, int bit) {
  if (!bitbuf_avail(bb)) bitbuf_grow(bb, bb->len * 2 + 1);

  bitbuf_setbit(bb, bb->len, bit);
  bb->len++;
}

void bitbuf_addbuf(bitbuf *dest, const bitbuf *src) {
  if (src->len > dest->alloc - dest->len) bitbuf_grow(dest, src->len);

  size_t pad, trash;
  pad = dest->len % 8;
  trash = min((8 - pad), src->len);

  size_t copycnt = src->len;
  bitbuf fresh = BITBUF_INIT;

  if (pad) {
    dest->buf[dest->len / 8] |= src->buf[0] >> pad;
    bitbuf_copy(&fresh, src);
    bitbuf_lsh(&fresh, trash);
    copycnt -= trash;
  } else {
    fresh = *src;
  }

  memcpy(dest->buf + BYTE_LEN(dest->len), fresh.buf, BYTE_LEN(copycnt));
  dest->len += fresh.len;

  if (pad) bitbuf_release(&fresh);
}

void bitbuf_reverse(bitbuf *bb, size_t start, size_t n) {
  size_t i, j;
  unsigned char head, tail;

  for (i = start, j = start + n - 1; i < j; ++i, --j) {
    head = bitbuf_getbit(bb, i);
    tail = bitbuf_getbit(bb, j);
    bitbuf_setbit(bb, i, tail);
    bitbuf_setbit(bb, j, head);
  }
}

void bitbuf_reverse_all(bitbuf *bb, size_t n) {
  size_t i;
  for (i = 0; i < bb->len; i += n) bitbuf_reverse(bb, i, n);
}

void bitbuf_lsh(bitbuf *bb, size_t n) {
  /* Throw away bytes that would have been lost anyway
   * with shifts greater than 8 */

  if (n > bb->len) {
    memset(bb->buf, 0, BYTE_LEN(bb->len));
    n = 0;
  }

  int skipcnt = n / 8;
  if (skipcnt) {
    memmove(bb->buf, bb->buf + skipcnt, BYTE_LEN(bb->len) - skipcnt);
    memset(bb->buf + BYTE_LEN(bb->len) - skipcnt, 0, skipcnt);
  }

  size_t rem = n % 8;
  if (rem) {
    size_t trash = 8 - rem;
    unsigned char trail;

    size_t i;
    for (i = 0; i < BYTE_LEN(bb->len) - 1; ++i) {
      trail = bb->buf[i + 1] >> trash;
      bb->buf[i] <<= rem;
      bb->buf[i] |= trail;
    }

    bb->buf[i] <<= rem;
  }
}

void bitbuf_rsh(bitbuf *bb, size_t n) {
  if (n > bb->len) {
    memset(bb->buf, 0, BYTE_LEN(bb->len));
    n = 0;
  }

  int skipcnt = n / 8;
  if (skipcnt) {
    memmove(bb->buf + skipcnt, bb->buf, BYTE_LEN(bb->len) - skipcnt);
    memset(bb->buf, 0, skipcnt);
  }

  size_t rem = n - (skipcnt * 8);
  if (rem) {
    size_t trash = 8 - rem;
    unsigned char trail;

    int i;
    for (i = BYTE_LEN(bb->len) - 1; i > 0; --i) {
      trail = bb->buf[i - 1] << trash;
      bb->buf[i] >>= rem;
      bb->buf[i] |= trail;
    }

    bb->buf[0] >>= rem;
  }
}

void bitbuf_align(bitbuf *a, bitbuf *b) {
  bitbuf *lg = a->len > b->len ? a : b;
  bitbuf *sm = a->len > b->len ? b : a;
  size_t diff = lg->len - sm->len;

  if (sm->len < lg->len) bitbuf_grow(sm, lg->len - sm->len);

  bitbuf_setlen(sm, lg->len);
  bitbuf_rsh(sm, diff);
}

void bitbuf_op(const bitbuf *a, const bitbuf *b, bitbuf *res, OperatorPtr op) {
  if (a->len != b->len)
    die("op: Buffers should be of same length to perform the operation");

  if (res->alloc <= a->len) bitbuf_grow(res, a->len - res->alloc + 8);

  size_t i;
  for (i = 0; i < BYTE_LEN(a->len); ++i)
    res->buf[i] = (*op)(a->buf[i], b->buf[i]);

  res->len = a->len;
}

static unsigned char xor (unsigned char a, unsigned char b) {
  return a ^ b;
} void bitbuf_xor(const bitbuf *a, const bitbuf *b, bitbuf *res) {
  OperatorPtr f_xor = xor;
  bitbuf_op(a, b, res, f_xor);
}
static unsigned char or (unsigned char a, unsigned char b) { return a | b; }
void bitbuf_or(const bitbuf *a, const bitbuf *b, bitbuf *res) {
  OperatorPtr f_or = or ;
  bitbuf_op(a, b, res, f_or);
}
static unsigned char and (unsigned char a, unsigned char b) { return a & b; }
void bitbuf_and(const bitbuf *a, const bitbuf *b, bitbuf *res) {
  OperatorPtr f_and = and;
  bitbuf_op(a, b, res, f_and);
}

void bitbuf_plus(const bitbuf *a, const bitbuf *b, bitbuf *res) {
  bitbuf lval = BITBUF_INIT;
  bitbuf rval = BITBUF_INIT;

  /* TODO optimize by not copying every time*/
  bitbuf_copy(&lval, a);
  bitbuf_copy(&rval, b);
  bitbuf_align(&lval, &rval);

  int i, sum, carry;
  carry = 0;
  for (i = BYTE_LEN(lval.len) - 1; i >= 0; --i) {
    sum = lval.buf[i] + rval.buf[i] + carry;
    res->buf[i] = sum;
    carry = getbit(sum, 8);
  }

  /* Handle the last carry */
  res->len = lval.len;
  if (carry) {
    size_t pad = 4 - res->len % 4;
    bitbuf_grow(res, pad);
    res->len += pad;
    bitbuf_rsh(res, pad);
    bitbuf_setbit(res, pad - 1, 1);
  }

  bitbuf_release(&lval);
  bitbuf_release(&rval);
}

void bitbuf_addstr(bitbuf *bb, const char *str, size_t base, size_t ulen) {
  /* Maximum length of string that can be converted at a time
   * considering the size limitation of unsigned long int */
  const size_t MAX_STR_SZ = sizeof(unsigned long int) * 8 / ulen;

  char win[MAX_STR_SZ + 1];
  win[MAX_STR_SZ] = '\0';
  unsigned long int val;
  size_t i, bitlen;
  int j;

  errno = 0;
  for (i = 0; i < strlen(str); i += MAX_STR_SZ) {
    strncpy(win, str + i, MAX_STR_SZ);
    val = strtoul(win, NULL, base);
    bitlen = strlen(win) * ulen;
    if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0))
      die("addstr: Error while converting string to bytes (strtoul)");

    for (j = bitlen - 1; j >= 0; --j) bitbuf_addbit(bb, getbit(val, j));
  }
}

void bitbuf_addstr_hex(bitbuf *bb, const char *str) {
  /* `base` is 16 since hex is base 16
   * `ulen` is 4 since one char of hex (nibble) equates to 4 bits
   */
  char c;
  const char *cur = str;

  for (; *str; ++str) {
    c = *str;
    if (!((c >= 48 && c <= 57) || (c >= 97 && c <= 102) ||
          (c >= 65 && c <= 70)))
      die("addstr: Non-hexadecimal number %c found", c);
  }

  bitbuf_addstr(bb, cur, 16, 4);
}

void bitbuf_addstr_bin(bitbuf *bb, const char *str) {
  char c;
  const char *cur = str;

  for (; *str; ++str) {
    c = *str;
    if (!(c == '1' || c == '0'))
      die("addstr: Non-hexadecimal number %c found", c);
  }

  bitbuf_addstr(bb, cur, 2, 1);
}

void bitbuf_insert(bitbuf *dest, const bitbuf *src, size_t idx) {
  bitbuf tail = BITBUF_INIT;
  bitbuf_slice(&tail, dest, idx, dest->len - idx);
  bitbuf_setlen(dest, idx);

  // Clear out dirty bits in the byte where new bits
  // will be inserted
  unsigned char *trash = &(dest->buf[idx / 8]);
  int clr = 8 - (idx % 8);
  *trash = *trash >> clr << clr;

  bitbuf_addbuf(dest, src);
  bitbuf_addbuf(dest, &tail);
}

char *bitbuf_rep(bitbuf *bb) {
  size_t slen = BYTE_LEN(bb->len) * 8 + 6;
  char *rep = (char *)calloc(slen, sizeof(char));
  char *cur = rep;
  size_t orglen = bb->len;

  /* Modify bb->len temporarily to un-ambiguate its lenght
   * while converting to hex
   */

  bb->len = orglen - orglen % 4;
  char hex[BYTE_LEN(bb->len) * 2 + 1];
  bitbuf_hex(bb, hex);
  cur += sprintf(rep, "0x%s", hex);
  bb->len = orglen;

  /* Convert leftovers to binary */
  size_t rem = orglen % 4;
  if (rem) {
    char bin[4];
    bitbuf binbuf = BITBUF_INIT;
    bitbuf_slice(&binbuf, bb, bb->len - rem, rem);
    bitbuf_bin(&binbuf, bin);
    sprintf(cur, " 0b%s", bin);
    bitbuf_release(&binbuf);
  }

  return rep;
}

void bitbuf_dump(bitbuf *bb) {
  char *rep = bitbuf_rep(bb);
  printf("%s\n", rep);
  free(rep);
}

size_t bitbuf_read(bitbuf *bb, FILE *fp) {
  bitbuf_reset(bb);
  fseek(fp, 0, SEEK_END);
  size_t fsize = ftell(fp);
  rewind(fp);

  size_t bitlen = fsize * 8;
  if (bb->alloc < bitlen) bitbuf_grow(bb, bitlen - bb->alloc);

  if (fread(bb->buf + BYTE_LEN(bb->len), 1, fsize, fp) != fsize)
    die("read: Could not read from file");

  bb->len = bitlen;
  return bitlen;
}

size_t bitbuf_write(bitbuf *bb, FILE *fp) {
  return bb->len ? fwrite(bb->buf, 1, BYTE_LEN(bb->len), fp) : 0;
}

void bitbuf_bin(const bitbuf *bb, char *str) {
  size_t i;
  unsigned char cur;

  for (i = 0; i < bb->len; ++i) {
    cur = bitbuf_getbit(bb, i);
    if (cur)
      str[i] = '1';
    else
      str[i] = '0';
  }
  str[i] = '\0';
}

void bitbuf_hex(const bitbuf *bb, char *str) {
  if (bb->len % 4 != 0)
    die("hex: Cannot convert to hex unambiguously - not multiple of nibbles");

  size_t i;
  for (i = 0; i < BYTE_LEN(bb->len); ++i)
    sprintf(str + i * 2, "%02x", bb->buf[i]);

  size_t rem = bb->len;
  if (bb->len % 4) rem = bb->len + (4 - bb->len % 4);
  str[rem / 4] = '\0';
}

void bitbuf_ascii(const bitbuf *bb, char *str) {
  memcpy(str, bb->buf, BYTE_LEN(bb->len));
  str[BYTE_LEN(bb->len)] = '\0';
}

unsigned long bitbuf_num(const bitbuf *bb) {
  unsigned long num = 0;
  if (bb->len > sizeof(num) * 8)
    die("num: Buffer too larger to conver to number");

  size_t i;
  for (i = 0; i < BYTE_LEN(bb->len) - 1; ++i) {
    num <<= 8;
    num += bb->buf[i];
  }

  /* Handle remaining bits */
  size_t rem = bb->len % 8;
  num <<= rem;
  num += bb->buf[i] >> (8 - rem);
  return num;
}
