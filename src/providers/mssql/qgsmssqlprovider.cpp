/***************************************************************************
      qgsmssqlprovider.cpp  -  Data provider for mssql server
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlprovider.h"

#include <QtGlobal>
#include <QFileInfo>
#include <QDataStream>
#include <QStringList>
#include <QMessageBox>
#include <QSettings>
#include <QRegExp>
#include <QUrl>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QStringBuilder>
#include <QWaitCondition>


#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgis.h"

#include "qgsmssqlsourceselect.h"
#include "qgsmssqldataitems.h"

#include "qgsmssqlfeatureiterator.h"

static const QString TEXT_PROVIDER_KEY = "mssql";
static const QString TEXT_PROVIDER_DESCRIPTION = "MSSQL spatial data provider";
int QgsMssqlProvider::sConnectionId = 0;

QgsMssqlProvider::QgsMssqlProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mNumberFeatures( 0 )
    , mCrs()
    , mWkbType( QGis::WKBUnknown )
{
  QgsDataSourceURI anUri = QgsDataSourceURI( uri );

  if ( !anUri.srid().isEmpty() )
    mSRId = anUri.srid().toInt();
  else
    mSRId = -1;

  mWkbType = anUri.wkbType();

  mValid = true;

  mUseWkb = false;
  mSkipFailures = false;

  mUserName = anUri.username();
  mPassword = anUri.password();
  mService = anUri.service();
  mDatabaseName = anUri.database();
  mHost = anUri.host();

  mUseEstimatedMetadata = anUri.useEstimatedMetadata();

  mSqlWhereClause = anUri.sql();

  mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );

  if ( !OpenDatabase( mDatabase ) )
  {
    setLastError( mDatabase.lastError().text() );
    QgsDebugMsg( mLastError );
    mValid = false;
    return;
  }

  // Create a query for default connection
  mQuery = QSqlQuery( mDatabase );

  // Database successfully opened; we can now issue SQL commands.
  if ( !anUri.schema().isEmpty() )
    mSchemaName = anUri.schema();
  else
    mSchemaName = "dbo";

  if ( !anUri.table().isEmpty() )
  {
    // the layer name has been specified
    mTableName = anUri.table();
    QStringList sl = mTableName.split( '.' );
    if ( sl.length() == 2 )
    {
      mSchemaName = sl[0];
      mTableName = sl[1];
    }
    mTables = QStringList( mTableName );
  }
  else
  {
    // Get a list of table
    mTables = mDatabase.tables( QSql::Tables );
    if ( mTables.count() > 0 )
      mTableName = mTables[0];
    else
      mValid = false;
  }
  if ( mValid )
  {
    if ( !anUri.keyColumn().isEmpty() )
      mFidColName = anUri.keyColumn();

    if ( !anUri.geometryColumn().isEmpty() )
      mGeometryColName = anUri.geometryColumn();

    if ( mSRId < 0 || mWkbType == QGis::WKBUnknown || mGeometryColName.isEmpty() )
      loadMetadata();
    loadFields();
    UpdateStatistics( mUseEstimatedMetadata );

    if ( mGeometryColName.isEmpty() )
    {
      // table contains no geometries
      mWkbType = QGis::WKBNoGeometry;
      mSRId = 0;
    }
  }

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "8 Bytes integer" ), "bigint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "4 Bytes integer" ), "int", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "2 Bytes integer" ), "smallint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "1 Bytes integer" ), "tinyint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "numeric", QVariant::Double, 1, 20, 0, 20 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "decimal", QVariant::Double, 1, 20, 0, 20 )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "real", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "float", QVariant::Double )

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "char", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), "varchar", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length unicode (nchar)" ), "nchar", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length unicode (nvarchar)" ), "nvarchar", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), "text", QVariant::String )
  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length unicode (ntext)" ), "text", QVariant::String )
  ;
}

QgsMssqlProvider::~QgsMssqlProvider()
{
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

QgsAbstractFeatureSource* QgsMssqlProvider::featureSource() const
{
  return new QgsMssqlFeatureSource( this );
}

QgsFeatureIterator QgsMssqlProvider::getFeatures( const QgsFeatureRequest& request )
{
  if ( !mValid )
  {
    QgsDebugMsg( "Read attempt on an invalid mssql data source" );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsMssqlFeatureIterator( new QgsMssqlFeatureSource( this ), true, request ) );
}

bool QgsMssqlProvider::OpenDatabase( QSqlDatabase db )
{
  if ( !db.isOpen() )
  {
    if ( !db.open() )
    {
      return false;
    }
  }
  return true;
}

QSqlDatabase QgsMssqlProvider::GetDatabase( QString service, QString host, QString database, QString username, QString password )
{
  QSqlDatabase db;
  QString connectionName;

  // create a separate database connection for each feature source
  QgsDebugMsg( "Creating a separate database connection" );

  if ( service.isEmpty() )
  {
    if ( !host.isEmpty() )
      connectionName = host + ".";

    if ( database.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider database name not specified" );
      return db;
    }

    connectionName += QString( "%1.%2" ).arg( database ).arg( sConnectionId++ );
  }
  else
    connectionName = service;

  if ( !QSqlDatabase::contains( connectionName ) )
  {
    db = QSqlDatabase::addDatabase( "QODBC", connectionName );
    db.setConnectOptions( "SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV" );
  }
  else
    db = QSqlDatabase::database( connectionName );

  db.setHostName( host );
  QString connectionString = "";
  if ( !service.isEmpty() )
  {
    // driver was specified explicitly
    connectionString = service;
  }
  else
  {
#ifdef WIN32
    connectionString = "driver={SQL Server}";
#else
    connectionString = "driver={FreeTDS};port=1433";
#endif
  }

  if ( !host.isEmpty() )
    connectionString += ";server=" + host;

  if ( !database.isEmpty() )
    connectionString += ";database=" + database;

  if ( password.isEmpty() )
    connectionString += ";trusted_connection=yes";
  else
    connectionString += ";uid=" + username + ";pwd=" + password;

  if ( !username.isEmpty() )
    db.setUserName( username );

  if ( !password.isEmpty() )
    db.setPassword( password );

  db.setDatabaseName( connectionString );
  QgsDebugMsg( connectionString );
  return db;
}

QVariant::Type QgsMssqlProvider::DecodeSqlType( QString sqlTypeName )
{
  QVariant::Type type = QVariant::Invalid;
  if ( sqlTypeName.startsWith( "decimal", Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( "numeric", Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( "real", Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( "float", Qt::CaseInsensitive ) )
  {
    type = QVariant::Double;
  }
  else if ( sqlTypeName.startsWith( "char", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "nchar", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "varchar", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "nvarchar", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "text", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "ntext", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "uniqueidentifier", Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else if ( sqlTypeName.startsWith( "smallint", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "int", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "bit", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "tinyint", Qt::CaseInsensitive ) )
  {
    type = QVariant::Int;
  }
  else if ( sqlTypeName.startsWith( "bigint", Qt::CaseInsensitive ) )
  {
    type = QVariant::LongLong;
  }
  else if ( sqlTypeName.startsWith( "binary", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "varbinary", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "image", Qt::CaseInsensitive ) )
  {
    type = QVariant::ByteArray;
  }
  else if ( sqlTypeName.startsWith( "date", Qt::CaseInsensitive ) )
  {
    type = QVariant::Date;
  }
  else if ( sqlTypeName.startsWith( "datetime", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "smalldatetime", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "datetime2", Qt::CaseInsensitive ) )
  {
    type = QVariant::DateTime;
  }
  else if ( sqlTypeName.startsWith( "time", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "timestamp", Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else
  {
    QgsDebugMsg( QString( "Unknown field type: %1" ).arg( sqlTypeName ) );
    // Everything else just dumped as a string.
    type = QVariant::String;
  }

  return type;
}

void QgsMssqlProvider::loadMetadata()
{
  mSRId = 0;
  mWkbType = QGis::WKBUnknown;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( QString( "select f_geometry_column, coord_dimension, srid, geometry_type from geometry_columns where f_table_schema = '%1' and f_table_name = '%2'" ).arg( mSchemaName ).arg( mTableName ) ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }
  if ( query.isActive() && query.next() )
  {
    mGeometryColName = query.value( 0 ).toString();
    mSRId = query.value( 2 ).toInt();
    mWkbType = getWkbType( query.value( 3 ).toString(), query.value( 1 ).toInt() );
  }
}

void QgsMssqlProvider::loadFields()
{
  mAttributeFields.clear();
  mDefaultValues.clear();
  // get field spec
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( QString( "exec sp_columns @table_name = N'%1', @table_owner = '%2'" ).arg( mTableName, mSchemaName ) ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return;
  }
  if ( query.isActive() )
  {
    int i = 0;
    QStringList pkCandidates;
    while ( query.next() )
    {
      QString sqlTypeName = query.value( 5 ).toString();
      if ( sqlTypeName == "geometry" || sqlTypeName == "geography" )
      {
        mGeometryColName = query.value( 3 ).toString();
        mGeometryColType = sqlTypeName;
        mParser.IsGeography = sqlTypeName == "geography";
      }
      else
      {
        QVariant::Type sqlType = DecodeSqlType( sqlTypeName );
        if ( sqlTypeName == "int identity" || sqlTypeName == "bigint identity" )
          mFidColName = query.value( 3 ).toString();
        else if ( sqlTypeName == "int" || sqlTypeName == "bigint" )
        {
          pkCandidates << query.value( 3 ).toString();
        }
        if ( sqlType == QVariant::String )
        {
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName,
              query.value( 7 ).toInt() ) );
        }
        else if ( sqlType == QVariant::Double )
        {
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName,
              query.value( 7 ).toInt(),
              query.value( 8 ).toInt() ) );
        }
        else
        {
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName ) );
        }

        if ( !query.value( 12 ).isNull() )
        {
          mDefaultValues.insert( i, query.value( 12 ) );
        }
        ++i;
      }
    }
    // get primary key
    if ( mFidColName.isEmpty() )
    {
      query.clear();
      query.setForwardOnly( true );
      if ( !query.exec( QString( "exec sp_pkeys @table_name = N'%1', @table_owner = '%2' " ).arg( mTableName, mSchemaName ) ) )
      {
        QString msg = query.lastError().text();
        QgsDebugMsg( msg );
      }
      if ( query.isActive() && query.next() )
      {
        mFidColName = query.value( 3 ).toString();
        return;
      }
      foreach ( QString pk, pkCandidates )
      {
        query.clear();
        query.setForwardOnly( true );
        if ( !query.exec( QString( "select count(distinct [%1]), count([%1]) from [%2].[%3]" )
                          .arg( pk )
                          .arg( mSchemaName )
                          .arg( mTableName ) ) )
        {
          QString msg = query.lastError().text();
          QgsDebugMsg( msg );
        }
        if ( query.isActive() && query.next() && query.value( 0 ).toInt() == query.value( 1 ).toInt() )
        {
          mFidColName = pk;
          return;
        }
      }
      QString error = QString( "No primary key could be found on table %1" ).arg( mTableName );
      QgsDebugMsg( error );
      mValid = false;
      setLastError( error );
    }
  }
}

QVariant QgsMssqlProvider::defaultValue( int fieldId )
{
  if ( mDefaultValues.contains( fieldId ) )
    return mDefaultValues[fieldId];
  else
    return QVariant( QString::null );
}

QString QgsMssqlProvider::storageType() const
{
  return "MSSQL spatial database";
}

// Returns the minimum value of an attribute
QVariant QgsMssqlProvider::minimumValue( int index )
{
  // get the field name
  QgsField fld = mAttributeFields[ index ];
  QString sql = QString( "select min([%1]) from " )
                .arg( fld.name() );

  sql += QString( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( query.isActive() && query.next() )
  {
    return query.value( 0 );
  }

  return QVariant( QString::null );
}

// Returns the maximum value of an attribute
QVariant QgsMssqlProvider::maximumValue( int index )
{
  // get the field name
  QgsField fld = mAttributeFields[ index ];
  QString sql = QString( "select max([%1]) from " )
                .arg( fld.name() );

  sql += QString( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( query.isActive() && query.next() )
  {
    return query.value( 0 );
  }

  return QVariant( QString::null );
}

// Returns the list of unique values of an attribute
void QgsMssqlProvider::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  uniqueValues.clear();

  // get the field name
  QgsField fld = mAttributeFields[ index ];
  QString sql = QString( "select distinct " );

  if ( limit > 0 )
  {
    sql += QString( " top %1 " ).arg( limit );
  }

  sql += QString( "[%1] from " )
         .arg( fld.name() );

  sql += QString( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( query.isActive() )
  {
    // read all features
    while ( query.next() )
    {
      uniqueValues.append( query.value( 0 ) );
    }
  }
}


// update the extent, wkb type and srid for this layer
void QgsMssqlProvider::UpdateStatistics( bool estimate )
{
  // get features to calculate the statistics
  QString statement;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  // Get the extents from the spatial index table to speed up load times.
  // We have to use max() and min() because you can have more then one index but the biggest area is what we want to use.
  QString sql = "SELECT min(bounding_box_xmin), min(bounding_box_ymin), max(bounding_box_xmax), max(bounding_box_ymax)"
                " FROM sys.spatial_index_tessellations WHERE object_id =  OBJECT_ID('[%1].[%2]')";

  statement = QString( sql ).arg( mSchemaName ).arg( mTableName );

  if ( query.exec( statement ) )
  {
    if ( query.next() && ( !query.value( 0 ).isNull() ||
                           !query.value( 1 ).isNull() ||
                           !query.value( 2 ).isNull() ||
                           !query.value( 3 ).isNull() ) )
    {
      QgsDebugMsg( "Found extents in spatial index" );
      mExtent.setXMinimum( query.value( 0 ).toDouble() );
      mExtent.setYMinimum( query.value( 1 ).toDouble() );
      mExtent.setXMaximum( query.value( 2 ).toDouble() );
      mExtent.setYMaximum( query.value( 3 ).toDouble() );
      return;
    }
  }

  QgsDebugMsg( query.lastError().text() );

  // If we can't find the extents in the spatial index table just do what we normally do.
  bool readAllGeography = false;
  if ( estimate )
  {
    if ( mGeometryColType == "geometry" )
      statement = QString( "select min([%1].STPointN(1).STX), min([%1].STPointN(1).STY), max([%1].STPointN(1).STX), max([%1].STPointN(1).STY)" ).arg( mGeometryColName );
    else
      statement = QString( "select min([%1].STPointN(1).Long), min([%1].STPointN(1).Lat), max([%1].STPointN(1).Long), max([%1].STPointN(1).Lat)" ).arg( mGeometryColName );
  }
  else
  {
    if ( mGeometryColType == "geometry" )
      statement = QString( "select min([%1].STEnvelope().STPointN(1).STX), min([%1].STEnvelope().STPointN(1).STY), max([%1].STEnvelope().STPointN(3).STX), max([%1].STEnvelope().STPointN(3).STY)" ).arg( mGeometryColName );
    else
    {
      statement = QString( "select [%1]" ).arg( mGeometryColName );
      readAllGeography = true;
    }
  }

  statement += QString( " from [%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    statement += " where (" + mSqlWhereClause + ")";
  }

  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( !query.isActive() )
  {
    return;
  }

  QgsGeometry geom;
  if ( !readAllGeography && query.next() )
  {
    mExtent.setXMinimum( query.value( 0 ).toDouble() );
    mExtent.setYMinimum( query.value( 1 ).toDouble() );
    mExtent.setXMaximum( query.value( 2 ).toDouble() );
    mExtent.setYMaximum( query.value( 3 ).toDouble() );
    return;
  }

  // We have to read all the geometry if readAllGeography is true.
  while ( query.next() )
  {
    QByteArray ar = query.value( 0 ).toByteArray();
    unsigned char* wkb = mParser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
    if ( wkb )
    {
      geom.fromWkb( wkb, mParser.GetWkbLen() );
      QgsRectangle rect = geom.boundingBox();

      if ( rect.xMinimum() < mExtent.xMinimum() )
        mExtent.setXMinimum( rect.xMinimum() );
      if ( rect.yMinimum() < mExtent.yMinimum() )
        mExtent.setYMinimum( rect.yMinimum() );
      if ( rect.xMaximum() > mExtent.xMaximum() )
        mExtent.setXMaximum( rect.xMaximum() );
      if ( rect.yMaximum() > mExtent.yMaximum() )
        mExtent.setYMaximum( rect.yMaximum() );

      mWkbType = geom.wkbType();
      mSRId = mParser.GetSRSId();
    }
  }
}

// Return the extent of the layer
QgsRectangle QgsMssqlProvider::extent()
{
  if ( mExtent.isEmpty() )
    UpdateStatistics( mUseEstimatedMetadata );
  return mExtent;
}

/**
 * Return the feature type
 */
