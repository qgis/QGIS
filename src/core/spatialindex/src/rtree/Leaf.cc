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

#include <cstring>

#include "../spatialindex/SpatialIndexImpl.h"
#include "RTree.h"
#include "Node.h"
#include "Index.h"
#include "Leaf.h"

using namespace SpatialIndex::RTree;

Leaf::~Leaf()
{
}

Leaf::Leaf( SpatialIndex::RTree::RTree* pTree, id_type id ): Node( pTree, id, 0, pTree->m_leafCapacity )
{
}

NodePtr Leaf::chooseSubtree( const Region& , uint32_t , std::stack<id_type>& )
{
  // should make sure to relinquish other PoolPointer lists that might be pointing to the
  // same leaf.
  return NodePtr( this, &( m_pTree->m_leafPool ) );
}

NodePtr Leaf::findLeaf( const Region& mbr, id_type id, std::stack<id_type>& )
{
  for ( uint32_t cChild = 0; cChild < m_children; ++cChild )
  {
    // should make sure to relinquish other PoolPointer lists that might be pointing to the
    // same leaf.
    if ( m_pIdentifier[cChild] == id && mbr == *( m_ptrMBR[cChild] ) ) return NodePtr( this, &( m_pTree->m_leafPool ) );
  }

  return NodePtr();
}

void Leaf::split( uint32_t dataLength, byte* pData, Region& mbr, id_type id, NodePtr& pLeft, NodePtr& pRight )
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
      throw Tools::NotSupportedException( "Leaf::split: Tree variant not supported." );
  }

  pLeft = m_pTree->m_leafPool.acquire();
  pRight = m_pTree->m_leafPool.acquire();

  if ( pLeft.get() == 0 ) pLeft = NodePtr( new Leaf( m_pTree, -1 ), &( m_pTree->m_leafPool ) );
  if ( pRight.get() == 0 ) pRight = NodePtr( new Leaf( m_pTree, -1 ), &( m_pTree->m_leafPool ) );

  pLeft->m_nodeMBR = m_pTree->m_infiniteRegion;
  pRight->m_nodeMBR = m_pTree->m_infiniteRegion;

  uint32_t cIndex;

  for ( cIndex = 0; cIndex < g1.size(); ++cIndex )
  {
    pLeft->insertEntry( m_pDataLength[g1[cIndex]], m_pData[g1[cIndex]], *( m_ptrMBR[g1[cIndex]] ), m_pIdentifier[g1[cIndex]] );
    // we don't want to delete the data array from this node's destructor!
    m_pData[g1[cIndex]] = 0;
  }

  for ( cIndex = 0; cIndex < g2.size(); ++cIndex )
  {
    pRight->insertEntry( m_pDataLength[g2[cIndex]], m_pData[g2[cIndex]], *( m_ptrMBR[g2[cIndex]] ), m_pIdentifier[g2[cIndex]] );
    // we don't want to delete the data array from this node's destructor!
    m_pData[g2[cIndex]] = 0;
  }
}

void Leaf::deleteData( id_type id, std::stack<id_type>& pathBuffer )
{
  uint32_t child;

  for ( child = 0; child < m_children; ++child )
  {
    if ( m_pIdentifier[child] == id ) break;
  }

  deleteEntry( child );
  m_pTree->writeNode( this );

  std::stack<NodePtr> toReinsert;
  NodePtr ptrThis( this, &( m_pTree->m_leafPool ) );
  condenseTree( toReinsert, pathBuffer, ptrThis );
  ptrThis.relinquish();

  // re-insert eliminated nodes.
  while ( ! toReinsert.empty() )
  {
    NodePtr n = toReinsert.top(); toReinsert.pop();
    m_pTree->deleteNode( n.get() );

    for ( uint32_t cChild = 0; cChild < n->m_children; ++cChild )
    {
      // keep this in the for loop. The tree height might change after insertions.
      byte* overflowTable = new byte[m_pTree->m_stats.m_u32TreeHeight];
      bzero( overflowTable, m_pTree->m_stats.m_u32TreeHeight );
      m_pTree->insertData_impl( n->m_pDataLength[cChild], n->m_pData[cChild], *( n->m_ptrMBR[cChild] ), n->m_pIdentifier[cChild], n->m_level, overflowTable );
      n->m_pData[cChild] = 0;
      delete[] overflowTable;
    }
    if ( n.get() == this ) n.relinquish();
  }
}
