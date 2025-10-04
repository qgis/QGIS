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
#include "MVRTree.h"

#include <cstring>

using namespace SpatialIndex::MVRTree;

SpatialIndex::MVRTree::Data::Data(uint32_t len, uint8_t* pData, TimeRegion& r, id_type id)
	: m_id(id), m_region(r), m_pData(nullptr), m_dataLength(len)
{
	if (m_dataLength > 0)
	{
		m_pData = new uint8_t[m_dataLength];
		memcpy(m_pData, pData, m_dataLength);
	}
}

SpatialIndex::MVRTree::Data::~Data()
{
	delete[] m_pData;
}

SpatialIndex::MVRTree::Data* SpatialIndex::MVRTree::Data::clone()
{
	return new Data(m_dataLength, m_pData, m_region, m_id);
}

SpatialIndex::id_type SpatialIndex::MVRTree::Data::getIdentifier() const
{
	return m_id;
}

void SpatialIndex::MVRTree::Data::getShape(IShape** out) const
{
	*out = new TimeRegion(m_region);
}

void SpatialIndex::MVRTree::Data::getData(uint32_t& len, uint8_t** data) const
{
	len = m_dataLength;
	*data = nullptr;

	if (m_dataLength > 0)
	{
		*data = new uint8_t[m_dataLength];
		memcpy(*data, m_pData, m_dataLength);
	}
}

uint32_t SpatialIndex::MVRTree::Data::getByteArraySize()
{
	return
		sizeof(id_type) +
		sizeof(uint32_t) +
		m_dataLength +
		m_region.getByteArraySize();
}

void SpatialIndex::MVRTree::Data::loadFromByteArray(const uint8_t* ptr)
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

void SpatialIndex::MVRTree::Data::storeToByteArray(uint8_t** data, uint32_t& len)
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

SpatialIndex::ISpatialIndex* SpatialIndex::MVRTree::returnMVRTree(SpatialIndex::IStorageManager& sm, Tools::PropertySet& ps)
{
	SpatialIndex::ISpatialIndex* si = new SpatialIndex::MVRTree::MVRTree(sm, ps);
	return si;
}

SpatialIndex::ISpatialIndex* SpatialIndex::MVRTree::createNewMVRTree(
	SpatialIndex::IStorageManager& sm,
	double fillFactor,
	uint32_t indexCapacity,
	uint32_t leafCapacity,
	uint32_t dimension,
	MVRTreeVariant rv,
	id_type& indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = fillFactor;
	ps.setProperty("FillFactor", var);

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

	ISpatialIndex* ret = returnMVRTree(sm, ps);

	var.m_varType = Tools::VT_LONGLONG;
	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.llVal;

	return ret;
}

SpatialIndex::ISpatialIndex* SpatialIndex::MVRTree::loadMVRTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = indexIdentifier;
	ps.setProperty("IndexIdentifier", var);

	return returnMVRTree(sm, ps);
}

SpatialIndex::MVRTree::MVRTree::MVRTree(IStorageManager& sm, Tools::PropertySet& ps) :
	m_pStorageManager(&sm),
	m_headerID(StorageManager::NewPage),
	m_treeVariant(RV_RSTAR),
	m_fillFactor(0.7),
	m_indexCapacity(100),
	m_leafCapacity(100),
	m_nearMinimumOverlapFactor(32),
	m_splitDistributionFactor(0.4),
	m_reinsertFactor(0.3),
	m_strongVersionOverflow(0.8),
	//m_strongVersionUnderflow(0.2),
	m_versionUnderflow(0.3),
	m_dimension(2),
	m_bTightMBRs(true),
	m_bHasVersionCopied(false),
	m_currentTime(0.0),
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
		else throw Tools::IllegalArgumentException("MVRTree: Property IndexIdentifier must be Tools::VT_LONGLONG");

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

SpatialIndex::MVRTree::MVRTree::~MVRTree()
{
	storeHeader();
}

//
// ISpatialIndex interface
//

