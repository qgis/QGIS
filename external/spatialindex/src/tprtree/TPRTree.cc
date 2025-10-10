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
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "TPRTree.h"

#include <cstring>

using namespace SpatialIndex::TPRTree;

SpatialIndex::TPRTree::Data::Data(uint32_t len, uint8_t* pData, MovingRegion& r, id_type id)
	: m_id(id), m_region(r), m_pData(nullptr), m_dataLength(len)
{
	if (m_dataLength > 0)
	{
		m_pData = new uint8_t[m_dataLength];
		memcpy(m_pData, pData, m_dataLength);
	}
}

SpatialIndex::TPRTree::Data::~Data()
{
	delete[] m_pData;
}

SpatialIndex::TPRTree::Data* SpatialIndex::TPRTree::Data::clone()
{
	return new Data(m_dataLength, m_pData, m_region, m_id);
}

SpatialIndex::id_type SpatialIndex::TPRTree::Data::getIdentifier() const
{
	return m_id;
}

void SpatialIndex::TPRTree::Data::getShape(IShape** out) const
{
	*out = new MovingRegion(m_region);
}

void SpatialIndex::TPRTree::Data::getData(uint32_t& len, uint8_t** data) const
{
	len = m_dataLength;
	*data = nullptr;

	if (m_dataLength > 0)
	{
		*data = new uint8_t[m_dataLength];
		memcpy(*data, m_pData, m_dataLength);
	}
}

uint32_t SpatialIndex::TPRTree::Data::getByteArraySize()
{
	return
		sizeof(id_type) +
		sizeof(uint32_t) +
		m_dataLength +
		m_region.getByteArraySize();
}

