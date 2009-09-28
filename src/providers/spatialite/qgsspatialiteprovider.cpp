/***************************************************************************
           qgsspatialiteprovider.cpp Data provider for SpatiaLite DBMS
begin                : Dec, 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cassert>

#include <qgis.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>

#include "qgsprovidercountcalcevent.h"
#include "qgsproviderextentcalcevent.h"

#include "qgsspatialiteprovider.h"

#include "qgslogger.h"

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

const QString SPATIALITE_KEY = "spatialite";
const QString SPATIALITE_DESCRIPTION = "SpatiaLite data provider";

QMap < QString, QgsSpatiaLiteProvider::SqliteHandles * >QgsSpatiaLiteProvider::SqliteHandles::handles;

QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri ): QgsVectorDataProvider( uri ),
    geomType( QGis::WKBUnknown ), sqliteHandle( NULL ), sqliteStatement( NULL ), mSrid( -1 ), spatialIndexRTree( false ), spatialIndexMbrCache( false )
{
  QgsDataSourceURI mUri = QgsDataSourceURI( uri );

  // parsing members from the uri structure
  mTableName = mUri.table();
  geometryColumn = mUri.geometryColumn();
  mSqlitePath = mUri.database();

  // trying to open the SQLite DB
  spatialite_init( 0 );
  valid = true;
  handle = SqliteHandles::openDb( mSqlitePath );
  if ( handle == NULL )
  {
    valid = false;
    return;
  }
  sqliteHandle = handle->handle();

  enabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
  enabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
  enabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
  enabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
  enabledCapabilities |= QgsVectorDataProvider::AddFeatures;
  enabledCapabilities |= QgsVectorDataProvider::AddAttributes;

  if ( !getGeometryDetails() )  // gets srid and geometry type
  {
    // the table is not a geometry table
    numberFeatures = 0;
    valid = false;

    QgsLogger::critical( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
  if ( !getTableSummary() )     // gets the extent and feature count
  {
    numberFeatures = 0;
    valid = false;

    QgsLogger::critical( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
  // load the columns list
  loadFields();
  if ( sqliteHandle == NULL )
  {
    valid = false;

    QgsLogger::critical( "Invalid SpatiaLite layer" );
    return;
  }
  //fill type names into sets
  mNativeTypes
  << QgsVectorDataProvider::NativeType( tr( "BLOB" ), "SQLITE_BLOB", QVariant::ByteArray )
  << QgsVectorDataProvider::NativeType( tr( "Text" ), "SQLITE_TEXT", QVariant::String )
  << QgsVectorDataProvider::NativeType( tr( "Double" ), "SQLITE_FLOAT", QVariant::Double, 0, 20, 0, 20 )
  << QgsVectorDataProvider::NativeType( tr( "Integer" ), "SQLITE_INTEGER", QVariant::LongLong, 0, 20 )
  ;
}

QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider()
{
  closeDb();
}

void QgsSpatiaLiteProvider::loadFields()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString pkName;
  int pkCount = 0;
  int fldNo = 0;
  char xSql[1024];

  attributeFields.clear();
  primaryKey.clear();

  QString sql = QString( "PRAGMA table_info(%1)" ).arg( quotedValue( mTableName ) );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString name = results[( i * columns ) + 1];
      const char *type = results[( i * columns ) + 2];
      QString pk = results[( i * columns ) + 5];
      if ( pk.toInt() != 0 )
      {
        // found a Primary Key column
        pkCount++;
        pkName = name;
      }

      if ( name != geometryColumn )
      {
        // for sure any SQLite value can be represented as SQLITE_TEXT
        QVariant::Type fieldType = QVariant::String;

        // making some assumptions in order to guess a more realistic type
        if ( strcasecmp( type, "int" ) == 0 ||
             strcasecmp( type, "integer" ) == 0 ||
             strcasecmp( type, "bigint" ) == 0 ||
             strcasecmp( type, "smallint" ) == 0 || strcasecmp( type, "tinyint" ) == 0 || strcasecmp( type, "boolean" ) == 0 )
        {
          fieldType = QVariant::Int;
        }
        else if ( strcasecmp( type, "real" ) == 0 ||
                  strcasecmp( type, "double" ) == 0 ||
                  strcasecmp( type, "double precision" ) == 0 || strcasecmp( type, "float" ) == 0 )
        {
          fieldType = QVariant::Double;
        }

        attributeFields.insert( fldNo++, QgsField( name, fieldType, type, 0, 0, "" ) );
      }
    }
  }
  sqlite3_free_table( results );

  if ( pkCount == 1 )
  {
    // setting the Primary Key column name
    primaryKey = pkName;
  }

  return;

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QString error = "loadFields() SQL error: ";
    error = errMsg;
    QgsLogger::critical( error );
    sqlite3_free( errMsg );
  }
}


QString QgsSpatiaLiteProvider::storageType() const
{
  return "SQLite database with SpatiaLite extension";
}


bool QgsSpatiaLiteProvider::featureAtId( int featureId, QgsFeature & feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  sqlite3_stmt *stmt = NULL;
  char xSql[1024];
  char geomName[128];

  QString sql = "SELECT ROWID";
  for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
  {
    const QgsField & fld = field( *it );
    const QString & fieldname = fld.name();
    sql += ", ";
    sql += fieldname;
  }
  if ( fetchGeometry )
  {
    sql += QString( ", AsBinary(%1)" ).arg( geometryColumn );
  }
  sql += QString( " FROM %1 WHERE ROWID = %2" ).arg( quotedValue( mTableName ) ).arg( featureId );

  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    return false;
  }

  int ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE )
  {
    // there are no more rows to fetch - we can stop looping destroying the SQLite statement
    sqlite3_finalize( stmt );
    return false;
  }
  if ( ret == SQLITE_ROW )
  {
    // one valid row has been fetched from the result set
    if ( !mFetchGeom )
    {
      // no geometry was required
      feature.setGeometryAndOwnership( 0, 0 );
    }

    int ic;
    int n_columns = sqlite3_column_count( stmt );
    for ( ic = 0; ic < n_columns; ic++ )
    {
      if ( ic == 0 )
      {
        // first column always contains the ROWID
        feature.setFeatureId( sqlite3_column_int( stmt, ic ) );
      }
      else
      {
        // iterate attributes
        bool fetched = false;
        int nAttr = 1;
        for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); it++ )
        {
          if ( nAttr == ic )
          {
            // ok, this one is the corresponding attribure
            if ( sqlite3_column_type( stmt, ic ) == SQLITE_INTEGER )
            {
              // INTEGER value
              feature.addAttribute( *it, sqlite3_column_int( stmt, ic ) );
              fetched = true;
            }
            else if ( sqlite3_column_type( stmt, ic ) == SQLITE_FLOAT )
            {
              // DOUBLE value
              feature.addAttribute( *it, sqlite3_column_double( stmt, ic ) );
              fetched = true;
            }
            else if ( sqlite3_column_type( stmt, ic ) == SQLITE_TEXT )
            {
              // TEXT value
              const char *txt = ( const char * ) sqlite3_column_text( stmt, ic );
              QString str = QString::fromUtf8( txt );
              feature.addAttribute( *it, str );
              fetched = true;
            }
            else
            {
              // assuming NULL
              feature.addAttribute( *it, QVariant( QString::null ) );
              fetched = true;
            }
          }
          nAttr++;
        }
        if ( fetched )
        {
          continue;
        }
        if ( mFetchGeom )
        {
          QString geoCol = QString( "AsBinary(%1)" ).arg( geometryColumn );
          strcpy( geomName, geoCol.toUtf8().constData() );
          if ( strcasecmp( geomName, sqlite3_column_name( stmt, ic ) ) == 0 )
          {
            if ( sqlite3_column_type( stmt, ic ) == SQLITE_BLOB )
            {
              const void *blob = sqlite3_column_blob( stmt, ic );
              size_t blob_size = sqlite3_column_bytes( stmt, ic );
              unsigned char *featureGeom = new unsigned char[blob_size + 1];
              memset( featureGeom, '\0', blob_size + 1 );
              memcpy( featureGeom, blob, blob_size );
              feature.setGeometryAndOwnership( featureGeom, blob_size + 1 );
            }
            else
            {
              // NULL geometry
              feature.setGeometryAndOwnership( 0, 0 );
            }
          }
        }
      }
    }
  }
  else
  {
    // some unexpected error occurred
    QString error = "sqlite3_step() error: ";
    error += sqlite3_errmsg( sqliteHandle );
    QgsLogger::critical( error );
    sqlite3_finalize( stmt );
    return false;
  }
  sqlite3_finalize( stmt );

  return true;
}

bool QgsSpatiaLiteProvider::nextFeature( QgsFeature & feature )
{
  char geomName[128];

  feature.setValid( false );
  if ( !valid )
  {
    QgsLogger::critical( "Read attempt on an invalid SpatiaLite data source" );
    return false;
  }

  if ( sqliteStatement == NULL )
  {
    QgsLogger::critical( "Invalid current SQLite statement" );
    return false;
  }

  int ret = sqlite3_step( sqliteStatement );
  if ( ret == SQLITE_DONE )
  {
    // there are no more rows to fetch - we can stop looping destroying the SQLite statement
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
    return false;
  }
  if ( ret == SQLITE_ROW )
  {
    // one valid row has been fetched from the result set
    if ( !mFetchGeom )
    {
      // no geometry was required
      feature.setGeometryAndOwnership( 0, 0 );
    }

    int ic;
    int n_columns = sqlite3_column_count( sqliteStatement );
    for ( ic = 0; ic < n_columns; ic++ )
    {
      if ( ic == 0 )
      {
        // first column always contains the ROWID
        feature.setFeatureId( sqlite3_column_int( sqliteStatement, ic ) );
      }
      else
      {
        // iterate attributes
        bool fetched = false;
        int nAttr = 1;
        for ( QgsAttributeList::const_iterator it = mAttributesToFetch.constBegin(); it != mAttributesToFetch.constEnd(); it++ )
        {
          if ( nAttr == ic )
          {
            // ok, this one is the corresponding attribure
            if ( sqlite3_column_type( sqliteStatement, ic ) == SQLITE_INTEGER )
            {
              // INTEGER value
              feature.addAttribute( *it, sqlite3_column_int( sqliteStatement, ic ) );
              fetched = true;
            }
            else if ( sqlite3_column_type( sqliteStatement, ic ) == SQLITE_FLOAT )
            {
              // DOUBLE value
              feature.addAttribute( *it, sqlite3_column_double( sqliteStatement, ic ) );
              fetched = true;
            }
            else if ( sqlite3_column_type( sqliteStatement, ic ) == SQLITE_TEXT )
            {
              // TEXT value
              const char *txt = ( const char * ) sqlite3_column_text( sqliteStatement, ic );
              QString str = QString::fromUtf8( txt );
              feature.addAttribute( *it, str );
              fetched = true;
            }
            else
            {
              // assuming NULL
              feature.addAttribute( *it, QVariant( QString::null ) );
              fetched = true;
            }
          }
          nAttr++;
        }
        if ( fetched )
        {
          continue;
        }
        if ( mFetchGeom )
        {
          QString geoCol = QString( "AsBinary(%1)" ).arg( geometryColumn );
          strcpy( geomName, geoCol.toUtf8().constData() );
          if ( strcasecmp( geomName, sqlite3_column_name( sqliteStatement, ic ) ) == 0 )
          {
            if ( sqlite3_column_type( sqliteStatement, ic ) == SQLITE_BLOB )
            {
              const void *blob = sqlite3_column_blob( sqliteStatement, ic );
              size_t blob_size = sqlite3_column_bytes( sqliteStatement, ic );
              unsigned char *featureGeom = new unsigned char[blob_size + 1];
              memset( featureGeom, '\0', blob_size + 1 );
              memcpy( featureGeom, blob, blob_size );
              feature.setGeometryAndOwnership( featureGeom, blob_size + 1 );
            }
            else
            {
              // NULL geometry
              feature.setGeometryAndOwnership( 0, 0 );
            }
          }
        }
      }
    }
  }
  else
  {
    // some unexpected error occurred
    QString error = "sqlite3_step() error: ";
    error += sqlite3_errmsg( sqliteHandle );
    QgsLogger::critical( error );
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
    return false;
  }

  feature.setValid( true );
  return true;
}

void QgsSpatiaLiteProvider::select( QgsAttributeList fetchAttributes, QgsRectangle rect, bool fetchGeometry, bool useIntersect )
{
// preparing the SQL statement
  char xSql[1024];

  if ( !valid )
  {
    QgsLogger::critical( "Read attempt on an invalid SpatiaLite data source" );
    return;
  }

  if ( sqliteStatement != NULL )
  {
    // finalizing the current SQLite statement
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
  }

  QString sql = "SELECT ROWID";
  for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
  {
    const QgsField & fld = field( *it );
    const QString & fieldname = fld.name();
    sql += ", ";
    sql += fieldname;
  }
  if ( fetchGeometry )
  {
    sql += QString( ", AsBinary(%1)" ).arg( geometryColumn );
  }
  sql += QString( " FROM %1" ).arg( quotedValue( mTableName ) );

  QString whereClause;

  if ( !rect.isEmpty() )
  {
    // some kind of MBR spatial filtering is required
    whereClause = " WHERE ";
    if ( useIntersect )
    {
      // we are requested to evaluate a true INTERSECT relationship
      QString mbr = QString( "%1, %2, %3, %4" ).
                    arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      whereClause += QString( "Intersects(%1, BuildMbr(%2)) AND " ).arg( geometryColumn ).arg( mbr );
    }
    if ( spatialIndexRTree )
    {
      // using the RTree spatial index
      QString mbrFilter = QString( "xmin <= %1 AND " ).arg( QString::number( rect.xMaximum(), 'f', 6 ) );
      mbrFilter += QString( "xmax >= %1 AND " ).arg( QString::number( rect.xMinimum(), 'f', 6 ) );
      mbrFilter += QString( "ymin <= %1 AND " ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      mbrFilter += QString( "ymax >= %1" ).arg( QString::number( rect.yMinimum(), 'f', 6 ) );
      QString idxName = QString( "idx_%1_%2" ).arg( mTableName ).arg( geometryColumn );
      whereClause += QString( "ROWID IN (SELECT pkid FROM %1 WHERE %2)" ).arg( idxName ).arg( mbrFilter );
    }
    else if ( spatialIndexMbrCache )
    {
      // using the MbrCache spatial index
      QString mbr = QString( "%1, %2, %3, %4" ).
                    arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      QString idxName = QString( "cache_%1_%2" ).arg( mTableName ).arg( geometryColumn );
      whereClause += QString( "ROWID IN (SELECT rowid FROM %1 WHERE mbr = FilterMbrIntersects(%2))" ).arg( idxName ).arg( mbr );
    }
    else
    {
      // using simple MBR filtering
      QString mbr = QString( "%1, %2, %3, %4" ).
                    arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( geometryColumn ).arg( mbr );
    }
  }

  if ( !whereClause.isEmpty() )
    sql += whereClause;

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &sqliteStatement, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    sqliteStatement = NULL;
    valid = false;
  }
}


QgsRectangle QgsSpatiaLiteProvider::extent()
{
  return layerExtent;
}


size_t QgsSpatiaLiteProvider::layerCount() const
{
  return 1;
}


/**
 * Return the feature type
 */
