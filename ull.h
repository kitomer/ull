// This file is part of Ull.
// vim: set expandtab tabstop=2 shiftwidth=2 softtabstop=2:
/*	
    Unrolled linked list.

		Unrolled linked list implementation in C (C99 standard) that
		keeps the list elements sorted and provides a find-nearest element lookup.
		
		@category   C99 library
		@author     Tom Kirchner <tom@tkirchner.com>
		@copyright  2017 Tom Kirchner
		@version    0.1 (2017/11/23)
		@link       https://www.github.com/kitomer/ull

		@license 

			Ull is free software: you can redistribute it and/or modify it under 
			the terms of the GNU General Public License as published by the 
			Free Software Foundation, either version 3 of the License, or 
			(at your option) any later version.

			Ull is distributed in the hope that it will be useful, but 
			WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
			or FITNESS FOR A PARTICULAR PURPOSE. 
			See the GNU General Public License for more details.

			You should have received a copy of the GNU General Public License 
			along with Dynmem. If not, see http://www.gnu.org/licenses/.
 
*/
#ifndef ULL_H
#define ULL_H

#include "dynmem.h"

typedef struct _ull ull;
typedef struct _ullnode ullnode;
typedef int (*ullcmpfunc)( void * a, void * b );
typedef void (*ulldebugfunc)( void * a );

int ull_init( ull * u, dynmem * m, ullcmpfunc f );
void ull_debug( ull * u, ulldebugfunc f );
int _ull_insert_new_node( ull * u, ullnode * prev, ullnode * next, ullnode * * new );
int _ull_insert_node_element( ullnode * n, size_t insert_at_index, void * elem );
int ull_insert( ull * u, void * elem );
int _ull_get_node_including_elem( ull * u, void * elem, ullnode * * n );
int ull_get_nearest( ull * u, void * elem, int exactly, void * * nearest );
size_t ull_size( ull * u );
int ull_get( ull * u, size_t pos, void * * value );
int	ull_remove_all( ull * u );

#endif

