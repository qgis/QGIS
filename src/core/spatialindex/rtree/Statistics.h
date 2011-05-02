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

#ifndef __spatialindex_rtree_statistics_h
#define __spatialindex_rtree_statistics_h

namespace SpatialIndex
{
  namespace RTree
  {
    class RTree;
    class Node;
    class Leaf;
    class Index;

    class Statistics : public SpatialIndex::IStatistics
    {
      public:
        Statistics();
        Statistics( const Statistics& );
        virtual ~Statistics();
        Statistics& operator=( const Statistics& );

        //
        // IStatistics interface
        //
        virtual unsigned long getReads() const;
        virtual unsigned long getWrites() const;
        virtual unsigned long getNumberOfNodes() const;
        virtual unsigned long getNumberOfData() const;

        virtual unsigned long getSplits() const;
        virtual unsigned long getHits() const;
        virtual unsigned long getMisses() const;
        virtual unsigned long getAdjustments() const;
        virtual unsigned long getQueryResults() const;
        virtual unsigned long getTreeHeight() const;
        virtual unsigned long getNumberOfNodesInLevel( unsigned long l ) const;

      private:
        void reset();

        unsigned long m_reads;

        unsigned long m_writes;

        unsigned long m_splits;

        unsigned long m_hits;

        unsigned long m_misses;

        unsigned long m_nodes;

        unsigned long m_adjustments;

        unsigned long m_queryResults;

        unsigned long m_data;

        unsigned long m_treeHeight;

        std::vector<unsigned long> m_nodesInLevel;

        friend class RTree;
        friend class Node;
        friend class Index;
        friend class Leaf;
        friend class BulkLoader;

        friend std::ostream& operator<<( std::ostream& os, const Statistics& s );
    }; // Statistics

    std::ostream& operator<<( std::ostream& os, const Statistics& s );
  }
}

#endif /*__spatialindex_rtree_statistics_h*/

