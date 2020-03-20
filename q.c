#include "q.h"

//
// This file implements a pthread safe fifo queue suitable for use with C.
// usage is described in the fib.c source, and this should be refactored
// to have in internal and a public interface ... for now, it is just a 
// toy to understand pthread_mutex and pthread_cond variables in a locking
// and sleeping context ....
//

/*
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   internal interface ... don't look at these ...
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

// unlock the Q ...
int q_unlock( Queue_t *Q )
{
   int nx ;
   nx = pthread_mutex_unlock( Q->lock ) ;
   if( nx != 0 )
   {
      printf( "Mutex UN-lock returned %d.\n", nx ) ; 
   }
   return nx ;
}

// lock the Q ...
int q_lock( Queue_t *Q )
{
   int nx ;
   nx = pthread_mutex_lock( Q->lock ) ;
   if( nx != 0 )
   {
      printf( "Mutex lock returned %d.\n", nx ) ; 
   }
   return nx ;
}

// signal a thread waiting on a condition variable ...
int q_signal( Cond_t *condition )
{
  int nx ;
  nx = pthread_cond_signal( condition ) ;
  if( nx != 0 )
  {
      printf( "Condition signal returned %d.\n", nx ) ; 
  }
  return nx;
}

// wait on a condition (and lock) ...
int q_wait( Cond_t *condition, Lock_t *lock )
{
   int nx ;
   nx = pthread_cond_wait( condition, lock ) ;
   if( nx != 0 )
   {
      printf( "Condition wait returned %d.\n", nx ) ; 
   }
   return nx;
}

// note the routines below can only be used when the
// data structure is locked, or mayhem may result.

// number of items in the queue ...
int q_qty( Queue_t *Q )
{
	if( q_empty( Q ) )
		return 0 ;

  return abs(Q->head - Q->tail) ;
}

// room available in the queue ...
int q_avail( Queue_t *Q )
{
	if( q_empty( Q ) )
		return Q->size ;

  return Q->size - q_qty( Q ) ;
}

// is the queue empty ? 1 : 0 ... 
int q_empty( Queue_t *Q )
{
	return ( Q->head == Q->tail ) ;
}

// is the queue full ? 1 : 0  ... 
int q_full( Queue_t *Q )
{
	int rv ;
	return (q_qty( Q ) < Q->size) ? 0 : 1 ;
}

/*
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   public interface ...
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

// create a Queue_t structure ...
Queue_t *q_create( int size, Func_t *siva )
{
   Queue_t *rv ;

   rv = (Queue_t *) calloc( 1, sizeof( Queue_t ) ) ; 
   if( !isNul( rv ) )
   {
      rv -> lock = (Lock_t *) calloc( 1, sizeof( Lock_t ) ) ;
      pthread_mutex_init( rv ->lock, NULL ) ; 

      rv -> is_empty = (Cond_t *) calloc( 1, sizeof( Cond_t ) ) ;
      pthread_cond_init( rv ->is_empty, NULL ) ; 

      rv -> is_full = (Cond_t *) calloc( 1, sizeof( Cond_t ) ) ;
      pthread_cond_init( rv ->is_full, NULL ) ; 

      rv ->task = (void **) calloc( size, sizeof( void* ) ) ;
      rv ->head = rv ->tail = 0 ; 
      rv ->size = size ;
   }
   return rv ;
}

// destroy a Queue_t structure ...
Queue_t *q_destroy( Queue_t * Q )
{
  if( !isNul( Q ) )
  {
    while( q_lock( Q ) );
    if( !isNul( Q->siva ) )
    {
       int n = 0 ;
       while( !q_empty( Q ) )
       {
          Q->siva( q_pop( Q ) ) ;
          n++ ;
       }
       if( n > 0 )
       {
           printf( "WARNING: %d items lost in Queue.\n", n ) ; 
       }
    }
    if( !isNul( Q->lock ) ) free( Q->lock ) ;
    if( !isNul( Q->is_empty ) ) free( Q->is_empty ) ;
    if( !isNul( Q->is_full ) ) free( Q->is_full ) ;
    if( !isNul( Q->task ) ) free( Q->task ) ;
    memset( Q, 0, sizeof( Queue_t ) ) ;
    free( Q ) ;
  }
  return (Queue_t *) NULL ;
}

// push a task onto the queue ...
int q_push( Queue_t *Q, void *T )
{
   int rv ; 
   while( q_lock( Q ) );
   while( q_full( Q ) ) q_wait( Q->is_full, Q->lock ) ;

   Q->task[Q->tail] = T ;
	 Q->tail = INCR( Q, Q->tail) ;
   rv = (int) T ;
   
   q_signal( Q->is_empty ) ;
   while( q_unlock( Q ) ) ;
   return rv ;
}

// pop the least recently added task off the queue ...
void *q_pop( Queue_t *Q )
{
  void *rv ;
  while( q_lock( Q ) ) ; 
  while( q_empty( Q ) ) q_wait( Q->is_empty, Q->lock ) ; 

  rv = Q->task[Q->head] ;
  Q->task[Q->head] = (void *) NULL ;
  Q->head = INCR( Q, Q->head ) ;

  q_signal( Q->is_full ) ;
  while( q_unlock( Q ) ) ;

  return rv ;
}

// get the number of elements in the queue (maybe) ...
int q_size( Queue_t *Q )
{
  int rv = -1 ;

  while( q_lock( Q ) ) ;
  rv = q_qty( Q ) ;
  while( q_unlock( Q ) ) ;

  return rv ;
}
