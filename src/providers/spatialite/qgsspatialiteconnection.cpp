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
#include "qgsspatialiteutils.h"
#include "qgssqliteutils.h"
#include "qgsvariantutils.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <cstdlib> // atoi

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

const QString QgsSpatiaLiteConnection::SPATIALITE_ARRAY_PREFIX = QStringLiteral( "json" );
const QString QgsSpatiaLiteConnection::SPATIALITE_ARRAY_SUFFIX = QStringLiteral( "list" );

QStringList QgsSpatiaLiteConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "SpatiaLite/connections" ) );
  return settings.childGroups();
}

QString QgsSpatiaLiteConnection::connectionPath( const QString &name )
{
  const QgsSettings settings;
  return settings.value( "/SpatiaLite/connections/" + name + "/sqlitepath" ).toString();
}

// -------

QgsSpatiaLiteConnection::QgsSpatiaLiteConnection( const QString &name )
{
  // "name" can be either a saved connection or a path to database

  const QgsSettings settings;
  mPath = settings.value( QStringLiteral( "SpatiaLite/connections/%1/sqlitepath" ).arg( name ) ).toString();
  if ( mPath.isNull() )
    mPath = name; // not found in settings - probably it's a path
}

QgsSpatiaLiteConnection::Error QgsSpatiaLiteConnection::fetchTables( bool loadGeometrylessTables )
{
  mErrorMsg = QString();

  const QFileInfo fi( mPath );
  if ( !fi.exists() )
    return NotExists;

  spatialite_database_unique_ptr database;
  int ret = database.open( fi.canonicalFilePath() );
  if ( ret )
    return FailedToOpen;

  ret = checkHasMetadataTables( database.get() );
  if ( !mErrorMsg.isNull() || ret == LayoutUnknown )
  {
    // unexpected error; invalid SpatiaLite DB
    return FailedToCheckMetadata;
  }

  if ( !getTableInfoAbstractInterface( database.get(), loadGeometrylessTables ) )
  {
    return FailedToGetTables;
  }

  return NoError;
}

bool QgsSpatiaLiteConnection::updateStatistics()
{
  const QFileInfo fi( mPath );
  if ( !fi.exists() )
    return false;

  spatialite_database_unique_ptr database;
  int ret = database.open( fi.canonicalFilePath() );
  if ( ret )
    return false;

  ret = update_layer_statistics( database.get(), nullptr, nullptr );

  return ret;
}

QList<QgsVectorDataProvider::NativeType> QgsSpatiaLiteConnection::nativeTypes()
{
  return QList<QgsVectorDataProvider::NativeType>()
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::ByteArray ), QStringLiteral( "BLOB" ), QVariant::ByteArray )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::String ), QStringLiteral( "TEXT" ), QVariant::String )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Double ), QStringLiteral( "FLOAT" ), QVariant::Double )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Int ), QStringLiteral( "INTEGER" ), QVariant::LongLong )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Date ), QStringLiteral( "DATE" ), QVariant::Date )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), QStringLiteral( "TIMESTAMP" ), QVariant::DateTime )

         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::StringList ), SPATIALITE_ARRAY_PREFIX.toUpper() + "TEXT" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::StringList, 0, 0, 0, 0, QVariant::String )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Double ), SPATIALITE_ARRAY_PREFIX.toUpper() + "REAL" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::Double )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Int ), SPATIALITE_ARRAY_PREFIX.toUpper() + "INTEGER" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::LongLong );
}

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
    mErrorMsg = tr( "table info on %1 failed" ).arg( QLatin1String( "geometry_columns" ) );
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
    mErrorMsg = tr( "table info on %1 failed" ).arg( QLatin1String( "spatial_ref_sys" ) );
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

  // List of system tables not to be shown if geometryless tables are requested
  QStringList ignoreTableNames = QgsSqliteUtils::systemTables();

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

      const QString tableName = QString::fromUtf8( lyr->TableName );
      ignoreTableNames << tableName;
      const QString column = QString::fromUtf8( lyr->GeometryName );
      ignoreTableNames << QStringLiteral( "idx_%1_%2" ).arg( tableName, column )
                       << QStringLiteral( "idx_%1_%2_node" ).arg( tableName, column )
                       << QStringLiteral( "idx_%1_%2_parent" ).arg( tableName, column )
                       << QStringLiteral( "idx_%1_%2_rowid" ).arg( tableName, column );
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
        const QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
        if ( !ignoreTableNames.contains( tableName, Qt::CaseInsensitive ) )
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
    const QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
    const QString column = QString::fromUtf8( results[( i * columns ) + 1] );
    const QString type = results[( i * columns ) + 2];
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
      const QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
      const QString column = QString::fromUtf8( results[( i * columns ) + 1] );
      const QString type = results[( i * columns ) + 2];
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
      const QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
      const QString column = QString::fromUtf8( results[( i * columns ) + 1] );
      const QString type = results[( i * columns ) + 2];
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
      const QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
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

