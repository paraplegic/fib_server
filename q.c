#include "q.h"

int q_unlock( Queue_t *Q )
{
   return pthread_mutex_unlock( Q->lock ) ;
}

int q_lock( Queue_t *Q )

{
   return pthread_mutex_lock( Q->lock ) ;
}

int q_signal( Cond_t *condition )
{
   return pthread_cond_signal( condition ) ;
}

int q_wait( Cond_t *condition, Lock_t *lock )
{
   return pthread_cond_wait( condition, lock ) ;
}

int q_qty( Queue_t *Q )
{
  return Q->head - Q->tail ;
}

int q_avail( Queue_t *Q )
{
  return Q->size - q_qty( Q ) ;
}

int q_empty( Queue_t *Q )
{
  return q_avail( Q ) == Q->size ;
}

int q_full( Queue_t *Q )
{
   return q_avail( Q ) < 1 ;
}

Queue_t *q_create( int size )
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

Queue_t *q_destroy( Queue_t * Q )
{
  if( !isNul( Q ) )
  {
    if( !isNul( Q->lock ) ) free( Q->lock ) ;
    if( !isNul( Q->is_empty ) ) free( Q->is_empty ) ;
    if( !isNul( Q->is_full ) ) free( Q->is_full ) ;
    if( !isNul( Q->task ) ) free( Q->task ) ;
    memset( Q, 0, sizeof( Queue_t ) ) ;
    free( Q ) ;
  }
  return (Queue_t *) NULL ;
}

int q_push( Queue_t *Q, void *T )
{
   q_lock( Q );
   while( q_full( Q ) ) q_wait( Q->is_full, Q->lock ) ;

   Q->task[Q->head] = T ;
   Q->head = INCR( Q, Q->head ) ;

   q_signal( Q->is_empty ) ;
   q_unlock( Q ) ;
}

void *q_pop( Queue_t *Q )
{
  void *rv ;
  q_lock( Q ); 
  while( q_empty( Q ) ) q_wait( Q->is_empty, Q->lock ) ; 

  rv = Q->task[ Q->tail ] ;
  Q->tail = INCR( Q, Q->tail ) ;

  q_signal( Q->is_full ) ;
  q_unlock( Q ) ;

  return rv ;
}
