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

#ifndef __spatialindex_rtree_rtree_h
#define __spatialindex_rtree_rtree_h

#include "Statistics.h"
#include "Node.h"
#include "PointerPoolNode.h"

namespace SpatialIndex
{
  namespace RTree
  {
    class RTree : public ISpatialIndex
    {
        class NNEntry;

      public:
        RTree( IStorageManager&, Tools::PropertySet& );
        // String                   Value     Description
        // ----------------------------------------------
        // IndexIndentifier         VT_LONG   If specified an existing index will be openened from the supplied
        //                          storage manager with the given index id. Behaviour is unspecified
        //                          if the index id or the storage manager are incorrect.
        // Dimension                VT_ULONG  Dimensionality of the data that will be inserted.
        // IndexCapacity            VT_ULONG  The index node capacity. Default is 100.
        // LeafCapactiy             VT_ULONG  The leaf node capacity. Default is 100.
        // FillFactor               VT_DOUBLE The fill factor. Default is 70%
        // TreeVariant              VT_LONG   Can be one of Linear, Quadratic or Rstar. Default is Rstar
        // NearMinimumOverlapFactor VT_ULONG  Default is 32.
        // SplitDistributionFactor  VT_DOUBLE Default is 0.4
        // ReinsertFactor           VT_DOUBLE Default is 0.3
        // EnsureTightMBRs          VT_BOOL   Default is true
        // IndexPoolCapacity        VT_LONG   Default is 100
        // LeafPoolCapacity         VT_LONG   Default is 100
        // RegionPoolCapacity       VT_LONG   Default is 1000
        // PointPoolCapacity        VT_LONG   Default is 500

        virtual ~RTree();

        //
        // ISpatialIndex interface
        //
        virtual void insertData( unsigned long len, const byte* pData, const IShape& shape, long shapeIdentifier );
        virtual bool deleteData( const IShape& shape, long id );
        virtual void containsWhatQuery( const IShape& query, IVisitor& v );
        virtual void intersectsWithQuery( const IShape& query, IVisitor& v );
        virtual void pointLocationQuery( const Point& query, IVisitor& v );
        virtual void nearestNeighborQuery( long k, const IShape& query, IVisitor& v, INearestNeighborComparator& );
        virtual void nearestNeighborQuery( long k, const IShape& query, IVisitor& v );
        virtual void selfJoinQuery( const IShape& s, IVisitor& v );
        virtual void queryStrategy( IQueryStrategy& qs );
        virtual void getIndexProperties( Tools::PropertySet& out ) const;
        virtual void addCommand( ICommand* pCommand, CommandType ct );
        virtual bool isIndexValid();
        virtual void getStatistics( IStatistics** out ) const;

      private:
        void initNew( Tools::PropertySet& );
        void initOld( Tools::PropertySet& ps );
        void storeHeader();
        void loadHeader();

        void insertData_impl( unsigned long dataLength, byte* pData, Region& mbr, long id );
        void insertData_impl( unsigned long dataLength, byte* pData, Region& mbr, long id, unsigned long level, byte* overflowTable );
        bool deleteData_impl( const Region& mbr, long id );

        long writeNode( Node* );
        NodePtr readNode( unsigned long );
        void deleteNode( Node* );

        void rangeQuery( RangeQueryType type, const IShape& query, IVisitor& v );
        void selfJoinQuery( long id1, long id2, const Region& r, IVisitor& vis );

        IStorageManager* m_pStorageManager;

        long m_rootID;
        long m_headerID;

        RTreeVariant m_treeVariant;

        double m_fillFactor;

        unsigned long m_indexCapacity;

        unsigned long m_leafCapacity;

        unsigned long m_nearMinimumOverlapFactor;
        // The R*-Tree 'p' constant, for calculating nearly minimum overlap cost.
        // [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
        // for Points and Rectangles', Section 4.1]

        double m_splitDistributionFactor;
        // The R*-Tree 'm' constant, for calculating spliting distributions.
        // [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
        // for Points and Rectangles', Section 4.2]

        double m_reinsertFactor;
        // The R*-Tree 'p' constant, for removing entries at reinserts.
        // [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
        //  for Points and Rectangles', Section 4.3]

        unsigned long m_dimension;

        Region m_infiniteRegion;

        Statistics m_stats;

        bool m_bTightMBRs;

        Tools::PointerPool<Point> m_pointPool;
        Tools::PointerPool<Region> m_regionPool;
        Tools::PointerPool<Node> m_indexPool;
        Tools::PointerPool<Node> m_leafPool;

        std::vector<Tools::SmartPointer<ICommand> > m_writeNodeCommands;
        std::vector<Tools::SmartPointer<ICommand> > m_readNodeCommands;
        std::vector<Tools::SmartPointer<ICommand> > m_deleteNodeCommands;

#ifdef PTHREADS
        pthread_rwlock_t m_rwLock;
#else
        bool m_rwLock;
#endif

        class NNEntry
        {
          public:
            long m_id;
            IEntry* m_pEntry;
            double m_minDist;

            NNEntry( long id, IEntry* e, double f ) : m_id( id ), m_pEntry( e ), m_minDist( f ) {}
            ~NNEntry() {}

            struct ascending : public std::binary_function<NNEntry*, NNEntry*, bool>
            {
              bool operator()( const NNEntry* __x, const NNEntry* __y ) const { return __x->m_minDist > __y->m_minDist; }
            };
        }; // NNEntry

        class NNComparator : public INearestNeighborComparator
        {
          public:
            double getMinimumDistance( const IShape& query, const IShape& entry )
            {
              return query.getMinimumDistance( entry );
            }

            double getMinimumDistance( const IShape& query, const IData& data )
            {
              IShape* pS;
              data.getShape( &pS );
              double ret = query.getMinimumDistance( *pS );
              delete pS;
              return ret;
            }
        }; // NNComparator

        class ValidateEntry
        {
          public:
            ValidateEntry( Region& r, NodePtr& pNode ) : m_parentMBR( r ), m_pNode( pNode ) {}

            Region m_parentMBR;
            NodePtr m_pNode;
        }; // ValidateEntry

        friend class Node;
        friend class Leaf;
        friend class Index;
        friend class BulkLoader;

        friend std::ostream& operator<<( std::ostream& os, const RTree& t );
    }; // RTree

    std::ostream& operator<<( std::ostream& os, const RTree& t );
  }
}

#endif /*__spatialindex_rtree_rtree_h*/

