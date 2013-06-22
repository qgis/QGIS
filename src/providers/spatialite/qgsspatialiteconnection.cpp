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

#include <QFileInfo>
#include <QSettings>
#include <stdlib.h> // atoi

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

QStringList QgsSpatiaLiteConnection::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/SpatiaLite/connections" );
  return settings.childGroups();
}

void QgsSpatiaLiteConnection::deleteConnection( QString name )
{
  QSettings settings;
  QString key = "/SpatiaLite/connections/" + name;
  settings.remove( key + "/sqlitepath" );
  settings.remove( key );
}

QString QgsSpatiaLiteConnection::connectionPath( QString name )
{
  QSettings settings;
  return settings.value( "/SpatiaLite/connections/" + name + "/sqlitepath" ).toString();
}

// -------

QgsSpatiaLiteConnection::QgsSpatiaLiteConnection( QString name )
{
  // "name" can be either a saved connection or a path to database

  QSettings settings;
  mPath = settings.value( QString( "/SpatiaLite/connections/%1/sqlitepath" ).arg( name ) ).toString();
  if ( mPath.isNull() )
    mPath = name; // not found in settings - probably it's a path
}

QgsSpatiaLiteConnection::Error QgsSpatiaLiteConnection::fetchTables( bool loadGeometrylessTables )
{
  mErrorMsg = QString();

  QFileInfo fi( mPath );
  if ( !fi.exists() )
  {
    return NotExists;
  }

  sqlite3* handle = openSpatiaLiteDb( fi.canonicalFilePath() );
  if ( handle == NULL )
  {
    return FailedToOpen;
  }

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

  if ( ret == LayoutCurrent && recentVersion == false )
  {
    // obsolete library version
    mErrorMsg = tr( "obsolete libspatialite: connecting to this DB requires using v.4.0 (or any subsequent)" );
    return FailedToCheckMetadata;
  }

#ifdef SPATIALITE_VERSION_GE_4_0_0
  // only if libspatialite version is >= 4.0.0
  // using v.4.0 Abstract Interface
  if ( !getTableInfoAbstractInterface( handle, loadGeometrylessTables ) )
  {
    return FailedToGetTables;
  }
  closeSpatiaLiteDb( handle );
  return NoError;
#endif

// obsolete library: still using the traditional approach
  if ( !getTableInfo( handle, loadGeometrylessTables ) )
  {
    return FailedToGetTables;
  }
  closeSpatiaLiteDb( handle );

  return NoError;
}

bool QgsSpatiaLiteConnection::updateStatistics()
{
#ifdef SPATIALITE_VERSION_GE_4_0_0
  QFileInfo fi( mPath );
  if ( !fi.exists() )
  {
    return false;
  }

  sqlite3* handle = openSpatiaLiteDb( fi.canonicalFilePath() );
  if ( handle == NULL )
  {
    return false;
  }

  bool ret = update_layer_statistics( handle, NULL, NULL );

  closeSpatiaLiteDb( handle );

  return ret;
#else
  return false;
#endif
}

sqlite3 *QgsSpatiaLiteConnection::openSpatiaLiteDb( QString path )
{
  sqlite3 *handle = NULL;
  int ret;
  // activating the SpatiaLite library
  spatialite_init( 0 );

  // trying to open the SQLite DB
  ret = sqlite3_open_v2( path.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE, NULL );
  if ( ret )
  {
    // failure
    mErrorMsg = sqlite3_errmsg( handle );
    return NULL;
  }
  return handle;
}

void QgsSpatiaLiteConnection::closeSpatiaLiteDb( sqlite3 * handle )
{
  if ( handle )
    sqlite3_close( handle );
}

int QgsSpatiaLiteConnection::checkHasMetadataTables( sqlite3* handle )
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
  const char *name;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  // checking if table GEOMETRY_COLUMNS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(geometry_columns)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mErrorMsg = tr( "table info on %1 failed" ).arg( "geometry_columns" );
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
    mErrorMsg = tr( "table info on %1 failed" ).arg( "spatial_ref_sys" );
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
    mErrorMsg += "\n";
    mErrorMsg += errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}

#ifdef SPATIALITE_VERSION_GE_4_0_0
// only if libspatialite version is >= 4.0.0
bool QgsSpatiaLiteConnection::getTableInfoAbstractInterface( sqlite3 * handle, bool loadGeometrylessTables )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
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
  list = gaiaGetVectorLayersList( handle, NULL, NULL, GAIA_VECTORS_LIST_FAST );
  if ( list != NULL )
  {
    gaiaVectorLayerPtr lyr = list->First;
    while ( lyr != NULL )
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
        mTables.append( TableEntry( tableName, QString(), "qgis_table" ) );
      }
    }
    sqlite3_free_table( results );
  }

  return true;

error:
  // unexpected IO error
  mErrorMsg = tr( "unknown error cause" );
  if ( errMsg != NULL )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
#endif

bool QgsSpatiaLiteConnection::getTableInfo( sqlite3 * handle, bool loadGeometrylessTables )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
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
      mTables.append( TableEntry( tableName, QString(), "qgis_table" ) );
    }
    sqlite3_free_table( results );
  }

  return true;

error:
  // unexpected IO error
  mErrorMsg = tr( "unknown error cause" );
  if ( errMsg != NULL )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}

QString QgsSpatiaLiteConnection::quotedValue( QString value ) const
{
  if ( value.isNull() )
    return "NULL";

  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}

bool QgsSpatiaLiteConnection::checkGeometryColumnsAuth( sqlite3 * handle )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QString( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'geometry_columns_auth'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, NULL );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] != NULL )
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


bool QgsSpatiaLiteConnection::checkViewsGeometryColumns( sqlite3 * handle )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QString( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'views_geometry_columns'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, NULL );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] != NULL )
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

bool QgsSpatiaLiteConnection::checkVirtsGeometryColumns( sqlite3 * handle )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  QString sql = QString( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'virts_geometry_columns'" );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, NULL );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] != NULL )
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

bool QgsSpatiaLiteConnection::isRasterlite1Datasource( sqlite3 * handle, const char *table )
{
// testing for RasterLite-1 datasources
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  bool exists = false;
  int len;
  char table_raster[4192];
  char sql[4192];

  strcpy( table_raster, table );
  len =  strlen( table_raster );
  if ( strlen( table_raster ) < 9 )
    return false;
  if ( strcmp( table_raster + len - 9, "_metadata" ) != 0 )
    return false;
  // ok, possible candidate
  strcpy( table_raster + len - 9, "_rasters" );

  // checking if the related "_RASTERS table exists
  sprintf( sql, "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '%s'", table_raster );

  ret = sqlite3_get_table( handle, sql, &results, &rows, &columns, NULL );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] != NULL )
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

bool QgsSpatiaLiteConnection::isDeclaredHidden( sqlite3 * handle, QString table, QString geom )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  bool isHidden = false;

  if ( !checkGeometryColumnsAuth( handle ) )
    return false;
  // checking if some Layer has been declared as HIDDEN
  QString sql = QString( "SELECT hidden FROM geometry_columns_auth"
                         " WHERE f_table_name=%1 and f_geometry_column=%2" ).arg( quotedValue( table ) ).
                arg( quotedValue( geom ) );

  ret = sqlite3_get_table( handle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      if ( results[( i * columns ) + 0] != NULL )
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
