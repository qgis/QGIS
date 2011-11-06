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

  checkHasMetadataTables( handle );
  if ( !mErrorMsg.isNull() )
  {
    // unexpected error; invalid SpatiaLite DB
    return FailedToCheckMetadata;
  }

  if ( !getTableInfo( handle, loadGeometrylessTables ) )
  {
    return FailedToGetTables;
  }
  closeSpatiaLiteDb( handle );

  return NoError;
}


sqlite3 *QgsSpatiaLiteConnection::openSpatiaLiteDb( QString path )
{
  sqlite3 *handle = NULL;
  int ret;
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

bool QgsSpatiaLiteConnection::checkHasMetadataTables( sqlite3* handle )
{
  bool gcSpatiaLite = false;
  bool rsSpatiaLite = false;
  bool tableName = false;
  bool geomColumn = false;
  bool coordDims = false;
  bool gcSrid = false;
  bool type = false;
  bool spatialIndex = false;
  bool srsSrid = false;
  bool authName = false;
  bool authSrid = false;
  bool refSysName = false;
  bool proj4text = false;
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
    goto error;
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
      if ( strcasecmp( name, "spatial_index_enabled" ) == 0 )
        spatialIndex = true;
    }
  }
  sqlite3_free_table( results );
  if ( tableName && geomColumn && type && coordDims && gcSrid && spatialIndex )
    gcSpatiaLite = true;

  // checking if table SPATIAL_REF_SYS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(spatial_ref_sys)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
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
    }
  }
  sqlite3_free_table( results );
  if ( srsSrid && authName && authSrid && refSysName && proj4text )
    rsSpatiaLite = true;

  // OK, this one seems to be a valid SpatiaLite DB
  if ( gcSpatiaLite && rsSpatiaLite )
    return true;

  // this seems to be a valid SQLite DB, but not a SpatiaLite's one
  return false;

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

bool QgsSpatiaLiteConnection::getTableInfo( sqlite3 * handle, bool loadGeometrylessTables )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  bool ok = false;
  QString sql;

  // the following query return the tables containing a Geometry column
  sql = "SELECT f_table_name, f_geometry_column, type "
        "FROM geometry_columns";
  ret = sqlite3_get_table( handle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
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
    ok = true;
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
    if ( rows < 1 )
      ;
    else
    {
      for ( i = 1; i <= rows; i++ )
      {
        QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
        QString column = QString::fromUtf8( results[( i * columns ) + 1] );
        QString type = results[( i * columns ) + 2];
        if ( isDeclaredHidden( handle, tableName, column ) )
          continue;

        mTables.append( TableEntry( tableName, column, type ) );
      }
      ok = true;
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
    if ( rows < 1 )
      ;
    else
    {
      for ( i = 1; i <= rows; i++ )
      {
        QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
        QString column = QString::fromUtf8( results[( i * columns ) + 1] );
        QString type = results[( i * columns ) + 2];
        if ( isDeclaredHidden( handle, tableName, column ) )
          continue;

        mTables.append( TableEntry( tableName, column, type ) );
      }
      ok = true;
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
    if ( rows < 1 )
      ;
    else
    {
      for ( i = 1; i <= rows; i++ )
      {
        QString tableName = QString::fromUtf8( results[( i * columns ) + 0] );
        mTables.append( TableEntry( tableName, QString(), "qgis_table" ) );
      }
      ok = true;
    }
    sqlite3_free_table( results );
  }

  return ok;

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
  if ( errMsg != NULL )
  {
    mErrorMsg = errMsg;
    sqlite3_free( errMsg );
  }
  return false;
}
