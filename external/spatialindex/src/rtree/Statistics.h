/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

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
			Statistics(const Statistics&);
			~Statistics() override;
			Statistics& operator=(const Statistics&);

			//
			// IStatistics interface
			//
			uint64_t getReads() const override;
			uint64_t getWrites() const override;
			uint32_t getNumberOfNodes() const override;
			uint64_t getNumberOfData() const override;

			virtual uint64_t getSplits() const;
			virtual uint64_t getHits() const;
			virtual uint64_t getMisses() const;
			virtual uint64_t getAdjustments() const;
			virtual uint64_t getQueryResults() const;
			virtual uint32_t getTreeHeight() const;
			virtual uint32_t getNumberOfNodesInLevel(uint32_t l) const;

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

			friend std::ostream& operator<<(std::ostream& os, const Statistics& s);
		}; // Statistics

		std::ostream& operator<<(std::ostream& os, const Statistics& s);
	}
}
