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

#include <limits>

#include "../spatialindex/SpatialIndexImpl.h"
#include "RTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"

using namespace SpatialIndex::RTree;

Index::~Index()
{
}

Index::Index( SpatialIndex::RTree::RTree* pTree, id_type id, uint32_t level ) : Node( pTree, id, level, pTree->m_indexCapacity )
{
}

NodePtr Index::chooseSubtree( const Region& mbr, uint32_t insertionLevel, std::stack<id_type>& pathBuffer )
{
  if ( m_level == insertionLevel ) return NodePtr( this, &( m_pTree->m_indexPool ) );

  pathBuffer.push( m_identifier );

  uint32_t child = 0;

  switch ( m_pTree->m_treeVariant )
  {
    case RV_LINEAR:
    case RV_QUADRATIC:
      child = findLeastEnlargement( mbr );
      break;
    case RV_RSTAR:
      if ( m_level == 1 )
      {
        // if this node points to leaves...
        child = findLeastOverlap( mbr );
      }
      else
      {
        child = findLeastEnlargement( mbr );
      }
      break;
    default:
      throw Tools::NotSupportedException( "Index::chooseSubtree: Tree variant not supported." );
  }
  assert( child != std::numeric_limits<uint32_t>::max() );

  NodePtr n = m_pTree->readNode( m_pIdentifier[child] );
  NodePtr ret = n->chooseSubtree( mbr, insertionLevel, pathBuffer );
  assert( n.unique() );
  if ( ret.get() == n.get() ) n.relinquish();

  return ret;
}

NodePtr Index::findLeaf( const Region& mbr, id_type id, std::stack<id_type>& pathBuffer )
{
  pathBuffer.push( m_identifier );

  for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
  {
    if ( m_ptrMBR[cChild]->containsRegion( mbr ) )
    {
      NodePtr n = m_pTree->readNode( m_pIdentifier[cChild] );
      NodePtr l = n->findLeaf( mbr, id, pathBuffer );
      if ( n.get() == l.get() ) n.relinquish();
      if ( l.get() != 0 ) return l;
    }
  }

  pathBuffer.pop();

  return NodePtr();
}

void Index::split( uint32_t dataLength, byte* pData, Region& mbr, id_type id, NodePtr& ptrLeft, NodePtr& ptrRight )
{
  ++( m_pTree->m_stats.m_u64Splits );

  std::vector<uint32_t> g1, g2;

  switch ( m_pTree->m_treeVariant )
  {
    case RV_LINEAR:
    case RV_QUADRATIC:
      rtreeSplit( dataLength, pData, mbr, id, g1, g2 );
      break;
    case RV_RSTAR:
      rstarSplit( dataLength, pData, mbr, id, g1, g2 );
      break;
    default:
      throw Tools::NotSupportedException( "Index::split: Tree variant not supported." );
  }

  ptrLeft = m_pTree->m_indexPool.acquire();
  ptrRight = m_pTree->m_indexPool.acquire();

  if ( ptrLeft.get() == 0 ) ptrLeft = NodePtr( new Index( m_pTree, m_identifier, m_level ), &( m_pTree->m_indexPool ) );
  if ( ptrRight.get() == 0 ) ptrRight = NodePtr( new Index( m_pTree, -1, m_level ), &( m_pTree->m_indexPool ) );

  ptrLeft->m_nodeMBR = m_pTree->m_infiniteRegion;
  ptrRight->m_nodeMBR = m_pTree->m_infiniteRegion;

  uint32_t cIndex;

  for ( cIndex = 0; cIndex < g1.size(); ++cIndex )
  {
    ptrLeft->insertEntry( 0, 0, *( m_ptrMBR[g1[cIndex]] ), m_pIdentifier[g1[cIndex]] );
  }

  for ( cIndex = 0; cIndex < g2.size(); ++cIndex )
  {
    ptrRight->insertEntry( 0, 0, *( m_ptrMBR[g2[cIndex]] ), m_pIdentifier[g2[cIndex]] );
  }
}

uint32_t Index::findLeastEnlargement( const Region& r ) const
{
  double area = std::numeric_limits<double>::max();
  uint32_t best = std::numeric_limits<uint32_t>::max();

  RegionPtr t = m_pTree->m_regionPool.acquire();

  for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
  {
    m_ptrMBR[cChild]->getCombinedRegion( *t, r );

    double a = m_ptrMBR[cChild]->getArea();
    double enl = t->getArea() - a;

    if ( enl < area )
    {
      area = enl;
      best = cChild;
    }
    else if ( enl == area )
    {
      // this will rarely happen, so compute best area on the fly only
      // when necessary.
      if ( a < m_ptrMBR[best]->getArea() ) best = cChild;
    }
  }

  return best;
}

