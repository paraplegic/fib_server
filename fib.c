#include "fib.h"
#include "task.h"
#include "q.h"
#include "lst.h"

// an (intentionally) inefficient fib generator which 
// moves very slowly beyond input number 44 ... and time
// of execution is a log function of the input value ...
int fib( int n )
{
   if( n <= 2 )
      return 1;

   return fib( n - 1 ) + fib( n - 2 ) ;
}

// must have usage info ...
void usage( int argc, char **argv )
{
   printf( "usage:  %s <port>\n", argv[0] ) ;
   exit( 1 ) ;
}


char *program_name = (char *) NULL ;

void cx_init( int argc, char **argv )
{
  program_name = argv[0] ; 
}

// general output routine ...
void cx_info( char *msg, int data )
{
   printf( "INFO %s: ", program_name ) ;
   printf( msg, data ) ;
   fflush( stdout ) ;
   return ;
}

// error message routine ...
int cx_error( char *msg )
{
   printf( "ERROR %s: ", program_name ) ;
   perror( msg );
   fflush( stdout ) ;
   fflush( stderr ) ;
   return 1; 
}

// we aren't coming back from this ...
int cx_die( char *msg )
{
   printf( "FATAL %s: ", program_name ) ;
   perror( msg );
   fflush( stdout ) ;
   fflush( stderr ) ;
   exit( 1 );
}

// select information:
//   read, write and exception sets, and timeouts (if used) ...
//   they needn't be global, but were prototyped this way.
fd_set fd_read  ;
fd_set fd_write ;
int    fd_num = 0 ; 
struct timeval tm_out ;

//
// each client gets it's own file descriptor and stream socket ...
List_t *client_list = (List_t *) NULL ;

//
// work moves from select() to the input queue, and
// from there to the work queue, back to the client
// this is how we pass a work assignment to the threads ...
Queue_t *inp_Q = (Queue_t *) NULL ;
Queue_t *wrk_Q = (Queue_t *) NULL ;

int cx_setsignals( void )
{
   signal( SIGPIPE, SIG_IGN ) ;
}

int cx_server( int wk_socket )
{
  int fd, max, mx, cx, rv = -1 ;

  tm_out.tv_sec  = 5 ;
  tm_out.tv_usec = 0 ;

  max = 0 ; 
  mx = lst_siz( client_list ) ; 
  FD_ZERO( &fd_read ) ; 
  for( cx = 0 ; cx < mx; cx++ )
  {
    fd = (int) lst_get( client_list, cx ); 
    FD_SET( fd, &fd_read ) ;
    if( fd > max )
         max = fd ;
  }

  fd_num = max+1 ;
  rv = select( fd_num, &fd_read, NULL, NULL, &tm_out ) ;
  if( rv < 0 )
  {
    cx_error( "select() returned -1" ) ;
    return rv ;
  }

  if( FD_ISSET( wk_socket, &fd_read ) )
  {
    cx = cx_next( wk_socket ) ;
  } 

  mx = lst_siz( client_list ) ;
  for( cx = 1 ; cx < mx ; cx++ )
  {
      fd = (int) lst_get( client_list, cx ) ;
      if( FD_ISSET( fd, &fd_read ) )
      {
         q_push( inp_Q, task_crt( fd, -1, -1 ) ) ;
      }
  }

  return rv ;
}

void *cx_task_write( void *arg )
{
  Task_t *T ;

  while( FOREVER )
  {
    T = q_pop( wrk_Q ) ;
    if( isNul( T ) ) 
        continue ;

    if( T->request < 50 ) 
    {

      T-> reply = fib( T->request ) ;
      cx_write( T->client, T->reply ) ;

    } else 
        cx_bad( T->client, "server refused fib(%d) (too large).\n", T->request ) ;

//    task_print( T ) ;
    T = task_del( T ) ; 
  }
}

void *cx_task_read( void *arg )
{
  Task_t *T ;

  while( FOREVER )
  {
    T = q_pop( inp_Q ) ;
    if( isNul( T ) )
      continue ;

    T->request = cx_read( T->client ) ;
//    task_print( T ) ;

    q_push( wrk_Q, T ) ;
  }
}

int cx_close( int client )
{
  return cx_client_del( client );
}

int cx_client_del( int client )
{
  int i, nx ;

  nx = lst_fnd( client_list, (void *) client ) ;
  if( nx < 0 )
     return 0 ;

  lst_del( client_list, nx ) ;
//  cx_info( "Closing file on socket %d.\n", client ) ;
  nx = close( client ); 
  if( nx < 0 )
  {
    switch( errno ) {
      case EIO:
      case EBADF:
        cx_info( "Closing bad file descriptor %d.\n", client ) ;
        break ;
      case EINTR:
      default:
        break ;
    }
  }
  return 1;
}

// client_list is a global list ... 
int cx_client_add( int client )
{
  if( isNul( client_list ) )
  {
      client_list = lst_crt( 1, NULL ) ;
  }
  return (int) lst_add( client_list, (void *) client ) ;
}

