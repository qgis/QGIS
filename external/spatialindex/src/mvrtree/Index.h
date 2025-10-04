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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

namespace SpatialIndex
{
	namespace MVRTree
	{
		class Index : public Node
		{
		public:
			~Index() override;

		private:
			Index(MVRTree* pTree, id_type id, uint32_t level);

			NodePtr chooseSubtree(const TimeRegion& mbr, uint32_t level, std::stack<id_type>& pathBuffer) override;
			NodePtr findLeaf(const TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer) override;

			void split(
				uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, NodePtr& left, NodePtr& right,
				TimeRegion& mbr2, id_type id2, bool bInsertMbr2 = false) override;

			uint32_t findLeastEnlargement(const TimeRegion&) const;
			uint32_t findLeastOverlap(const TimeRegion&) const;

			void adjustTree(Node*, std::stack<id_type>&);
			void adjustTree(Node* n, Node* nn, std::stack<id_type>& pathBuffer);

			class OverlapEntry
			{
			public:
				uint32_t m_index;
				double m_enlargement;
				TimeRegionPtr m_original;
				TimeRegionPtr m_combined;
				double m_oa;
				double m_ca;

				static int compareEntries(const void* pv1, const void* pv2)
				{
					OverlapEntry* pe1 = * (OverlapEntry**) pv1;
					OverlapEntry* pe2 = * (OverlapEntry**) pv2;

					if (pe1->m_enlargement < pe2->m_enlargement) return -1;
					if (pe1->m_enlargement > pe2->m_enlargement) return 1;
					return 0;
				}
			}; // OverlapEntry

			friend class MVRTree;
			friend class Node;
		}; // Index
	}
}
#pragma GCC diagnostic pop
