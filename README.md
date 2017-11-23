# ull

[![Build Status](https://travis-ci.org/kitomer/ull.svg)](https://travis-ci.org/kitomer/ull)
[![Inline docs](http://inch-ci.org/github/kitomer/ull.svg?branch=master)](http://inch-ci.org/github/kitomer/ull)
[![HitCount](http://hits.dwyl.io/kitomer/ull.svg)](http://hits.dwyl.io/kitomer/ull)

Unrolled linked list implementation in C (C99 standard) that keeps the list elements
sorted and provides a find-nearest element lookup.

In principle an unrolled linked list is a list data structure that combines the characteristics of
a linked list and an array by splitting an array into linked nodes each beeing nearly full. This
allows for nearly O(1) lookup and insertion time complexity.

## Features

- keep as many elements in the list as needed
- time complexities of data structure:
  - O(~1) insertion/deletion
  - O(~1) lookup

## Getting Started

Get a copy of the dependency [dynmem](https://www.github.com/kitomer/dynmem).

```c
#include "ull.h"

int cmp( void * a, void * b )
{
  int ia = *((int*)a);
  int ib = *((int*)a);
  return ( ia < ib ? -1 : ( ia > ib ? 1 : 0 ) );
}

int main( void )
{
  ull u;
  dynmem d;
  int i = 42, j = 41;
  int * k = 0;
  dynmem_init( &d, sizeof(int) );
  ull_init( &u, &d, cmp );
  
  ull_insert( &u, (void*)&i );
  printf("u has %ld elements\n", ull_size( &u ));
  
  ull_get_nearest( &u, (void*)j, 0, (void**)k );
  printf("k = %d\n", *k);
  
  return 0;
}
```

### Dependencies / Prerequisites

- A standard C library.
- [dynmem](https://www.github.com/kitomer/dynmem) - one C file/header.

### Installing / Deployment / Integration

#### Method A: Use the source files directly in your project

Just copy the c source and header to your project tree and integrate in your build process.
The Makefile contains an example compile command.

#### Method B: Compile as a shared library and link to your project

You can use the Makefile to compile it into a shared library using gcc:

    $ make lib

To install it globally to /usr/local/lib use this Makefile target (as root):

    $ make install

The resulting file (ends with .so.VERSION e.g. .so.0.1.0) is the shared library and
can be linked to your C project, for example using gcc:

    $ gcc -o myproject -lull myproject.c 

In order for the library to be found when executing programs that link to it, you can
add the following lines to the end  of your /etc/bashrc file (if not already present):

    export PATH=$PATH:/usr/local/lib
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

## Used by

Coming soon.

## Built With

* [gcc](https://gcc.gnu.org/) - The compiler used
* [glibc](https://www.gnu.org/s/libc/) - The C standard library used

## Contributing

[![Contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/kitomer/ull/issues)

A dedicated contributing document is coming soon.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, 
see the [tags on this repository](https://github.com/kitomer/ull/tags). 

## Authors

* **Tom Kirchner** - *Initial work and maintainer* - [kitomer](https://github.com/kitomer)

See also the list of [contributors](https://github.com/kitomer/ull/contributors)
who participated in this project.

## License

This project is licensed under the GNU General Public License (version 3 or later) -
see the [LICENSE](LICENSE) file for details

## Acknowledgments

Coming soon.