QGis::WkbType QgsSpatiaLiteProvider::geometryType() const
{
  return geomType;
}

/**
 * Return the feature type
 */
long QgsSpatiaLiteProvider::featureCount() const
{
  return numberFeatures;
}

/**
 * Return the number of fields
 */
uint QgsSpatiaLiteProvider::fieldCount() const
{
  return attributeFields.size();
}


void QgsSpatiaLiteProvider::rewind()
{
  if ( sqliteStatement )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
  }
  loadFields();
}

QgsCoordinateReferenceSystem QgsSpatiaLiteProvider::crs()
{
  QgsCoordinateReferenceSystem srs;
  srs.createFromProj4( mProj4text );
  return srs;
}


bool QgsSpatiaLiteProvider::isValid()
{
  return valid;
}


QString QgsSpatiaLiteProvider::name() const
{
  return SPATIALITE_KEY;
}                               //  QgsSpatiaLiteProvider::name()


QString QgsSpatiaLiteProvider::description() const
{
  return SPATIALITE_DESCRIPTION;
}                               //  QgsSpatiaLiteProvider::description()

const QgsFieldMap & QgsSpatiaLiteProvider::fields() const
{
  return attributeFields;
}

// Returns the minimum value of an attribute
QVariant QgsSpatiaLiteProvider::minimumValue( int index )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString minValue;
  char xSql[1024];

  // get the field name
  const QgsField & fld = field( index );

  QString sql = QString( "SELECT Min(%1) FROM %2" ).arg( fld.name() ).arg( quotedValue( mTableName ) );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      minValue = results[( i * columns ) + 0];
    }
  }
  sqlite3_free_table( results );

  if ( minValue.isEmpty() )
  {
    // NULL or not found
    return QVariant( QString::null );
  }
  else
  {
    // returning as DOUBLE
    return minValue.toDouble();
  }

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QString error = "minValue() SQL error: ";
    error = errMsg;
    QgsLogger::critical( error );
    sqlite3_free( errMsg );
  }
  return QVariant( QString::null );
}

