// Spatial Index Library
//
// Copyright (C) 2002 Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#ifndef __spatialindex_rtree_pointer_pool_node_h
#define __spatialindex_rtree_pointer_pool_node_h

#include "Node.h"

namespace Tools
{
  template<> class PointerPool<RTree::Node>
  {
    public:
      explicit PointerPool( uint32_t capacity ) : m_capacity( capacity )
      {
#ifndef NDEBUG
        m_hits = 0;
        m_misses = 0;
        m_pointerCount = 0;
#endif
      }

      ~PointerPool()
      {
        assert( m_pool.size() <= m_capacity );

        while ( ! m_pool.empty() )
        {
          RTree::Node* x = m_pool.top(); m_pool.pop();
#ifndef NDEBUG
          --m_pointerCount;
#endif
          delete x;
        }

#ifndef NDEBUG
        std::cerr << "Lost pointers: " << m_pointerCount << std::endl;
#endif
      }

      PoolPointer<RTree::Node> acquire()
      {
        if ( ! m_pool.empty() )
        {
          RTree::Node* p = m_pool.top(); m_pool.pop();
#ifndef NDEBUG
          ++m_hits;
#endif

          return PoolPointer<RTree::Node>( p, this );
        }
#ifndef NDEBUG
        else
        {
          // fixme: well sort of...
          ++m_pointerCount;
          ++m_misses;
        }
#endif

        return PoolPointer<RTree::Node>();
      }

      void release( RTree::Node* p )
      {
        if ( p != 0 )
        {
          if ( m_pool.size() < m_capacity )
          {
            if ( p->m_pData != 0 )
            {
              for ( uint32_t cChild = 0; cChild < p->m_children; ++cChild )
              {
                // there is no need to set the pointer to zero, after deleting it,
                // since it will be redeleted only if it is actually initialized again,
                // a fact that will be depicted by variable m_children.
                if ( p->m_pData[cChild] != 0 ) delete[] p->m_pData[cChild];
              }
            }

            p->m_level = 0;
            p->m_identifier = -1;
            p->m_children = 0;
            p->m_totalDataLength = 0;

            m_pool.push( p );
          }
          else
          {
#ifndef NDEBUG
            --m_pointerCount;
#endif
            delete p;
          }

          assert( m_pool.size() <= m_capacity );
        }
      }

      uint32_t getCapacity() const { return m_capacity; }
      void setCapacity( uint32_t c )
      {
        m_capacity = c;
      }

    protected:
      uint32_t m_capacity;
      std::stack<RTree::Node*> m_pool;

#ifndef NDEBUG
    public:
      uint64_t m_hits;
      uint64_t m_misses;
      uint64_t m_pointerCount;
#endif
  };
}

#endif /* __spatialindex_rtree_pointer_pool_node_h */
