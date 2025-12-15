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

#include <limits>

#include <spatialindex/SpatialIndex.h>

#include "MVRTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"

using namespace SpatialIndex;
using namespace SpatialIndex::MVRTree;

Index::~Index()
= default;

Index::Index(SpatialIndex::MVRTree::MVRTree* pTree, id_type id, uint32_t level) : Node(pTree, id, level, pTree->m_indexCapacity)
{
}

NodePtr Index::chooseSubtree(const TimeRegion& mbr, uint32_t insertionLevel, std::stack<id_type>& pathBuffer)
{
	if (m_level == insertionLevel) return NodePtr(this, &(m_pTree->m_indexPool));

	pathBuffer.push(m_identifier);

	uint32_t child = 0;

	switch (m_pTree->m_treeVariant)
	{
		case RV_LINEAR:
		case RV_QUADRATIC:
			child = findLeastEnlargement(mbr);
			break;
		case RV_RSTAR:
			if (m_level == 1)
			{
				// if this node points to leaves...
				child = findLeastOverlap(mbr);
			}
			else
			{
				child = findLeastEnlargement(mbr);
			}
		break;
		default:
			throw Tools::NotSupportedException("Index::chooseSubtree: Tree variant not supported.");
	}
	assert (child != std::numeric_limits<uint32_t>::max());

	NodePtr n = m_pTree->readNode(m_pIdentifier[child]);
	NodePtr ret = n->chooseSubtree(mbr, insertionLevel, pathBuffer);
	assert(n.unique());
	if (ret.get() == n.get()) n.relinquish();

	return ret;
}

NodePtr Index::findLeaf(const TimeRegion& mbr, id_type id, std::stack<id_type>& pathBuffer)
{
	pathBuffer.push(m_identifier);

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		// check live nodes only.
		if (m_ptrMBR[cChild]->m_endTime < std::numeric_limits<double>::max()) continue;
		//if (m_ptrMBR[cChild]->m_endTime < std::numeric_limits<double>::max() ||
		//		m_ptrMBR[cChild]->m_startTime > mbr.m_startTime) continue;

		if (m_ptrMBR[cChild]->containsRegion(mbr))
		{
			NodePtr n = m_pTree->readNode(m_pIdentifier[cChild]);
			NodePtr l = n->findLeaf(mbr, id, pathBuffer);
			if (n.get() == l.get()) n.relinquish();
			if (l.get() != nullptr) return l;
		}
	}

	pathBuffer.pop();

	return NodePtr();
}

void Index::split(
	uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, NodePtr& pLeft, NodePtr& pRight,
	TimeRegion& mbr2, id_type id2, bool bInsertMbr2)
{
	++(m_pTree->m_stats.m_u64Splits);

	std::vector<uint32_t> g1, g2;

	switch (m_pTree->m_treeVariant)
	{
		case RV_LINEAR:
		case RV_QUADRATIC:
			rtreeSplit(dataLength, pData, mbr, id, g1, g2, mbr2, id2, bInsertMbr2);
			break;
		case RV_RSTAR:
			rstarSplit(dataLength, pData, mbr, id, g1, g2, mbr2, id2, bInsertMbr2);
			break;
		default:
			throw Tools::NotSupportedException("Index::split: Tree variant not supported.");
	}

	pLeft = m_pTree->m_indexPool.acquire();
	pRight = m_pTree->m_indexPool.acquire();

	if (pLeft.get() == nullptr) pLeft = NodePtr(new Index(m_pTree, m_identifier, m_level), &(m_pTree->m_indexPool));
	if (pRight.get() == nullptr) pRight = NodePtr(new Index(m_pTree, -1, m_level), &(m_pTree->m_indexPool));

	pLeft->m_nodeMBR = m_pTree->m_infiniteRegion;
	pRight->m_nodeMBR = m_pTree->m_infiniteRegion;

	uint32_t cIndex;

	for (cIndex = 0; cIndex < g1.size(); ++cIndex)
	{
		pLeft->insertEntry(0, nullptr, *(m_ptrMBR[g1[cIndex]]), m_pIdentifier[g1[cIndex]]);
	}

	for (cIndex = 0; cIndex < g2.size(); ++cIndex)
	{
		pRight->insertEntry(0, nullptr, *(m_ptrMBR[g2[cIndex]]), m_pIdentifier[g2[cIndex]]);
	}
}

