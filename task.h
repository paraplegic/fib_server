#ifndef TASK_DEFINED
#define TASK_DEFINED
#include <sys/time.h>

#define isNul( x )      ((void *) x == (void *) NULL )

typedef struct _tsk_ {
  unsigned ctime ;
  int client ;
  int request ;
  int reply ;
} Task_t ;

int     task_now( void ) ;
Task_t *task_crt( int client, int rqst, int reply ) ;
Task_t *task_del( Task_t *T ) ;

#endif // TASK_DEFINED
