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

#include <qgis.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>

#include "qgsvectorlayerimport.h"
#include "qgsspatialiteprovider.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QFileInfo>
#include <QDir>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

const QString SPATIALITE_KEY = "spatialite";
const QString SPATIALITE_DESCRIPTION = "SpatiaLite data provider";

QMap < QString, QgsSpatiaLiteProvider::SqliteHandles * >QgsSpatiaLiteProvider::SqliteHandles::handles;



bool QgsSpatiaLiteProvider::convertField( QgsField &field )
{
  QString fieldType = "TEXT"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "BIGINT";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::String:
      fieldType = "TEXT";
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = "INTEGER";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = "REAL";
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = "NUMERIC";
      }
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

QgsVectorLayerImport::ImportError
QgsSpatiaLiteProvider::createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options );

  // populate members from the uri structure
  QgsDataSourceURI dsUri( uri );
  QString sqlitePath = dsUri.database();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QgsDebugMsg( "Database is: " + sqlitePath );
  QgsDebugMsg( "Table name is: " + tableName );
  QgsDebugMsg( "Geometry column is: " + geometryColumn );

  // create the table
  {
    SqliteHandles *handle;
    sqlite3 *sqliteHandle = NULL;
    char *errMsg = NULL;
    int toCommit = false;
    QString sql;

    // trying to open the SQLite DB
    spatialite_init( 0 );
    handle = SqliteHandles::openDb( sqlitePath );
    if ( handle == NULL )
    {
      QgsDebugMsg( "Connection to database failed. Import of layer aborted." );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Connection to database failed" );
      return QgsVectorLayerImport::ErrConnectionFailed;
    }

    sqliteHandle = handle->handle();

    // get the pk's name and type
    if ( primaryKey.isEmpty() )
    {
      // if no pk name was passed, define the new pk field name
      int index = 0;
      QString pk = primaryKey = "pk";
      for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
      {
        if ( fldIt.value().name() == pk )
        {
          // it already exists, try again with a new name
          primaryKey = QString( "%1_%2" ).arg( pk ).arg( index++ );
          fldIt = fields.begin();
        }
      }
    }
    else
    {
      // search for the passed field
      for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
      {
        if ( fldIt.value().name() == primaryKey )
        {
          // found, get the field type
          QgsField fld = fldIt.value();
          if ( convertField( fld ) )
          {
            primaryKeyType = fld.typeName();
          }
        }
      }
    }

    // if the field doesn't not exist yet, create it as a int field
    if ( primaryKeyType.isEmpty() )
    {
      primaryKeyType = "INTEGER";
      /* TODO
      // check the feature count to choose if create a bigint pk field
      if ( layer->featureCount() > 0xFFFFFF )
      {
        primaryKeyType = "BIGINT";
      }*/
    }

    try
    {
      int ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      toCommit = true;

      if ( overwrite )
      {
        // delete the table if exists and the related entry in geometry_columns, then re-create it
        sql = QString( "DROP TABLE IF EXISTS %1" )
              .arg( quotedIdentifier( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );

        sql = QString( "DELETE FROM geometry_columns WHERE f_table_name = %1" )
              .arg( quotedValue( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }

      sql = QString( "CREATE TABLE %1 (%2 %3 PRIMARY KEY)" )
            .arg( quotedIdentifier( tableName ) )
            .arg( quotedIdentifier( primaryKey ) )
            .arg( primaryKeyType );

      ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      // get geometry type, dim and srid
      int dim = 2;
      long srid = srs->postgisSrid();

      switch ( wkbType )
      {
        case QGis::WKBPoint25D:
          dim = 3;
        case QGis::WKBPoint:
          geometryType = "POINT";
          break;

        case QGis::WKBLineString25D:
          dim = 3;
        case QGis::WKBLineString:
          geometryType = "LINESTRING";
          break;

        case QGis::WKBPolygon25D:
          dim = 3;
        case QGis::WKBPolygon:
          geometryType = "POLYGON";
          break;

        case QGis::WKBMultiPoint25D:
          dim = 3;
        case QGis::WKBMultiPoint:
          geometryType = "MULTIPOINT";
          break;

        case QGis::WKBMultiLineString25D:
          dim = 3;
        case QGis::WKBMultiLineString:
          geometryType = "MULTILINESTRING";
          break;

        case QGis::WKBMultiPolygon25D:
          dim = 3;
        case QGis::WKBMultiPolygon:
          geometryType = "MULTIPOLYGON";
          break;

        case QGis::WKBUnknown:
          geometryType = "GEOMETRY";
          break;

        case QGis::WKBNoGeometry:
        default:
          dim = 0;
          break;
      }

      // create geometry column
      if ( !geometryType.isEmpty() )
      {
        sql = QString( "SELECT AddGeometryColumn(%1, %2, %3, %4, %5)" )
              .arg( QgsSpatiaLiteProvider::quotedValue( tableName ) )
              .arg( QgsSpatiaLiteProvider::quotedValue( geometryColumn ) )
              .arg( srid )
              .arg( QgsSpatiaLiteProvider::quotedValue( geometryType ) )
              .arg( dim );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }
      else
      {
        geometryColumn = QString();
      }

      ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

    }
    catch ( SLException &e )
    {
      QgsDebugMsg( QString( "creation of data source %1 failed. %2" )
                   .arg( tableName )
                   .arg( e.errorMessage() )
                 );

      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of data source %1 failed. %2" )
                        .arg( tableName )
                        .arg( e.errorMessage() );


      if ( toCommit )
      {
        // ROLLBACK after some previous error
        sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
      }

      SqliteHandles::closeDb( handle );
      return QgsVectorLayerImport::ErrCreateLayer;
    }

    SqliteHandles::closeDb( handle );
    QgsDebugMsg( "layer " + tableName  + " created." );
  }

  // use the provider to edit the table
  dsUri.setDataSource( "", tableName, geometryColumn, QString(), primaryKey );
  QgsSpatiaLiteProvider *provider = new QgsSpatiaLiteProvider( dsUri.uri() );
  if ( !provider->isValid() )
  {
    QgsDebugMsg( "The layer " + tableName + " just created is not valid or not supported by the provider." );
    if ( errorMessage )
      *errorMessage = QObject::tr( "loading of the layer %1 failed" )
                      .arg( tableName );

    delete provider;
    return QgsVectorLayerImport::ErrInvalidLayer;
  }

  QgsDebugMsg( "layer loaded" );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 1;

    // get the list of fields
    QList<QgsField> flist;
    for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      QgsField fld = fldIt.value();
      if ( fld.name() == primaryKey )
        continue;

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsg( "Found a field with the same name of the geometry column. Skip it!" );
        continue;
      }

      if ( !convertField( fld ) )
      {
        QgsDebugMsg( "error creating field " + fld.name() + ": unsupported type" );
        if ( errorMessage )
          *errorMessage = QObject::tr( "unsupported type for field %1" )
                          .arg( fld.name() );

        delete provider;
        return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsg( "creating field #" + QString::number( fldIt.key() ) +
                   " -> #" + QString::number( offset ) +
                   " name " + fld.name() +
                   " type " + QString( QVariant::typeToName( fld.type() ) ) +
                   " typename " + fld.typeName() +
                   " width " + QString::number( fld.length() ) +
                   " precision " + QString::number( fld.precision() ) );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIt.key(), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      QgsDebugMsg( "error creating fields " );
      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of fields failed" );

      delete provider;
      return QgsVectorLayerImport::ErrAttributeCreationFailed;
    }

    QgsDebugMsg( "Done creating fields" );
  }
  return QgsVectorLayerImport::NoError;
}



QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri )
    : QgsVectorDataProvider( uri )
    , geomType( QGis::WKBUnknown )
    , sqliteHandle( NULL )
    , sqliteStatement( NULL )
    , mSrid( -1 )
    , spatialIndexRTree( false )
    , spatialIndexMbrCache( false )
{
  nDims = GAIA_XY;
  QgsDataSourceURI anUri = QgsDataSourceURI( uri );

  // parsing members from the uri structure
  mTableName = anUri.table();
  mGeometryColumn = anUri.geometryColumn();
  mSqlitePath = anUri.database();
  mSubsetString = anUri.sql();
  mPrimaryKey = anUri.keyColumn();
  mQuery = mTableName;

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

  if ( !checkLayerType() )  // check if this one Layer is based on a Table, View or VirtualShapefile
  {
    // invalid metadata
    numberFeatures = 0;
    valid = false;

    QgsDebugMsg( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
  enabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
  if (( mTableBased || mViewBased ) &&  !mReadOnly )
  {
    // enabling editing only for Tables [excluding Views and VirtualShapes]
    enabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
    enabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
    enabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
    enabledCapabilities |= QgsVectorDataProvider::AddFeatures;
    enabledCapabilities |= QgsVectorDataProvider::AddAttributes;
  }

  if ( !getGeometryDetails() )  // gets srid and geometry type
  {
    // the table is not a geometry table
    numberFeatures = 0;
    valid = false;

    QgsDebugMsg( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
  if ( !getTableSummary() )     // gets the extent and feature count
  {
    numberFeatures = 0;
    valid = false;

    QgsDebugMsg( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
  // load the columns list
  loadFields();
  if ( sqliteHandle == NULL )
  {
    valid = false;

    QgsDebugMsg( "Invalid SpatiaLite layer" );
    return;
  }
  //fill type names into sets
  mNativeTypes
  << QgsVectorDataProvider::NativeType( tr( "Binary object (BLOB)" ), "SQLITE_BLOB", QVariant::ByteArray )
  << QgsVectorDataProvider::NativeType( tr( "Text" ), "SQLITE_TEXT", QVariant::String )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "SQLITE_FLOAT", QVariant::Double, 0, 20, 0, 20 )
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), "SQLITE_INTEGER", QVariant::LongLong, 0, 20 )
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
  sqlite3_stmt *stmt = NULL;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString pkName;
  int pkCount = 0;
  int fldNo = 0;
  QString sql;

  attributeFields.clear();

  if ( !isQuery )
  {
    mPrimaryKey.clear();

    sql = QString( "PRAGMA table_info(%1)" ).arg( quotedIdentifier( mTableName ) );

    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
      goto error;
    if ( rows < 1 )
      ;
    else
    {
      for ( i = 1; i <= rows; i++ )
      {
        QString name = QString::fromUtf8( results[( i * columns ) + 1] );
        const char *type = results[( i * columns ) + 2];
        QString pk = results[( i * columns ) + 5];
        if ( pk.toInt() != 0 )
        {
          // found a Primary Key column
          pkCount++;
          pkName = name;
        }

        if ( name != mGeometryColumn )
        {
          // for sure any SQLite value can be represented as SQLITE_TEXT
          QVariant::Type fieldType = QVariant::String;

          // making some assumptions in order to guess a more realistic type
          if ( strcasecmp( type, "int" ) == 0 ||
               strcasecmp( type, "integer" ) == 0 ||
               strcasecmp( type, "bigint" ) == 0 ||
               strcasecmp( type, "smallint" ) == 0 ||
               strcasecmp( type, "tinyint" ) == 0 ||
               strcasecmp( type, "boolean" ) == 0 )
          {
            fieldType = QVariant::Int;
          }
          else if ( strcasecmp( type, "real" ) == 0 ||
                    strcasecmp( type, "double" ) == 0 ||
                    strcasecmp( type, "double precision" ) == 0 ||
                    strcasecmp( type, "float" ) == 0 )
          {
            fieldType = QVariant::Double;
          }

          attributeFields.insert( fldNo++, QgsField( name, fieldType, type, 0, 0, "" ) );
        }
      }
    }
    sqlite3_free_table( results );
  }
  else
  {
    sql = QString( "select * from %1 limit 1" ).arg( mQuery );

    if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
      return;
    }

    ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE )
    {
      // there are no rows to fetch
      sqlite3_finalize( stmt );
      return;
    }

    if ( ret == SQLITE_ROW )
    {
      // one valid row has been fetched from the result set
      columns = sqlite3_column_count( stmt );
      for ( i = 0; i < columns; i++ )
      {
        QString name = QString::fromUtf8( sqlite3_column_name( stmt, i ) );
        const char *type = sqlite3_column_decltype( stmt, i );
        if ( type == NULL )
          type = "TEXT";

        if ( name == mPrimaryKey )
        {
          pkCount++;
          pkName = name;
        }

        if ( name != mGeometryColumn )
        {
          // for sure any SQLite value can be represented as SQLITE_TEXT
          QVariant::Type fieldType = QVariant::String;

          // making some assumptions in order to guess a more realistic type
          if ( strcasecmp( type, "int" ) == 0 ||
               strcasecmp( type, "integer" ) == 0 ||
               strcasecmp( type, "bigint" ) == 0 ||
               strcasecmp( type, "smallint" ) == 0 ||
               strcasecmp( type, "tinyint" ) == 0 ||
               strcasecmp( type, "boolean" ) == 0 )
          {
            fieldType = QVariant::Int;
          }
          else if ( strcasecmp( type, "real" ) == 0 ||
                    strcasecmp( type, "double" ) == 0 ||
                    strcasecmp( type, "double precision" ) == 0 ||
                    strcasecmp( type, "float" ) == 0 )
          {
            fieldType = QVariant::Double;
          }

          attributeFields.insert( fldNo++, QgsField( name, fieldType, type, 0, 0, "" ) );
        }
      }
    }
    sqlite3_finalize( stmt );
  }

  if ( pkCount == 1 )
  {
    // setting the Primary Key column name
    mPrimaryKey = pkName;
  }

  return;

error:
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errMsg != NULL )
  {
    sqlite3_free( errMsg );
  }
}