// Returns the maximum value of an attribute
QVariant QgsSpatiaLiteProvider::maximumValue( int index )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString maxValue;
  char xSql[1024];

  // get the field name
  const QgsField & fld = field( index );

  QString sql = QString( "SELECT Max(%1) FROM %2" ).arg( fld.name() ).arg( quotedValue( mTableName ) );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      maxValue = results[( i * columns ) + 0];
    }
  }
  sqlite3_free_table( results );

  if ( maxValue.isEmpty() )
  {
    // NULL or not found
    return QVariant( QString::null );
  }
  else
  {
    // returning as DOUBLE
    return maxValue.toDouble();
  }

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QString error = "maxValue() SQL error: ";
    error = errMsg;
    QgsLogger::critical( error );
    sqlite3_free( errMsg );
  }
  return QVariant( QString::null );
}

// Returns the list of unique values of an attribute
void QgsSpatiaLiteProvider::uniqueValues( int index, QList < QVariant > &uniqueValues )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  QString sql;
  QString txt;
  char xSql[1024];

  uniqueValues.clear();

  // get the field name
  const QgsField & fld = field( index );

  sql = QString( "SELECT DISTINCT %1 FROM %2 ORDER BY %1" ).arg( fld.name() ).arg( quotedValue( mTableName ) );

  // SQLite prepared statement
  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    return;
  }

  while ( 1 )
  {
    // this one is an infinitive loop, intended to fetch any row
    int ret = sqlite3_step( stmt );

    if ( ret == SQLITE_DONE )
    {
      // there are no more rows to fetch - we can stop looping
      break;
    }

    if ( ret == SQLITE_ROW )
    {
      // fetching one column value
      switch ( sqlite3_column_type( stmt, 0 ) )
      {
        case SQLITE_INTEGER:
          uniqueValues.append( QString( "%1" ).arg( sqlite3_column_int( stmt, 0 ) ) );
          break;
        case SQLITE_FLOAT:
          uniqueValues.append( QString( "%1" ).arg( sqlite3_column_double( stmt, 0 ) ) );
          break;
        case SQLITE_TEXT:
          txt = ( const char * ) sqlite3_column_text( stmt, 0 );
          uniqueValues.append( txt );
          break;
        default:
          txt = "";
          uniqueValues.append( txt );
          break;
      };
    }
    else
    {
      // some unexpected error occurred
      const char *err = sqlite3_errmsg( sqliteHandle );
      int len = strlen( err );
      errMsg = ( char * ) sqlite3_malloc( len + 1 );
      strcpy( errMsg, err );
      goto abort;
    }
  }
  sqlite3_finalize( stmt );

  return;

