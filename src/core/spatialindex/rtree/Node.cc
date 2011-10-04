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

#include "RTree.h"
#include "Node.h"
#include "Index.h"

using std::stack;
using std::vector;
using namespace SpatialIndex::RTree;

//
// Tools::IObject interface
//
Tools::IObject* Node::clone()
{
  throw Tools::NotSupportedException( "IObject::clone should never be called." );
}

//
// Tools::ISerializable interface
//
unsigned long Node::getByteArraySize()
{
  return
    ( sizeof( long ) +
      sizeof( long ) +
      sizeof( long ) +
      ( m_children *( m_pTree->m_dimension * sizeof( double ) * 2 + sizeof( long ) + sizeof( unsigned long ) ) ) +
      m_totalDataLength +
      ( 2 * m_pTree->m_dimension * sizeof( double ) ) );
}

void Node::loadFromByteArray( const byte* ptr )
{
  m_nodeMBR = m_pTree->m_infiniteRegion;

  // skip the node type information, it is not needed.
  ptr += sizeof( long );

  memcpy( &m_level, ptr, sizeof( long ) );
  ptr += sizeof( long );

  memcpy( &m_children, ptr, sizeof( long ) );
  ptr += sizeof( long );

  for ( unsigned long cChild = 0; cChild < m_children; cChild++ )
  {
    m_ptrMBR[cChild] = m_pTree->m_regionPool.acquire();
    *( m_ptrMBR[cChild] ) = m_pTree->m_infiniteRegion;

    memcpy( m_ptrMBR[cChild]->m_pLow, ptr, m_pTree->m_dimension * sizeof( double ) );
    ptr += m_pTree->m_dimension * sizeof( double );
    memcpy( m_ptrMBR[cChild]->m_pHigh, ptr, m_pTree->m_dimension * sizeof( double ) );
    ptr += m_pTree->m_dimension * sizeof( double );
    memcpy( &( m_pIdentifier[cChild] ), ptr, sizeof( long ) );
    ptr += sizeof( long );

    memcpy( &( m_pDataLength[cChild] ), ptr, sizeof( unsigned long ) );
    ptr += sizeof( unsigned long );

    if ( m_pDataLength[cChild] > 0 )
    {
      m_totalDataLength += m_pDataLength[cChild];
      m_pData[cChild] = new byte[m_pDataLength[cChild]];
      memcpy( m_pData[cChild], ptr, m_pDataLength[cChild] );
      ptr += m_pDataLength[cChild];
    }
    else
    {
      m_pData[cChild] = 0;
    }

    //m_nodeMBR.combineRegion(*(m_ptrMBR[cChild]));
  }

  memcpy( m_nodeMBR.m_pLow, ptr, m_pTree->m_dimension * sizeof( double ) );
  ptr += m_pTree->m_dimension * sizeof( double );
  memcpy( m_nodeMBR.m_pHigh, ptr, m_pTree->m_dimension * sizeof( double ) );
  //ptr += m_pTree->m_dimension * sizeof(double);
}

void Node::storeToByteArray( byte** data, unsigned long& len )
{
  len = getByteArraySize();

  *data = new byte[len];
  byte* ptr = *data;

  long type;

  if ( m_level == 0 ) type = PersistentLeaf;
  else type = PersistentIndex;

  memcpy( ptr, &type, sizeof( long ) );
  ptr += sizeof( long );

  memcpy( ptr, &m_level, sizeof( long ) );
  ptr += sizeof( long );

  memcpy( ptr, &m_children, sizeof( long ) );
  ptr += sizeof( long );

  for ( unsigned long cChild = 0; cChild < m_children; cChild++ )
  {
    memcpy( ptr, m_ptrMBR[cChild]->m_pLow, m_pTree->m_dimension * sizeof( double ) );
    ptr += m_pTree->m_dimension * sizeof( double );
    memcpy( ptr, m_ptrMBR[cChild]->m_pHigh, m_pTree->m_dimension * sizeof( double ) );
    ptr += m_pTree->m_dimension * sizeof( double );
    memcpy( ptr, &( m_pIdentifier[cChild] ), sizeof( long ) );
    ptr += sizeof( long );

    memcpy( ptr, &( m_pDataLength[cChild] ), sizeof( unsigned long ) );
    ptr += sizeof( unsigned long );

    if ( m_pDataLength[cChild] > 0 )
    {
      memcpy( ptr, m_pData[cChild], m_pDataLength[cChild] );
      ptr += m_pDataLength[cChild];
    }
  }

  // store the node MBR for efficiency. This increases the node size a little bit.
  memcpy( ptr, m_nodeMBR.m_pLow, m_pTree->m_dimension * sizeof( double ) );
  ptr += m_pTree->m_dimension * sizeof( double );
  memcpy( ptr, m_nodeMBR.m_pHigh, m_pTree->m_dimension * sizeof( double ) );
  //ptr += m_pTree->m_dimension * sizeof(double);

  Q_ASSERT( len == ( ptr - *data ) + m_pTree->m_dimension * sizeof( double ) );
}