bool QgsSpatiaLiteProvider::hasTriggers()
{
  int ret;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString sql;

  sql = QString( "SELECT * FROM sqlite_master WHERE type='trigger' AND tbl_name=%1" )
        .arg( quotedIdentifier( mTableName ) );

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  sqlite3_free_table( results );
  return ( ret == SQLITE_OK && rows > 0 );
}


QString QgsSpatiaLiteProvider::storageType() const
{
  return "SQLite database with SpatiaLite extension";
}


bool QgsSpatiaLiteProvider::featureAtId( QgsFeatureId featureId, QgsFeature & feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  sqlite3_stmt *stmt = NULL;

  feature.setValid( false );

  QString primaryKey = !isQuery ? "ROWID" : quotedIdentifier( mPrimaryKey );
  QString whereClause = QString( "%1=%2" ).arg( primaryKey ).arg( featureId );

  if ( !mSubsetString.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " and ";

    whereClause += "(" + mSubsetString + ")";
  }

  if ( !prepareStatement( stmt, fetchAttributes, fetchGeometry, whereClause ) )
  {
    // some error occurred
    return false;
  }

  if ( !getFeature( stmt, fetchGeometry, feature, fetchAttributes ) )
  {
    sqlite3_finalize( stmt );
    return false;
  }

  sqlite3_finalize( stmt );

  feature.setValid( true );
  return true;
}

bool QgsSpatiaLiteProvider::nextFeature( QgsFeature & feature )
{
  feature.setValid( false );
  if ( !valid )
  {
    QgsDebugMsg( "Read attempt on an invalid SpatiaLite data source" );
    return false;
  }

  if ( sqliteStatement == NULL )
  {
    QgsDebugMsg( "Invalid current SQLite statement" );
    return false;
  }

  if ( !getFeature( sqliteStatement, mFetchGeom, feature, mAttributesToFetch ) )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
    return false;
  }

  feature.setValid( true );
  return true;
}

bool QgsSpatiaLiteProvider::getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                                        QgsFeature &feature,
                                        const QgsAttributeList &fetchAttributes )
{
  int ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE )
  {
    // there are no more rows to fetch
    return false;
  }
  if ( ret == SQLITE_ROW )
  {
    // one valid row has been fetched from the result set
    if ( !fetchGeometry )
    {
      // no geometry was required
      feature.setGeometryAndOwnership( 0, 0 );
    }

    feature.clearAttributeMap();

    int ic;
    int n_columns = sqlite3_column_count( stmt );
    for ( ic = 0; ic < n_columns; ic++ )
    {
      if ( ic == 0 )
      {
        // first column always contains the ROWID (or the primary key)
        QgsFeatureId fid = sqlite3_column_int64( stmt, ic );
        QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 3 );
        feature.setFeatureId( fid );
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
        if ( fetchGeometry )
        {
          if ( ic == mGeomColIdx )
          {
            if ( sqlite3_column_type( stmt, ic ) == SQLITE_BLOB )
            {
              unsigned char *featureGeom = NULL;
              size_t geom_size = 0;
              const void *blob = sqlite3_column_blob( stmt, ic );
              size_t blob_size = sqlite3_column_bytes( stmt, ic );
              convertToGeosWKB(( const unsigned char * )blob, blob_size,
                               &featureGeom, &geom_size );
              if ( featureGeom )
                feature.setGeometryAndOwnership( featureGeom, geom_size );
              else
                feature.setGeometryAndOwnership( 0, 0 );
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
    QgsMessageLog::logMessage( tr( "SQLite error getting feature: %1" ).arg( QString::fromUtf8( sqlite3_errmsg( sqliteHandle ) ) ), tr( "SpatiaLite" ) );
    return false;
  }

  return true;
}

int QgsSpatiaLiteProvider::computeSizeFromGeosWKB2D( const unsigned char *blob,
    size_t size, int type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob;
  int gsize = 5;

  p_in = blob + 5;
  switch ( type )
  {
      // compunting the required size
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
      }
      break;
    default:
      gsize += computeSizeFromMultiWKB2D( p_in, nDims, little_endian,
                                          endian_arch );
      break;
  }

  return gsize;
}

int QgsSpatiaLiteProvider::computeSizeFromMultiWKB2D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
        // compunting the required size
      case GAIA_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 2 * sizeof( double );
        break;
      case GAIA_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
        break;
      case GAIA_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 2 * sizeof( double ) );
        }
        break;
    }
  }

  return size;
}

int QgsSpatiaLiteProvider::computeSizeFromGeosWKB3D( const unsigned char *blob,
    size_t size, int type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob;
  int gsize = 5;

  p_in = blob + 5;
  switch ( type )
  {
      // compunting the required size
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    default:
      gsize += computeSizeFromMultiWKB3D( p_in, nDims, little_endian,
                                          endian_arch );
      break;
  }

  return gsize;
}

int QgsSpatiaLiteProvider::computeSizeFromMultiWKB3D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
        // compunting the required size
      case GEOS_3D_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 3 * sizeof( double );
        break;
      case GEOS_3D_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GEOS_3D_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
    }
  }

  return size;
}

void QgsSpatiaLiteProvider::convertFromGeosWKB( const unsigned char *blob,
    size_t blob_size,
    unsigned char **wkb,
    size_t *geom_size,
    int nDims )
{
// attempting to convert from 2D/3D GEOS own WKB
  int type;
  int gDims;
  int gsize;
  int little_endian;
  int endian_arch = gaiaEndianArch();

  *wkb = NULL;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GEOS_3D_POINT || type == GEOS_3D_LINESTRING
       || type == GEOS_3D_POLYGON
       || type == GEOS_3D_MULTIPOINT || type == GEOS_3D_MULTILINESTRING
       || type == GEOS_3D_MULTIPOLYGON || type == GEOS_3D_GEOMETRYCOLLECTION )
    gDims = 3;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    gDims = 2;
  else
    return;

  if ( gDims == 2 && nDims == GAIA_XY )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memset( wkbGeom, '\0', blob_size + 1 );
    memcpy( wkbGeom, blob, blob_size );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

// we need creating a GAIA WKB
  if ( gDims == 3 )
    gsize = computeSizeFromGeosWKB3D( blob, blob_size, type, nDims,
                                      little_endian, endian_arch );
  else
    gsize = computeSizeFromGeosWKB2D( blob, blob_size, type, nDims,
                                      little_endian, endian_arch );

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

  if ( gDims == 3 )
    convertFromGeosWKB3D( blob, blob_size, wkbGeom, gsize, nDims,
                          little_endian, endian_arch );
  else
    convertFromGeosWKB2D( blob, blob_size, wkbGeom, gsize, nDims,
                          little_endian, endian_arch );

  *wkb = wkbGeom;
  *geom_size = gsize;
}

