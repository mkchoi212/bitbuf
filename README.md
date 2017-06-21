<p align="center">
    <img src="./bitbuf_logo.gif">

  <h3 align="center">bitbuf</h3>


<p align="center">
    C API for creation and analysis of binary data
    <br>
    <br>
    <a href="https://travis-ci.org/mkchoi212/bitbuf"><img src="https://travis-ci.org/mkchoi212/bitbuf.svg?branch=master"></a>
    <a href="https://codecov.io/gh/mkchoi212/bitbuf"><img src="https://codecov.io/gh/mkchoi212/bitbuf/branch/master/graph/badge.svg"></a>
  </p>
</p>
<br>

bitbuf is a fast C API designed to help make the creation and analysis of binary data as simple and natural as possible.

bitbuf structures can be constructed from integers, strings, or files.
They can also be sliced, appended, reversed, inserted and so on with simple functions.
They can also be searched, replaced, read from, and navigated with ease.

# How bitbuf works
All operations work under a single struct named `bitbuf`. The struct is composed of three variables; `buf`, `alloc`, and `len`.

`buf` is a pointer to an `unsigned char` array, which holds all the data. 
`alloc` is a `size_t` variable that indicates how many bits have been allocated for usage behind the scenes.
`len` then shows how much of the allocated space is actually being used to store valid data.

# Getting Started
## Initialization
First things first. We hvae to know how to create a bitbuf.

```c
bitbuf b = BITBUF_INIT;
```

Here, `BITBUF_INIT` is a macro that initializes all variables within the `bitbuf` structure to zero.
This is required at all times as C does not initialize variables with their default values.

OK, we've just created a bitbuf structure on the stack. Cool, but how do I initialize it with bits stored in the structure?

Well, you have several choices to choose from.

- `init( size_t )` 
    - Initalize an empty buffer
- `init_zero( size_t )` 
    - Initialize a buffer filled with n zeros.
- `init_file( const char * )`
    - Initialize a buffer with contents from a file
- `init_str( const char * )`
    - Initialize a buffer from strings
- `init_sub( bitbuf *src )`
    - Initialize a buffer with contents from another buffer

Out of the five methods, the most versatile method to create a bitbuf is with strings.
So, I will use that as the main example.

```c
bitbuf b = BITBUF_INIT;
bitbuf_init_str( &b, "0xdeadbeef" );
```
Here, we just filled bitbuf from a hexadecimal string `0xdeadbeef`. 
The supported prefixes for this function are `0x` for hexadecimal strings and `0b` for binary strings.

There are lots of things we can do with our new bitbuf, the simplest of which is to print them.

## Representation
```c
(lldb) bitbuf_dump( &b );
(lldb) 0xdeadbeef
```

Note that I just passed the address of the bitbuf to the function as it takes a pointer to a bitbuf as its only argument.

Now, when they are simply dumped to stdout, they are represented in the simplest hex or binary representation of themselves. If you prefer you can pick the representation that you want.

```c
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

```c
bitbuf_addbit( &b, 1 );
char hexStr[ b.len / 4 + 1 ];
bitbuf_hex( &b, hexStr );
>> hex: Cannot convert to hex unambiguously - not multiple of nibbles
>> Error 1
```

On the first line, we added a single bit to a bitbuf that already contained `0xdeadbeef`, making its 32 + 1 bits long.
But this means that since each hex digit is 4 bits long, there is no unambiguous way to represent it as a hex.
When similar exceptions are met within the library, the program will output an error message to the stderr and exit.

## Modifying bitbuf
A bitbuf can be treated just like an array of bits. You can slice it, delete sections, append bitbufs and more.

If you ask for a single bit, an unsigned char is returned as it is the smallest type available in C and reminds the user once again that a bit can either be 0 or 1.

To join bitbufs, you can use `bitbuf_addbuf` or `bitbuf_addstr_[hex | bin ]`.
```c
bitbuf_addstr_hex( &b, "cafe" )
```

## Finding and Replacing
`bitbuf_find` is provided to search for bit patterns within a bitbuf. You can choose whether to search from the beginning or any bit position.
In addition, you can also specifiy the number of garbles allowed during the search; number of bits mismatched.

```c
bitbuf b = BITBUF_INIT;
bitbuf_init_file( &b, "FILE_NAME" );

bitbuf pat = BITBUF_INIT;
bitbuf_init_str( &pat, "0xcafe" );

int hit;
hit = 0;

while( ( hit = bitbuf_find( &b, &pat, 32, hit ) ) != -1 )   // find returns -1 when no patterns are found
    ++hit;
    
bitbuf_release( &b );
bitbuf_release( &pat );
```

# Example
The sieve of Eratosthenes is an ancient (and very inefficient) method of finding prime numbers. The algorithm starts with the number 2 (which is prime) and marks all of its multiples as not prime, it then continues with the next unmarked integer (which will also be prime) and marks all of its multiples as not prime.

So to print all primes under a million you could write:

```c
const size_t MAX_LIM = 1000000;                // We will do a search until million
bitbuf buf = BITBUF_INIT;
bitbuf_init_zero( &buf, 1000000 );             // Create million zero bits. They will be set to indicate if that bit position isn't prime

size_t i, j;
for( i = 2; i < MAX_LIM; ++i ) {
    if( !bitbuf_getbit( &buf, i ) ) {
        printf( "%i\n", i );
        
        for( j = i * 2; j < MAX_LIM; j+=i ) 
            bitbuf_setbit( &buf, j, 1 );       // Set all multiples of current prime to 1
    }
}

bitbuf_release( &buf );
```
This example illustrates both bit checking and setting.

One reason you might want to use bitbuf for this purpose - instead of a plain array - is that the million bits only take up a million bits in memory, whereas for a list of integers it would be much more. Try asking for a billion elements in a list - unless you’ve got some really nice hardware it will fail, whereas a billion element bitbuf only takes **125MB**.

# TODO
- Implement `bitbuf_prependbuf`
- Implement `bitbuf_insert`
