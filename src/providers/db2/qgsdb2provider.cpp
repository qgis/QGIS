/***************************************************************************
  qgsdb2provider.cpp - Data provider for DB2 server
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
  Adapted from MSSQL provider by Tamas Szekeres
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2provider.h"
#include "qgsdb2dataitems.h"
#include "qgsdb2featureiterator.h"
#include "qgsdb2geometrycolumns.h"
#include <qgscoordinatereferencesystem.h>
#include <qgsdataitem.h>
#include <qgslogger.h>
#include "qgscrscache.h"
#include "qgscredentials.h"

static const QString PROVIDER_KEY = "DB2";
static const QString PROVIDER_DESCRIPTION = "DB2 Spatial Extender provider";

int QgsDb2Provider::sConnectionId = 0;

QgsDb2Provider::QgsDb2Provider( QString uri )
    : QgsVectorDataProvider( uri )
    , mNumberFeatures( 0 )
    , mEnvironment( ENV_LUW )
    , mWkbType( QGis::WKBUnknown )
{
  QgsDebugMsg( "uri: " + uri );
  QgsDataSourceURI anUri = QgsDataSourceURI( uri );
  if ( !anUri.srid().isEmpty() )
    mSRId = anUri.srid().toInt();
  else
    mSRId = -1;

  if ( 0 != anUri.newWkbType() )
  {
    mWkbType = QGis::fromNewWkbType( anUri.newWkbType() );
  }
  QgsDebugMsg( QString( "mWkbType: %1" ).arg( mWkbType ) );
  QgsDebugMsg( QString( "new mWkbType: %1" ).arg( anUri.newWkbType() ) );

  mValid = true;
  mSkipFailures = false;
  int dim; // Not used
  db2WkbTypeAndDimension( mWkbType, mGeometryColType, dim ); // Get DB2 geometry type name

  mFidColName = anUri.keyColumn().toUpper();
  QgsDebugMsg( "mFidColName " + mFidColName );
  mExtents = anUri.param( "extents" );
  QgsDebugMsg( "mExtents " + mExtents );

  mUseEstimatedMetadata = anUri.useEstimatedMetadata();
  QgsDebugMsg( QString( "mUseEstimatedMetadata: '%1'" ).arg( mUseEstimatedMetadata ) );
  mSqlWhereClause = anUri.sql();
  QString errMsg;
  mDatabase = getDatabase( uri, errMsg );
  mConnInfo = anUri.connectionInfo();
  QgsCoordinateReferenceSystem layerCrs = crs();
  QgsDebugMsg( "CRS: " + layerCrs.toWkt() );

  if ( !errMsg.isEmpty() )
  {
    setLastError( errMsg );
    QgsDebugMsg( mLastError );
    mValid = false;
    return;
  }

  // Create a query for default connection
  mQuery = QSqlQuery( mDatabase );

  mSchemaName = anUri.schema();

  mTableName = anUri.table().toUpper();
  QStringList sl = mTableName.split( '.' );
  if ( sl.length() == 2 )  // Never seems to be the case
  {
    mSchemaName = sl[0];
    mTableName = sl[1];
  }
  if ( mSchemaName.isEmpty() )
  {
    mSchemaName = anUri.username().toUpper();
  }

  QgsDebugMsg( "mSchemaName: '" + mSchemaName + "; mTableName: '" + mTableName );

  if ( !anUri.geometryColumn().isEmpty() )
    mGeometryColName = anUri.geometryColumn().toUpper();

  loadFields();
  updateStatistics();

  if ( mGeometryColName.isEmpty() )
  {
    // table contains no geometries
    mWkbType = QGis::WKBNoGeometry;
    mSRId = 0;
  }

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "8 Bytes integer" ), "bigint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "4 Bytes integer" ), "integer", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "2 Bytes integer" ), "smallint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "numeric", QVariant::Double, 1, 31, 0, 31 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "decimal", QVariant::Double, 1, 31, 0, 31 )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "real", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "double", QVariant::Double )

  // date/time types
  << QgsVectorDataProvider::NativeType( tr( "Date" ), "date", QVariant::Date, -1, -1, -1, -1 )
  << QgsVectorDataProvider::NativeType( tr( "Time" ), "time", QVariant::Time, -1, -1, -1, -1 )
  << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), "datetime", QVariant::DateTime, -1, -1, -1, -1 )

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "char", QVariant::String, 1, 254 )
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length (varchar)" ), "varchar", QVariant::String, 1, 32704 )
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length large object (clob)" ), "clob", QVariant::String, 1, 2147483647 )
  //DBCLOB is for 1073741824 double-byte characters, data length should be the same as CLOB (2147483647)?
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length large object (dbclob)" ), "dbclob", QVariant::String, 1, 1073741824 )
  ;
}

QgsDb2Provider::~QgsDb2Provider()
{
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

QSqlDatabase QgsDb2Provider::getDatabase( const QString &connInfo, QString &errMsg )
{
  QSqlDatabase db;
  QString service;
  QString driver;
  QString host;
  QString databaseName;
  QString port;
  QString userName;
  QString password;
  QString connectionName;
  QString connectionString;

  QgsDataSourceURI uri( connInfo );
  // Fill in the password if authentication is used
  QString expandedConnectionInfo = uri.connectionInfo( true );
  QgsDebugMsg( "expanded connInfo: " + expandedConnectionInfo );
  QgsDataSourceURI uriExpanded( expandedConnectionInfo );

  userName = uriExpanded.username();
  password = uriExpanded.password();
  service = uriExpanded.service();
  databaseName = uriExpanded.database();
  host = uriExpanded.host();
  port = uriExpanded.port();
  driver = uriExpanded.driver();
  QgsDebugMsg( QString( "driver: '%1'; host: '%2'; databaseName: '%3'" ).arg( driver, host, databaseName ) );
  if ( service.isEmpty() )
  {
    if ( driver.isEmpty() || host.isEmpty() || databaseName.isEmpty() )
    {
      QgsDebugMsg( "service not provided, a required argument is empty." );
      return db;
    }
    connectionName = databaseName + ".";
  }
  else
  {
    connectionName = service;
  }
  QgsDebugMsg( "connectionName: " + connectionName );
  /* if new database connection */
  if ( !QSqlDatabase::contains( connectionName ) )
  {
    QgsDebugMsg( "new connection. create new QODBC mapping" );
    db = QSqlDatabase::addDatabase( "QODBC3", connectionName );
  }
  else  /* if existing database connection */
  {
    QgsDebugMsg( "found existing connection, use the existing one" );
    db = QSqlDatabase::database( connectionName );
  }
  db.setHostName( host );
  db.setPort( port.toInt() );
  bool connected = false;
  int i = 0;
  QgsCredentials::instance()->lock();
  while ( !connected && i < 3 )
  {
    i++;
    // Don't prompt if this is the first time and we have both userName and password
    // This is needed for Python or any non-GUI process
    if ( userName.isEmpty() || password.isEmpty() || ( !connected && i > 1 ) )
    {
      bool ok = QgsCredentials::instance()->get( databaseName, userName,
                password, errMsg );
      if ( !ok )
      {
        errMsg = "Cancel clicked";
        QgsDebugMsg( errMsg );
        QgsCredentials::instance()->unlock();
        break;
      }
    }

    db.setUserName( userName );
    db.setPassword( password );

    /* start building connection string */
    if ( service.isEmpty() )
    {
      connectionString = QString( "Driver={%1};Hostname=%2;Port=%3;"
                                  "Protocol=TCPIP;Database=%4;Uid=%5;Pwd=%6;" )
                         .arg( driver,
                               host )
                         .arg( db.port() )
                         .arg( databaseName,
                               userName,
                               password );
    }
    else
    {
      connectionString = service;
    }
    QgsDebugMsg( "ODBC connection string: " + connectionString );

    db.setDatabaseName( connectionString ); //for QODBC driver, the name can be a DSN or connection string
    if ( db.open() )
    {
      connected = true;
      errMsg = "";
    }
    else
    {
      errMsg = db.lastError().text();
      QgsDebugMsg( "DB not open" + errMsg );
    }
  }
  if ( connected )
  {
    QgsCredentials::instance()->put( databaseName, userName, password );
  }
  QgsCredentials::instance()->unlock();

  return db;
}