void QgsSpatiaLiteProvider::convertFromGeosWKB2D( const unsigned char *blob,
    size_t blob_size,
    unsigned char *wkb,
    size_t geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 2D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 2D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
      // setting Geometry TYPE
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
      // setting Geometry values
    case GAIA_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GAIA_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GAIA_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
            // setting sub-Geometry values
          case GAIA_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}

void QgsSpatiaLiteProvider::convertFromGeosWKB3D( const unsigned char *blob,
    size_t blob_size,
    unsigned char *wkb,
    size_t geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 3D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 3D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
      // setting Geometry TYPE
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
      // setting Geometry values
    case GEOS_3D_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      p_in += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GEOS_3D_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
            // setting sub-Geometry values
          case GEOS_3D_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GEOS_3D_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              p_in += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GEOS_3D_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                p_in += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}

void QgsSpatiaLiteProvider::convertToGeosWKB( const unsigned char *blob,
    size_t blob_size,
    unsigned char **wkb,
    size_t *geom_size )
{
// attempting to convert to 2D/3D GEOS own WKB
  int type;
  int dims;
  int little_endian;
  int endian_arch = gaiaEndianArch();
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  size_t gsize = 6;
  const unsigned char *p_in;
  unsigned char *p_out;
  double coord;

  *wkb = NULL;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GAIA_POINTZ || type == GAIA_LINESTRINGZ
       || type == GAIA_POLYGONZ
       || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
       || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ )
    dims = 3;
  else if ( type == GAIA_POINTM || type == GAIA_LINESTRINGM
            || type == GAIA_POLYGONM || type == GAIA_MULTIPOINTM
            || type == GAIA_MULTILINESTRINGM || type == GAIA_MULTIPOLYGONM
            || type == GAIA_GEOMETRYCOLLECTIONM )
    dims = 3;
  else if ( type == GAIA_POINTZM || type == GAIA_LINESTRINGZM
            || type == GAIA_POLYGONZM || type == GAIA_MULTIPOINTZM
            || type == GAIA_MULTILINESTRINGZM || type == GAIA_MULTIPOLYGONZM
            || type == GAIA_GEOMETRYCOLLECTIONZM )
    dims = 4;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    dims = 2;
  else
    return;

  if ( dims == 2 )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memset( wkbGeom, '\0', blob_size + 1 );
    memcpy( wkbGeom, blob, blob_size );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

// we need creating a 3D GEOS WKB
  p_in = blob + 5;
  switch ( type )
  {
      // compunting the required size
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gsize += 3 * sizeof( double );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      gsize += points * ( 3 * sizeof( double ) );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
      }
      break;
    default:
      gsize += computeMultiWKB3Dsize( p_in, little_endian, endian_arch );
      break;
  }

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

// building GEOS 3D WKB
  *wkbGeom = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
      // setting Geometry TYPE
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POINT, 1, endian_arch );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_LINESTRING, 1, endian_arch );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
    case GAIA_POLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POLYGON, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTM:
    case GAIA_MULTIPOINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOINT, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGM:
    case GAIA_MULTILINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTILINESTRING, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONM:
    case GAIA_MULTIPOLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOLYGON, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_GEOMETRYCOLLECTION, 1, endian_arch );
      break;
  }
  p_in = blob + 5;
  p_out = wkbGeom + 5;
  switch ( type )
  {
      // setting Geometry values
    case GAIA_POINTM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      break;
    case GAIA_POINTZ:
    case GAIA_POINTZM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( type == GAIA_POINTZM )
        p_in += sizeof( double );
      break;
    case GAIA_LINESTRINGM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_LINESTRINGZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_POLYGONZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOINTM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_MULTIPOINTZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_MULTILINESTRINGM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_MULTILINESTRINGZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOLYGONM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type == GAIA_MULTIPOLYGONZM )
              p_in += sizeof( double );
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINTZ:
          case GAIA_POINTM:
          case GAIA_POINTZM:
            gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGM:
          case GAIA_LINESTRINGZM:
            gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONM:
          case GAIA_POLYGONZM:
            gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
            // setting sub-Geometry values
          case GAIA_POINTM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            break;
          case GAIA_POINTZ:
          case GAIA_POINTZM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type2 == GAIA_POINTZM )
              p_in += sizeof( double );
            break;
          case GAIA_LINESTRINGM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGZM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( type2 == GAIA_LINESTRINGZM )
                p_in += sizeof( double );
            }
            break;
          case GAIA_POLYGONM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONZM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( type2 == GAIA_POLYGONZM )
                  p_in += sizeof( double );
              }
            }
            break;
        }
      }
      break;
  }

  *wkb = wkbGeom;
  *geom_size = gsize;
}

int QgsSpatiaLiteProvider::computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian, int endian_arch )
{
// computing the required size to store a GEOS 3D MultiXX
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
        // compunting the required size
      case GAIA_POINTZ:
      case GAIA_POINTM:
        size += 3 * sizeof( double );
        p_in += 3 * sizeof( double );
        break;
      case GAIA_POINTZM:
        size += 3 * sizeof( double );
        p_in += 4 * sizeof( double );
        break;
      case GAIA_LINESTRINGZ:
      case GAIA_LINESTRINGM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GAIA_LINESTRINGZM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
        break;
      case GAIA_POLYGONZ:
      case GAIA_POLYGONM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
      case GAIA_POLYGONZM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 4 * sizeof( double ) );
        }
        break;
    }
  }

  return size;
}

QString QgsSpatiaLiteProvider::subsetString()
{
  return mSubsetString;
}

bool QgsSpatiaLiteProvider::setSubsetString( QString theSQL, bool updateFeatureCount )
{
  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;

  // update URI
  QgsDataSourceURI uri = QgsDataSourceURI( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );

  // update feature count and extents
  if ( updateFeatureCount && getTableSummary() )
  {
    return true;
  }

  mSubsetString = prevSubsetString;

  // restore URI
  uri = QgsDataSourceURI( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );

  getTableSummary();

  return false;
}