QGis::WkbType QgsMssqlProvider::geometryType() const
{
  return mWkbType;
}

/**
 * Return the feature type
 */
long QgsMssqlProvider::featureCount() const
{
  // Return the count that we get from the subset.
  if ( !mSqlWhereClause.isEmpty() )
    return mNumberFeatures;

  // If there is no subset set we can get the count from the system tables.
  // Which is faster then doing select count(*)
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  QString sql = "SELECT rows"
                " FROM sys.tables t"
                " JOIN sys.partitions p ON t.object_id = p.object_id AND p.index_id IN (0,1)"
                " WHERE SCHEMA_NAME(t.schema_id) = '%1' AND OBJECT_NAME(t.OBJECT_ID) = '%2'";

  QString statement = QString( sql ).arg( mSchemaName ).arg( mTableName );

  if ( query.exec( statement ) && query.next() )
  {
    return query.value( 0 ).toInt();
  }
  else
  {
    // We couldn't get the rows from the sys tables. Can that ever happen?
    // Should just do a select count(*) here.
    return -1;
  }
}

const QgsFields & QgsMssqlProvider::fields() const
{
  return mAttributeFields;
}

bool QgsMssqlProvider::isValid()
{
  return mValid;
}

bool QgsMssqlProvider::addFeatures( QgsFeatureList & flist )
{
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    QString statement;
    QString values;
    statement = QString( "INSERT INTO [%1].[%2] (" ).arg( mSchemaName, mTableName );

    bool first = true;
    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    const QgsAttributes& attrs = it->attributes();

    for ( int i = 0; i < attrs.count(); ++i )
    {
      QgsField fld = mAttributeFields[i];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains( i ) && mDefaultValues[i] == attrs[i] )
        continue; // skip fields having default values

      if ( !first )
      {
        statement += ",";
        values += ",";
      }
      else
        first = false;

      statement += QString( "[%1]" ).arg( fld.name() );
      values += QString( "?" );
    }

    // append geometry column name
    if ( !mGeometryColName.isEmpty() )
    {
      if ( !first )
      {
        statement += ",";
        values += ",";
      }

      statement += QString( "[%1]" ).arg( mGeometryColName );
      if ( mGeometryColType == "geometry" )
      {
        if ( mUseWkb )
          values += QString( "geometry::STGeomFromWKB(%1,%2).MakeValid()" ).arg(
                      QString( "?" ), QString::number( mSRId ) );
        else
          values += QString( "geometry::STGeomFromText(%1,%2).MakeValid()" ).arg(
                      QString( "?" ), QString::number( mSRId ) );
      }
      else
      {
        if ( mUseWkb )
          values += QString( "geography::STGeomFromWKB(%1,%2)" ).arg(
                      QString( "?" ), QString::number( mSRId ) );
        else
          values += QString( "geography::STGeomFromText(%1,%2)" ).arg(
                      QString( "?" ), QString::number( mSRId ) );
      }
    }

    statement += ") VALUES (" + values + ")";

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        QString msg = query.lastError().text();
        QgsDebugMsg( msg );
        pushError( msg );
        return false;
      }
      else
        continue;
    }

    for ( int i = 0; i < attrs.count(); ++i )
    {
      QgsField fld = mAttributeFields[i];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains( i ) && mDefaultValues[i] == attrs[i] )
        continue; // skip fields having default values

      QVariant::Type type = fld.type();
      if ( attrs[i].isNull() || !attrs[i].isValid() )
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
        query.addBindValue( attrs[i].toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        query.addBindValue( attrs[i].toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        query.addBindValue( attrs[i].toString() );
      }
      else
      {
        query.addBindValue( attrs[i] );
      }
    }

    if ( !mGeometryColName.isEmpty() )
    {
      QgsGeometry *geom = it->geometry();
      if ( mUseWkb )
      {
        QByteArray bytea = QByteArray(( char* )geom->asWkb(), geom->wkbSize() );
        query.addBindValue( bytea, QSql::In | QSql::Binary );
      }
      else
      {
        QString wkt = geom->exportToWkt();
        query.addBindValue( wkt );
      }
    }

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


    statement = QString( "SELECT IDENT_CURRENT('%1.%2')" ).arg( mSchemaName, mTableName );

    if ( !query.exec( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }

    if ( !query.next() )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }
    it->setFeatureId( query.value( 0 ).toLongLong() );
  }

  return true;
}