void SpatialIndex::TPRTree::Data::loadFromByteArray(const uint8_t* ptr)
{
	memcpy(&m_id, ptr, sizeof(id_type));
	ptr += sizeof(id_type);

	delete[] m_pData;
	m_pData = nullptr;

	memcpy(&m_dataLength, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	if (m_dataLength > 0)
	{
		m_pData = new uint8_t[m_dataLength];
		memcpy(m_pData, ptr, m_dataLength);
		ptr += m_dataLength;
	}

	m_region.loadFromByteArray(ptr);
}

void SpatialIndex::TPRTree::Data::storeToByteArray(uint8_t** data, uint32_t& len)
{
	// it is thread safe this way.
	uint32_t regionsize;
	uint8_t* regiondata = nullptr;
	m_region.storeToByteArray(&regiondata, regionsize);

	len = sizeof(id_type) + sizeof(uint32_t) + m_dataLength + regionsize;

	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	memcpy(ptr, &m_id, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_dataLength, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	if (m_dataLength > 0)
	{
		memcpy(ptr, m_pData, m_dataLength);
		ptr += m_dataLength;
	}

	memcpy(ptr, regiondata, regionsize);
	delete[] regiondata;
	// ptr += regionsize;
}

SpatialIndex::ISpatialIndex* SpatialIndex::TPRTree::returnTPRTree(SpatialIndex::IStorageManager& sm, Tools::PropertySet& ps)
{
	SpatialIndex::ISpatialIndex* si = new SpatialIndex::TPRTree::TPRTree(sm, ps);
	return si;
}

SpatialIndex::ISpatialIndex* SpatialIndex::TPRTree::createNewTPRTree(
	SpatialIndex::IStorageManager& sm,
	double fillFactor,
	uint32_t indexCapacity,
	uint32_t leafCapacity,
	uint32_t dimension,
	TPRTreeVariant rv,
	double horizon,
	id_type& indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = fillFactor;
	ps.setProperty("FillFactor", var);

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = horizon;
	ps.setProperty("Horizon", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = indexCapacity;
	ps.setProperty("IndexCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = leafCapacity;
	ps.setProperty("LeafCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = dimension;
	ps.setProperty("Dimension", var);

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = rv;
	ps.setProperty("TreeVariant", var);

	ISpatialIndex* ret = returnTPRTree(sm, ps);

	var.m_varType = Tools::VT_LONGLONG;
	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.llVal;

	return ret;
}

SpatialIndex::ISpatialIndex* SpatialIndex::TPRTree::loadTPRTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = indexIdentifier;
	ps.setProperty("IndexIdentifier", var);

	return returnTPRTree(sm, ps);
}

SpatialIndex::TPRTree::TPRTree::TPRTree(IStorageManager& sm, Tools::PropertySet& ps) :
	m_pStorageManager(&sm),
	m_rootID(StorageManager::NewPage),
	m_headerID(StorageManager::NewPage),
	m_treeVariant(TPRV_RSTAR),
	m_fillFactor(0.7),
	m_indexCapacity(100),
	m_leafCapacity(100),
	m_nearMinimumOverlapFactor(32),
	m_splitDistributionFactor(0.4),
	m_reinsertFactor(0.3),
	m_dimension(2),
	m_bTightMBRs(true),
	m_currentTime(0.0),
	m_horizon(20.0),
	m_pointPool(500),
	m_regionPool(1000),
	m_indexPool(100),
	m_leafPool(100)
{
	Tools::Variant var = ps.getProperty("IndexIdentifier");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType == Tools::VT_LONGLONG) m_headerID = var.m_val.llVal;
		else if (var.m_varType == Tools::VT_LONG) m_headerID = var.m_val.lVal;
			// for backward compatibility only.
		else throw Tools::IllegalArgumentException("TPRTree: Property IndexIdentifier must be Tools::VT_LONGLONG");

		initOld(ps);
	}
	else
	{
		initNew(ps);
		var.m_varType = Tools::VT_LONGLONG;
		var.m_val.llVal = m_headerID;
		ps.setProperty("IndexIdentifier", var);
	}
}

SpatialIndex::TPRTree::TPRTree::~TPRTree()
{
	storeHeader();
}

//
// ISpatialIndex interface
//

void SpatialIndex::TPRTree::TPRTree::insertData(uint32_t len, const uint8_t* pData, const IShape& shape, id_type id)
{
	if (shape.getDimension() != m_dimension) throw Tools::IllegalArgumentException("insertData: Shape has the wrong number of dimensions.");
	const IEvolvingShape* es = dynamic_cast<const IEvolvingShape*>(&shape);
	if (es == nullptr) throw Tools::IllegalArgumentException("insertData: Shape does not support the Tools::IEvolvingShape interface.");
	const Tools::IInterval *pivI  = dynamic_cast<const Tools::IInterval*>(&shape);
	if (pivI == nullptr) throw Tools::IllegalArgumentException("insertData: Shape does not support the Tools::IInterval interface.");

	if (pivI->getLowerBound() < m_currentTime) throw Tools::IllegalArgumentException("insertData: Shape start time is older than tree current time.");

	Region mbr;
	shape.getMBR(mbr);
	Region vbr;
	es->getVMBR(vbr);
	assert(mbr.m_dimension == vbr.m_dimension);

	MovingRegionPtr mr = m_regionPool.acquire();
	mr->makeDimension(mbr.m_dimension);

	memcpy(mr->m_pLow, mbr.m_pLow, mbr.m_dimension * sizeof(double));
	memcpy(mr->m_pHigh, mbr.m_pHigh, mbr.m_dimension * sizeof(double));
	memcpy(mr->m_pVLow, vbr.m_pLow, vbr.m_dimension * sizeof(double));
	memcpy(mr->m_pVHigh, vbr.m_pHigh, vbr.m_dimension * sizeof(double));
	mr->m_startTime = pivI->getLowerBound();
	mr->m_endTime = std::numeric_limits<double>::max();

	uint8_t* buffer = nullptr;

	if (len > 0)
	{
		buffer = new uint8_t[len];
		memcpy(buffer, pData, len);
	}

	m_currentTime = mr->m_startTime;
	insertData_impl(len, buffer, *mr, id);
		// the buffer is stored in the tree. Do not delete here.
}

// shape.m_startTime should be the time when the object was inserted initially.
// shape.m_endTime should be the time of the deletion (current time).
bool SpatialIndex::TPRTree::TPRTree::deleteData(const IShape& shape, id_type id)
{
	if (shape.getDimension() != m_dimension) throw Tools::IllegalArgumentException("insertData: Shape has the wrong number of dimensions.");
	const IEvolvingShape* es = dynamic_cast<const IEvolvingShape*>(&shape);
	if (es == nullptr) throw Tools::IllegalArgumentException("insertData: Shape does not support the Tools::IEvolvingShape interface.");
	const Tools::IInterval *pivI  = dynamic_cast<const Tools::IInterval*>(&shape);
	if (pivI == nullptr) throw Tools::IllegalArgumentException("insertData: Shape does not support the Tools::IInterval interface.");

	Region mbr;
	shape.getMBR(mbr);
	Region vbr;
	es->getVMBR(vbr);
	assert(mbr.m_dimension == vbr.m_dimension);

	MovingRegionPtr mr = m_regionPool.acquire();
	mr->makeDimension(mbr.m_dimension);

	memcpy(mr->m_pLow, mbr.m_pLow, mbr.m_dimension * sizeof(double));
	memcpy(mr->m_pHigh, mbr.m_pHigh, mbr.m_dimension * sizeof(double));
	memcpy(mr->m_pVLow, vbr.m_pLow, vbr.m_dimension * sizeof(double));
	memcpy(mr->m_pVHigh, vbr.m_pHigh, vbr.m_dimension * sizeof(double));
	mr->m_startTime = pivI->getLowerBound();
	mr->m_endTime = std::numeric_limits<double>::max();

	m_currentTime = pivI->getUpperBound();
	bool ret = deleteData_impl(*mr, id);

	return ret;
}

void SpatialIndex::TPRTree::TPRTree::internalNodesQuery(const IShape& /* query */, IVisitor& /* v */)
{
	throw Tools::IllegalStateException("internalNodesQuery: not impelmented yet.");
}

void SpatialIndex::TPRTree::TPRTree::containsWhatQuery(const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("containsWhatQuery: Shape has the wrong number of dimensions.");
	rangeQuery(ContainmentQuery, query, v);
}

void SpatialIndex::TPRTree::TPRTree::intersectsWithQuery(const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("intersectsWithQuery: Shape has the wrong number of dimensions.");
	rangeQuery(IntersectionQuery, query, v);
}

void SpatialIndex::TPRTree::TPRTree::pointLocationQuery(const Point& query, IVisitor& v)
{
	if (query.m_dimension != m_dimension) throw Tools::IllegalArgumentException("pointLocationQuery: Shape has the wrong number of dimensions.");
	Region r(query, query);
	rangeQuery(IntersectionQuery, r, v);
}

void SpatialIndex::TPRTree::TPRTree::nearestNeighborQuery(uint32_t, const IShape&, IVisitor&, INearestNeighborComparator&)
{
	throw Tools::IllegalStateException("nearestNeighborQuery: not impelmented yet.");
}

void SpatialIndex::TPRTree::TPRTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("nearestNeighborQuery: Shape has the wrong number of dimensions.");
	NNComparator nnc;
	nearestNeighborQuery(k, query, v, nnc);
}

void SpatialIndex::TPRTree::TPRTree::selfJoinQuery(const IShape&, IVisitor&)
{
	throw Tools::IllegalStateException("selfJoinQuery: not impelmented yet.");
}

void SpatialIndex::TPRTree::TPRTree::queryStrategy(IQueryStrategy& qs)
{
	id_type next = m_rootID;
	bool hasNext = true;

	while (hasNext)
	{
		NodePtr n = readNode(next);
		qs.getNextEntry(*n, next, hasNext);
	}
}

void SpatialIndex::TPRTree::TPRTree::getIndexProperties(Tools::PropertySet& out) const
{
	Tools::Variant var;

	// dimension
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_dimension;
	out.setProperty("Dimension", var);

	// index capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_indexCapacity;
	out.setProperty("IndexCapacity", var);

	// leaf capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_leafCapacity;
	out.setProperty("LeafCapacity", var);

	// Tree variant
	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = m_treeVariant;
	out.setProperty("TreeVariant", var);

	// fill factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_fillFactor;
	out.setProperty("FillFactor", var);

	// horizon
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_horizon;
	out.setProperty("Horizon", var);

	// near minimum overlap factor
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_nearMinimumOverlapFactor;
	out.setProperty("NearMinimumOverlapFactor", var);

	// split distribution factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_splitDistributionFactor;
	out.setProperty("SplitDistributionFactor", var);

	// reinsert factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_reinsertFactor;
	out.setProperty("ReinsertFactor", var);

	// tight MBRs
	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = m_bTightMBRs;
	out.setProperty("EnsureTightMBRs", var);

	// index pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_indexPool.getCapacity();
	out.setProperty("IndexPoolCapacity", var);

	// leaf pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_leafPool.getCapacity();
	out.setProperty("LeafPoolCapacity", var);

	// region pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_regionPool.getCapacity();
	out.setProperty("RegionPoolCapacity", var);

	// point pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_pointPool.getCapacity();
	out.setProperty("PointPoolCapacity", var);

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = m_headerID;
	out.setProperty("IndexIdentifier", var);

}

void SpatialIndex::TPRTree::TPRTree::addCommand(ICommand* pCommand, CommandType ct)
{
	switch (ct)
	{
		case CT_NODEREAD:
			m_readNodeCommands.push_back(std::shared_ptr<ICommand>(pCommand));
			break;
		case CT_NODEWRITE:
			m_writeNodeCommands.push_back(std::shared_ptr<ICommand>(pCommand));
			break;
		case CT_NODEDELETE:
			m_deleteNodeCommands.push_back(std::shared_ptr<ICommand>(pCommand));
			break;
	}
}

bool SpatialIndex::TPRTree::TPRTree::isIndexValid()
{
	bool ret = true;

	std::stack<ValidateEntry> st;
	NodePtr root = readNode(m_rootID);

	if (root->m_level != m_stats.m_treeHeight - 1)
	{
		std::cerr << "Invalid tree height." << std::endl;
		return false;
	}

	std::map<uint32_t, uint32_t> nodesInLevel;
	nodesInLevel.insert(std::pair<uint32_t, uint32_t>(root->m_level, 1));

	ValidateEntry e(root->m_nodeMBR, root);
	st.push(e);

	while (! st.empty())
	{
		e = st.top(); st.pop();

		MovingRegion tmpRegion;
		tmpRegion = m_infiniteRegion;

		// I have to rely on the parent information here, since none of the node's
		// children might have a reference time equal to their parents (e.g., after
		// a split).
		tmpRegion.m_startTime = e.m_parentMBR.m_startTime;

		for (uint32_t cDim = 0; cDim < tmpRegion.m_dimension; ++cDim)
		{
			tmpRegion.m_pLow[cDim] = std::numeric_limits<double>::max();
			tmpRegion.m_pHigh[cDim] = -std::numeric_limits<double>::max();
			tmpRegion.m_pVLow[cDim] = std::numeric_limits<double>::max();
			tmpRegion.m_pVHigh[cDim] = -std::numeric_limits<double>::max();

			for (uint32_t cChild = 0; cChild < e.m_pNode->m_children; ++cChild)
			{
				tmpRegion.m_pLow[cDim] = std::min(tmpRegion.m_pLow[cDim], e.m_pNode->m_ptrMBR[cChild]->getExtrapolatedLow(cDim, tmpRegion.m_startTime));
				tmpRegion.m_pHigh[cDim] = std::max(tmpRegion.m_pHigh[cDim], e.m_pNode->m_ptrMBR[cChild]->getExtrapolatedHigh(cDim, tmpRegion.m_startTime));
				tmpRegion.m_pVLow[cDim] = std::min(tmpRegion.m_pVLow[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pVLow[cDim]);
				tmpRegion.m_pVHigh[cDim] = std::max(tmpRegion.m_pVHigh[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pVHigh[cDim]);
			}
			tmpRegion.m_pLow[cDim] -= 2.0 * std::numeric_limits<double>::epsilon();
			tmpRegion.m_pHigh[cDim] += 2.0 * std::numeric_limits<double>::epsilon();
		}

		if (! (tmpRegion == e.m_pNode->m_nodeMBR))
		{
			std::cerr << "Invalid parent information." << std::endl;
			ret = false;
		}
		if (! (tmpRegion == e.m_parentMBR))
		{
			std::cerr << "Error in parent." << std::endl;
			ret = false;
		}

		if (e.m_pNode->m_level != 0)
		{
			for (uint32_t cChild = 0; cChild < e.m_pNode->m_children; ++cChild)
			{
				NodePtr ptrN = readNode(e.m_pNode->m_pIdentifier[cChild]);
				ValidateEntry tmpEntry(*(e.m_pNode->m_ptrMBR[cChild]), ptrN);

				auto itNodes = nodesInLevel.find(tmpEntry.m_pNode->m_level);

				if (itNodes == nodesInLevel.end())
				{
					nodesInLevel.insert(std::pair<uint32_t, uint32_t>(tmpEntry.m_pNode->m_level, 1l));
				}
				else
				{
					nodesInLevel[tmpEntry.m_pNode->m_level] = nodesInLevel[tmpEntry.m_pNode->m_level] + 1;
				}

				st.push(tmpEntry);
			}
		}
	}

	uint32_t nodes = 0;
	for (uint32_t cLevel = 0; cLevel < m_stats.m_treeHeight; ++cLevel)
	{
		if (nodesInLevel[cLevel] != m_stats.m_nodesInLevel[cLevel])
		{
			std::cerr << "Invalid nodesInLevel information." << std::endl;
			ret = false;
		}

		nodes += m_stats.m_nodesInLevel[cLevel];
	}

	if (nodes != m_stats.m_nodes)
	{
		std::cerr << "Invalid number of nodes information." << std::endl;
		ret = false;
	}

	return ret;
}

void SpatialIndex::TPRTree::TPRTree::getStatistics(IStatistics** out) const
{
	*out = new Statistics(m_stats);
}

void SpatialIndex::TPRTree::TPRTree::flush()
{
	storeHeader();
}

void SpatialIndex::TPRTree::TPRTree::initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_LONG ||
			(var.m_val.lVal != TPRV_RSTAR))
			throw Tools::IllegalArgumentException("initNew: Property TreeVariant must be Tools::VT_LONG and of TPRTreeVariant type");

		m_treeVariant = static_cast<TPRTreeVariant>(var.m_val.lVal);
	}

	// fill factor
	// it cannot be larger than 50%, since linear and quadratic split algorithms
	// require assigning to both nodes the same number of entries.
	var = ps.getProperty("FillFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property FillFactor must be Tools::VT_DOUBLE and in (0.0, 1.0) for RSTAR");

		m_fillFactor = var.m_val.dblVal;
	}

	// horizon
	var = ps.getProperty("Horizon");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal == std::numeric_limits<double>::max())
			throw Tools::IllegalArgumentException("initNew: Property Horizon must be Tools::VT_DOUBLE and a positive constant");

		m_horizon = var.m_val.dblVal;
	}

	// index capacity
	var = ps.getProperty("IndexCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 4)
			throw Tools::IllegalArgumentException("initNew: Property IndexCapacity must be Tools::VT_ULONG and >= 4");

		m_indexCapacity = var.m_val.ulVal;
	}

	// leaf capacity
	var = ps.getProperty("LeafCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 4)
			throw Tools::IllegalArgumentException("initNew: Property LeafCapacity must be Tools::VT_ULONG and >= 4");

		m_leafCapacity = var.m_val.ulVal;
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_ULONG ||
			var.m_val.ulVal < 1 ||
			var.m_val.ulVal > m_indexCapacity ||
			var.m_val.ulVal > m_leafCapacity)
			throw Tools::IllegalArgumentException("initNew: Property NearMinimumOverlapFactor must be Tools::VT_ULONG and less than both index and leaf capacities");

		m_nearMinimumOverlapFactor = var.m_val.ulVal;
	}

	// split distribution factor
	var = ps.getProperty("SplitDistributionFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property SplitDistributionFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_splitDistributionFactor = var.m_val.dblVal;
	}

	// reinsert factor
	var = ps.getProperty("ReinsertFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property ReinsertFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_reinsertFactor = var.m_val.dblVal;
	}

	// dimension
	var = ps.getProperty("Dimension");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property Dimension must be Tools::VT_ULONG");
		if (var.m_val.ulVal <= 1)
			throw Tools::IllegalArgumentException("initNew: Property Dimension must be greater than 1");

		m_dimension = var.m_val.ulVal;
	}

	// tight MBRs
	var = ps.getProperty("EnsureTightMBRs");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL)
			throw Tools::IllegalArgumentException("initNew: Property EnsureTightMBRs must be Tools::VT_BOOL");

		m_bTightMBRs = var.m_val.blVal;
	}

	// index pool capacity
	var = ps.getProperty("IndexPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property IndexPoolCapacity must be Tools::VT_ULONG");

		m_indexPool.setCapacity(var.m_val.ulVal);
	}

	// leaf pool capacity
	var = ps.getProperty("LeafPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property LeafPoolCapacity must be Tools::VT_ULONG");

		m_leafPool.setCapacity(var.m_val.ulVal);
	}

	// region pool capacity
	var = ps.getProperty("RegionPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property RegionPoolCapacity must be Tools::VT_ULONG");

		m_regionPool.setCapacity(var.m_val.ulVal);
	}

	// point pool capacity
	var = ps.getProperty("PointPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property PointPoolCapacity must be Tools::VT_ULONG");

		m_pointPool.setCapacity(var.m_val.ulVal);
	}

	m_infiniteRegion.makeInfinite(m_dimension);

	m_stats.m_treeHeight = 1;
	m_stats.m_nodesInLevel.push_back(0);

	Leaf root(this, -1);
	m_rootID = writeNode(&root);

	storeHeader();
}

