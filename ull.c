#include "ull.h"

// unrolled linked list structure (stored byte chunks)
#define ULL_ELEMENTS_PER_NODE 32
struct _ullnode {
  struct _ullnode * prev;
  struct _ullnode * next;
  void * elements [ ULL_ELEMENTS_PER_NODE ];
  size_t num_elements;
};
struct _ull {
  // a dynmem from which to allocate the nodes
  dynmem * nodes_memory;
  // root node
  ullnode * root;
  // total size (for fast lookup)
  //size_t num_elements;
  // number of nodes (for fast lookup)
  size_t num_nodes;
  // compare function
  ullcmpfunc cmpfunc;
};

// inits the list
int ull_init( ull * u, dynmem * m, ullcmpfunc f )
{
  if( ULL_ELEMENTS_PER_NODE < 2 ) {
    // number of elements per node must be at least 2 to ensure logic
    return 0;
  }
  if( u && m && f ) {
    u->root = 0;
    //u->num_elements = 0;
    u->num_nodes = 0;
    u->cmpfunc = f;
		u->nodes_memory = m;
		dynmem_resize( m, 24 );
    return 1;
  }
  return 0;
}

void ull_debug( ull * u, ulldebugfunc f )
{
  if( u ) {
    ullnode * n = u->root;
    int i = 0;
    printf("<ull nodes_memory %s, root %s, num_nodes %ld\n",
      (u->nodes_memory == NULL ? "NULL" : "DEF"), 
      (u->root == NULL ? "NULL" : "DEF"), u->num_nodes);
    printf("  memory: ");
      dynmem_debug( u->nodes_memory );
    while( n ) {
      printf("  <ullnode %4d, prev %s, next %s, elements %s, num_elements %ld / %ld>\n",
        i,
        (n->prev == NULL ? "NULL" : "DEF"),
        (n->next == NULL ? "NULL" : "DEF"),
        ((n->elements)[0] == NULL ? "NULL" : "DEF"), n->num_elements, (size_t)(ULL_ELEMENTS_PER_NODE) );
      if( f ) {
        size_t j = 0;
        for( j = 0; j < n->num_elements; j++ ) {
          printf("    [  %4ld] ", j);
          f( (n->elements)[j] );
        }
        if( n->num_elements < ULL_ELEMENTS_PER_NODE ) {
          printf("    [..%4ld] not set\n", (size_t)(ULL_ELEMENTS_PER_NODE) - 1 );
        }
      }
      n = n->next;
      i++;
    }
    printf(">\n");  
  }
  else {
    printf("<NULL>");
  }
}

int _ull_insert_new_node( ull * u, ullnode * prev, ullnode * next, ullnode * * new )
{
  if( u && new ) {
		//if( dynmem_resize( u->nodes_memory, u->num_nodes + 1, 0 ) &&
    //    dynmem_get( u->nodes_memory, u->num_nodes, 1, (void **)&newnode ) &&
    //    newnode ) {
		//ullnode n = { 0 };
		//size_t idx = dynmem_size( u->nodes_memory );
		//if( dynmem_set( u->nodes_memory, idx, 1, (void*)&n ) ) {
		//	ullnode * newnode = 0;
		//	// get pointer to stored ullnode
		//	if( dynmem_get( u->nodes_memory, idx, 1, (void**)&newnode ) ) {
		ullnode n = { 0 };
		ullnode * newnode = 0;
		//printf("NEW NODE ?\n");
		if( dynmem_push( u->nodes_memory, (void*)&n, (void**)&newnode ) && newnode ) {

			//printf("  -> PUSHED\n");

			// init node
			newnode->prev = ( prev ? prev : 0 );
			if( prev ) { // let previous node point to new node
				prev->next = newnode;
			}
			newnode->next = ( next ? next : 0 );
			if( next ) { // let new node point to next node
				next->prev = newnode;
			}
			newnode->num_elements = 0;
			*(newnode->elements) = 0;			
			// inc total node counter
			u->num_nodes ++;
			// result
			*new = newnode;
			return 1;
    }
  }
  return 0;
}

int _ull_insert_node_element( ullnode * n, size_t insert_at_index, void * elem )
{
  if( n ) {
    // shift elements after insert pos one up
    if( n->num_elements > insert_at_index ) {
			//printf("move up\n");
			for( size_t i = n->num_elements; i > insert_at_index; i-- ) {
				(n->elements)[ i ] = (n->elements)[ i - 1 ];
			}
		}
    // set at pos
    (n->elements)[ insert_at_index ] = elem;
    n->num_elements ++;
		//printf("inserted elem: ");
		//tmplvar_debug( (n->elements)[ insert_at_index ] );
    return 1;
  }
  return 1;
}