bool QgsMssqlProvider::addAttributes( const QList<QgsField> &attributes )
{
  QString statement;

  for ( QList<QgsField>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QString type = it->typeName();
    if ( type == "char" || type == "varchar" )
    {
      if ( it->length() > 0 )
        type = QString( "%1(%2)" ).arg( type ).arg( it->length() );
    }
    else if ( type == "numeric" || type == "decimal" )
    {
      if ( it->length() > 0 && it->precision() > 0 )
        type = QString( "%1(%2,%3)" ).arg( type ).arg( it->length() ).arg( it->precision() );
    }

    if ( statement.isEmpty() )
    {
      statement = QString( "ALTER TABLE [%1].[%2] ADD " ).arg(
                    mSchemaName, mTableName );
    }
    else
      statement += ",";

    statement += QString( "[%1] %2" ).arg( it->name(), type );
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return false;
  }

  return true;
}

bool QgsMssqlProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QString statement;

  for ( QgsAttributeIds::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    if ( statement.isEmpty() )
    {
      statement = QString( "ALTER TABLE [%1].[%2] DROP COLUMN " ).arg( mSchemaName, mTableName );
    }
    else
      statement += ",";

    statement += QString( "[%1]" ).arg( mAttributeFields[*it].name() );
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return false;
  }

  query.finish();
  loadFields();
  return true;
}