uint32_t Index::findLeastOverlap( const Region& r ) const
{
  OverlapEntry** entries = new OverlapEntry*[m_children];

  double leastOverlap = std::numeric_limits<double>::max();
  double me = std::numeric_limits<double>::max();
  OverlapEntry* best = 0;

  // find combined region and enlargement of every entry and store it.
  for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
  {
    try
    {
      entries[cChild] = new OverlapEntry();
    }
    catch ( ... )
    {
      for ( uint32_t i = 0; i < cChild; ++i ) delete entries[i];
      delete[] entries;
      throw;
    }

    entries[cChild]->m_index = cChild;
    entries[cChild]->m_original = m_ptrMBR[cChild];
    entries[cChild]->m_combined = m_pTree->m_regionPool.acquire();
    m_ptrMBR[cChild]->getCombinedRegion( *( entries[cChild]->m_combined ), r );
    entries[cChild]->m_oa = entries[cChild]->m_original->getArea();
    entries[cChild]->m_ca = entries[cChild]->m_combined->getArea();
    entries[cChild]->m_enlargement = entries[cChild]->m_ca - entries[cChild]->m_oa;

    if ( entries[cChild]->m_enlargement < me )
    {
      me = entries[cChild]->m_enlargement;
      best = entries[cChild];
    }
    else if ( entries[cChild]->m_enlargement == me && entries[cChild]->m_oa < best->m_oa )
    {
      best = entries[cChild];
    }
  }

  if ( me < -std::numeric_limits<double>::epsilon() || me > std::numeric_limits<double>::epsilon() )
  {
    uint32_t cIterations;

    if ( m_children > m_pTree->m_nearMinimumOverlapFactor )
    {
      // sort entries in increasing order of enlargement.
      ::qsort( entries, m_children,
               sizeof( OverlapEntry* ),
               OverlapEntry::compareEntries );
      assert( entries[0]->m_enlargement <= entries[m_children - 1]->m_enlargement );

      cIterations = m_pTree->m_nearMinimumOverlapFactor;
    }
    else
    {
      cIterations = m_children;
    }

    // calculate overlap of most important original entries (near minimum overlap cost).
    for ( uint32_t cIndex = 0; cIndex < cIterations; ++cIndex )
    {
      double dif = 0.0;
      OverlapEntry* e = entries[cIndex];

      for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
      {
        if ( e->m_index != cChild )
        {
          double f = e->m_combined->getIntersectingArea( *( m_ptrMBR[cChild] ) );
          if ( f != 0.0 ) dif += f - e->m_original->getIntersectingArea( *( m_ptrMBR[cChild] ) );
        }
      } // for (cChild)

      if ( dif < leastOverlap )
      {
        leastOverlap = dif;
        best = entries[cIndex];
      }
      else if ( dif == leastOverlap )
      {
        if ( e->m_enlargement == best->m_enlargement )
        {
          // keep the one with least area.
          if ( e->m_original->getArea() < best->m_original->getArea() ) best = entries[cIndex];
        }
        else
        {
          // keep the one with least enlargement.
          if ( e->m_enlargement < best->m_enlargement ) best = entries[cIndex];
        }
      }
    } // for (cIndex)
  }

  uint32_t ret = best->m_index;

  for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
  {
    delete entries[cChild];
  }
  delete[] entries;

  return ret;
}

void Index::adjustTree( Node* n, std::stack<id_type>& pathBuffer )
{
  ++( m_pTree->m_stats.m_u64Adjustments );

  // find entry pointing to old node;
  uint32_t child;
  for ( child = 0; child < m_children; ++child )
  {
    if ( m_pIdentifier[child] == n->m_identifier ) break;
  }

  // MBR needs recalculation if either:
  //   1. the NEW child MBR is not contained.
  //   2. the OLD child MBR is touching.
  bool bContained = m_nodeMBR.containsRegion( n->m_nodeMBR );
  bool bTouches = m_nodeMBR.touchesRegion( *( m_ptrMBR[child] ) );
  bool bRecompute = ( ! bContained || ( bTouches && m_pTree->m_bTightMBRs ) );

  *( m_ptrMBR[child] ) = n->m_nodeMBR;

  if ( bRecompute )
  {
    for ( uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim )
    {
      m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
      m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

      for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
      {
        m_nodeMBR.m_pLow[cDim] = std::min( m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim] );
        m_nodeMBR.m_pHigh[cDim] = std::max( m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim] );
      }
    }
  }

  m_pTree->writeNode( this );

  if ( bRecompute && ( ! pathBuffer.empty() ) )
  {
    id_type cParent = pathBuffer.top(); pathBuffer.pop();
    NodePtr ptrN = m_pTree->readNode( cParent );
    Index* p = static_cast<Index*>( ptrN.get() );
    p->adjustTree( this, pathBuffer );
  }
}

void Index::adjustTree( Node* n1, Node* n2, std::stack<id_type>& pathBuffer, byte* overflowTable )
{
  ++( m_pTree->m_stats.m_u64Adjustments );

  // find entry pointing to old node;
  uint32_t child;
  for ( child = 0; child < m_children; ++child )
  {
    if ( m_pIdentifier[child] == n1->m_identifier ) break;
  }

  // MBR needs recalculation if either:
  //   1. the NEW child MBR is not contained.
  //   2. the OLD child MBR is touching.
  bool bContained = m_nodeMBR.containsRegion( n1->m_nodeMBR );
  bool bTouches = m_nodeMBR.touchesRegion( *( m_ptrMBR[child] ) );
  bool bRecompute = ( ! bContained || ( bTouches && m_pTree->m_bTightMBRs ) );

  *( m_ptrMBR[child] ) = n1->m_nodeMBR;

  if ( bRecompute )
  {
    for ( uint32_t cDim = 0; cDim < m_nodeMBR.m_dimension; ++cDim )
    {
      m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
      m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

      for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
      {
        m_nodeMBR.m_pLow[cDim] = std::min( m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim] );
        m_nodeMBR.m_pHigh[cDim] = std::max( m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim] );
      }
    }
  }

  // No write necessary here. insertData will write the node if needed.
  //m_pTree->writeNode(this);

  bool bAdjusted = insertData( 0, 0, n2->m_nodeMBR, n2->m_identifier, pathBuffer, overflowTable );

  // if n2 is contained in the node and there was no split or reinsert,
  // we need to adjust only if recalculation took place.
  // In all other cases insertData above took care of adjustment.
  if (( ! bAdjusted ) && bRecompute && ( ! pathBuffer.empty() ) )
  {
    id_type cParent = pathBuffer.top(); pathBuffer.pop();
    NodePtr ptrN = m_pTree->readNode( cParent );
    Index* p = static_cast<Index*>( ptrN.get() );
    p->adjustTree( this, pathBuffer );
  }
}
