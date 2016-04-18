#include <stdio.h>
#include "task.h"
#include "q.h"

#define rqst_CONTINUE	1
#define rqst_EXIT	-1
#define FOREVER		(1 == 1)
#define Q_SIZE		512
#define N_THREADS	4

// must have usage info ...
void usage( int argc, char **argv )
{
   printf( "usage:  %s <N>\nwhere N=# of tasks.\n", argv[0] ) ;
   exit( 1 ) ;
}

void *cx_read_task( void *arg );

Queue_t *inp_Q = (Queue_t *) NULL ;
int bucket[4] = {0};

int main( int argc, char **argv )
{
   int wk_socket, nx, cx, max ;
   pthread_t *threads[10];

   if( argc < 2 )
   {
     usage( argc, argv ) ;
   }

   max = atol( argv[1] ) ;

   // input and work queues created here ...
   inp_Q = q_create( Q_SIZE, (Func_t *) task_del ) ;

   // just a few worker threads: N_THREADS readers ... 
   for( cx = 0 ; cx < N_THREADS ; cx++ )
   {
     nx = pthread_create( (pthread_t *) &threads[cx], NULL, cx_read_task, (void *) ((long) cx) ) ;
     if( nx != 0 )
     {
        printf( "create thread %d returns %d.\n", cx, nx ) ; 
        exit( 1 ) ; 
     }
   }

   for( cx = 0 ; cx < max ; cx++ )
   {
      // the task_crt does a malloc of a structure ...
      q_push( inp_Q, task_crt( 0, rqst_CONTINUE, -1 ) ) ;
   }

   q_push( inp_Q, task_crt( 0, rqst_EXIT, -1 ) ) ;
   q_push( inp_Q, task_crt( 0, rqst_EXIT, -1 ) ) ;
   q_push( inp_Q, task_crt( 0, rqst_EXIT, -1 ) ) ;
   q_push( inp_Q, task_crt( 0, rqst_EXIT, -1 ) ) ;
   sleep( 2 ) ;

   int sum = 0 ; 
   for( cx = 0 ; cx < 4 ; cx++ )
   {
      sum += bucket[cx] ;
      printf( "bucket %d:\t%d\n", cx, bucket[cx] ) ;
   }
   printf( "sum: %d\n", sum ) ; 
   printf( "rem: %d\n", q_size( inp_Q ) ) ;

   inp_Q = q_destroy( inp_Q ) ;
   exit( 0 ) ;
}

void *cx_read_task( void *arg )
{
  Task_t *T ;
  long     thread ;

  thread = (long) arg ;

  bucket[thread] = 0 ;
  while( FOREVER )
  {
    T = q_pop( inp_Q ) ;
    if( isNul( T ) )
    {
      printf( "Null task detected in thread %d.\n", thread ) ;
      continue ;
    }

    if( T->request == rqst_EXIT )
    {
      printf( "Thread %d terminated.\n", thread ) ;
      pthread_exit( (void *) NULL ) ; 
    }
    bucket[thread] += 1;

    // free's the structure 
    T = task_del( T ) ;
  }
  return (void *) NULL ;
}
