#include <stdio.h>
#include <unistd.h>
#include "task.h"
#include "q.h"

#define rqst_CONTINUE	1
#define rqst_EXIT	-1
#define FOREVER		(1 == 1)
#define Q_SIZE		512
#define N_THREADS	8
#define SLEEP		4

// must have usage info ...
void usage( int argc, char **argv )
{
   printf( "usage:  %s <N>\nwhere N=# of tasks.\n", argv[0] ) ;
   exit( 1 ) ;
}

void *cx_read_task( void *arg );

Queue_t *inp_Q = (Queue_t *) NULL ;
int bucket[N_THREADS] = {0};
int mike[N_THREADS] = {0};

int main( int argc, char **argv )
{
   int wk_socket, nx, cx, max ;
   pthread_t *threads[N_THREADS];

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
   printf( "Sleeping for %d seconds to synch %d threads.\n", SLEEP, N_THREADS ) ;
   sleep( SLEEP ); 

   // the main worker queue is filled here ... 
   for( cx = 0 ; cx < max ; cx++ )
   {
      // the task_crt does a malloc of a structure ...
      q_push( inp_Q, task_crt( 0, rqst_CONTINUE, -1 ) ) ;
//      printf( "+" ) ;
   }

   // tell the threads to exit ... 
   for( cx = 0 ; cx < N_THREADS ; cx++ )
   {
      q_push( inp_Q, task_crt( 0, rqst_EXIT, -1 ) ) ;
   }

   cx = 0 ; 
   int not_done = N_THREADS ; 
   while( not_done )
   {
      nx = pthread_join( *threads[cx], NULL ); 
      if( nx == 0 )
      {
        printf( "Thread %d joined.\n", cx ) ;
        not_done -= 1 ;
        cx++ ;
      }
   }

   int mikes = 0, sum = 0 ; 
   for( cx = 0 ; cx < N_THREADS ; cx++ )
   {
      sum += bucket[cx] ;
      mikes += mike[cx] ;
      printf( "bucket %d:\t%d misses: %d\n", cx, bucket[cx], mike[cx] ) ;
   }
   printf( "sum:\t\t%d (%d)\n", sum, (max-sum) ) ; 
   printf( "rem:\t %d\n", q_size( inp_Q ) ) ;
   printf( "misses:\t%d\n", mikes ) ;
   printf( "Q size: %d N threads: %d\n", Q_SIZE, N_THREADS ) ;

   inp_Q = q_destroy( inp_Q ) ;
   exit( 0 ) ;
}

void *cx_read_task( void *arg )
{
  Task_t *T ;
  long     thread ;

  thread = (long) arg ;
  bucket[thread] = 0 ;

  printf( "Thread %ld STARTED.\n", thread ) ;
  while( FOREVER )
  {
    if( isNul( inp_Q ) )
    {
      printf( "Input queue lost on thread %ld.\n", thread ) ;
      pthread_exit( (void *) NULL ) ; 
    }

    T = q_pop( inp_Q ) ;
    if( isNul( T ) )
    {
      printf( "Null task detected in thread %ld.\n", thread ) ;
      mike[thread] += 1;
      pthread_exit( (void *) NULL ) ; 
      continue ;
    }

    if( T->request == rqst_EXIT )
    {
      T = task_del( T ) ;
      printf( "Thread %ld TERMINATED.\n", thread ) ;
      pthread_exit( (void *) NULL ) ; 
    }

    // free's the structure, and count the trial ...
    T = task_del( T ) ;
    bucket[thread] += 1;
  }
  pthread_exit( (void *) NULL ) ; 
  return (void *) NULL ;
}