abort:
  QString msg = QString( "getUniqueValues SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );
}

bool QgsSpatiaLiteProvider::addFeatures( QgsFeatureList & flist )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  QString values;
  int ia;
  char xSql[1024];

  if ( flist.size() == 0 )
    return true;
  const QgsAttributeMap & attributevec = flist[0].attributeMap();

  strcpy( xSql, "BEGIN" );
  int ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  if ( !primaryKey.isEmpty() )
  {
    sql = QString( "INSERT INTO %1 (%2, %3" ).
          arg( quotedValue( mTableName ) ).arg( quotedValue( primaryKey ) ).arg( quotedValue( geometryColumn ) );
    values = QString( ") VALUES (NULL, GeomFromWKB(?, %1)" ).arg( mSrid );
  }
  else
  {
    sql = QString( "INSERT INTO %1 (%2" ).arg( quotedValue( mTableName ) ).arg( quotedValue( geometryColumn ) );
    values = QString( ") VALUES (GeomFromWKB(?, %1)" ).arg( mSrid );
  }

  for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
  {
    QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );
    if ( fit == attributeFields.end() )
      continue;

    QString fieldname = fit->name();
    if ( fieldname.isEmpty() || fieldname == geometryColumn || fieldname == primaryKey )
      continue;

    sql += ", ";
    sql += quotedValue( fieldname );
    values += ", ?";
  }

  sql += values;
  sql += ")";

  // SQLite prepared statement
  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    return false;
  }

  for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); features++ )
  {
    // looping on each feature to insert
    const QgsAttributeMap & attributevec = features->attributeMap();

    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );

    // binding GEOMETRY to Prepared Statement
    const unsigned char *wkb = features->geometry()->asWkb();
    sqlite3_bind_blob( stmt, 1, wkb, features->geometry()->wkbSize(), SQLITE_STATIC );

    // initializing the column counter
    ia = 1;

    for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
    {
      // binding values for each attribute
      QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );
      if ( fit == attributeFields.end() )
        continue;

      QString fieldname = fit->name();
      if ( fieldname.isEmpty() || fieldname == geometryColumn || fieldname == primaryKey )
        continue;

      QVariant::Type type = fit->type();
      if ( it->toString().isEmpty() )
      {
        // assuming to be a NULL value
        type = QVariant::Invalid;
      }

      if ( type == QVariant::Int )
      {
        // binding an INTEGER value
        sqlite3_bind_int( stmt, ++ia, it->toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        sqlite3_bind_double( stmt, ++ia, it->toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        QString txt = it->toString();
        int len = txt.toUtf8().length() + 1;
        char *vl = new char [len];
        strcpy( vl, txt.toUtf8().constData() );
        sqlite3_bind_text( stmt, ++ia, vl, len, SQLITE_TRANSIENT );
        delete [] vl;
      }
      else
      {
        // binding a NULL value
        sqlite3_bind_null( stmt, ++ia );
      }
    }

    // performing actual row insert
    ret = sqlite3_step( stmt );

    if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    {
      numberFeatures++;
    }
    else
    {
      // some unexpected error occurred
      const char *err = sqlite3_errmsg( sqliteHandle );
      int len = strlen( err );
      errMsg = ( char * ) sqlite3_malloc( len + 1 );
      strcpy( errMsg, err );
      goto abort;
    }

  }
  sqlite3_finalize( stmt );

  strcpy( xSql, "COMMIT" );
  ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  return true;