//
// SpatialIndex::IEntry interface
//
long Node::getIdentifier() const
{
  return m_identifier;
}

void Node::getShape( IShape** out ) const
{
  *out = new Region( m_nodeMBR );
}

//
// SpatialIndex::INode interface
//
unsigned long Node::getChildrenCount() const
{
  return m_children;
}

long Node::getChildIdentifier( unsigned long index ) const
{
  if ( index >= m_children )
    throw Tools::IndexOutOfBoundsException( index );

  return m_pIdentifier[index];
}

void Node::getChildShape( unsigned long index, IShape** out ) const
{
  if ( index >= m_children )
    throw Tools::IndexOutOfBoundsException( index );

  *out = new Region( *( m_ptrMBR[index] ) );
}

unsigned long Node::getLevel() const
{
  return m_level;
}

bool Node::isLeaf() const
{
  return ( m_level == 0 );
}

bool Node::isIndex() const
{
  return ( m_level != 0 );
}

//
// Internal
//

Node::Node() :
    m_pTree( 0 ),
    m_level( 0 ),
    m_identifier( -1 ),
    m_children( 0 ),
    m_capacity( 0 ),
    m_pData( 0 ),
    m_ptrMBR( 0 ),
    m_pIdentifier( 0 ),
    m_pDataLength( 0 ),
    m_totalDataLength( 0 )
{
}

#ifdef _MSC_VER
// MSVC seems to find RTree* pTree ambiguous
Node::Node( SpatialIndex::RTree::RTree* pTree, long id, unsigned long level, unsigned long capacity ) :
#else
Node::Node( RTree* pTree, long id, unsigned long level, unsigned long capacity ) :
#endif//_MSC_VER
    m_pTree( pTree ),
    m_level( level ),
    m_identifier( id ),
    m_children( 0 ),
    m_capacity( capacity ),
    m_pData( 0 ),
    m_ptrMBR( 0 ),
    m_pIdentifier( 0 ),
    m_pDataLength( 0 ),
    m_totalDataLength( 0 )
{
  m_nodeMBR.makeInfinite( m_pTree->m_dimension );

  try
  {
    m_pDataLength = new unsigned long[m_capacity + 1];
    m_pData = new byte*[m_capacity + 1];
    m_ptrMBR = new RegionPtr[m_capacity + 1];
    m_pIdentifier = new long[m_capacity + 1];
  }
  catch ( ... )
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
  if ( m_pData != 0 )
  {
    for ( unsigned long cChild = 0; cChild < m_children; cChild++ )
    {
      if ( m_pData[cChild] != 0 ) delete[] m_pData[cChild];
    }

    delete[] m_pData;
  }

  delete[] m_pDataLength;
  delete[] m_ptrMBR;
  delete[] m_pIdentifier;
}

Node& Node::operator=( const Node & n )
{
  Q_UNUSED( n );
  throw Tools::IllegalStateException( "operator =: This should never be called." );
}

void Node::insertEntry( unsigned long dataLength, byte* pData, Region& mbr, long id )
{
  Q_ASSERT( m_children < m_capacity );

  m_pDataLength[m_children] = dataLength;
  m_pData[m_children] = pData;
  m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
  *( m_ptrMBR[m_children] ) = mbr;
  m_pIdentifier[m_children] = id;

  m_totalDataLength += dataLength;
  m_children++;

  m_nodeMBR.combineRegion( mbr );
}

