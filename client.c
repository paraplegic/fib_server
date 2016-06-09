#include <stdio.h>
#include "task.h"
#include "fib.h"
#include "q.h"

#define N_TRYS	1024
#define FIB_MAX	  40

int main( int argc, char **argv )
{
  char *u_host, *u_port ;
  int   host, i, fib ;
  int   t_start, t_end, start, end ;
  double latency ;

  if( argc < 3 )
  {
    usage();
  }

  cx_init( argc, argv ) ;

  u_host = argv[1];
  u_port = argv[2];

  host = cx_open( u_host, u_port );
  if( host < 0 )
  {
	printf( "unable to open host: %s on port %s.\n", u_host, u_port );
  }

  latency = 0;
  t_start = task_now() ;
  for( i = 0 ; i < N_TRYS ; i++ )
  {
     start = task_now() ;
     cx_write( host, 20 );
     fib = cx_read( host );
     end = task_now() ;
     printf( "%d ", fib ) ;

     latency += (double) end - (double) start ;
  }
  printf( "\n" ) ;
  cx_close( host );
  t_end = task_now() ;
  latency = latency / N_TRYS ;

  printf( "%d requests in %8.3f secs average latency: %8.3f msecs\n", N_TRYS, (double)((t_end - t_start)/1000000.0), latency/1000.0 ) ;
  return 0;
}
