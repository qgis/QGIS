/**
 * Declaration of linked list templates. 
 *
 * Note: Any list element needs a constructor that accepts 0
 *       (zero) as input, in other words, ElemType(0) must
 *       exist to represent null elements.
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


#ifndef _LISTHH_
#define _LISTHH_


/***************************************************************/
/************************* Linked List *************************/

/***********************/
template<class ElemType >
class ListNode
{
public:
  ListNode( ElemType e, ListNode *n )
    { elem = e; next = n; }

  ElemType elem;
  ListNode *next;
};


/**
 * Linked List
 */
template<class T>
class List
{
public:

  List();
  ~List();

  void head()       { _node = _head; }
  void next()       { if ( _node ) _node = _node->next; }

  T get()           { return( _node ? _node->elem : T(0) ); }
  T first()         { return( _head ? _head->elem : T(0) ); }
  
  void append( T );
  void insert( T );        ///< Insert after the current node.
  void insertFirst( T );
  void insertLast( T e )   { append( e ); }
  void clear();
  int  size()              { return( _size ); }
  int  length()            { return( _size ); }
  

private:

  int         _size;
  ListNode<T> *_head;
  ListNode<T> *_node;
};




//************************************************************/
/********************* Double Linked List ********************/

/***********************/
template<class ElemType >
class DListNode
{
public:
  DListNode( ElemType e, DListNode *p, DListNode *n )
    { elem = e; prev = p; next = n; }

  ElemType   elem;
  DListNode *prev;
  DListNode *next;
};


/** 
 * Double Linked List
 */
template<class T>
class DList
{
  typedef DListNode<T> TNode;


public:

  DList();
  ~DList();

  void head()       { _node = _head; }
  void tail()       { _node = _tail; }
  void next()       { if ( _node ) _node = _node->next; }
  void prev()       { if ( _node ) _node = _node->prev; }

  int get( T *pe ); ///< Return 0 and *pe=0, if reached the end of list.

  T get()           { return( _node ? _node->elem : T(0) ); }
  T first()         { return( _head ? _head->elem : T(0) ); }
  T last()          { return( _tail ? _tail->elem : T(0) ); }

  void change( T e ) { if ( _node ) _node->elem = e; }

  /** Current node is not modified. */
  void append( T e )       { insertLast( e ); }
  void insert( T e )       { insertAfter( e ); }
  void insertBefore( T );
  void insertAfter( T );
  void insertFirst( T );
  void insertLast( T );

  T remove();    ///< Remove current element from list, returning it.
  T removeFirst()    { head(); return( remove() ); }
  T removeLast()     { tail(); return( remove() ); }

  void clear();
  int  size()              { return( _size ); }
  int  length()            { return( _size ); }

  void exchange( DList<T> &lst ); ///< Exchange elements between this list and 'lst'.

  /** \warning To use this method you have to be sure that the current 
   *  node, when "getPos" is called, is also present when 
   *  calling "setPos".
   */
  void *getPos()               { return( (void *) _node ); }
  /** \warning To use this method you have to be sure that the current 
   *  node, when "getPos" is called, is also present when 
   *  calling "setPos".
   */
  void  setPos( void *pos )    { _node = (DListNode<T> *) pos; }

  void print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  int    _size;
  TNode *_head;
  TNode *_tail;
  TNode *_node;
};




/****************************************************************/
/********************** Ordered Linked List *********************/

/***************************************/
template<class ElemType, class KeyType >
class OrdListNode
{
public:
  OrdListNode( ElemType e, KeyType k, OrdListNode *n )
    { elem = e; key = k; next = n; }

  ElemType  elem;
  KeyType   key;
  OrdListNode *next;
};


/** 
 * Ordered Linked List
 *
 * @param T Type of element to be stored.
 * @param K Type of key to be used.
 */
template<class T, class K>
class OrdList
{
public:

 /** Constructor
  *
  *  - order > 0  -> ascending.
  *  - order < 0  -> descending.
  *  - order = 0  -> unordered (insert at current position).
  */
  OrdList( char order );
  ~OrdList();

  void head()       { _node = _head; _prev = &_head; }
  void next();
  int  Goto( int index );

  T first()         { return( _head ? _head->elem : T(0) ); }
  K firstKey()      { return( _head ? _head->key  : K(0) ); }
  T get()           { return( _node ? _node->elem : T(0) ); }
  K getKey()        { return( _node ? _node->key  : K(0) ); }
  
  void insert( T, K );        ///< Insert before equal nodes.
  void insertAfter( T, K );   ///< Insert after equal nodes.

  T remove();      ///< Remove current element from list, returning it.
  T remove( T x ); ///< Remove 'x' (current position will point to the following one).

  void clear();
  int  size()   { return( _size ); }
  int  length() { return( _size ); }
  
  void print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  char _order;
  int  _size;

  OrdListNode<T,K>  *_head;
  OrdListNode<T,K>  *_node;
  OrdListNode<T,K> **_prev;
};



/******************************************************************/
/******************* Ordered Double Linked List *******************/

/**************************************/
template<class ElemType, class KeyType>
class DOrdListNode
{
public:
  DOrdListNode( ElemType e, KeyType k, DOrdListNode *p,
		DOrdListNode *n )
    { elem = e; key = k; prev = p; next = n; }

  ElemType     elem;
  KeyType      key;
  DOrdListNode *prev;
  DOrdListNode *next;
};


/** 
 * Ordered Double Linked List
 *
 * @param T Type of element to be stored.
 * @param K Type of key to be used.
 */
template<class T, class K>
class DOrdList
{
  typedef DOrdListNode<T,K> TNode;

public:

 /** Constructor
  *
  *  - order > 0  -> ascending.
  *  - order < 0  -> descending.
  *  - order = 0  -> unordered (insert at current position).
  */
  DOrdList( char order );
  ~DOrdList();

  void head()       { _node = _head; }
  void tail()       { _node = _tail; }
  void next()       { if ( _node ) _node = _node->next; }
  void prev()       { if ( _node ) _node = _node->prev; }

  T first()         { return( _head ? _head->elem : T(0) ); }
  K firstKey()      { return( _head ? _head->key  : K(0) ); }
  T get()           { return( _node ? _node->elem : T(0) ); }
  K getKey()        { return( _node ? _node->key  : K(0) ); }

  int Goto( int pos );
  int Find( T e );     ///< Return 1 if 'e' is found. (current = e)

  int  SetKey( K );    ///< Change key of current element, reordering it.
  void insert( T, K ); ///< Insert element using apropriate ordering (current=new).
  T    remove();       ///< Remove current element from list, returning it.
  T    remove( T x );  ///< Remove 'x' (current position will point to the following one).

  T removeFirst()    { head(); return( remove() ); }
  T removeLast()     { tail(); return( remove() ); }

  void clear();
  int  size()   { return( _size ); }
  int  length() { return( _size ); }
  
  void print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  char   _order;
  int    _size;
  TNode *_head;
  TNode *_tail;
  TNode *_node;
};



#endif