void Node::deleteEntry( unsigned long index )
{
  Q_ASSERT( index < m_children );

  // cache it, since I might need it for "touches" later.
  RegionPtr ptrR = m_ptrMBR[index];

  m_totalDataLength -= m_pDataLength[index];
  if ( m_pData[index] != 0 ) delete[] m_pData[index];

  if ( m_children > 1 && index != m_children - 1 )
  {
    m_pDataLength[index] = m_pDataLength[m_children - 1];
    m_pData[index] = m_pData[m_children - 1];
    m_ptrMBR[index] = m_ptrMBR[m_children - 1];
    m_pIdentifier[index] = m_pIdentifier[m_children - 1];
  }

  m_children--;

  // WARNING: index has now changed. Do not use it below here.

  if ( m_children == 0 )
  {
    m_nodeMBR = m_pTree->m_infiniteRegion;
  }
  else if ( m_pTree->m_bTightMBRs && m_nodeMBR.touchesRegion( *ptrR ) )
  {
    for ( unsigned long cDim = 0; cDim < m_nodeMBR.m_dimension; cDim++ )
    {
      m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
      m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

      for ( unsigned long cChild = 0; cChild < m_children; cChild++ )
      {
        m_nodeMBR.m_pLow[cDim] = qMin( m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim] );
        m_nodeMBR.m_pHigh[cDim] = qMax( m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim] );
      }
    }
  }
}

bool Node::insertData( unsigned long dataLength, byte* pData, Region& mbr, long id, stack<long>& pathBuffer, byte* overflowTable )
{
  if ( m_children < m_capacity )
  {
    bool adjusted = false;

    // this has to happen before insertEntry modifies m_nodeMBR.
    bool b = m_nodeMBR.containsRegion( mbr );

    insertEntry( dataLength, pData, mbr, id );
    m_pTree->writeNode( this );

    if (( ! b ) && ( ! pathBuffer.empty() ) )
    {
      long cParent = pathBuffer.top(); pathBuffer.pop();
      NodePtr ptrN = m_pTree->readNode( cParent );
      Index* p = static_cast<Index*>( ptrN.get() );
      p->adjustTree( this, pathBuffer );
      adjusted = true;
    }

    return adjusted;
  }
  else if ( m_pTree->m_treeVariant == RV_RSTAR && ( ! pathBuffer.empty() ) && overflowTable[m_level] == 0 )
  {
    overflowTable[m_level] = 1;

    vector<long> vReinsert, vKeep;
    reinsertData( dataLength, pData, mbr, id, vReinsert, vKeep );

    unsigned long lReinsert = vReinsert.size();
    unsigned long lKeep = vKeep.size();

    byte** reinsertdata = 0;
    RegionPtr* reinsertmbr = 0;
    long* reinsertid = 0;
    unsigned long* reinsertlen = 0;
    byte** keepdata = 0;
    RegionPtr* keepmbr = 0;
    long* keepid = 0;
    unsigned long* keeplen = 0;

    try
    {
      reinsertdata = new byte*[lReinsert];
      reinsertmbr = new RegionPtr[lReinsert];
      reinsertid = new long[lReinsert];
      reinsertlen = new unsigned long[lReinsert];

      keepdata = new byte*[m_capacity + 1];
      keepmbr = new RegionPtr[m_capacity + 1];
      keepid = new long[m_capacity + 1];
      keeplen = new unsigned long[m_capacity + 1];
    }
    catch ( ... )
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

    unsigned long cIndex;

    for ( cIndex = 0; cIndex < lReinsert; cIndex++ )
    {
      reinsertlen[cIndex] = m_pDataLength[vReinsert[cIndex]];
      reinsertdata[cIndex] = m_pData[vReinsert[cIndex]];
      reinsertmbr[cIndex] = m_ptrMBR[vReinsert[cIndex]];
      reinsertid[cIndex] = m_pIdentifier[vReinsert[cIndex]];
    }

    for ( cIndex = 0; cIndex < lKeep; cIndex++ )
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

    for ( unsigned long cChild = 0; cChild < m_children; cChild++ ) m_totalDataLength += m_pDataLength[cChild];

    for ( unsigned long cDim = 0; cDim < m_nodeMBR.m_dimension; cDim++ )
    {
      m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
      m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

      for ( unsigned long cChild = 0; cChild < m_children; cChild++ )
      {
        m_nodeMBR.m_pLow[cDim] = qMin( m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim] );
        m_nodeMBR.m_pHigh[cDim] = qMax( m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim] );
      }
    }

    m_pTree->writeNode( this );

    // Divertion from R*-Tree algorithm here. First adjust
    // the path to the root, then start reinserts, to avoid complicated handling
    // of changes to the same node from multiple insertions.
    long cParent = pathBuffer.top(); pathBuffer.pop();
    NodePtr ptrN = m_pTree->readNode( cParent );
    Index* p = static_cast<Index*>( ptrN.get() );
    p->adjustTree( this, pathBuffer );

    for ( cIndex = 0; cIndex < lReinsert; cIndex++ )
    {
      m_pTree->insertData_impl(
        reinsertlen[cIndex], reinsertdata[cIndex],
        *( reinsertmbr[cIndex] ), reinsertid[cIndex],
        m_level, overflowTable );
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
    split( dataLength, pData, mbr, id, n, nn );

    if ( pathBuffer.empty() )
    {
      n->m_level = m_level;
      nn->m_level = m_level;
      n->m_identifier = -1;
      nn->m_identifier = -1;
      m_pTree->writeNode( n.get() );
      m_pTree->writeNode( nn.get() );

      NodePtr ptrR = m_pTree->m_indexPool.acquire();
      if ( ptrR.get() == 0 )
      {
        ptrR = NodePtr( new Index( m_pTree, m_pTree->m_rootID, m_level + 1 ), &( m_pTree->m_indexPool ) );
      }
      else
      {
        //ptrR->m_pTree = m_pTree;
        ptrR->m_identifier = m_pTree->m_rootID;
        ptrR->m_level = m_level + 1;
        ptrR->m_nodeMBR = m_pTree->m_infiniteRegion;
      }

      ptrR->insertEntry( 0, 0, n->m_nodeMBR, n->m_identifier );
      ptrR->insertEntry( 0, 0, nn->m_nodeMBR, nn->m_identifier );

      m_pTree->writeNode( ptrR.get() );

      m_pTree->m_stats.m_nodesInLevel[m_level] = 2;
      m_pTree->m_stats.m_nodesInLevel.push_back( 1 );
      m_pTree->m_stats.m_treeHeight = m_level + 2;
    }
    else
    {
      n->m_level = m_level;
      nn->m_level = m_level;
      n->m_identifier = m_identifier;
      nn->m_identifier = -1;

      m_pTree->writeNode( n.get() );
      m_pTree->writeNode( nn.get() );

      long cParent = pathBuffer.top(); pathBuffer.pop();
      NodePtr ptrN = m_pTree->readNode( cParent );
      Index* p = static_cast<Index*>( ptrN.get() );
      p->adjustTree( n.get(), nn.get(), pathBuffer, overflowTable );
    }

    return true;
  }
}