bool QgsDb2Provider::openDatabase( QSqlDatabase db )
{
  QgsDebugMsg( "openDatabase" );
  if ( !db.isOpen() )
  {
    if ( !db.open() )
    {
      QgsDebugMsg( "Database could not be opened." );
      return false;
    }
  }
  return true;
}

// loadFields() gets the type from the field record
void QgsDb2Provider::loadFields()
{
  mAttributeFields.clear();
  //mDefaultValues.clear();
  QString table = QString( "%1.%2" ).arg( mSchemaName, mTableName );

  // Use the Qt functionality to get the fields and their definitions.
  QSqlRecord r = mDatabase.record( table );
  int fieldCount = r.count();

  for ( int i = 0; i < fieldCount; i++ )
  {
    QSqlField f = r.field( i );
    int typeID = f.typeID(); // seems to be DB2 numeric type id (standard?)
    QString sqlTypeName = db2TypeName( typeID );
    QVariant::Type sqlType = f.type();
    QgsDebugMsg( QString( "name: %1; length: %2; sqlTypeID: %3; sqlTypeName: %4" )
                 .arg( f.name() ).arg( f.length() ).arg( QString::number( typeID ), sqlTypeName ) );
    if ( f.name() == mGeometryColName ) continue; // Got this with uri, just skip
    if ( sqlType == QVariant::String )
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName,
          f.length()
        ) );
    }
    else if ( sqlType == QVariant::Double )
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName,
          f.length(),
          f.precision()
        ) );
    }
    else
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName
        ) );
    }

    if ( !f.defaultValue().isNull() )
    {
      mDefaultValues.insert( i, f.defaultValue() );
    }
// Hack to get primary key since the primaryIndex function above doesn't work
// on z/OS. Pick first integer column.
    if ( mFidColName.length() == 0 &&
         ( sqlType == QVariant::LongLong || sqlType == QVariant::Int ) )
    {
      mFidColName = f.name();
    }
  }
}

QVariant::Type QgsDb2Provider::decodeSqlType( int typeId )
{
  QVariant::Type type = QVariant::Invalid;
  switch ( typeId )
  {
    case -5:     //BIGINT
      type = QVariant::LongLong;
      break;

    case -3:     //VARBINARY
      type = QVariant::ByteArray;
      break;

    case 1:     //CHAR
    case 12:    //VARCHAR
      type = QVariant::String;
      break;

    case 4:     //INTEGER
      type = QVariant::Int;
      break;

    case 3:     //NUMERIC and DECIMAL
    case 7:     //REAL
    case 8:     //DOUBLE
      type = QVariant::Double;
      break;

    case 9:    //DATE
      type = QVariant::String; // don't know why it doesn't like QVariant::Date
      break;

    case 10:    //TIME
      type = QVariant::Time;
      break;

    case 11:    //TIMESTAMP
      type = QVariant::String; // don't know why it doesn't like QVariant::DateTime
      break;

    default:
      // Everything else just dumped as a string.
      type = QVariant::String;
  }

  return type;
}

