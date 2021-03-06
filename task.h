#ifndef TASK_DEFINED
#define TASK_DEFINED
#include <sys/time.h>

#define isNul( x )      ((void *) x == (void *) NULL )

// tasks carry a creation timestamp (microseconds), and
// the file descriptor, the request and the reply.  This
// is basically all you need to use for select(), and where
// the client will always be an int file descriptor, the request
// and reply might be strings, or other stuff ... 

typedef struct _tsk_ {
  unsigned ctime ;
  int client ;
  int request ;
  int reply ;
} Task_t ;

int     task_now( void ) ;
Task_t *task_crt( int client, int rqst, int reply ) ;
Task_t *task_del( Task_t *T ) ;
int		task_print( Task_t *T ) ;

#endif // TASK_DEFINED