void Node::reinsertData( unsigned long dataLength, byte* pData, Region& mbr, long id, vector<long>& reinsert, vector<long>& keep )
{
  ReinsertEntry** v = new ReinsertEntry*[m_capacity + 1];

  m_pDataLength[m_children] = dataLength;
  m_pData[m_children] = pData;
  m_ptrMBR[m_children] = m_pTree->m_regionPool.acquire();
  *( m_ptrMBR[m_children] ) = mbr;
  m_pIdentifier[m_children] = id;

  PointPtr nc = m_pTree->m_pointPool.acquire();
  m_nodeMBR.getCenter( *nc );
  PointPtr c = m_pTree->m_pointPool.acquire();

  for ( unsigned long cChild = 0; cChild < m_capacity + 1; cChild++ )
  {
    try
    {
      v[cChild] = new ReinsertEntry( cChild, 0.0 );
    }
    catch ( ... )
    {
      for ( unsigned long i = 0; i < cChild; i++ ) delete v[i];
      delete[] v;
      throw;
    }

    m_ptrMBR[cChild]->getCenter( *c );

    // calculate relative distance of every entry from the node MBR (ignore square root.)
    for ( unsigned long cDim = 0; cDim < m_nodeMBR.m_dimension; cDim++ )
    {
      double d = nc->m_pCoords[cDim] - c->m_pCoords[cDim];
      v[cChild]->m_dist += d * d;
    }
  }

  // sort by increasing order of distances.
  ::qsort( v, m_capacity + 1, sizeof( ReinsertEntry* ), ReinsertEntry::compareReinsertEntry );

  unsigned long cReinsert = ( unsigned long ) std::floor(( m_capacity + 1 ) * m_pTree->m_reinsertFactor );

  unsigned long cCount;

  for ( cCount = 0; cCount < cReinsert; cCount++ )
  {
    reinsert.push_back( v[cCount]->m_id );
    delete v[cCount];
  }

  for ( cCount = cReinsert; cCount < m_capacity + 1; cCount++ )
  {
    keep.push_back( v[cCount]->m_id );
    delete v[cCount];
  }

  delete[] v;
}