// uses compare function to insert element in a sorted fashion
int ull_insert( ull * u, void * elem )
{
  if( u->num_nodes == 0 ) {
    // init first node with one element
    ullnode * new = 0;
    if( _ull_insert_new_node( u, 0, 0, &new ) ) {
      // insert element
      new->num_elements = 1;
      (new->elements)[0] = elem;
			new->prev = NULL;
			new->next = NULL;
      // insert node as root node
      u->root = new;
      return 1;
    }
  }
  else {
    // insert in a sorted fashion
    ullnode * best = 0;
    if( _ull_get_node_including_elem( u, elem, &best ) && best ) {
      int res = 0;
      if( best->num_elements > 0 && (u->cmpfunc)( elem, (best->elements)[0] ) < 0 ) {
        // elem is before best node -> put as first node element
				//printf("put as first (0)\n");
        res = _ull_insert_node_element( best, 0, elem );
      }
      else if( best->num_elements > 0 && (u->cmpfunc)( elem, (best->elements)[best->num_elements - 1] ) > 0 ) {
        // elem is after best node -> append to node elements
				//printf("put as last (as %ld)\n", best->num_elements);
        res = _ull_insert_node_element( best, best->num_elements, elem );
      }
      else {
        // elem is inside best node, find a place inside node elements
        size_t i = 0;
				//printf("put inside\n");
        for( i = 0; i < best->num_elements; i++ ) {
          void * e = (best->elements)[ i ];
          if( i == 0 && (u->cmpfunc)( elem, e ) < 0 ) {
            // before current element (only applicable for first element)
						//printf("  put before first (0)\n");
            res = _ull_insert_node_element( best, i, elem );
            break;
          }
          else if( (u->cmpfunc)( elem, e ) >= 0 && 
              ( i == best->num_elements - 1 || (u->cmpfunc)( elem, (best->elements)[ i + 1 ] ) <= 0 ) ) {
            // after current element
						//printf("  put after %ld (as %ld)\n", i, i+1);
            res = _ull_insert_node_element( best, i + 1, elem );
            break;
          }
        }
        res = 1;
      }
      // check if best node is full and needs to be split
      // (always ensure a minimum of ONE empty element slot per node, for the logic above
      // and optimally all nodes should not be fuller than 80%)
      if( res &&
          ( ( (double)(best->num_elements) / (double)(ULL_ELEMENTS_PER_NODE) ) > 0.8 ||
            (ULL_ELEMENTS_PER_NODE - best->num_elements) < 1 ) ) {
        
				//printf("needs split\n");
        // more than 80% full -> split in two nodes
        ullnode * new = 0;
        // insert a new node after best node
        if( _ull_insert_new_node( u, best, best->next, &new ) && new ) {
          // put second half of best's elements into new node
          size_t firstnew = (size_t)( (double)(best->num_elements) / (double)2.0 + 0.1 );
          size_t i = 0;
          for( i = firstnew; i < best->num_elements; i++ ) {
            (new->elements)[ i - firstnew ] = (best->elements)[ i ];
            new->num_elements ++;
          }
          best->num_elements = firstnew;
        }
      }
      return res;
    }
  }
  return 0;
}

// find the ullnode that would/does best include given element
// -> does NOT check if given element is ACTUALLY inside the node, just in the RANGE of the node's elements!
// -> if given element is BETWEEN two nodes (or before first node or after last node) then
//    the node with the LEAST elements is returned (or first/last node)
int _ull_get_node_including_elem( ull * u, void * elem, ullnode * * n )
{
  if( n ) {
    ullnode * cur = u->root;
    ullnode * prev = 0; // previous node
    while( cur && cur->num_elements > 0 ) { // stop if empty node is found (which should never happen)
      void * first = (cur->elements)[ 0 ];
      void * last = (cur->elements)[ cur->num_elements - 1 ];

      if( (u->cmpfunc)( elem, first ) < 0 ) {
        // elem is before first element of this node
        if( prev ) {
          // the previous node is the best for elem
          *n = prev;
          return 1;
        }
        else {
          // this node is the best for elem
          *n = cur;
          return 1;
        }
      }
      else if( (u->cmpfunc)( elem, last ) <= 0 ) {
        // this node is best for elem
        *n = cur;
        return 1;
      }
      else if( ! cur->next ) {
        // there is no next node, so current node is best for elem
        *n = cur;
        return 1;
      }
      // else: go to next node and check that
      cur = cur->next;
    }
  }
  return 0; // no best node found
}

// uses compare function to retrieve nearest element
int ull_get_nearest( ull * u, void * elem, int exactly, void * * nearest )
{
  ullnode * best = 0;
  if( _ull_get_node_including_elem( u, elem, &best ) && best ) {
    // try to find element inside node
    size_t i = 0;
    for( i = 0; i < best->num_elements; i++ ) {
      void * e = (best->elements)[ i ];
      int cmpi = (u->cmpfunc)( elem, e );
      
      if( cmpi == 0 || // exact match
          ( ! exactly && (
            cmpi < 0 || // is before current element
            ( cmpi > 0 && i < best->num_elements - 1 && (u->cmpfunc)( elem, (best->elements)[ i + 1 ] ) < 0 ) || // is between this and next element
            ( cmpi > 0 && i == best->num_elements - 1 ) ) ) ) { // is after this element but there are no next elements
        // current element is nearest
        *nearest = (best->elements)[ i ];
        return 1;
      }
    }
  }
  return 0;
}

// nice to have
size_t ull_size( ull * u )
{
	return 0;
  //return ( u ? u->num_elements : 0 );
}

// nice to have
int ull_get( ull * u, size_t pos, void * * value )
{
  // ...
  return 1;
}

int ull_remove_all( ull * u )
{
  if( u ) {
    u->root = 0;
    //u->num_elements = 0;
    u->num_nodes = 0;
    return 1;
  }
	return 0;
}
