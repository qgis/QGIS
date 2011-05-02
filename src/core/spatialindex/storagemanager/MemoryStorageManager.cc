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

#include "../spatialindex/SpatialIndexImpl.h"

#include "MemoryStorageManager.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;
using std::stack;
using std::vector;

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

MemoryStorageManager::MemoryStorageManager( Tools::PropertySet& ps )
{
}

MemoryStorageManager::~MemoryStorageManager()
{
  for ( vector<Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); it++ ) delete *it;
}

void MemoryStorageManager::loadByteArray( const long id, unsigned long& len, byte** data )
{
  Entry* e;
  try
  {
    e = m_buffer.at( id );
    if ( e == 0 ) throw Tools::InvalidPageException( id );
  }
  catch ( std::out_of_range )
  {
    throw Tools::InvalidPageException( id );
  }

  len = e->m_length;
  *data = new byte[len];

  memcpy( *data, e->m_pData, len );
}

void MemoryStorageManager::storeByteArray( long& id, const unsigned long len, const byte* const data )
{
  if ( id == NewPage )
  {
    Entry* e = new Entry( len, data );

    if ( m_emptyPages.empty() )
    {
      m_buffer.push_back( e );
      id = m_buffer.size() - 1;
    }
    else
    {
      id = m_emptyPages.top(); m_emptyPages.pop();
      m_buffer[id] = e;
    }
  }
  else
  {
    Entry* e_old;
    try
    {
      e_old = m_buffer.at( id );
      if ( e_old == 0 ) throw Tools::InvalidPageException( id );
    }
    catch ( std::out_of_range )
    {
      throw Tools::InvalidPageException( id );
    }

    Entry* e = new Entry( len, data );

    delete e_old;
    m_buffer[id] = e;
  }
}

void MemoryStorageManager::deleteByteArray( const long id )
{
  Entry* e;
  try
  {
    e = m_buffer.at( id );
    if ( e == 0 ) throw Tools::InvalidPageException( id );
  }
  catch ( std::out_of_range )
  {
    throw Tools::InvalidPageException( id );
  }

  m_buffer[id] = 0;
  m_emptyPages.push( id );

  delete e;
}

