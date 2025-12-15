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

#include "RTree.h"
#include "Node.h"
#include "Index.h"

using namespace SpatialIndex;
using namespace SpatialIndex::RTree;

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
		(m_children * (m_pTree->m_dimension * sizeof(double) * 2 + sizeof(id_type) + sizeof(uint32_t))) +
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

	for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
	{
		m_ptrMBR[u32Child] = m_pTree->m_regionPool.acquire();
		*(m_ptrMBR[u32Child]) = m_pTree->m_infiniteRegion;

		memcpy(m_ptrMBR[u32Child]->m_pLow, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(m_ptrMBR[u32Child]->m_pHigh, ptr, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(&(m_pIdentifier[u32Child]), ptr, sizeof(id_type));
		ptr += sizeof(id_type);

		memcpy(&(m_pDataLength[u32Child]), ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		if (m_pDataLength[u32Child] > 0)
		{
			m_totalDataLength += m_pDataLength[u32Child];
			m_pData[u32Child] = new uint8_t[m_pDataLength[u32Child]];
			memcpy(m_pData[u32Child], ptr, m_pDataLength[u32Child]);
			ptr += m_pDataLength[u32Child];
		}
		else
		{
			m_pData[u32Child] = nullptr;
		}

		//m_nodeMBR.combineRegion(*(m_ptrMBR[u32Child]));
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

	for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
	{
		memcpy(ptr, m_ptrMBR[u32Child]->m_pLow, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, m_ptrMBR[u32Child]->m_pHigh, m_pTree->m_dimension * sizeof(double));
		ptr += m_pTree->m_dimension * sizeof(double);
		memcpy(ptr, &(m_pIdentifier[u32Child]), sizeof(id_type));
		ptr += sizeof(id_type);

		memcpy(ptr, &(m_pDataLength[u32Child]), sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		if (m_pDataLength[u32Child] > 0)
		{
			memcpy(ptr, m_pData[u32Child], m_pDataLength[u32Child]);
			ptr += m_pDataLength[u32Child];
		}
	}

	// store the node MBR for efficiency. This increases the node size a little bit.
	memcpy(ptr, m_nodeMBR.m_pLow, m_pTree->m_dimension * sizeof(double));
	ptr += m_pTree->m_dimension * sizeof(double);
	memcpy(ptr, m_nodeMBR.m_pHigh, m_pTree->m_dimension * sizeof(double));
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
	*out = new Region(m_nodeMBR);
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

	*out = new Region(*(m_ptrMBR[index]));
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

Node::Node(SpatialIndex::RTree::RTree* pTree, id_type id, uint32_t level, uint32_t capacity) :
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
		m_ptrMBR = new RegionPtr[m_capacity + 1];
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
		for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
		{
			if (m_pData[u32Child] != nullptr) delete[] m_pData[u32Child];
		}

		delete[] m_pData;
	}

	delete[] m_pDataLength;
	delete[] m_ptrMBR;
	delete[] m_pIdentifier;
}

Node& Node::operator=(const Node&)
{
	throw Tools::IllegalStateException("operator =: This should never be called.");
}

void Node::insertEntry(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id)
{
	assert(m_children < m_capacity);

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	m_totalDataLength += dataLength;
	++m_children;

	m_nodeMBR.combineRegion(mbr);
}

void Node::deleteEntry(uint32_t index)
{
	assert(index >= 0 && index < m_children);

	// cache it, since I might need it for "touches" later.
	RegionPtr ptrR = m_ptrMBR[index];

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
	else if (m_pTree->m_bTightMBRs && m_nodeMBR.touchesRegion(*ptrR))
	{
		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[u32Child]->m_pLow[cDim]);
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[u32Child]->m_pHigh[cDim]);
			}
		}
	}
}