void QgsSpatiaLiteProvider::select( QgsAttributeList fetchAttributes, QgsRectangle rect, bool fetchGeometry, bool useIntersect )
{
  QString primaryKey = !isQuery ? "ROWID" : quotedIdentifier( mPrimaryKey );

  if ( !valid )
  {
    QgsDebugMsg( "Read attempt on an invalid SpatiaLite data source" );
    return;
  }

  if ( sqliteStatement != NULL )
  {
    // finalizing the current SQLite statement
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
  }

  QString whereClause;

  if ( !rect.isEmpty() && !mGeometryColumn.isNull() )
  {
    // some kind of MBR spatial filtering is required
    if ( useIntersect )
    {
      // we are requested to evaluate a true INTERSECT relationship
      QString mbr = QString( "%1, %2, %3, %4" ).
                    arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      whereClause += QString( "Intersects(%1, BuildMbr(%2)) AND " ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mbr );
    }
    if ( mVShapeBased )
    {
      // handling a VirtualShape layer
      QString mbr = QString( "%1, %2, %3, %4" ).
                    arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                    arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
      whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mbr );
    }
    else
    {
      if ( spatialIndexRTree )
      {
        // using the RTree spatial index
        QString mbrFilter = QString( "xmin <= %1 AND " ).arg( QString::number( rect.xMaximum(), 'f', 6 ) );
        mbrFilter += QString( "xmax >= %1 AND " ).arg( QString::number( rect.xMinimum(), 'f', 6 ) );
        mbrFilter += QString( "ymin <= %1 AND " ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
        mbrFilter += QString( "ymax >= %1" ).arg( QString::number( rect.yMinimum(), 'f', 6 ) );
        QString idxName = QString( "idx_%1_%2" ).arg( mIndexTable ).arg( mIndexGeometry );
        whereClause += QString( "%1 IN (SELECT pkid FROM %2 WHERE %3)" )
                       .arg( quotedIdentifier( primaryKey ) )
                       .arg( quotedIdentifier( idxName ) )
                       .arg( mbrFilter );
      }
      else if ( spatialIndexMbrCache )
      {
        // using the MbrCache spatial index
        QString mbr = QString( "%1, %2, %3, %4" ).
                      arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                      arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                      arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
        QString idxName = QString( "cache_%1_%2" ).arg( mIndexTable ).arg( mIndexGeometry );
        whereClause += QString( "%1 IN (SELECT rowid FROM %2 WHERE mbr = FilterMbrIntersects(%3))" )
                       .arg( quotedIdentifier( primaryKey ) )
                       .arg( quotedIdentifier( idxName ) )
                       .arg( mbr );
      }
      else
      {
        // using simple MBR filtering
        QString mbr = QString( "%1, %2, %3, %4" ).
                      arg( QString::number( rect.xMinimum(), 'f', 6 ) ).
                      arg( QString::number( rect.yMinimum(), 'f', 6 ) ).
                      arg( QString::number( rect.xMaximum(), 'f', 6 ) ).arg( QString::number( rect.yMaximum(), 'f', 6 ) );
        whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mbr );
      }
    }
  }

  if ( !mSubsetString.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
    {
      whereClause += " AND ";
    }
    whereClause += "( " + mSubsetString + ")";
  }

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  // preparing the SQL statement
  if ( !prepareStatement( sqliteStatement, fetchAttributes, fetchGeometry, whereClause ) )
  {
    // some error occurred
    sqliteStatement = NULL;
    return;
  }
}

bool QgsSpatiaLiteProvider::prepareStatement(
  sqlite3_stmt *&stmt,
  const QgsAttributeList &fetchAttributes,
  bool fetchGeometry,
  QString whereClause )
{
  if ( fetchGeometry && mGeometryColumn.isNull() )
  {
    return false;
  }

  QString primaryKey = !isQuery ? "ROWID" : quotedIdentifier( mPrimaryKey );

  QString sql = QString( "SELECT %1" ).arg( primaryKey );
  int colIdx = 1; // column 0 is primary key
  for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
  {
    const QgsField & fld = field( *it );
    QString fieldname = quotedIdentifier( fld.name() );
    const QString type = fld.typeName().toLower();
    if ( type.contains( "geometry" ) || type.contains( "point" ) ||
         type.contains( "line" ) || type.contains( "polygon" ) )
    {
      fieldname = QString( "AsText(%1)" ).arg( fieldname );
    }
    sql += "," + fieldname;
    colIdx++;
  }
  if ( fetchGeometry )
  {
    sql += QString( ", AsBinary(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
    mGeomColIdx = colIdx;
  }
  sql += QString( " FROM %1" ).arg( mQuery );

  if ( !whereClause.isEmpty() )
    sql += QString( " WHERE %1" ).arg( whereClause );

  if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
    return false;
  }

  return true;
}


QgsRectangle QgsSpatiaLiteProvider::extent()
{
  return layerExtent;
}

void QgsSpatiaLiteProvider::updateExtents()
{
  getTableSummary();
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

  // get the field name
  const QgsField & fld = field( index );

  QString sql = QString( "SELECT Min(%1) FROM %2" ).arg( quotedIdentifier( fld.name() ) ).arg( mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ")";
  }

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    return convertValue( fld.type(), minValue );
  }

error:
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errMsg != NULL )
  {
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

  // get the field name
  const QgsField & fld = field( index );

  QString sql = QString( "SELECT Max(%1) FROM %2" ).arg( quotedIdentifier( fld.name() ) ).arg( mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ")";
  }

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    return convertValue( fld.type(), maxValue );
  }

error:
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errMsg != NULL )
  {
    sqlite3_free( errMsg );
  }
  return QVariant( QString::null );
}

// Returns the list of unique values of an attribute
void QgsSpatiaLiteProvider::uniqueValues( int index, QList < QVariant > &uniqueValues, int limit )
{
  sqlite3_stmt *stmt = NULL;
  QString sql;
  QString txt;

  uniqueValues.clear();

  // get the field name
  QgsFieldMap::const_iterator fieldIt = attributeFields.find( index );
  if ( fieldIt == attributeFields.constEnd() )
  {
    return; //invalid field
  }
  const QgsField& fld = fieldIt.value();

  sql = QString( "SELECT DISTINCT %1 FROM %2 ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) ).arg( mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ")";
  }

  if ( limit >= 0 )
  {
    sql += QString( " LIMIT %1" ).arg( limit );
  }

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
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
          uniqueValues.append( QString::fromUtf8(( const char * ) sqlite3_column_text( stmt, 0 ) ) );
          break;
        default:
          uniqueValues.append( "" );
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return;
    }
  }

  sqlite3_finalize( stmt );

  return;
}


