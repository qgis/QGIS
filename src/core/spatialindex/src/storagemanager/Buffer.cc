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
#include "Buffer.h"

Buffer::Buffer( IStorageManager& sm, Tools::PropertySet& ps ) :
    m_capacity( 10 ),
    m_bWriteThrough( false ),
    m_pStorageManager( &sm ),
    m_u64Hits( 0 )
{
  Tools::Variant var = ps.getProperty( "Capacity" );
  if ( var.m_varType != Tools::VT_EMPTY )
  {
    if ( var.m_varType != Tools::VT_ULONG ) throw Tools::IllegalArgumentException( "Property Capacity must be Tools::VT_ULONG" );
    m_capacity = var.m_val.ulVal;
  }

  var = ps.getProperty( "WriteThrough" );
  if ( var.m_varType != Tools::VT_EMPTY )
  {
    if ( var.m_varType != Tools::VT_BOOL ) throw Tools::IllegalArgumentException( "Property WriteThrough must be Tools::VT_BOOL" );
    m_bWriteThrough = var.m_val.blVal;
  }
}

Buffer::~Buffer()
{
  for ( std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it )
  {
    if (( *it ).second->m_bDirty )
    {
      id_type page = ( *it ).first;
      m_pStorageManager->storeByteArray( page, ( *it ).second->m_length, ( *it ).second->m_pData );
    }
    delete( *it ).second;
  }
}

void Buffer::loadByteArray( const id_type page, uint32_t& len, byte** data )
{
  std::map<id_type, Entry*>::iterator it = m_buffer.find( page );

  if ( it != m_buffer.end() )
  {
    ++m_u64Hits;
    len = ( *it ).second->m_length;
    *data = new byte[len];
    memcpy( *data, ( *it ).second->m_pData, len );
  }
  else
  {
    m_pStorageManager->loadByteArray( page, len, data );
    addEntry( page, new Entry( len, static_cast<const byte*>( *data ) ) );
  }
}

void Buffer::storeByteArray( id_type& page, const uint32_t len, const byte* const data )
{
  if ( page == NewPage )
  {
    m_pStorageManager->storeByteArray( page, len, data );
    assert( m_buffer.find( page ) == m_buffer.end() );
    addEntry( page, new Entry( len, data ) );
  }
  else
  {
    if ( m_bWriteThrough )
    {
      m_pStorageManager->storeByteArray( page, len, data );
    }

    Entry* e = new Entry( len, data );
    if ( m_bWriteThrough == false ) e->m_bDirty = true;

    std::map<id_type, Entry*>::iterator it = m_buffer.find( page );
    if ( it != m_buffer.end() )
    {
      delete( *it ).second;
      ( *it ).second = e;
      if ( m_bWriteThrough == false ) ++m_u64Hits;
    }
    else
    {
      addEntry( page, e );
    }
  }
}

void Buffer::deleteByteArray( const id_type page )
{
  std::map<id_type, Entry*>::iterator it = m_buffer.find( page );
  if ( it != m_buffer.end() )
  {
    delete( *it ).second;
    m_buffer.erase( it );
  }

  m_pStorageManager->deleteByteArray( page );
}

void Buffer::clear()
{
  for ( std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it )
  {
    if (( *it ).second->m_bDirty )
    {
      id_type page = ( *it ).first;
      m_pStorageManager->storeByteArray( page, (( *it ).second )->m_length, static_cast<const byte*>((( *it ).second )->m_pData ) );
    }

    delete( *it ).second;
  }

  m_buffer.clear();
  m_u64Hits = 0;
}

uint64_t Buffer::getHits()
{
  return m_u64Hits;
}