void SpatialIndex::TPRTree::TPRTree::initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_LONG ||
			(var.m_val.lVal != TPRV_RSTAR))
			throw Tools::IllegalArgumentException("initOld: Property TreeVariant must be Tools::VT_LONG and of TPRTreeVariant type");

		m_treeVariant = static_cast<TPRTreeVariant>(var.m_val.lVal);
	}

	// horizon
	var = ps.getProperty("Horizon");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal == std::numeric_limits<double>::max())
			throw Tools::IllegalArgumentException("initOld: Property Horizon must be Tools::VT_DOUBLE and a positive constant");

		m_horizon = var.m_val.dblVal;
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_ULONG ||
			var.m_val.ulVal < 1 ||
			var.m_val.ulVal > m_indexCapacity ||
			var.m_val.ulVal > m_leafCapacity)
			throw Tools::IllegalArgumentException("initOld: Property NearMinimumOverlapFactor must be Tools::VT_ULONG and less than both index and leaf capacities");

		m_nearMinimumOverlapFactor = var.m_val.ulVal;
	}

	// split distribution factor
	var = ps.getProperty("SplitDistributionFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initOld: Property SplitDistributionFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_splitDistributionFactor = var.m_val.dblVal;
	}

	// reinsert factor
	var = ps.getProperty("ReinsertFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initOld: Property ReinsertFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_reinsertFactor = var.m_val.dblVal;
	}

	// tight MBRs
	var = ps.getProperty("EnsureTightMBRs");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL) throw Tools::IllegalArgumentException("initOld: Property EnsureTightMBRs must be Tools::VT_BOOL");

		m_bTightMBRs = var.m_val.blVal;
	}

	// index pool capacity
	var = ps.getProperty("IndexPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property IndexPoolCapacity must be Tools::VT_ULONG");

		m_indexPool.setCapacity(var.m_val.ulVal);
	}

	// leaf pool capacity
	var = ps.getProperty("LeafPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property LeafPoolCapacity must be Tools::VT_ULONG");

		m_leafPool.setCapacity(var.m_val.ulVal);
	}

	// region pool capacity
	var = ps.getProperty("RegionPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property RegionPoolCapacity must be Tools::VT_ULONG");

		m_regionPool.setCapacity(var.m_val.ulVal);
	}

	// point pool capacity
	var = ps.getProperty("PointPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property PointPoolCapacity must be Tools::VT_ULONG");

		m_pointPool.setCapacity(var.m_val.ulVal);
	}

	m_infiniteRegion.makeInfinite(m_dimension);
}