/* open a socket on a well known port (all interfaces) */
int cx_wellknown( int port )
{
   struct sockaddr_in server ;
   int rv = -1 ;

   rv = socket(AF_INET, SOCK_STREAM, 0);
   if (rv < 0) 
   {
     cx_die("ERROR: opening socket");
   }

   if (setsockopt( rv, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
   {
     cx_die("ERROR: setsockopt (SO_REUSEADDR) failed");
   }

   bzero( &server, sizeof( server ) ) ;
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons( port ) ;

   if( bind( rv, (const struct sockaddr *) &server, sizeof( server ) ) < 0 )
   {
     cx_die("ERROR: binding socket");
   }

   if( listen( rv, 5 ) < 0 )
   {
     cx_die( "ERROR: on listen" );
   }

   cx_setsignals() ;
   cx_client_add( rv ) ;
   return rv ;
}

/* refactor to return a child fd */
int cx_next( int wk_socket )
{
  struct sockaddr_in client ;
  int rv = -1;

  socklen_t siz = sizeof( client ) ;
  rv = accept( wk_socket, (struct sockaddr *) &client, (socklen_t *) &siz );
  if( rv < 0 )
  {
//    cx_die( "ERROR: on accept" ) ;
    return rv ;
  }
  cx_client_add( rv ) ;
  return rv ;
}

/* read a positive integer from client */
int cx_read( int client )
{
  char buf[buf_SIZE] ;
  int nx ;

 again:
  bzero( buf, sizeof( buf ) ) ;
  nx = read( client, buf, sizeof( buf ) );
  if( nx < 0 )
  {
    if( errno == EAGAIN ) 
        goto again;
    return nx ;
  }

  if( nx == 0 )
  {
    cx_client_del( client ) ;
    return nx ;
  }
  return atol( buf ) ;
}

/* write an integer to client */
int cx_write( int client, int value )
{
  char buf[buf_SIZE];
  int i, nx ;

  bzero( buf, sizeof( buf ) ) ;
  nx = snprintf( buf, sizeof( buf ), "%u\n", value ) ;
  if( nx > 0 )
  {
 again:
    nx = write( client, buf, strlen( buf ) ) ;
    if( nx < 0 )
    {
      if( errno == EAGAIN ) 
          goto again;
    }
    if( nx == 0 )
    {
      cx_close( client ) ;
      if( errno == EPIPE )
      {
        cx_info( "EPIPE detected on descriptor %d", client ) ;
      }
    }
  } 
  return nx ;
}

int cx_bad( int client, char *msg, int val )
{
  char buf[buf_SIZE];
  int nx ;

  nx = snprintf( buf, sizeof( buf ), msg, val ) ;
  if( nx > 0 )
  {
 again:
    nx = write( client, buf, strlen( buf ) ) ;
    if( nx < 0 )
    {
      if( errno == EAGAIN ) 
           goto again;
    }
  } 
  return nx ;
}

int cx_open( char *host, char *u_port )
{
    int rv, port ;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    rv = -1 ;
    port = atol( u_port );
    if( port < 0 )
    {
        printf( "ERROR: illegal port specified: %s\n", u_port );
        return rv ;
    }

    rv = socket(AF_INET, SOCK_STREAM, 0);
    if ( rv < 0 )
    {
        printf( "ERROR: opening socket to %s:%d\n", host, port );
        return rv ;
    }

    server = gethostbyname( host );
    if( isNul( server ) ) {
        printf( "ERROR: gethost cannot find host %s:%d\n", host, port );
        return -1 ;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if( connect( rv, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 )
    {
        printf("ERROR: connecting to %s:%d.\n", host, port );
        return -1 ;
    }

    cx_setsignals() ;
    return rv ;

}

#ifdef TEST

int main( int argc, char **argv )
{
   int wk_socket, cx ;
   pthread_t *threads[10];

   if( argc < 2 )
   {
     usage( argc, argv ) ;
   }

   cx_init( argc, argv ) ; 

   // input and work queues created here ...
   inp_Q = q_create( 128, (Func_t *) task_del ) ;
   wrk_Q = q_create( 128, (Func_t *) task_del ) ;

   // just a few worker threads: 2 readers and 3 writers ... 
   cx = 0 ; 
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_read, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_read, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_read, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_read, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_write, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_write, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_write, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_task_write, NULL ) ;

   // if this returns, it should be a good socket, on a well known port
   // as provided at run time as an argument ... should be > 1024 ... 
   cx_info( "Starting service on port %d\n", atol( argv[1] ) ) ; 
   wk_socket = cx_wellknown( atol( argv[1] ) ) ;
   while( FOREVER )
   {
      cx_server( wk_socket ) ;
   }

   // no mechanism is available yet to shut down this server ...
   // but we should design that in, and if so, clean up the queues,
   // flush and close the clients, and in general .. clean up ...
   inp_Q = q_destroy( inp_Q ) ;
   wrk_Q = q_destroy( wrk_Q ) ;
   cx_info( "Unexpected server return.\n", 0 ) ; 
   exit( 1 ) ;
}

#endif /* TEST */
