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
#include <QTextStream>
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

static const QString TEXT_PROVIDER_KEY = "mssql";
static const QString TEXT_PROVIDER_DESCRIPTION = "MSSQL spatial data provider";

QgsMssqlProvider::QgsMssqlProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mFieldCount( 0 )
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

  mUseEstimatedMetadata = anUri.useEstimatedMetadata();

  mDatabase = GetDatabase( anUri.service(), anUri.host(), anUri.database(), anUri.username(), anUri.password() );

  if ( !OpenDatabase( mDatabase ) )
  {
    setLastError( mDatabase.lastError( ).text( ) );
    mValid = false;
    return;
  }

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

    if ( mFidColName.isEmpty() )
      mValid = false;
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

QSqlDatabase QgsMssqlProvider::GetDatabase( QString driver, QString host, QString database, QString username, QString password )
{
  QSqlDatabase db;
  QString connectionName;
  if ( driver.isEmpty() )
  {
    if ( host.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider host name not specified" );
      return db;
    }

    if ( database.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider database name not specified" );
      return db;
    }
    connectionName = host + "." + database;
  }
  else
    connectionName = driver;

  if ( !QSqlDatabase::contains( connectionName ) )
    db = QSqlDatabase::addDatabase( "QODBC", connectionName );
  else
    db = QSqlDatabase::database( connectionName );

  db.setHostName( host );
  QString connectionString = "";
  if ( !driver.isEmpty() )
  {
    // driver was specified explicitly
    connectionString = driver;
  }
  else
  {
#ifdef WIN32
    connectionString = "driver={SQL Server}";
#else
    connectionString = "driver={FreeTDS}";
#endif
    if ( !host.isEmpty() )
      connectionString += ";server=" + host;

    if ( !database.isEmpty() )
      connectionString += ";database=" + database;

    if ( password.isEmpty() )
      connectionString += ";trusted_connection=yes";
    else
      connectionString += ";uid=" + username + ";pwd=" + password;
  }

  if ( !username.isEmpty() )
    db.setUserName( username );

  if ( !password.isEmpty() )
    db.setPassword( password );

  db.setDatabaseName( connectionString );
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
            sqlTypeName.startsWith( "ntext", Qt::CaseInsensitive ) )
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
    type = QVariant::String;
  }
  else if ( sqlTypeName.startsWith( "datetime", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "smalldatetime", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "datetime2", Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else if ( sqlTypeName.startsWith( "time", Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( "timestamp", Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else
  {
    QgsDebugMsg( QString( "Unknown field type: %1" ).arg( sqlTypeName ) );
  }

  return type;
}

void QgsMssqlProvider::loadMetadata()
{
  mSRId = 0;
  mWkbType = QGis::WKBUnknown;

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  if ( !mQuery.exec( QString( "select f_geometry_column, coord_dimension, srid, geometry_type from geometry_columns where f_table_schema = '%1' and f_table_name = '%2'" ).arg( mSchemaName ).arg( mTableName ) ) )
  {
    QString msg = mQuery.lastError().text();
    QgsDebugMsg( msg );
  }
  if ( mQuery.isActive() )
  {
    if ( mQuery.next() )
    {
      mGeometryColName = mQuery.value( 0 ).toString();
      mSRId = mQuery.value( 2 ).toInt();
      mWkbType = getWkbType( mQuery.value( 3 ).toString(), mQuery.value( 1 ).toInt() );
    }
  }
}

void QgsMssqlProvider::loadFields()
{
  mAttributeFields.clear();
  mDefaultValues.clear();
  // get field spec
  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  if ( !mQuery.exec( QString( "exec sp_columns N'%1', NULL, NULL, NULL, NULL" ).arg( mTableName ) ) )
  {
    QString msg = mQuery.lastError().text();
    QgsDebugMsg( msg );
    return;
  }
  if ( mQuery.isActive() )
  {
    int i = 0;
    QStringList pkCandidates;
    while ( mQuery.next() )
    {
      QString sqlTypeName = mQuery.value( 5 ).toString();
      if ( sqlTypeName == "geometry" || sqlTypeName == "geography" )
      {
        mGeometryColName = mQuery.value( 3 ).toString();
        mGeometryColType = sqlTypeName;
        parser.IsGeography = sqlTypeName == "geography";
      }
      else
      {
        QVariant::Type sqlType = DecodeSqlType( sqlTypeName );
        if ( sqlTypeName == "int identity" || sqlTypeName == "bigint identity" )
          mFidColName = mQuery.value( 3 ).toString();
        else if ( sqlTypeName == "int" || sqlTypeName == "bigint" )
        {
          pkCandidates << mQuery.value( 3 ).toString();
        }
        mAttributeFields.insert(
          i, QgsField(
            mQuery.value( 3 ).toString(), sqlType,
            sqlTypeName,
            mQuery.value( 7 ).toInt(),
            mQuery.value( 6 ).toInt() ) );

        if ( !mQuery.value( 12 ).isNull() )
        {
          mDefaultValues.insert( i, mQuery.value( 12 ) );
        }
        ++i;
      }
    }
    // get primary key
    if ( mFidColName.isEmpty() )
    {
      mQuery.clear();
      mQuery.setForwardOnly( true );
      if ( !mQuery.exec( QString( "exec sp_pkeys N'%1', NULL, NULL" ).arg( mTableName ) ) )
      {
        QString msg = mQuery.lastError().text();
        QgsDebugMsg( msg );
      }
      if ( mQuery.isActive() )
      {
        if ( mQuery.next() )
        {
          mFidColName = mQuery.value( 3 ).toString();
          return;
        }
      }
      foreach( QString pk, pkCandidates )
      {
        mQuery.clear();
        mQuery.setForwardOnly( true );
        if ( !mQuery.exec( QString( "select count(distinct [%1]), count([%1]) from [%2].[%3]" )
                           .arg( pk )
                           .arg( mSchemaName )
                           .arg( mTableName ) ) )
        {
          QString msg = mQuery.lastError().text();
          QgsDebugMsg( msg );
        }
        if ( mQuery.isActive() )
        {
          if ( mQuery.next() )
          {
            if ( mQuery.value( 0 ).toInt() == mQuery.value( 1 ).toInt() )
            {
              mFidColName = pk;
              return;
            }
          }
        }
      }
    }
  }
}

QVariant QgsMssqlProvider::defaultValue( int fieldId )
{
  if ( mDefaultValues.contains(fieldId) )
    return mDefaultValues[fieldId];
  else
    return QVariant( QString::null );
}

QString QgsMssqlProvider::storageType() const
{
  return "MSSQL spatial database";
}

bool QgsMssqlProvider::featureAtId( QgsFeatureId featureId,
                                    QgsFeature& feature,
                                    bool fetchGeometry,
                                    QgsAttributeList fetchAttributes )
{
  // build sql statement
  QString query( "select " );
  mFieldCount = 0;
  for ( QgsAttributeList::iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it )
  {
    if ( mFieldCount != 0 )
      query += ",";
    query += "[" + mAttributeFields[*it].name() + "]";
    ++mFieldCount;
  }

  mFidCol = -1;
  if ( !mFidColName.isEmpty() )
  {
    if ( mFieldCount != 0 )
      query += ",";
    query += "[" + mFidColName + "]";
    mFidCol = mFieldCount;
    ++mFieldCount;
  }
  else
    return false;
  mGeometryCol = -1;
  if ( fetchGeometry && !mGeometryColName.isEmpty() )
  {
    if ( mFieldCount != 0 )
      query += ",";
    query += "[" + mGeometryColName + "]";
    mGeometryCol = mFieldCount;
    ++mFieldCount;
  }
  query += " from ";
  if ( !mSchemaName.isEmpty() )
    query += "[" + mSchemaName + "].";

  query += "[" + mTableName + "]";
  // set attribute filter
  query += QString( " where [%1] = %2" ).arg( mFidColName, QString::number( featureId ) );

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  // issue the sql query
  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  if ( !mQuery.exec( query ) )
  {
    QString msg = mQuery.lastError().text();
    QgsDebugMsg( msg );
  }

  return nextFeature( feature );
}


bool QgsMssqlProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );
  if ( !mValid )
  {
    QgsDebugMsg( "Read attempt on an invalid MSSQL data source" );
    return false;
  }

  if ( !mQuery.isActive() )
  {
    QgsDebugMsg( "Read attempt on inactive query" );
    return false;
  }

  feature.clearAttributeMap();

  if ( mQuery.next() )
  {
    int col = 0;
    for ( QgsAttributeList::iterator it = mAttributesToFetch.begin(); it != mAttributesToFetch.end(); ++it )
    {
      feature.addAttribute( *it, mQuery.value( col ) );
      col++;
    }

    if ( mFidCol >= 0 )
    {
      feature.setFeatureId( mQuery.value( col ).toInt() );
      col++;
    }

    if ( mGeometryCol >= 0 )
    {
      QByteArray ar = mQuery.value( col ).toByteArray();
      unsigned char* wkb = parser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
      if ( wkb )
      {
        feature.setGeometryAndOwnership( wkb, parser.GetWkbLen() );
      }
      col++;
    }

    feature.setValid( true );
    return true;
  }
  return false;
} // nextFeature


void QgsMssqlProvider::select( QgsAttributeList fetchAttributes,
                               QgsRectangle rect,
                               bool fetchGeometry,
                               bool useIntersect )
{
  Q_UNUSED( useIntersect );
  // build sql statement
  mStatement = QString( "select " );
  mFieldCount = 0;
  for ( QgsAttributeList::iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it )
  {
    if ( mFieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mAttributeFields[*it].name() + "]";
    ++mFieldCount;
  }

  mFidCol = -1;
  if ( !mFidColName.isEmpty() )
  {
    if ( mFieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mFidColName + "]";
    mFidCol = mFieldCount;
    ++mFieldCount;
  }
  mGeometryCol = -1;
  if ( fetchGeometry && !mGeometryColName.isEmpty() )
  {
    if ( mFieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mGeometryColName + "]";
    mGeometryCol = mFieldCount;
    ++mFieldCount;
  }

  mStatement += " from ";
  if ( !mSchemaName.isEmpty() )
    mStatement += "[" + mSchemaName + "].";

  mStatement += "[" + mTableName + "]";
  // set spatial filter
  if ( !rect.isEmpty() )
  {
    // polygons should be CCW for SqlGeography
    QString r;
    QTextStream foo( &r );

    foo.setRealNumberPrecision( 8 );
    foo.setRealNumberNotation( QTextStream::FixedNotation );
    foo <<  rect.xMinimum() << " " <<  rect.yMinimum() << ", "
    <<  rect.xMaximum() << " " <<  rect.yMinimum() << ", "
    <<  rect.xMaximum() << " " <<  rect.yMaximum() << ", "
    <<  rect.xMinimum() << " " <<  rect.yMaximum() << ", "
    <<  rect.xMinimum() << " " <<  rect.yMinimum();

    mStatement += QString( " where [%1].STIntersects([%2]::STGeomFromText('POLYGON((%3))',%4)) = 1" ).arg(
        mGeometryColName, mGeometryColType, r, QString::number( mSRId ) );
  }
  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  // issue the sql query
  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  if ( mFieldCount > 0 )
  {
    if ( !mQuery.exec( mStatement ) )
    {
      QString msg = mQuery.lastError().text();
      QgsDebugMsg( msg );
    }
  }
  else
  {
    QgsDebugMsg( "QgsMssqlProvider::select no fields have been requested" );
  }
}

// update the extent, feature count, wkb type and srid for this layer
void QgsMssqlProvider::UpdateStatistics( bool estimate )
{
  mNumberFeatures = 0;
  // get features to calculate the statistics
  QString statement;
  bool readAll = false;
  if ( estimate )
  {
    if ( mGeometryColType == "geometry" )
      statement = QString( "select min([%1].STPointN(1).STX), min([%1].STPointN(1).STY), max([%1].STPointN(1).STX), max([%1].STPointN(1).STY), COUNT([%1])" ).arg( mGeometryColName );
    else
      statement = QString( "select min([%1].STPointN(1).Long), min([%1].STPointN(1).Lat), max([%1].STPointN(1).Long), max([%1].STPointN(1).Lat), COUNT([%1])" ).arg( mGeometryColName );
  }
  else
  {
    if ( mGeometryColType == "geometry" )
      statement = QString( "select min([%1].STEnvelope().STPointN(1).STX), min([%1].STEnvelope().STPointN(1).STY), max([%1].STEnvelope().STPointN(3).STX), max([%1].STEnvelope().STPointN(3).STY), count([%1])" ).arg( mGeometryColName );
    else
    {
      statement = QString( "select [%1]" ).arg( mGeometryColName );
      readAll = true;
    }
  }

  if ( mSchemaName.isEmpty() )
    statement += QString( " from [%1]" ).arg( mTableName );
  else
    statement += QString( " from [%1].[%2]" ).arg( mSchemaName, mTableName );

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );

  if ( !mQuery.exec( statement ) )
  {
    QString msg = mQuery.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( mQuery.isActive() )
  {
    QgsGeometry geom;
    if ( !readAll )
    {
      if ( mQuery.next() )
      {
        mExtent.setXMinimum( mQuery.value( 0 ).toDouble() );
        mExtent.setYMinimum( mQuery.value( 1 ).toDouble() );
        mExtent.setXMaximum( mQuery.value( 2 ).toDouble() );
        mExtent.setYMaximum( mQuery.value( 3 ).toDouble() );
        mNumberFeatures = mQuery.value( 4 ).toInt();
      }
    }
    else
    {
      // read all features
      while ( mQuery.next() )
      {
        QByteArray ar = mQuery.value( 0 ).toByteArray();
        unsigned char* wkb = parser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
        if ( wkb )
        {
          geom.fromWkb( wkb, parser.GetWkbLen() );
          QgsRectangle rect = geom.boundingBox();

          if ( mNumberFeatures > 0 )
          {
            if ( rect.xMinimum() < mExtent.xMinimum() )
              mExtent.setXMinimum( rect.xMinimum() );
            if ( rect.yMinimum() < mExtent.yMinimum() )
              mExtent.setYMinimum( rect.yMinimum() );
            if ( rect.xMaximum() > mExtent.xMaximum() )
              mExtent.setXMaximum( rect.xMaximum() );
            if ( rect.yMaximum() > mExtent.yMaximum() )
              mExtent.setYMaximum( rect.yMaximum() );
          }
          else
          {
            mExtent = rect;
            mWkbType = geom.wkbType();
            mSRId = parser.GetSRSId();
          }
          ++mNumberFeatures;
        }
      }
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
  return mNumberFeatures;
}

/**
 * Return the number of fields
 */
uint QgsMssqlProvider::fieldCount() const
{
  return mAttributeFields.size();
}


const QgsFieldMap & QgsMssqlProvider::fields() const
{
  return mAttributeFields;
}

void QgsMssqlProvider::rewind()
{
  if ( !mStatement.isEmpty() )
  {
    // reissue the sql query
    mQuery = QSqlQuery( mDatabase );
    mQuery.setForwardOnly( true );
    mQuery.exec( mStatement );
  }
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
    if ( mSchemaName.isEmpty() )
      statement = QString( "INSERT INTO [%1].[%2] (" ).arg( QString( "dbo" ), mTableName );
    else
      statement = QString( "INSERT INTO [%1].[%2] (" ).arg( mSchemaName, mTableName );

    bool first = true;
    mQuery = QSqlQuery( mDatabase );
    mQuery.setForwardOnly( true );

    const QgsAttributeMap& attrs = it->attributeMap();

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields[it2.key()];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains(it2.key()) && mDefaultValues[it2.key()] == *it2 )
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
                      QString( "?" ) , QString::number( mSRId ) );
        else
          values += QString( "geometry::STGeomFromText(%1,%2).MakeValid()" ).arg(
                      QString( "?" ) , QString::number( mSRId ) );
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
    if ( !mQuery.prepare( statement ) )
    {
      QString msg = mQuery.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        QString msg = mQuery.lastError().text();
        QgsDebugMsg( msg );
        pushError( msg );
        return false;
      }
      else
        continue;
    }

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields[it2.key()];

      if ( fld.typeName().endsWith( " identity", Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains(it2.key()) && mDefaultValues[it2.key()] == *it2 )
        continue; // skip fields having default values

      QVariant::Type type = fld.type();
      if ( it2->isNull() || !it2->isValid() )
      {
        // binding null values
        if ( type == QVariant::Date || type == QVariant::DateTime )
          mQuery.addBindValue( QVariant( QVariant::String ) );
        else
          mQuery.addBindValue( QVariant( type ) );
      }
      else if ( type == QVariant::Int )
      {
        // binding an INTEGER value
        mQuery.addBindValue( it2->toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        mQuery.addBindValue( it2->toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        mQuery.addBindValue( it2->toString() );
      }
      else
      {
        mQuery.addBindValue( *it2 );
      }
    }

    if ( !mGeometryColName.isEmpty() )
    {
      QgsGeometry *geom = it->geometry();
      if ( mUseWkb )
      {
        QByteArray bytea = QByteArray(( char* )geom->asWkb(), geom->wkbSize() );
        mQuery.addBindValue( bytea,  QSql::In | QSql::Binary );
      }
      else
      {
        QString wkt = geom->exportToWkt();
        mQuery.addBindValue( wkt );
      }
    }

    if ( !mQuery.exec() )
    {
      QString msg = mQuery.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }
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
      if ( mSchemaName.isEmpty() )
        statement = QString( "ALTER TABLE [%1].[%2] ADD " ).arg(
                      QString( "dbo" ), mTableName );
      else
        statement = QString( "ALTER TABLE [%1].[%2] ADD " ).arg(
                      mSchemaName, mTableName );
    }
    else
      statement += ",";

    statement += QString( "[%1] %2" ).arg( it->name(), type );
  }

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  if ( !mQuery.exec( statement ) )
  {
    QString msg = mQuery.lastError().text();
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
    QgsFieldMap::const_iterator field_it = mAttributeFields.find( *it );
    if ( field_it == mAttributeFields.constEnd() )
      continue;

    if ( statement.isEmpty() )
    {
      if ( mSchemaName.isEmpty() )
        statement = QString( "ALTER TABLE [%1].[%2] DROP COLUMN " ).arg( QString( "dbo" ), mTableName );
      else
        statement = QString( "ALTER TABLE [%1].[%2] DROP COLUMN " ).arg( mSchemaName, mTableName );
    }
    else
      statement += ",";

    statement += QString( "[%1]" ).arg( field_it->name() );

    //delete the attribute from attributeFields
    mAttributeFields.remove( *it );
  }

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  return mQuery.exec( statement );
}


bool QgsMssqlProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  if ( attr_map.isEmpty() )
    return true;

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    QString statement;
    if ( mSchemaName.isEmpty() )
      statement = QString( "UPDATE [%1].[%2] SET " ).arg( QString( "dbo" ), mTableName );
    else
      statement = QString( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    bool first = true;
    mQuery = QSqlQuery( mDatabase );
    mQuery.setForwardOnly( true );

    const QgsAttributeMap& attrs = it.value();

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
    if ( !mQuery.prepare( statement ) )
    {
      QString msg = mQuery.lastError().text();
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
          mQuery.addBindValue( QVariant( QVariant::String ) );
        else
          mQuery.addBindValue( QVariant( type ) );
      }
      else if ( type == QVariant::Int )
      {
        // binding an INTEGER value
        mQuery.addBindValue( it2->toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        mQuery.addBindValue( it2->toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        mQuery.addBindValue( it2->toString() );
      }
      else
      {
        mQuery.addBindValue( *it2 );
      }
    }

    if ( !mQuery.exec() )
    {
      QString msg = mQuery.lastError().text();
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

  for ( QgsGeometryMap::iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();
    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    QString statement;
    if ( mSchemaName.isEmpty() )
      statement = QString( "UPDATE [%1].[%2] SET " ).arg( QString( "dbo" ), mTableName );
    else
      statement = QString( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    mQuery = QSqlQuery( mDatabase );
    mQuery.setForwardOnly( true );

    if ( mGeometryColType == "geometry" )
    {
      if ( mUseWkb )
        statement += QString( "[%1]=geometry::STGeomFromWKB(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QString( "?" ) , QString::number( mSRId ) );
      else
        statement += QString( "[%1]=geometry::STGeomFromText(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QString( "?" ) , QString::number( mSRId ) );
    }
    else
    {
      if ( mUseWkb )
        statement += QString( "[%1]=geography::STGeomFromWKB(%2,%3)" ).arg(
                       mGeometryColName, QString( "?" ) , QString::number( mSRId ) );
      else
        statement += QString( "[%1]=geography::STGeomFromText(%2,%3)" ).arg(
                       mGeometryColName, QString( "?" ) , QString::number( mSRId ) );
    }

    // set attribute filter
    statement += QString( " WHERE [%1]=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    if ( !mQuery.prepare( statement ) )
    {
      QString msg = mQuery.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }

    // add geometry param
    if ( mUseWkb )
    {
      QByteArray bytea = QByteArray(( char* )it->asWkb(), it->wkbSize() );
      mQuery.addBindValue( bytea,  QSql::In | QSql::Binary );
    }
    else
    {
      QString wkt = it->exportToWkt();
      mQuery.addBindValue( wkt );
    }

    if ( !mQuery.exec() )
    {
      QString msg = mQuery.lastError().text();
      QgsDebugMsg( msg );
      return false;
    }
  }

  return true;
}

bool QgsMssqlProvider::deleteFeatures( const QgsFeatureIds & id )
{
  QString featureIds;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( featureIds.isEmpty() )
      featureIds = FID_TO_STRING( *it );
    else
      featureIds += "," + FID_TO_STRING( *it );
  }

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  QString statement;
  if ( mSchemaName.isEmpty() )
    statement = QString( "DELETE FROM [%1].[%2] WHERE [%3] IN (%4)" ).arg( QString( "dbo" ),
                mTableName, mFidColName, featureIds );
  else
    statement = QString( "DELETE FROM [%1].[%2] WHERE [%3] IN (%4)" ).arg( mSchemaName,
                mTableName, mFidColName, featureIds );

  return mQuery.exec( statement );
}

int QgsMssqlProvider::capabilities() const
{
  return CreateSpatialIndex | CreateAttributeIndex | AddFeatures | DeleteFeatures |
         ChangeAttributeValues | ChangeGeometries | AddAttributes | DeleteAttributes |
         QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
}

bool QgsMssqlProvider::createSpatialIndex()
{
  if ( mUseEstimatedMetadata )
    UpdateStatistics( false );

  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  QString statement;
  if ( mSchemaName.isEmpty() )
    statement = QString( "CREATE SPATIAL INDEX [qgs_%1_sidx] ON [%2].[%3] ( [%4] )" ).arg(
                  mGeometryColName, QString( "dbo" ), mTableName, mGeometryColName );
  else
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
  return mQuery.exec( statement );
}

bool QgsMssqlProvider::createAttributeIndex( int field )
{
  mQuery = QSqlQuery( mDatabase );
  mQuery.setForwardOnly( true );
  QString statement;

  if ( field < 0 || field >= mAttributeFields.size() )
  {
    QgsDebugMsg( "createAttributeIndex invalid index" );
    return false;
  }

  if ( mSchemaName.isEmpty() )
    statement = QString( "CREATE NONCLUSTERED INDEX [qgs_%1_idx] ON [%2].[%3] ( [%4] )" ).arg(
                  mGeometryColName, QString( "dbo" ), mTableName, mAttributeFields[field].name() );
  else
    statement = QString( "CREATE NONCLUSTERED INDEX [qgs_%1_idx] ON [%2].[%3] ( [%4] )" ).arg(
                  mGeometryColName, mSchemaName, mTableName, mAttributeFields[field].name() );

  return mQuery.exec( statement );
}

QgsCoordinateReferenceSystem QgsMssqlProvider::crs()
{
  if ( !mCrs.isValid() && mSRId > 0 )
  {
    // try to load crs
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );
    query.exec( QString( "select srtext from spatial_ref_sys where srid = %1" ).arg( QString::number( mSRId ) ) );
    if ( query.isActive() )
    {
      if ( query.next() )
      {
        if ( mCrs.createFromWkt( query.value( 0 ).toString() ) )
          return mCrs;
      }
    }
    query.exec( QString( "select well_known_text from sys.spatial_reference_systems where spatial_reference_id = %1" ).arg( QString::number( mSRId ) ) );
    if ( query.isActive() )
    {
      if ( query.next() )
      {
        if ( mCrs.createFromWkt( query.value( 0 ).toString() ) )
          return mCrs;
      }
    }
  }
  return mCrs;
}


QString  QgsMssqlProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // ::name()



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

  // connect to database
  QSqlDatabase db = QgsMssqlProvider::GetDatabase( dsUri.service(),
                    dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError( ).text( );
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
    geometryColumn = "qgs_geometry";

  if ( primaryKey.isEmpty() )
    primaryKey = "qgs_fid";

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "qgs_fid";
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
      *errorMessage = q.lastError( ).text( );
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
        *errorMessage = q.lastError( ).text( );
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
        *errorMessage = q.lastError( ).text( );
      return QgsVectorLayerImport::ErrCreateLayer;
    }
  }

  sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                 "CREATE TABLE [%1].[%2]([%3] [int] IDENTITY(1,1) NOT NULL, [%4] [geometry] NULL CONSTRAINT [PK_%2] PRIMARY KEY CLUSTERED ( [%3] ASC ))\n"
                 "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'\n"
                 "INSERT INTO [geometry_columns] ([f_table_catalog], [f_table_schema] ,[f_table_name], "
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
      *errorMessage = q.lastError( ).text( );
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
    int offset = geometryColumn.isEmpty() ? 0 : 1;

    // get the list of fields
    QList<QgsField> flist;
    for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      QgsField fld = fldIt.value();
      if ( fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( fldIt.key(), 0 );
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
        oldToNewAttrIdxMap->insert( fldIt.key(), offset++ );
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

QGISEXTERN void *selectWidget( QWidget *parent, Qt::WFlags fl )
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
  const QgsFieldMap &fields,
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
