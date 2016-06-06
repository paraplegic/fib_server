#include "fib.h"
#include "task.h"
#include "q.h"

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

// general output routine ...
void cx_info( char *msg, int data )
{
   perror( "system reports:" );
   printf( msg, data ) ;
   fflush( stdout ) ;
   return ;
}

// error message routine ...
int cx_error( char *msg )
{
   perror( msg );
   fflush( stdout ) ;
   fflush( stderr ) ;
   return 1; 
}

// we aren't coming back from this ...
int cx_die( char *msg )
{
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

typedef enum {
  Unused = -1,
  Closed =  0,
  Active =  1
} Client_State_t ;

//
// each client gets it's own file descriptor and stream socket ...
int n_clients = 0 ; 
Client_State_t clients[FD_SETSIZE] = {Unused} ; 
#define client_add( client )	clients[client] = Active ; n_clients = client 
#define client_del( client )	clients[client] = Closed ; n_clients--

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
  int fd, max, cx, rv = -1 ;

  tm_out.tv_sec  = 5 ;
  tm_out.tv_usec = 0 ;

  max = 0 ; 
  FD_ZERO( &fd_read ) ; 
  for( fd = 0 ; fd < FD_SETSIZE; fd++ )
  {
    if( clients[fd] == Active )
    {
      FD_SET( fd, &fd_read ) ;
    }
    if( fd > max )
         max = fd ;
  }

  cx_info( "%d ", max ) ;
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
    cx_client_add( cx ) ; 
  } 

  for( fd = 0 ; fd < n_clients ; fd++ )
  {
      if( FD_ISSET( fd, &fd_read ) )
      {
         q_push( inp_Q, task_crt( clients[fd], -1, -1 ) ) ;
      }
  }

  return rv ;
}

void *cx_write_task( void *arg )
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

void *cx_read_task( void *arg )
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

int cx_client_died( int client )
{
  return cx_client_del( client ) ;
}

int cx_close( int client )
{
  return cx_client_del( client );
}

int cx_client_del( int client )
{
  int i, nx ;

  cx_info( "closing file on socket %d.\n", client ) ;
  nx = close( client ); 
  if( nx < 0 )
  {
    switch( errno ) {
      case EIO:
      case EBADF:
        cx_info( "ERROR closing file on socket %d.\n", client ) ;
        cx_die( "ERROR closing file" ) ;
        break ;
      case EINTR:
      default:
        break ;
    }
  }
  client_del( client );
  return 1;
}

int cx_client_add( int client )
{
  cx_info( "adding client: %d.\n", client ) ;
  return 1; 
}

/* open a socket on a well known port (all interfaces) */
int cx_wellknown( int port )
{
   struct sockaddr_in server ;
   int rv = -1 ;

   rv = socket(AF_INET, SOCK_STREAM, 0);
   if (rv < 0) 
   {
     cx_die("ERROR opening socket");
   }

   if (setsockopt( rv, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
   {
     cx_die("ERROR setsockopt (SO_REUSEADDR) failed");
   }

   bzero( &server, sizeof( server ) ) ;
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons( port ) ;

   if( bind( rv, (const struct sockaddr *) &server, sizeof( server ) ) < 0 )
   {
     cx_die("ERROR binding socket");
   }

   if( listen( rv, 5 ) < 0 )
   {
     cx_die( "ERROR on listen" );
   }

   cx_setsignals() ;
   client_add( rv ) ;
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
    cx_die( "ERROR on accept" ) ;
  }
  client_add( rv ) ;
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
    cx_die( "ERROR on read from client" ) ;
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
      if( errno == EAGAIN ) goto again;
      cx_die( "ERROR on write to client" ) ;
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
      if( errno == EAGAIN ) goto again;
      cx_die( "ERROR on write to client" ) ;
    }
  } 
  return nx ;
}

int cx_open( char *host, char *u_port )
{
    int rv, port ;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    port = atol( u_port );
    if( port < 0 )
    {
        printf( "ERROR illegal port specified: %s\n", u_port );
    }

    rv = socket(AF_INET, SOCK_STREAM, 0);
    if ( rv < 0 )
    {
        printf( "ERROR opening socket\n" );
    }

    server = gethostbyname( host );
    if( isNul( server ) ) {
        printf( "ERROR, no such host\n");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if( connect( rv, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 )
    {
        printf("ERROR connectingi\n");
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

   // input and work queues created here ...
   inp_Q = q_create( 128, (Func_t *) task_del ) ;
   wrk_Q = q_create( 128, (Func_t *) task_del ) ;

   // just a few worker threads: 2 readers and 3 writers ... 
   cx = 0 ; 
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_read_task, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_read_task, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_read_task, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_write_task, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_write_task, NULL ) ;
   pthread_create( (pthread_t *) &threads[cx++], NULL, cx_write_task, NULL ) ;

   // if this returns, it should be a good socket, on a well known port
   // as provided at run time as an argument ... should be > 1024 ... 
   cx_info( "Starting service on port %d: ", atol( argv[1] ) ) ; 
   wk_socket = cx_wellknown( atol( argv[1] ) ) ;
   cx_info( "Done.\n", 0 ) ; 
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
