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
  _size = 0;
  _head = _node = (ListNode<T> *)0;
}



/******************/
/*** Destructor ***/

template<class T>
List<T>::~List()
{
  clear();
}



/**************/
/*** append ***/

template<class T>
void
List<T>::append( T elem )
{
  ListNode<T> *new_node = new ListNode<T>( elem, 0 );

  if ( !_head )
    _head = _node = new_node;
  else
    {
      ListNode<T> *last = _head;
      while ( last->next )
	last = last->next;
      last->next = new_node;
    }

  _size++;
}



/***************/
/*** insert  ***/

template<class T>
void
List<T>::insert( T elem )
{
  ListNode<T> *new_node = new ListNode<T>( elem, 0 );

  // If first node.
  if ( !_head )
    _head = _node = new_node;

  // If current node is out of the list, insert at the beginning.
  else if ( !_node )
    {
      new_node->next = _head;
      _head = _node = new_node;
    }

  else
    {
      new_node->next = _node->next;
      _node->next = new_node;
    }

  _size++;
}



/********************/
/*** insert First ***/

template<class T>
void
List<T>::insertFirst( T elem )
{
   _node = _head = new ListNode<T>( elem, _head );
   _size++;
}



/*************/
/*** clear ***/

template<class T>
void
List<T>::clear()
{
  _size = 0;
  _node = _head;
  while ( _node )
    {
      _head = _head->next;
      delete( _node );
      _node = _head;
    }
}


 


/*******************************************************/
/************************ DList ************************/

/*******************/
/*** Constructor ***/

template<class T>
DList<T>::DList()
{
  _size = 0;
  _head = _tail = _node = 0;
}


/******************/
/*** Destructor ***/

template<class T>
DList<T>::~DList()
{
  clear();
}


/***********/
/*** get ***/

template<class T>
int
DList<T>::get( T *pe )
{
  if ( _node )
    {
      *pe = _node->elem;
      return( 1 );
    }
  else
    {
      *pe = T( 0 );
      return( 0 );
    }
}


/*********************/
/*** insert Before ***/

template<class T>
void
DList<T>::insertBefore( T elem )
{
  TNode *new_node = new TNode( elem, 0, 0 );

  // If first node.
  if ( !_head )
    _head = _tail = _node = new_node;

  // If end of list.
  else if ( !_node )
    {
      _tail->next = new_node;
      new_node->prev = _tail;
      _tail = new_node;
    }

  // Insert before current node.
  else
    {
      if ( _node->prev )
	_node->prev->next = new_node;
      else
	_head = new_node;
      new_node->next = _node;
      new_node->prev = _node->prev;
      _node->prev   = new_node;
    }

  _size++;
}


/********************/
/*** insert After ***/

template<class T>
void
DList<T>::insertAfter( T elem )
{
  TNode *new_node = new TNode( elem, 0, 0 );

  // If first node.
  if ( !_head )
    _head = _tail = _node = new_node;

  // If end of list.
  else if ( !_node )
    {
      _tail->next = new_node;
      new_node->prev = _tail;
      _tail = new_node;
    }

  // Insert after current node.
  else
    {
      if ( _node->next )
	_node->next->prev = new_node;
      else
	_tail = new_node;
      new_node->prev = _node;
      new_node->next = _node->next;
      _node->next   = new_node;
    }

  _size++;
}


/********************/
/*** insert First ***/

template<class T>
void
DList<T>::insertFirst( T elem )
{
  TNode *new_node = new TNode( elem, 0, _head );

  // If first node.
  if ( !_head )
    _tail = _node = new_node;
  else
    _head->prev = new_node;
  
  _head = new_node;
  _size++;
}


/*******************/
/*** insert Last ***/

template<class T>
void
DList<T>::insertLast( T elem )
{
  TNode *new_node = new TNode( elem, _tail, 0 );

  // If first node.
  if ( !_tail )
    _head = _node = new_node;
  else
    _tail->next = new_node;
  
  _tail = new_node;
  _size++;
}


/****************/
/*** exchange ***/

template<class T>
void
DList<T>::exchange( DList<T> &l )
{
  TNode *aux;
  
  aux      = _head;
  _head   = l._head;
  l._head = aux;
    
  aux      = _tail;
  _tail   = l._tail;
  l._tail = aux;
    
  aux      = _node;
  _node   = l._node;
  l._node = aux;

  int size = _size;
  _size   = l._size;
  l._size = size;
}