void Node::rtreeSplit( unsigned long dataLength, byte* pData, Region& mbr, long id, vector<long>& group1, vector<long>& group2 )
{
  unsigned long cChild;
  unsigned long minimumLoad = static_cast<unsigned long>( std::floor( m_capacity * m_pTree->m_fillFactor ) );

  // use this mask array for marking visited entries.
  byte* mask = new byte[m_capacity + 1];
  memset( mask, 0, m_capacity + 1 );

  // insert new data in the node for easier manipulation. Data arrays are always
  // by one larger than node capacity.
  m_pDataLength[m_capacity] = dataLength;
  m_pData[m_capacity] = pData;
  m_ptrMBR[m_capacity] = m_pTree->m_regionPool.acquire();
  *( m_ptrMBR[m_capacity] ) = mbr;
  m_pIdentifier[m_capacity] = id;
  // m_totalDataLength does not need to be increased here.

  // initialize each group with the seed entries.
  unsigned long seed1, seed2;
  pickSeeds( seed1, seed2 );

  group1.push_back( seed1 );
  group2.push_back( seed2 );

  mask[seed1] = 1;
  mask[seed2] = 1;

  // find MBR of each group.
  RegionPtr mbr1 = m_pTree->m_regionPool.acquire();
  *mbr1 = *( m_ptrMBR[seed1] );
  RegionPtr mbr2 = m_pTree->m_regionPool.acquire();
  *mbr2 = *( m_ptrMBR[seed2] );

  // count how many entries are left unchecked (exclude the seeds here.)
  unsigned long cRemaining = m_capacity + 1 - 2;

  while ( cRemaining > 0 )
  {
    if ( minimumLoad - group1.size() == cRemaining )
    {
      // all remaining entries must be assigned to group1 to comply with minimun load requirement.
      for ( cChild = 0; cChild < m_capacity + 1; cChild++ )
      {
        if ( mask[cChild] == 0 )
        {
          group1.push_back( cChild );
          mask[cChild] = 1;
          cRemaining--;
        }
      }
    }
    else if ( minimumLoad - group2.size() == cRemaining )
    {
      // all remaining entries must be assigned to group2 to comply with minimun load requirement.
      for ( cChild = 0; cChild < m_capacity + 1; cChild++ )
      {
        if ( mask[cChild] == 0 )
        {
          group2.push_back( cChild );
          mask[cChild] = 1;
          cRemaining--;
        }
      }
    }
    else
    {
      // For all remaining entries compute the difference of the cost of grouping an
      // entry in either group. When done, choose the entry that yielded the maximum
      // difference. In case of linear split, select any entry (e.g. the first one.)
      long sel = -1;
      double md1 = 0.0, md2 = 0.0;
      double m = -std::numeric_limits<double>::max();
      double d1, d2, d;
      double a1 = mbr1->getArea();
      double a2 = mbr2->getArea();

      RegionPtr a = m_pTree->m_regionPool.acquire();
      RegionPtr b = m_pTree->m_regionPool.acquire();

      for ( cChild = 0; cChild < m_capacity + 1; cChild++ )
      {
        if ( mask[cChild] == 0 )
        {
          mbr1->getCombinedRegion( *a, *( m_ptrMBR[cChild] ) );
          d1 = a->getArea() - a1;
          mbr2->getCombinedRegion( *b, *( m_ptrMBR[cChild] ) );
          d2 = b->getArea() - a2;
          d = qAbs( d1 - d2 );

          if ( d > m )
          {
            m = d;
            md1 = d1; md2 = d2;
            sel = cChild;
            if ( m_pTree->m_treeVariant == RV_LINEAR || m_pTree->m_treeVariant == RV_RSTAR ) break;
          }
        }
      }

      // determine the group where we should add the new entry.
      long group = -1;

      if ( md1 < md2 )
      {
        group1.push_back( sel );
        group = 1;
      }
      else if ( md2 < md1 )
      {
        group2.push_back( sel );
        group = 2;
      }
      else if ( a1 < a2 )
      {
        group1.push_back( sel );
        group = 1;
      }
      else if ( a2 < a1 )
      {
        group2.push_back( sel );
        group = 2;
      }
      else if ( group1.size() < group2.size() )
      {
        group1.push_back( sel );
        group = 1;
      }
      else if ( group2.size() < group1.size() )
      {
        group2.push_back( sel );
        group = 2;
      }
      else
      {
        group1.push_back( sel );
        group = 1;
      }
      mask[sel] = 1;
      cRemaining--;
      if ( group == 1 )
      {
        mbr1->combineRegion( *( m_ptrMBR[sel] ) );
      }
      else
      {
        mbr2->combineRegion( *( m_ptrMBR[sel] ) );
      }
    }
  }

  delete[] mask;
}

