# bitbuf
[![Build Status](https://travis-ci.org/mkchoi212/bitbuf.svg?branch=master)](https://travis-ci.org/mkchoi212/bitbuf)
[![codecov](https://codecov.io/gh/mkchoi212/bitbuf/branch/master/graph/badge.svg)](https://codecov.io/gh/mkchoi212/bitbuf)

bitbuf is a C API designed to help make the creation and analysis of binary data as simple and natural as possible.

bitbuf structures can be constructed from integers, strings, or files.
They can also be sliced, appended, reversed, inserted and so on with simple functions.
They can also be searched, replaced, read from, and navigated with ease.

# How bitbuf works
All operations in this library works under a single structure named `bitbuf`. The structure is composed of three variables; `buf`, `alloc`, and `len`.

First, `buf` is a pointer to an `unsigned char` array, which holds all the bits. 
`alloc` is a `size_t` variable that indicates how many bits have been allocated for usage behind the scenes.
`len` then shows how much of the allocated space is actually being used to store valid data.

# Getting Started
## Initialization
First things first. We hvae to know how to create a bitbuf.

```
bitbuf b = BITBUF_INIT;
```

Here, `BITBUF_INIT` is a macro that initializes all variables within the `bitbuf` structure to zero.
This is required at all times as C does not initialize variables with their default values.

OK, we've just created a bitbuf structure on the stack. Cool, but how do I initialize it with bits stored in the structure?

Well, you have several choices to choose from.
- `init( size_t )` : Initalized an empty buffer
- `init_zero( size_t )` : Initalized a buffer filled with n zeros.
- `init_file( const char * )` : Initalized a buffer with contents from a file
- `init_str( const char * )` : Initalized a buffer from strings
- `init_sub( bitbuf *src )` : Initalized a buffer with contents from another buffer

Out of the five methods, the most versatile method to create a bitbuf is with strings.
So, I will use that as the main example.

```
bitbuf b = BITBUF_INIT;
bitbuf_init_str( &b, "0xdeadbeef" );
```
Here, we just filled bitbuf from a hexadecimal string `0xdeadbeef`. 
The supported prefixes for this function are `0x` for hexadecimal strings and `0b` for binary strings.

There are lots of things we can do with our new bitbuf, the simplest of which is to print them.

## Representation
```
(lldb) bitbuf_dump( &b );
(lldb) 0xdeadbeef
```

Note that I just passed the address of the bitbuf to the function as it takes a pointer to a bitbuf as its only argument.

Now, when they are simply dumped to stdout, they are represented in the simplest hex or binary representation of themselves. If you prefer you can pick the representation that you want.

```
char binStr[ b.len + 1 ];   // +1 char for the null terminating character
bitbuf_bin( &b, binStr );
printf( binStr ); 
>> 11011110101011011011111011101111
```

Here, note that I created a char array that is later used to store the binary representation of the bitbuf. When converting bitbuf to their string representations, make sure to allocate a long enough array as not doing so will likely crash your program; here, we refer to bitbuf's `len` variable, which stores the number of bits contained in the buffer.

To get different representations of a bitbuf, you can use the functions below.
- bin
- hex
- rep (mix of hex and bin for buffers of ambiguous length)
- ascii
- num
    - Limited to the size of unsigned long

Ok, while we are at it, let's try some more.

```
bitbuf_addbit( &b, 1 );
char hexStr[ b.len / 4 + 1 ];
bitbuf_hex( &b, hexStr );
>> hex: Cannot convert to hex unambiguously - not multiple of nibbles
>> Error 1
```

On the first line, we added a single bit to a bitbuf that already contained `0xdeadbeef`, making its 32 + 1 bits long.
But this means that since each hex digit is 4 bits long, there is no unambiguous way to represent it as a hex.
When similar exceptions are met within the library, the program will output an error message to the stderr and exit.
