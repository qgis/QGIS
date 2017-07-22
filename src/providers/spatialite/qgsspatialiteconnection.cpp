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
    mPath = sa_list[1];
  }
  mPath = settings.value( QStringLiteral( "SpatiaLite/connections/%1/sqlitepath" ).arg( mSubKey ) ).toString();
  if ( mPath.isNull() )
  {
    mSubKey = "";
    mPath = name; // not found in settings - probably it's a path
  }
}
SpatialiteDbInfo *QgsSpatiaLiteConnection::CreateSpatialiteConnection( QString sLayerName,  bool bLoadLayers,  bool bShared )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  SpatialiteDbInfo::SpatialSniff sniffType = SpatialiteDbInfo::SniffMinimal;
  if ( bLoadLayers )
  {
    sniffType = SpatialiteDbInfo::SniffLoadLayers;
  }
  QgsSqliteHandle *qSqliteHandle = QgsSqliteHandle::openDb( mPath, bShared, sLayerName, bLoadLayers );
  if ( ( qSqliteHandle ) && ( qSqliteHandle->getSpatialiteDbInfo() ) )
  {
    spatialiteDbInfo = qSqliteHandle->getSpatialiteDbInfo();
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
    {
      if ( !spatialiteDbInfo->GetSpatialiteDbInfo( sLayerName, bLoadLayers, sniffType ) )
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
#if 0
// TODO: Remove after replacing with SpatialiteDbInfo
QgsSpatiaLiteConnection::Error QgsSpatiaLiteConnection::fetchTables( bool loadGeometrylessTables )
{
  mErrorMsg = QString();

  QFileInfo fi( mPath );
  if ( !fi.exists() )
    return NotExists;

  sqlite3 *handle = openSpatiaLiteDb( fi.canonicalFilePath() );
  if ( !handle )
    return FailedToOpen;

  int ret = checkHasMetadataTables( handle );
  if ( !mErrorMsg.isNull() || ret == LayoutUnknown )
  {
    // unexpected error; invalid SpatiaLite DB
    return FailedToCheckMetadata;
  }

  bool recentVersion = false;
#ifdef SPATIALITE_VERSION_GE_4_0_0
  // only if libspatialite version is >= 4.0.0
  recentVersion = true;
#endif

  if ( ret == LayoutCurrent && !recentVersion )
  {
    // obsolete library version
    mErrorMsg = tr( "obsolete libspatialite: connecting to this DB requires using v.4.0 (or any subsequent)" );
    return FailedToCheckMetadata;
  }

#ifdef SPATIALITE_VERSION_GE_4_0_0
  // only if libspatialite version is >= 4.0.0
  // using v.4.0 Abstract Interface
  if ( !getTableInfoAbstractInterface( handle, loadGeometrylessTables ) )
#else
  // obsolete library: still using the traditional approach
  if ( !getTableInfo( handle, loadGeometrylessTables ) )
#endif
  {
    return FailedToGetTables;
  }
  closeSpatiaLiteDb( handle );

  return NoError;
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::updateStatistics()
{
#ifdef SPATIALITE_VERSION_GE_4_0_0
  QFileInfo fi( mPath );
  if ( !fi.exists() )
    return false;

  sqlite3 *handle = openSpatiaLiteDb( fi.canonicalFilePath() );
  if ( !handle )
    return false;

  bool ret = update_layer_statistics( handle, nullptr, nullptr );

  closeSpatiaLiteDb( handle );

  return ret;
#else
  return false;
#endif
}
// TODO: Remove after replacing with SpatialiteDbInfo
sqlite3 *QgsSpatiaLiteConnection::openSpatiaLiteDb( const QString &path )
{
  sqlite3 *handle = nullptr;
  int ret;
  // trying to open the SQLite DB
  ret = QgsSLConnect::sqlite3_open_v2( path.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE, nullptr );
  if ( ret )
  {
    // failure
    mErrorMsg = sqlite3_errmsg( handle );
    return nullptr;
  }
  return handle;
}
// TODO: Remove after replacing with SpatialiteDbInfo
void QgsSpatiaLiteConnection::closeSpatiaLiteDb( sqlite3 *handle )
{
  if ( handle )
    QgsSLConnect::sqlite3_close( handle );
}
// TODO: Remove after replacing with SpatialiteDbInfo
int QgsSpatiaLiteConnection::checkHasMetadataTables( sqlite3 *handle )
{
  bool gcSpatiaLite = false;
  bool rsSpatiaLite = false;
  bool gcSpatiaLite4 = false;
  bool rsSpatiaLite4 = false;
  bool tableName = false;
  bool geomColumn = false;
  bool coordDims = false;
  bool gcSrid = false;
  bool type = false;
  bool geometry_type = false;
  bool spatialIndex = false;
  bool srsSrid = false;
  bool authName = false;
  bool authSrid = false;
  bool refSysName = false;
  bool proj4text = false;
  bool srtext = false;
  int ret;
  const char *name = nullptr;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  // checking if table GEOMETRY_COLUMNS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(geometry_columns)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mErrorMsg = tr( "table info on %1 failed" ).arg( QStringLiteral( "geometry_columns" ) );
    goto error;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      name = results[( i * columns ) + 1];
      if ( strcasecmp( name, "f_table_name" ) == 0 )
        tableName = true;
      if ( strcasecmp( name, "f_geometry_column" ) == 0 )
        geomColumn = true;
      if ( strcasecmp( name, "coord_dimension" ) == 0 )
        coordDims = true;
      if ( strcasecmp( name, "srid" ) == 0 )
        gcSrid = true;
      if ( strcasecmp( name, "type" ) == 0 )
        type = true;
      if ( strcasecmp( name, "geometry_type" ) == 0 )
        geometry_type = true;
      if ( strcasecmp( name, "spatial_index_enabled" ) == 0 )
        spatialIndex = true;
    }
  }
  sqlite3_free_table( results );
  if ( tableName && geomColumn && type && coordDims && gcSrid && spatialIndex )
    gcSpatiaLite = true;
  if ( tableName && geomColumn && geometry_type && coordDims && gcSrid && spatialIndex )
    gcSpatiaLite4 = true;

  // checking if table SPATIAL_REF_SYS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(spatial_ref_sys)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mErrorMsg = tr( "table info on %1 failed" ).arg( QStringLiteral( "spatial_ref_sys" ) );
    goto error;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      name = results[( i * columns ) + 1];
      if ( strcasecmp( name, "srid" ) == 0 )
        srsSrid = true;
      if ( strcasecmp( name, "auth_name" ) == 0 )
        authName = true;
      if ( strcasecmp( name, "auth_srid" ) == 0 )
        authSrid = true;
      if ( strcasecmp( name, "ref_sys_name" ) == 0 )
        refSysName = true;
      if ( strcasecmp( name, "proj4text" ) == 0 )
        proj4text = true;
      if ( strcasecmp( name, "srtext" ) == 0 )
        srtext = true;
    }
  }
  sqlite3_free_table( results );
  if ( srsSrid && authName && authSrid && refSysName && proj4text )
    rsSpatiaLite = true;
  if ( srsSrid && authName && authSrid && refSysName && proj4text && srtext )
    rsSpatiaLite4 = true;

  // OK, this one seems to be a valid SpatiaLite DB
  if ( gcSpatiaLite4 && rsSpatiaLite4 )
    return LayoutCurrent;
  if ( gcSpatiaLite && rsSpatiaLite )
    return LayoutLegacy;

  // this seems to be a valid SQLite DB, but not a SpatiaLite's one
  return LayoutUnknown;