bool QgsSpatiaLiteProvider::addFeatures( QgsFeatureList & flist )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;
  QString values;
  QString separator;
  int ia, ret;

  if ( flist.size() == 0 )
    return true;
  const QgsAttributeMap & attributevec = flist[0].attributeMap();

  ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  sql = QString( "INSERT INTO %1(" ).arg( quotedIdentifier( mTableName ) );
  values = QString( ") VALUES (" );
  separator = "";

  if ( !mPrimaryKey.isEmpty() )
  {
    sql += separator + quotedIdentifier( mPrimaryKey );
    values += separator + "NULL";
    separator = ",";
  }

  if ( !mGeometryColumn.isNull() )
  {
    sql += separator + quotedIdentifier( mGeometryColumn );
    values += separator + QString( "GeomFromWKB(?, %2)" ).arg( mSrid );
    separator = ",";
  }

  for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
  {
    QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );
    if ( fit == attributeFields.end() )
      continue;

    QString fieldname = fit->name();
    if ( fieldname.isEmpty() || fieldname == mGeometryColumn || fieldname == mPrimaryKey )
      continue;

    sql += separator + quotedIdentifier( fieldname );
    values += separator + "?";
  }

  sql += values;
  sql += ")";

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
    return false;
  }

  for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); features++ )
  {
    // looping on each feature to insert
    const QgsAttributeMap & attributevec = features->attributeMap();

    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );

    // initializing the column counter
    ia = 0;

    if ( !mGeometryColumn.isNull() )
    {
      // binding GEOMETRY to Prepared Statement
      unsigned char *wkb = NULL;
      size_t wkb_size;
      convertFromGeosWKB( features->geometry()->asWkb(),
                          features->geometry()->wkbSize(),
                          &wkb, &wkb_size, nDims );
      if ( !wkb )
        sqlite3_bind_null( stmt, ++ia );
      else
        sqlite3_bind_blob( stmt, ++ia, wkb, wkb_size, free );
    }

    for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
    {
      // binding values for each attribute
      QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );
      if ( fit == attributeFields.end() )
        continue;

      QString fieldname = fit->name();
      if ( fieldname.isEmpty() || fieldname == mGeometryColumn || fieldname == mPrimaryKey )
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
        QByteArray ba = it->toString().toUtf8();
        sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
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

  ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  return true;

abort:
  pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ) );
  if ( errMsg )
  {
    sqlite3_free( errMsg );
  }

  if ( toCommit )
  {
    // ROLLBACK after some previous error
    sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds &id )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;

  int ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  sql = QString( "DELETE FROM %1 WHERE ROWID=?" ).arg( quotedIdentifier( mTableName ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ) );
    return false;
  }

  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    // looping on each feature to be deleted
    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );

    qint64 fid = FID_TO_NUMBER( *it );
    sqlite3_bind_int64( stmt, 1, fid );

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

  ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  return true;

abort:
  pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ) );
  if ( errMsg )
  {
    sqlite3_free( errMsg );
  }

  if ( toCommit )
  {
    // ROLLBACK after some previous error
    sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;

  int ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QString( "ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3" )
          .arg( mTableName )
          .arg( iter->name() )
          .arg( iter->typeName() );
    ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
    if ( ret != SQLITE_OK )
    {
      // some error occurred
      goto abort;
    }
  }

  ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  // reload columns
  rewind();

  return true;

abort:
  pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ) );
  if ( errMsg )
  {
    sqlite3_free( errMsg );
  }

  if ( toCommit )
  {
    // ROLLBACK after some previous error
    sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;

  int ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    QgsFeatureId fid = iter.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    QString sql = QString( "UPDATE %1 SET " ).arg( quotedIdentifier( mTableName ) );
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

      QVariant::Type type = siter->type();
      if ( siter->toString().isEmpty() )
      {
        // assuming to be a NULL value
        type = QVariant::Invalid;
      }

      if ( type == QVariant::Invalid )
      {
        // binding a NULL value
        sql += QString( "%1=NULL" ).arg( quotedIdentifier( fieldName ) );
      }
      else if ( type == QVariant::Int || type == QVariant::Double )
      {
        // binding a NUMERIC value
        sql += QString( "%1=%2" ).arg( quotedIdentifier( fieldName ) ).arg( siter->toString() );
      }
      else
      {
        // binding a TEXT value
        sql += QString( "%1=%2" ).arg( quotedIdentifier( fieldName ) ).arg( quotedValue( siter->toString() ) );
      }
    }
    sql += QString( " WHERE ROWID=%1" ).arg( fid );

    ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
    if ( ret != SQLITE_OK )
    {
      // some error occurred
      goto abort;
    }
  }

  ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }

  return true;

abort:
  pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ) );
  if ( errMsg )
  {
    sqlite3_free( errMsg );
  }

  if ( toCommit )
  {
    // ROLLBACK after some previous error
    sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
  }

  return false;
}

bool QgsSpatiaLiteProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  sqlite3_stmt *stmt = NULL;
  char *errMsg = NULL;
  bool toCommit = false;
  QString sql;

  int ret = sqlite3_exec( sqliteHandle, "BEGIN", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  toCommit = true;

  sql =
    QString( "UPDATE %1 SET %2=GeomFromWKB(?, %3) WHERE ROWID = ?" )
    .arg( quotedIdentifier( mTableName ) )
    .arg( quotedIdentifier( mGeometryColumn ) )
    .arg( mSrid );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle, sql.toUtf8().constData(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( sqliteHandle ) ), tr( "SpatiaLite" ) );
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
      unsigned char *wkb = NULL;
      size_t wkb_size;
      convertFromGeosWKB( iter->asWkb(), iter->wkbSize(), &wkb, &wkb_size,
                          nDims );
      if ( !wkb )
        sqlite3_bind_null( stmt, 1 );
      else
        sqlite3_bind_blob( stmt, 1, wkb, wkb_size, free );
      sqlite3_bind_int64( stmt, 2, FID_TO_NUMBER( iter.key() ) );

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

  ret = sqlite3_exec( sqliteHandle, "COMMIT", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    // some error occurred
    goto abort;
  }
  return true;

abort:
  pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ) );
  if ( errMsg )
  {
    sqlite3_free( errMsg );
  }

  if ( toCommit )
  {
    // ROLLBACK after some previous error
    sqlite3_exec( sqliteHandle, "ROLLBACK", NULL, NULL, NULL );
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

bool QgsSpatiaLiteProvider::SqliteHandles::checkMetadata( sqlite3 *handle )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  int spatial_type = 0;
  ret = sqlite3_get_table( handle, "SELECT CheckSpatialMetadata()", &results, &rows, &columns, NULL );
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
  if ( spatial_type == 1 )
    return true;
  return false;
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
    QgsDebugMsg( QString( "Failure while connecting to: %1\n%2" )
                 .arg( dbPath )
                 .arg( QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) ) ) );
    return NULL;
  }

  // checking the DB for sanity
  if ( !checkMetadata( sqlite_handle ) )
  {
    // failure
    QgsDebugMsg( QString( "Failure while connecting to: %1\n\ninvalid metadata tables" ).arg( dbPath ) );
    sqlite3_close( sqlite_handle );
    return NULL;
  }
  // activating Foreign Key constraints
  sqlite3_exec( sqlite_handle, "PRAGMA foreign_keys = 1", NULL, 0, NULL );

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

  Q_ASSERT( i.value() == handle );
  Q_ASSERT( i.value()->ref > 0 );

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