bool Node::insertData(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id, std::stack<id_type>& pathBuffer, uint8_t* overflowTable)
{
	if (m_children < m_capacity)
	{
		bool adjusted = false;

		// this has to happen before insertEntry modifies m_nodeMBR.
		bool b = m_nodeMBR.containsRegion(mbr);

		insertEntry(dataLength, pData, mbr, id);
		m_pTree->writeNode(this);

		if ((! b) && (! pathBuffer.empty()))
		{
			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(this, pathBuffer);
			adjusted = true;
		}

		return adjusted;
	}
	else if (m_pTree->m_treeVariant == RV_RSTAR && (! pathBuffer.empty()) && overflowTable[m_level] == 0)
	{
		overflowTable[m_level] = 1;

		std::vector<uint32_t> vReinsert, vKeep;
		reinsertData(dataLength, pData, mbr, id, vReinsert, vKeep);

		uint32_t lReinsert = static_cast<uint32_t>(vReinsert.size());
		uint32_t lKeep = static_cast<uint32_t>(vKeep.size());

		uint8_t** reinsertdata = nullptr;
		RegionPtr* reinsertmbr = nullptr;
		id_type* reinsertid = nullptr;
		uint32_t* reinsertlen = nullptr;
		uint8_t** keepdata = nullptr;
		RegionPtr* keepmbr = nullptr;
		id_type* keepid = nullptr;
		uint32_t* keeplen = nullptr;

		try
		{
			reinsertdata = new uint8_t*[lReinsert];
			reinsertmbr = new RegionPtr[lReinsert];
			reinsertid = new id_type[lReinsert];
			reinsertlen = new uint32_t[lReinsert];

			keepdata = new uint8_t*[m_capacity + 1];
			keepmbr = new RegionPtr[m_capacity + 1];
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

		for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child) m_totalDataLength += m_pDataLength[u32Child];

		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
			m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[u32Child]->m_pLow[cDim]);
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[u32Child]->m_pHigh[cDim]);
			}
		}

		m_pTree->writeNode(this);

		// Divertion from R*-Tree algorithm here. First adjust
		// the path to the root, then start reinserts, to avoid complicated handling
		// of changes to the same node from multiple insertions.
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer, true);

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
			m_pTree->m_stats.m_u32TreeHeight = m_level + 2;
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

void Node::reinsertData(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id, std::vector<uint32_t>& reinsert, std::vector<uint32_t>& keep)
{
	ReinsertEntry** v = new ReinsertEntry*[m_capacity + 1];

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	PointPtr nc = m_pTree->m_pointPool.acquire();
	m_nodeMBR.getCenter(*nc);
	PointPtr c = m_pTree->m_pointPool.acquire();

	for (uint32_t u32Child = 0; u32Child < m_capacity + 1; ++u32Child)
	{
		try
		{
			v[u32Child] = new ReinsertEntry(u32Child, 0.0);
		}
		catch (...)
		{
			for (uint32_t i = 0; i < u32Child; ++i) delete v[i];
			delete[] v;
			throw;
		}

		m_ptrMBR[u32Child]->getCenter(*c);

		// calculate relative distance of every entry from the node MBR (ignore square root.)
		for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
		{
			double d = nc->m_pCoords[cDim] - c->m_pCoords[cDim];
			v[u32Child]->m_dist += d * d;
		}
	}

	// sort by increasing order of distances.
	::qsort(v, m_capacity + 1, sizeof(ReinsertEntry*), ReinsertEntry::compareReinsertEntry);

	uint32_t cReinsert = static_cast<uint32_t>(std::floor((m_capacity + 1) * m_pTree->m_reinsertFactor));

	uint32_t cCount;

	for (cCount = 0; cCount < m_capacity + 1; ++cCount)
	{
		if (cCount < m_capacity + 1 - cReinsert)
		{
			// Keep all but cReinsert nodes
			keep.push_back(v[cCount]->m_index);
		}
		else
		{
			// Remove cReinsert nodes which will be
			// reinserted into the tree. Since our array
			// is already sorted in ascending order this
			// matches the order suggested in the paper.
			reinsert.push_back(v[cCount]->m_index);
		}
		delete v[cCount];
	}

	delete[] v;
}

