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

  void Head()       { f_node = f_head; }
  void Next()       { if ( f_node ) f_node = f_node->next; }

  T Get()           { return( f_node ? f_node->elem : T(0) ); }
  T First()         { return( f_head ? f_head->elem : T(0) ); }
  
  void Append( T );
  void Insert( T );        ///< Insert after the current node.
  void InsertFirst( T );
  void InsertLast( T e )   { Append( e ); }
  void Clear();
  int  Size()              { return( f_size ); }
  int  Length()            { return( f_size ); }
  

private:

  int         f_size;
  ListNode<T> *f_head;
  ListNode<T> *f_node;
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

  void Head()       { f_node = f_head; }
  void Tail()       { f_node = f_tail; }
  void Next()       { if ( f_node ) f_node = f_node->next; }
  void Prev()       { if ( f_node ) f_node = f_node->prev; }

  int Get( T *pe ); ///< Return 0 and *pe=0, if reached the end of list.

  T Get()           { return( f_node ? f_node->elem : T(0) ); }
  T First()         { return( f_head ? f_head->elem : T(0) ); }
  T Last()          { return( f_tail ? f_tail->elem : T(0) ); }

  void Change( T e ) { if ( f_node ) f_node->elem = e; }

  /** Current node is not modified. */
  void Append( T e )       { InsertLast( e ); }
  void Insert( T e )       { InsertAfter( e ); }
  void InsertBefore( T );
  void InsertAfter( T );
  void InsertFirst( T );
  void InsertLast( T );

  T Delete();    ///< Delete current element from list, returning it.
  T DeleteFirst()    { Head(); return( Delete() ); }
  T DeleteLast()     { Tail(); return( Delete() ); }

  void Clear();
  int  Size()              { return( f_size ); }
  int  Length()            { return( f_size ); }

  void Exchange( DList<T> &lst ); ///< Exchange elements between this list and 'lst'.

  /** \warning To use this method you have to be sure that the current 
   *  node, when "GetPos" is called, is also present when 
   *  calling "SetPos".
   */
  void *GetPos()               { return( (void *) f_node ); }
  /** \warning To use this method you have to be sure that the current 
   *  node, when "GetPos" is called, is also present when 
   *  calling "SetPos".
   */
  void  SetPos( void *pos )    { f_node = (DListNode<T> *) pos; }

  void Print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  int    f_size;
  TNode *f_head;
  TNode *f_tail;
  TNode *f_node;
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

  void Head()       { f_node = f_head; f_prev = &f_head; }
  void Next();
  int  Goto( int index );

  T First()         { return( f_head ? f_head->elem : T(0) ); }
  K FirstKey()      { return( f_head ? f_head->key  : K(0) ); }
  T Get()           { return( f_node ? f_node->elem : T(0) ); }
  K GetKey()        { return( f_node ? f_node->key  : K(0) ); }
  
  void Insert( T, K );        ///< Insert before equal nodes.
  void InsertAfter( T, K );   ///< Insert after equal nodes.

  T Delete();      ///< Delete current element from list, returning it.
  T Delete( T x ); ///< Delete 'x' (current position will point to the following one).

  void Clear();
  int  Size()   { return( f_size ); }
  int  Length() { return( f_size ); }
  
  void Print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  char f_order;
  int  f_size;

  OrdListNode<T,K>  *f_head;
  OrdListNode<T,K>  *f_node;
  OrdListNode<T,K> **f_prev;
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

  void Head()       { f_node = f_head; }
  void Tail()       { f_node = f_tail; }
  void Next()       { if ( f_node ) f_node = f_node->next; }
  void Prev()       { if ( f_node ) f_node = f_node->prev; }

  T First()         { return( f_head ? f_head->elem : T(0) ); }
  K FirstKey()      { return( f_head ? f_head->key  : K(0) ); }
  T Get()           { return( f_node ? f_node->elem : T(0) ); }
  K GetKey()        { return( f_node ? f_node->key  : K(0) ); }

  int Goto( int pos );
  int Find( T e );     ///< Return 1 if 'e' is found. (current = e)

  int  SetKey( K );    ///< Change key of current element, reordering it.
  void Insert( T, K ); ///< Insert element using apropriate ordering (current=new).
  T    Delete();       ///< Delete current element from list, returning it.
  T    Delete( T x );  ///< Delete 'x' (current position will point to the following one).

  T DeleteFirst()    { Head(); return( Delete() ); }
  T DeleteLast()     { Tail(); return( Delete() ); }

  void Clear();
  int  Size()   { return( f_size ); }
  int  Length() { return( f_size ); }
  
  void Print( char *top_delimiter="", char *bottom_delimiter="" );


private:

  char   f_order;
  int    f_size;
  TNode *f_head;
  TNode *f_tail;
  TNode *f_node;
};



#endif

