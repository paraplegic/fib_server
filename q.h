#ifndef QUEUE_DEFINEd
#define QUEUE_DEFINEd

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef pthread_mutex_t Lock_t ;
typedef pthread_cond_t Cond_t ;
typedef void *(Func_t( void * ));

typedef struct _que_ {
  Lock_t *lock ;		// lock for the entire queue ...
  Cond_t *is_empty ;		// queue is empty ...
  Cond_t *is_full ;		// queue is full ...
  Func_t *siva ;		// the destroyer ...
  int 	  head ;		// add to the head ...
  int     tail ;		// remove from the tail ...
  int     size ;		// user will tell us ...
  void   **task ;		// opaque task pointer array ...
} Queue_t ;

#define isNul( x )      ((void *) x == (void *) NULL )
#define INCR( q, val )	((val+1)%q->size)
#define DECR( q, val )	(((val-1) > -1)?(val-1):0)


/*
   -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   public interface ... 
   -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

Queue_t *q_create( int size, Func_t *siva ) ;
Queue_t *q_destroy( Queue_t *Q ) ;
int      q_push( Queue_t *Q, void *T ) ;
void    *q_pop( Queue_t *Q ) ;
int	 q_size( Queue_t *Q ) ;

#endif // QUEUE_DEFINEd
