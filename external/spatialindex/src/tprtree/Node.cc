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

#include "TPRTree.h"
#include "Node.h"
#include "Index.h"

using namespace SpatialIndex;
using namespace SpatialIndex::TPRTree;

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
		(m_children * (4 * m_pTree->m_dimension * sizeof(double) + sizeof(double) + sizeof(id_type) + sizeof(uint32_t))) +
		m_totalDataLength +
		(4 * m_pTree->m_dimension * sizeof(double)));
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
	m_nodeMBR.m_endTime = std::numeric_limits<double>::max();
	//memcpy(&(m_nodeMBR.m_endTime), ptr, sizeof(double));
	//ptr += sizeof(double);

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		m_ptrMBR[cChild] = m_pTree->m_regionPool.acquire();
		m_ptrMBR[cChild]->makeDimension(m_pTree->m_dimension);

		memcpy(m_ptrMBR[cChild]->m_pLow, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(m_ptrMBR[cChild]->m_pHigh, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(m_ptrMBR[cChild]->m_pVLow, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(m_ptrMBR[cChild]->m_pVHigh, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(&(m_ptrMBR[cChild]->m_startTime), ptr, sizeof(double));
		ptr += sizeof(double);
		m_ptrMBR[cChild]->m_endTime = std::numeric_limits<double>::max();

		memcpy(&(m_pIdentifier[cChild]), ptr, sizeof(id_type));
		ptr += sizeof(id_type);

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
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(m_nodeMBR.m_pVLow, ptr, m_pTree->m_dimension * sizeof(double));
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(m_nodeMBR.m_pVHigh, ptr, m_pTree->m_dimension * sizeof(double));
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

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		memcpy(ptr, m_ptrMBR[cChild]->m_pLow, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, m_ptrMBR[cChild]->m_pHigh, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, m_ptrMBR[cChild]->m_pVLow, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, m_ptrMBR[cChild]->m_pVHigh, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, &(m_ptrMBR[cChild]->m_startTime), sizeof(double));
		ptr += sizeof(double);

		memcpy(ptr, &(m_pIdentifier[cChild]), sizeof(id_type));
		ptr += sizeof(id_type);

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
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(ptr, m_nodeMBR.m_pVLow, m_pTree->m_dimension * sizeof(double));
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(ptr, m_nodeMBR.m_pVHigh, m_pTree->m_dimension * sizeof(double));
	//ptr += m_pTree->m_dimension * sizeof(double);

	assert(len == (ptr - *data) + m_pTree->m_dimension * sizeof(double));
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
	*out = new MovingRegion(m_nodeMBR);
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

	*out = new MovingRegion(*(m_ptrMBR[index]));
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

Node::Node(SpatialIndex::TPRTree::TPRTree* pTree, id_type id, uint32_t level, uint32_t capacity) :
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
		m_pDataLength = new uint32_t[m_capacity + 1];
		m_pData = new uint8_t*[m_capacity + 1];
		m_ptrMBR = new MovingRegionPtr[m_capacity + 1];
		m_pIdentifier = new id_type[m_capacity + 1];
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
	}

	delete[] m_pDataLength;
	delete[] m_ptrMBR;
	delete[] m_pIdentifier;
}

Node& Node::operator=(const Node&)
{
	throw Tools::IllegalStateException("Node::operator =: This should never be called.");
}

bool Node::insertEntry(uint32_t dataLength, uint8_t* pData, MovingRegion& mbr, id_type id)
{
	assert(m_children < m_capacity);

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	m_totalDataLength += dataLength;
	++m_children;

	if (m_nodeMBR.m_startTime != m_pTree->m_currentTime)
	{
		m_nodeMBR.m_startTime = m_pTree->m_currentTime;

		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();
			m_nodeMBR.m_pVLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pVHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < m_children; ++cChild)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->getExtrapolatedLow(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->getExtrapolatedHigh(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pVLow[cDim] = std::min(m_nodeMBR.m_pVLow[cDim], m_ptrMBR[cChild]->m_pVLow[cDim]);
				m_nodeMBR.m_pVHigh[cDim] = std::max(m_nodeMBR.m_pVHigh[cDim], m_ptrMBR[cChild]->m_pVHigh[cDim]);
			}
			m_nodeMBR.m_pLow[cDim] -= 2.0 * std::numeric_limits<double>::epsilon();
			m_nodeMBR.m_pHigh[cDim] += 2.0 * std::numeric_limits<double>::epsilon();
		}
	}
	else if (
		//m_nodeMBR.m_pLow[0] != std::numeric_limits<double>::max() &&
		! m_nodeMBR.containsRegionAfterTime(m_pTree->m_currentTime, mbr))
	{
		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			double l = m_nodeMBR.getExtrapolatedLow(cDim, m_pTree->m_currentTime);
			double rl = mbr.getExtrapolatedLow(cDim, m_pTree->m_currentTime);
			if (rl <= l)
			{
				m_nodeMBR.m_pLow[cDim] = rl - 2.0 * std::numeric_limits<double>::epsilon();
			}

			double h = m_nodeMBR.getExtrapolatedHigh(cDim, m_pTree->m_currentTime);
			double rh = mbr.getExtrapolatedHigh(cDim, m_pTree->m_currentTime);
			if (rh >= h)
			{
				m_nodeMBR.m_pHigh[cDim] = rh + 2.0 * std::numeric_limits<double>::epsilon();
			}

			m_nodeMBR.m_pVLow[cDim] = std::min(m_nodeMBR.m_pVLow[cDim], mbr.m_pVLow[cDim]);
			m_nodeMBR.m_pVHigh[cDim] = std::max(m_nodeMBR.m_pVHigh[cDim], mbr.m_pVHigh[cDim]);
		}
	}

	return true;
}