/**************/
/*** remove ***/

template<class T>
T
DList<T>::remove()
{
  if ( !_node )
    return( T(0) );

  T ret = _node->elem;

  if ( _node->next )
    _node->next->prev = _node->prev;

  if ( _node->prev )
    _node->prev->next = _node->next;

  if ( _head == _node )
    _head = _node->next;

  if ( _tail == _node )
    _tail = _node->prev;

  TNode *old = _node;
  _node = _node->next;

  delete( old );
  _size--;

  return( ret );
}


/*************/
/*** clear ***/

template<class T>
void
DList<T>::clear()
{
  _size = 0;
  _node = _head;
  while ( _node )
    {
      _head = _head->next;
      delete( _node );
      _node = _head;
    }

  _head = _tail = _node = 0;
}



/*************/
/*** print ***/

template<class T>
void
DList<T>::print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "size: %d.\n", _size );
  
  TNode *p = _head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == _node ? '>' : ' ' );
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
  _size  = 0;
  _order = order;
  _prev  = &_head;
  _head  = _node = 0;
}



/******************/
/*** Destructor ***/

template<class T, class K>
OrdList<T,K>::~OrdList()
{
  clear();
}



/************/
/*** next ***/

template<class T, class K>
void
OrdList<T,K>::next()
{
  if ( _node )
    {
      _prev = &(_node->next);
      _node = _node->next;
    }
  
}



/************/
/*** Goto ***/

template<class T, class K>
int
OrdList<T,K>::Goto( int index )
{
  head();
  if ( index >= _size )
    return( 0 );

  for ( int i = 0; i < index; i++ )
    next();

  return( index );
}



/***************/
/*** insert  ***/

template<class T, class K>
void
OrdList<T,K>::insert( T elem, K key )
{
  OrdListNode<T,K> *new_node = new OrdListNode<T,K>( elem, key, 0 );

  // If first node.
  if ( !_head )
    {
      _prev = &_head;
      _head = _node = new_node;
    }

  // Search in the list to insert in the correct position.
  else
    {
      // Go back to the beginning of the list if the position
      // of the new node has passed.
      if ( !_node || ((_order*_node->key) >= (_order*key)) )
	{
	  _prev = &_head;
	  _node = _head;
	}

      // Look after the correct position.
      while ( _node && ((_order*_node->key) < (_order*key)) )
	{
	  _prev = &(_node->next);
	  _node = _node->next;
	}

      // Insert new node.
      new_node->next = _node;
      *_prev = new_node;
      _prev  = &(new_node->next);
    }

  _size++;
}



/********************/
/*** insert After ***/

template<class T, class K>
void
OrdList<T,K>::insertAfter( T elem, K key )
{
  OrdListNode<T,K> *new_node = new OrdListNode<T,K>( elem, key, 0 );

  // If first node.
  if ( !_head )
    {
      _prev = &_head;
      _head = _node = new_node;
    }

  // Search in the list to insert in the correct position.
  else
    {
      // Go back to the beginning of the list if the position
      // of the new node has passed.
      if ( !_node || ((_order*_node->key) > (_order*key)) )
	{
	  _prev = &_head;
	  _node = _head;
	}

      //  Look after the correct position.
      while ( _node && ((_order*_node->key) <= (_order*key)) )
	{
	  _prev = &(_node->next);
	  _node = _node->next;
	}

      // Insert new node.
      new_node->next = _node;
      *_prev = new_node;
      _prev  = &(new_node->next);
    }

  _size++;
}



/**************/
/*** remove ***/

template<class T, class K>
T
OrdList<T,K>::remove()
{
  if ( !_node )
    return( T(0) );

  T ret = _node->elem;

  *_prev  = _node->next;
  delete( _node );
  _node = *_prev;
  _size--;

  return( ret );
}



/**************/
/*** remove ***/

template<class T, class K>
T
OrdList<T,K>::remove( T x )
{
  T elem;
  for ( head(); elem = get(); next() )
    if ( elem == x )
      return( remove() );

  return( T(0) );
}



/*************/
/*** clear ***/

template<class T, class K>
void
OrdList<T,K>::clear()
{
  _size = 0;
  _node = _head;
  while ( _node )
    {
      _head = _head->next;
      delete( _node );
      _node = _head;
    }

  _prev = &_head;
  _head = _node = (OrdListNode<T,K> *)0;
}



/*************/
/*** print ***/