void Node::rtreeSplit(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2)
{
	uint32_t u32Child;
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
	// m_totalDataLength does not need to be increased here.

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
			for (u32Child = 0; u32Child < m_capacity + 1; ++u32Child)
			{
				if (mask[u32Child] == 0)
				{
					group1.push_back(u32Child);
					mask[u32Child] = 1;
					--cRemaining;
				}
			}
		}
		else if (minimumLoad - group2.size() == cRemaining)
		{
			// all remaining entries must be assigned to group2 to comply with minimun load requirement.
			for (u32Child = 0; u32Child < m_capacity + 1; ++u32Child)
			{
				if (mask[u32Child] == 0)
				{
					group2.push_back(u32Child);
					mask[u32Child] = 1;
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

			for (u32Child = 0; u32Child < m_capacity + 1; ++u32Child)
			{
				if (mask[u32Child] == 0)
				{
					mbr1->getCombinedRegion(*a, *(m_ptrMBR[u32Child]));
					d1 = a->getArea() - a1;
					mbr2->getCombinedRegion(*b, *(m_ptrMBR[u32Child]));
					d2 = b->getArea() - a2;
					d = std::abs(d1 - d2);

					if (d > m)
					{
						m = d;
						md1 = d1; md2 = d2;
						sel = u32Child;
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

void Node::rstarSplit(uint32_t dataLength, uint8_t* pData, Region& mbr, id_type id, std::vector<uint32_t>& group1, std::vector<uint32_t>& group2)
{
	RstarSplitEntry** dataLow = nullptr;
	RstarSplitEntry** dataHigh = nullptr;

	try
	{
		dataLow = new RstarSplitEntry*[m_capacity + 1];
		dataHigh = new RstarSplitEntry*[m_capacity + 1];
	}
	catch (...)
	{
		delete[] dataLow;
		throw;
	}

	m_pDataLength[m_capacity] = dataLength;
	m_pData[m_capacity] = pData;
	m_ptrMBR[m_capacity] = m_pTree->m_regionPool.acquire();
	*(m_ptrMBR[m_capacity]) = mbr;
	m_pIdentifier[m_capacity] = id;
	// m_totalDataLength does not need to be increased here.

	uint32_t nodeSPF = static_cast<uint32_t>(
		std::floor((m_capacity + 1) * m_pTree->m_splitDistributionFactor));
	uint32_t splitDistribution = (m_capacity + 1) - (2 * nodeSPF) + 2;

	uint32_t u32Child = 0, cDim, cIndex;

	for (u32Child = 0; u32Child <= m_capacity; ++u32Child)
	{
		try
		{
			dataLow[u32Child] = new RstarSplitEntry(m_ptrMBR[u32Child].get(), u32Child, 0);
		}
		catch (...)
		{
			for (uint32_t i = 0; i < u32Child; ++i) delete dataLow[i];
			delete[] dataLow;
			delete[] dataHigh;
			throw;
		}

		dataHigh[u32Child] = dataLow[u32Child];
	}

	double minimumMargin = std::numeric_limits<double>::max();
	uint32_t splitAxis = std::numeric_limits<uint32_t>::max();
	uint32_t sortOrder = std::numeric_limits<uint32_t>::max();

	// chooseSplitAxis.
	for (cDim = 0; cDim < m_pTree->m_dimension; ++cDim)
	{
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareLow);
		::qsort(dataHigh, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareHigh);

		// calculate sum of margins and overlap for all distributions.
		double marginl = 0.0;
		double marginh = 0.0;

		Region bbl1, bbl2, bbh1, bbh2;

		for (u32Child = 1; u32Child <= splitDistribution; ++u32Child)
		{
			uint32_t l = nodeSPF - 1 + u32Child;

			bbl1 = *(dataLow[0]->m_pRegion);
			bbh1 = *(dataHigh[0]->m_pRegion);

			for (cIndex = 1; cIndex < l; ++cIndex)
			{
				bbl1.combineRegion(*(dataLow[cIndex]->m_pRegion));
				bbh1.combineRegion(*(dataHigh[cIndex]->m_pRegion));
			}

			bbl2 = *(dataLow[l]->m_pRegion);
			bbh2 = *(dataHigh[l]->m_pRegion);

			for (cIndex = l + 1; cIndex <= m_capacity; ++cIndex)
			{
				bbl2.combineRegion(*(dataLow[cIndex]->m_pRegion));
				bbh2.combineRegion(*(dataHigh[cIndex]->m_pRegion));
			}

			marginl += bbl1.getMargin() + bbl2.getMargin();
			marginh += bbh1.getMargin() + bbh2.getMargin();
		} // for (u32Child)

		double margin = std::min(marginl, marginh);

		// keep minimum margin as split axis.
		if (margin < minimumMargin)
		{
			minimumMargin = margin;
			splitAxis = cDim;
			sortOrder = (marginl < marginh) ? 0 : 1;
		}

		// increase the dimension according to which the data entries should be sorted.
		for (u32Child = 0; u32Child <= m_capacity; ++u32Child)
		{
			dataLow[u32Child]->m_sortDim = cDim + 1;
		}
	} // for (cDim)

	for (u32Child = 0; u32Child <= m_capacity; ++u32Child)
	{
		dataLow[u32Child]->m_sortDim = splitAxis;
	}

	::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), (sortOrder == 0) ? RstarSplitEntry::compareLow : RstarSplitEntry::compareHigh);

	double ma = std::numeric_limits<double>::max();
	double mo = std::numeric_limits<double>::max();
	uint32_t splitPoint = std::numeric_limits<uint32_t>::max();

	Region bb1, bb2;

	for (u32Child = 1; u32Child <= splitDistribution; ++u32Child)
	{
		uint32_t l = nodeSPF - 1 + u32Child;

		bb1 = *(dataLow[0]->m_pRegion);

		for (cIndex = 1; cIndex < l; ++cIndex)
		{
			bb1.combineRegion(*(dataLow[cIndex]->m_pRegion));
		}

		bb2 = *(dataLow[l]->m_pRegion);

		for (cIndex = l + 1; cIndex <= m_capacity; ++cIndex)
		{
			bb2.combineRegion(*(dataLow[cIndex]->m_pRegion));
		}

		double o = bb1.getIntersectingArea(bb2);

		if (o < mo)
		{
			splitPoint = u32Child;
			mo = o;
			ma = bb1.getArea() + bb2.getArea();
		}
		else if (o == mo)
		{
			double a = bb1.getArea() + bb2.getArea();

			if (a < ma)
			{
				splitPoint = u32Child;
				ma = a;
			}
		}
	} // for (u32Child)

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
}

void Node::pickSeeds(uint32_t& index1, uint32_t& index2)
{
	double separation = -std::numeric_limits<double>::max();
	double inefficiency = -std::numeric_limits<double>::max();
	uint32_t cDim, u32Child, cIndex;

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

				for (u32Child = 1; u32Child <= m_capacity; ++u32Child)
				{
					if (m_ptrMBR[u32Child]->m_pLow[cDim] > m_ptrMBR[greatestLower]->m_pLow[cDim]) greatestLower = u32Child;
					if (m_ptrMBR[u32Child]->m_pHigh[cDim] < m_ptrMBR[leastUpper]->m_pHigh[cDim]) leastUpper = u32Child;

					leastLower = std::min(m_ptrMBR[u32Child]->m_pLow[cDim], leastLower);
					greatestUpper = std::max(m_ptrMBR[u32Child]->m_pHigh[cDim], greatestUpper);
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
			for (u32Child = 0; u32Child < m_capacity; ++u32Child)
			{
				double a = m_ptrMBR[u32Child]->getArea();

				for (cIndex = u32Child + 1; cIndex <= m_capacity; ++cIndex)
				{
					// get the combined MBR of those two entries.
					Region r;
					m_ptrMBR[u32Child]->getCombinedRegion(r, *(m_ptrMBR[cIndex]));

					// find the inefficiency of grouping these entries together.
					double d = r.getArea() - a - m_ptrMBR[cIndex]->getArea();

					if (d > inefficiency)
					{
						inefficiency = d;
						index1 = u32Child;
						index2 = cIndex;
					}
				}  // for (cIndex)
			} // for (u32Child)

			break;
		default:
			throw Tools::NotSupportedException("Node::pickSeeds: Tree variant not supported.");
	}
}

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
			m_pTree->m_stats.m_u32TreeHeight -= 1;
			// HACK: pending deleteNode for deleted child will decrease nodesInLevel, later on.
			m_pTree->m_stats.m_nodesInLevel[m_pTree->m_stats.m_u32TreeHeight - 1] = 2;
		}
		else
		{
			// due to data removal.
			if (m_pTree->m_bTightMBRs)
			{
				for (uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim)
				{
					m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
					m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

					for (uint32_t u32Child = 0; u32Child < m_children; ++u32Child)
					{
						m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[u32Child]->m_pLow[cDim]);
						m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[u32Child]->m_pHigh[cDim]);
					}
				}
			}

            // write parent node back to storage.
			m_pTree->writeNode(this);
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
			if (m_pTree->m_bTightMBRs)
			{
				for (uint32_t cDim = 0; cDim < p->m_nodeMBR.m_dimension; ++cDim)
				{
					p->m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
					p->m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

					for (uint32_t u32Child = 0; u32Child < p->m_children; ++u32Child)
					{
						p->m_nodeMBR.m_pLow[cDim] = std::min(p->m_nodeMBR.m_pLow[cDim], p->m_ptrMBR[u32Child]->m_pLow[cDim]);
						p->m_nodeMBR.m_pHigh[cDim] = std::max(p->m_nodeMBR.m_pHigh[cDim], p->m_ptrMBR[u32Child]->m_pHigh[cDim]);
					}
				}
			}
		}

		// write parent node back to storage.
		m_pTree->writeNode(p);

		p->condenseTree(toReinsert, pathBuffer, ptrParent);
	}
}
