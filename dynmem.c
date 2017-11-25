// This file is part of Dynmem.
// vim: set expandtab tabstop=2 shiftwidth=2 softtabstop=2:
/*	
    Manage dynamic memory buffers.

		Dynmem is a typedef for a dynamic memory buffer and functions
		for managing buffers of that type.
		
		@category   C99 library
		@author     Tom Kirchner <tom@tkirchner.com>
		@copyright  2017 Tom Kirchner
		@version    0.1 (2017/11/19)
		@link       https://www.github.com/kitomer/dynmem

		@license 

			Dynmem is free software: you can redistribute it and/or modify it under 
			the terms of the GNU General Public License as published by the 
			Free Software Foundation, either version 3 of the License, or 
			(at your option) any later version.

			Dynmem is distributed in the hope that it will be useful, but 
			WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
			or FITNESS FOR A PARTICULAR PURPOSE. 
			See the GNU General Public License for more details.

			You should have received a copy of the GNU General Public License 
			along with Dynmem. If not, see http://www.gnu.org/licenses/.
 
*/

#include "dynmem.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct _dynmem {
  unsigned char * bytes;
  size_t elemsize;
  size_t reserved; // number of elements max storable
  size_t length; // max number of elements used/accessed (arbitrary)
  double avg_length_accessed; // average max number of elements accessed through dynmem_set() or dynmem_get()
  size_t num_accesses; // number of accesses (used to determine if downsizing is allowed)
};

void hexdump( void * addr, size_t len )
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	if( len == 0 ) {
		printf("  ZERO LENGTH\n");
		return;
	}
	if( len < 0 ) {
		printf("  NEGATIVE LENGTH: %ld\n", len);
		return;
	}
	for( i = 0; i < len; i++ ) {
		if( (i % 16) == 0 ) {
			if( i != 0 ) {
				printf ("  %s\n", buff);
			}
			printf("  %04x ", i);
		}
		printf(" %02x", pc[i]);

		// and store a printable ASCII character for later.
		if( (pc[i] < 0x20) || (pc[i] > 0x7e) ) {
			buff[ i % 16 ] = '.';
		}
		else {
			buff[i % 16] = pc[i];
		}
		buff[ (i % 16) + 1 ] = '\0';
	}

	// pad out last line if not exactly 16 characters.
	while( (i % 16) != 0 ) {
		printf("   ");
		i++;
	}
	printf("  %s\n", buff);
}

size_t round_up_to_power_of_2( size_t x )
{
	size_t y = 2;
	while( y <= x ) {
		y = y * (size_t)2;
	}
	return y;
}

int dynmem_test( void )
{
	dynmem d;
	dynmem_init( &d, 1 );
	char str[ 256 ] = "abc";
	char * strp = 0;
	//int i = 0;

	//dynmem_resize( &d, 42 );
	dynmem_set( &d, 0, 3, (void *)str );
	dynmem_debug( &d );
	
	dynmem_get( &d, 0, 1, (void **)&strp );
	printf("[0] '%.1s'\n", strp);
	printf("[1,2] '%.2s'\n", strp+1);

	//for( i = 0; i < 300; i++ ) {
	//  int rnd = RANDO(0,300);
	//  //printf("rnd %d\n", rnd);
	//  dynmem_set( &d, rnd, 1, (void *)str );
	//}
	dynmem_set( &d, 3, 1, (void *)str );
	dynmem_set( &d, 42, 1, (void *)str );
	dynmem_set( &d, 12, 1, (void *)str );
	
	dynmem_debug( &d );
	//dynmem_resize( &d, 100 );
	//dynmem_debug( &d );
	//dynmem_resize( &d, 100 );
	//dynmem_debug( &d );

	dynmem_truncate( &d );
	dynmem_debug( &d );
				
	return 0;
}

int dynmem_init( dynmem * mem, size_t elemsize )
{
	if( mem ) {
		mem->bytes = NULL;
		mem->elemsize = elemsize;
		mem->reserved = 0;
		mem->length = 0;
		mem->avg_length_accessed = 0.0;
		mem->num_accesses = 0;
		return 1;
	}
	return 0;
}

void dynmem_debug( dynmem * mem )
{
	if( mem ) {
		size_t numbytes = ( mem->reserved * mem->elemsize );
		double kb = ( (double)numbytes / (double)1024.0 );
		printf("<dynmem bytes %s, elemsize %ld, length %ld, avg_length_accessed %.4lf, num_accesses %ld, reserved %ld (%.4lfKB, %ldB)\n",
			(mem->bytes == NULL ? "NULL" : "DEF"), mem->elemsize, mem->length, mem->avg_length_accessed, mem->num_accesses, mem->reserved, kb, numbytes );
		hexdump( mem->bytes, mem->elemsize * mem->length );
		printf(">\n");
	}
	else {
		printf("<NULL>\n");
	}
}

