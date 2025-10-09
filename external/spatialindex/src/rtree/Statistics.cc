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

#include "Statistics.h"

using namespace SpatialIndex::RTree;

Statistics::Statistics()
{
	reset();
}

Statistics::Statistics(const Statistics& s)
{
	m_u64Reads  = s.m_u64Reads;
	m_u64Writes = s.m_u64Writes;
	m_u64Splits = s.m_u64Splits;
	m_u64Hits   = s.m_u64Hits;
	m_u64Misses = s.m_u64Misses;
	m_u32Nodes  = s.m_u32Nodes;
	m_u64Adjustments = s.m_u64Adjustments;
	m_u64QueryResults = s.m_u64QueryResults;
	m_u64Data = s.m_u64Data;
	m_u32TreeHeight = s.m_u32TreeHeight;
	m_nodesInLevel = s.m_nodesInLevel;
}

Statistics::~Statistics()
= default;

Statistics& Statistics::operator=(const Statistics& s)
{
	if (this != &s)
	{
		m_u64Reads  = s.m_u64Reads;
		m_u64Writes = s.m_u64Writes;
		m_u64Splits = s.m_u64Splits;
		m_u64Hits   = s.m_u64Hits;
		m_u64Misses = s.m_u64Misses;
		m_u32Nodes  = s.m_u32Nodes;
		m_u64Adjustments = s.m_u64Adjustments;
		m_u64QueryResults = s.m_u64QueryResults;
		m_u64Data = s.m_u64Data;
		m_u32TreeHeight = s.m_u32TreeHeight;
		m_nodesInLevel = s.m_nodesInLevel;
	}

	return *this;
}

uint64_t Statistics::getReads() const
{
	return m_u64Reads;
}

uint64_t Statistics::getWrites() const
{
	return m_u64Writes;
}

uint32_t Statistics::getNumberOfNodes() const
{
	return m_u32Nodes;
}

uint64_t Statistics::getNumberOfData() const
{
	return m_u64Data;
}

uint64_t Statistics::getSplits() const
{
	return m_u64Splits;
}

uint64_t Statistics::getHits() const
{
	return m_u64Hits;
}

uint64_t Statistics::getMisses() const
{
	return m_u64Misses;
}

uint64_t Statistics::getAdjustments() const
{
	return m_u64Adjustments;
}

uint64_t Statistics::getQueryResults() const
{
	return m_u64QueryResults;
}

uint32_t Statistics::getTreeHeight() const
{
	return m_u32TreeHeight;
}

uint32_t Statistics::getNumberOfNodesInLevel(uint32_t l) const
{
	uint32_t u32Nodes;
	try
	{
		u32Nodes = m_nodesInLevel.at(l);
	}
	catch (...)
	{
		throw Tools::IndexOutOfBoundsException(l);
	}

	return u32Nodes;
}

void Statistics::reset()
{
	m_u64Reads  = 0;
	m_u64Writes = 0;
	m_u64Splits = 0;
	m_u64Hits   = 0;
	m_u64Misses = 0;
	m_u32Nodes  = 0;
	m_u64Adjustments = 0;
	m_u64QueryResults = 0;
	m_u64Data = 0;
	m_u32TreeHeight = 0;
	m_nodesInLevel.clear();
}

std::ostream& SpatialIndex::RTree::operator<<(std::ostream& os, const Statistics& s)
{
	os	<< "Reads: " << s.m_u64Reads << std::endl
		<< "Writes: " << s.m_u64Writes << std::endl
		<< "Hits: " << s.m_u64Hits << std::endl
		<< "Misses: " << s.m_u64Misses << std::endl
		<< "Tree height: " << s.m_u32TreeHeight << std::endl
		<< "Number of data: " << s.m_u64Data << std::endl
		<< "Number of nodes: " << s.m_u32Nodes << std::endl;

	for (uint32_t u32Level = 0; u32Level < s.m_u32TreeHeight; ++u32Level)
	{
		os << "Level " << u32Level << " pages: " << s.m_nodesInLevel[u32Level] << std::endl;
	}

	os	<< "Splits: " << s.m_u64Splits << std::endl
		<< "Adjustments: " << s.m_u64Adjustments << std::endl
		<< "Query results: " << s.m_u64QueryResults << std::endl;

	return os;
}
