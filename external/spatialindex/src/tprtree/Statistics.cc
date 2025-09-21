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

using namespace SpatialIndex::TPRTree;

Statistics::Statistics()
{
	reset();
}

Statistics::Statistics(const Statistics& s)
{
	m_reads  = s.m_reads;
	m_writes = s.m_writes;
	m_splits = s.m_splits;
	m_hits   = s.m_hits;
	m_misses = s.m_misses;
	m_nodes  = s.m_nodes;
	m_adjustments = s.m_adjustments;
	m_queryResults = s.m_queryResults;
	m_data = s.m_data;
	m_treeHeight = s.m_treeHeight;
	m_nodesInLevel = s.m_nodesInLevel;
}

Statistics::~Statistics()
= default;

Statistics& Statistics::operator=(const Statistics& s)
{
	if (this != &s)
	{
		m_reads  = s.m_reads;
		m_writes = s.m_writes;
		m_splits = s.m_splits;
		m_hits   = s.m_hits;
		m_misses = s.m_misses;
		m_nodes  = s.m_nodes;
		m_adjustments = s.m_adjustments;
		m_queryResults = s.m_queryResults;
		m_data = s.m_data;
		m_treeHeight = s.m_treeHeight;
		m_nodesInLevel = s.m_nodesInLevel;
	}

	return *this;
}

uint64_t Statistics::getReads() const
{
	return m_reads;
}

uint64_t Statistics::getWrites() const
{
	return m_writes;
}

uint32_t Statistics::getNumberOfNodes() const
{
	return m_nodes;
}

uint64_t Statistics::getNumberOfData() const
{
	return m_data;
}

uint64_t Statistics::getSplits() const
{
	return m_splits;
}

uint64_t Statistics::getHits() const
{
	return m_hits;
}

uint64_t Statistics::getMisses() const
{
	return m_misses;
}

uint64_t Statistics::getAdjustments() const
{
	return m_adjustments;
}

uint64_t Statistics::getQueryResults() const
{
	return m_queryResults;
}

uint32_t Statistics::getTreeHeight() const
{
	return m_treeHeight;
}

uint32_t Statistics::getNumberOfNodesInLevel(uint32_t l) const
{
	uint32_t cNodes;
	try
	{
		cNodes = m_nodesInLevel.at(l);
	}
	catch (...)
	{
		throw Tools::IndexOutOfBoundsException(l);
	}

	return cNodes;
}

void Statistics::reset()
{
	m_reads  = 0;
	m_writes = 0;
	m_splits = 0;
	m_hits   = 0;
	m_misses = 0;
	m_nodes  = 0;
	m_adjustments = 0;
	m_queryResults = 0;
	m_data = 0;
	m_treeHeight = 0;
	m_nodesInLevel.clear();
}

std::ostream& SpatialIndex::TPRTree::operator<<(std::ostream& os, const Statistics& s)
{
	os	<< "Reads: " << s.m_reads << std::endl
		<< "Writes: " << s.m_writes << std::endl
		<< "Hits: " << s.m_hits << std::endl
		<< "Misses: " << s.m_misses << std::endl
		<< "Tree height: " << s.m_treeHeight << std::endl
		<< "Number of data: " << s.m_data << std::endl
		<< "Number of nodes: " << s.m_nodes << std::endl;

	for (uint32_t cLevel = 0; cLevel < s.m_treeHeight; ++cLevel)
	{
		os << "Level " << cLevel << " pages: " << s.m_nodesInLevel[cLevel] << std::endl;
	}

	os	<< "Splits: " << s.m_splits << std::endl
		<< "Adjustments: " << s.m_adjustments << std::endl
		<< "Query results: " << s.m_queryResults << std::endl;

	return os;
}