bool QgsMssqlProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
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

    QString statement = QString( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    bool first = true;
    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields[it2.key()];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( !first )
        statement += ",";
      else
        first = false;

      statement += QString( "[%1]=?" ).arg( fld.name() );
    }

    if ( first )
      return true; // no fields have been changed

    // set attribute filter
    statement += QString( " WHERE [%1]=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields[it2.key()];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

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
      else
      {
        query.addBindValue( *it2 );
      }
    }

    if ( !query.exec() )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }
  }

  return true;
}

bool QgsMssqlProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  if ( geometry_map.isEmpty() )
    return true;

  if ( mFidColName.isEmpty() )
    return false;

  for ( QgsGeometryMap::iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();
    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    QString statement;
    statement = QString( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    if ( mGeometryColType == "geometry" )
    {
      if ( mUseWkb )
        statement += QString( "[%1]=geometry::STGeomFromWKB(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QString( "?" ), QString::number( mSRId ) );
      else
        statement += QString( "[%1]=geometry::STGeomFromText(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QString( "?" ), QString::number( mSRId ) );
    }
    else
    {
      if ( mUseWkb )
        statement += QString( "[%1]=geography::STGeomFromWKB(%2,%3)" ).arg(
                       mGeometryColName, QString( "?" ), QString::number( mSRId ) );
      else
        statement += QString( "[%1]=geography::STGeomFromText(%2,%3)" ).arg(
                       mGeometryColName, QString( "?" ), QString::number( mSRId ) );
    }

    // set attribute filter
    statement += QString( " WHERE [%1]=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    if ( !query.prepare( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }

    // add geometry param
    if ( mUseWkb )
    {
      QByteArray bytea = QByteArray(( char* )it->asWkb(), it->wkbSize() );
      query.addBindValue( bytea, QSql::In | QSql::Binary );
    }
    else
    {
      QString wkt = it->exportToWkt();
      query.addBindValue( wkt );
    }

    if ( !query.exec() )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }
  }

  return true;
}

bool QgsMssqlProvider::deleteFeatures( const QgsFeatureIds & id )
{
  if ( mFidColName.isEmpty() )
    return false;

  QString featureIds;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( featureIds.isEmpty() )
      featureIds = FID_TO_STRING( *it );
    else
      featureIds += "," + FID_TO_STRING( *it );
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QString statement;
  statement = QString( "DELETE FROM [%1].[%2] WHERE [%3] IN (%4)" ).arg( mSchemaName,
              mTableName, mFidColName, featureIds );

  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return false;
  }

  return true;
}

int QgsMssqlProvider::capabilities() const
{
  if ( mFidColName.isEmpty() )
    return CreateSpatialIndex | CreateAttributeIndex | AddFeatures | AddAttributes;
  else
    return CreateSpatialIndex | CreateAttributeIndex | AddFeatures | DeleteFeatures |
           ChangeAttributeValues | ChangeGeometries | AddAttributes | DeleteAttributes |
           QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
}

bool QgsMssqlProvider::createSpatialIndex()
{
  if ( mUseEstimatedMetadata )
    UpdateStatistics( false );

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QString statement;
  statement = QString( "CREATE SPATIAL INDEX [qgs_%1_sidx] ON [%2].[%3] ( [%4] )" ).arg(
                mGeometryColName, mSchemaName, mTableName, mGeometryColName );

  if ( mGeometryColType == "geometry" )
  {
    statement += QString( " USING GEOMETRY_GRID WITH (BOUNDING_BOX =(%1, %2, %3, %4))" ).arg(
                   QString::number( mExtent.xMinimum() ), QString::number( mExtent.yMinimum() ),
                   QString::number( mExtent.xMaximum() ), QString::number( mExtent.yMaximum() ) );
  }
  else
  {
    statement += " USING GEOGRAPHY_GRID";
  }

  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return false;
  }

  return true;
}

