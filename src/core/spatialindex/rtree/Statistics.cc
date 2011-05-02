// Spatial Index Library
//
// Copyright (C) 2002 Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include "../spatialindex/SpatialIndexImpl.h"

#include "Statistics.h"

using namespace SpatialIndex::RTree;

Statistics::Statistics()
{
  reset();
}

Statistics::Statistics( const Statistics& s )
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
{
}

Statistics& Statistics::operator=( const Statistics & s )
{
  if ( this != &s )
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

unsigned long Statistics::getReads() const
{
  return m_reads;
}

unsigned long Statistics::getWrites() const
{
  return m_writes;
}

unsigned long Statistics::getNumberOfNodes() const
{
  return m_nodes;
}

unsigned long Statistics::getNumberOfData() const
{
  return m_data;
}

unsigned long Statistics::getSplits() const
{
  return m_splits;
}

unsigned long Statistics::getHits() const
{
  return m_hits;
}

unsigned long Statistics::getMisses() const
{
  return m_misses;
}

unsigned long Statistics::getAdjustments() const
{
  return m_adjustments;
}

unsigned long Statistics::getQueryResults() const
{
  return m_queryResults;
}

unsigned long Statistics::getTreeHeight() const
{
  return m_treeHeight;
}

unsigned long Statistics::getNumberOfNodesInLevel( unsigned long l ) const
{
  unsigned long cNodes;
  try
  {
    cNodes = m_nodesInLevel.at( l );
  }
  catch ( ... )
  {
    throw Tools::IndexOutOfBoundsException( l );
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

std::ostream& SpatialIndex::RTree::operator<<( std::ostream& os, const Statistics& s )
{
  using std::endl;

  os << "Reads: " << s.m_reads << endl
  << "Writes: " << s.m_writes << endl
  << "Hits: " << s.m_hits << endl
  << "Misses: " << s.m_misses << endl
  << "Tree height: " << s.m_treeHeight << endl
  << "Number of data: " << s.m_data << endl
  << "Number of nodes: " << s.m_nodes << endl;

  for ( unsigned long cLevel = 0; cLevel < s.m_treeHeight; cLevel++ )
  {
    os << "Level " << cLevel << " pages: " << s.m_nodesInLevel[cLevel] << endl;
  }

  os << "Splits: " << s.m_splits << endl
  << "Adjustments: " << s.m_adjustments << endl
  << "Query results: " << s.m_queryResults << endl;

  return os;
}