uint32_t Index::findLeastEnlargement(const TimeRegion& r) const
{
	double area = std::numeric_limits<double>::max();
	uint32_t best = std::numeric_limits<uint32_t>::max();

	TimeRegionPtr t = m_pTree->m_regionPool.acquire();

	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		// if this child is already dead do not consider it.
		if (m_ptrMBR[cChild]->m_endTime <= r.m_startTime) continue;

		m_ptrMBR[cChild]->getCombinedRegion(*t, r);

		double a = m_ptrMBR[cChild]->getArea();
		double enl = t->getArea() - a;

		if (enl < area)
		{
			area = enl;
			best = cChild;
		}
		else if (
				enl > area - std::numeric_limits<double>::epsilon() &&
				enl < area + std::numeric_limits<double>::epsilon())
		{
			if (a < m_ptrMBR[best]->getArea()) best = cChild;
		}
	}

	return best;
}

uint32_t Index::findLeastOverlap(const TimeRegion& r) const
{
	OverlapEntry** entries = new OverlapEntry*[m_children];

	double leastOverlap = std::numeric_limits<double>::max();
	double me = std::numeric_limits<double>::max();
	OverlapEntry* best = nullptr;
	uint32_t cLiveEntries = 0;

	// find combined region and enlargement of every entry and store it.
	for (uint32_t cChild = 0; cChild < m_children; ++cChild)
	{
		if (m_ptrMBR[cChild]->m_endTime <= r.m_startTime) continue;

		try
		{
			entries[cLiveEntries] = new OverlapEntry();
		}
		catch (...)
		{
			for (uint32_t i = 0; i < cLiveEntries; ++i) delete entries[i];
			delete[] entries;
			throw;
		}

		entries[cLiveEntries]->m_index = cChild;
		entries[cLiveEntries]->m_original = m_ptrMBR[cChild];
		entries[cLiveEntries]->m_combined = m_pTree->m_regionPool.acquire();
		m_ptrMBR[cChild]->getCombinedRegion(*(entries[cLiveEntries]->m_combined), r);
		entries[cLiveEntries]->m_oa = entries[cLiveEntries]->m_original->getArea();
		entries[cLiveEntries]->m_ca = entries[cLiveEntries]->m_combined->getArea();
		entries[cLiveEntries]->m_enlargement = entries[cLiveEntries]->m_ca - entries[cLiveEntries]->m_oa;

		if (entries[cLiveEntries]->m_enlargement < me)
		{
			me = entries[cLiveEntries]->m_enlargement;
			best = entries[cLiveEntries];
		}
		else if (entries[cLiveEntries]->m_enlargement == me && entries[cLiveEntries]->m_oa < best->m_oa)
		{
			best = entries[cLiveEntries];
		}
		++cLiveEntries;
	}

	if (me < -std::numeric_limits<double>::epsilon() || me > std::numeric_limits<double>::epsilon())
	{
		uint32_t cIterations;

		if (cLiveEntries > m_pTree->m_nearMinimumOverlapFactor)
		{
			// sort entries in increasing order of enlargement.
			::qsort(entries, cLiveEntries,
							sizeof(OverlapEntry*),
							OverlapEntry::compareEntries);
			assert(entries[0]->m_enlargement <= entries[m_children - 1]->m_enlargement);

			cIterations = m_pTree->m_nearMinimumOverlapFactor;
		}
		else
		{
			cIterations = cLiveEntries;
		}

		// calculate overlap of most important original entries (near minimum overlap cost).
		for (uint32_t cIndex = 0; cIndex < cIterations; ++cIndex)
		{
			double dif = 0.0;
			OverlapEntry* e = entries[cIndex];

			for (uint32_t cChild = 0; cChild < cLiveEntries; ++cChild)
			{
				if (cIndex != cChild)
				{
					double f = e->m_combined->getIntersectingArea(*(entries[cChild]->m_original));
					if (f != 0.0) dif += f - e->m_original->getIntersectingArea(*(entries[cChild]->m_original));
				}
			} // for (cChild)

			if (dif < leastOverlap)
			{
				leastOverlap = dif;
				best = e;
			}
			else if (dif == leastOverlap)
			{
				if (e->m_enlargement == best->m_enlargement)
				{
					// keep the one with least area.
					if (e->m_original->getArea() < best->m_original->getArea()) best = e;
				}
				else
				{
					// keep the one with least enlargement.
					if (e->m_enlargement < best->m_enlargement) best = e;
				}
			}
		} // for (cIndex)
	}

	uint32_t ret = best->m_index;

	for (uint32_t cChild = 0; cChild < cLiveEntries; ++cChild)
	{
		delete entries[cChild];
	}
	delete[] entries;

	return ret;
}