bool QgsSpatiaLiteConnection::checkGeometryColumnsAuth( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  const QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'geometry_columns_auth'" );

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


bool QgsSpatiaLiteConnection::checkViewsGeometryColumns( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  const QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'views_geometry_columns'" );

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

bool QgsSpatiaLiteConnection::checkVirtsGeometryColumns( sqlite3 *handle )
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  bool exists = false;

  // checking the metadata tables
  const QString sql = QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'virts_geometry_columns'" );

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
  char sql[4258];

  strncpy( table_raster, table, sizeof table_raster );
  table_raster[ sizeof table_raster - 1 ] = '\0';

  const size_t len = strlen( table_raster );
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
  const QString sql = QString( "SELECT hidden FROM geometry_columns_auth"
                               " WHERE f_table_name=%1 and f_geometry_column=%2" ).arg( QgsSqliteUtils::quotedString( table ),
                                   QgsSqliteUtils::quotedString( geom ) );

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





static void fcnRegexp( sqlite3_context *ctx, int /*argc*/, sqlite3_value *argv[] )
{
  const QRegularExpression re( reinterpret_cast<const char *>( sqlite3_value_text( argv[0] ) ) );
  const QString string( reinterpret_cast<const char *>( sqlite3_value_text( argv[1] ) ) );

  if ( !re.isValid() )
    return sqlite3_result_error( ctx, "invalid operand", -1 );

  sqlite3_result_int( ctx, string.contains( re ) );
}




QMap < QString, QgsSqliteHandle * > QgsSqliteHandle::sHandles;
QMutex QgsSqliteHandle::sHandleMutex;


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

QgsSqliteHandle *QgsSqliteHandle::openDb( const QString &dbPath, bool shared )
{
  //QMap < QString, QgsSqliteHandle* >&handles = QgsSqliteHandle::handles;
  const QMutexLocker locker( &sHandleMutex );

  if ( shared && sHandles.contains( dbPath ) )
  {
    QgsDebugMsg( QStringLiteral( "Using cached connection for %1" ).arg( dbPath ) );
    sHandles[dbPath]->ref++;
    return sHandles[dbPath];
  }

  QgsDebugMsg( QStringLiteral( "New sqlite connection for " ) + dbPath );
  spatialite_database_unique_ptr database;
  if ( database.open_v2( dbPath, shared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, nullptr ) )
  {
    // failure
    QgsDebugMsg( QStringLiteral( "Failure while connecting to: %1\n%2" )
                 .arg( dbPath,
                       QString::fromUtf8( sqlite3_errmsg( database.get() ) ) ) );
    return nullptr;
  }

  // checking the DB for sanity
  if ( !checkMetadata( database.get() ) )
  {
    // failure
    QgsDebugMsg( QStringLiteral( "Failure while connecting to: %1\n\ninvalid metadata tables" ).arg( dbPath ) );
    return nullptr;
  }

  // add REGEXP function
  sqlite3_create_function( database.get(), "REGEXP", 2, SQLITE_UTF8, nullptr, fcnRegexp, nullptr, nullptr );

  // activating Foreign Key constraints
  ( void )sqlite3_exec( database.get(), "PRAGMA foreign_keys = 1", nullptr, nullptr, nullptr );

  QgsDebugMsg( QStringLiteral( "Connection to the database was successful" ) );

  QgsSqliteHandle *handle = new QgsSqliteHandle( std::move( database ), dbPath, shared );
  if ( shared )
    sHandles.insert( dbPath, handle );

  return handle;
}

void QgsSqliteHandle::closeDb( QgsSqliteHandle *&handle )
{
  if ( handle->ref == -1 )
  {
    // not shared
    delete handle;
  }
  else
  {
    const QMutexLocker locker( &sHandleMutex );
    QMap < QString, QgsSqliteHandle * >::iterator i;
    for ( i = sHandles.begin(); i != sHandles.end() && i.value() != handle; ++i )
      ;

    Q_ASSERT( i.value() == handle );
    Q_ASSERT( i.value()->ref > 0 );

    if ( --i.value()->ref == 0 )
    {
      delete i.value();
      i = sHandles.erase( i );
    }
  }

  handle = nullptr;
}

void QgsSqliteHandle::closeAll()
{
  const QMutexLocker locker( &sHandleMutex );
  qDeleteAll( sHandles );
  sHandles.clear();
}
