/***************************************************************************
    qgssqlitehandle.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssqlitehandle.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include <QFileInfo>
#include <cstdlib> // atoi

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

// -- ---------------------------------- --
// QgsSqliteHandle
// -- ---------------------------------- --
QMap < QString, QgsSqliteHandle * > QgsSqliteHandle::sHandles;
#if defined(SPATIALITE_HAS_INIT_EX)
QHash<sqlite3 *, void *> QgsSqliteHandle::sSpatialiteConnections;
#endif
QgsSqliteHandle *QgsSqliteHandle::openDb( const QString &dbPath, bool shared,  QString sLayerName, bool bLoadLayers, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  QString sDbPath = dbPath;
  QFileInfo file_info( dbPath );
  // for canonicalFilePath, the file must exist
  if ( file_info.exists() )
  {
    // SpatialiteDbInfo uses the actual absolute path [with resolved soft-links]
    sDbPath = file_info.canonicalFilePath();
  }
  SpatialiteDbInfo::SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown;
  if ( shared && sHandles.contains( sDbPath ) )
  {
    QgsDebugMsg( QString( "Using cached connection(%1) for sDbPath[%2] shared[%3] connection_count[%4]" ).arg( sHandles[sDbPath]->ref ).arg( sDbPath ).arg( shared ).arg( sHandles[sDbPath]->ref ) );
    sHandles[sDbPath]->ref++;
    return sHandles[sDbPath];
  }
  QgsDebugMsg( QString( "New sqlite connection for [%1]" ).arg( sDbPath ) );
  // [will be created if a valid create-option is given and the file does not exist]
  SpatialiteDbInfo *spatialiteDbInfo = SpatialiteDbInfo::CreateSpatialiteConnection( sDbPath, shared, sLayerName, bLoadLayers, dbCreateOption, sniffType );
  if ( spatialiteDbInfo )
  {
    // The file exists, is a Sqlite3-File and a connection has been made.
    if ( spatialiteDbInfo->isDbValid() )
    {
      if ( spatialiteDbInfo->isConnectionShared() )
      {
        // SpatialiteDbInfo will return the actual absolute path [with resolved soft-links]
        sHandles.insert( spatialiteDbInfo->getDatabaseFileName(), spatialiteDbInfo->getQSqliteHandle() );
      }
      // The file Sqlite3-Container is supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
      if ( !spatialiteDbInfo->isDbGdalOgr() )
      {
        // The file Sqlite3-Container is supported by QgsSpatiaLiteProvider.
      }
      else
      {
        // The file Sqlite3-Container is supported by QgsOgrProvider or QgsGdalProvider.
      }
      return spatialiteDbInfo->getQSqliteHandle();
    }
    else
    {
      // Either not a Sqlite3 file or the Sqlite3-Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
      if ( !spatialiteDbInfo->isDbSqlite3() )
      {
        // Is not aSqlite3 file
      }
      else
      {
        // The Sqlite3-Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
        // - but allow anybody to use the QgsSqliteHandle as desired
      }
      return spatialiteDbInfo->getQSqliteHandle();
    }
  }
  return nullptr;
}
void QgsSqliteHandle::closeDb( QgsSqliteHandle *&handle )
{
  if ( handle->ref == -1 )
  {
    // not shared
    handle->sqliteClose();
    delete handle;
  }
  else
  {
    QMap < QString, QgsSqliteHandle * >::iterator i;
    for ( i = sHandles.begin(); i != sHandles.end() && i.value() != handle; ++i )
      ;
    Q_ASSERT( i.value() == handle );
    Q_ASSERT( i.value()->ref > 0 );
    if ( --i.value()->ref == 0 )
    {
      i.value()->sqliteClose();
      delete i.value();
      sHandles.remove( i.key() );
    }
  }
  handle = nullptr;
}
void QgsSqliteHandle::closeAll()
{
  QMap < QString, QgsSqliteHandle * >::iterator i;
  for ( i = sHandles.begin(); i != sHandles.end(); ++i )
  {
    i.value()->sqliteClose();
    delete i.value();
  }
  sHandles.clear();
}
bool QgsSqliteHandle::initRasterlite2()
{
  bool bRc = false;
  if ( sqlite_handle )
  {
#ifdef RASTERLITE2_VERSION_GE_1_1_0
    if ( !rl2PrivateData )
    {
      rl2PrivateData = rl2_alloc_private();
      // register the rl2-functions as sql-commands, verbose=0
      rl2_init( sqlite_handle, rl2PrivateData, 0 );
      mIsRasterLite2Active = true;
    }
    if ( rl2PrivateData )
    {
      bRc = true;
    }
#endif
  }
  return bRc;
}
void QgsSqliteHandle::sqliteClose()
{
  if ( sqlite_handle )
  {
#ifdef RASTERLITE2_VERSION_GE_1_1_0
    if ( rl2PrivateData )
    {
      rl2_cleanup_private( rl2PrivateData );
      rl2PrivateData = nullptr;
      mIsRasterLite2Active = false;
    }
#endif
    QgsSqliteHandle::sqlite3_close( sqlite_handle );
    if ( mSpatialiteDbInfo )
    {
      delete mSpatialiteDbInfo;
      mSpatialiteDbInfo = nullptr;
    }
    sqlite_handle = nullptr;
  }
}
int QgsSqliteHandle::sqlite3_open( const char *filename, sqlite3 **ppDb )
{
#if defined(SPATIALITE_HAS_INIT_EX)
  void *spliteInternalCache = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  int res = ::sqlite3_open( filename, ppDb );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( res == SQLITE_OK )
  {
    spatialite_init_ex( *ppDb, spliteInternalCache, 0 );
    sSpatialiteConnections.insert( *ppDb, spliteInternalCache );
  }
#endif

  return res;
}

int QgsSqliteHandle::sqlite3_close( sqlite3 *db )
{
  int res = ::sqlite3_close( db );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( sSpatialiteConnections.contains( db ) )
  {
    spatialite_cleanup_ex( sSpatialiteConnections.take( db ) );
    spatialite_shutdown();
  }
#endif

  if ( res != SQLITE_OK )
  {
    QgsDebugMsg( QString( "sqlite3_close() failed: %1" ).arg( res ) );
  }

  return res;
}

int QgsSqliteHandle::sqlite3_open_v2( const char *filename, sqlite3 **ppDb, int flags, const char *zVfs )
{
#if defined(SPATIALITE_HAS_INIT_EX)
  void *spliteInternalCache = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  int res = ::sqlite3_open_v2( filename, ppDb, flags, zVfs );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( res == SQLITE_OK )
  {
    spatialite_init_ex( *ppDb, spliteInternalCache, 0 );
    sSpatialiteConnections.insert( *ppDb, spliteInternalCache );
  }
#endif

  return res;
}

int QgsSqliteHandle::sqlite3_close_v2( sqlite3 *db )
{
  int res = ::sqlite3_close( db );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( sSpatialiteConnections.contains( db ) )
  {
    spatialite_cleanup_ex( sSpatialiteConnections.take( db ) );
    spatialite_shutdown();
  }
#endif

  if ( res != SQLITE_OK )
  {
    QgsDebugMsg( QString( "sqlite3_close() failed: %1" ).arg( res ) );
  }

  return res;
}