void Node::deleteEntry(uint32_t index)
{
	assert(index >= 0 && index < m_children);

	// cache it, since I might need it for "touches" later.
	MovingRegionPtr ptrR = m_ptrMBR[index];

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
	}
	else //if (m_pTree->m_bTightMBRs && m_nodeMBR.touchesRegion(*ptrR))
	{
		m_nodeMBR.m_startTime = m_pTree->m_currentTime;

		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();
			m_nodeMBR.m_pVLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pVHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < m_children; ++cChild)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->getExtrapolatedLow(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->getExtrapolatedHigh(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pVLow[cDim] = std::min(m_nodeMBR.m_pVLow[cDim], m_ptrMBR[cChild]->m_pVLow[cDim]);
				m_nodeMBR.m_pVHigh[cDim] = std::max(m_nodeMBR.m_pVHigh[cDim], m_ptrMBR[cChild]->m_pVHigh[cDim]);
			}
			m_nodeMBR.m_pLow[cDim] -= 2.0 * std::numeric_limits<double>::epsilon();
			m_nodeMBR.m_pHigh[cDim] += 2.0 * std::numeric_limits<double>::epsilon();
		}

	}
}

bool Node::insertData(uint32_t dataLength, uint8_t* pData, MovingRegion& mbr, id_type id, std::stack<id_type>& pathBuffer, uint8_t* overflowTable)
{
	if (m_children < m_capacity)
	{
		bool bNeedToAdjust = insertEntry(dataLength, pData, mbr, id);
		m_pTree->writeNode(this);

		if (bNeedToAdjust && ! pathBuffer.empty())
		{
			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(this, pathBuffer);
		}

		return bNeedToAdjust;
	}
	else if (false &&
		     m_pTree->m_treeVariant == TPRV_RSTAR &&
			 !pathBuffer.empty() &&
			 overflowTable[m_level] == 0)
	{
		overflowTable[m_level] = 1;

		std::vector<uint32_t> vReinsert, vKeep;
		reinsertData(dataLength, pData, mbr, id, vReinsert, vKeep);

		uint32_t lReinsert = static_cast<uint32_t>(vReinsert.size());
		uint32_t lKeep = static_cast<uint32_t>(vKeep.size());

		uint8_t** reinsertdata = nullptr;
		MovingRegionPtr* reinsertmbr = nullptr;
		id_type* reinsertid = nullptr;
		uint32_t* reinsertlen = nullptr;
		uint8_t** keepdata = nullptr;
		MovingRegionPtr* keepmbr = nullptr;
		id_type* keepid = nullptr;
		uint32_t* keeplen = nullptr;

		try
		{
			reinsertdata = new uint8_t*[lReinsert];
			reinsertmbr = new MovingRegionPtr[lReinsert];
			reinsertid = new id_type[lReinsert];
			reinsertlen = new uint32_t[lReinsert];

			keepdata = new uint8_t*[m_capacity + 1];
			keepmbr = new MovingRegionPtr[m_capacity + 1];
			keepid = new id_type[m_capacity + 1];
			keeplen = new uint32_t[m_capacity + 1];
		}
		catch (...)
		{
			delete[] reinsertdata;
			delete[] reinsertmbr;
			delete[] reinsertid;
			delete[] reinsertlen;
			delete[] keepdata;
			delete[] keepmbr;
			delete[] keepid;
			delete[] keeplen;
			throw;
		}

		uint32_t cIndex;

		for (cIndex = 0; cIndex < lReinsert; ++cIndex)
		{
			reinsertlen[cIndex] = m_pDataLength[vReinsert[cIndex]];
			reinsertdata[cIndex] = m_pData[vReinsert[cIndex]];
			reinsertmbr[cIndex] = m_ptrMBR[vReinsert[cIndex]];
			reinsertid[cIndex] = m_pIdentifier[vReinsert[cIndex]];
		}

		for (cIndex = 0; cIndex < lKeep; ++cIndex)
		{
			keeplen[cIndex] = m_pDataLength[vKeep[cIndex]];
			keepdata[cIndex] = m_pData[vKeep[cIndex]];
			keepmbr[cIndex] = m_ptrMBR[vKeep[cIndex]];
			keepid[cIndex] = m_pIdentifier[vKeep[cIndex]];
		}

		delete[] m_pDataLength;
		delete[] m_pData;
		delete[] m_ptrMBR;
		delete[] m_pIdentifier;

		m_pDataLength = keeplen;
		m_pData = keepdata;
		m_ptrMBR = keepmbr;
		m_pIdentifier = keepid;
		m_children = lKeep;
		m_totalDataLength = 0;

		for (uint32_t cChild = 0; cChild < m_children; ++cChild) m_totalDataLength += m_pDataLength[cChild];

		m_nodeMBR.m_startTime = m_pTree->m_currentTime;

		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();
			m_nodeMBR.m_pVLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pVHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < m_children; ++cChild)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->getExtrapolatedLow(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->getExtrapolatedHigh(cDim, m_nodeMBR.m_startTime));
				m_nodeMBR.m_pVLow[cDim] = std::min(m_nodeMBR.m_pVLow[cDim], m_ptrMBR[cChild]->m_pVLow[cDim]);
				m_nodeMBR.m_pVHigh[cDim] = std::max(m_nodeMBR.m_pVHigh[cDim], m_ptrMBR[cChild]->m_pVHigh[cDim]);
			}
			m_nodeMBR.m_pLow[cDim] -= 2.0 * std::numeric_limits<double>::epsilon();
			m_nodeMBR.m_pHigh[cDim] += 2.0 * std::numeric_limits<double>::epsilon();
		}

		m_pTree->writeNode(this);

		// Divertion from R*-Tree algorithm here. First adjust
		// the path to the root, then start reinserts, to avoid complicated handling
		// of changes to the same node from multiple insertions.
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);

		for (cIndex = 0; cIndex < lReinsert; ++cIndex)
		{
			m_pTree->insertData_impl(
				reinsertlen[cIndex], reinsertdata[cIndex],
				*(reinsertmbr[cIndex]), reinsertid[cIndex],
				m_level, overflowTable);
		}

		delete[] reinsertdata;
		delete[] reinsertmbr;
		delete[] reinsertid;
		delete[] reinsertlen;

		return true;
	}
	else
	{
		NodePtr n;
		NodePtr nn;
		split(dataLength, pData, mbr, id, n, nn);

		if (pathBuffer.empty())
		{
			n->m_level = m_level;
			nn->m_level = m_level;
			n->m_identifier = -1;
			nn->m_identifier = -1;

			m_pTree->writeNode(n.get());
			m_pTree->writeNode(nn.get());

			NodePtr ptrR = m_pTree->m_indexPool.acquire();
			if (ptrR.get() == nullptr)
			{
				ptrR = NodePtr(new Index(m_pTree, m_pTree->m_rootID, m_level + 1), &(m_pTree->m_indexPool));
			}
			else
			{
				//ptrR->m_pTree = m_pTree;
				ptrR->m_identifier = m_pTree->m_rootID;
				ptrR->m_level = m_level + 1;
				ptrR->m_nodeMBR = m_pTree->m_infiniteRegion;
			}

			ptrR->insertEntry(0, nullptr, n->m_nodeMBR, n->m_identifier);
			ptrR->insertEntry(0, nullptr, nn->m_nodeMBR, nn->m_identifier);

			m_pTree->writeNode(ptrR.get());

			m_pTree->m_stats.m_nodesInLevel[m_level] = 2;
			m_pTree->m_stats.m_nodesInLevel.push_back(1);
			m_pTree->m_stats.m_treeHeight = m_level + 2;
		}
		else
		{
			n->m_level = m_level;
			nn->m_level = m_level;
			n->m_identifier = m_identifier;
			nn->m_identifier = -1;

			m_pTree->writeNode(n.get());
			m_pTree->writeNode(nn.get());

			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(n.get(), nn.get(), pathBuffer, overflowTable);
		}

		return true;
	}
}

