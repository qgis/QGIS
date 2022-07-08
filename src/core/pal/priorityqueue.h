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

#ifndef PAL_PRIORITYQUEUE_H
#define PAL_PRIORITYQUEUE_H

#define SIP_NO_FILE


#include <iostream>

#define LEFT(x) (2*x+1)
#define RIGHT(x) (2*x+2)
#define PARENT(x) ((x-1)/2)


namespace pal
{

  /**
   * \ingroup core
   * \brief Custom priority queue for use in pal labeling engine.
   * \class pal::PriorityQueue
   * \note not available in Python bindings
   */
  class PriorityQueue
  {

    public:

      /**
       * \brief Create a priority queue of max size n
       * \\param n max size of the queuet
       * \\param p external vector representing the priority
       * \\param min best element has the smallest p when min is TRUE and has the biggest when min is FALSE
       */
      PriorityQueue( int n, int maxId, bool min );
      ~PriorityQueue();

      //! PriorityQueue cannot be copied.
      PriorityQueue( const PriorityQueue & ) = delete;
      //! PriorityQueue cannot be copied.
      PriorityQueue &operator=( const PriorityQueue & ) = delete;

      void print();

      int getSize() const;
      int getSizeByPos() const;

      bool isIn( int key ) const;

      int getBest(); // O(log n)

      void remove( int key );
      void insert( int key, double p );

      void sort(); // O(n log n)

      void downheap( int id );
      void upheap( int key );

      void decreaseKey( int key );
      void setPriority( int key, double new_p );


      int getId( int key ) const;
    private:

      int size;
      int maxsize;
      int maxId;
      int *heap = nullptr;
      double *p = nullptr;
      int *pos = nullptr;

      bool ( *greater )( double l, double r );
  };

} // namespace

#endif
