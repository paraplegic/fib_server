#include "lst.h"

List_t *lst_crt( int hint, Code_t destructor )
{
  List_t *rv = (List_t *) NULL;

  rv = calloc( 1, sizeof( List_t ) );
  if( !isNul( rv ) )
  {
    rv ->siva = destructor ;
    rv ->argsz = hint ;
    rv ->argc = 0 ; 
    rv ->argv = calloc( hint, sizeof( void * ) ) ;
  }
  return rv ;
}

List_t *lst_dst( List_t *List )
{
   int i ; 
   if( !isNul( List ) )
   {
      if( !isNul( List ->siva ) )
      {
         for( i = 0 ; i < List->argc ; i++ )
         {
            if( !isNul( List->argv[i] ) )
               List->siva( List->argv[i] ) ;
         }
      }
      free( List ->argv ) ;
      List ->argv = (void **) NULL ; 
      free( List ) ;
      List = (List_t *) NULL ; 
   }
   return List ;
}

int lst_siz( List_t *List )
{
   if( !isNul( List ) )
     return List ->argc ;

   return -1 ;
}

int lst_cap( List_t *List )
{
   if( !isNul( List ) )
     return List ->argsz ;

   return -1 ;
}

void   *lst_get( List_t *List, int index )
{
   if( index < List->argsz )
   {
      return List ->argv[index] ;
   }
   return (void *) NULL ;
}

int     lst_del( List_t *List, int index )
{
   int i ;
   if( index < List->argc )
   {
      if( !isNul( List ->siva ) )
      {
        if( !isNul( List->argv[index] ) )
          List->siva( List->argv[index] ) ;
      }
      for( i = index ; i < List->argc ; i++ )
      {
        List->argv[i] = List->argv[i+1] ;
      }
      List->argc-- ;
      return index ;
   }
   return -1 ;
}

int     lst_fnd( List_t *List, void *val )
{
   int i ;

   if( !isNul( List ) )
   {
      for( i = 0 ; i < List->argc ; i++ )
      {
          if( List->argv[i] == val )
            return i ; 
      }
   }
   return -1 ;
}

int     lst_add( List_t *List, void *val )
{
   int newsz ;
   if( List ->argc < List->argsz )
   {
       List->argv[ List->argc++ ] = val ;
       return 1 ;
   } 

   newsz = ( List->argsz > Kilo( 1 ) ) ?  (int)( List->argsz * 1.2 ) : (int) (List->argsz * 2) ;
   if( lst_exp( List, newsz ) )
      return lst_add( List, val ) ;

   return -1 ;
}

int     lst_exp( List_t *List, int newsz )
{
   int i, tmp ; 
   if( List ->argsz < newsz )
   {
      List ->argv = realloc( List ->argv, (newsz * sizeof( void * )) ) ;
      tmp = List->argsz ;
      List->argsz = newsz ;
      for( i = tmp ; i < newsz ; i++ )
      {
        List->argv[i] = (void *) NULL ; 
      }
      return 1 ;
   }
   return 0 ;
}
