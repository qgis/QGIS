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
        virtual uint64_t getReads() const;
        virtual uint64_t getWrites() const;
        virtual uint32_t getNumberOfNodes() const;
        virtual uint64_t getNumberOfData() const;

        virtual uint64_t getSplits() const;
        virtual uint64_t getHits() const;
        virtual uint64_t getMisses() const;
        virtual uint64_t getAdjustments() const;
        virtual uint64_t getQueryResults() const;
        virtual uint32_t getTreeHeight() const;
        virtual uint32_t getNumberOfNodesInLevel( uint32_t l ) const;

      private:
        void reset();

        uint64_t m_u64Reads;

        uint64_t m_u64Writes;

        uint64_t m_u64Splits;

        uint64_t m_u64Hits;

        uint64_t m_u64Misses;

        uint32_t m_u32Nodes;

        uint64_t m_u64Adjustments;

        uint64_t m_u64QueryResults;

        uint64_t m_u64Data;

        uint32_t m_u32TreeHeight;

        std::vector<uint32_t> m_nodesInLevel;

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
