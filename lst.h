#ifndef LIST_INCLUDED
#define LIST_INCLUDED

typedef (*fun)() Code_t ;
typedef struct _lst_ {
  int    argsz ; 
  int    argc ;
  void **argv ;
} List_t ;

List_t *lst_crt( int hint, Code_t *destructor );
List_t *lst_dst( List_t *List );
void   *lst_get( List_t *List, int index );
int     lst_del( List_t *List, int index );
int     lst_add( List_t *List, void *val );
int     lst_fnd( List_t *List, void *key );


#endif // LIST_INCLUDED