void Index::adjustTree(Node* n, std::stack<id_type>& pathBuffer)
{
	++(m_pTree->m_stats.m_u64Adjustments);

	// find entry pointing to old node;
	uint32_t child;
	for (child = 0; child < m_children; ++child)
	{
		if (m_pIdentifier[child] == n->m_identifier) break;
	}

	// MBR needs recalculation if either:
	//   1. the NEW child MBR is not contained.
	//   2. the OLD child MBR is touching.
	bool bContained = m_nodeMBR.containsRegion(n->m_nodeMBR);
	bool bTouches = m_nodeMBR.touchesRegion(*(m_ptrMBR[child]));
	bool bRecompute = (! bContained || (bTouches && m_pTree->m_bTightMBRs));

	// we should not adjust time here
	double st = m_ptrMBR[child]->m_startTime;
	double en = m_ptrMBR[child]->m_endTime;
	*(m_ptrMBR[child]) = n->m_nodeMBR;
	m_ptrMBR[child]->m_startTime = st;
	m_ptrMBR[child]->m_endTime = en;

	if (bRecompute)
	{
		// no need to update times here. The inserted MBR is younger than all nodes.

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
	}

	m_pTree->writeNode(this);

	if (bRecompute && (! pathBuffer.empty()))
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);
	}
}

void Index::adjustTree(Node* n, Node* nn, std::stack<id_type>& pathBuffer)
{
	++(m_pTree->m_stats.m_u64Adjustments);

	// find entry pointing to old node;
	uint32_t child, child2 = m_capacity;
	for (child = 0; child < m_children; ++child)
	{
		if (m_pIdentifier[child] == nn->m_identifier) child2 = child;
		if (m_pIdentifier[child] == n->m_identifier) break;
	}

	if (child2 == m_capacity)
	{
		for (child2 = child + 1; child2 < m_children; ++child2)
		{
			if (m_pIdentifier[child2] == nn->m_identifier) break;
		}
	}

	// MBR needs recalculation if either:
	//   1. the NEW child MBR is not contained.
	//   2. the OLD child MBR is touching.
	//   3. the SIBLING MBR is touching.
	bool b1 = m_nodeMBR.containsRegion(n->m_nodeMBR);
	bool b2 = m_nodeMBR.touchesRegion(*(m_ptrMBR[child]));
	bool b3 = m_nodeMBR.touchesRegion(*(m_ptrMBR[child2]));
	bool bRecompute = (! b1) || ((b2 || b3) && m_pTree->m_bTightMBRs);

	// we should not adjust time here
	double st = m_ptrMBR[child]->m_startTime;
	double en = m_ptrMBR[child]->m_endTime;
	*(m_ptrMBR[child]) = n->m_nodeMBR;
	m_ptrMBR[child]->m_startTime = st;
	m_ptrMBR[child]->m_endTime = en;

	st = m_ptrMBR[child2]->m_startTime;
	en = m_ptrMBR[child2]->m_endTime;
	*(m_ptrMBR[child2]) = nn->m_nodeMBR;
	m_ptrMBR[child2]->m_startTime = st;
	m_ptrMBR[child2]->m_endTime = en;

	if (bRecompute)
	{
		// no need to update times here. The inserted MBR is younger than all nodes.

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
	}

	m_pTree->writeNode(this);

	if (bRecompute && (! pathBuffer.empty()))
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);
	}
}
