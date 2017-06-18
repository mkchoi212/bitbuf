# bitbuf
[![Build Status](https://travis-ci.org/mkchoi212/bitbuf.svg?branch=master)](https://travis-ci.org/mkchoi212/bitbuf)

bitbuf is a C API designed to help make the creation and analysis of binary data as simple and natural as possible.

bitbuf structures can be constructed from integers, strings, or files.
They can also be sliced, appended, reversed, inserted and so on with simple functions.
They can also be searched, replaced, read from, and navigated with ease.

# How bitbuf works
All operations in this library works under a single structure named `bitbuf`. The structure is composed of three variables; `buf`, `alloc`, and `len`.

First, `buf` is a pointer to an `unsigned char` array, which holds all the bits. 
`alloc` is a `size_t` variable that indicates how many bits have been allocated for usage behind the scenes.
`len` then shows how much of the allocated space is actually being used to store valid data.

# Examples
- Creation
```
bitbuf b = BITBUF_INIT;
bitbuf_init_str( &b, "0xdeadbeef" );        // Supported prefixes are 0x and 0b
```
or

```
bitbuf_init_file( &b, "VALID_BIT_FILE" );
```

- Seaching
```
bitbuf b = BITBUF_INIT;
bitbuf_init_file( &b, "VALID_BIT_FILE" );

bitbuf pattern  = BITBUF_INIT;
bitbuf_init_str( &pattern "0b1101" );

int hit;
hit = 0

while( ( hit = bitbuf_find( &b, &pattern, 1, hit ) ) != -1 ) 
    ++hit;
```
