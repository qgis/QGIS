/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include <iostream>

namespace pal
{

  class Layer;

  /**
   * \brief Generic cell for LinkedList
   * \param Type Generic class
   */
  template <class Type>
  class Cell
  {
    public:
      /** \brief Create a new Cell
       * \param item The object to store
       */
      Cell( Type item ) : item( item ), next( NULL ) {};
      Type item;
      Cell *next;
  };

  /**
   * \brief Generic queue
   * \param Type generic class
   * \todo thread safe
   */
  template <class Type>
  class LinkedList
  {

      friend class Layer;

    private:
      Cell<Type> *first;
      Cell<Type> *last;
      int s;

      bool ( *compare )( Type a, Type b );

    public:

      /**
       * \brief Create a new empty list
       */
      LinkedList( bool ( *compare )( Type a, Type b ) ) : first( NULL ), last( NULL ), s( 0 ), compare( compare ) {};
      /**
       * \brief delete the list
       */
      ~LinkedList();

      //void setCompareMethod (bool (*compare)(Type a, Type b));

      /**
       * \brief is item into list ?
       * \return true or false
       */
      bool isIn( Type item );

      /**
       * \brief Insert an item at the end
       * \param item The item to insert
       */
      void push_back( Type item );

      /**
       * \brief Insert an item in first position
       * \param item The item to insert
       */
      void push_front( Type item );

      /**
       * \brief extract and return the last item
       * \return Extracted item
       */
      Type pop_front();

      /**
       * \brief get the list size
       * \return the number of item into the queue
       */
      int size();

      /**
       * \brief get the first cell
       * \return the first elem, as iterator
       */
      Cell<Type> *getFirst();

      Cell<Type> *search( Type item );

      void remove( Type item );

      void clean();
  };



  template <class Type>
  LinkedList<Type>::~LinkedList()
  {
    Cell<Type> *cur;
    Cell<Type> *it;

    cur = first;

    while ( cur )
    {
      it = cur;
      cur = cur->next;
      delete it;
    }
  }

  template <class Type>void LinkedList<Type>::push_back( Type item )
  {
    if ( s == 0 )
    {
      first = new Cell<Type> ( item );
      last = first;
    }
    else
    {
      last->next = new Cell<Type> ( item );
      last = last->next;
    }
    s++;
  }

  template <class Type>void LinkedList<Type>::push_front( Type item )
  {
    Cell<Type> *n = new Cell<Type> ( item );
    n->next = first;
    first = n;
    s++;
  }


  template<class Type>Type  LinkedList<Type>::pop_front()
  {
    Type item;
    Cell<Type> *it;

    if ( first )
    {
      it = first;
      item = it->item;
      first = first->next;
      delete it;
      s--;
    }
    else
      item = Type( 0 );

    return item;
  }

  template <class Type>bool LinkedList<Type>::isIn( Type item )
  {
    Cell<Type> *cur = first;

    while ( cur )
    {
      if ( item == cur->item )
        return true;
      cur = cur->next;
    }

    return false;
  }

  template <class Type>int LinkedList<Type>::size()
  {
    return s;
  }


  template <class Type>Cell<Type> *LinkedList<Type>::getFirst()
  {
    return first;
  }


  template <class Type> void LinkedList<Type>::remove( Type item )
  {
    Cell<Type> *p = first;
    Cell<Type> *q;

    if ( first )
    {
      if ( compare( item, p->item ) )
      {
        first = p->next;
        s--;
        delete p;
        return;
      }
      while ( p->next && !compare( p->next->item, item ) ) {p = p->next;}

      if ( p->next )
      {
        q = p->next;
        p->next = q->next;
        s--;
        delete q;
        if ( !p->next )
          last = p;
      }
    }
  }

  template <class Type> Cell<Type> * LinkedList<Type>::search( Type item )
  {
    Cell<Type> *p = first;

    while ( p && !compare( p->item, item ) )
    {
      p = p->next;
    }

    return p;
  }


  /*template <class Type> void LinkedList<Type>::setCompareMethod (bool (*compare)(Type a, Type b)){
     this->compare = compare;
  }*/

  template <class Type> void LinkedList<Type>::clean()
  {
    Cell<Type> *it = first;
    Cell<Type> *cur;

    while ( it )
    {
      cur = it;
      it = it->next;
      delete cur;
    }
    first = last = NULL;
    s = 0;
  }
} // end namespace
#endif
