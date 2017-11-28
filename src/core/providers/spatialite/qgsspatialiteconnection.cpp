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
  QFileInfo file_info( mDbPath );
  // for canonicalFilePath, the file must exist
  if ( file_info.exists() )
  {
    // SpatialiteDbInfo uses the actual absolute path [with resolved soft-links]
    mDbPath = file_info.canonicalFilePath();
  }
}
SpatialiteDbInfo *QgsSpatiaLiteConnection::CreateSpatialiteConnection( QString sLayerName,  bool bLoadLayers,  bool bShared, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  QgsSqliteHandle *qSqliteHandle = QgsSqliteHandle::openDb( mDbPath, bShared, sLayerName, bLoadLayers, dbCreateOption );
  if ( ( qSqliteHandle ) && ( qSqliteHandle->getSpatialiteDbInfo() ) )
  {
    spatialiteDbInfo = qSqliteHandle->getSpatialiteDbInfo();
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
    {
      if ( ( !spatialiteDbInfo->isDbSpatialite() ) && ( !spatialiteDbInfo->isDbGdalOgr() ) )
      {
        // The read Sqlite3 Container is not supported by QgsSpatiaLiteProvider, QRasterLite2Provider,QgsOgrProvider or QgsGdalProvider.
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
