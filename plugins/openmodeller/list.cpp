/**
 * Definition of list templates.
 * 
 * @file
 * @author Mauro E S Munoz, Josue O Freitas Jr
 * @date   1995-10-05
 * $Id$
 * 
 * LICENSE INFORMATION 
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include "list.hh"


/******************************************************/
/************************ List ************************/

/*******************/
/*** Constructor ***/

template<class T>
List<T>::List()
{
  f_size = 0;
  f_head = f_node = (ListNode<T> *)0;
}



/******************/
/*** Destructor ***/

template<class T>
List<T>::~List()
{
  Clear();
}



/**************/
/*** Append ***/

template<class T>
void
List<T>::Append( T elem )
{
  ListNode<T> *new_node = new ListNode<T>( elem, 0 );

  if ( !f_head )
    f_head = f_node = new_node;
  else
    {
      ListNode<T> *last = f_head;
      while ( last->next )
	last = last->next;
      last->next = new_node;
    }

  f_size++;
}



/***************/
/*** Insert  ***/

template<class T>
void
List<T>::Insert( T elem )
{
  ListNode<T> *new_node = new ListNode<T>( elem, 0 );

  // If first node.
  if ( !f_head )
    f_head = f_node = new_node;

  // If current node is out of the list, insert at the beginning.
  else if ( !f_node )
    {
      new_node->next = f_head;
      f_head = f_node = new_node;
    }

  else
    {
      new_node->next = f_node->next;
      f_node->next = new_node;
    }

  f_size++;
}



/********************/
/*** Insert First ***/

template<class T>
void
List<T>::InsertFirst( T elem )
{
   f_node = f_head = new ListNode<T>( elem, f_head );
   f_size++;
}



/*************/
/*** Clear ***/

template<class T>
void
List<T>::Clear()
{
  f_size = 0;
  f_node = f_head;
  while ( f_node )
    {
      f_head = f_head->next;
      delete( f_node );
      f_node = f_head;
    }
}


 


/*******************************************************/
/************************ DList ************************/

/*******************/
/*** Constructor ***/

template<class T>
DList<T>::DList()
{
  f_size = 0;
  f_head = f_tail = f_node = 0;
}


/******************/
/*** Destructor ***/

template<class T>
DList<T>::~DList()
{
  Clear();
}


/***********/
/*** Get ***/

template<class T>
int
DList<T>::Get( T *pe )
{
  if ( f_node )
    {
      *pe = f_node->elem;
      return( 1 );
    }
  else
    {
      *pe = T( 0 );
      return( 0 );
    }
}


/*********************/
/*** Insert Before ***/

template<class T>
void
DList<T>::InsertBefore( T elem )
{
  TNode *new_node = new TNode( elem, 0, 0 );

  // If first node.
  if ( !f_head )
    f_head = f_tail = f_node = new_node;

  // If end of list.
  else if ( !f_node )
    {
      f_tail->next = new_node;
      new_node->prev = f_tail;
      f_tail = new_node;
    }

  // Insert before current node.
  else
    {
      if ( f_node->prev )
	f_node->prev->next = new_node;
      else
	f_head = new_node;
      new_node->next = f_node;
      new_node->prev = f_node->prev;
      f_node->prev   = new_node;
    }

  f_size++;
}


/********************/
/*** Insert After ***/

template<class T>
void
DList<T>::InsertAfter( T elem )
{
  TNode *new_node = new TNode( elem, 0, 0 );

  // If first node.
  if ( !f_head )
    f_head = f_tail = f_node = new_node;

  // If end of list.
  else if ( !f_node )
    {
      f_tail->next = new_node;
      new_node->prev = f_tail;
      f_tail = new_node;
    }

  // Insert after current node.
  else
    {
      if ( f_node->next )
	f_node->next->prev = new_node;
      else
	f_tail = new_node;
      new_node->prev = f_node;
      new_node->next = f_node->next;
      f_node->next   = new_node;
    }

  f_size++;
}


/********************/
/*** Insert First ***/

template<class T>
void
DList<T>::InsertFirst( T elem )
{
  TNode *new_node = new TNode( elem, 0, f_head );

  // If first node.
  if ( !f_head )
    f_tail = f_node = new_node;
  else
    f_head->prev = new_node;
  
  f_head = new_node;
  f_size++;
}


/*******************/
/*** Insert Last ***/

template<class T>
void
DList<T>::InsertLast( T elem )
{
  TNode *new_node = new TNode( elem, f_tail, 0 );

  // If first node.
  if ( !f_tail )
    f_head = f_node = new_node;
  else
    f_tail->next = new_node;
  
  f_tail = new_node;
  f_size++;
}


/****************/
/*** Exchange ***/

template<class T>
void
DList<T>::Exchange( DList<T> &l )
{
  TNode *aux;
  
  aux      = f_head;
  f_head   = l.f_head;
  l.f_head = aux;
    
  aux      = f_tail;
  f_tail   = l.f_tail;
  l.f_tail = aux;
    
  aux      = f_node;
  f_node   = l.f_node;
  l.f_node = aux;

  int size = f_size;
  f_size   = l.f_size;
  l.f_size = size;
}


