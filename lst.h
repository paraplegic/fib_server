#ifndef LIST_INCLUDED
#define LIST_INCLUDED

#include <stdlib.h>

#define Kilo( x )	( 1024 * x )
#define isNul( x )	((void *) x == (void *) NULL )

typedef int (*Code_t)() ;
typedef struct _lst_ {
  Code_t siva ;
  int    argsz ; 
  int    argc ;
  void **argv ;
} List_t ;

List_t *lst_crt( int hint, Code_t destructor );
List_t *lst_dst( List_t *List );
void   *lst_get( List_t *List, int index );
int     lst_fnd( List_t *List, void *val );
int     lst_lkp( List_t *List, Code_t compare, void *Key );
int     lst_add( List_t *List, void *val );
int     lst_del( List_t *List, int index );
int     lst_siz( List_t *List );
int     lst_cap( List_t *List );
int     lst_exp( List_t *List, int newsz );

#endif // LIST_INCLUDED
