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

#include "Buffer.h"

using namespace SpatialIndex::StorageManager;
using std::map;

Buffer::Buffer( IStorageManager& sm, Tools::PropertySet& ps ) :
    m_capacity( 10 ),
    m_bWriteThrough( false ),
    m_pStorageManager( &sm ),
    m_hits( 0 )
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
  for ( map<long, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); it++ )
  {
    Entry* e = ( *it ).second;
    long id = ( *it ).first;
    if ( e->m_bDirty ) m_pStorageManager->storeByteArray( id, e->m_length, e->m_pData );
    delete e;
  }
}

void Buffer::loadByteArray( const long id, unsigned long& len, byte** data )
{
  map<long, Entry*>::iterator it = m_buffer.find( id );

  if ( it != m_buffer.end() )
  {
    m_hits++;
    Entry* e = ( *it ).second;
    len = e->m_length;
    *data = new byte[len];
    memcpy( *data, e->m_pData, len );
  }
  else
  {
    m_pStorageManager->loadByteArray( id, len, data );
    Entry* e = new Entry( len, ( const byte * ) *data );
    addEntry( id, e );
  }
}

void Buffer::storeByteArray( long& id, const unsigned long len, const byte* const data )
{
  if ( id == NewPage )
  {
    m_pStorageManager->storeByteArray( id, len, data );
    assert( m_buffer.find( id ) == m_buffer.end() );
    Entry* e = new Entry( len, data );
    addEntry( id, e );
  }
  else
  {
    if ( m_bWriteThrough )
    {
      m_pStorageManager->storeByteArray( id, len, data );
    }

    Entry* e = new Entry( len, data );
    if ( !m_bWriteThrough ) e->m_bDirty = true;

    map<long, Entry*>::iterator it = m_buffer.find( id );
    if ( it != m_buffer.end() )
    {
      delete( *it ).second;
      ( *it ).second = e;
      if ( !m_bWriteThrough ) m_hits++;
    }
    else
    {
      addEntry( id, e );
    }
  }
}

void Buffer::deleteByteArray( const long id )
{
  map<long, Entry*>::iterator it = m_buffer.find( id );
  if ( it != m_buffer.end() )
  {
    delete( *it ).second;
    m_buffer.erase( it );
  }

  m_pStorageManager->deleteByteArray( id );
}

void Buffer::clear()
{
  for ( map<long, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); it++ )
  {
    if (( *it ).second->m_bDirty )
    {
      long id = ( *it ).first;
      m_pStorageManager->storeByteArray( id, (( *it ).second )->m_length, ( const byte * )(( *it ).second )->m_pData );
    }

    delete( *it ).second;
  }

  m_buffer.clear();
  m_hits = 0;
}

unsigned long Buffer::getHits()
{
  return m_hits;
}
