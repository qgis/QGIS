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

#include <cstdio>

#include "internalexception.h"
#include "priorityqueue.h"

namespace pal
{

  bool smaller( double l, double r )
  {
    return l > r;
  }

  bool bigger( double l, double r )
  {
    return l < r;
  }

// O (size log size)
  PriorityQueue::PriorityQueue( int n, int maxId, bool min ) : size( 0 ), maxsize( n ), maxId( maxId )
  {
    heap = new int[maxsize];
    p = new double[maxsize];
    pos = new int[maxId+1];

    int i;

    for ( i = 0; i <= maxId; i++ )
      pos[i] = -1;


    if ( min )
      greater = smaller;
    else
      greater = bigger;
  }

  PriorityQueue::~PriorityQueue()
  {
    delete[] heap;
    delete[] p;
    delete[] pos;
  }

  int PriorityQueue::getSize()
  {
    return size;
  }

// O(log size)
  int PriorityQueue::getBest()
  {
    if ( size <= 0 )
      throw InternalException::Empty();

    int return_value = heap[0];

    //std::cerr << "getBest" << std::endl;
    //std::cerr << "    key: " << return_value << std::endl;
    //std::cerr << "   Size: " << size << std::endl;

    size--;


    pos[heap[0]] = -1;

    if ( size > 0 )
    {
      pos[heap[size]] = 0;

      heap[0] = heap[size];
      p[0] = p[size];
      downheap( 0 );
    }

    return return_value;
  }


  bool PriorityQueue::isIn( int key )
  {
    return key <= maxId && pos[key] >= 0;
  }

  int PriorityQueue::getId( int key )
  {
    return key <= maxId ? pos[key] : -1;
  }

  void PriorityQueue::insert( int key, double p )
  {
    if ( size == maxsize || key > maxId || key < 0 )
      throw InternalException::Full();

    //std::cerr << "insert" << std::endl;
    //std::cerr << "   key: " << key << std::endl;
    //std::cerr << "   size: " << size << std::endl;

    heap[size] = key;
    pos[key] = size;
    this->p[size] = p;

    size++;


    upheap( key );
  }


// O(size)
//
  void PriorityQueue::remove( int key )
  {
    if ( key < 0 || key > maxId )
      return;
    int i = pos[key];

    //std::cerr << "Remove " << key << std::endl;
    //std::cerr << "   pos[key]: " << i << std::endl;

    if ( i >= 0 )
    {

      //std::cerr << "size:" << size << std::endl;
      //std::cerr << "heap[size]:" << heap[size] << std::endl;
      //std::cerr << "pos[heap[size]]:" << pos[heap[size]] << std::endl;
      //std::cerr << "pos[key]:" << pos[key] << std::endl;

      size--;
      pos[heap[size]] = i;
      pos[key] = -1;

      heap[i] = heap[size];
      p[i]    = p[size];

      downheap( i );
    }
  }

// O (size log size)
  void PriorityQueue::sort()
  {
    int i;
    int pi = 2;
    while ( size > pi ) pi *= 2;

    i = pi / 2 - 2;

    for ( i = size - 1; i >= 0; i-- )
      downheap( i );

  }


  void PriorityQueue::upheap( int key )
  {
    int i;
    int i2;

    int tmpT;
    double tmpP;


    if ( key < 0 || key > maxId )
      return;

    i = pos[key];

    if ( i >= -1 )
    {
      while ( i > 0 )
      {
        if ( greater( p[PARENT( i )], p[i] ) )
        {
          i2 = PARENT( i );

          pos[heap[i]] = i2;
          pos[heap[i2]] = i;

          tmpT = heap[i];
          tmpP = p[i];

          heap[i] = heap[i2];
          p[i]    = p[i2];

          heap[i2] = tmpT;
          p[i2]    = tmpP;

          i = i2;
        }
        else
          break;
      }
    }
  }

// O(log n)
  void PriorityQueue::downheap( int id )
  {
    int min_child;
    int tmpT;
    double tmpP;

    for ( ;; )
    {
      if ( LEFT( id ) < size )
      {
        if ( RIGHT( id ) < size )
        {
          min_child = greater( p[RIGHT( id )], p[LEFT( id )] ) ? LEFT( id ) : RIGHT( id );
        }
        else
          min_child = LEFT( id );
      }
      else // leaf
        break;

      if ( greater( p[id], p[min_child] ) )
      {
        pos[heap[id]] = min_child;
        pos[heap[min_child]] = id;

        tmpT = heap[id];
        tmpP = p[id];

        heap[id] = heap[min_child];
        p[id]    = p[min_child];

        heap[min_child] = tmpT;
        p[min_child]    = tmpP;

        id = min_child;
      }
      else
        break;
    }
  }

  void PriorityQueue::setPriority( int key, double new_p )
  {

    if ( key < 0 || key > maxId )
      return;

    int i = pos[key];

    if ( i < 0 )
    {
      insert( key, new_p );
      return;
    }

    p[i] = new_p;;

    upheap( key );
    downheap( pos[key] );
  }


  void PriorityQueue::decreaseKey( int key )
  {

    if ( key < 0 || key > maxId )
      return;

    int i = pos[key];

    if ( i < 0 )
      return;

    p[i]--;

    upheap( key );
    downheap( pos[key] );
  }


  void PriorityQueue::print()
  {
    int i;

    fprintf( stderr, "Size: %d\nMaxSize: %d\n", size, maxsize );

    for ( i = 0; i < size; i++ )
    {
      //printf ("key: %7d  ->  index: %7d -> key: %7d   p: %7d\n", i, pos[i], heap[pos[i]], p[pos[i]]);
      fprintf( stderr, "id: %7d  ->  key: %7d -> id: %7d   p: %7f\n", i, heap[i], pos[heap[i]], p[i] );
    }
    fprintf( stderr, "\n" );

  }


  int PriorityQueue::getSizeByPos()
  {
    int i;
    int count = 0;
    for ( i = 0; i < maxsize; i++ )
    {
      if ( pos[i] >= 0 )
        count++;
    }
    return count;
  }

} // namespace