/**************/
/*** Delete ***/

template<class T>
T
DList<T>::Delete()
{
  if ( !f_node )
    return( T(0) );

  T ret = f_node->elem;

  if ( f_node->next )
    f_node->next->prev = f_node->prev;

  if ( f_node->prev )
    f_node->prev->next = f_node->next;

  if ( f_head == f_node )
    f_head = f_node->next;

  if ( f_tail == f_node )
    f_tail = f_node->prev;

  TNode *old = f_node;
  f_node = f_node->next;

  delete( old );
  f_size--;

  return( ret );
}


/*************/
/*** Clear ***/

template<class T>
void
DList<T>::Clear()
{
  f_size = 0;
  f_node = f_head;
  while ( f_node )
    {
      f_head = f_head->next;
      delete( f_node );
      f_node = f_head;
    }

  f_head = f_tail = f_node = 0;
}



/*************/
/*** Print ***/

template<class T>
void
DList<T>::Print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "Size: %d.\n", f_size );
  
  TNode *p = f_head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == f_node ? '>' : ' ' );
      printf( "  Elem:  %p\n", p->elem );
      printf( "  Prev:  %p\n", p->prev );
      printf( "  Prox:  %p\n", p->next );
    }
  printf( "%s", bottom_delimiter );
}


 


/*********************************************************/
/************************ OrdList ************************/

/*******************/
/*** Constructor ***/

template<class T, class K>
OrdList<T,K>::OrdList( char order )
{
  f_size  = 0;
  f_order = order;
  f_prev  = &f_head;
  f_head  = f_node = 0;
}



/******************/
/*** Destructor ***/

template<class T, class K>
OrdList<T,K>::~OrdList()
{
  Clear();
}



/************/
/*** Next ***/

template<class T, class K>
void
OrdList<T,K>::Next()
{
  if ( f_node )
    {
      f_prev = &(f_node->next);
      f_node = f_node->next;
    }
  
}



/************/
/*** Goto ***/

template<class T, class K>
int
OrdList<T,K>::Goto( int index )
{
  Head();
  if ( index >= f_size )
    return( 0 );

  for ( int i = 0; i < index; i++ )
    Next();

  return( index );
}



/***************/
/*** Insert  ***/

template<class T, class K>
void
OrdList<T,K>::Insert( T elem, K key )
{
  OrdListNode<T,K> *new_node = new OrdListNode<T,K>( elem, key, 0 );

  // If first node.
  if ( !f_head )
    {
      f_prev = &f_head;
      f_head = f_node = new_node;
    }

  // Search in the list to insert in the correct position.
  else
    {
      // Go back to the beginning of the list if the position
      // of the new node has passed.
      if ( !f_node || ((f_order*f_node->key) >= (f_order*key)) )
	{
	  f_prev = &f_head;
	  f_node = f_head;
	}

      // Look after the correct position.
      while ( f_node && ((f_order*f_node->key) < (f_order*key)) )
	{
	  f_prev = &(f_node->next);
	  f_node = f_node->next;
	}

      // Insert new node.
      new_node->next = f_node;
      *f_prev = new_node;
      f_prev  = &(new_node->next);
    }

  f_size++;
}



/********************/
/*** Insert After ***/

template<class T, class K>
void
OrdList<T,K>::InsertAfter( T elem, K key )
{
  OrdListNode<T,K> *new_node = new OrdListNode<T,K>( elem, key, 0 );

  // If first node.
  if ( !f_head )
    {
      f_prev = &f_head;
      f_head = f_node = new_node;
    }

  // Search in the list to insert in the correct position.
  else
    {
      // Go back to the beginning of the list if the position
      // of the new node has passed.
      if ( !f_node || ((f_order*f_node->key) > (f_order*key)) )
	{
	  f_prev = &f_head;
	  f_node = f_head;
	}

      //  Look after the correct position.
      while ( f_node && ((f_order*f_node->key) <= (f_order*key)) )
	{
	  f_prev = &(f_node->next);
	  f_node = f_node->next;
	}

      // Insert new node.
      new_node->next = f_node;
      *f_prev = new_node;
      f_prev  = &(new_node->next);
    }

  f_size++;
}



/**************/
/*** Delete ***/

template<class T, class K>
T
OrdList<T,K>::Delete()
{
  if ( !f_node )
    return( T(0) );

  T ret = f_node->elem;

  *f_prev  = f_node->next;
  delete( f_node );
  f_node = *f_prev;
  f_size--;

  return( ret );
}



/**************/
/*** Delete ***/

template<class T, class K>
T
OrdList<T,K>::Delete( T x )
{
  T elem;
  for ( Head(); elem = Get(); Next() )
    if ( elem == x )
      return( Delete() );

  return( T(0) );
}



/*************/
/*** Clear ***/

template<class T, class K>
void
OrdList<T,K>::Clear()
{
  f_size = 0;
  f_node = f_head;
  while ( f_node )
    {
      f_head = f_head->next;
      delete( f_node );
      f_node = f_head;
    }

  f_prev = &f_head;
  f_head = f_node = (OrdListNode<T,K> *)0;
}