QString QgsSpatiaLiteProvider::quotedIdentifier( QString id )
{
  id.replace( "\"", "\"\"" );
  return id.prepend( "\"" ).append( "\"" );
}

QString QgsSpatiaLiteProvider::quotedValue( QString value )
{
  if ( value.isNull() )
    return "NULL";

  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}

bool QgsSpatiaLiteProvider::checkLayerType()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  int count = 0;

  mTableBased = false;
  mViewBased = false;
  mVShapeBased = false;
  isQuery = false;

  QString sql;

  if ( mGeometryColumn.isNull() )
  {
    // checking if is a non-spatial table
    sql = QString( "SELECT type FROM sqlite_master "
                   "WHERE lower(name) = lower(%1) "
                   "AND type in ('table', 'view') " ).arg( quotedValue( mTableName ) );

    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      QString type = QString( results[( columns ) + 0] );
      if ( type == "table" )
      {
        mTableBased = true;
        mReadOnly = false;
      }
      else if ( type == "view" )
      {
        mViewBased = true;
        mReadOnly = !hasTriggers();
      }
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = 0;
    }
    sqlite3_free_table( results );
  }
  else if ( mQuery.startsWith( "(select", Qt::CaseInsensitive ) &&
            mQuery.endsWith( ")" ) )
  {
    // checking if this one is a select query

    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegExp regex;
    do
    {
      alias = QString( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QString( "(\\\"?)%1\\1" ).arg( QRegExp::escape( alias ) );
      regex.setPattern( pattern );
      regex.setCaseSensitivity( Qt::CaseInsensitive );
    }
    while ( mQuery.contains( regex ) );

    // convert the custom query into a subquery
    mQuery = QString( "%1 as %2" )
             .arg( mQuery )
             .arg( quotedIdentifier( alias ) );

    sql = QString( "SELECT 0 FROM %1 LIMIT 1" ).arg( mQuery );
    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      isQuery = true;
      mReadOnly = true;
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = 0;
    }
    sqlite3_free_table( results );
  }
  else
  {
    // checking if this one is a Table-based layer
    sql = QString( "SELECT read_only FROM geometry_columns "
                   "LEFT JOIN geometry_columns_auth "
                   "USING (f_table_name, f_geometry_column) "
                   "WHERE f_table_name=%1 and f_geometry_column=%2" )
          .arg( quotedValue( mTableName ) )
          .arg( quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      if ( errMsg && strcmp( errMsg, "no such table: geometry_columns_auth" ) == 0 )
      {
        sqlite3_free( errMsg );
        sql = QString( "SELECT 0 FROM geometry_columns WHERE f_table_name=%1 and f_geometry_column=%2" )
              .arg( quotedValue( mTableName ) )
              .arg( quotedValue( mGeometryColumn ) );
        ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      }
    }
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mTableBased = true;
      mReadOnly = false;
      for ( i = 1; i <= rows; i++ )
      {
        if ( results[( i * columns ) + 0] != NULL )
        {
          if ( atoi( results[( i * columns ) + 0] ) != 0 )
            mReadOnly = true;
        }
      }
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = 0;
    }
    sqlite3_free_table( results );

    // checking if this one is a View-based layer
    sql = QString( "SELECT view_name, view_geometry FROM views_geometry_columns"
                   " WHERE view_name=%1 and view_geometry=%2" ).arg( quotedValue( mTableName ) ).
          arg( quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mViewBased = true;
      mReadOnly = !hasTriggers();
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = 0;
    }
    sqlite3_free_table( results );

    // checking if this one is a VirtualShapefile-based layer
    sql = QString( "SELECT virt_name, virt_geometry FROM virts_geometry_columns"
                   " WHERE virt_name=%1 and virt_geometry=%2" ).arg( quotedValue( mTableName ) ).
          arg( quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mVShapeBased = true;
      mReadOnly = true;
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = 0;
    }
    sqlite3_free_table( results );
  }

  if ( !isQuery )
  {
    mQuery = quotedIdentifier( mTableName );
  }

// checking for validity
  return count == 1;
}

bool QgsSpatiaLiteProvider::getGeometryDetails()
{
  bool ret = false;
  if ( mGeometryColumn.isNull() )
  {
    geomType = QGis::WKBNoGeometry;
    return true;
  }

  if ( mTableBased )
    ret = getTableGeometryDetails();
  if ( mViewBased )
    ret = getViewGeometryDetails();
  if ( mVShapeBased )
    ret = getVShapeGeometryDetails();
  if ( isQuery )
    ret = getQueryGeometryDetails();
  return ret;
}

bool QgsSpatiaLiteProvider::getTableGeometryDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  mIndexTable = mTableName;
  mIndexGeometry = mGeometryColumn;

  QString sql = QString( "SELECT type, srid, spatial_index_enabled, coord_dimension FROM geometry_columns"
                         " WHERE f_table_name=%1 and f_geometry_column=%2" ).arg( quotedValue( mTableName ) ).
                arg( quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
      QString dims = results[( i * columns ) + 3];

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
      if ( dims == "XY" || dims == "2" )
      {
        nDims = GAIA_XY;
      }
      else if ( dims == "XYZ" || dims == "3" )
      {
        nDims = GAIA_XY_Z;
      }
      else if ( dims == "XYM" )
      {
        nDims = GAIA_XY_M;
      }
      else if ( dims == "XYZM" )
      {
        nDims = GAIA_XY_Z_M;
      }

    }
  }
  sqlite3_free_table( results );

  if ( geomType == QGis::WKBUnknown || mSrid < 0 )
    goto error;

  return getSridDetails();

error:
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errMsg != NULL )
  {
    sqlite3_free( errMsg );
  }
  return false;
}

bool QgsSpatiaLiteProvider::getViewGeometryDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  QString sql = QString( "SELECT type, srid, spatial_index_enabled, f_table_name, f_geometry_column "
                         " FROM views_geometry_columns"
                         " JOIN geometry_columns USING (f_table_name, f_geometry_column)"
                         " WHERE view_name=%1 and view_geometry=%2" ).arg( quotedValue( mTableName ) ).
                arg( quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
      mIndexTable = results[( i * columns ) + 3];
      mIndexGeometry = results[( i * columns ) + 4];

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

  return getSridDetails();

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
    sqlite3_free( errMsg );
  }
  return false;
}

bool QgsSpatiaLiteProvider::getVShapeGeometryDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  QString sql = QString( "SELECT type, srid FROM virts_geometry_columns"
                         " WHERE virt_name=%1 and virt_geometry=%2" ).arg( quotedValue( mTableName ) ).
                arg( quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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

    }
  }
  sqlite3_free_table( results );

  if ( geomType == QGis::WKBUnknown || mSrid < 0 )
    goto error;

  return getSridDetails();

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
    sqlite3_free( errMsg );
  }
  return false;
}

