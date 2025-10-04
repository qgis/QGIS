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
		class MVRTree;
		class Leaf;
		class Index;
		class Node;

		typedef Tools::PoolPointer<Node> NodePtr;

		class Node : public SpatialIndex::INode
		{
		public:
			~Node() override;

			//
			// Tools::IObject interface
			//
			IObject* clone() override;

			//
			// Tools::ISerializable interface
			//
			uint32_t getByteArraySize() override;
			void loadFromByteArray(const uint8_t* data) override;
			void storeToByteArray(uint8_t** data, uint32_t& len) override;

			//
			// SpatialIndex::IEntry interface
			//
			id_type getIdentifier() const override;
			void getShape(IShape** out) const override;

			//
			// SpatialIndex::INode interface
			//
			uint32_t getChildrenCount() const override;
			id_type getChildIdentifier(uint32_t index)  const override;
			void getChildShape(uint32_t index, IShape** out)  const override;
                        void getChildData(uint32_t index, uint32_t& length, uint8_t** data) const override;
			uint32_t getLevel() const override;
			bool isIndex() const override;
			bool isLeaf() const override;

		private:
			Node();
			Node(MVRTree* pTree, id_type id, uint32_t level, uint32_t capacity);

			virtual Node& operator=(const Node&);

			virtual void insertEntry(uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id);
			virtual bool deleteEntry(uint32_t index);

			virtual bool insertData(
				uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer,
				TimeRegion& mbr2, id_type id2, bool bInsertMbr2 = false, bool forceAdjust = false);
			virtual void insertData(TimeRegion& mbr1, id_type id1, TimeRegion& mbr2, id_type id2, Node* oldVersion, std::stack<id_type>& pathBuffer);
			virtual bool deleteData(id_type id, double delTime, std::stack<id_type>& pathBuffer, bool adjustMBR = false);

			virtual void rtreeSplit(
				uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2,
				TimeRegion& mbr2, id_type id2, bool bInsertMbr2 = false);
			virtual void rstarSplit(
				uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2,
				TimeRegion& mbr2, id_type id2, bool bInsertMbr2 = false);

			virtual void pickSeeds(uint32_t& index1, uint32_t& index2, uint32_t total);

			virtual NodePtr chooseSubtree(const TimeRegion& mbr, uint32_t level, std::stack<id_type>& pathBuffer) = 0;
			virtual NodePtr findLeaf(const TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer) = 0;
			virtual NodePtr findNode(const TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer);

			virtual void split(
				uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, NodePtr& left, NodePtr& right,
				TimeRegion& mbr2, id_type id2, bool bInsertMbr2 = false) = 0;

			MVRTree* m_pTree{nullptr};
				// Parent of all nodes.

			uint32_t m_level{0};
				// The level of the node in the tree.
				// Leaves are always at level 0.

			id_type m_identifier{-1};
				// The unique ID of this node.

			uint32_t m_children{0};
				// The number of children pointed by this node.

			uint32_t m_capacity{0};
				// Specifies the node capacity.

			TimeRegion m_nodeMBR;
				// The minimum bounding region enclosing all data contained in the node.

			uint8_t** m_pData{nullptr};
				// The data stored in the node.

			TimeRegionPtr* m_ptrMBR{nullptr};
				// The corresponding data MBRs.

			id_type* m_pIdentifier{nullptr};
				// The corresponding data identifiers.

			uint32_t* m_pDataLength{nullptr};

			uint32_t m_totalDataLength{0};

			class RstarSplitEntry
			{
			public:
				RstarSplitEntry(TimeRegion* pr, uint32_t index, uint32_t dimension) :
					m_pRegion(pr), m_index(index), m_sortDim(dimension) {}

				static int compareLow(const void* pv1, const void* pv2)
				{
					RstarSplitEntry* pe1 = * (RstarSplitEntry**) pv1;
					RstarSplitEntry* pe2 = * (RstarSplitEntry**) pv2;

					if (pe1->m_pRegion->m_pLow[pe1->m_sortDim] < pe2->m_pRegion->m_pLow[pe2->m_sortDim]) return -1;
					if (pe1->m_pRegion->m_pLow[pe1->m_sortDim] > pe2->m_pRegion->m_pLow[pe2->m_sortDim]) return 1;
					return 0;
				}

				static int compareHigh(const void* pv1, const void* pv2)
				{
					RstarSplitEntry* pe1 = * (RstarSplitEntry**) pv1;
					RstarSplitEntry* pe2 = * (RstarSplitEntry**) pv2;

					if (pe1->m_pRegion->m_pHigh[pe1->m_sortDim] < pe2->m_pRegion->m_pHigh[pe2->m_sortDim]) return -1;
					if (pe1->m_pRegion->m_pHigh[pe1->m_sortDim] > pe2->m_pRegion->m_pHigh[pe2->m_sortDim]) return 1;
					return 0;
				}

				TimeRegion* m_pRegion;
				uint32_t m_index;
				uint32_t m_sortDim;
			}; // RstarSplitEntry

			class DeleteDataEntry
			{
			public:
				DeleteDataEntry(uint32_t index, double d) : m_index(index), m_increase(d) {}

				static bool compare(DeleteDataEntry e1, DeleteDataEntry e2) { return e1.m_increase < e2.m_increase; }

				uint32_t m_index;
				double m_increase;
			}; // DeleteDataEntry

			// Needed to access protected members without having to cast from Node.
			// It is more efficient than using member functions to access protected members.
			friend class MVRTree;
			friend class Leaf;
			friend class Index;
			friend class Tools::PointerPool<Node>;
		}; // Node
	}
}
#pragma GCC diagnostic pop