void Node::reinsertData(uint32_t dataLength, uint8_t* pData, MovingRegion& mbr, id_type id, std::vector<uint32_t>& reinsert, std::vector<uint32_t>& keep)
{
	ReinsertEntry** v = new ReinsertEntry*[m_capacity + 1];

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	Tools::Interval ivT(m_pTree->m_currentTime, m_pTree->m_currentTime + m_pTree->m_horizon);

	for (uint32_t cChild = 0; cChild < m_capacity + 1; ++cChild)
	{
		try
		{
			v[cChild] = new ReinsertEntry(cChild, 0.0);
		}
		catch (...)
		{
			for (uint32_t i = 0; i < cChild; ++i) delete v[i];
			delete[] v;
			throw;
		}

		v[cChild]->m_dist = m_nodeMBR.getCenterDistanceInTime(ivT, *(m_ptrMBR[cChild]));
	}

	// sort by increasing order of distances.
	::qsort(v, m_capacity + 1, sizeof(ReinsertEntry*), ReinsertEntry::compareReinsertEntry);

	uint32_t cReinsert = static_cast<uint32_t>(std::floor((m_capacity + 1) * m_pTree->m_reinsertFactor));

	uint32_t cCount;

	for (cCount = 0; cCount < cReinsert; ++cCount)
	{
		reinsert.push_back(v[cCount]->m_index);
		delete v[cCount];
	}

	for (cCount = cReinsert; cCount < m_capacity + 1; ++cCount)
	{
		keep.push_back(v[cCount]->m_index);
		delete v[cCount];
	}

	delete[] v;
}