bool QgsSpatiaLiteProvider::getQueryGeometryDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  QString fType( "" );
  QString xSrid( "" );

  // get stuff from the relevant column instead. This may (will?)
  // fail if there is no data in the relevant table.
  QString sql = QString( "select srid(%1), geometrytype(%1) from %2" )
                .arg( quotedIdentifier( mGeometryColumn ) )
                .arg( mQuery );

  //it is possible that the where clause restricts the feature type
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mSubsetString;
  }

  sql += " limit 1";

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      xSrid = results[( i * columns ) + 0];
      fType = results[( i * columns ) + 1];
    }
  }
  sqlite3_free_table( results );

  if ( !xSrid.isEmpty() && !fType.isEmpty() )
  {
    if ( fType == "GEOMETRY" )
    {
      // check to see if there is a unique geometry type
      sql = QString( "select distinct "
                     "case"
                     " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                     " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                     " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                     " end "
                     "from %2" )
            .arg( quotedIdentifier( mGeometryColumn ) )
            .arg( mQuery );

      if ( !mSubsetString.isEmpty() )
        sql += " where " + mSubsetString;

      ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      if ( ret != SQLITE_OK )
        goto error;
      if ( rows != 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          fType = results[( 1 * columns ) + 0];
        }
      }
      sqlite3_free_table( results );
    }

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
  }

  if ( geomType == QGis::WKBUnknown || mSrid < 0 )
    goto error;

  return getSridDetails();

error:
  // unexpected error
  if ( errMsg != NULL )
  {
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
    sqlite3_free( errMsg );
  }
  return false;
}

bool QgsSpatiaLiteProvider::getSridDetails()
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;

  QString sql = QString( "SELECT proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( mSrid );

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
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

  QString sql = QString( "SELECT Min(MbrMinX(%1)), Min(MbrMinY(%1)), "
                         "Max(MbrMaxX(%1)), Max(MbrMaxY(%1)), Count(*) " "FROM %2" )
                .arg( quotedIdentifier( mGeometryColumn ) )
                .arg( mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ")";
  }

  ret = sqlite3_get_table( sqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( errMsg ), tr( "SpatiaLite" ) );
    sqlite3_free( errMsg );
  }
  return false;
}

const QgsField & QgsSpatiaLiteProvider::field( int index ) const
{
  QgsFieldMap::const_iterator it = attributeFields.find( index );

  if ( it == attributeFields.constEnd() )
  {
    QgsDebugMsg( QString( "Field %1 not found." ).arg( index ) );
  }

  return it.value();
}



/**
 * Class factory to return a pointer to a newly created
 * QgsSpatiaLiteProvider object
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

QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsSpatiaLiteProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

// -------------

static bool initializeSpatialMetadata( sqlite3 *sqlite_handle, QString& errCause )
{
  // attempting to perform self-initialization for a newly created DB
  int ret;
  char sql[1024];
  char *errMsg = NULL;
  int count = 0;
  int i;
  char **results;
  int rows;
  int columns;

  if ( sqlite_handle == NULL )
    return false;
  // checking if this DB is really empty
  strcpy( sql, "SELECT Count(*) from sqlite_master" );
  ret = sqlite3_get_table( sqlite_handle, sql, &results, &rows, &columns, NULL );
  if ( ret != SQLITE_OK )
    return false;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
      count = atoi( results[( i * columns ) + 0] );
  }
  sqlite3_free_table( results );

  if ( count > 0 )
    return false;

  // all right, it's empty: proceding to initialize
  strcpy( sql, "SELECT InitSpatialMetadata()" );
  ret = sqlite3_exec( sqlite_handle, sql, NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to initialize SpatialMetadata:\n" );
    errCause += QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }
  spatial_ref_sys_init( sqlite_handle, 0 );
  return true;
}

QGISEXTERN bool createDb( const QString& dbPath, QString& errCause )
{
  QgsDebugMsg( "creating a new db" );

  QFileInfo fullPath = QFileInfo( dbPath );
  QDir path = fullPath.dir();
  QgsDebugMsg( QString( "making this dir: %1" ).arg( path.absolutePath() ) );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath( ) );

  // creating/opening the new database
  spatialite_init( 0 );
  sqlite3 *sqlite_handle;
  int ret = sqlite3_open_v2( dbPath.toUtf8().constData(), &sqlite_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
  if ( ret )
  {
    // an error occurred
    errCause = QObject::tr( "Could not create a new database\n" );
    errCause += QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) );
    sqlite3_close( sqlite_handle );
    return false;
  }
  // activating Foreign Key constraints
  char *errMsg = NULL;
  ret = sqlite3_exec( sqlite_handle, "PRAGMA foreign_keys = 1", NULL, 0, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to activate FOREIGN_KEY constraints [%1]" ).arg( errMsg );
    sqlite3_free( errMsg );
    sqlite3_close( sqlite_handle );
    return false;
  }
  bool init_res = ::initializeSpatialMetadata( sqlite_handle, errCause );

  // all done: closing the DB connection
  sqlite3_close( sqlite_handle );

  return init_res;
}

// -------------

QGISEXTERN bool deleteLayer( const QString& dbPath, const QString& tableName, QString& errCause )
{
  QgsDebugMsg( "deleting layer " + tableName );

  spatialite_init( 0 );
  QgsSpatiaLiteProvider::SqliteHandles* hndl = QgsSpatiaLiteProvider::SqliteHandles::openDb( dbPath );
  if ( !hndl )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }
  sqlite3* sqlite_handle = hndl->handle();

  // drop the table

  QString sql = QString( "DROP TABLE " ) + QgsSpatiaLiteProvider::quotedIdentifier( tableName );
  QgsDebugMsg( sql );
  char *errMsg = NULL;
  int ret = sqlite3_exec( sqlite_handle, sql.toUtf8().constData(), NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to delete table %1:\n" ).arg( tableName );
    errCause += QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    QgsSpatiaLiteProvider::SqliteHandles::closeDb( hndl );
    return false;
  }

  // remove table from geometry columns
  sql = QString( "DELETE FROM geometry_columns WHERE f_table_name = %1" )
        .arg( QgsSpatiaLiteProvider::quotedValue( tableName ) );
  ret = sqlite3_exec( sqlite_handle, sql.toUtf8().constData(), NULL, NULL, NULL );
  if ( ret != SQLITE_OK )
  {
    QgsDebugMsg( "sqlite error: " + QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) ) );
  }

  // TODO: remove spatial indexes?

  // run VACUUM to free unused space and compact the database
  ret = sqlite3_exec( sqlite_handle, "VACUUM", NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to run VACUUM after deleting table on database " + dbPath );
  }

  QgsSpatiaLiteProvider::SqliteHandles::closeDb( hndl );

  return true;
}