void Node::rstarSplit( unsigned long dataLength, byte* pData, Region& mbr, long id, std::vector<long>& group1, std::vector<long>& group2 )
{
  RstarSplitEntry** dataLow = 0;
  RstarSplitEntry** dataHigh = 0;

  try
  {
    dataLow = new RstarSplitEntry*[m_capacity + 1];
    dataHigh = new RstarSplitEntry*[m_capacity + 1];
  }
  catch ( ... )
  {
    delete[] dataLow;
    throw;
  }

  m_pDataLength[m_capacity] = dataLength;
  m_pData[m_capacity] = pData;
  m_ptrMBR[m_capacity] = m_pTree->m_regionPool.acquire();
  *( m_ptrMBR[m_capacity] ) = mbr;
  m_pIdentifier[m_capacity] = id;
  // m_totalDataLength does not need to be increased here.

  unsigned long nodeSPF = static_cast<unsigned long>( std::floor(( m_capacity + 1 ) * m_pTree->m_splitDistributionFactor ) );
  unsigned long splitDistribution = ( m_capacity + 1 ) - ( 2 * nodeSPF ) + 2;

  unsigned long cChild = 0, cDim, cIndex;

  for ( cChild = 0; cChild <= m_capacity; cChild++ )
  {
    try
    {
      dataLow[cChild] = new RstarSplitEntry( m_ptrMBR[cChild].get(), cChild, 0 );
    }
    catch ( ... )
    {
      for ( unsigned long i = 0; i < cChild; i++ ) delete dataLow[i];
      delete[] dataLow;
      delete[] dataHigh;
      throw;
    }

    dataHigh[cChild] = dataLow[cChild];
  }

  double minimumMargin = std::numeric_limits<double>::max();
  long splitAxis = -1;
  long sortOrder = -1;

  // chooseSplitAxis.
  for ( cDim = 0; cDim < m_pTree->m_dimension; cDim++ )
  {
    ::qsort( dataLow, m_capacity + 1, sizeof( RstarSplitEntry* ), RstarSplitEntry::compareLow );
    ::qsort( dataHigh, m_capacity + 1, sizeof( RstarSplitEntry* ), RstarSplitEntry::compareHigh );

    // calculate sum of margins and overlap for all distributions.
    double marginl = 0.0;
    double marginh = 0.0;

    Region bbl1, bbl2, bbh1, bbh2;

    for ( cChild = 1; cChild <= splitDistribution; cChild++ )
    {
      unsigned long l = nodeSPF - 1 + cChild;

      bbl1 = *( dataLow[0]->m_pRegion );
      bbh1 = *( dataHigh[0]->m_pRegion );

      for ( cIndex = 1; cIndex < l; cIndex++ )
      {
        bbl1.combineRegion( *( dataLow[cIndex]->m_pRegion ) );
        bbh1.combineRegion( *( dataHigh[cIndex]->m_pRegion ) );
      }

      bbl2 = *( dataLow[l]->m_pRegion );
      bbh2 = *( dataHigh[l]->m_pRegion );

      for ( cIndex = l + 1; cIndex <= m_capacity; cIndex++ )
      {
        bbl2.combineRegion( *( dataLow[cIndex]->m_pRegion ) );
        bbh2.combineRegion( *( dataHigh[cIndex]->m_pRegion ) );
      }

      marginl += bbl1.getMargin() + bbl2.getMargin();
      marginh += bbh1.getMargin() + bbh2.getMargin();
    } // for (cChild)

    double margin = qMin( marginl, marginh );

    // keep minimum margin as split axis.
    if ( margin < minimumMargin )
    {
      minimumMargin = margin;
      splitAxis = cDim;
      sortOrder = ( marginl < marginh ) ? 0 : 1;
    }

    // increase the dimension according to which the data entries should be sorted.
    for ( cChild = 0; cChild <= m_capacity; cChild++ )
    {
      dataLow[cChild]->m_sortDim = cDim + 1;
    }
  } // for (cDim)

  for ( cChild = 0; cChild <= m_capacity; cChild++ )
  {
    dataLow[cChild]->m_sortDim = splitAxis;
  }

  ::qsort( dataLow, m_capacity + 1, sizeof( RstarSplitEntry* ), ( sortOrder == 0 ) ? RstarSplitEntry::compareLow : RstarSplitEntry::compareHigh );

  double ma = std::numeric_limits<double>::max();
  double mo = std::numeric_limits<double>::max();
  long splitPoint = -1;

  Region bb1, bb2;

  for ( cChild = 1; cChild <= splitDistribution; cChild++ )
  {
    unsigned long l = nodeSPF - 1 + cChild;

    bb1 = *( dataLow[0]->m_pRegion );

    for ( cIndex = 1; cIndex < l; cIndex++ )
    {
      bb1.combineRegion( *( dataLow[cIndex]->m_pRegion ) );
    }

    bb2 = *( dataLow[l]->m_pRegion );

    for ( cIndex = l + 1; cIndex <= m_capacity; cIndex++ )
    {
      bb2.combineRegion( *( dataLow[cIndex]->m_pRegion ) );
    }

    double o = bb1.getIntersectingArea( bb2 );

    if ( o < mo )
    {
      splitPoint = cChild;
      mo = o;
      ma = bb1.getArea() + bb2.getArea();
    }
    else if ( o == mo )
    {
      double a = bb1.getArea() + bb2.getArea();

      if ( a < ma )
      {
        splitPoint = cChild;
        ma = a;
      }
    }
  } // for (cChild)

  unsigned long l1 = nodeSPF - 1 + splitPoint;

  for ( cIndex = 0; cIndex < l1; cIndex++ )
  {
    group1.push_back( dataLow[cIndex]->m_id );
    delete dataLow[cIndex];
  }

  for ( cIndex = l1; cIndex <= m_capacity; cIndex++ )
  {
    group2.push_back( dataLow[cIndex]->m_id );
    delete dataLow[cIndex];
  }

  delete[] dataLow;
  delete[] dataHigh;
}

