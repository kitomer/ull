#include <stdio.h>
#include "ull.h"

int cmp( void * a, void * b )
{
  int ia = *((int*)a);
  int ib = *((int*)a);
  return ( ia < ib ? -1 : ( ia > ib ? 1 : 0 ) );
}

void dbg( void * a )
{
  int ia = *((int*)a);
	printf("%d", ia);
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
	ull_debug( &u, dbg );
  printf("u has %ld elements\n", ull_size( &u ));
  
  ull_get_nearest( &u, (void*)&j, 0, (void**)&k );
  printf("k = %d\n", *k);
  
  return 0;
}
