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

#include <time.h>
#include <stdlib.h>
#include <cmath>

#include "../spatialindex/SpatialIndexImpl.h"
#include "RandomEvictionsBuffer.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

IBuffer* SpatialIndex::StorageManager::returnRandomEvictionsBuffer( IStorageManager& sm, Tools::PropertySet& ps )
{
  IBuffer* b = new RandomEvictionsBuffer( sm, ps );
  return b;
}

IBuffer* SpatialIndex::StorageManager::createNewRandomEvictionsBuffer( IStorageManager& sm, uint32_t capacity, bool bWriteThrough )
{
  Tools::Variant var;
  Tools::PropertySet ps;

  var.m_varType = Tools::VT_ULONG;
  var.m_val.ulVal = capacity;
  ps.setProperty( "Capacity", var );

  var.m_varType = Tools::VT_BOOL;
  var.m_val.blVal = bWriteThrough;
  ps.setProperty( "WriteThrough", var );

  return returnRandomEvictionsBuffer( sm, ps );
}

RandomEvictionsBuffer::RandomEvictionsBuffer( IStorageManager& sm, Tools::PropertySet& ps ) : Buffer( sm, ps )
{
  srand48( static_cast<uint32_t>( time( 0 ) ) );
}

RandomEvictionsBuffer::~RandomEvictionsBuffer()
{
}

void RandomEvictionsBuffer::addEntry( id_type page, Entry* e )
{
  assert( m_buffer.size() <= m_capacity );

  if ( m_buffer.size() == m_capacity ) removeEntry();
  assert( m_buffer.find( page ) == m_buffer.end() );
  m_buffer.insert( std::pair<id_type, Entry*>( page, e ) );
}

void RandomEvictionsBuffer::removeEntry()
{
  if ( m_buffer.size() == 0 ) return;

  double random;

  random = drand48();

  uint32_t entry = static_cast<uint32_t>( floor((( double ) m_buffer.size() ) * random ) );

  std::map<id_type, Entry*>::iterator it = m_buffer.begin();
  for ( uint32_t cIndex = 0; cIndex < entry; cIndex++ ) ++it;

  if (( *it ).second->m_bDirty )
  {
    id_type page = ( *it ).first;
    m_pStorageManager->storeByteArray( page, (( *it ).second )->m_length, ( const byte * )(( *it ).second )->m_pData );
  }

  delete( *it ).second;
  m_buffer.erase( it );
}