abort:
  QString msg = QString( "addFeatures SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );

  if ( toCommit == true )
  {
    // ROLLBACK after some previous error
    strcpy( xSql, "ROLLBACK" );
    sqlite3_exec( sqliteHandle, xSql, NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds & id )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  char xSql[1024];

  strcpy( xSql, "BEGIN" );
  int ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  sql = QString( "DELETE FROM %1 WHERE ROWID = ?" ).arg( quotedValue( mTableName ) );

  // SQLite prepared statement
  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    return false;
  }

  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    // looping on each feature to be deleted
    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );

    sqlite3_bind_int( stmt, 1, *it );

    // performing actual row deletion
    ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    {
      numberFeatures--;
    }
    else
    {
      // some unexpected error occurred
      const char *err = sqlite3_errmsg( sqliteHandle );
      int len = strlen( err );
      errMsg = ( char * ) sqlite3_malloc( len + 1 );
      strcpy( errMsg, err );
      goto abort;
    }
  }
  sqlite3_finalize( stmt );

  strcpy( xSql, "COMMIT" );
  ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  return true;

abort:
  QString msg = QString( "deleteFeatures SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );

  if ( toCommit == true )
  {
    // ROLLBACK after some previous error
    strcpy( xSql, "ROLLBACK" );
    sqlite3_exec( sqliteHandle, xSql, NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  char xSql[1024];

  strcpy( xSql, "BEGIN" );
  int ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QString( "ALTER TABLE %1 ADD COLUMN %2 %3" )
          .arg( quotedValue( mTableName ) )
          .arg( quotedValue( iter->name() ) )
          .arg( iter->typeName() );
    strcpy( xSql, sql.toUtf8().constData() );
    ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
    if ( ret != SQLITE_OK )
    {
      // some error occurred
      goto abort;
    }
  }

  strcpy( xSql, "COMMIT" );
  ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  return true;

abort:
  QString msg = QString( "assAttributes SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );

  if ( toCommit == true )
  {
    // ROLLBACK after some previous error
    strcpy( xSql, "ROLLBACK" );
    sqlite3_exec( sqliteHandle, xSql, NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  char xSql[1024];

  strcpy( xSql, "BEGIN" );
  int ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    int fid = iter.key();

    // skip added features
    if ( fid < 0 )
      continue;

    QString sql = QString( "UPDATE %1 SET " ).arg( mTableName );
    bool first = true;

    const QgsAttributeMap & attrs = iter.value();

    // cycle through the changed attributes of the feature
    for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
    {
      QString fieldName = field( siter.key() ).name();

      if ( !first )
        sql += ",";
      else
        first = false;

      sql += QString( "%1=%2" ).arg( quotedValue( fieldName ) ).arg( quotedValue( siter->toString() ) );
    }
    sql += QString( " WHERE ROWID=%1" ).arg( fid );

    strcpy( xSql, sql.toUtf8().constData() );
    ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
    if ( ret != SQLITE_OK )
    {
      // some error occurred
      goto abort;
    }
  }

  strcpy( xSql, "COMMIT" );
  ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  return true;

abort:
  QString msg = QString( "changeAttributeValues SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );

  if ( toCommit == true )
  {
    // ROLLBACK after some previous error
    strcpy( xSql, "ROLLBACK" );
    sqlite3_exec( sqliteHandle, xSql, NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  char xSql[1024];

  strcpy( xSql, "BEGIN" );
  int ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  sql =
    QString( "UPDATE %1 SET %2 = GeomFromWKB(?, %3) WHERE ROWID = ?" ).
    arg( quotedValue( mTableName ) ).arg( quotedValue( geometryColumn ) ).arg( mSrid );

  // SQLite prepared statement
  strcpy( xSql, sql.toUtf8().constData() );
  if ( sqlite3_prepare_v2( sqliteHandle, xSql, strlen( xSql ), &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QString errCause = sqlite3_errmsg( sqliteHandle );
    QString msg = tr( "SQLite error: %1\n\nSQL: %2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    return false;
  }

  for ( QgsGeometryMap::iterator iter = geometry_map.begin(); iter != geometry_map.end(); ++iter )
  {
    // looping on each feature to change
    if ( iter->asWkb() )
    {

      // resetting Prepared Statement and bindings
      sqlite3_reset( stmt );
      sqlite3_clear_bindings( stmt );

      // binding GEOMETRY to Prepared Statement
      const unsigned char *wkb = iter->asWkb();
      sqlite3_bind_blob( stmt, 1, wkb, iter->wkbSize(), SQLITE_STATIC );
      sqlite3_bind_int( stmt, 2, iter.key() );

      // performing actual row update
      ret = sqlite3_step( stmt );
      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        ;
      else
      {
        // some unexpected error occurred
        const char *err = sqlite3_errmsg( sqliteHandle );
        int len = strlen( err );
        errMsg = ( char * ) sqlite3_malloc( len + 1 );
        strcpy( errMsg, err );
        goto abort;
      }

    }
  }
  sqlite3_finalize( stmt );

  strcpy( xSql, "COMMIT" );
  ret = sqlite3_exec( sqliteHandle, xSql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  return true;

abort:
  QString msg = QString( "addFeatures SQL error:\n%1\n" ).arg( sql );
  if ( errMsg )
  {
    msg += errMsg;
    sqlite3_free( errMsg );
  }
  else
    msg += "unknown cause";
  QgsLogger::critical( msg );

  if ( toCommit == true )
  {
    // ROLLBACK after some previous error
    strcpy( xSql, "ROLLBACK" );
    sqlite3_exec( sqliteHandle, xSql, NULL, NULL, NULL );
  }

  return false;
}


int QgsSpatiaLiteProvider::capabilities() const
{
  return enabledCapabilities;
}

void QgsSpatiaLiteProvider::closeDb()
{
// trying to close the SQLite DB
  if ( sqliteStatement )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
  }
  if ( handle )
  {
    SqliteHandles::closeDb( handle );
  }
}

QgsSpatiaLiteProvider::SqliteHandles * QgsSpatiaLiteProvider::SqliteHandles::openDb( const QString & dbPath )
{
  sqlite3 *sqlite_handle;

  QMap < QString, QgsSpatiaLiteProvider::SqliteHandles * >&handles = QgsSpatiaLiteProvider::SqliteHandles::handles;

  if ( handles.contains( dbPath ) )
  {
    QgsDebugMsg( QString( "Using cached connection for %1" ).arg( dbPath ) );
    handles[dbPath]->ref++;
    return handles[dbPath];
  }

  QgsDebugMsg( QString( "New sqlite connection for " ) + dbPath );
  if ( sqlite3_open_v2( dbPath.toUtf8().constData(), &sqlite_handle, SQLITE_OPEN_READWRITE, NULL ) )
  {
    // failure

    QString errCause = sqlite3_errmsg( sqlite_handle );
    QString msg = tr( "Failure while connecting to: %1\n\n%2" ).arg( dbPath ).arg( errCause );
    QgsLogger::critical( msg );
    return NULL;
  }

  QgsDebugMsg( "Connection to the database was successful" );

  SqliteHandles *handle = new SqliteHandles( sqlite_handle );
  handles.insert( dbPath, handle );

  return handle;
}

void QgsSpatiaLiteProvider::SqliteHandles::closeDb( SqliteHandles * &handle )
{
  closeDb( handles, handle );
}

void QgsSpatiaLiteProvider::SqliteHandles::closeDb( QMap < QString, SqliteHandles * >&handles, SqliteHandles * &handle )
{
  QMap < QString, SqliteHandles * >::iterator i;
  for ( i = handles.begin(); i != handles.end() && i.value() != handle; i++ )
    ;

  assert( i.value() == handle );
  assert( i.value()->ref > 0 );

  if ( --i.value()->ref == 0 )
  {
    i.value()->sqliteClose();
    delete i.value();
    handles.remove( i.key() );
  }

  handle = NULL;
}

void QgsSpatiaLiteProvider::SqliteHandles::sqliteClose()
{
  if ( sqlite_handle )
  {
    sqlite3_close( sqlite_handle );
    sqlite_handle = NULL;
  }
}

QString QgsSpatiaLiteProvider::quotedValue( QString value ) const
{
  if ( value.isNull() )
    return "NULL";

  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}

bool QgsSpatiaLiteProvider::getGeometryDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  char xSql[1024];

  QString sql = QString( "SELECT type, srid, spatial_index_enabled FROM geometry_columns"
                         " WHERE f_table_name=%1 and f_geometry_column=%2" ).arg( quotedValue( mTableName ) ).
                arg( quotedValue( geometryColumn ) );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString fType = results[( i * columns ) + 0];
      QString xSrid = results[( i * columns ) + 1];
      QString spatialIndex = results[( i * columns ) + 2];

      if ( fType == "POINT" )
      {
        geomType = QGis::WKBPoint;
      }
      else if ( fType == "MULTIPOINT" )
      {
        geomType = QGis::WKBMultiPoint;
      }
      else if ( fType == "LINESTRING" )
      {
        geomType = QGis::WKBLineString;
      }
      else if ( fType == "MULTILINESTRING" )
      {
        geomType = QGis::WKBMultiLineString;
      }
      else if ( fType == "POLYGON" )
      {
        geomType = QGis::WKBPolygon;
      }
      else if ( fType == "MULTIPOLYGON" )
      {
        geomType = QGis::WKBMultiPolygon;
      }
      mSrid = xSrid.toInt();
      if ( spatialIndex.toInt() == 1 )
      {
        spatialIndexRTree = true;
      }
      if ( spatialIndex.toInt() == 2 )
      {
        spatialIndexMbrCache = true;
      }

    }
  }
  sqlite3_free_table( results );

  if ( geomType == QGis::WKBUnknown || mSrid < 0 )
    goto error;

  sql = QString( "SELECT proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( mSrid );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      mProj4text = results[( i * columns ) + 0];
    }
  }
  sqlite3_free_table( results );

  return true;

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QString errCause = errMsg;
    QString msg = QString( "getGeometryDetails SQL error: %1\n\n%2" ).arg( sql ).arg( errCause );
    QgsLogger::critical( msg );
    sqlite3_free( errMsg );
  }
  return false;
}

bool QgsSpatiaLiteProvider::getTableSummary()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  char xSql[1024];

  QString sql = QString( "SELECT Min(MbrMinX(%1)), Min(MbrMinY(%1)), "
                         "Max(MbrMaxX(%1)), Max(MbrMaxY(%1)), Count(*) " "FROM %2" ).arg( geometryColumn ).arg( quotedValue( mTableName ) );

  strcpy( xSql, sql.toUtf8().constData() );
  ret = sqlite3_get_table( sqliteHandle, xSql, &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString minX = results[( i * columns ) + 0];
      QString minY = results[( i * columns ) + 1];
      QString maxX = results[( i * columns ) + 2];
      QString maxY = results[( i * columns ) + 3];
      QString count = results[( i * columns ) + 4];

      layerExtent.set( minX.toDouble(), minY.toDouble(), maxX.toDouble(), maxY.toDouble() );
      numberFeatures = count.toLong();
    }
  }
  sqlite3_free_table( results );
  return true;

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QString error = "getTableSummary() SQL error: ";
    error = errMsg;
    QgsLogger::critical( error );
    sqlite3_free( errMsg );
  }
  return false;
}

const QgsField & QgsSpatiaLiteProvider::field( int index ) const
{
  QgsFieldMap::const_iterator it = attributeFields.find( index );

  if ( it == attributeFields.constEnd() )
  {
    QgsLogger::critical( "Field " + QString::number( index ) + " not found." );
  }

  return it.value();
}



/**
 * Class factory to return a pointer to a newly created
 * QgsPostgresProvider object
 */
QGISEXTERN QgsSpatiaLiteProvider *classFactory( const QString * uri )
{
  return new QgsSpatiaLiteProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return SPATIALITE_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return SPATIALITE_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
