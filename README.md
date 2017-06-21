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

Think of it as C++'s `bitset` on crack, with the size of your entire memory as the only limitation of its size.

# How bitbuf works

bitbuf is designed to be as lightweight as possible and can be considered to be just a list of binary digits. They are however stored efficiently - although there are a variety of ways of creating and viewing the binary data, bitbuf stores the byte data, and all views are calculated as needed.

All operations work under a struct named `bitbuf`. The struct is composed of three variables; `buf`, `alloc`, and `len`.

`buf` is a pointer to an `unsigned char` byte array, which holds all the data. 
`alloc` is a variable that indicates how many bits have been allocated for usage behind the scenes.
`len` then shows how much of the allocated space is actually being used to store the data.

# Getting Started
Typing `make` will generate a static library `libbitbuf.a` in your current directory. You may then move it to a directory of your choice that has been specified by a `LD_LIBRARY_PATH`.

## Initialization
First things first. Here is how you create a bitbuf.

```c
bitbuf b = BITBUF_INIT;
```

Here, `BITBUF_INIT` is a macro that initializes all variables within the `bitbuf` structure.
**This is required at all times as C does not initialize variables with their default values.**

OK, we've just created a bitbuf structure on the stack. Cool, but how do I initialize it with binary data stored in it?

Well, you have several choices to choose from.

- `init( size_t )` 
    - Initalize an empty buffer
- `init_zero( size_t )` 
    - Initialize with n zeros.
- `init_file( const char * )`
    - Initialize with contents from a file
- `init_str( const char * )`
    - Initialize from strings
- `init_sub( bitbuf *src )`
    - Initialize with contents from another buffer

Out of the five methods, the most versatile method to create a bitbuf is with strings. So, I will use that as the main example.

```c
bitbuf b = BITBUF_INIT;
bitbuf_init_str( &b, "0xdeadbeef 0b110" );
```
Here, we just created a bitbuf of length 35 from a string that consists of both hexadecimal and binary strings. Note that the second segment of the input string is just 3 bits long. bitbuf handles all data that does not fall into byte units internally.

The supported prefixes for this function are `0x` for hexadecimal strings and `0b` for binary strings. 

There are lots of things we can do with our new bitbuf, the simplest of which is to print them.

## Representation
```
>> bitbuf_dump( &b );
0xdeadbeef 0b110
```

Note that I just passed the address of the bitbuf to the function as it takes a pointer to a bitbuf as its only argument.

Now, when they are simply dumped to stdout, they are represented in the simplest hex | binary representation of themselves. If you prefer you can pick the representation that you want.

```c
>> char binStr[ b.len + 1 ];   // +1 char for the null terminating character
>> bitbuf_bin( &b, binStr );
>> printf( "%s", binStr ); 
11011110101011011011111011101111110
```

Here, note that I created a char array that is later used to store the binary representation of the bitbuf. When converting bitbuf to their string representations, make sure to allocate a long enough array as not doing so will likely crash your program; here, we refer to bitbuf's `len` variable, which stores the number of bits contained in the buffer.

To get different representations of a bitbuf, you can use the functions below.

- `bitbuf_bin`
- `bitbuf_hex`
- `bitbuf_rep` (mix of hex and bin for buffers of ambiguous length)
- `bitbuf_ascii`
- `bitbuf_num`

Ok, while we are at it, let's try one.

```c
>> char hexStr[ b.len / 4 + 1 ];
>> bitbuf_hex( &b, hexStr );
hex: Cannot convert to hex unambiguously - not multiple of nibbles
Error 1
```

bitbuf `b` we just passed into the function is 35 bits long.
This means that since each hex digit is 4 bits long, there is no unambiguous way to represent it as a hex.
When similar exceptions are met within the library, the program will output an error message to the stderr and exit.

## Modifying bitbuf
A bitbuf can be treated just like an array of bits. You can slice it, delete sections, append bitbufs and more.

If you ask for a single bit, an unsigned char is returned as it is the smallest type available in C and reminds the user once again that a bit can either be 0 or 1.

To join bitbufs, you can use `bitbuf_addbuf` or `bitbuf_addstr_[hex | bin ]`.

```c
bitbuf_addstr_hex( &b, "cafe" )
```

## Finding and Replacing
`bitbuf_find` is provided to search for binary patterns within a bitbuf. You can choose whether to search from the beginning or any bit position.
In addition, you can also specifiy the number of garbles (errors) allowed during the search.

Here is a simple program that searches for a pattern within a bitbuf and prints the bit position of all matching occurences.

```c
bitbuf b = BITBUF_INIT;
bitbuf_init_file( &b, "FILE_NAME" );

bitbuf pat = BITBUF_INIT;
bitbuf_init_str( &pat, "0xcafe" );

int cur;
cur = 0;

// find returns -1 when no patterns are found
while( ( cur = bitbuf_find( &b, &pat, 32, cur ) ) != -1 ) {	printf( "%i\n", cur );
    ++cur;
}
    
bitbuf_release( &b );
bitbuf_release( &pat );
```

# Example - Sieve of Eratosthenes
The sieve of Eratosthenes is an ancient (and very inefficient) method of finding prime numbers. The algorithm starts with the number 2 (which is a prime) and marks all of its multiples as not prime. It then continues with the next unmarked integer (which will also be prime) and marks all of its multiples as not prime.

So, to print all primes under a million you could write:

```c
// We will do a search until million
const size_t MAX_SCH = 1000000; 
              
// Create million zero bits
bitbuf buf = BITBUF_INIT;
bitbuf_init_zero( &buf, MAX_SCH );    
        
size_t i, j;
for( i = 2; i < MAX_LIM; ++i ) {
    if( !bitbuf_getbit( &buf, i ) ) {
        printf( "%i\n", i );
        
        // Set all multiples of current prime to not prime
        for( j = i * 2; j < MAX_SCH; j+=i ) 
            bitbuf_setbit( &buf, j, 1 );    
    }
}

bitbuf_release( &buf );
```

One reason you might want to use bitbuf for this purpose - instead of a plain array - is that the million bits only take up million bits in memory, whereas an array of integers would take up much more space. Try making an array with billion integers - it will fail unless you’ve got some really nice hardware. A billion element bitbuf is only **125MB** big.

# TODO
- Implement `bitbuf_prependbuf`
- Implement `bitbuf_insert`
