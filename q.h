#ifndef QUEUE_DEFINEd
#define QUEUE_DEFINEd

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define isNul( x )      ((void *) x == (void *) NULL )

typedef pthread_mutex_t Lock_t ;
typedef pthread_cond_t Cond_t ;

typedef struct _que_ {
  Lock_t *lock ;
  Cond_t *is_empty ;
  Cond_t *is_full ;
  int 	  head ;
  int     tail ;
  int     size ;
  void   **task ;
} Queue_t ;

#define INCR( q, val )	((val+1)%q->size)
#define DECR( q, val )	(((val-1) > -1)?(val-1):0)

Queue_t *q_create( int size ) ;
Queue_t *q_destroy( Queue_t *Q ) ;
int      q_push( Queue_t *Q, void *T ) ;
void    *q_pop( Queue_t *Q ) ;
int      q_size( Queue_t *Q ) ;

#endif // QUEUE_DEFINEd