bool QgsMssqlProvider::createAttributeIndex( int field )
{
  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QString statement;

  if ( field < 0 || field >= mAttributeFields.size() )
  {
    QgsDebugMsg( "createAttributeIndex invalid index" );
    return false;
  }

  statement = QString( "CREATE NONCLUSTERED INDEX [qgs_%1_idx] ON [%2].[%3] ( [%4] )" ).arg(
                mGeometryColName, mSchemaName, mTableName, mAttributeFields[field].name() );

  if ( !query.exec( statement ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
    return false;
  }

  return true;
}

QgsCoordinateReferenceSystem QgsMssqlProvider::crs()
{
  if ( !mCrs.isValid() && mSRId > 0 )
  {
    // try to load crs
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );
    bool execOk = query.exec( QString( "select srtext from spatial_ref_sys where srid = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() )
    {
      if ( query.next() && mCrs.createFromWkt( query.value( 0 ).toString() ) )
        return mCrs;

      query.finish();
    }
    query.clear();
    execOk = query.exec( QString( "select well_known_text from sys.spatial_reference_systems where spatial_reference_id = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() && query.next() && mCrs.createFromWkt( query.value( 0 ).toString() ) )
      return mCrs;
  }
  return mCrs;
}

QString QgsMssqlProvider::subsetString()
{
  return mSqlWhereClause;
}

QString  QgsMssqlProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // ::name()

bool QgsMssqlProvider::setSubsetString( QString theSQL, bool )
{
  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "select count(*) from " );

  sql += QString( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " where %1" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( sql ) )
  {
    pushError( query.lastError().text() );
    mSqlWhereClause = prevWhere;
    return false;
  }

  if ( query.isActive() && query.next() )
    mNumberFeatures = query.value( 0 ).toInt();

  QgsDataSourceURI anUri = QgsDataSourceURI( dataSourceUri() );
  anUri.setSql( mSqlWhereClause );

  setDataSourceUri( anUri.uri() );

  mExtent.setMinimal();

  return true;
}

QString  QgsMssqlProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
} //  QgsMssqlProvider::name()

QStringList QgsMssqlProvider::subLayers() const
{
  return mTables;
}

bool QgsMssqlProvider::convertField( QgsField &field )
{
  QString fieldType = "nvarchar(max)"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "bigint";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::String:
      fieldType = "nvarchar(max)";
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = "int";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = "float";
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = "decimal";
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

void QgsMssqlProvider::mssqlWkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim )
{
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
}

QGis::WkbType QgsMssqlProvider::getWkbType( QString geometryType, int dim )
{
  if ( dim == 3 )
  {
    if ( geometryType == "POINT" )
      return QGis::WKBPoint25D;
    if ( geometryType == "LINESTRING" )
      return QGis::WKBLineString25D;
    if ( geometryType == "POLYGON" )
      return QGis::WKBPolygon25D;
    if ( geometryType == "MULTIPOINT" )
      return QGis::WKBMultiPoint25D;
    if ( geometryType == "MULTILINESTRING" )
      return QGis::WKBMultiLineString25D;
    if ( geometryType == "MULTIPOLYGON" )
      return QGis::WKBMultiPolygon25D;
    else
      return QGis::WKBUnknown;
  }
  else
  {
    if ( geometryType == "POINT" )
      return QGis::WKBPoint;
    if ( geometryType == "LINESTRING" )
      return QGis::WKBLineString;
    if ( geometryType == "POLYGON" )
      return QGis::WKBPolygon;
    if ( geometryType == "MULTIPOINT" )
      return QGis::WKBMultiPoint;
    if ( geometryType == "MULTILINESTRING" )
      return QGis::WKBMultiLineString;
    if ( geometryType == "MULTIPOLYGON" )
      return QGis::WKBMultiPolygon;
    else
      return QGis::WKBUnknown;
  }
}


QgsVectorLayerImport::ImportError QgsMssqlProvider::createEmptyLayer(
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

  // connect to database
  QSqlDatabase db = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return QgsVectorLayerImport::ErrConnectionFailed;
  }

  QString dbName = dsUri.database();

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  if ( schemaName.isEmpty() )
    schemaName = "dbo";

  if ( geometryColumn.isEmpty() )
    geometryColumn = "geom";

  if ( primaryKey.isEmpty() )
    primaryKey = "qgs_fid";

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "qgs_fid";
    for ( int i = 0, n = fields.size(); i < n; ++i )
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
    for ( int i = 0, n = fields.size(); i < n; ++i )
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

  // if the field doesn't not exist yet, create it as a serial field
  if ( primaryKeyType.isEmpty() )
    primaryKeyType = "serial";

  QString sql;
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );

  // initialize metadata tables (same as OGR SQL)
  sql = QString( "IF NOT EXISTS (SELECT * FROM sys.objects WHERE "
                 "object_id = OBJECT_ID(N'[dbo].[geometry_columns]') AND type in (N'U')) "
                 "CREATE TABLE geometry_columns (f_table_catalog varchar(128) not null, "
                 "f_table_schema varchar(128) not null, f_table_name varchar(256) not null, "
                 "f_geometry_column varchar(256) not null, coord_dimension integer not null, "
                 "srid integer not null, geometry_type varchar(30) not null, "
                 "CONSTRAINT geometry_columns_pk PRIMARY KEY (f_table_catalog, "
                 "f_table_schema, f_table_name, f_geometry_column));\n"
                 "IF NOT EXISTS (SELECT * FROM sys.objects "
                 "WHERE object_id = OBJECT_ID(N'[dbo].[spatial_ref_sys]') AND type in (N'U')) "
                 "CREATE TABLE spatial_ref_sys (srid integer not null "
                 "PRIMARY KEY, auth_name varchar(256), auth_srid integer, srtext varchar(2048), proj4text varchar(2048))" );
  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return QgsVectorLayerImport::ErrCreateLayer;
  }

  // set up spatial reference id
  int srid = 0;
  if ( srs->isValid() )
  {
    srid = srs->srsid();
    QString auth_srid = "null";
    QString auth_name = "null";
    QStringList sl = srs->authid().split( ':' );
    if ( sl.length() == 2 )
    {
      auth_name = "'" + sl[0] + "'";
      auth_srid = sl[1];
    }
    sql = QString( "IF NOT EXISTS (SELECT * FROM spatial_ref_sys WHERE srid=%1) INSERT INTO spatial_ref_sys (srid, auth_name, auth_srid, srtext, proj4text) VALUES (%1, %2, %3, '%4', '%5')" )
          .arg( srs->srsid() )
          .arg( auth_name )
          .arg( auth_srid )
          .arg( srs->toWkt() )
          .arg( srs->toProj4() );
    if ( !q.exec( sql ) )
    {
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return QgsVectorLayerImport::ErrCreateLayer;
    }
  }

  // get wkb type and dimension
  QString geometryType;
  int dim = 2;
  mssqlWkbTypeAndDimension( wkbType, geometryType, dim );

  if ( overwrite )
  {
    // remove the old table with the same name
    sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) BEGIN DROP TABLE [%1].[%2] DELETE FROM geometry_columns where f_table_schema = '%1' and f_table_name = '%2' END;" )
          .arg( schemaName ).arg( tableName );
    if ( !q.exec( sql ) )
    {
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return QgsVectorLayerImport::ErrCreateLayer;
    }
  }

  sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                 "CREATE TABLE [%1].[%2]([%3] [int] IDENTITY(1,1) NOT NULL, [%4] [geometry] NULL CONSTRAINT [PK_%2] PRIMARY KEY CLUSTERED ( [%3] ASC ))\n"
                 "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'\n"
                 "INSERT INTO [geometry_columns] ([f_table_catalog], [f_table_schema],[f_table_name], "
                 "[f_geometry_column],[coord_dimension],[srid],[geometry_type]) VALUES ('%5', '%1', '%2', '%4', %6, %7, '%8')" )
        .arg( schemaName )
        .arg( tableName )
        .arg( primaryKey )
        .arg( geometryColumn )
        .arg( dbName )
        .arg( QString::number( dim ) )
        .arg( QString::number( srid ) )
        .arg( geometryType );

  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return QgsVectorLayerImport::ErrCreateLayer;
  }

  // clear any resources hold by the query
  q.clear();
  q.setForwardOnly( true );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );
  QgsMssqlProvider *provider = new QgsMssqlProvider( dsUri.uri() );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the MSSQL provider failed" );

    delete provider;
    return QgsVectorLayerImport::ErrInvalidLayer;
  }

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 0;

    // get the list of fields
    QList<QgsField> flist;
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      QgsField fld = fields[i];
      if ( oldToNewAttrIdxMap && fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( fields.indexFromName( fld.name() ), 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
      {
        // Found a field with the same name of the geometry column. Skip it!
        continue;
      }

      if ( !convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        delete provider;
        return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
      }

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fields.indexFromName( fld.name() ), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      delete provider;
      return QgsVectorLayerImport::ErrAttributeCreationFailed;
    }
  }
  return QgsVectorLayerImport::NoError;
}



/**
 * Class factory to return a pointer to a newly created
 * QgsMssqlProvider object
 */
QGISEXTERN QgsMssqlProvider *classFactory( const QString *uri )
{
  return new QgsMssqlProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN void *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsMssqlSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  Q_UNUSED( thePath );
  return new QgsMssqlRootItem( parentItem, "MSSQL", "mssql:" );
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
  return QgsMssqlProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}
