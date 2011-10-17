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

#pragma once

namespace SpatialIndex
{
  namespace RTree
  {
    class Index : public Node
    {
      public:
        virtual ~Index();

      protected:
        Index( RTree* pTree, id_type id, uint32_t level );

        virtual NodePtr chooseSubtree( const Region& mbr, uint32_t level, std::stack<id_type>& pathBuffer );
        virtual NodePtr findLeaf( const Region& mbr, id_type id, std::stack<id_type>& pathBuffer );

        virtual void split( uint32_t dataLength, byte* pData, Region& mbr, id_type id, NodePtr& left, NodePtr& right );

        uint32_t findLeastEnlargement( const Region& ) const;
        uint32_t findLeastOverlap( const Region& ) const;

        void adjustTree( Node*, std::stack<id_type>& );
        void adjustTree( Node*, Node*, std::stack<id_type>&, byte* overflowTable );

        class OverlapEntry
        {
          public:
            uint32_t m_index;
            double m_enlargement;
            RegionPtr m_original;
            RegionPtr m_combined;
            double m_oa;
            double m_ca;

            static int compareEntries( const void* pv1, const void* pv2 )
            {
              OverlapEntry* pe1 = * ( OverlapEntry** ) pv1;
              OverlapEntry* pe2 = * ( OverlapEntry** ) pv2;

              if ( pe1->m_enlargement < pe2->m_enlargement ) return -1;
              if ( pe1->m_enlargement > pe2->m_enlargement ) return 1;
              return 0;
            }
        }; // OverlapEntry

        friend class RTree;
        friend class Node;
        friend class BulkLoader;
    }; // Index
  }
}