error:
  // unexpected IO error
  if ( errMsg )
  {
    mErrorMsg += '\n';
    mErrorMsg += errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
#ifdef SPATIALITE_VERSION_GE_4_0_0
// TODO: Remove after replacing with SpatialiteDbInfo
// only if libspatialite version is >= 4.0.0
bool QgsSpatiaLiteConnection::getTableInfoAbstractInterface( sqlite3 *handle, bool loadGeometrylessTables )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString sql;
  gaiaVectorLayersListPtr list;

  const char *version = spatialite_version();
  if ( isdigit( *version ) && *version >= '4' )
    ; // OK, linked against libspatialite v.4.0 (or any subsequent)
  else
  {
    mErrorMsg = tr( "obsolete libspatialite: AbstractInterface is unsupported" );
    return false;
  }

// attempting to load the VectorLayersList
  list = gaiaGetVectorLayersList( handle, nullptr, nullptr, GAIA_VECTORS_LIST_FAST );
  if ( list )
  {
    gaiaVectorLayerPtr lyr = list->First;
    while ( lyr )
    {
      // populating the QGIS own Layers List
      if ( lyr->AuthInfos )
      {
        if ( lyr->AuthInfos->IsHidden )
        {
          // skipping any Hidden layer
          lyr = lyr->Next;
          continue;
        }
      }

      QString tableName = QString::fromUtf8( lyr->TableName );
      QString column = QString::fromUtf8( lyr->GeometryName );
      QString type = tr( "UNKNOWN" );
      switch ( lyr->GeometryType )
      {
        case GAIA_VECTOR_GEOMETRY:
          type = tr( "GEOMETRY" );
          break;
        case GAIA_VECTOR_POINT:
          type = tr( "POINT" );
          break;
        case GAIA_VECTOR_LINESTRING:
          type = tr( "LINESTRING" );
          break;
        case GAIA_VECTOR_POLYGON:
          type = tr( "POLYGON" );
          break;
        case GAIA_VECTOR_MULTIPOINT:
          type = tr( "MULTIPOINT" );
          break;
        case GAIA_VECTOR_MULTILINESTRING:
          type = tr( "MULTILINESTRING" );
          break;
        case GAIA_VECTOR_MULTIPOLYGON:
          type = tr( "MULTIPOLYGON" );
          break;
        case GAIA_VECTOR_GEOMETRYCOLLECTION:
          type = tr( "GEOMETRYCOLLECTION" );
          break;
      }
      mTables.append( TableEntry( tableName, column, type ) );

      lyr = lyr->Next;
    }
    gaiaFreeVectorLayersList( list );
  }

  if ( loadGeometrylessTables )
  {
    // get all tables
    sql = "SELECT name "
          "FROM sqlite_master "
          "WHERE type in ('table', 'view')";
    ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
      goto error;
    if ( rows < 1 )
      ;
    else
    {
      for ( i = 1; i <= rows; i++ )
      {
        QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
        mTables.append( TableEntry( tableName, QString(), QStringLiteral( "qgis_table" ) ) );
      }
    }
    sqlite3_free_table( results );
  }

  return true;

error:
  // unexpected IO error
  mErrorMsg = tr( "unknown error cause" );
  if ( errMsg )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
#endif
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::getTableInfo( sqlite3 *handle, bool loadGeometrylessTables )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString sql;

  // the following query return the tables containing a Geometry column
  sql = "SELECT f_table_name, f_geometry_column, type "
        "FROM geometry_columns";
  ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  for ( i = 1; i <= rows; i++ )
  {
    if ( isRasterlite1Datasource( handle, results[( i * columns ) + 0] ) )
      continue;
    QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
    QString column = QString::fromUtf8( results[( i * columns ) + 1] );
    QString type = results[( i * columns ) + 2];
    if ( isDeclaredHidden( handle, tableName, column ) )
      continue;

    mTables.append( TableEntry( tableName, column, type ) );
  }
  sqlite3_free_table( results );

  if ( checkViewsGeometryColumns( handle ) )
  {
    // the following query return the views supporting a Geometry column
    sql = "SELECT view_name, view_geometry, type "
          "FROM views_geometry_columns "
          "JOIN geometry_columns USING (f_table_name, f_geometry_column)";
    ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
      goto error;
    for ( i = 1; i <= rows; i++ )
    {
      QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
      QString column = QString::fromUtf8( results[( i * columns ) + 1] );
      QString type = results[( i * columns ) + 2];
      if ( isDeclaredHidden( handle, tableName, column ) )
        continue;

      mTables.append( TableEntry( tableName, column, type ) );
    }
    sqlite3_free_table( results );
  }

  if ( checkVirtsGeometryColumns( handle ) )
  {
    // the following query return the VirtualShapefiles
    sql = "SELECT virt_name, virt_geometry, type "
          "FROM virts_geometry_columns";
    ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
      goto error;
    for ( i = 1; i <= rows; i++ )
    {
      QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
      QString column = QString::fromUtf8( results[( i * columns ) + 1] );
      QString type = results[( i * columns ) + 2];
      if ( isDeclaredHidden( handle, tableName, column ) )
        continue;

      mTables.append( TableEntry( tableName, column, type ) );
    }
    sqlite3_free_table( results );
  }

  if ( loadGeometrylessTables )
  {
    // get all tables
    sql = "SELECT name "
          "FROM sqlite_master "
          "WHERE type in ('table', 'view')";
    ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
      goto error;
    for ( i = 1; i <= rows; i++ )
    {
      QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
      mTables.append( TableEntry( tableName, QString(), QStringLiteral( "qgis_table" ) ) );
    }
    sqlite3_free_table( results );
  }

  return true;

error:
  // unexpected IO error
  mErrorMsg = tr( "unknown error cause" );
  if ( errMsg )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
QString QgsSpatiaLiteConnection::quotedValue( QString value ) const
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  value.replace( '\'', QLatin1String( "''" ) );
  return value.prepend( '\'' ).append( '\'' );
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::checkGeometryColumnsAuth( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'geometry_columns_auth'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] )
      {
        const char *name = results[( i * columns ) + 0];
        if ( name )
          exists = true;
      }
    }
  }
  sqlite3_free_table( results );
  return exists;
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::checkViewsGeometryColumns( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'views_geometry_columns'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] )
      {
        const char *name = results[( i * columns ) + 0];
        if ( name )
          exists = true;
      }
    }
  }
  sqlite3_free_table( results );
  return exists;
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::checkVirtsGeometryColumns( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'virts_geometry_columns'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] )
      {
        const char *name = results[( i * columns ) + 0];
        if ( name )
          exists = true;
      }
    }
  }
  sqlite3_free_table( results );
  return exists;
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::isRasterlite1Datasource( sqlite3 *handle, const char *table )
{
// testing for RasterLite-1 datasources
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;
  char table_raster[4192];
  char sql[4192];

  strncpy( table_raster, table, sizeof sql );
  table_raster[ sizeof sql - 1 ] = '\0';

  size_t len = strlen( table_raster );
  if ( strlen( table_raster ) < 9 )
    return false;
  if ( strcmp( table_raster + len - 9, "_metadata" ) != 0 )
    return false;
  // OK, possible candidate
  strcpy( table_raster + len - 9, "_rasters" );

  // checking if the related "_RASTERS table exists
  sprintf( sql, "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '%s'", table_raster );

  ret = sqlite3_get_table( handle, sql, &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] )
      {
        const char *name = results[( i * columns ) + 0];
        if ( name )
          exists = true;
      }
    }
  }
  sqlite3_free_table( results );
  return exists;
}
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSpatiaLiteConnection::isDeclaredHidden( sqlite3 *handle, const QString &table, const QString &geom )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  bool isHidden = false;

  if ( !checkGeometryColumnsAuth( handle ) )
    return false;
  // checking if some Layer has been declared as HIDDEN
  QString sql = QString( "SELECT hidden FROM geometry_columns_auth"
                         " WHERE f_table_name=%1 and f_geometry_column=%2" ).arg( quotedValue( table ),
                             quotedValue( geom ) );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] )
      {
        if ( atoi( results[( i * columns ) + 0] ) != 0 )
          isHidden = true;
      }
    }
  }
  sqlite3_free_table( results );

  return isHidden;