void Node::pickSeeds( unsigned long& index1, unsigned long& index2 )
{
  double separation = -std::numeric_limits<double>::max();
  double inefficiency = -std::numeric_limits<double>::max();
  unsigned long cDim, cChild, cIndex;

  switch ( m_pTree->m_treeVariant )
  {
    case RV_LINEAR:
    case RV_RSTAR:
      for ( cDim = 0; cDim < m_pTree->m_dimension; cDim++ )
      {
        double leastLower = m_ptrMBR[0]->m_pLow[cDim];
        double greatestUpper = m_ptrMBR[0]->m_pHigh[cDim];
        unsigned long greatestLower = 0;
        unsigned long leastUpper = 0;
        double width;

        for ( cChild = 1; cChild <= m_capacity; cChild++ )
        {
          if ( m_ptrMBR[cChild]->m_pLow[cDim] > m_ptrMBR[greatestLower]->m_pLow[cDim] ) greatestLower = cChild;
          if ( m_ptrMBR[cChild]->m_pHigh[cDim] < m_ptrMBR[leastUpper]->m_pHigh[cDim] ) leastUpper = cChild;

          leastLower = qMin( m_ptrMBR[cChild]->m_pLow[cDim], leastLower );
          greatestUpper = qMax( m_ptrMBR[cChild]->m_pHigh[cDim], greatestUpper );
        }

        width = greatestUpper - leastLower;
        if ( width <= 0 ) width = 1;

        double f = ( m_ptrMBR[greatestLower]->m_pLow[cDim] - m_ptrMBR[leastUpper]->m_pHigh[cDim] ) / width;

        if ( f > separation )
        {
          index1 = leastUpper;
          index2 = greatestLower;
          separation = f;
        }
      }  // for (cDim)

      if ( index1 == index2 )
      {
        if ( index2 == 0 ) index2++;
        else index2--;
      }

      break;
    case RV_QUADRATIC:
      // for each pair of Regions (account for overflow Region too!)
      for ( cChild = 0; cChild < m_capacity; cChild++ )
      {
        double a = m_ptrMBR[cChild]->getArea();

        for ( cIndex = cChild + 1; cIndex <= m_capacity; cIndex++ )
        {
          // get the combined MBR of those two entries.
          Region r;
          m_ptrMBR[cChild]->getCombinedRegion( r, *( m_ptrMBR[cIndex] ) );

          // find the inefficiency of grouping these entries together.
          double d = r.getArea() - a - m_ptrMBR[cIndex]->getArea();

          if ( d > inefficiency )
          {
            inefficiency = d;
            index1 = cChild;
            index2 = cIndex;
          }
        }  // for (cIndex)
      } // for (cChild)

      break;
    default:
      throw Tools::NotSupportedException( "Node::pickSeeds: Tree variant not supported." );
  }
}