void SpatialIndex::MVRTree::MVRTree::insertData(uint32_t len, const uint8_t* pData, const IShape& shape, id_type id)
{
	if (shape.getDimension() != m_dimension) throw Tools::IllegalArgumentException("insertData: Shape has the wrong number of dimensions.");
	const Tools::IInterval* ti = dynamic_cast<const Tools::IInterval*>(&shape);
	if (ti == nullptr) throw Tools::IllegalArgumentException("insertData: Shape does not support the Tools::IInterval interface.");
	if (ti->getLowerBound() < m_currentTime) throw Tools::IllegalArgumentException("insertData: Shape start time is older than tree current time.");

	// convert the shape into a TimeRegion (R-Trees index regions only; i.e., approximations of the shapes).
	Region mbrold;
	shape.getMBR(mbrold);

	TimeRegionPtr mbr = m_regionPool.acquire();
	mbr->makeDimension(mbrold.m_dimension);

	memcpy(mbr->m_pLow, mbrold.m_pLow, mbrold.m_dimension * sizeof(double));
	memcpy(mbr->m_pHigh, mbrold.m_pHigh, mbrold.m_dimension * sizeof(double));
	mbr->m_startTime = ti->getLowerBound();
	mbr->m_endTime = std::numeric_limits<double>::max();

	uint8_t* buffer = nullptr;

	if (len > 0)
	{
		buffer = new uint8_t[len];
		memcpy(buffer, pData, len);
	}

	insertData_impl(len, buffer, *mbr, id);
		// the buffer is stored in the tree. Do not delete here.
}

bool SpatialIndex::MVRTree::MVRTree::deleteData(const IShape& shape, id_type id)
{
	if (shape.getDimension() != m_dimension) throw Tools::IllegalArgumentException("deleteData: Shape has the wrong number of dimensions.");
	const Tools::IInterval* ti = dynamic_cast<const Tools::IInterval*>(&shape);
	if (ti == nullptr) throw Tools::IllegalArgumentException("deleteData: Shape does not support the Tools::IInterval interface.");

	Region mbrold;
	shape.getMBR(mbrold);

	TimeRegionPtr mbr = m_regionPool.acquire();
	mbr->makeDimension(mbrold.m_dimension);

	memcpy(mbr->m_pLow, mbrold.m_pLow, mbrold.m_dimension * sizeof(double));
	memcpy(mbr->m_pHigh, mbrold.m_pHigh, mbrold.m_dimension * sizeof(double));
	mbr->m_startTime = ti->getLowerBound();
	mbr->m_endTime = ti->getUpperBound();

	bool ret = deleteData_impl(*mbr, id);

	return ret;
}


void SpatialIndex::MVRTree::MVRTree::internalNodesQuery(const IShape& /* query */, IVisitor& /* v */)
{
	throw Tools::IllegalStateException("internalNodesQuery: not impelmented yet.");
}

void SpatialIndex::MVRTree::MVRTree::containsWhatQuery(const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("containsWhatQuery: Shape has the wrong number of dimensions.");
	rangeQuery(ContainmentQuery, query, v);
}

void SpatialIndex::MVRTree::MVRTree::intersectsWithQuery(const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("intersectsWithQuery: Shape has the wrong number of dimensions.");
	rangeQuery(IntersectionQuery, query, v);
}

void SpatialIndex::MVRTree::MVRTree::pointLocationQuery(const Point& query, IVisitor& v)
{
	if (query.m_dimension != m_dimension) throw Tools::IllegalArgumentException("pointLocationQuery: Shape has the wrong number of dimensions.");
	const Tools::IInterval* ti = dynamic_cast<const Tools::IInterval*>(&query);
	if (ti == nullptr) throw Tools::IllegalArgumentException("pointLocationQuery: Shape does not support the Tools::IInterval interface.");
	TimeRegion r(query, query, *ti);
	rangeQuery(IntersectionQuery, r, v);
}

void SpatialIndex::MVRTree::MVRTree::nearestNeighborQuery(uint32_t, const IShape&, IVisitor&, INearestNeighborComparator&)
{
	throw Tools::IllegalStateException("nearestNeighborQuery: not impelmented yet.");
}

void SpatialIndex::MVRTree::MVRTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	if (query.getDimension() != m_dimension) throw Tools::IllegalArgumentException("nearestNeighborQuery: Shape has the wrong number of dimensions.");
	NNComparator nnc;
	nearestNeighborQuery(k, query, v, nnc);
}