/*
void Node::rtreeSplit(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2)
{
	uint32_t cChild;
	uint32_t minimumLoad = static_cast<uint32_t>(std::floor(m_capacity * m_pTree->m_fillFactor));

	// use this mask array for marking visited entries.
	uint8_t* mask = new uint8_t[m_capacity + 1];
	memset(mask, 0, m_capacity + 1);

	// insert new data in the node for easier manipulation. Data arrays are always
	// by one larger than node capacity.
	m_pDataLength[m_capacity] = dataLength;
	m_pData[m_capacity] = pData;
	m_ptrMBR[m_capacity] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_capacity]) = mbr;
	m_pIdentifier[m_capacity] = id;

	// initialize each group with the seed entries.
	uint32_t seed1, seed2;
	pickSeeds(seed1, seed2);

	group1.push_back(seed1);
	group2.push_back(seed2);

	mask[seed1] = 1;
	mask[seed2] = 1;

	// find MBR of each group.
	RegionPtr mbr1 = m_pTree->m_regionPool.acquire();
	*mbr1 = *(m_ptrMBR[seed1]);
	RegionPtr mbr2 = m_pTree->m_regionPool.acquire();
	*mbr2 = *(m_ptrMBR[seed2]);

	// count how many entries are left unchecked (exclude the seeds here.)
	uint32_t cRemaining = m_capacity + 1 - 2;

	while (cRemaining > 0)
	{
		if (minimumLoad - group1.size() == cRemaining)
		{
			// all remaining entries must be assigned to group1 to comply with minimun load requirement.
			for (cChild = 0; cChild < m_capacity + 1; ++cChild)
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
			for (cChild = 0; cChild < m_capacity + 1; ++cChild)
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
			double a1 = mbr1->getArea();
			double a2 = mbr2->getArea();

			RegionPtr a = m_pTree->m_regionPool.acquire();
			RegionPtr b = m_pTree->m_regionPool.acquire();

			for (cChild = 0; cChild < m_capacity + 1; ++cChild)
			{
				if (mask[cChild] == 0)
				{
					mbr1->getCombinedRegion(*a, *(m_ptrMBR[cChild]));
					d1 = a->getArea() - a1;
					mbr2->getCombinedRegion(*b, *(m_ptrMBR[cChild]));
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
			int32_t group = -1;

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
				mbr1->combineRegion(*(m_ptrMBR[sel]));
			}
			else
			{
				mbr2->combineRegion(*(m_ptrMBR[sel]));
			}
		}
	}

	delete[] mask;
}
*/