void SpatialIndex::TPRTree::TPRTree::storeHeader()
{
	const uint32_t headerSize =
		sizeof(id_type) +						// m_rootID
		sizeof(TPRTreeVariant) +				// m_treeVariant
		sizeof(double) +						// m_fillFactor
		sizeof(uint32_t) +						// m_indexCapacity
		sizeof(uint32_t) +						// m_leafCapacity
		sizeof(uint32_t) +						// m_nearMinimumOverlapFactor
		sizeof(double) +						// m_splitDistributionFactor
		sizeof(double) +						// m_reinsertFactor
		sizeof(uint32_t) +						// m_dimension
		sizeof(char) +							// m_bTightMBRs
		sizeof(uint32_t) +						// m_stats.m_nodes
		sizeof(uint64_t) +						// m_stats.m_data
		sizeof(double) +						// m_currentTime
		sizeof(double) +						// m_horizon
		sizeof(uint32_t) +						// m_stats.m_treeHeight
		m_stats.m_treeHeight * sizeof(uint32_t);// m_stats.m_nodesInLevel

	uint8_t* header = new uint8_t[headerSize];
	uint8_t* ptr = header;

	memcpy(ptr, &m_rootID, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_treeVariant, sizeof(TPRTreeVariant));
	ptr += sizeof(TPRTreeVariant);
	memcpy(ptr, &m_fillFactor, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_indexCapacity, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &m_leafCapacity, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &m_nearMinimumOverlapFactor, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &m_splitDistributionFactor, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_reinsertFactor, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	char c = (char) m_bTightMBRs;
	memcpy(ptr, &c, sizeof(char));
	ptr += sizeof(char);
	memcpy(ptr, &(m_stats.m_nodes), sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &(m_stats.m_data), sizeof(uint64_t));
	ptr += sizeof(uint64_t);
	memcpy(ptr, &m_currentTime, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_horizon, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &(m_stats.m_treeHeight), sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t cLevel = 0; cLevel < m_stats.m_treeHeight; ++cLevel)
	{
		memcpy(ptr, &(m_stats.m_nodesInLevel[cLevel]), sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	m_pStorageManager->storeByteArray(m_headerID, headerSize, header);

	delete[] header;
}

void SpatialIndex::TPRTree::TPRTree::loadHeader()
{
	uint32_t headerSize;
	uint8_t* header = nullptr;
	m_pStorageManager->loadByteArray(m_headerID, headerSize, &header);

	uint8_t* ptr = header;

	memcpy(&m_rootID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_treeVariant, ptr, sizeof(TPRTreeVariant));
	ptr += sizeof(TPRTreeVariant);
	memcpy(&m_fillFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_indexCapacity, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&m_leafCapacity, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&m_nearMinimumOverlapFactor, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&m_splitDistributionFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_reinsertFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	char c;
	memcpy(&c, ptr, sizeof(char));
	m_bTightMBRs = (c != 0);
	ptr += sizeof(char);
	memcpy(&(m_stats.m_nodes), ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&(m_stats.m_data), ptr, sizeof(uint64_t));
	ptr += sizeof(uint64_t);
	memcpy(&m_currentTime, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_horizon, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&(m_stats.m_treeHeight), ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t cLevel = 0; cLevel < m_stats.m_treeHeight; ++cLevel)
	{
		uint32_t cNodes;
		memcpy(&cNodes, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
		m_stats.m_nodesInLevel.push_back(cNodes);
	}

	delete[] header;
}

void SpatialIndex::TPRTree::TPRTree::insertData_impl(uint32_t dataLength, uint8_t* pData, MovingRegion& mr, id_type id)
{
	assert(mr.getDimension() == m_dimension);
	assert(m_currentTime <= mr.m_startTime);

	std::stack<id_type> pathBuffer;
	uint8_t* overflowTable = nullptr;

	try
	{
		NodePtr root = readNode(m_rootID);

		overflowTable = new uint8_t[root->m_level];
		memset(overflowTable, 0, root->m_level);

		NodePtr l = root->chooseSubtree(mr, 0, pathBuffer);
		if (l.get() == root.get())
		{
			assert(root.unique());
			root.relinquish();
		}
		l->insertData(dataLength, pData, mr, id, pathBuffer, overflowTable);

		delete[] overflowTable;
		++(m_stats.m_data);
	}
	catch (...)
	{
		delete[] overflowTable;
		throw;
	}
}

void SpatialIndex::TPRTree::TPRTree::insertData_impl(uint32_t dataLength, uint8_t* pData, MovingRegion& mr, id_type id, uint32_t level, uint8_t* overflowTable)
{
	assert(mr.getDimension() == m_dimension);

	std::stack<id_type> pathBuffer;
	NodePtr root = readNode(m_rootID);
	NodePtr n = root->chooseSubtree(mr, level, pathBuffer);

	assert(n->m_level == level);

	if (n.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}
	n->insertData(dataLength, pData, mr, id, pathBuffer, overflowTable);
}

bool SpatialIndex::TPRTree::TPRTree::deleteData_impl(const MovingRegion& mr, id_type id)
{
	assert(mr.m_dimension == m_dimension);

	std::stack<id_type> pathBuffer;

	NodePtr root = readNode(m_rootID);
	NodePtr l = root->findLeaf(mr, id, pathBuffer);
	if (l.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}

	if (l.get() != nullptr)
	{
		Leaf* pL = static_cast<Leaf*>(l.get());
		pL->deleteData(id, pathBuffer);
		--(m_stats.m_data);
		return true;
	}

	return false;
}

SpatialIndex::id_type SpatialIndex::TPRTree::TPRTree::writeNode(Node* n)
{
	uint8_t* buffer;
	uint32_t dataLength;
	n->storeToByteArray(&buffer, dataLength);

	id_type page;
	if (n->m_identifier < 0) page = StorageManager::NewPage;
	else page = n->m_identifier;

	try
	{
		m_pStorageManager->storeByteArray(page, dataLength, buffer);
		delete[] buffer;
	}
	catch (InvalidPageException& e)
	{
		delete[] buffer;
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("writeNode: failed with Tools::InvalidPageException");
	}

	if (n->m_identifier < 0)
	{
		n->m_identifier = page;
		++(m_stats.m_nodes);

		m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel[n->m_level] + 1;
	}

	++(m_stats.m_writes);

	for (size_t cIndex = 0; cIndex < m_writeNodeCommands.size(); ++cIndex)
	{
		m_writeNodeCommands[cIndex]->execute(*n);
	}

	return page;
}

SpatialIndex::TPRTree::NodePtr SpatialIndex::TPRTree::TPRTree::readNode(id_type id)
{
	uint32_t dataLength;
	uint8_t* buffer;

	try
	{
		m_pStorageManager->loadByteArray(id, dataLength, &buffer);
	}
	catch (InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("readNode: failed with Tools::InvalidPageException");
	}

	try
	{
		uint32_t nodeType;
		memcpy(&nodeType, buffer, sizeof(uint32_t));

		NodePtr n;

		if (nodeType == PersistentIndex) n = m_indexPool.acquire();
		else if (nodeType == PersistentLeaf) n = m_leafPool.acquire();
		else throw Tools::IllegalStateException("readNode: failed reading the correct node type information");

		if (n.get() == nullptr)
		{
			if (nodeType == PersistentIndex) n = NodePtr(new Index(this, -1, 0), &m_indexPool);
			else if (nodeType == PersistentLeaf) n = NodePtr(new Leaf(this, -1), &m_leafPool);
		}

		//n->m_pTree = this;
		n->m_identifier = id;
		n->loadFromByteArray(buffer);

		++(m_stats.m_reads);

		for (size_t cIndex = 0; cIndex < m_readNodeCommands.size(); ++cIndex)
		{
			m_readNodeCommands[cIndex]->execute(*n);
		}

		delete[] buffer;
		return n;
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}
}

void SpatialIndex::TPRTree::TPRTree::deleteNode(Node* n)
{
	try
	{
		m_pStorageManager->deleteByteArray(n->m_identifier);
	}
	catch (InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("deleteNode: failed with Tools::InvalidPageException");
	}

	--(m_stats.m_nodes);
	m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel[n->m_level] - 1;

	for (size_t cIndex = 0; cIndex < m_deleteNodeCommands.size(); ++cIndex)
	{
		m_deleteNodeCommands[cIndex]->execute(*n);
	}
}

void SpatialIndex::TPRTree::TPRTree::rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v)
{
	const MovingRegion* mr = dynamic_cast<const MovingRegion*>(&query);
	if (mr == nullptr) throw Tools::IllegalArgumentException("rangeQuery: Shape has to be a moving region.");
	if (mr->m_startTime < m_currentTime || mr->m_endTime >= m_currentTime + m_horizon)
		throw Tools::IllegalArgumentException("rangeQuery: Query time interval does not intersect current horizon.");

	std::stack<NodePtr> st;
	NodePtr root = readNode(m_rootID);

	if (root->m_children > 0 && mr->intersectsRegionInTime(root->m_nodeMBR)) st.push(root);

	while (! st.empty())
	{
		NodePtr n = st.top(); st.pop();

		if (n->m_level == 0)
		{
			v.visitNode(*n);

			for (uint32_t cChild = 0; cChild < n->m_children; ++cChild)
			{
				bool b;
				if (type == ContainmentQuery) b = mr->containsRegionInTime(*(n->m_ptrMBR[cChild]));
				else b = mr->intersectsRegionInTime(*(n->m_ptrMBR[cChild]));

				if (b)
				{
					Data data = Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
					v.visitData(data);
					++(m_stats.m_queryResults);
				}
			}
		}
		else
		{
			v.visitNode(*n);

			for (uint32_t cChild = 0; cChild < n->m_children; ++cChild)
			{
				if (mr->intersectsRegionInTime(*(n->m_ptrMBR[cChild]))) st.push(readNode(n->m_pIdentifier[cChild]));
			}
		}
	}
}

std::ostream& SpatialIndex::TPRTree::operator<<(std::ostream& os, const TPRTree& t)
{
	os	<< "Dimension: " << t.m_dimension << std::endl
		<< "Fill factor: " << t.m_fillFactor << std::endl
		<< "Horizon: " << t.m_horizon << std::endl
		<< "Index capacity: " << t.m_indexCapacity << std::endl
		<< "Leaf capacity: " << t.m_leafCapacity << std::endl
		<< "Tight MBRs: " << ((t.m_bTightMBRs) ? "enabled" : "disabled") << std::endl;

	if (t.m_treeVariant == TPRV_RSTAR)
	{
		os	<< "Near minimum overlap factor: " << t.m_nearMinimumOverlapFactor << std::endl
			<< "Reinsert factor: " << t.m_reinsertFactor << std::endl
			<< "Split distribution factor: " << t.m_splitDistributionFactor << std::endl;
	}

	if (t.m_stats.getNumberOfNodesInLevel(0) > 0)
		os	<< "Utilization: " << 100 * t.m_stats.getNumberOfData() / (t.m_stats.getNumberOfNodesInLevel(0) * t.m_leafCapacity) << "%" << std::endl
			<< t.m_stats;

	return os;
}