void SpatialIndex::MVRTree::MVRTree::selfJoinQuery(const IShape&, IVisitor&)
{
	throw Tools::IllegalStateException("selfJoinQuery: not impelmented yet.");
}

void SpatialIndex::MVRTree::MVRTree::queryStrategy(IQueryStrategy& qs)
{
	id_type next = m_roots[m_roots.size() - 1].m_id;
	bool hasNext = true;

	while (hasNext)
	{
		NodePtr n = readNode(next);
		qs.getNextEntry(*n, next, hasNext);
	}
}

void SpatialIndex::MVRTree::MVRTree::getIndexProperties(Tools::PropertySet& out) const
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

	// strong version overflow
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_strongVersionOverflow;
	out.setProperty("StrongVersionOverflow", var);

	// strong version underflow
	//var.m_varType = Tools::VT_DOUBLE;
	//var.m_val.dblVal = m_strongVersionUnderflow;
	//out.setProperty("StrongVersionUnderflow", var);

	// weak version underflow
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_versionUnderflow;
	out.setProperty("VersionUnderflow", var);

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = m_headerID;
	out.setProperty("IndexIdentifier", var);
}

void SpatialIndex::MVRTree::MVRTree::addCommand(ICommand* pCommand, CommandType ct)
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

bool SpatialIndex::MVRTree::MVRTree::isIndexValid()
{
	bool ret = true;
	std::stack<ValidateEntry> st;
	std::set<id_type> visitedEntries;
	uint32_t degenerateEntries = 0;

	for (uint32_t cRoot = 0; cRoot < m_roots.size(); ++cRoot)
	{
		NodePtr root = readNode(m_roots[cRoot].m_id);

		if (root->m_level != m_stats.m_treeHeight[cRoot] - 1)
		{
			std::cerr << "Invalid tree height." << std::endl;
			return false;
		}

		ValidateEntry e(0, root->m_nodeMBR, root);
		e.m_bIsDead = (root->m_nodeMBR.m_endTime < std::numeric_limits<double>::max()) ? true : false;
		st.push(e);
	}

	while (! st.empty())
	{
		ValidateEntry e = st.top(); st.pop();

		std::set<id_type>::iterator itSet = visitedEntries.find(e.m_pNode->m_identifier);
		if (itSet == visitedEntries.end())
		{
			visitedEntries.insert(e.m_pNode->m_identifier);
			if (e.m_pNode->m_nodeMBR.m_startTime == e.m_pNode->m_nodeMBR.m_endTime) ++degenerateEntries;
		}

		TimeRegion tmpRegion;
		tmpRegion = m_infiniteRegion;

		for (uint32_t cDim = 0; cDim < tmpRegion.m_dimension; ++cDim)
		{
			for (uint32_t cChild = 0; cChild < e.m_pNode->m_children; ++cChild)
			{
				tmpRegion.m_pLow[cDim] = std::min(tmpRegion.m_pLow[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pLow[cDim]);
				tmpRegion.m_pHigh[cDim] = std::max(tmpRegion.m_pHigh[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pHigh[cDim]);
			}
		}

		tmpRegion.m_startTime = e.m_pNode->m_nodeMBR.m_startTime;
		tmpRegion.m_endTime = e.m_pNode->m_nodeMBR.m_endTime;
		if (! (tmpRegion == e.m_pNode->m_nodeMBR))
		{
			std::cerr << "Invalid parent information." << std::endl;
			ret = false;
		}

		if (! e.m_bIsDead)
		{
			tmpRegion.m_startTime = e.m_parentMBR.m_startTime;
			tmpRegion.m_endTime = e.m_parentMBR.m_endTime;
			if (! (tmpRegion == e.m_parentMBR))
			{
				std::cerr << "Error in parent (Node id: " << e.m_pNode->m_identifier << ", Parent id: " << e.m_parentID << ")." << std::endl;
				ret = false;
			}
		}

		if (e.m_pNode->m_level != 0)
		{
			for (uint32_t cChild = 0; cChild < e.m_pNode->m_children; ++cChild)
			{
				NodePtr ptrN = readNode(e.m_pNode->m_pIdentifier[cChild]);

				bool bIsDead =
					(e.m_pNode->m_ptrMBR[cChild]->m_endTime < std::numeric_limits<double>::max() || e.m_bIsDead) ? true : false;

				// if the parent says that this child is dead, force it dead since
				// this information is not propagated for efficiency and is inconsistent.
				if (bIsDead) ptrN->m_nodeMBR.m_endTime = e.m_pNode->m_ptrMBR[cChild]->m_endTime;

				ValidateEntry tmpEntry(e.m_pNode->m_identifier, *(e.m_pNode->m_ptrMBR[cChild]), ptrN);
				tmpEntry.m_bIsDead = bIsDead;
				st.push(tmpEntry);
			}
		}
	}

	//std::cerr << "Total accessible nodes: " << visitedEntries.size() << std::endl;
	//std::cerr << "Degenerate nodes: " << degenerateEntries << std::endl;

	return ret;
}

void SpatialIndex::MVRTree::MVRTree::getStatistics(IStatistics** out) const
{
	*out = new Statistics(m_stats);
}

void SpatialIndex::MVRTree::MVRTree::flush()
{
	storeHeader();
}

void SpatialIndex::MVRTree::MVRTree::initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG || (var.m_val.lVal != RV_LINEAR && var.m_val.lVal != RV_QUADRATIC && var.m_val.lVal != RV_RSTAR))
			throw Tools::IllegalArgumentException("initNew: Property TreeVariant must be Tools::VT_LONG and of MVRTreeVariant type");

		m_treeVariant = static_cast<MVRTreeVariant>(var.m_val.lVal);
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
			//((m_treeVariant == RV_LINEAR || m_treeVariant == RV_QUADRATIC) && var.m_val.dblVal > 0.5) ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property FillFactor must be Tools::VT_DOUBLE and in (0.0, 1.0) for RSTAR, (0.0, 0.5) for LINEAR and QUADRATIC");

		m_fillFactor = var.m_val.dblVal;
	}

	// index capacity
	var = ps.getProperty("IndexCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 10)
			throw Tools::IllegalArgumentException("initNew: Property IndexCapacity must be Tools::VT_ULONG and >= 10");

		m_indexCapacity = var.m_val.ulVal;
	}

	// leaf capacity
	var = ps.getProperty("LeafCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 10)
			throw Tools::IllegalArgumentException("initNew: Property LeafCapacity must be Tools::VT_ULONG and >= 10");

		m_leafCapacity = var.m_val.ulVal;
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 1 ||	var.m_val.ulVal > m_indexCapacity ||	var.m_val.ulVal > m_leafCapacity)
			throw Tools::IllegalArgumentException("initNew: Property NearMinimumOverlapFactor must be Tools::VT_ULONG and less than both index and leaf capacities");

		m_nearMinimumOverlapFactor = var.m_val.ulVal;
	}

	// split distribution factor
	var = ps.getProperty("SplitDistributionFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property SplitDistributionFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_splitDistributionFactor = var.m_val.dblVal;
	}

	// reinsert factor
	var = ps.getProperty("ReinsertFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property ReinsertFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_reinsertFactor = var.m_val.dblVal;
	}

	// dimension
	var = ps.getProperty("Dimension");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initNew: Property Dimension must be Tools::VT_ULONG");
		if (var.m_val.ulVal <= 1) throw Tools::IllegalArgumentException("initNew: Property Dimension must be greater than 1");

		m_dimension = var.m_val.ulVal;
	}

	// tight MBRs
	var = ps.getProperty("EnsureTightMBRs");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL) throw Tools::IllegalArgumentException("initNew: Property EnsureTightMBRs must be Tools::VT_BOOL");

		m_bTightMBRs = var.m_val.blVal;
	}

	// index pool capacity
	var = ps.getProperty("IndexPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initNew: Property IndexPoolCapacity must be Tools::VT_ULONG");

		m_indexPool.setCapacity(var.m_val.ulVal);
	}

	// leaf pool capacity
	var = ps.getProperty("LeafPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initNew: Property LeafPoolCapacity must be Tools::VT_ULONG");

		m_leafPool.setCapacity(var.m_val.ulVal);
	}

	// region pool capacity
	var = ps.getProperty("RegionPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initNew: Property RegionPoolCapacity must be Tools::VT_ULONG");

		m_regionPool.setCapacity(var.m_val.ulVal);
	}

	// point pool capacity
	var = ps.getProperty("PointPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initNew: Property PointPoolCapacity must be Tools::VT_ULONG");

		m_pointPool.setCapacity(var.m_val.ulVal);
	}

	// strong version overflow
	var = ps.getProperty("StrongVersionOverflow");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property StrongVersionOverflow must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_strongVersionOverflow = var.m_val.dblVal;
	}

	// strong version underflow
	//var = ps.getProperty("StrongVersionUnderflow");
	//if (var.m_varType != Tools::VT_EMPTY)
	//{
	//	if (var.m_varType != Tools::VT_DOUBLE ||
	//			var.m_val.dblVal <= 0.0 ||
	//			var.m_val.dblVal >= 1.0) throw Tools::IllegalArgumentException("Property StrongVersionUnderflow must be Tools::VT_DOUBLE and in (0.0, 1.0)");

	//	m_strongVersionUnderflow = var.m_val.dblVal;
	//}

	// weak version underflow
	var = ps.getProperty("VersionUnderflow");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property VersionUnderflow must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_versionUnderflow = var.m_val.dblVal;
	}

	m_infiniteRegion.makeInfinite(m_dimension);

	m_stats.m_treeHeight.push_back(1);
	m_stats.m_nodesInLevel.push_back(1);

	Leaf root(this, -1);
	root.m_nodeMBR.m_startTime = 0.0;
	root.m_nodeMBR.m_endTime = std::numeric_limits<double>::max();
	writeNode(&root);
	m_roots.emplace_back(root.m_identifier, root.m_nodeMBR.m_startTime, root.m_nodeMBR.m_endTime);

	storeHeader();
}

