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
#ifndef DYNMEM_H
#define DYNMEM_H

#include <string.h>

// type that represents a heap allocated dynamic memory buffer
// -> all LOGICAL ACCESSING functions (get/set/resize/push/pop) MUST ENSURE ENOUGH MEMORY is allocated
// -> INTERNALLY the length_accessed length is used to determine how to scale the memory up/down AS SEEN FIT
//    MAYBE: only ever scale up! special function dynmem_crop() to scale DOWN to a reasonable memory amount if requested!  
typedef struct _dynmem {
	// don't mess with this...
  unsigned char * bytes;
  size_t elemsize;
  size_t reserved; // number of elements max storable
  size_t length; // max number of elements used/accessed (arbitrary)
  double avg_length_accessed; // average max number of elements accessed through dynmem_set() or dynmem_get()
  size_t num_accesses; // number of accesses (used to determine if downsizing is allowed)
}
dynmem;

#define DYNMEM_GENERAL_MIN_ELEMENTS 32
#ifndef DYNMEM_USE_MULTITHREADED_GLOBAL_BYTE_STORAGE
	#define DYNMEM_USE_MULTITHREADED_GLOBAL_BYTE_STORAGE 1
#endif

extern int dynmem_init( dynmem * mem, size_t elemsize );
extern void dynmem_debug( dynmem * mem );
extern int dynmem_reserve( dynmem * mem, size_t numelems );
extern int dynmem_resize( dynmem * mem, size_t length );
extern int dynmem_truncate( dynmem * mem );
extern int dynmem_set( dynmem * mem, size_t elemoffset, size_t numelems, void * bytes );
extern int dynmem_get( dynmem * mem, size_t elemoffset, size_t numelems, void * * bytes );
extern int dynmem_push( dynmem * mem, void * inbytes, void * * outbytes );
extern int dynmem_pop( dynmem * mem, void * * bytes );
extern size_t dynmem_length( dynmem * mem );
extern size_t dynmem_get_as_string( dynmem * mem, char * * str );

#endif
