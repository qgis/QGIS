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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>

#ifdef WIN32
#include <io.h>
#ifdef _MSC_VER
#include <basetsd.h>
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif//_MSC_VER
#define fsync(fd) _commit(fd)
#endif

#include "DiskStorageManager.h"

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;
using std::map;
using std::vector;

#ifdef _MSC_VER
typedef SSIZE_T ssize_t;
#endif//_MSC_VER

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::returnDiskStorageManager( Tools::PropertySet& ps )
{
  IStorageManager* sm = new DiskStorageManager( ps );
  return sm;
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::createNewDiskStorageManager( std::string& baseName, unsigned long pageSize )
{
  Tools::Variant var;
  Tools::PropertySet ps;

  var.m_varType = Tools::VT_BOOL;
  var.m_val.blVal = true;
  ps.setProperty( "Overwrite", var );
  // overwrite the file if it exists.

  var.m_varType = Tools::VT_PCHAR;
  var.m_val.pcVal = const_cast<char*>( baseName.c_str() );
  ps.setProperty( "FileName", var );
  // .idx and .dat extensions will be added.

  var.m_varType = Tools::VT_ULONG;
  var.m_val.ulVal = pageSize;
  ps.setProperty( "PageSize", var );
  // specify the page size. Since the index may also contain user defined data
  // there is no way to know how big a single node may become. The storage manager
  // will use multiple pages per node if needed. Off course this will slow down performance.

  return returnDiskStorageManager( ps );
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::loadDiskStorageManager( std::string& baseName )
{
  Tools::Variant var;
  Tools::PropertySet ps;

  var.m_varType = Tools::VT_PCHAR;
  var.m_val.pcVal = const_cast<char*>( baseName.c_str() );
  ps.setProperty( "FileName", var );
  // .idx and .dat extensions will be added.

  return returnDiskStorageManager( ps );
}

DiskStorageManager::DiskStorageManager( Tools::PropertySet& ps ) : m_pageSize( 0 ), m_nextPage( -1 ), m_buffer( 0 )
{
  Tools::Variant var;

  // Open/Create flag.
  bool bOverwrite = false;
  var = ps.getProperty( "Overwrite" );

  if ( var.m_varType != Tools::VT_EMPTY )
  {
    if ( var.m_varType != Tools::VT_BOOL ) throw Tools::IllegalArgumentException( "Property Overwrite must be Tools::VT_BOOL" );
    bOverwrite = var.m_val.blVal;
  }

  // storage filename.
  var = ps.getProperty( "FileName" );

  if ( var.m_varType != Tools::VT_EMPTY )
  {
    if ( var.m_varType != Tools::VT_PCHAR ) throw Tools::IllegalArgumentException( "Property FileName must be Tools::VT_PCHAR" );

    int cLen = strlen( var.m_val.pcVal );

    char* pIndexFile = new char[cLen + 10];
    char* pDataFile = new char[cLen + 10];

    sprintf( pIndexFile, "%s.idx", var.m_val.pcVal );
    sprintf( pDataFile, "%s.dat", var.m_val.pcVal );

    // check if file exists.
    bool bFileExists = true;
    int e1 = access( pIndexFile, F_OK );
    int e2 = access( pDataFile, F_OK );
    if ( e1 != 0 || e2 != 0 ) bFileExists = false;

    if ( ! bFileExists ) bOverwrite = true;

    // check if file can be read/written.
    if ( !bOverwrite )
    {
      int e1 = access( pIndexFile, R_OK | W_OK );
      int e2 = access( pDataFile, R_OK | W_OK );

      if (( e1 != 0 || e2 != 0 ) && bFileExists )
      {
        delete[] pIndexFile;
        delete[] pDataFile;
        throw Tools::IllegalArgumentException( "Index file cannot be read/writen." );
      }
    }

    int cMode = ( bOverwrite ) ? O_CREAT | O_RDWR | O_TRUNC : O_RDWR;

    m_indexFile = open( pIndexFile, cMode, 0644 );
    if ( m_indexFile < 0 )
    {
      delete[] pIndexFile;
      delete[] pDataFile;
      throw Tools::IllegalArgumentException( "Index file cannot be opened." );
    }

    m_dataFile = open( pDataFile, cMode, 0644 );
    if ( m_dataFile < 0 )
    {
      delete[] pIndexFile;
      delete[] pDataFile;
      throw Tools::IllegalArgumentException( "Data file cannot be opened." );
    }

    delete[] pIndexFile;
    delete[] pDataFile;
  }
  else
  {
    throw Tools::IllegalArgumentException( "Property FileName was not specified." );
  }

  // find page size.
  if ( bOverwrite )
  {
    var = ps.getProperty( "PageSize" );

    if ( var.m_varType != Tools::VT_EMPTY )
    {
      if ( var.m_varType != Tools::VT_ULONG ) throw Tools::IllegalArgumentException( "Property PageSize must be Tools::VT_ULONG" );
      m_pageSize = var.m_val.ulVal;
      m_nextPage = 0;
    }
    else
    {
      throw Tools::IllegalArgumentException( "A new storage manager is created and property PageSize was not specified." );
    }
  }
  else
  {
    ssize_t bytesread = read( m_indexFile, &m_pageSize, sizeof( unsigned long ) );
    if ( bytesread != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Failed reading pageSize." );

    bytesread = read( m_indexFile, &m_nextPage, sizeof( long ) );
    if ( bytesread != sizeof( long ) ) throw Tools::IllegalStateException( "Failed reading nextPage." );
  }

  // create buffer.
  m_buffer = new byte[m_pageSize];
  memset( m_buffer, 0, m_pageSize );

  if ( !bOverwrite )
  {
    unsigned long count;
    long id, page;
    ssize_t bytesread;

    // load empty pages in memory.
    bytesread = read( m_indexFile, &count, sizeof( unsigned long ) );
    if ( bytesread != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

    for ( unsigned long cCount = 0; cCount < count; cCount++ )
    {
      bytesread = read( m_indexFile, &page, sizeof( long ) );
      if ( bytesread != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );
      m_emptyPages.push( page );
    }

    // load index table in memory.
    bytesread = read( m_indexFile, &count, sizeof( unsigned long ) );
    if ( bytesread != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

    for ( unsigned long cCount = 0; cCount < count; cCount++ )
    {
      Entry* e = new Entry();

      bytesread = read( m_indexFile, &id, sizeof( long ) );
      if ( bytesread != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

      bytesread = read( m_indexFile, &( e->m_length ), sizeof( unsigned long ) );
      if ( bytesread != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

      unsigned long count2;
      bytesread = read( m_indexFile, &count2, sizeof( unsigned long ) );
      if ( bytesread != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

      for ( unsigned long cCount2 = 0; cCount2 < count2; cCount2++ )
      {
        bytesread = read( m_indexFile, &page, sizeof( long ) );
        if ( bytesread != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );
        e->m_pages.push_back( page );
      }
      m_pageIndex.insert( std::pair<long, Entry* >( id, e ) );
    }
  }
}

DiskStorageManager::~DiskStorageManager()
{
  flush();
  close( m_indexFile );
  close( m_dataFile );
  if ( m_buffer != 0 ) delete[] m_buffer;

  map<long, Entry*>::iterator it = m_pageIndex.begin();

  while ( it != m_pageIndex.end() )
  {
    delete( *it ).second;
    it++;
  }
}

void DiskStorageManager::flush()
{
  ssize_t byteswritten;

  off_t seek = lseek( m_indexFile, 0, SEEK_SET );
  if ( seek < 0 ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

  byteswritten = write( m_indexFile, &m_pageSize, sizeof( unsigned long ) );
  if ( byteswritten != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );
  byteswritten = write( m_indexFile, &m_nextPage, sizeof( long ) );
  if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

  unsigned long count = m_emptyPages.size();
  long id, page;

  byteswritten = write( m_indexFile, &count, sizeof( unsigned long ) );
  if ( byteswritten != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

  while ( ! m_emptyPages.empty() )
  {
    page = m_emptyPages.top(); m_emptyPages.pop();
    byteswritten = write( m_indexFile, &page, sizeof( long ) );
    if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );
  }

  count = m_pageIndex.size();

  byteswritten = write( m_indexFile, &count, sizeof( unsigned long ) );
  if ( byteswritten != sizeof( unsigned long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

  map<long, Entry*>::iterator it = m_pageIndex.begin();

  while ( it != m_pageIndex.end() )
  {
    id = ( *it ).first;
    byteswritten = write( m_indexFile, &id, sizeof( long ) );
    if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

    count = ( *it ).second->m_length;
    byteswritten = write( m_indexFile, &count, sizeof( long ) );
    if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

    count = ( *it ).second->m_pages.size();
    byteswritten = write( m_indexFile, &count, sizeof( long ) );
    if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );

    for ( unsigned long cIndex = 0; cIndex < count; cIndex++ )
    {
      page = ( *it ).second->m_pages[cIndex];
      byteswritten = write( m_indexFile, &page, sizeof( long ) );
      if ( byteswritten != sizeof( long ) ) throw Tools::IllegalStateException( "Corrupted storage manager index file." );
    }
    it++;
  }

  fsync( m_indexFile );
  fsync( m_dataFile );
}

void DiskStorageManager::loadByteArray( const long id, unsigned long& len, byte** data )
{
  map<long, Entry*>::iterator it = m_pageIndex.find( id );

  if ( it == m_pageIndex.end() ) throw Tools::InvalidPageException( id );

  vector<long>& pages = ( *it ).second->m_pages;
  unsigned long cNext = 0;
  unsigned long cTotal = pages.size();

  len = ( *it ).second->m_length;
  *data = new byte[len];

  byte* ptr = *data;
  unsigned long cLen;
  unsigned long cRem = len;

  do
  {
    off_t seek = lseek( m_dataFile, pages[cNext] * m_pageSize, SEEK_SET );
    if ( seek < 0 ) throw Tools::IllegalStateException( "Corrupted data file." );

    ssize_t bytesread = read( m_dataFile, m_buffer, m_pageSize );
    if ( bytesread <= 0 ) throw Tools::IllegalStateException( "Corrupted data file." );

    cLen = ( cRem > m_pageSize ) ? m_pageSize : cRem;
    memcpy( ptr, m_buffer, cLen );

    ptr += cLen;
    cRem -= cLen;
    cNext++;
  }
  while ( cNext < cTotal );
}

void DiskStorageManager::storeByteArray( long& id, const unsigned long len, const byte* const data )
{
  if ( id == NewPage )
  {
    Entry* e = new Entry();
    e->m_length = len;

    const byte* ptr = data;
    long cPage;
    unsigned long cRem = len;
    unsigned long cLen;

    while ( cRem > 0 )
    {
      if ( ! m_emptyPages.empty() )
      {
        cPage = m_emptyPages.top(); m_emptyPages.pop();
      }
      else
      {
        cPage = m_nextPage;
        m_nextPage++;
      }

      cLen = ( cRem > m_pageSize ) ? m_pageSize : cRem;
      memcpy( m_buffer, ptr, cLen );

      off_t seek = lseek( m_dataFile, cPage * m_pageSize, SEEK_SET );
      if ( seek < 0 ) throw Tools::IllegalStateException( "Corrupted data file." );
      ssize_t byteswritten = write( m_dataFile, m_buffer, m_pageSize );
      if ( byteswritten <= 0 ) throw Tools::IllegalStateException( "Corrupted data file." );

      ptr += cLen;
      cRem -= cLen;
      e->m_pages.push_back( cPage );
    }

    id = e->m_pages[0];
    m_pageIndex.insert( std::pair<long, Entry*>( id, e ) );
  }
  else
  {
    // find the entry.
    map<long, Entry*>::iterator it = m_pageIndex.find( id );

    // check if it exists.
    if ( it == m_pageIndex.end() ) throw Tools::IndexOutOfBoundsException( id );

    Entry* oldEntry = ( *it ).second;

    m_pageIndex.erase( it );

    Entry* e = new Entry();
    e->m_length = len;

    const byte* ptr = data;
    long cPage;
    unsigned long cRem = len;
    unsigned long cLen, cNext = 0;

    while ( cRem > 0 )
    {
      if ( cNext < oldEntry->m_pages.size() )
      {
        cPage = oldEntry->m_pages[cNext];
        cNext++;
      }
      else if ( ! m_emptyPages.empty() )
      {
        cPage = m_emptyPages.top(); m_emptyPages.pop();
      }
      else
      {
        cPage = m_nextPage;
        m_nextPage++;
      }

      cLen = ( cRem > m_pageSize ) ? m_pageSize : cRem;
      memcpy( m_buffer, ptr, cLen );

      off_t seek = lseek( m_dataFile, cPage * m_pageSize, SEEK_SET );
      if ( seek < 0 ) throw Tools::IllegalStateException( "Corrupted data file." );
      ssize_t byteswritten = write( m_dataFile, m_buffer, m_pageSize );
      if ( byteswritten <= 0 ) throw Tools::IllegalStateException( "Corrupted data file." );

      ptr += cLen;
      cRem -= cLen;
      e->m_pages.push_back( cPage );
    }

    while ( cNext < oldEntry->m_pages.size() )
    {
      m_emptyPages.push( oldEntry->m_pages[cNext] );
      cNext++;
    }

    m_pageIndex.insert( std::pair<long, Entry*>( id, e ) );
    delete oldEntry;
  }
}

void DiskStorageManager::deleteByteArray( const long id )
{
  map<long, Entry*>::iterator it = m_pageIndex.find( id );

  if ( it == m_pageIndex.end() ) throw Tools::InvalidPageException( id );

  for ( unsigned long cIndex = 0; cIndex < ( *it ).second->m_pages.size(); cIndex++ )
  {
    m_emptyPages.push(( *it ).second->m_pages[cIndex] );
  }

  delete( *it ).second;
  m_pageIndex.erase( it );
}