void SpatialIndex::MVRTree::MVRTree::initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG || (var.m_val.lVal != RV_LINEAR && var.m_val.lVal != RV_QUADRATIC && var.m_val.lVal != RV_RSTAR))
			throw Tools::IllegalArgumentException("initOld: Property TreeVariant must be Tools::VT_LONG and of MVRTreeVariant type");

		m_treeVariant = static_cast<MVRTreeVariant>(var.m_val.lVal);
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 1 || var.m_val.ulVal > m_indexCapacity || var.m_val.ulVal > m_leafCapacity)
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
		if (var.m_varType != Tools::VT_DOUBLE ||var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
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

void SpatialIndex::MVRTree::MVRTree::storeHeader()
{
	const uint32_t headerSize =
		sizeof(uint32_t) +											// size of m_roots
		static_cast<uint32_t>(m_roots.size())
		* (sizeof(id_type) + 2 * sizeof(double)) +					// m_roots
		sizeof(MVRTreeVariant) +									// m_treeVariant
		sizeof(double)+												// m_fillFactor
		sizeof(uint32_t) +											// m_indexCapacity
		sizeof(uint32_t) +											// m_leafCapacity
		sizeof(uint32_t) +											// m_nearMinimumOverlapFactor
		sizeof(double) +											// m_splitDistributionFactor
		sizeof(double) +											// m_reinsertFactor
		sizeof(uint32_t) +											// m_dimension
		sizeof(uint8_t) +												// m_bTightMBRs
		sizeof(uint32_t) +											// m_stats.m_nodes
		sizeof(uint64_t) +											// m_stats.m_totalData
		sizeof(uint32_t) +											// m_stats.m_deadIndexNodes
		sizeof(uint32_t) +											// m_stats.m_deadLeafNodes
		sizeof(uint64_t) +											// m_stats.m_data
		sizeof(uint32_t) +											// size of m_stats.m_treeHeight
		static_cast<uint32_t>(m_stats.m_treeHeight.size())
		* sizeof(uint32_t) +										// m_stats.m_treeHeight
		sizeof(double) +											// m_strongVersionOverflow
		//sizeof(double) +											// m_strongVersionUnderflow
		sizeof(double) +											// m_versionUnderflow
		sizeof(double) +											// m_currentTime
		sizeof(uint32_t) +											// m_nodesInLevel size
		static_cast<uint32_t>(m_stats.m_nodesInLevel.size())
		* sizeof(uint32_t);											// m_nodesInLevel values

	uint8_t* header = new uint8_t[headerSize];
	uint8_t* ptr = header;

	uint32_t u32I = static_cast<uint32_t>(m_roots.size());
	memcpy(ptr, &u32I, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (size_t cIndex = 0; cIndex < m_roots.size(); ++cIndex)
	{
		RootEntry& e = m_roots[cIndex];
		memcpy(ptr, &(e.m_id), sizeof(id_type));
		ptr += sizeof(id_type);
		memcpy(ptr, &(e.m_startTime), sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &(e.m_endTime), sizeof(double));
		ptr += sizeof(double);
	}

	memcpy(ptr, &m_treeVariant, sizeof(MVRTreeVariant));
	ptr += sizeof(MVRTreeVariant);
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
	uint8_t c = (uint8_t) m_bTightMBRs;
	memcpy(ptr, &c, sizeof(uint8_t));
	ptr += sizeof(uint8_t);
	memcpy(ptr, &(m_stats.m_u32Nodes), sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &(m_stats.m_u64TotalData), sizeof(uint64_t));
	ptr += sizeof(uint64_t);
	memcpy(ptr, &(m_stats.m_u32DeadIndexNodes), sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &(m_stats.m_u32DeadLeafNodes), sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &(m_stats.m_u64Data), sizeof(uint64_t));
	ptr += sizeof(uint64_t);

	u32I = static_cast<uint32_t>(m_stats.m_treeHeight.size());
	memcpy(ptr, &u32I, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (size_t cIndex = 0; cIndex < m_stats.m_treeHeight.size(); ++cIndex)
	{
		u32I = m_stats.m_treeHeight[cIndex];
		memcpy(ptr, &u32I, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	memcpy(ptr, &m_strongVersionOverflow, sizeof(double));
	ptr += sizeof(double);
	//memcpy(ptr, &m_strongVersionUnderflow, sizeof(double));
	//ptr += sizeof(double);
	memcpy(ptr, &m_versionUnderflow, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_currentTime, sizeof(double));
	ptr += sizeof(double);

	u32I = static_cast<uint32_t>(m_stats.m_nodesInLevel.size());
	memcpy(ptr, &u32I, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (size_t cLevel = 0; cLevel < m_stats.m_nodesInLevel.size(); ++cLevel)
	{
		u32I = m_stats.m_nodesInLevel[cLevel];
		memcpy(ptr, &u32I, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	m_pStorageManager->storeByteArray(m_headerID, headerSize, header);

	delete[] header;
}

void SpatialIndex::MVRTree::MVRTree::loadHeader()
{
	uint32_t headerSize;
	uint8_t* header = nullptr;
	m_pStorageManager->loadByteArray(m_headerID, headerSize, &header);

	uint8_t* ptr = header;

	uint32_t rootsSize;
	memcpy(&rootsSize, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t cIndex = 0; cIndex < rootsSize; ++cIndex)
	{
		RootEntry e;
		memcpy(&(e.m_id), ptr, sizeof(id_type));
		ptr += sizeof(id_type);
		memcpy(&(e.m_startTime), ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&(e.m_endTime), ptr, sizeof(double));
		ptr += sizeof(double);
		m_roots.push_back(e);
	}

	memcpy(&m_treeVariant, ptr, sizeof(MVRTreeVariant));
	ptr += sizeof(MVRTreeVariant);
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
	uint8_t c;
	memcpy(&c, ptr, sizeof(uint8_t));
	m_bTightMBRs = (c != 0);
	ptr += sizeof(uint8_t);
	memcpy(&(m_stats.m_u32Nodes), ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&(m_stats.m_u64TotalData), ptr, sizeof(uint64_t));
	ptr += sizeof(uint64_t);
	memcpy(&(m_stats.m_u32DeadIndexNodes), ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&(m_stats.m_u32DeadLeafNodes), ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&(m_stats.m_u64Data), ptr, sizeof(uint64_t));
	ptr += sizeof(uint64_t);

	uint32_t treeHeightSize;
	memcpy(&treeHeightSize, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t cIndex = 0; cIndex < treeHeightSize; ++cIndex)
	{
		uint32_t u32I;
		memcpy(&u32I, ptr, sizeof(uint32_t));
		m_stats.m_treeHeight.push_back(u32I);
		ptr += sizeof(uint32_t);
	}

	memcpy(&m_strongVersionOverflow, ptr, sizeof(double));
	ptr += sizeof(double);
	//memcpy(&m_strongVersionUnderflow, ptr, sizeof(double));
	//ptr += sizeof(double);
	memcpy(&m_versionUnderflow, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_currentTime, ptr, sizeof(double));
	ptr += sizeof(double);

	uint32_t nodesInLevelSize;
	memcpy(&nodesInLevelSize, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	for (uint32_t cLevel = 0; cLevel < nodesInLevelSize; ++cLevel)
	{
		uint32_t u32I;
		memcpy(&u32I, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
		m_stats.m_nodesInLevel.push_back(u32I);
	}

	delete[] header;
}

void SpatialIndex::MVRTree::MVRTree::insertData_impl(uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id)
{
	assert(mbr.getDimension() == m_dimension);
	assert(m_currentTime <= mbr.m_startTime);

	std::stack<id_type> pathBuffer;
	m_currentTime = mbr.m_startTime;

	NodePtr root = readNode(m_roots[m_roots.size() - 1].m_id);
	NodePtr l = root->chooseSubtree(mbr, 0, pathBuffer);

	if (l.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}
	l->insertData(dataLength, pData, mbr, id, pathBuffer, m_infiniteRegion, -1, false);

	++(m_stats.m_u64Data);
	++(m_stats.m_u64TotalData);
}

void SpatialIndex::MVRTree::MVRTree::insertData_impl(uint32_t dataLength, uint8_t* pData, TimeRegion& mbr, id_type id, uint32_t level)
{
	assert(mbr.getDimension() == m_dimension);

	std::stack<id_type> pathBuffer;

	NodePtr root = readNode(m_roots[m_roots.size() - 1].m_id);
	NodePtr l = root->chooseSubtree(mbr, level, pathBuffer);

	assert(l->m_level == level);

	if (l.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}
	l->insertData(dataLength, pData, mbr, id, pathBuffer, m_infiniteRegion, -1, false);
}

bool SpatialIndex::MVRTree::MVRTree::deleteData_impl(const TimeRegion& mbr, id_type id)
{
	assert(mbr.m_dimension == m_dimension);

	m_currentTime = mbr.m_endTime;

	std::stack<id_type> pathBuffer;
	NodePtr root = readNode(m_roots[m_roots.size() - 1].m_id);
	NodePtr l = root->findLeaf(mbr, id, pathBuffer);

	if (l.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}

	if (l.get() != nullptr)
	{
		l->deleteData(id, mbr.m_endTime, pathBuffer);
		--(m_stats.m_u64Data);
		return true;
	}

	return false;
}

SpatialIndex::id_type SpatialIndex::MVRTree::MVRTree::writeNode(Node* n)
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
		++(m_stats.m_u32Nodes);
	}

	++(m_stats.m_u64Writes);

	for (size_t cIndex = 0; cIndex < m_writeNodeCommands.size(); ++cIndex)
	{
		m_writeNodeCommands[cIndex]->execute(*n);
	}

	return page;
}

SpatialIndex::MVRTree::NodePtr SpatialIndex::MVRTree::MVRTree::readNode(id_type id)
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

		++(m_stats.m_u64Reads);

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

void SpatialIndex::MVRTree::MVRTree::deleteNode(Node* n)
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

	--(m_stats.m_u32Nodes);

	for (size_t cIndex = 0; cIndex < m_deleteNodeCommands.size(); ++cIndex)
	{
		m_deleteNodeCommands[cIndex]->execute(*n);
	}
}

void SpatialIndex::MVRTree::MVRTree::rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v)
{
	// any shape that implements IInterval and IShape, can be used here.
	// FIXME: I am not using ITimeShape yet, even though I should.

	const Tools::IInterval* ti = dynamic_cast<const Tools::IInterval*>(&query);
	if (ti == nullptr) throw Tools::IllegalArgumentException("rangeQuery: Shape does not support the Tools::IInterval interface.");

	std::set<id_type> visitedNodes;
	std::set<id_type> visitedData;
	std::stack<NodePtr> st;
	std::vector<id_type> ids;
	findRootIdentifiers(*ti, ids);

	for (size_t cRoot = 0; cRoot < ids.size(); ++cRoot)
	{
		NodePtr root = readNode(ids[cRoot]);
		if (root->m_children > 0 && query.intersectsShape(root->m_nodeMBR)) st.push(root);
	}

	while (! st.empty())
	{
		NodePtr n = st.top(); st.pop();
		visitedNodes.insert(n->m_identifier);

		if (n->m_level == 0)
		{
			v.visitNode(*n);

			for (uint32_t cChild = 0; cChild < n->m_children; ++cChild)
			{
				if (visitedData.find(n->m_pIdentifier[cChild]) != visitedData.end()) continue;

				bool b;
				if (type == ContainmentQuery) b = (n->m_ptrMBR[cChild])->intersectsInterval(*ti) && query.containsShape(*(n->m_ptrMBR[cChild]));
				else b = (n->m_ptrMBR[cChild])->intersectsInterval(*ti) && query.intersectsShape(*(n->m_ptrMBR[cChild]));

				if (b)
				{
					visitedData.insert(n->m_pIdentifier[cChild]);
					Data data = Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
					v.visitData(data);
					++(m_stats.m_u64QueryResults);
				}
			}
		}
		else
		{
			v.visitNode(*n);

			for (uint32_t cChild = 0; cChild < n->m_children; ++cChild)
			{
				if (
					visitedNodes.find(n->m_pIdentifier[cChild]) == visitedNodes.end() &&
					n->m_ptrMBR[cChild]->intersectsInterval(*ti) &&
					query.intersectsShape(*(n->m_ptrMBR[cChild])))
					st.push(readNode(n->m_pIdentifier[cChild]));
			}
		}
	}
}

void SpatialIndex::MVRTree::MVRTree::findRootIdentifiers(const Tools::IInterval& ti, std::vector<id_type>& ids)
{
	ids.clear();

	for (size_t cRoot = 0; cRoot < m_roots.size(); ++cRoot)
	{
		RootEntry& e = m_roots[cRoot];
		if (ti.intersectsInterval(Tools::IT_RIGHTOPEN, e.m_startTime, e.m_endTime)) ids.push_back(e.m_id);
	}
}

std::string SpatialIndex::MVRTree::MVRTree::printRootInfo() const
{
	std::ostringstream s;

	for (size_t cRoot = 0; cRoot < m_roots.size(); ++cRoot)
	{
		const RootEntry& e = m_roots[cRoot];

		s << "Root " << cRoot << ":  Start " << e.m_startTime << ", End " << e.m_endTime << std::endl;
	}

	return s.str();
}

std::ostream& SpatialIndex::MVRTree::operator<<(std::ostream& os, const MVRTree& t)
{
	os 	<< "Dimension: " << t.m_dimension << std::endl
		<< "Fill factor: " << t.m_fillFactor << std::endl
		<< "Index capacity: " << t.m_indexCapacity << std::endl
		<< "Leaf capacity: " << t.m_leafCapacity << std::endl
		<< "Tight MBRs: " << ((t.m_bTightMBRs) ? "enabled" : "disabled") << std::endl;

	if (t.m_treeVariant == RV_RSTAR)
	{
		os 	<< "Near minimum overlap factor: " << t.m_nearMinimumOverlapFactor << std::endl
			<< "Reinsert factor: " << t.m_reinsertFactor << std::endl
			<< "Split distribution factor: " << t.m_splitDistributionFactor << std::endl
			<< "Strong version overflow: " << t.m_strongVersionOverflow << std::endl
			//<< "Strong version underflow: " << t.m_strongVersionUnderflow << std::endl
			<< "Weak version underflow: " << t.m_versionUnderflow << std::endl;
	}

	// it is difficult to count utilization
	//os << "Utilization: " << 100 * t.m_stats.m_totalData / (t.m_stats.getNumberOfNodesInLevel(0) * t.m_leafCapacity) << "%" << std::endl

	os << t.m_stats;
	os << t.printRootInfo();

	return os;
}