template<class T, class K>
void
OrdList<T,K>::print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "Order:   %s.\n", _order > 0 ? "asc" : "desc" );
  printf( "size: %d.\n", _size );
  
  OrdListNode<T,K> *p = _head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == _node ? '>' : ' ' );
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
  _size  = 0;
  _order = order;
  _head  = _tail = _node = (TNode *)0;
}


/******************/
/*** Destructor ***/

template<class T, class K>
DOrdList<T,K>::~DOrdList()
{
  clear();
}


/************/
/*** Goto ***/

template<class T, class K>
int
DOrdList<T,K>::Goto( int pos )
{
  if ( pos >= _size )
    return( 0 );

  _node = _head;
  for ( int i = 0; i < pos; i++ )
    _node = _node->next;

  return( 1 );
}



/************/
/*** Find ***/

template<class T, class K>
int
DOrdList<T,K>::Find( T elem )
{
  for ( _node = _head; _node; _node = _node->next )
    if ( _node->elem == elem )
      return( 1 );

  return( 0 );
}



/***************/
/*** Set Key ***/

template<class T, class K>
int
DOrdList<T,K>::SetKey( K key )
{
  if ( !_node )
    return( 0 );

  insert( remove(), key );
  return( 1 );
}



/***************/
/*** insert  ***/

template<class T, class K>
void
DOrdList<T,K>::insert( T elem, K key )
{
  TNode *new_node = new TNode( elem, key, 0, 0 );

  // If first node.
  if ( !_head )
    _head = _tail = _node = new_node;

  // Search in the list to insert in the correct position.
  else
    {
      // If beginning or end of list, go back to the beginning.
      if ( !_node )
	_node = _head;

      // If position of new node has passed, go back.
      if ( (_order*_node->key) > (_order*key) )
	{
	  while( _node->prev && ((_order*_node->prev->key) > (_order*key)) )
	    _node = _node->prev;

	  if ( _node->prev )
	    _node->prev->next = new_node;
	  else
	    _head = new_node;
	  new_node->next = _node;
	  new_node->prev = _node->prev;
	  _node->prev = new_node;
	}

      // If position of the new node is further away, continue utill find it.
      else
	{
	  while( _node->next && ((_order*_node->next->key) < (_order*key)) )
	    _node = _node->next;

	  if ( _node->next )
	    _node->next->prev = new_node;
	  else
	    _tail = new_node;
	  new_node->next = _node->next;
	  new_node->prev = _node;
	  _node->next = new_node;
	}

      _node = new_node;
    }

  _size++;
}


/**************/
/*** remove ***/

template<class T, class K>
T
DOrdList<T,K>::remove()
{
  if ( !_node )
    return( T(0) );

  T ret = _node->elem;

  if ( _node->next )
    _node->next->prev = _node->prev;

  if ( _node->prev )
    _node->prev->next = _node->next;

  if ( _head == _node )
    _head = _node->next;

  if ( _tail == _node )
    _tail = _node->prev;

  TNode *old = _node;
  _node = _node->next;

  delete( old );
  _size--;

  return( ret );
}


/**************/
/*** remove ***/

template<class T, class K>
T
DOrdList<T,K>::remove( T x )
{
  T elem;
  for ( head(); elem = get(); next() )
    if ( elem == x )
      return( remove() );

  return( T(0) );
}



/*************/
/*** clear ***/

template<class T, class K>
void
DOrdList<T,K>::clear()
{
  _size = 0;
  _node = _head;
  while ( _node )
    {
      _head = _head->next;
      delete( _node );
      _node = _head;
    }

  _head = _tail = _node = 0;
}



/*************/
/*** print ***/

template<class T, class K>
void
DOrdList<T,K>::print( char *top_delimiter, char *bottom_delimiter )
{
  printf( "%s", top_delimiter );
  printf( "Order:   %s.\n", _order > 0 ? "asc" : "desc" );
  printf( "size: %d.\n", _size );
  
  TNode *p = _head;
  for ( int i = 0; p; i++, p = p->next )
    {
      printf( "Node %d [%p]\n", i, p );
      printf( "%c", p == _node ? '>' : ' ' );
      printf( " Key: %f\n", (float)p->key );
      printf( "  Elem:  %p\n", p->elem );
      printf( "  Prev:  %p\n", p->prev );
      printf( "  Prox:  %p\n", p->next );
    }
  printf( "%s", bottom_delimiter );
}