// Return the DB2 type name for the type numeric value
QString QgsDb2Provider::db2TypeName( int typeId )
{
  QString typeName = "";
  switch ( typeId )
  {
    case -3:     //VARBINARY
      typeName = "VARBINARY"; // also for spatial types
      break;

    case 1:     //CHAR
      typeName = "CHAR";
      break;

    case 12:    //VARCHAR
      typeName = "VARCHAR";
      break;

    case 4:     //INTEGER
      typeName = "INTEGER";
      break;

    case -5:     //BIGINT
      typeName = "BIGINT";
      break;

    case 3:     //NUMERIC and DECIMAL
      typeName =  "DECIMAL";
      break;

    case 7:     //REAL
      typeName =  "REAL";
      break;

    case 8:     //DOUBLE
      typeName =  "DOUBLE";
      break;

    case 9:    //DATE
      typeName =  "DATE";
      break;

    case 10:    //TIME
      typeName =  "TIME";
      break;

    case 11:    //TIMESTAMP
      typeName =  "TIMESTAMP";
      break;

    default:
      typeName = "UNKNOWN";
  }

  return typeName;
}

QgsAbstractFeatureSource* QgsDb2Provider::featureSource() const
{
  return new QgsDb2FeatureSource( this );
}

QgsFeatureIterator QgsDb2Provider::getFeatures( const QgsFeatureRequest& request )
{
  if ( !mValid )
  {
    QgsDebugMsg( "Read attempt on an invalid db2 data source" );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsDb2FeatureIterator( new QgsDb2FeatureSource( this ), true, request ) );
}

QGis::WkbType QgsDb2Provider::geometryType() const
{
  return mWkbType;
}

long QgsDb2Provider::featureCount() const
{
  // Return the count that we get from the subset.
  if ( !mSqlWhereClause.isEmpty() )
    return mNumberFeatures;

  // On LUW, this could be selected from syscat.tables but I'm not sure if this
  // is actually correct if RUNSTATS hasn't been done.
  // On z/OS, we don't have access to the system catalog.
  // Use count(*) as the easiest approach
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  QString sql = "SELECT COUNT(*) FROM %1.%2";
  QString statement = QString( sql ).arg( mSchemaName, mTableName );
  QgsDebugMsg( statement );
  if ( query.exec( statement ) && query.next() )
  {
    QgsDebugMsg( QString( "count: %1" ).arg( query.value( 0 ).toInt() ) );
    return query.value( 0 ).toInt();
  }
  else
  {
    QgsDebugMsg( "Failed" );
    QgsDebugMsg( query.lastError().text() );
    return -1;
  }
}

const QgsFields &QgsDb2Provider::fields() const
{
  return mAttributeFields;
}

QgsCoordinateReferenceSystem QgsDb2Provider::crs()
{
  if ( !mCrs.isValid() && mSRId > 0 )
  {
    mCrs.createFromSrid( mSRId );
    if ( mCrs.isValid() )
    {
      return mCrs;
    }

    // try to load crs from the database tables as a fallback
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );
    bool execOk = query.exec( QString( "SELECT DEFINITION FROM DB2GSE.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() )
    {
      if ( query.next() )
      {
        mCrs = QgsCRSCache::instance()->crsByWkt( query.value( 0 ).toString() );
        if ( mCrs.isValid() )
          return mCrs;
      }
    }
  }
  return mCrs;
}

// update the extent for this layer
void QgsDb2Provider::updateStatistics()
{
  // get features to calculate the statistics
  QString statement;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  statement = QString( "SELECT MIN(DB2GSE.ST_MINX(%1)), MIN(DB2GSE.ST_MINY(%1)), MAX(DB2GSE.ST_MAXX(%1)), MAX(DB2GSE.ST_MAXY(%1))" ).arg( mGeometryColName );

  statement += QString( " FROM %1.%2" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    statement += " WHERE (" + mSqlWhereClause + ")";
  }
  QgsDebugMsg( statement );

  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( query.lastError().text() );
  }

  if ( !query.isActive() )
  {
    return;
  }

  if ( query.next() )
  {
    mExtent.setXMinimum( query.value( 0 ).toDouble() );
    mExtent.setYMinimum( query.value( 1 ).toDouble() );
    mExtent.setXMaximum( query.value( 2 ).toDouble() );
    mExtent.setYMaximum( query.value( 3 ).toDouble() );
    QgsDebugMsg( QString( "after setting; mExtent: %1" ).arg( mExtent.toString() ) );
  }

  QgsDebugMsg( QString( "mSRId: %1" ).arg( mSRId ) );
  QgsDb2GeometryColumns gc( mDatabase );
  int rc = gc.open( mSchemaName, mTableName );  // returns SQLCODE if failure
  if ( rc == 0 )
  {
    mEnvironment = gc.db2Environment();
    if ( -1 == mSRId )
    {
      QgsDb2LayerProperty layer;
      gc.populateLayerProperty( layer );
      if ( !layer.srid.isEmpty() )
      {
        mSRId = layer.srid.toInt();
        mSrsName = layer.srsName;
      }
      mGeometryColType = layer.type;
      QgsDebugMsg( QString( "srs_id: %1; srs_name: %2; mGeometryColType: %3" )
                   .arg( mSRId ).arg( mSrsName, mGeometryColType ) );
      return;
    }
  }
  else
  {
    QgsDebugMsg( "Couldn't get srid from geometry columns" );
  }

  // Try to get the srid from the data if srid isn't already set
  QgsDebugMsg( QString( "mSRId: %1" ).arg( mSRId ) );
  if ( -1 == mSRId )
  {
    query.clear();
    statement = QString( "SELECT DB2GSE.ST_SRID(%1) FROM %2.%3 FETCH FIRST ROW ONLY" )
                .arg( mGeometryColName, mSchemaName, mTableName );

    QgsDebugMsg( statement );

    if ( !query.exec( statement ) || !query.isActive() )
    {
      QgsDebugMsg( query.lastError().text() );
    }

    if ( query.next() )
    {
      mSRId = query.value( 0 ).toInt();
      QgsDebugMsg( QString( "srid from data: %1" ).arg( mSRId ) );
      return;
    }
    else
    {
      QgsDebugMsg( "Couldn't get srid from data" );
    }
  }
}

