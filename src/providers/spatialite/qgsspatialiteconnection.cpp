/***************************************************************************
    qgsspatialiteconnection.cpp
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
#include "qgsspatialiteconnection.h"
#include "qgsslconnect.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include <QFileInfo>
#include <cstdlib> // atoi

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

QStringList QgsSpatiaLiteConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "SpatiaLite/connections" ) );
  return settings.childGroups();
}
void QgsSpatiaLiteConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  QString key = "/SpatiaLite/connections/" + name;
  settings.remove( key + "/sqlitepath" );
  settings.remove( key );
}
int QgsSpatiaLiteConnection::deleteInvalidConnections( )
{
  int i_count = 0;
  Q_FOREACH ( const QString &name, QgsSpatiaLiteConnection::connectionList() )
  {
    // retrieving the SQLite DB name and full path
    QFileInfo db_file( QgsSpatiaLiteConnection::connectionPath( name ) );
    if ( !db_file.exists() )
    {
      QgsSpatiaLiteConnection::deleteConnection( name );
      i_count++;
    }
  }
  return i_count;
}
QString QgsSpatiaLiteConnection::connectionPath( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/SpatiaLite/connections/" + name + "/sqlitepath" ).toString();
}
// -------
QgsSpatiaLiteConnection::QgsSpatiaLiteConnection( const QString &name )
{
  // "name" can be either a saved connection or a path to database
  mSubKey = name;
  QgsSettings settings;
  if ( mSubKey.indexOf( '@' ) > 0 )
  {
    QStringList sa_list = mSubKey.split( '@' );
    mSubKey = sa_list[0];
    mDbPath = sa_list[1];
  }
  mDbPath = settings.value( QStringLiteral( "SpatiaLite/connections/%1/sqlitepath" ).arg( mSubKey ) ).toString();
  if ( mDbPath.isNull() )
  {
    mSubKey = "";
    mDbPath = name; // not found in settings - probably it's a path
  }
}
SpatialiteDbInfo *QgsSpatiaLiteConnection::CreateSpatialiteConnection( QString sLayerName,  bool bLoadLayers,  bool bShared )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  QgsSqliteHandle *qSqliteHandle = QgsSqliteHandle::openDb( mDbPath, bShared, sLayerName, bLoadLayers );
  if ( ( qSqliteHandle ) && ( qSqliteHandle->getSpatialiteDbInfo() ) )
  {
    spatialiteDbInfo = qSqliteHandle->getSpatialiteDbInfo();
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
    {
      if ( ( !spatialiteDbInfo->isDbSpatialite() ) && ( !spatialiteDbInfo->isDbGdalOgr() ) )
      {
        // The read Sqlite3 Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider.
      }
    }
    else
    {
      // File does not exist or is not a Sqlite3 Container.
      delete spatialiteDbInfo;
      spatialiteDbInfo = nullptr;
    }
  }
  return spatialiteDbInfo;
}
// -- ---------------------------------- --
// QgsSqliteHandle
// -- ---------------------------------- --
QMap < QString, QgsSqliteHandle * > QgsSqliteHandle::sHandles;
QgsSqliteHandle *QgsSqliteHandle::openDb( const QString &dbPath, bool shared,  QString sLayerName, bool bLoadLayers )
{
  //QMap < QString, QgsSqliteHandle* >&handles = QgsSqliteHandle::handles;
  if ( shared && sHandles.contains( dbPath ) )
  {
    QgsDebugMsg( QString( "Using cached connection for %1" ).arg( dbPath ) );
    sHandles[dbPath]->ref++;
    qDebug() << QString( "QgsSqliteHandle::openDb(%1) -1- dbPath[%2] shared[%3] connection_count[%4]" ).arg( sHandles[dbPath]->ref ).arg( dbPath ).arg( shared ).arg( sHandles[dbPath]->ref );
    return sHandles[dbPath];
  }
  QgsDebugMsg( QString( "New sqlite connection for " ) + dbPath );
  SpatialiteDbInfo *spatialiteDbInfo = SpatialiteDbInfo::CreateSpatialiteConnection( dbPath, shared, sLayerName, bLoadLayers );
  if ( spatialiteDbInfo )
  {
    // The file exists, is a Sqlite3-File and a connection has been made.
    if ( spatialiteDbInfo->isDbValid() )
    {
      if ( spatialiteDbInfo->isConnectionShared() )
        sHandles.insert( spatialiteDbInfo->getDatabaseFileName(), spatialiteDbInfo->getQSqliteHandle() );
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
void QgsSqliteHandle::initRasterlite2()
{
  if ( sqlite_handle )
  {
#ifdef RASTERLITE2_VERSION_GE_1_1_0
    if ( !rl2PrivateData )
    {
      rl2PrivateData = rl2_alloc_private();
      rl2_init( sqlite_handle, rl2PrivateData, 0 );
    }
#endif
  }
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
    }
#endif
    QgsSLConnect::sqlite3_close( sqlite_handle );
    if ( mSpatialiteDbInfo )
    {
      delete mSpatialiteDbInfo;
      mSpatialiteDbInfo = nullptr;
    }
    sqlite_handle = nullptr;
  }
}

