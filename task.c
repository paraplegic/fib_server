#include <stdio.h>
#include <stdlib.h>
#include "task.h"

int task_created = 0 ; 
int task_destroy = 0 ; 

int task_now( void )
{
  struct timeval tv ;
  long rv = -1 ;

  if( !gettimeofday( &tv, NULL ) )
  {
     rv = ( tv.tv_sec * 1000000 ) + tv.tv_usec ; 
  }
  return rv ;
}

Task_t *task_crt( int client, int reqst, int reply )
{
   Task_t *rv ;
 
   rv = (Task_t *) calloc( 1, sizeof( Task_t ) ) ; 
   if( !isNul( rv ) )
   {
     rv->ctime = task_now() ;
     rv->client = client ;
     rv->request = reqst ;
     rv->reply = reply ;
     task_created++ ;
   }
   return rv ;
}

Task_t *task_del( Task_t *T )
{
  if( !isNul( T ) )
  {
    free( T ) ;
    task_destroy++ ;
  }
  return (Task_t *) NULL ;
}

int task_print( Task_t *T )
{
  if( !isNul( T ) )
    if( T->reply == -1 )
      return printf( "T @%u: (%d %d %d)\n", T->ctime, T->client, T->request, T->reply ) ;
    else
      return printf( "T @%u: (%d %d %u)\n", T->ctime, T->client, T->request, T->reply ) ;
  return -1 ;
}