void Node::rstarSplit(uint32_t dataLength, uint8_t* pData, MovingRegion& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2)
{
	RstarSplitEntry** dataLow = nullptr;
	RstarSplitEntry** dataHigh = nullptr;
	RstarSplitEntry** dataVLow = nullptr;
	RstarSplitEntry** dataVHigh = nullptr;

	try
	{
		dataLow = new RstarSplitEntry*[m_capacity + 1];
		dataHigh = new RstarSplitEntry*[m_capacity + 1];
		dataVLow = new RstarSplitEntry*[m_capacity + 1];
		dataVHigh = new RstarSplitEntry*[m_capacity + 1];
	}
	catch (...)
	{
		delete[] dataLow;
		delete[] dataHigh;
		delete[] dataVLow;
		delete[] dataVHigh;
		throw;
	}

	m_pDataLength[m_capacity] = dataLength;
	m_pData[m_capacity] = pData;
	m_ptrMBR[m_capacity] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_capacity]) = mbr;
	m_pIdentifier[m_capacity] = id;

	uint32_t nodeSPF = static_cast<uint32_t>(std::floor((m_capacity + 1) * m_pTree->m_splitDistributionFactor));
	uint32_t splitDistribution = (m_capacity + 1) - (2 * nodeSPF) + 2;

	Tools::Interval ivT(m_pTree->m_currentTime, m_pTree->m_currentTime + m_pTree->m_horizon);

	uint32_t cChild = 0, cDim, cIndex;

	for (cChild = 0; cChild <= m_capacity; ++cChild)
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
		dataVLow[cChild] = dataLow[cChild];
		dataVHigh[cChild] = dataLow[cChild];
	}

	double minimumMargin = std::numeric_limits<double>::max();
	uint32_t splitAxis = std::numeric_limits<uint32_t>::max();
	uint32_t sortOrder = std::numeric_limits<uint32_t>::max();

	// chooseSplitAxis.
	for (cDim = 0; cDim < m_pTree->m_dimension; ++cDim)
	{
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareLow);
		::qsort(dataHigh, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareHigh);
		::qsort(dataVLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareVLow);
		::qsort(dataVHigh, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareVHigh);

		// calculate sum of margins and overlap for all distributions.
		double marginl = 0.0;
		double marginh = 0.0;
		double marginvl = 0.0;
		double marginvh = 0.0;

		MovingRegion bbl1, bbl2, bbh1, bbh2;
		MovingRegion bbvl1, bbvl2, bbvh1, bbvh2;

		for (cChild = 1; cChild <= splitDistribution; ++cChild)
		{
			uint32_t l = nodeSPF - 1 + cChild;

			bbl1 = *(dataLow[0]->m_pRegion);
			bbh1 = *(dataHigh[0]->m_pRegion);
			bbvl1 = *(dataVLow[0]->m_pRegion);
			bbvh1 = *(dataVHigh[0]->m_pRegion);

			for (cIndex = 1; cIndex < l; ++cIndex)
			{
				bbl1.combineRegionAfterTime(m_pTree->m_currentTime, *(dataLow[cIndex]->m_pRegion));
				bbh1.combineRegionAfterTime(m_pTree->m_currentTime, *(dataHigh[cIndex]->m_pRegion));
				bbvl1.combineRegionAfterTime(m_pTree->m_currentTime, *(dataVLow[cIndex]->m_pRegion));
				bbvh1.combineRegionAfterTime(m_pTree->m_currentTime, *(dataVHigh[cIndex]->m_pRegion));
			}

			bbl2 = *(dataLow[l]->m_pRegion);
			bbh2 = *(dataHigh[l]->m_pRegion);
			bbvl2 = *(dataVLow[l]->m_pRegion);
			bbvh2 = *(dataVHigh[l]->m_pRegion);

			for (cIndex = l + 1; cIndex <= m_capacity; ++cIndex)
			{
				bbl2.combineRegionAfterTime(m_pTree->m_currentTime, *(dataLow[cIndex]->m_pRegion));
				bbh2.combineRegionAfterTime(m_pTree->m_currentTime, *(dataHigh[cIndex]->m_pRegion));
				bbvl2.combineRegionAfterTime(m_pTree->m_currentTime, *(dataVLow[cIndex]->m_pRegion));
				bbvh2.combineRegionAfterTime(m_pTree->m_currentTime, *(dataVHigh[cIndex]->m_pRegion));
			}

			marginl += bbl1.getProjectedSurfaceAreaInTime(ivT) + bbl2.getProjectedSurfaceAreaInTime(ivT);
			marginh += bbh1.getProjectedSurfaceAreaInTime(ivT) + bbh2.getProjectedSurfaceAreaInTime(ivT);
			marginvl += bbvl1.getProjectedSurfaceAreaInTime(ivT) + bbvl2.getProjectedSurfaceAreaInTime(ivT);
			marginvh += bbvh1.getProjectedSurfaceAreaInTime(ivT) + bbvh2.getProjectedSurfaceAreaInTime(ivT);
		} // for (cChild)

		double margin = std::min(std::min(marginl, marginh), std::min(marginvl, marginvh));

		// keep minimum margin as split axis.
		if (margin < minimumMargin)
		{
			minimumMargin = margin;
			splitAxis = cDim;
			if (marginl < marginh && marginl < marginvl && marginl < marginvh) sortOrder = 0;
			else if (marginh < marginl && marginh < marginvl && marginh < marginvh) sortOrder = 1;
			else if (marginvl < marginl && marginvl < marginh && marginvl < marginvh) sortOrder = 2;
			else if (marginvh < marginl && marginvh < marginh && marginvh < marginvl) sortOrder = 3;
		}

		// increase the dimension according to which the data entries should be sorted.
		for (cChild = 0; cChild <= m_capacity; ++cChild)
		{
			dataLow[cChild]->m_sortDim = cDim + 1;
		}
	} // for (cDim)

	for (cChild = 0; cChild <= m_capacity; ++cChild)
	{
		dataLow[cChild]->m_sortDim = splitAxis;
	}

	if (sortOrder == 0)
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareLow);
	else if (sortOrder == 1)
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareHigh);
	else if (sortOrder == 2)
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareVLow);
	else if (sortOrder == 3)
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareVHigh);

	double ma = std::numeric_limits<double>::max();
	double mo = std::numeric_limits<double>::max();
	uint32_t splitPoint = std::numeric_limits<uint32_t>::max();

	MovingRegion bb1, bb2;

	for (cChild = 1; cChild <= splitDistribution; ++cChild)
	{
		uint32_t l = nodeSPF - 1 + cChild;

		bb1 = *(dataLow[0]->m_pRegion);

		for (cIndex = 1; cIndex < l; ++cIndex)
		{
			bb1.combineRegionAfterTime(m_pTree->m_currentTime, *(dataLow[cIndex]->m_pRegion));
		}

		bb2 = *(dataLow[l]->m_pRegion);

		for (cIndex = l + 1; cIndex <= m_capacity; ++cIndex)
		{
			bb2.combineRegionAfterTime(m_pTree->m_currentTime, *(dataLow[cIndex]->m_pRegion));
		}

		double o = bb1.getIntersectingAreaInTime(ivT, bb2);

		if (o < mo)
		{
			splitPoint = cChild;
			mo = o;
			ma = bb1.getAreaInTime(ivT) + bb2.getAreaInTime(ivT);
		}
		else if (o == mo)
		{
			double a = bb1.getAreaInTime(ivT) + bb2.getAreaInTime(ivT);

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

	for (cIndex = l1; cIndex <= m_capacity; ++cIndex)
	{
		group2.push_back(dataLow[cIndex]->m_index);
		delete dataLow[cIndex];
	}

	delete[] dataLow;
	delete[] dataHigh;
	delete[] dataVLow;
	delete[] dataVHigh;
}

/*
void Node::pickSeeds(uint32_t& index1, uint32_t& index2)
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

				for (cChild = 1; cChild <= m_capacity; ++cChild)
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
			for (cChild = 0; cChild < m_capacity; ++cChild)
			{
				double a = m_ptrMBR[cChild]->getArea();

				for (cIndex = cChild + 1; cIndex <= m_capacity; ++cIndex)
				{
					// get the combined MBR of those two entries.
					Region r;
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
*/

void Node::condenseTree(std::stack<NodePtr>& toReinsert, std::stack<id_type>& pathBuffer, NodePtr& ptrThis)
{
	uint32_t minimumLoad = static_cast<uint32_t>(std::floor(m_capacity * m_pTree->m_fillFactor));

	if (pathBuffer.empty())
	{
		// eliminate root if it has only one child.
		if (m_level != 0 && m_children == 1)
		{
			NodePtr ptrN = m_pTree->readNode(m_pIdentifier[0]);
			m_pTree->deleteNode(ptrN.get());
			ptrN->m_identifier = m_pTree->m_rootID;
			m_pTree->writeNode(ptrN.get());

			m_pTree->m_stats.m_nodesInLevel.pop_back();
			m_pTree->m_stats.m_treeHeight -= 1;
			// HACK: pending deleteNode for deleted child will decrease nodesInLevel, later on.
			m_pTree->m_stats.m_nodesInLevel[m_pTree->m_stats.m_treeHeight - 1] = 2;
		}
	}
	else
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrParent = m_pTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrParent.get());

		// find the entry in the parent, that points to this node.
		uint32_t child;

		for (child = 0; child != p->m_children; ++child)
		{
			if (p->m_pIdentifier[child] == m_identifier) break;
		}

		if (m_children < minimumLoad)
		{
			// used space less than the minimum
			// 1. eliminate node entry from the parent. deleteEntry will fix the parent's MBR.
			p->deleteEntry(child);
			// 2. add this node to the stack in order to reinsert its entries.
			toReinsert.push(ptrThis);
		}
		else
		{
			// adjust the entry in 'p' to contain the new bounding region of this node.
			*(p->m_ptrMBR[child]) = m_nodeMBR;

			// global recalculation necessary since the MBR can only shrink in size,
			// due to data removal.
			//if (m_pTree->m_bTightMBRs)
			//{

				p->m_nodeMBR.m_startTime = m_pTree->m_currentTime;

				for (uint32_t cDim = 0; cDim < p->m_nodeMBR.m_dimension; ++cDim)
				{
					p->m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
					p->m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();
					p->m_nodeMBR.m_pVLow[cDim] = std::numeric_limits<double>::max();
					p->m_nodeMBR.m_pVHigh[cDim] = -std::numeric_limits<double>::max();

					for (uint32_t cChild = 0; cChild < p->m_children; ++cChild)
					{
						p->m_nodeMBR.m_pLow[cDim] = std::min(p->m_nodeMBR.m_pLow[cDim], p->m_ptrMBR[cChild]->getExtrapolatedLow(cDim, m_pTree->m_currentTime));
						p->m_nodeMBR.m_pHigh[cDim] = std::max(p->m_nodeMBR.m_pHigh[cDim], p->m_ptrMBR[cChild]->getExtrapolatedHigh(cDim, m_pTree->m_currentTime));
						p->m_nodeMBR.m_pVLow[cDim] = std::min(p->m_nodeMBR.m_pVLow[cDim], p->m_ptrMBR[cChild]->m_pVLow[cDim]);
						p->m_nodeMBR.m_pVHigh[cDim] = std::max(p->m_nodeMBR.m_pVHigh[cDim], p->m_ptrMBR[cChild]->m_pVHigh[cDim]);
					}
					p->m_nodeMBR.m_pLow[cDim] -= 2.0 * std::numeric_limits<double>::epsilon();
					p->m_nodeMBR.m_pHigh[cDim] += 2.0 * std::numeric_limits<double>::epsilon();
				}
			//}
		}

		// write parent node back to storage.
		m_pTree->writeNode(p);

		p->condenseTree(toReinsert, pathBuffer, ptrParent);
	}
}
