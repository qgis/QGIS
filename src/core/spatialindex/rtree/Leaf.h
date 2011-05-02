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

#ifndef __spatialindex_rtree_leaf_h
#define __spatialindex_rtree_leaf_h

namespace SpatialIndex
{
  namespace RTree
  {
    class Leaf : public Node
    {
      public:
        virtual ~Leaf();

      private:
        Leaf( RTree* pTree, long id );

        virtual NodePtr chooseSubtree( const Region& mbr, unsigned long level, std::stack<long>& pathBuffer );
        virtual NodePtr findLeaf( const Region& mbr, long id, std::stack<long>& pathBuffer );

        virtual void split( unsigned long dataLength, byte* pData, Region& mbr, long id, NodePtr& left, NodePtr& right );

        virtual void deleteData( long id, std::stack<long>& pathBuffer );

        friend class RTree;
        friend class BulkLoader;
    }; // Leaf
  }
}

#endif /*__spatialindex_rtree_leaf_h*/