error:
  // unexpected IO error
  mErrorMsg = tr( "unknown error cause" );
  if ( errMsg )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
#endif
// -- ---------------------------------- --
// QgsSqliteHandle
// -- ---------------------------------- --
QMap < QString, QgsSqliteHandle * > QgsSqliteHandle::sHandles;
#if 0
// TODO: Remove after replacing with SpatialiteDbInfo
bool QgsSqliteHandle::checkMetadata( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  int spatial_type = 0;
  ret = sqlite3_get_table( handle, "SELECT CheckSpatialMetadata()", &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    goto skip;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
      spatial_type = atoi( results[( i * columns ) + 0] );
  }
  sqlite3_free_table( results );
skip:
  if ( spatial_type == 1 || spatial_type == 3 )
    return true;
  return false;
}
#endif
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
#if 0
  sqlite3 *sqlite_handle = nullptr;
  if ( QgsSLConnect::sqlite3_open_v2( dbPath.toUtf8().constData(), &sqlite_handle, shared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, nullptr ) )
  {
    // failure
    QgsDebugMsg( QString( "Failure while connecting to: %1\n%2" )
                 .arg( dbPath,
                       QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) ) ) );
    return nullptr;
  }
  // qDebug() << QString( "QgsSqliteHandle::openDb -2- Layer[%1] dbPath[%2] " ).arg( sLayerName).arg( dbPath );
  SpatialiteDbInfo *spatialiteDbInfo = QgsSpatiaLiteUtils::GetSpatialiteDbInfoWrapper( dbPath, sLayerName, bLoadLayers, sqlite_handle );
  if ( ( !spatialiteDbInfo ) || ( !spatialiteDbInfo->isDbValid() ) )
  {
    // failure
    if ( spatialiteDbInfo )
    {
      if ( !spatialiteDbInfo->isDbSqlite3() )
      {
        QgsDebugMsg( QString( "Failure while connecting to: %1\n\nThe read Database is not a Sqlite3-Container=%2" ).arg( dbPath ).arg( spatialiteDbInfo->isDbSqlite3() ) );
      }
      else
      {
        QgsDebugMsg( QString( "Failure while connecting to: %1\n\nThe read Database is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider. Sqlite3-Container=%2" ).arg( dbPath ).arg( spatialiteDbInfo->isDbSqlite3() ) );
      }
      spatialiteDbInfo = nullptr;
    }
    else
    {
      QgsDebugMsg( QString( "Failure while connecting to: %1\n\nThe creation of the SpatialiteDbInfo failed. " ).arg( dbPath ) );
    }
    QgsSLConnect::sqlite3_close( sqlite_handle );
    return nullptr;
  }
  // activating Foreign Key constraints [done in SpatialiteDbInfo]
  // ( void )sqlite3_exec( sqlite_handle, "PRAGMA foreign_keys = 1", nullptr, 0, nullptr );
  QgsDebugMsg( "Connection to the database was successful" );
  // qDebug() << QString( "QgsSqliteHandle::openDb -3- Layer[%1] dbPath[%2] " ).arg( sLayerName).arg( dbPath );
  QgsSqliteHandle *handle = new QgsSqliteHandle( sqlite_handle, dbPath, shared );
  if ( spatialiteDbInfo->attachQSqliteHandle( handle ) )
  {
    qDebug() << QString( "QgsSqliteHandle::openDb(%1,%2,%3) -z- Layers-Loaded[%4] Layers-Found[%5] dbPath[%6] " ).arg( handle->ref ).arg( shared ).arg( spatialiteDbInfo->isDbReadOnly() ).arg( spatialiteDbInfo->dbLayersCount() ).arg( spatialiteDbInfo->dbVectorLayersCount() ).arg( dbPath );
  }
#endif
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