// makes sure enough memory for given amount of elements (AND .length elements) is allocated
// -> changes storage memory
// -> upsize: to next multiple of n
// -> downsize (if allowed): downsize to MAX(.max_access,newlength)
int dynmem_reserve( dynmem * mem, size_t numelems )
{
	//printf("--- dynmem_reserve() ---\n");
	if( mem ) {
		//size_t current_length = mem->reserved;
		size_t min_length = ( numelems > mem->length ? numelems : mem->length );
		size_t new_length = 0;
		size_t avg_length = (size_t)( (double)(mem->avg_length_accessed) * (double)2.0 );

		//printf("need %ld, avg %.0lf, accesses %ld -> newlen %ld\n", numelems, mem->avg_length_accessed, mem->num_accesses, new_length);

		// if more than double the average length is allocated, the memory is downsized
		if( mem->num_accesses > 42 && ( (double)(mem->reserved) / (double)2.0 ) > (double)avg_length ) {
			// take max(min_length,avg_length)
			min_length = ( avg_length > min_length ? avg_length : min_length );
		}

		new_length =
			round_up_to_power_of_2( 
				min_length < DYNMEM_GENERAL_MIN_ELEMENTS ? DYNMEM_GENERAL_MIN_ELEMENTS : min_length );

		// reallocate
		if( new_length > mem->reserved ) {
			//printf("[need %ld] reallocating from %ld to %ld (%lf avg)\n", numelems, mem->reserved, new_length, mem->avg_length_accessed);
			//printf("REALLOC FROM %ld bytes TO %ld bytes (REQUESTED %ld)\n", mem->reserved * mem->elemsize, new_length * mem->elemsize, numelems );
			if( mem->bytes == NULL ) {
				mem->bytes = malloc( new_length * mem->elemsize );
			}
			else {
				mem->bytes = realloc( mem->bytes, new_length * mem->elemsize );				
			}
			if( mem->bytes == NULL && new_length > 0 ) {
				//printf("BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD\n");
				return 0;
			}
			else {
				mem->reserved = new_length;
				mem->length = ( mem->length > new_length ? new_length : mem->length );
			}
		}
		return 1;
	}
	return 0;
}

// set .length and make sure enough memory for length elements is allocated
int dynmem_resize( dynmem * mem, size_t length )
{
	if( mem ) {
		//printf("--- dynmem_resize() ---\n");
		// set how many elements the memory SHOULD be able to hold (from the user perspective)
		mem->length = length;
		// make sure there is AT LEAST that amount of memory allocated
		return dynmem_reserve( mem, length );
	}
	return 0;
}

// dynmem_resize(0)
int dynmem_truncate( dynmem * mem )
{
	//printf("--- dynmem_truncate() ---\n");
	return dynmem_resize( mem, 0 );
}

// set n elements
// update also .max_access
int dynmem_set( dynmem * mem, size_t elemoffset, size_t numelems, void * bytes )
{
	//printf("--- dynmem_set() ---\n");
	if( mem && dynmem_reserve( mem, elemoffset + numelems ) ) {
		//printf("SET DYNMEM %ld %ld (LEN %ld, RESERV %ld)\n", elemoffset, numelems, mem->length, mem->reserved);
		if( bytes ) {
			// copy given bytes into dynmem storage
			unsigned char * ptr = (unsigned char*)(mem->bytes) + ( elemoffset * mem->elemsize );
			//printf("MEMCPY( elemoff %ld, elemsize %ld ) %ld elems to byteoffset %ld (reserved %ld bytes\n)",
			//	elemoffset, mem->elemsize, numelems,
			//	elemoffset * mem->elemsize, mem->reserved * mem->elemsize );
			memcpy( ptr, bytes, mem->elemsize * numelems );
		}
		// update length
		mem->length = ( ( elemoffset + numelems ) > mem->length ? ( elemoffset + numelems ) : mem->length );
		// update average length accessed
		mem->avg_length_accessed = ( (double)( mem->avg_length_accessed + elemoffset + numelems ) / (double)2.0 );
		mem->num_accesses ++;
		//printf("  -> SET OK: new len %ld\n", mem->length);
		return 1;
	}
	return 0;
}

// get n elements (must be in range of current length)
// update also .max_access
int dynmem_get( dynmem * mem, size_t elemoffset, size_t numelems, void * * bytes )
{
	//printf("--- dynmem_get() ---\n");
	if( mem && ( elemoffset + numelems ) <= mem->length /* && dynmem_reserve( mem, elemoffset + numelems ) */ ) {
		//printf("GET DYNMEM %ld %ld (LEN %ld, RESERV %ld)\n", elemoffset, numelems, mem->length, mem->reserved);
		if( bytes ) {
			// copy bytes from dynmem storage to output
			*bytes = mem->bytes + ( elemoffset * mem->elemsize );
		}
		// update average length accessed
		mem->avg_length_accessed = ( (double)( mem->avg_length_accessed + elemoffset + numelems ) / (double)2.0 );
		mem->num_accesses ++;
		//printf("  -> GET OK\n");
		return 1;
	}
	return 0;
}

// set .length, return new ptr
int dynmem_push( dynmem * mem, void * inbytes, void * * outbytes )
{
	//printf("--- dynmem_push() ---\n");
  size_t elemoffset = dynmem_length( mem );
  return dynmem_set( mem, elemoffset, 1, inbytes ) && dynmem_get( mem, elemoffset, 1, outbytes );
}

// set .length
int dynmem_pop( dynmem * mem, void * * bytes )
{
	//printf("--- dynmem_pop() ---\n");
  if( mem && mem->length > 0 && dynmem_get( mem, mem->length, 1, bytes ) ) {
    return dynmem_resize( mem, dynmem_length( mem ) - 1 );
  }
  return 0;
}

// get .length
size_t dynmem_length( dynmem * mem )
{
  return ( mem ? mem->length : 0 );
}

// sets last byte to '\0' and returns char* to first byte
size_t dynmem_get_as_string( dynmem * mem, char * * str )
{
  if( mem ) {
    // set the byte after the last element to '\0'
    char * p = 0;
    size_t len = mem->elemsize * mem->length;
    dynmem_reserve( mem, mem->length + 1 ); // resize so that the final '\0' can be set
    p = (char*)(mem->bytes);
    p[ len ] = '\0';
    if( str ) {
      *str = p;
    }
    return len;
  }
  else {
    // pretend
    static char empty[1] = { '\0' };  
    if( str ) {
      *str = empty;
    }
    return 0;
  }
}