/*************/
/*** Print ***/

template<class T, class K>
void
OrdList<T,K>::Print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "Order:   %s.\n", f_order > 0 ? "asc" : "desc" );
  printf( "Size: %d.\n", f_size );
  
  OrdListNode<T,K> *p = f_head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == f_node ? '>' : ' ' );
      printf( " Key: %f\n", (float)p->key );
      printf( "  Elem:  %p\n", p->elem );
      printf( "  Prev:  %p\n", p->prev );
      printf( "  Prox:  %p\n", p->next );
    }
  printf( "%s", bottom_delimiter );
}


 

/**********************************************************/
/************************ DOrdList ************************/

/*******************/
/*** Constructor ***/

template<class T, class K>
DOrdList<T,K>::DOrdList( char order )
{
  f_size  = 0;
  f_order = order;
  f_head  = f_tail = f_node = (TNode *)0;
}


/******************/
/*** Destructor ***/

template<class T, class K>
DOrdList<T,K>::~DOrdList()
{
  Clear();
}


/************/
/*** Goto ***/

template<class T, class K>
int
DOrdList<T,K>::Goto( int pos )
{
  if ( pos >= f_size )
    return( 0 );

  f_node = f_head;
  for ( int i = 0; i < pos; i++ )
    f_node = f_node->next;

  return( 1 );
}



/************/
/*** Find ***/

template<class T, class K>
int
DOrdList<T,K>::Find( T elem )
{
  for ( f_node = f_head; f_node; f_node = f_node->next )
    if ( f_node->elem == elem )
      return( 1 );

  return( 0 );
}



/***************/
/*** Set Key ***/

template<class T, class K>
int
DOrdList<T,K>::SetKey( K key )
{
  if ( !f_node )
    return( 0 );

  Insert( Delete(), key );
  return( 1 );
}



/***************/
/*** Insert  ***/

template<class T, class K>
void
DOrdList<T,K>::Insert( T elem, K key )
{
  TNode *new_node = new TNode( elem, key, 0, 0 );

  // If first node.
  if ( !f_head )
    f_head = f_tail = f_node = new_node;

  // Search in the list to insert in the correct position.
  else
    {
      // If beginning or end of list, go back to the beginning.
      if ( !f_node )
	f_node = f_head;

      // If position of new node has passed, go back.
      if ( (f_order*f_node->key) > (f_order*key) )
	{
	  while( f_node->prev && ((f_order*f_node->prev->key) > (f_order*key)) )
	    f_node = f_node->prev;

	  if ( f_node->prev )
	    f_node->prev->next = new_node;
	  else
	    f_head = new_node;
	  new_node->next = f_node;
	  new_node->prev = f_node->prev;
	  f_node->prev = new_node;
	}

      // If position of the new node is further away, continue utill find it.
      else
	{
	  while( f_node->next && ((f_order*f_node->next->key) < (f_order*key)) )
	    f_node = f_node->next;

	  if ( f_node->next )
	    f_node->next->prev = new_node;
	  else
	    f_tail = new_node;
	  new_node->next = f_node->next;
	  new_node->prev = f_node;
	  f_node->next = new_node;
	}

      f_node = new_node;
    }

  f_size++;
}


/**************/
/*** Delete ***/

template<class T, class K>
T
DOrdList<T,K>::Delete()
{
  if ( !f_node )
    return( T(0) );

  T ret = f_node->elem;

  if ( f_node->next )
    f_node->next->prev = f_node->prev;

  if ( f_node->prev )
    f_node->prev->next = f_node->next;

  if ( f_head == f_node )
    f_head = f_node->next;

  if ( f_tail == f_node )
    f_tail = f_node->prev;

  TNode *old = f_node;
  f_node = f_node->next;

  delete( old );
  f_size--;

  return( ret );
}


/**************/
/*** Delete ***/

template<class T, class K>
T
DOrdList<T,K>::Delete( T x )
{
  T elem;
  for ( Head(); elem = Get(); Next() )
    if ( elem == x )
      return( Delete() );

  return( T(0) );
}



/*************/
/*** Clear ***/

template<class T, class K>
void
DOrdList<T,K>::Clear()
{
  f_size = 0;
  f_node = f_head;
  while ( f_node )
    {
      f_head = f_head->next;
      delete( f_node );
      f_node = f_head;
    }

  f_head = f_tail = f_node = 0;
}



/*************/
/*** Print ***/

template<class T, class K>
void
DOrdList<T,K>::Print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "Order:   %s.\n", f_order > 0 ? "asc" : "desc" );
  printf( "Size: %d.\n", f_size );
  
  TNode *p = f_head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == f_node ? '>' : ' ' );
      printf( " Key: %f\n", (float)p->key );
      printf( "  Elem:  %p\n", p->elem );
      printf( "  Prev:  %p\n", p->prev );
      printf( "  Prox:  %p\n", p->next );
    }
  printf( "%s", bottom_delimiter );
}

