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
#include <spatialindex/SpatialIndex.h>

#include "../rtree/RTree.h"
#include "../mvrtree/MVRTree.h"
#include "../tprtree/TPRTree.h"

SpatialIndex::InvalidPageException::InvalidPageException(id_type id)
{
	std::ostringstream s;
	s << "Unknown page id " << id;
	m_error = s.str();
}

std::string SpatialIndex::InvalidPageException::what()
{
	return "InvalidPageException: " + m_error;
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const ISpatialIndex& i)
{
	const SpatialIndex::RTree::RTree* pRTree = dynamic_cast<const SpatialIndex::RTree::RTree*>(&i);
	if (pRTree != nullptr)
	{
		os << *pRTree;
		return os;
	}

	const SpatialIndex::MVRTree::MVRTree* pMVRTree = dynamic_cast<const SpatialIndex::MVRTree::MVRTree*>(&i);
	if (pMVRTree != nullptr)
	{
		os << *pMVRTree;
		return os;
	}

	const SpatialIndex::TPRTree::TPRTree* pTPRTree = dynamic_cast<const SpatialIndex::TPRTree::TPRTree*>(&i);
	if (pTPRTree != nullptr)
	{
		os << *pTPRTree;
		return os;
	}

	std::cerr << "ISpatialIndex operator<<: Not implemented yet for this index type." << std::endl;
	return os;
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const IStatistics& s)
{
	const SpatialIndex::RTree::Statistics* pRTreeStats = dynamic_cast<const SpatialIndex::RTree::Statistics*>(&s);
	if (pRTreeStats != nullptr)
	{
		os << *pRTreeStats;
		return os;
	}

	const SpatialIndex::MVRTree::Statistics* pMVRTreeStats = dynamic_cast<const SpatialIndex::MVRTree::Statistics*>(&s);
	if (pMVRTreeStats != nullptr)
	{
		os << * pMVRTreeStats;
		return os;
	}

	const SpatialIndex::TPRTree::Statistics* pTPRTreeStats = dynamic_cast<const SpatialIndex::TPRTree::Statistics*>(&s);
	if (pTPRTreeStats != nullptr)
	{
		os << * pTPRTreeStats;
		return os;
	}

	std::cerr << "IStatistics operator<<: Not implemented yet for this index type." << std::endl;
	return os;
}

