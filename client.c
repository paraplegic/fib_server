#include <stdio.h>
#include "task.h"
#include "fib.h"
#include "q.h"

#define N_TRYS	20480
#define FIB_MAX	  40
#define MYFIB	  10

void client_usage( int argc, char **argv )
{
  printf( "%s: <host> <port>\n", argv[0] ) ;
  exit( 1 ) ;
}

int main( int argc, char **argv )
{
  char *u_host, *u_port ;
  int   host, i, fib ;
  int   t_start, t_end, start, end ;
  double latency ;

  if( argc < 3 )
  {
    client_usage( argc, argv );
  }

  u_host = argv[1];
  u_port = argv[2];

  cx_init( argc, argv ) ;
  host = cx_open( u_host, u_port );
  if( host < 0 )
  {
    perror( "ERROR" );
    exit( host );
  }

  latency = 0;
  t_start = task_now() ;
  for( i = 0 ; i < N_TRYS ; i++ )
  {
     int rqst = i % 40 ;
     start = task_now() ;
     cx_write( host, rqst );
     fib = cx_read( host );
     end = task_now() ;
     // printf( "%d ", fib ) ;

     latency += (double) end - (double) start ;
  }
  // printf( "\n" ) ;
  cx_close( host );
  t_end = task_now() ;
  latency = latency / N_TRYS ;

  printf( "%d requests in %8.3f secs average latency: %8.3f msecs\n", N_TRYS, (double)((t_end - t_start)/1000000.0), latency/1000.0 ) ;
  return 0;
}