void Node::condenseTree( stack<NodePtr>& toReinsert, stack<long>& pathBuffer, NodePtr& ptrThis )
{
  unsigned long minimumLoad = static_cast<unsigned long>( std::floor( m_capacity * m_pTree->m_fillFactor ) );

  if ( pathBuffer.empty() )
  {
    // eliminate root if it has only one child.
    if ( m_level != 0 && m_children == 1 )
    {
      NodePtr ptrN = m_pTree->readNode( m_pIdentifier[0] );
      m_pTree->deleteNode( ptrN.get() );
      ptrN->m_identifier = m_pTree->m_rootID;
      m_pTree->writeNode( ptrN.get() );

      m_pTree->m_stats.m_nodesInLevel.pop_back();
      m_pTree->m_stats.m_treeHeight -= 1;
      // HACK: pending deleteNode for deleted child will decrease nodesInLevel, later on.
      m_pTree->m_stats.m_nodesInLevel[m_pTree->m_stats.m_treeHeight - 1] = 2;
    }
  }
  else
  {
    long cParent = pathBuffer.top(); pathBuffer.pop();
    NodePtr ptrParent = m_pTree->readNode( cParent );
    Index* p = static_cast<Index*>( ptrParent.get() );

    // find the entry in the parent, that points to this node.
    unsigned long child;

    for ( child = 0; child != p->m_children; child++ )
    {
      if ( p->m_pIdentifier[child] == m_identifier ) break;
    }

    if ( m_children < minimumLoad )
    {
      // used space less than the minimum
      // 1. eliminate node entry from the parent. deleteEntry will fix the parent's MBR.
      p->deleteEntry( child );
      // 2. add this node to the stack in order to reinsert its entries.
      toReinsert.push( ptrThis );
    }
    else
    {
      // adjust the entry in 'p' to contain the new bounding region of this node.
      *( p->m_ptrMBR[child] ) = m_nodeMBR;

      // global recalculation necessary since the MBR can only shrink in size,
      // due to data removal.
      if ( m_pTree->m_bTightMBRs )
      {
        for ( unsigned long cDim = 0; cDim < p->m_nodeMBR.m_dimension; cDim++ )
        {
          p->m_nodeMBR.m_pLow[cDim] = std::numeric_limits<double>::max();
          p->m_nodeMBR.m_pHigh[cDim] = -std::numeric_limits<double>::max();

          for ( unsigned long cChild = 0; cChild < p->m_children; cChild++ )
          {
            p->m_nodeMBR.m_pLow[cDim] = qMin( p->m_nodeMBR.m_pLow[cDim], p->m_ptrMBR[cChild]->m_pLow[cDim] );
            p->m_nodeMBR.m_pHigh[cDim] = qMax( p->m_nodeMBR.m_pHigh[cDim], p->m_ptrMBR[cChild]->m_pHigh[cDim] );
          }
        }
      }
    }

    // write parent node back to storage.
    m_pTree->writeNode( p );

    p->condenseTree( toReinsert, pathBuffer, ptrParent );
  }
}