QgsRectangle QgsDb2Provider::extent()
{
  QgsDebugMsg( QString( "entering; mExtent: %1" ).arg( mExtent.toString() ) );
  if ( mExtent.isEmpty() )
    updateStatistics();
  return mExtent;
}

bool QgsDb2Provider::isValid()
{
  return true; //DB2 only has valid geometries
}

QString QgsDb2Provider::subsetString()
{
  return mSqlWhereClause;
}

bool QgsDb2Provider::setSubsetString( const QString& theSQL, bool )
{
  QString prevWhere = mSqlWhereClause;
  QgsDebugMsg( theSQL );
  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "SELECT COUNT(*) FROM " );

  sql += QString( "%1.%2" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
  }

  if ( !openDatabase( mDatabase ) )
  {
    return false;
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QgsDebugMsg( sql );
  if ( !query.exec( sql ) )
  {
    pushError( query.lastError().text() );
    mSqlWhereClause = prevWhere;
    QgsDebugMsg( query.lastError().text() );
    return false;
  }

  if ( query.isActive() && query.next() )
  {
    mNumberFeatures = query.value( 0 ).toInt();
    QgsDebugMsg( QString( "count: %1" ).arg( mNumberFeatures ) );
  }
  else
  {
    pushError( query.lastError().text() );
    mSqlWhereClause = prevWhere;
    QgsDebugMsg( query.lastError().text() );
    return false;
  }

  QgsDataSourceURI anUri = QgsDataSourceURI( dataSourceUri() );
  anUri.setSql( mSqlWhereClause );

  setDataSourceUri( anUri.uri() );

  mExtent.setMinimal();

  emit dataChanged();

  return true;
}

void QgsDb2Provider::db2WkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim )
{
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBPoint:
      geometryType = "ST_POINT";
      break;

    case QGis::WKBLineString25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBLineString:
      geometryType = "ST_LINESTRING";
      break;

    case QGis::WKBPolygon25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBPolygon:
      geometryType = "ST_POLYGON";
      break;

    case QGis::WKBMultiPoint25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBMultiPoint:
      geometryType = "ST_MULTIPOINT";
      break;

    case QGis::WKBMultiLineString25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBMultiLineString:
      geometryType = "ST_MULTILINESTRING";
      break;

    case QGis::WKBMultiPolygon25D:
      dim = 3;
      FALLTHROUGH;
    case QGis::WKBMultiPolygon:
      geometryType = "ST_MULTIPOLYGON";
      break;

    case QGis::WKBUnknown:
      geometryType = "ST_GEOMETRY";
      break;

    case QGis::WKBNoGeometry:
    default:
      dim = 0;
      break;
  }
}

bool QgsDb2Provider::deleteFeatures( const QgsFeatureIds & id )
{
  if ( mFidColName.isEmpty() )
    return false;

  QString featureIds;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( featureIds.isEmpty() )
      featureIds = FID_TO_STRING( *it );
    else
      featureIds += ',' + FID_TO_STRING( *it );
  }

  if ( !mDatabase.isOpen() )
  {
    QString errMsg;
    mDatabase = getDatabase( mConnInfo, errMsg );
    if ( !errMsg.isEmpty() )
    {
      return false;
    }
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QString statement;
  statement = QString( "DELETE FROM %1.%2 WHERE %3 IN (%4)" ).arg( mSchemaName,
              mTableName, mFidColName, featureIds );
  QgsDebugMsg( statement );
  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( query.lastError().text() );
    return false;
  }

  return true;
}


