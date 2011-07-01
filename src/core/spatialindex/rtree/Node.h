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

#ifndef __spatialindex_rtree_node_h
#define __spatialindex_rtree_node_h

namespace SpatialIndex
{
  namespace RTree
  {
    class RTree;
    class Leaf;
    class Index;
    class Node;

    typedef Tools::PoolPointer<Node> NodePtr;

    class Node : public SpatialIndex::INode
    {
      public:
        virtual ~Node();

        //
        // Tools::IObject interface
        //
        virtual Tools::IObject* clone();

        //
        // Tools::ISerializable interface
        //
        virtual unsigned long getByteArraySize();
        virtual void loadFromByteArray( const byte* data );
        virtual void storeToByteArray( byte** data, unsigned long& len );

        //
        // SpatialIndex::IEntry interface
        //
        virtual long getIdentifier() const;
        virtual void getShape( IShape** out ) const;

        //
        // SpatialIndex::INode interface
        //
        virtual unsigned long getChildrenCount() const;
        virtual long getChildIdentifier( unsigned long index )  const;
        virtual void getChildShape( unsigned long index, IShape** out )  const;
        virtual unsigned long getLevel() const;
        virtual bool isIndex() const;
        virtual bool isLeaf() const;

      private:
        Node();
        Node( RTree* pTree, long id, unsigned long level, unsigned long capacity );

        virtual Node& operator=( const Node& );

        virtual void insertEntry( unsigned long dataLength, byte* pData, Region& mbr, long id );
        virtual void deleteEntry( unsigned long index );

        virtual bool insertData( unsigned long dataLength, byte* pData, Region& mbr, long id, std::stack<long>& pathBuffer, byte* overflowTable );
        virtual void reinsertData( unsigned long dataLength, byte* pData, Region& mbr, long id, std::vector<long>& reinsert, std::vector<long>& keep );

        virtual void rtreeSplit( unsigned long dataLength, byte* pData, Region& mbr, long id, std::vector<long>& group1, std::vector<long>& group2 );
        virtual void rstarSplit( unsigned long dataLength, byte* pData, Region& mbr, long id, std::vector<long>& group1, std::vector<long>& group2 );

        virtual void pickSeeds( unsigned long& index1, unsigned long& index2 );

        virtual void condenseTree( std::stack<NodePtr>& toReinsert, std::stack<long>& pathBuffer, NodePtr& ptrThis );

        //virtual void load(unsigned long len, byte* const data);
        //virtual void store(unsigned long& len, byte** data);

        virtual NodePtr chooseSubtree( const Region& mbr, unsigned long level, std::stack<long>& pathBuffer ) = 0;
        virtual NodePtr findLeaf( const Region& mbr, long id, std::stack<long>& pathBuffer ) = 0;

        virtual void split( unsigned long dataLength, byte* pData, Region& mbr, long id, NodePtr& left, NodePtr& right ) = 0;

        RTree* m_pTree;
        // Parent of all nodes.

        unsigned long m_level;
        // The level of the node in the tree.
        // Leaves are always at level 0.

        long m_identifier;
        // The unique ID of this node.

        unsigned long m_children;
        // The number of children pointed by this node.

        unsigned long m_capacity;
        // Specifies the node capacity.

        Region m_nodeMBR;
        // The minimum bounding region enclosing all data contained in the node.

        byte** m_pData;
        // The data stored in the node.

        RegionPtr* m_ptrMBR;
        // The corresponding data MBRs.

        long* m_pIdentifier;
        // The corresponding data identifiers.

        unsigned long* m_pDataLength;

        unsigned long m_totalDataLength;

        class RstarSplitEntry
        {
          public:
            Region* m_pRegion;
            long m_id;
            unsigned long m_sortDim;

            RstarSplitEntry( Region* pr, long id, unsigned long dimension ) :
                m_pRegion( pr ), m_id( id ), m_sortDim( dimension ) {}

            static int compareLow( const void* pv1, const void* pv2 )
            {
              RstarSplitEntry* pe1 = * ( RstarSplitEntry** ) pv1;
              RstarSplitEntry* pe2 = * ( RstarSplitEntry** ) pv2;

              Q_ASSERT( pe1->m_sortDim == pe2->m_sortDim );

              if ( pe1->m_pRegion->m_pLow[pe1->m_sortDim] < pe2->m_pRegion->m_pLow[pe2->m_sortDim] ) return -1;
              if ( pe1->m_pRegion->m_pLow[pe1->m_sortDim] > pe2->m_pRegion->m_pLow[pe2->m_sortDim] ) return 1;
              return 0;
            }

            static int compareHigh( const void* pv1, const void* pv2 )
            {
              RstarSplitEntry* pe1 = * ( RstarSplitEntry** ) pv1;
              RstarSplitEntry* pe2 = * ( RstarSplitEntry** ) pv2;

              Q_ASSERT( pe1->m_sortDim == pe2->m_sortDim );

              if ( pe1->m_pRegion->m_pHigh[pe1->m_sortDim] < pe2->m_pRegion->m_pHigh[pe2->m_sortDim] ) return -1;
              if ( pe1->m_pRegion->m_pHigh[pe1->m_sortDim] > pe2->m_pRegion->m_pHigh[pe2->m_sortDim] ) return 1;
              return 0;
            }
        }; // RstarSplitEntry

        class ReinsertEntry
        {
          public:
            long m_id;
            double m_dist;

            ReinsertEntry( long id, double dist ) : m_id( id ), m_dist( dist ) {}

            static int compareReinsertEntry( const void* pv1, const void* pv2 )
            {
              ReinsertEntry* pe1 = * ( ReinsertEntry** ) pv1;
              ReinsertEntry* pe2 = * ( ReinsertEntry** ) pv2;

              if ( pe1->m_dist < pe2->m_dist ) return -1;
              if ( pe1->m_dist > pe2->m_dist ) return 1;
              return 0;
            }
        }; // ReinsertEntry

        // Needed to access protected members without having to cast from Node.
        // It is more efficient than using member functions to access protected members.
        friend class RTree;
        friend class Leaf;
        friend class Index;
        friend class Tools::PointerPool<Node>;
        friend class BulkLoader;
    }; // Node
  }
}

#endif /*__spatialindex_rtree_node_h*/
