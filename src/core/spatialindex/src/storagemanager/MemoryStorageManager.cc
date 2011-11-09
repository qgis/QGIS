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

#include <stdexcept>
#include <cstring>

#include "../spatialindex/SpatialIndexImpl.h"
#include "MemoryStorageManager.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::returnMemoryStorageManager( Tools::PropertySet& ps )
{
  IStorageManager* sm = new MemoryStorageManager( ps );
  return sm;
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::createNewMemoryStorageManager()
{
  Tools::PropertySet ps;
  return returnMemoryStorageManager( ps );
}

MemoryStorageManager::MemoryStorageManager( Tools::PropertySet& )
{
}

MemoryStorageManager::~MemoryStorageManager()
{
  for ( std::vector<Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it ) delete *it;
}

void MemoryStorageManager::loadByteArray( const id_type page, uint32_t& len, byte** data )
{
  Entry* e;
  try
  {
    e = m_buffer.at( page );
    if ( e == 0 ) throw InvalidPageException( page );
  }
  catch ( std::out_of_range )
  {
    throw InvalidPageException( page );
  }

  len = e->m_length;
  *data = new byte[len];

  memcpy( *data, e->m_pData, len );
}

void MemoryStorageManager::storeByteArray( id_type& page, const uint32_t len, const byte* const data )
{
  if ( page == NewPage )
  {
    Entry* e = new Entry( len, data );

    if ( m_emptyPages.empty() )
    {
      m_buffer.push_back( e );
      page = m_buffer.size() - 1;
    }
    else
    {
      page = m_emptyPages.top(); m_emptyPages.pop();
      m_buffer[page] = e;
    }
  }
  else
  {
    Entry* e_old;
    try
    {
      e_old = m_buffer.at( page );
      if ( e_old == 0 ) throw InvalidPageException( page );
    }
    catch ( std::out_of_range )
    {
      throw InvalidPageException( page );
    }

    Entry* e = new Entry( len, data );

    delete e_old;
    m_buffer[page] = e;
  }
}

void MemoryStorageManager::deleteByteArray( const id_type page )
{
  Entry* e;
  try
  {
    e = m_buffer.at( page );
    if ( e == 0 ) throw InvalidPageException( page );
  }
  catch ( std::out_of_range )
  {
    throw InvalidPageException( page );
  }

  m_buffer[page] = 0;
  m_emptyPages.push( page );

  delete e;
}