bool QgsDb2Provider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  QgsDebugMsg( "Entering" );
  if ( attr_map.isEmpty() )
    return true;

  if ( mFidColName.isEmpty() )
    return false;

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    const QgsAttributeMap& attrs = it.value();
    if ( attrs.isEmpty() )
      continue;

    QString statement = QString( "UPDATE %1.%2 SET " ).arg( mSchemaName, mTableName );

    bool first = true;
    if ( !mDatabase.isOpen() )
    {
      QString errMsg;
      mDatabase = getDatabase( mConnInfo, errMsg );
      if ( !errMsg.isEmpty() )
      {
        return false;
      }
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( !first )
        statement += ',';
      else
        first = false;

      statement += QString( "%1=?" ).arg( fld.name() );
    }

    if ( first )
      return true; // no fields have been changed

    // set attribute filter
    statement += QString( " WHERE %1=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QgsDebugMsg( query.lastError().text() );
      return false;
    }
    QgsDebugMsg( statement );
    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.name().isEmpty() )
        continue; // invalid

      QVariant::Type type = fld.type();
      if ( it2->isNull() || !it2->isValid() )
      {
        // binding null values
        if ( type == QVariant::Date || type == QVariant::DateTime )
          query.addBindValue( QVariant( QVariant::String ) );
        else
          query.addBindValue( QVariant( type ) );
      }
      else if ( type == QVariant::Int )
      {
        // binding an INTEGER value
        query.addBindValue( it2->toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        query.addBindValue( it2->toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        query.addBindValue( it2->toString() );
      }
      else if ( type == QVariant::DateTime )
      {
        // binding a DATETIME value
        query.addBindValue( it2->toDateTime().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::Date )
      {
        // binding a DATE value
        query.addBindValue( it2->toDate().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::Time )
      {
        // binding a TIME value
        query.addBindValue( it2->toTime().toString( Qt::ISODate ) );
      }
      else
      {
        query.addBindValue( *it2 );
      }
    }

    if ( !query.exec() )
    {
      QgsDebugMsg( query.lastError().text() );
      return false;
    }
  }
  return true;
}

bool QgsDb2Provider::addFeatures( QgsFeatureList & flist )
{
  QgsDebugMsg( "mGeometryColType: " + mGeometryColType );
  int writeCount = 0;
  bool copyOperation = false;

  if ( !mDatabase.isOpen() )
  {
    QString errMsg;
    mDatabase = getDatabase( mConnInfo, errMsg );
    if ( !errMsg.isEmpty() )
    {
      QgsDebugMsg( "getDatabase failed: " + errMsg );
      return false;
    }
  }
  mDatabase.transaction();
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QSqlQuery queryFid = QSqlQuery( mDatabase );
  queryFid.setForwardOnly( true );

  QgsFeature it = flist.at( 0 );
  QString statement;
  QString values;
  statement = QString( "INSERT INTO %1.%2 (" ).arg( mSchemaName, mTableName );

  bool first = true;

// Get the first geometry and its wkbType as when we are doing drag/drop,
// the wkbType is not passed to the DB2 provider from QgsVectorLayerImport
// Can't figure out how to resolved "unreferenced" wkbType compile message
// Don't really do anything with it at this point
#if 0
  QgsGeometry *geom = it.geometry();
  QGis::WkbType wkbType = geom->wkbType();
  QgsDebugMsg( QString( "wkbType: %1" ).arg( wkbType ) );
  QgsDebugMsg( QString( "mWkbType: %1" ).arg( mWkbType ) );
#endif

  QgsAttributes attrs = it.attributes();
  QgsDebugMsg( QString( "attrs.count: %1" ).arg( attrs.count() ) );
  QgsDebugMsg( QString( "fields.count: %1" ).arg( mAttributeFields.count() ) );
  if ( mAttributeFields.count() == ( attrs.count() + 1 ) )
  {
    copyOperation = true; // FID is first field but no attribute in attrs
  }
  else if ( mAttributeFields.count() !=  attrs.count() )
  {
    QgsDebugMsg( "Count mismatch - failing" );
    return false;
  }


  if ( attrs.count() != mAttributeFields.count() )
  {
    QgsDebugMsg( "field counts don't match" );
//  return false;
  }

  for ( int i = 0; i < mAttributeFields.count(); ++i )
  {
    QgsField fld = mAttributeFields.at( i );
    QgsDebugMsg( QString( "i: %1; got field: %2" ).arg( i ).arg( fld.name() ) );

    if ( fld.name().isEmpty() )
      continue; // invalid

    if ( mFidColName == fld.name() )
      continue; // skip identity field

//      if ( mDefaultValues.contains( i ) && mDefaultValues[i] == attrs.at( i ) )
//        continue; // skip fields having default values

    if ( !first )
    {
      statement += ',';
      values += ',';
    }
    else
      first = false;

    statement += QString( "%1" ).arg( fld.name() );
    values += QString( "?" );
  }

  // append geometry column name
  if ( !mGeometryColName.isEmpty() )
  {
    if ( !first )
    {
      statement += ',';
      values += ',';
    }

    statement += QString( "%1" ).arg( mGeometryColName );

    values += QString( "db2gse.%1(CAST (%2 AS BLOB(2M)),%3)" )
              .arg( mGeometryColType,
                    QString( "?" ),
                    QString::number( mSRId ) );
  }

  QgsDebugMsg( statement );
  QgsDebugMsg( values );
  statement += ") VALUES (" + values + ')';
  QgsDebugMsg( statement );

  QgsDebugMsg( "Prepare statement" );
  // use prepared statement to prevent from sql injection
  if ( !query.prepare( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    pushError( msg );
    return false;
  }


  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    attrs = it->attributes();

    int fieldIdx = 0;
    if ( copyOperation )
    {
      fieldIdx = 1;  // skip first (FID) field if copying from shapefile
    }
    int bindIdx = 0;
    for ( int i = 0; i < attrs.count(); i++ )
    {
      QgsField fld = mAttributeFields.at( fieldIdx++ );
      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mFidColName == fld.name() )
        continue; // skip identity field

//      if ( mDefaultValues.contains( i ) && mDefaultValues[i] == attrs.at( i ) )
//        continue; // skip fields having default values

      QVariant::Type type = fld.type();
      if ( attrs.at( i ).isNull() || !attrs.at( i ).isValid() )
      {
        // binding null values
        if ( type == QVariant::Date || type == QVariant::DateTime )
          query.bindValue( bindIdx,  QVariant( QVariant::String ) );
        else
          query.bindValue( bindIdx,  QVariant( type ) );
      }
      else if ( type == QVariant::Int )
      {
        // binding an INTEGER value
        query.bindValue( bindIdx,  attrs.at( i ).toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        query.bindValue( bindIdx,  attrs.at( i ).toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        query.bindValue( bindIdx,  attrs.at( i ).toString() );
      }
      else if ( type == QVariant::Time )
      {
        // binding a TIME value
        query.bindValue( bindIdx,  attrs.at( i ).toTime().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::Date )
      {
        // binding a DATE value
        query.bindValue( bindIdx,  attrs.at( i ).toDate().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::DateTime )
      {
        // binding a DATETIME value
        query.bindValue( bindIdx,  attrs.at( i ).toDateTime().toString( Qt::ISODate ) );
      }
      else
      {
        query.bindValue( bindIdx,  attrs.at( i ) );
      }

#if 0
      QgsDebugMsg( QString( "bound i: %1; name: %2; value: %3; bindIdx: %4" ).
                   arg( i ).arg( fld.name() ).arg( attrs.at( i ).toString() ).arg( bindIdx ) );
#endif
      bindIdx++;
    }

    if ( !mGeometryColName.isEmpty() )
    {
      const QgsGeometry *geom = it->constGeometry();

      QByteArray bytea = QByteArray(( char* )geom->asWkb(), ( int ) geom->wkbSize() );
      query.bindValue( bindIdx,  bytea, QSql::In | QSql::Binary );
    }

    QList<QVariant> list = query.boundValues().values();

// Show bound values
#if 0
    for ( int i = 0; i < list.size(); ++i )
    {
      QgsDebugMsg( QString( "i: %1; value: %2; type: %3" )
                   .arg( i ).arg( list.at( i ).toString().toAscii().data() ).arg( list.at( i ).typeName() ) );
    }
#endif
    if ( !query.exec() )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }

    statement = QString( "select IDENTITY_VAL_LOCAL() AS IDENTITY "
                         "FROM SYSIBM.SYSDUMMY1" );
//    QgsDebugMsg( statement );
    if ( !queryFid.exec( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }

    if ( !queryFid.next() )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }
    it->setFeatureId( queryFid.value( 0 ).toLongLong() );
    writeCount++;
//    QgsDebugMsg( QString( "count: %1; featureId: %2" ).arg( writeCount ).arg( queryFid.value( 0 ).toLongLong() ) );
  }
  bool commitStatus = mDatabase.commit();
  QgsDebugMsg( QString( "commitStatus: %1; write count: %2; featureId: %3" )
               .arg( commitStatus ).arg( writeCount ).arg( queryFid.value( 0 ).toLongLong() ) );
  if ( !commitStatus )
  {
    pushError( "Commit of new features failed" );
    return false;
  }
  return true;
}

int QgsDb2Provider::capabilities() const
{
  int cap = AddFeatures;
  bool hasGeom = false;
  if ( !mGeometryColName.isEmpty() )
  {
    hasGeom = true;
//    cap |= CreateSpatialIndex;
  }

  if ( mFidColName.isEmpty() )
    return cap;
  else
  {
    if ( hasGeom )
      cap |= ChangeGeometries | QgsVectorDataProvider::SelectGeometryAtId;

    return cap | DeleteFeatures | ChangeAttributeValues |
           QgsVectorDataProvider::SelectAtId;
  }
}

bool QgsDb2Provider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( geometry_map.isEmpty() )
    return true;

  if ( mFidColName.isEmpty() )
    return false;

  for ( QgsGeometryMap::const_iterator it = geometry_map.constBegin(); it != geometry_map.constEnd(); ++it )
  {
    QgsFeatureId fid = it.key();
    // skip added features
    if ( FID_IS_NEW( fid ) )
    {
      continue;
    }

    QString statement;
    statement = QString( "UPDATE %1.%2 SET %3 = " )
                .arg( mSchemaName, mTableName, mGeometryColName );

    if ( !mDatabase.isOpen() )
    {
      QString errMsg;
      mDatabase = getDatabase( mConnInfo, errMsg );
      if ( !errMsg.isEmpty() )
      {
        return false;
      }
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    statement += QString( "db2gse.%1(CAST (%2 AS BLOB(2M)),%3)" )
                 .arg( mGeometryColType,
                       QString( "?" ),
                       QString::number( mSRId ) );

    // set attribute filter
    statement += QString( " WHERE %1=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );
    QgsDebugMsg( statement );
    if ( !query.prepare( statement ) )
    {
      QgsDebugMsg( query.lastError().text() );
      return false;
    }

    // add geometry param
    QByteArray bytea = QByteArray(( char* )it->asWkb(), ( int ) it->wkbSize() );
    query.addBindValue( bytea, QSql::In | QSql::Binary );

    if ( !query.exec() )
    {
      QgsDebugMsg( query.lastError().text() );
      return false;
    }
  }

  return true;
}

QgsVectorLayerImport::ImportError QgsDb2Provider::createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
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

  QString connInfo = dsUri.connectionInfo();
  QString errMsg;
  QString srsName;
  QgsDebugMsg( "uri: " + uri );

  // connect to database
  QSqlDatabase db = QgsDb2Provider::getDatabase( connInfo, errMsg );

  if ( !errMsg.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = errMsg;
    return QgsVectorLayerImport::ErrConnectionFailed;
  }

  // Get the SRS name using srid, needed to register the spatial column
  // srs->posgisSrid() seems to return the authority id which is
  // most often the EPSG id.  Hopefully DB2 has defined an SRS using this
  // value as the srid / srs_id.  If not, we are out of luck.
  QgsDebugMsg( "srs: " + srs->toWkt() );
  long srid = srs->postgisSrid();
  QgsDebugMsg( QString( "srid: %1" ).arg( srid ) );
  if ( srid >= 0 )
  {
    QSqlQuery query( db );
    QString statement = QString( "SELECT srs_name FROM db2gse.st_spatial_reference_systems where srs_id=%1" )
                        .arg( srid );
    QgsDebugMsg( statement );

    if ( !query.exec( statement ) || !query.isActive() )
    {
      QgsDebugMsg( query.lastError().text() );
    }

    if ( query.next() )
    {
      srsName = query.value( 0 ).toString();
      QgsDebugMsg( QString( "srs_name: %1" ).arg( srsName ) );
    }
    else
    {
      QgsDebugMsg( "Couldn't get srs_name from db2gse.st_spatial_reference_systems" );
    }
  }

  QString schemaName = dsUri.schema().toUpper();
  QString tableName = dsUri.table().toUpper();
  QString fullName;

  if ( schemaName.isEmpty() )
  {
    schemaName = dsUri.username().toUpper();  // set schema to user name
  }
  fullName = schemaName + "." + tableName;

  QString geometryColumn = dsUri.geometryColumn().toUpper();
  QString primaryKey = dsUri.keyColumn().toUpper();
  QString primaryKeyType;

  // TODO - this is a bad hack to cope with shapefiles.
  // The wkbType from the shapefile header is usually a multi-type
  // even if all the data is a single-type. If we create the column as
  // a multi-type, the insert will fail if the actual data is a single-type
  // due to type mismatch.
  // We could potentially defer adding the spatial column until addFeatures is
  // called the first time, but QgsVectorLayerImport doesn't pass the CRS/srid
  // information to the DB2 provider and we need this information to register
  // the spatial column.
  // This hack is problematic because the drag/drop will fail if the
  // actual data is a multi-type which is possible with a shapefile or
  // other data source.
  QGis::WkbType wkbTypeSingle;
  wkbTypeSingle = QGis::singleType( wkbType );
  if ( wkbType != QGis::WKBNoGeometry && geometryColumn.isEmpty() )
    geometryColumn = "GEOM";

  if ( primaryKey.isEmpty() )
    primaryKey = "QGS_FID";

  // get the pk's name and type
  // if no pk name was passed, define the new pk field name
  int fieldCount = fields.size();
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "QGS_FID";
    for ( int i = 0; i < fieldCount; ++i )
    {
      if ( fields[i].name() == primaryKey )
      {
        // it already exists, try again with a new name
        primaryKey = QString( "%1_%2" ).arg( pk ).arg( index++ );
        i = 0;
      }
    }
  }
  else
  {
    // search for the passed field
    for ( int i = 0; i < fieldCount; ++i )
    {
      if ( fields[i].name() == primaryKey )
      {
        // found, get the field type
        QgsField fld = fields[i];
        if ( convertField( fld ) )
        {
          primaryKeyType = fld.typeName();
        }
      }
    }
  }
  QgsDebugMsg( "primaryKeyType: '" + primaryKeyType + "'" );

  QString sql;
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );

  // get wkb type and dimension
  QString geometryType;
  int dim = 2;
  db2WkbTypeAndDimension( wkbTypeSingle, geometryType, dim );
  QgsDebugMsg( QString( "wkbTypeSingle: %1; geometryType: %2" ).arg( wkbTypeSingle ).arg( geometryType ) );
  if ( overwrite )
  {
    // remove the old table with the same name
    sql = "DROP TABLE " + fullName;
    if ( !q.exec( sql ) )
    {
      if ( q.lastError().number() != -206 ) // -206 is "not found" just ignore
      {
        QString lastError = q.lastError().text();
        QgsDebugMsg( lastError );
        if ( errorMessage )
        {
          *errorMessage = lastError;
        }
        return QgsVectorLayerImport::ErrCreateLayer;
      }
    }
  }

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();
  QString attr2Create = "";
  if ( fields.size() > 0 )
  {
    int offset = 0;

    // get the list of fields
    QgsDebugMsg( "PrimaryKey: '" + primaryKey + "'" );
    for ( int i = 0; i < fieldCount; ++i )
    {
      QgsField fld = fields.field( i );
      QgsDebugMsg( QString( "i: %1; fldIdx: %2; offset: %3" )
                   .arg( i ).arg( fields.fieldNameIndex( fld.name() ) ).arg( offset ) );

      if ( oldToNewAttrIdxMap && fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( i , 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
      {
        // Found a field with the same name of the geometry column. Skip it!
        continue;
      }
      QString db2Field = qgsFieldToDb2Field( fld );

      if ( db2Field.isEmpty() )
      {
        if ( errorMessage )
        {
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );
        }
        return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
      }

      if ( oldToNewAttrIdxMap )
      {
        oldToNewAttrIdxMap->insert( fields.fieldNameIndex( fld.name() ), offset++ );
      }
      attr2Create += ',' + db2Field.toUpper();
    }
    QgsDebugMsg( attr2Create );
    if ( !geometryColumn.isEmpty() )
    {
      sql = QString( // need to set specific geometry type
              "CREATE TABLE %1(%2 BIGINT NOT NULL PRIMARY KEY GENERATED ALWAYS AS IDENTITY, "
              "%3 DB2GSE.%4 %5) " )
            .arg( fullName,
                  primaryKey,
                  geometryColumn,
                  geometryType,
                  attr2Create );
    }
    else
    {
      //geometryless table
      sql = QString( // need to set specific geometry type
              "CREATE TABLE %1.%2(%3 INTEGER NOT NULL PRIMARY KEY GENERATED ALWAYS %4) " )
            .arg( schemaName,
                  tableName,
                  primaryKey,
                  attr2Create );
    }
    QgsDebugMsg( sql );
    if ( !q.exec( sql ) )
    {
      QString lastError = q.lastError().text();
      QgsDebugMsg( lastError );
      if ( errorMessage )
      {
        *errorMessage = lastError;
      }
      return QgsVectorLayerImport::ErrCreateLayer;
    }


    if ( !geometryColumn.isEmpty() )
    {
      int computeExtents = 0;
      int msgCode = 0;
      int outCode;
      int outMsg;
      QVariant msgText( " " );
      QSqlQuery query( db );
      int db2Environment = ENV_LUW;

// get the environment
      QgsDb2GeometryColumns gc( db );
      int rc = gc.open( schemaName, tableName );  // returns SQLCODE if failure
      if ( rc == 0 )
      {
        db2Environment = gc.db2Environment();
      }
      if ( ENV_LUW == db2Environment )
      {
        sql = QString( "CALL DB2GSE.ST_Register_Spatial_Column(?, ?, ?, ?, ?, ?, ?)" );
        outCode = 5;
        outMsg = 6;
      }
      else // z/OS doesn't support 'computeExtents' parameter and has different schema
      {
        sql = QString( "CALL SYSPROC.ST_Register_Spatial_Column(?, ?, ?, ?, ?, ?)" );
        outCode = 4;
        outMsg = 5;
      }
      query.prepare( sql );
      query.bindValue( 0, schemaName );
      query.bindValue( 1, tableName );
      query.bindValue( 2, geometryColumn );
      query.bindValue( 3, srsName );
      if ( ENV_LUW == db2Environment )
      {
        query.bindValue( 4, computeExtents );
      }

      query.bindValue( outCode, msgCode, QSql::Out );
      query.bindValue( outMsg, msgText, QSql::Out );

      if ( !query.exec() )
      {
        QgsDebugMsg( QString( "error: %1; sql: %2" ).arg( query.lastError().text(), query.lastQuery() ) );
      }
      else
      {
        msgCode = query.boundValue( outCode ).toInt();
        msgText = query.boundValue( outMsg ).toString();  // never gets a value...
        if ( 0 != msgCode )
        {
          QgsDebugMsg( QString( "Register failed with code: %1; text: '%2'" ).arg( msgCode ).arg( msgText.toString() ) );
        }
        else
        {
          QgsDebugMsg( "Register successful" );
        }
      }

      QList<QVariant> list = query.boundValues().values();
      for ( int i = 0; i < list.size(); ++i )
      {
        QgsDebugMsg( QString( "i: %1; value: %2; type: %3" )
                     .arg( i ).arg( list.at( i ).toString().toAscii().data() ).arg( list.at( i ).typeName() ) );
      }

    }
    // clear any resources hold by the query
    q.clear();
    q.setForwardOnly( true );

  }
  QgsDebugMsg( "successfully created empty layer" );
  return QgsVectorLayerImport::NoError;
}

QString QgsDb2Provider::qgsFieldToDb2Field( QgsField field )
{
  QString result = "";
  switch ( field.type() )
  {
    case QVariant::LongLong:
      result = "BIGINT";
      break;

    case QVariant::DateTime:
      result = "TIMESTAMP";
      break;

    case QVariant::Date:
      result = "DATE";
      break;

    case QVariant::Time:
      result = "TIME";
      break;

    case QVariant::String:
      result = QString( "VARCHAR(%1)" ).arg( field.length() );
      break;

    case QVariant::Int:
      result = "INTEGER";
      break;

    case QVariant::Double:
      if ( field.length() <= 0 || field.precision() <= 0 )
      {
        result = "DOUBLE";
      }
      else
      {
        result = QString( "DECIMAL(%1,%2)" ).arg( field.length(), field.precision() );
      }
      break;

    default:
      break;
  }
  if ( !result.isEmpty() )
  {
    result = field.name() + ' ' + result;
  }
  return result;
}
bool QgsDb2Provider::convertField( QgsField &field )
{
  QString fieldType = "VARCHAR"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "BIGINT";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = "TIMESTAMP";
      fieldPrec = -1;
      break;

    case QVariant::Date:
      fieldType = "DATE";
      fieldPrec = -1;
      break;

    case QVariant::Time:
      fieldType = "TIME";
      fieldPrec = -1;
      break;

    case QVariant::String:
      fieldType = "VARCHAR";
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
        fieldType = "DOUBLE";
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = "DECIMAL";
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


QString QgsDb2Provider::name() const
{
  return PROVIDER_KEY;
}

QString QgsDb2Provider::description() const
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN QgsDb2Provider *classFactory( const QString *uri )
{
  return new QgsDb2Provider( *uri );
}

QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN void *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsDb2SourceSelect( parent, fl );
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  Q_UNUSED( thePath );
  QgsDebugMsg( "DB2: Browser Panel; data item detected." );
  return new QgsDb2RootItem( parentItem, PROVIDER_KEY, "DB2:" );
}


QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsDb2Provider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}
