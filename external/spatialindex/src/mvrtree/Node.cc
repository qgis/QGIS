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

#include <cstring>
#include <cmath>
#include <limits>

#include <spatialindex/SpatialIndex.h>

#include "MVRTree.h"
#include "Node.h"
#include "Index.h"
#include "Leaf.h"

using namespace SpatialIndex;
using namespace SpatialIndex::MVRTree;

//
// Tools::IObject interface
//
Tools::IObject* Node::clone()
{
	throw Tools::NotSupportedException("IObject::clone should never be called.");
}

//
// Tools::ISerializable interface
//
uint32_t Node::getByteArraySize()
{
	return
		(sizeof(uint32_t) +
		sizeof(uint32_t) +
		sizeof(uint32_t) +
		sizeof(double) +
		sizeof(double) +
		(m_children * (m_pTree->m_dimension * sizeof(double) * 2 + sizeof(id_type) + 2 * sizeof(double) + sizeof(uint32_t))) +
		m_totalDataLength +
		(2 * m_pTree->m_dimension * sizeof(double)));
}

void Node::loadFromByteArray(const uint8_t* ptr)
{
	m_nodeMBR = m_pTree->m_infiniteRegion;

	// skip the node type information, it is not needed.
	ptr += sizeof(uint32_t);

	memcpy(&m_level, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(&m_children, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(&(m_nodeMBR.m_startTime), ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&(m_nodeMBR.m_endTime), ptr, sizeof(double));
	ptr += sizeof(double);

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		m_ptrMBR[cChild] = m_pTree->m_regionPool.acquire();
		*(m_ptrMBR[cChild]) = m_pTree->m_infiniteRegion;

		memcpy(m_ptrMBR[cChild]->m_pLow, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(m_ptrMBR[cChild]->m_pHigh, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(&(m_pIdentifier[cChild]), ptr, sizeof(id_type));
		ptr += sizeof(id_type);
		memcpy(&(m_ptrMBR[cChild]->m_startTime), ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&(m_ptrMBR[cChild]->m_endTime), ptr, sizeof(double));
		ptr += sizeof(double);

		memcpy(&(m_pDataLength[cChild]), ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		if (m_pDataLength[cChild] > 0)
		{
			m_totalDataLength += m_pDataLength[cChild];
			m_pData[cChild] = new uint8_t[m_pDataLength[cChild]];
			memcpy(m_pData[cChild], ptr, m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
		else
		{
			m_pData[cChild] = nullptr;
		}

		//m_nodeMBR.combineRegion(*(m_ptrMBR[cChild]));
	}

	memcpy(m_nodeMBR.m_pLow, ptr, m_pTree->m_dimension * sizeof(double));
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(m_nodeMBR.m_pHigh, ptr, m_pTree->m_dimension * sizeof(double));
	//ptr += m_pTree->m_dimension * sizeof(double);
}

void Node::storeToByteArray(uint8_t** data, uint32_t& len)
{
	len = getByteArraySize();

	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	uint32_t nodeType;

	if (m_level == 0) nodeType = PersistentLeaf;
	else nodeType = PersistentIndex;

	memcpy(ptr, &nodeType, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(ptr, &m_level, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(ptr, &m_children, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	memcpy(ptr, &(m_nodeMBR.m_startTime), sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &(m_nodeMBR.m_endTime), sizeof(double));
	ptr += sizeof(double);

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		memcpy(ptr, m_ptrMBR[cChild]->m_pLow, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, m_ptrMBR[cChild]->m_pHigh, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, &(m_pIdentifier[cChild]), sizeof(id_type));
		ptr += sizeof(id_type);
		memcpy(ptr, &(m_ptrMBR[cChild]->m_startTime), sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &(m_ptrMBR[cChild]->m_endTime), sizeof(double));
		ptr += sizeof(double);

		memcpy(ptr, &(m_pDataLength[cChild]), sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		if (m_pDataLength[cChild] > 0)
		{
			memcpy(ptr, m_pData[cChild], m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
	}

	// store the node MBR for efficiency. This increases the node size a little bit.
	memcpy(ptr, m_nodeMBR.m_pLow, m_pTree->m_dimension * sizeof(double));
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(ptr, m_nodeMBR.m_pHigh, m_pTree->m_dimension * sizeof(double));
	//ptr += m_pTree->m_dimension * sizeof(double);
}

//
// SpatialIndex::IEntry interface
//
SpatialIndex::id_type Node::getIdentifier() const
{
	return m_identifier;
}

void Node::getShape(IShape** out) const
{
	*out = new TimeRegion(m_nodeMBR);
}

//
// SpatialIndex::INode interface
//
uint32_t Node::getChildrenCount() const
{
	return m_children;
}

SpatialIndex::id_type Node::getChildIdentifier(uint32_t index) const
{
	if (index >= m_children) throw Tools::IndexOutOfBoundsException(index);

	return m_pIdentifier[index];
}

void Node::getChildShape(uint32_t index, IShape** out) const
{
	if (index >= m_children) throw Tools::IndexOutOfBoundsException(index);

	*out = new TimeRegion(*(m_ptrMBR[index]));
}


void Node::getChildData(uint32_t index, uint32_t& length, uint8_t** data) const
{
        if (index >= m_children) throw Tools::IndexOutOfBoundsException(index);
        if (m_pData[index] == nullptr)
        {
                length = 0;
                data = nullptr;
        }
        else
        {
                length = m_pDataLength[index];
                *data = m_pData[index];
        }
}

uint32_t Node::getLevel() const
{
	return m_level;
}

bool Node::isLeaf() const
{
	return (m_level == 0);
}

bool Node::isIndex() const
{
	return (m_level != 0);
}

//
// Internal
//

Node::Node()
= default;

	Node::Node(SpatialIndex::MVRTree::MVRTree* pTree, id_type id, uint32_t level, uint32_t capacity) :
	m_pTree(pTree),
	m_level(level),
	m_identifier(id),
	m_children(0),
	m_capacity(capacity),
	m_pData(nullptr),
	m_ptrMBR(nullptr),
	m_pIdentifier(nullptr),
	m_pDataLength(nullptr),
	m_totalDataLength(0)
{
	m_nodeMBR.makeInfinite(m_pTree->m_dimension);

	try
	{
		m_pDataLength = new uint32_t[m_capacity + 2];
		m_pData = new uint8_t*[m_capacity + 2];
		m_ptrMBR = new TimeRegionPtr[m_capacity + 2];
		m_pIdentifier = new id_type[m_capacity + 2];
	}
	catch (...)
	{
		delete[] m_pDataLength;
		delete[] m_pData;
		delete[] m_ptrMBR;
		delete[] m_pIdentifier;
		throw;
	}
}

Node::~Node()
{
	if (m_pData != nullptr)
	{
		for (uint32_t cChild = 0; cChild < m_children; ++cChild)
		{
			if (m_pData[cChild] != nullptr) delete[] m_pData[cChild];
		}

		delete[] m_pData;
		delete[] m_pDataLength;
	}

	if (m_ptrMBR != nullptr) delete[] m_ptrMBR;
	if (m_pIdentifier != nullptr) delete[] m_pIdentifier;
}

Node& Node::operator=(const Node&)
{
	throw Tools::IllegalStateException("operator =: This should never be called.");
}

void Node::insertEntry(uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id)
{
	assert(m_children < m_capacity);

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	m_totalDataLength += dataLength;
	++m_children;

	m_nodeMBR.combineRegionInTime(mbr);
}

bool Node::deleteEntry(uint32_t index)
{
	assert(index >= 0 && index < m_children);

	// cache it, since I might need it for "touches" later.
	TimeRegionPtr ptrR = m_ptrMBR[index];

	m_totalDataLength -= m_pDataLength[index];
	if (m_pData[index] != nullptr) delete[] m_pData[index];

	if (m_children > 1 && index != m_children - 1)
	{
		m_pDataLength[index] = m_pDataLength[m_children - 1];
		m_pData[index] = m_pData[m_children - 1];
		m_ptrMBR[index] = m_ptrMBR[m_children - 1];
		m_pIdentifier[index] = m_pIdentifier[m_children - 1];
	}

	--m_children;

	// WARNING: index has now changed. Do not use it below here.

	if (m_children == 0)
	{
		m_nodeMBR = m_pTree->m_infiniteRegion;
		return true;
	}
	else if (m_pTree->m_bTightMBRs && m_nodeMBR.touchesShape(*ptrR))
	{
		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < m_children; ++cChild)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim]);
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim]);
			}
		}
		return true;
	}

	return false;
}

bool Node::insertData(
	uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer,
	TimeRegion& mbr2, id_type id2, bool bInsertMbr2, bool bForceAdjust)
{
	// we should be certain that when bInsertMbr2 is true the node needs to be version split

	// this function returns true only if the node under modification has been stored (writeNode(this))
	// it is needed since some times after a version copy we do not need to actually store the node. Only
	// the parent has to be notified to modify the entry pointing
	// to this node with the appropriate deletion time (thus we save one disk access)

	if ((! bInsertMbr2) && m_children < m_capacity)
	{
		// the node has empty space. Insert the entry here

		// this has to happen before insertEntry modifies m_nodeMBR.
		bool b = m_nodeMBR.containsShape(mbr);

		insertEntry(dataLength, pData, mbr, id);
		m_pTree->writeNode(this);

		// a forced adjust might be needed when a child has modified it MBR due to an entry deletion
		// (when the entry start time becomes equal to the entry end time after a version copy)
		if ((! b || bForceAdjust) && (! pathBuffer.empty()))
		{
			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(this, pathBuffer);
		}

		return true;
	}
	else
	{
		// do a version copy

		bool bIsRoot = pathBuffer.empty();

		NodePtr ptrCopy;

		// copy live entries of this node into a new node. Create an index or a leaf respectively
		if (m_level == 0)
		{
			ptrCopy = m_pTree->m_leafPool.acquire();
			if (ptrCopy.get() == nullptr) ptrCopy = NodePtr(new Leaf(m_pTree, - 1), &(m_pTree->m_leafPool));
			else ptrCopy->m_nodeMBR = m_pTree->m_infiniteRegion;
		}
		else
		{
			ptrCopy = m_pTree->m_indexPool.acquire();
			if (ptrCopy.get() == nullptr) ptrCopy = NodePtr(new Index(m_pTree, -1, m_level), &(m_pTree->m_indexPool));
			else
			{
				ptrCopy->m_level = m_level;
				ptrCopy->m_nodeMBR = m_pTree->m_infiniteRegion;
			}
		}

		for (uint32_t cChild = 0; cChild < m_children; ++cChild)
		{
			if (! (m_ptrMBR[cChild]->m_endTime < std::numeric_limits<double>::max()))
			{
				uint8_t* data = nullptr;

				if (m_pDataLength[cChild] > 0)
				{
					data = new uint8_t[m_pDataLength[cChild]];
					memcpy(data, m_pData[cChild], m_pDataLength[cChild] * sizeof(uint8_t));
				}
				ptrCopy->insertEntry(m_pDataLength[cChild], data, *(m_ptrMBR[cChild]), m_pIdentifier[cChild]);
				ptrCopy->m_ptrMBR[ptrCopy->m_children - 1]->m_startTime = mbr.m_startTime;
			}
		}

		ptrCopy->m_nodeMBR.m_startTime = mbr.m_startTime;
		m_nodeMBR.m_endTime = mbr.m_startTime;

		uint32_t children = (bInsertMbr2) ? ptrCopy->m_children + 2 : ptrCopy->m_children + 1;
		assert(children > 0);

		if (children >= m_pTree->m_strongVersionOverflow * m_capacity)
		{
			// strong version overflow. Split!
			NodePtr n;
			NodePtr nn;
			ptrCopy->split(dataLength, pData, mbr, id, n, nn, mbr2, id2, bInsertMbr2);
			assert(n->m_children > 1 && nn->m_children > 1);

			if (bIsRoot)
			{
				// it is a root node. Special handling required.
				n->m_level = ptrCopy->m_level;
				nn->m_level = ptrCopy->m_level;
				n->m_identifier = -1;
				nn->m_identifier = -1;

				m_pTree->writeNode(n.get());
				m_pTree->writeNode(nn.get());

				NodePtr ptrR = m_pTree->m_indexPool.acquire();
				if (ptrR.get() == nullptr) ptrR = NodePtr(new Index(m_pTree, -1, ptrCopy->m_level + 1), &(m_pTree->m_indexPool));
				else
				{
					//ptrR->m_pTree = m_pTree;
					//ptrR->m_identifier = -1;
					ptrR->m_level = ptrCopy->m_level + 1;
					ptrR->m_nodeMBR = m_pTree->m_infiniteRegion;
				}

				ptrR->insertEntry(0, nullptr, n->m_nodeMBR, n->m_identifier);
				ptrR->insertEntry(0, nullptr, nn->m_nodeMBR, nn->m_identifier);

				if (m_nodeMBR.m_startTime == m_nodeMBR.m_endTime)
				{
					ptrR->m_identifier = m_identifier;
					m_pTree->writeNode(ptrR.get());
					m_pTree->m_stats.m_treeHeight[m_pTree->m_stats.m_treeHeight.size() - 1] = ptrR->m_level + 1;
					m_pTree->m_stats.m_nodesInLevel.at(n->m_level) = m_pTree->m_stats.m_nodesInLevel[n->m_level] + 1;
					assert(m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_startTime == ptrCopy->m_nodeMBR.m_startTime &&
								 m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_endTime == ptrCopy->m_nodeMBR.m_endTime);
				}
				else
				{
					m_pTree->writeNode(this);
					m_pTree->writeNode(ptrR.get());

					assert(m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_id == m_identifier);
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_startTime = m_nodeMBR.m_startTime;
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_endTime = m_nodeMBR.m_endTime;
					m_pTree->m_roots.emplace_back(ptrR->m_identifier, ptrR->m_nodeMBR.m_startTime, ptrR->m_nodeMBR.m_endTime);
					m_pTree->m_stats.m_treeHeight.push_back(ptrR->m_level + 1);
					m_pTree->m_stats.m_nodesInLevel.at(n->m_level) = m_pTree->m_stats.m_nodesInLevel[n->m_level] + 2;
					if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
					else ++(m_pTree->m_stats.m_u32DeadLeafNodes);
				}

				if (ptrR->m_level >= m_pTree->m_stats.m_nodesInLevel.size()) m_pTree->m_stats.m_nodesInLevel.push_back(1);
				else m_pTree->m_stats.m_nodesInLevel.at(ptrR->m_level) = m_pTree->m_stats.m_nodesInLevel[ptrR->m_level] + 1;

				return true;
			}
			else
			{
				bool b = false;

				n->m_level = ptrCopy->m_level;
				nn->m_level = ptrCopy->m_level;
/*
				if (m_nodeMBR.m_startTime == m_nodeMBR.m_endTime)
				{
					n->m_identifier = m_identifier;
					m_pTree->m_stats.m_nodesInLevel[n->m_level] = m_pTree->m_stats.m_nodesInLevel[n->m_level] + 1;
					b = true;
				}
				else
				{
					n->m_identifier = -1;
					m_pTree->m_stats.m_nodesInLevel[n->m_level] = m_pTree->m_stats.m_nodesInLevel[n->m_level] + 2;
				}
*/
				n->m_identifier = -1;
				nn->m_identifier = -1;

				m_pTree->m_stats.m_nodesInLevel.at(n->m_level) = m_pTree->m_stats.m_nodesInLevel[n->m_level] + 2;
				if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
				else ++(m_pTree->m_stats.m_u32DeadLeafNodes);

				m_pTree->writeNode(n.get());
				m_pTree->writeNode(nn.get());

				id_type cParent = pathBuffer.top(); pathBuffer.pop();
				NodePtr ptrN = m_pTree->readNode(cParent);
				Index* p = static_cast<Index*>(ptrN.get());
				++(m_pTree->m_stats.m_u64Adjustments);

				// this is the special insertion function for two new nodes, defined below
				p->insertData(n->m_nodeMBR, n->m_identifier, nn->m_nodeMBR, nn->m_identifier, this, pathBuffer);

				return b;
			}
		}
		//else if (children < m_pTree->m_strongVersionUnderflow * m_capacity)
		//{
	 	//	do not do this for now
		//}
		else
		{
			// the entry contains the appropriate number of live entries

			ptrCopy->insertEntry(dataLength, pData, mbr, id);
			if (bInsertMbr2) ptrCopy->insertEntry(0, nullptr, mbr2, id2);

			if (bIsRoot)
			{
				if (m_nodeMBR.m_startTime == m_nodeMBR.m_endTime)
				{
					ptrCopy->m_identifier = m_identifier;
					m_pTree->writeNode(ptrCopy.get());
					assert(m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_startTime == ptrCopy->m_nodeMBR.m_startTime &&
								 m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_endTime == ptrCopy->m_nodeMBR.m_endTime);
				}
				else
				{
					m_pTree->writeNode(ptrCopy.get());
					m_pTree->writeNode(this);

					assert(m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_id == m_identifier);
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_startTime = m_nodeMBR.m_startTime;
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_endTime = m_nodeMBR.m_endTime;
					m_pTree->m_roots.emplace_back(ptrCopy->m_identifier, ptrCopy->m_nodeMBR.m_startTime, ptrCopy->m_nodeMBR.m_endTime);
					m_pTree->m_stats.m_treeHeight.push_back(ptrCopy->m_level + 1);

					m_pTree->m_stats.m_nodesInLevel.at(ptrCopy->m_level) = m_pTree->m_stats.m_nodesInLevel[ptrCopy->m_level] + 1;
					if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
					else ++(m_pTree->m_stats.m_u32DeadLeafNodes);
				}

				return true;
			}
			else
			{
				m_pTree->writeNode(ptrCopy.get());

				m_pTree->m_stats.m_nodesInLevel.at(ptrCopy->m_level) = m_pTree->m_stats.m_nodesInLevel[ptrCopy->m_level] + 1;
				if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
				else ++(m_pTree->m_stats.m_u32DeadLeafNodes);

				id_type cParent = pathBuffer.top(); pathBuffer.pop();
				NodePtr ptrN = m_pTree->readNode(cParent);
				Index* p = static_cast<Index*>(ptrN.get());
				++(m_pTree->m_stats.m_u64Adjustments);

				uint32_t child;
				for (child = 0; child < p->m_children; ++child)
				{
					if (p->m_pIdentifier[child] == m_identifier) break;
				}

				// it might be needed to update the MBR since the child MBR might have changed
				// from an entry deletion (from insertData, below, when m_startTime == m_endTime)
				double st = p->m_ptrMBR[child]->m_startTime;
				*(p->m_ptrMBR[child]) = m_nodeMBR;
				p->m_ptrMBR[child]->m_startTime = st;
				//p->m_ptrMBR[child]->m_endTime = mbr.m_startTime;

				// insert this new version copy into the parent
				p->insertData(0, nullptr, ptrCopy->m_nodeMBR, ptrCopy->m_identifier, pathBuffer, m_pTree->m_infiniteRegion, -1, false);

				return false;
			}
		}
	}
}

void Node::insertData(TimeRegion& mbr1, id_type id1, TimeRegion& mbr2, id_type id2, Node* oldVersion, std::stack<id_type>& pathBuffer)
{
	// this should be called only from insertData above
	// it tries to fit two new entries into the node

	uint32_t child;
	for (child = 0; child < m_children; ++child)
	{
		if (m_pIdentifier[child] == oldVersion->m_identifier) break;
	}

	// save the original node MBR
	bool bAdjust = false;
	TimeRegionPtr ptrR = m_pTree->m_regionPool.acquire();
	*ptrR = m_nodeMBR;

	// it might be needed to update the MBR since the child MBR might have changed
	// from an entry deletion (when m_startTime == m_endTime)
	double st = m_ptrMBR[child]->m_startTime;
	*(m_ptrMBR[child]) = oldVersion->m_nodeMBR;
	m_ptrMBR[child]->m_startTime = st;
	//m_ptrMBR[child]->m_endTime = oldVersion->m_nodeMBR.m_endTime;

	if (m_children < m_capacity - 1)
	{
		// there is enough space for both new entries

		insertEntry(0, nullptr, mbr1, id1);
		insertEntry(0, nullptr, mbr2, id2);

		m_pTree->writeNode(this);

		if ((! pathBuffer.empty()) && (bAdjust || ! (ptrR->containsShape(mbr1) && ptrR->containsShape(mbr2))))
		{
			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(this, pathBuffer);
		}
	}
	else
	{
		// call a normal insertData which will trigger a version copy
		// insertData will adjust the parent since this node will certainly do a version copy
		bool bStored = insertData(0, nullptr, mbr1, id1, pathBuffer, mbr2, id2, true);
		if (! bStored) m_pTree->writeNode(this);
	}
}

bool Node::deleteData(id_type id, double delTime, std::stack<id_type>& pathBuffer, bool bForceAdjust)
{
	// it returns true if a new root has been created because all the entries of the old root have died.
	// This is needed in case the root dies while there are pending reinsertions from multiple levels

	uint32_t child = m_capacity;
	uint32_t alive = 0;
	bool bAdjustParent = false;
	TimeRegionPtr oldNodeMBR = m_pTree->m_regionPool.acquire();
	*oldNodeMBR = m_nodeMBR;
	NodePtr parent;

	// make sure that there are no "snapshot" entries
	// find how many children are alive and locate the entry to be deleted
	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		assert(m_level != 0 || (m_ptrMBR[cChild]->m_startTime != m_ptrMBR[cChild]->m_endTime));
		if (! (m_ptrMBR[cChild]->m_endTime < std::numeric_limits<double>::max())) ++alive;
		if (m_pIdentifier[cChild] == id) child = cChild;
	}

	assert(child < m_capacity);

	// either make the entry dead or, if its start time is equal to the deletion time,
	// delete it from the node completely (in which case the parent MBR might need adjustment)
	bool bAdjusted = false;

	if (m_level == 0 && m_ptrMBR[child]->m_startTime == delTime)
	{
		bAdjusted = deleteEntry(child);
		bAdjustParent = bAdjusted;
	}
	else
	{
		m_ptrMBR[child]->m_endTime = delTime;
	}

	// if it has not been adjusted yet (by deleteEntry) and it should be adjusted, do it.
	// a forced adjustment is needed when a child node has adjusted its own MBR and signals
	// the parent to adjust it, also.
	if ((! bAdjusted) && bForceAdjust)
	{
		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < m_children; ++cChild)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim]);
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim]);
			}
		}
		// signal our parent to adjust its MBR also
		bAdjustParent = true;
	}

	// one less live entry from now on
	--alive;

	if (alive < m_pTree->m_versionUnderflow * m_capacity && (! pathBuffer.empty()))
	{
		// if the weak version condition is broken, try to resolve it
		// if this is a leaf and it can still hold some entries (since all entries might be dead now and
		// the node full) try to borrow a live entry from a sibling
		// [Yufei Tao, Dimitris Papadias, 'MV3R-Tree: A Spatio-Temporal Access Method for Timestamp and
		// Interval Queries', Section 3.3]
		if (m_level == 0 && m_children < m_capacity)
		{
			parent = m_pTree->readNode(pathBuffer.top());
			pathBuffer.pop();

			// find us in our parent
			for (child = 0; child < parent->m_children; ++child)
			{
				if (parent->m_pIdentifier[child] == m_identifier) break;
			}

			// remember that the parent might be younger than us, pointing to us through a pointer
			// created with a version copy. So the actual start time of this node through the path
			// from the root might actually be different than the stored start time.
			double actualNodeStartTime = parent->m_ptrMBR[child]->m_startTime;

			// find an appropriate sibling
			for (uint32_t cSibling = 0; cSibling < parent->m_children; ++cSibling)
			{
				// it has to be different than us, it has to be alive and its MBR should intersect ours
				if (
					parent->m_pIdentifier[cSibling] != m_identifier &&
					! (parent->m_ptrMBR[cSibling]->m_endTime < std::numeric_limits<double>::max()) &&
					parent->m_ptrMBR[cSibling]->intersectsShape(m_nodeMBR))
				{
					NodePtr sibling = m_pTree->readNode(parent->m_pIdentifier[cSibling]);
					std::vector<DeleteDataEntry> toCheck;
					alive = 0;

					// if this child does not have a single parent, we cannot borrow an entry.
					bool bSingleParent = true;

					for (uint32_t cSiblingChild = 0; cSiblingChild < sibling->m_children; ++cSiblingChild)
					{
						// if the insertion time of any child is smaller than the starting time stored in the
						// parent of this node than the node has more than one parent
						if (sibling->m_ptrMBR[cSiblingChild]->m_startTime < parent->m_ptrMBR[cSibling]->m_startTime)
						{
							bSingleParent = false;
							break;
						}

						// find the live sibling entries, and also the ones that can be moved to this node
						// sort them by area enlargement
						if (! (sibling->m_ptrMBR[cSiblingChild]->m_endTime < std::numeric_limits<double>::max()))
						{
							++alive;
							if (sibling->m_ptrMBR[cSiblingChild]->m_startTime >= actualNodeStartTime)
							{
								TimeRegionPtr tmpR = m_pTree->m_regionPool.acquire();
								*tmpR = m_nodeMBR;
								tmpR->combineRegion(*(sibling->m_ptrMBR[cSiblingChild]));
								double a = tmpR->getArea();
								if (a <= m_nodeMBR.getArea() * 1.1) toCheck.emplace_back(cSiblingChild, a);
							}
						}
					}

					// if the sibling has more than one parent or if we cannot remove an entry because we will
					// cause a weak version overflow, this sibling is not appropriate
					if ((! bSingleParent) || toCheck.empty() || alive == m_pTree->m_versionUnderflow * sibling->m_capacity + 1) continue;

					// create interval counters for checking weak version condition
					// [Yufei Tao, Dimitris Papadias, 'MV3R-Tree: A Spatio-Temporal Access Method for Timestamp and
					// Interval Queries', Section 3.2]
					std::set<double> Si;
					for (uint32_t cSiblingChild = 0; cSiblingChild < sibling->m_children; ++cSiblingChild)
					{
						Si.insert(sibling->m_ptrMBR[cSiblingChild]->m_startTime);
						Si.insert(sibling->m_ptrMBR[cSiblingChild]->m_endTime);
					}
					// duplicate entries have been removed and the set is sorted
					uint32_t* SiCounts = new uint32_t[Si.size() - 1];
					memset(SiCounts, 0, (Si.size() - 1) * sizeof(uint32_t));

					for (uint32_t cSiblingChild = 0; cSiblingChild < sibling->m_children; ++cSiblingChild)
					{
						std::set<double>::iterator it1 = Si.begin();
						std::set<double>::iterator it2 = Si.begin();
						for (size_t cIndex = 0; cIndex < Si.size() - 1; ++cIndex)
						{
							++it2;
							if (
								sibling->m_ptrMBR[cSiblingChild]->m_startTime <= *it1 &&
								sibling->m_ptrMBR[cSiblingChild]->m_endTime >= *it2
							) ++(SiCounts[cIndex]);
							++it1;
						}
					}

					std::vector<DeleteDataEntry> Sdel;

					for (size_t cCheck = 0; cCheck < toCheck.size(); ++cCheck)
					{
						bool good = true;

						// check if it can be removed without a weak version underflow
						std::set<double>::iterator it1 = Si.begin();
						std::set<double>::iterator it2 = Si.begin();
						for (size_t cIndex = 0; cIndex < Si.size() - 1; ++cIndex)
						{
							++it2;
							if (
								sibling->m_ptrMBR[toCheck[cCheck].m_index]->m_startTime <= *it1 &&
								sibling->m_ptrMBR[toCheck[cCheck].m_index]->m_endTime >= *it2 &&
								SiCounts[cIndex] <= m_pTree->m_versionUnderflow * sibling->m_capacity)
							{
								good = false;
								break;
							}
							++it1;
						}
						if (good) Sdel.push_back(toCheck[cCheck]);
					}

					delete[] SiCounts;

					if (Sdel.empty()) continue;

					// we found some entries. Sort them according to least enlargement, insert the best entry into
					// this node, remove it from the sibling and update the MBRs of the parent

					sort(Sdel.begin(), Sdel.end(), DeleteDataEntry::compare);
					uint32_t entry = Sdel[0].m_index;
					bool b1 = m_nodeMBR.containsShape(*(sibling->m_ptrMBR[entry]));
					bool b2 = sibling->m_nodeMBR.touchesShape(*(sibling->m_ptrMBR[entry]));

					insertEntry(sibling->m_pDataLength[entry], sibling->m_pData[entry], *(sibling->m_ptrMBR[entry]), sibling->m_pIdentifier[entry]);
					sibling->m_pData[entry] = nullptr;

					// the weak version condition check above, guarantees that.
					assert(sibling->m_children > 1);
					sibling->deleteEntry(entry);

					m_pTree->writeNode(this);
					m_pTree->writeNode(sibling.get());

					Index* p = static_cast<Index*>(parent.get());
					if (((! b1) || bAdjustParent) && b2) p->adjustTree(this, sibling.get(), pathBuffer);
					else if ((! b1) || bAdjustParent) p->adjustTree(this, pathBuffer);
					else if (b2) p->adjustTree(sibling.get(), pathBuffer);

					return false;
				}
			}
		}

		// either this is not a leaf, or an appropriate sibling was not found, so make this node dead
		// and reinsert all live entries from the root
		m_nodeMBR.m_endTime = delTime;
		m_pTree->writeNode(this);
		if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
		else ++(m_pTree->m_stats.m_u32DeadLeafNodes);

		if (parent.get() == nullptr)
		{
			parent = m_pTree->readNode(pathBuffer.top());
			pathBuffer.pop();
		}

		if (bAdjustParent)
		{
			// the correct child pointer might have been calculated already from earlier
			if (child < parent->m_children && m_identifier != parent->m_pIdentifier[child])
			{
				for (child = 0; child < parent->m_children; ++child)
				{
					if (parent->m_pIdentifier[child] == m_identifier) break;
				}
			}

			// both start time and end time should be preserved since deleteData below needs
			// to know how many entries where alive, including this one
			double st = parent->m_ptrMBR[child]->m_startTime;
			double en = parent->m_ptrMBR[child]->m_endTime;
			*(parent->m_ptrMBR[child]) = m_nodeMBR;
			parent->m_ptrMBR[child]->m_startTime = st;
			parent->m_ptrMBR[child]->m_endTime = en;
		}

		// delete this node from the parent node.
		// if this node had been adjusted and its old MBR was touching the parent MBR, the
		// parent MBR needs to be adjusted also.
		// the deletion has to happen first, since the reinsertions might modify the path to this node
		bool bNewRoot = parent->deleteData(m_identifier, delTime, pathBuffer, (bAdjustParent && parent->m_nodeMBR.touchesShape(*oldNodeMBR)));

		// reinsert all the live entries from the root

		// normally I should not modify any node instances, since writeNode might be caching nodes
		// in main memory, even though I have persisted them, so I have to make copies

		// this code will try and reinsert whole paths if possible. It might be the case, though,
		// that a root died, which means that all the live data entries have to be scanned and reinserted themselves
		for (child = 0; child < m_children; ++child)
		{
			if (! (m_ptrMBR[child]->m_endTime < std::numeric_limits<double>::max()))
			{
				if (! bNewRoot || m_level == 0)
				{
					m_ptrMBR[child]->m_startTime = delTime;
					m_pTree->insertData_impl(m_pDataLength[child], m_pData[child], *(m_ptrMBR[child]), m_pIdentifier[child], m_level);
					// make sure we do not delete the data array from this node's destructor
					m_pData[child] = nullptr;
				}
				else
				{
					std::stack<NodePtr> Sins;
					Sins.push(m_pTree->readNode(m_pIdentifier[child]));
					while (! Sins.empty())
					{
						NodePtr p = Sins.top(); Sins.pop();
						if (p->m_level == 0)
						{
							for (uint32_t cIndex= 0; cIndex < p->m_children; ++cIndex)
							{
								if (! (p->m_ptrMBR[cIndex]->m_endTime < std::numeric_limits<double>::max()))
								{
									p->m_ptrMBR[cIndex]->m_startTime = delTime;
									m_pTree->insertData_impl(p->m_pDataLength[cIndex], p->m_pData[cIndex], *(p->m_ptrMBR[cIndex]), p->m_pIdentifier[cIndex], p->m_level);
									// make sure we do not delete the data array from this node's destructor
									p->m_pData[cIndex] = nullptr;
								}
							}
						}
						else
						{
							for (uint32_t cIndex= 0; cIndex < p->m_children; ++cIndex)
							{
								if (! (p->m_ptrMBR[cIndex]->m_endTime < std::numeric_limits<double>::max()))
								{
									Sins.push(m_pTree->readNode(p->m_pIdentifier[cIndex]));
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// either this is a root node or there is no weak version condition

		if (alive == 0 && pathBuffer.empty())
		{
			if (m_children > 0)
			{
				// all root children are dead. Create a new root
				m_nodeMBR.m_endTime = delTime;
				m_pTree->m_bHasVersionCopied = false;

				if (m_nodeMBR.m_startTime == m_nodeMBR.m_endTime)
				{
					Leaf root(m_pTree, m_identifier);
					root.m_nodeMBR.m_startTime = m_nodeMBR.m_endTime;
					root.m_nodeMBR.m_endTime = std::numeric_limits<double>::max();
					m_pTree->writeNode(&root);

					m_pTree->m_stats.m_treeHeight[m_pTree->m_stats.m_treeHeight.size() - 1] = 1;
					if (m_pTree->m_stats.m_nodesInLevel.at(m_level) == 1) m_pTree->m_stats.m_nodesInLevel.pop_back();
					else m_pTree->m_stats.m_nodesInLevel.at(m_level) = m_pTree->m_stats.m_nodesInLevel[m_level] - 1;
					m_pTree->m_stats.m_nodesInLevel.at(0) = m_pTree->m_stats.m_nodesInLevel[0] + 1;
				}
				else
				{
					m_pTree->writeNode(this);

					if (m_level > 0) ++(m_pTree->m_stats.m_u32DeadIndexNodes);
					else ++(m_pTree->m_stats.m_u32DeadLeafNodes);

					Leaf root(m_pTree, -1);
					root.m_nodeMBR.m_startTime = m_nodeMBR.m_endTime;
					root.m_nodeMBR.m_endTime = std::numeric_limits<double>::max();
					m_pTree->writeNode(&root);
					assert(m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_id == m_identifier);
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_startTime = m_nodeMBR.m_startTime;
					m_pTree->m_roots[m_pTree->m_roots.size() - 1].m_endTime = m_nodeMBR.m_endTime;
					m_pTree->m_roots.emplace_back(root.m_identifier, root.m_nodeMBR.m_startTime, root.m_nodeMBR.m_endTime);

					m_pTree->m_stats.m_treeHeight.push_back(1);
					m_pTree->m_stats.m_nodesInLevel.at(root.m_level) = m_pTree->m_stats.m_nodesInLevel[root.m_level] + 1;
				}
				return true;
			}
			else
			{
				assert(m_level == 0);
				m_pTree->writeNode(this);
				m_pTree->m_bHasVersionCopied = false;
				return false;
			}
		}
		else if (bAdjustParent && (! pathBuffer.empty()))
		{
			// the parent needs to be adjusted
			m_pTree->writeNode(this);
			parent = m_pTree->readNode(pathBuffer.top());
			pathBuffer.pop();
			Index* p = static_cast<Index*>(parent.get());
			p->adjustTree(this, pathBuffer);
		}
		else
		{
			m_pTree->writeNode(this);
		}
	}

	return false;
}

void Node::rtreeSplit(
	uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2,
	TimeRegion& mbr2, id_type id2, bool bInsertMbr2)
{
	uint32_t cChild;
	uint32_t minimumLoad = static_cast<uint32_t>(std::floor(m_capacity * m_pTree->m_fillFactor));

	uint32_t cTotal = (bInsertMbr2) ? m_children + 2 : m_children + 1;

	// use this mask array for marking visited entries.
	uint8_t* mask = new uint8_t[cTotal];
	memset(mask, 0, cTotal);

	// insert new data in the node for easier manipulation. Data arrays are always
	// by two larger than node capacity.
	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	if (bInsertMbr2)
	{
		m_pDataLength[m_children + 1] = 0;
		m_pData[m_children + 1] = nullptr;
		m_ptrMBR[m_children + 1] = m_pTree->m_regionPool.acquire();
		*(m_ptrMBR[m_children + 1]) = mbr2;
		m_pIdentifier[m_children + 1] = id2;
	}

	// initialize each group with the seed entries.
	uint32_t seed1, seed2;
	pickSeeds(seed1, seed2, cTotal);

	group1.push_back(seed1);
	group2.push_back(seed2);

	mask[seed1] = 1;
	mask[seed2] = 1;

	// find MBR of each group.
	TimeRegionPtr mbrA = m_pTree->m_regionPool.acquire();
	*mbrA = *(m_ptrMBR[seed1]);
	TimeRegionPtr mbrB = m_pTree->m_regionPool.acquire();
	*mbrB = *(m_ptrMBR[seed2]);

	// count how many entries are left unchecked (exclude the seeds here.)
	uint32_t cRemaining = cTotal - 2;

	while (cRemaining > 0)
	{
		if (minimumLoad - group1.size() == cRemaining)
		{
			// all remaining entries must be assigned to group1 to comply with minimun load requirement.
			for (cChild = 0; cChild < cTotal; ++cChild)
			{
				if (mask[cChild] == 0)
				{
					group1.push_back(cChild);
					mask[cChild] = 1;
					--cRemaining;
				}
			}
		}
		else if (minimumLoad - group2.size() == cRemaining)
		{
			// all remaining entries must be assigned to group2 to comply with minimun load requirement.
			for (cChild = 0; cChild < cTotal; ++cChild)
			{
				if (mask[cChild] == 0)
				{
					group2.push_back(cChild);
					mask[cChild] = 1;
					--cRemaining;
				}
			}
		}
		else
		{
			// For all remaining entries compute the difference of the cost of grouping an
			// entry in either group. When done, choose the entry that yielded the maximum
			// difference. In case of linear split, select any entry (e.g. the first one.)
			uint32_t sel;
			double md1 = 0.0, md2 = 0.0;
			double m = -std::numeric_limits<double>::max();
			double d1, d2, d;
			double a1 = mbrA->getArea();
			double a2 = mbrB->getArea();

			TimeRegionPtr a = m_pTree->m_regionPool.acquire();
			TimeRegionPtr b = m_pTree->m_regionPool.acquire();

			for (cChild = 0; cChild < cTotal; ++cChild)
			{
				if (mask[cChild] == 0)
				{
					mbrA->getCombinedRegion(*a, *(m_ptrMBR[cChild]));
					d1 = a->getArea() - a1;
					mbrB->getCombinedRegion(*b, *(m_ptrMBR[cChild]));
					d2 = b->getArea() - a2;
					d = std::abs(d1 - d2);

					if (d > m)
					{
						m = d;
						md1 = d1; md2 = d2;
						sel = cChild;
						if (m_pTree->m_treeVariant== RV_LINEAR || m_pTree->m_treeVariant == RV_RSTAR) break;
					}
				}
			}

			// determine the group where we should add the new entry.
			int32_t group = 1;

			if (md1 < md2)
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (md2 < md1)
			{
				group2.push_back(sel);
				group = 2;
			}
			else if (a1 < a2)
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (a2 < a1)
			{
				group2.push_back(sel);
				group = 2;
			}
			else if (group1.size() < group2.size())
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (group2.size() < group1.size())
			{
				group2.push_back(sel);
				group = 2;
			}
			else
			{
				group1.push_back(sel);
				group = 1;
			}
			mask[sel] = 1;
			--cRemaining;
			if (group == 1)
			{
				mbrA->combineRegion(*(m_ptrMBR[sel]));
			}
			else
			{
				mbrB->combineRegion(*(m_ptrMBR[sel]));
			}
		}
	}

	delete[] mask;
}

void Node::rstarSplit(
	uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2,
	TimeRegion& mbr2, id_type id2, bool bInsertMbr2)
{
	RstarSplitEntry** dataLow = nullptr;
	RstarSplitEntry** dataHigh = nullptr;

	uint32_t cTotal = (bInsertMbr2) ? m_children + 2 : m_children + 1;

	try
	{
		dataLow = new RstarSplitEntry*[cTotal];
		dataHigh = new RstarSplitEntry*[cTotal];
	}
	catch (...)
	{
		delete[] dataLow;
		throw;
	}

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	if (bInsertMbr2)
	{
		m_pDataLength[m_children + 1] = 0;
		m_pData[m_children + 1] = nullptr;
		m_ptrMBR[m_children + 1] = m_pTree->m_regionPool.acquire();
		*(m_ptrMBR[m_children + 1]) = mbr2;
		m_pIdentifier[m_children + 1] = id2;
	}

	uint32_t nodeSPF = static_cast<uint32_t>(std::floor(cTotal * m_pTree->m_splitDistributionFactor));
	uint32_t splitDistribution = cTotal - (2 * nodeSPF) + 2;

	uint32_t cChild = 0, cDim, cIndex;

	for (cChild = 0; cChild < cTotal; ++cChild)
	{
		try
		{
			dataLow[cChild] = new RstarSplitEntry(m_ptrMBR[cChild].get(), cChild, 0);
		}
		catch (...)
		{
			for (uint32_t i = 0; i < cChild; ++i) delete dataLow[i];
			delete[] dataLow;
			delete[] dataHigh;
			throw;
		}

		dataHigh[cChild] = dataLow[cChild];
	}

	double minimumMargin = std::numeric_limits<double>::max();
	uint32_t splitAxis = std::numeric_limits<uint32_t>::max();
	uint32_t sortOrder = std::numeric_limits<uint32_t>::max();

	// chooseSplitAxis.
	for (cDim = 0; cDim < m_pTree->m_dimension; ++cDim)
	{
		::qsort(dataLow,
						cTotal,
						sizeof(RstarSplitEntry*),
						RstarSplitEntry::compareLow);

		::qsort(dataHigh,
						cTotal,
						sizeof(RstarSplitEntry*),
						RstarSplitEntry::compareHigh);

		// calculate sum of margins and overlap for all distributions.
		double marginl = 0.0;
		double marginh = 0.0;

		TimeRegion bbl1, bbl2, bbh1, bbh2;

		for (cChild = 1; cChild <= splitDistribution; ++cChild)
		{
			uint32_t l = nodeSPF - 1 + cChild;

			bbl1 = *(dataLow[0]->m_pRegion);
			bbh1 = *(dataHigh[0]->m_pRegion);

			for (cIndex = 1; cIndex < l; ++cIndex)
			{
				bbl1.combineRegion(*(dataLow[cIndex]->m_pRegion));
				bbh1.combineRegion(*(dataHigh[cIndex]->m_pRegion));
			}

			bbl2 = *(dataLow[l]->m_pRegion);
			bbh2 = *(dataHigh[l]->m_pRegion);

			for (cIndex = l + 1; cIndex < cTotal; ++cIndex)
			{
				bbl2.combineRegion(*(dataLow[cIndex]->m_pRegion));
				bbh2.combineRegion(*(dataHigh[cIndex]->m_pRegion));
			}

			marginl += bbl1.getMargin() + bbl2.getMargin();
			marginh += bbh1.getMargin() + bbh2.getMargin();
		} // for (cChild)

		double margin = std::min(marginl, marginh);

		// keep minimum margin as split axis.
		if (margin < minimumMargin)
		{
			minimumMargin = margin;
			splitAxis = cDim;
			sortOrder = (marginl < marginh) ? 0 : 1;
		}

		// increase the dimension according to which the data entries should be sorted.
		for (cChild = 0; cChild < cTotal; ++cChild)
		{
			dataLow[cChild]->m_sortDim = cDim + 1;
		}
	} // for (cDim)

	for (cChild = 0; cChild < cTotal; ++cChild)
	{
		dataLow[cChild]->m_sortDim = splitAxis;
	}

	::qsort(
		dataLow,
		cTotal,
		sizeof(RstarSplitEntry*),
		(sortOrder == 0) ? RstarSplitEntry::compareLow : RstarSplitEntry::compareHigh);

	double ma = std::numeric_limits<double>::max();
	double mo = std::numeric_limits<double>::max();
	uint32_t splitPoint = std::numeric_limits<uint32_t>::max();

	TimeRegion bb1, bb2;

	for (cChild = 1; cChild <= splitDistribution; ++cChild)
	{
		uint32_t l = nodeSPF - 1 + cChild;

		bb1 = *(dataLow[0]->m_pRegion);

		for (cIndex = 1; cIndex < l; ++cIndex)
		{
			bb1.combineRegion(*(dataLow[cIndex]->m_pRegion));
		}

		bb2 = *(dataLow[l]->m_pRegion);

		for (cIndex = l + 1; cIndex < cTotal; ++cIndex)
		{
			bb2.combineRegion(*(dataLow[cIndex]->m_pRegion));
		}

		double o = bb1.getIntersectingArea(bb2);

		if (o < mo)
		{
			splitPoint = cChild;
			mo = o;
			ma = bb1.getArea() + bb2.getArea();
		}
		else if (o == mo)
		{
			double a = bb1.getArea() + bb2.getArea();

			if (a < ma)
			{
				splitPoint = cChild;
				ma = a;
			}
		}
	} // for (cChild)

	uint32_t l1 = nodeSPF - 1 + splitPoint;

	for (cIndex = 0; cIndex < l1; ++cIndex)
	{
		group1.push_back(dataLow[cIndex]->m_index);
		delete dataLow[cIndex];
	}

	for (cIndex = l1; cIndex < cTotal; ++cIndex)
	{
		group2.push_back(dataLow[cIndex]->m_index);
		delete dataLow[cIndex];
	}

	delete[] dataLow;
	delete[] dataHigh;
}

void Node::pickSeeds(uint32_t& index1, uint32_t& index2, uint32_t total)
{
	double separation = -std::numeric_limits<double>::max();
	double inefficiency = -std::numeric_limits<double>::max();
	uint32_t cDim, cChild, cIndex;

	switch (m_pTree->m_treeVariant)
	{
		case RV_LINEAR:
		case RV_RSTAR:
			for (cDim = 0; cDim < m_pTree->m_dimension; ++cDim)
			{
				double leastLower = m_ptrMBR[0]->m_pLow[cDim];
				double greatestUpper = m_ptrMBR[0]->m_pHigh[cDim];
				uint32_t greatestLower = 0;
				uint32_t leastUpper = 0;
				double width;

				for (cChild = 1; cChild < total; ++cChild)
				{
					if (m_ptrMBR[cChild]->m_pLow[cDim] > m_ptrMBR[greatestLower]->m_pLow[cDim]) greatestLower = cChild;
					if (m_ptrMBR[cChild]->m_pHigh[cDim] < m_ptrMBR[leastUpper]->m_pHigh[cDim]) leastUpper = cChild;

					leastLower = std::min(m_ptrMBR[cChild]->m_pLow[cDim], leastLower);
					greatestUpper = std::max(m_ptrMBR[cChild]->m_pHigh[cDim], greatestUpper);
				}

				width = greatestUpper - leastLower;
				if (width <= 0) width = 1;

				double f = (m_ptrMBR[greatestLower]->m_pLow[cDim] - m_ptrMBR[leastUpper]->m_pHigh[cDim]) / width;

				if (f > separation)
				{
					index1 = leastUpper;
					index2 = greatestLower;
					separation = f;
				}
			}  // for (cDim)

			if (index1 == index2)
			{
				if (index2 == 0) ++index2;
				else --index2;
			}

			break;
		case RV_QUADRATIC:
			// for each pair of Regions (account for overflow Region too!)
			for (cChild = 0; cChild < total - 1; ++cChild)
			{
				double a = m_ptrMBR[cChild]->getArea();

				for (cIndex = cChild + 1; cIndex < total; ++cIndex)
				{
					// get the combined MBR of those two entries.
					TimeRegion r;
					m_ptrMBR[cChild]->getCombinedRegion(r, *(m_ptrMBR[cIndex]));

					// find the inefficiency of grouping these entries together.
					double d = r.getArea() - a - m_ptrMBR[cIndex]->getArea();

					if (d > inefficiency)
					{
						inefficiency = d;
						index1 = cChild;
						index2 = cIndex;
					}
				}  // for (cIndex)
			} // for (cChild)

			break;
		default:
			throw Tools::NotSupportedException("Node::pickSeeds: Tree variant not supported.");
	}
}

NodePtr Node::findNode(const TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer)
{
	pathBuffer.push(m_identifier);

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		if (m_pIdentifier[cChild] == id)
			return m_pTree->readNode(m_pIdentifier[cChild]);

		if (m_ptrMBR[cChild]->containsShape(mbr))
		{
			NodePtr n = m_pTree->readNode(m_pIdentifier[cChild]);
			NodePtr l = n->findNode(mbr, id, pathBuffer);
			assert(n.get() != l.get());
			if (l.get() != nullptr) return l;
		}
	}

	pathBuffer.pop();

	return NodePtr();
}
