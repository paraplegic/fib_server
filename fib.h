#ifndef FIB_INCLUDED
#define FIB_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#ifndef FD_SETSIZE
  #define FD_SETSIZE	1024
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define isNul( x )	((void *) x == (void *) NULL )
#define FOREVER		(1 == 1)

#define buf_SIZE 64
#define q_SIZE 32

void cx_info( char *msg, int data ) ;
int cx_error( char *msg ) ;
int cx_die( char *msg ) ;
int cx_server_init( int wk_socket ) ;
int cx_server( int wk_socket ) ;
void *cx_write_task( void *arg );
void *cx_read_task( void *arg );
int cx_client_died( int client );
int cx_client_del( int client );
int cx_client_add( int client );
int cx_wellknown( int port );
int cx_next( int wk_socket );
int cx_read( int client );
int cx_write( int client, int value );
int cx_bad( int client, char *msg, int val ) ;

#endif // FIB_INCLUDED
