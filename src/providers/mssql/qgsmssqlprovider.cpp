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
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgis.h"

#include "qgsmssqldataitems.h"
#include "qgsmssqlfeatureiterator.h"

#ifdef HAVE_GUI
#include "qgsmssqlsourceselect.h"
#include "qgssourceselectprovider.h"
#endif

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mssql" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "MSSQL spatial data provider" );
int QgsMssqlProvider::sConnectionId = 0;

QgsMssqlProvider::QgsMssqlProvider( const QString &uri )
  : QgsVectorDataProvider( uri )
  , mNumberFeatures( 0 )
  , mFidColIdx( -1 )
  , mCrs()
  , mWkbType( QgsWkbTypes::Unknown )
{
  QgsDataSourceUri anUri = QgsDataSourceUri( uri );

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
    mSchemaName = QStringLiteral( "dbo" );

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
    if ( !mTables.isEmpty() )
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

    if ( mSRId < 0 || mWkbType == QgsWkbTypes::Unknown || mGeometryColName.isEmpty() )
    {
      loadMetadata();
    }
    loadFields();
    UpdateStatistics( mUseEstimatedMetadata );

    if ( mGeometryColName.isEmpty() )
    {
      // table contains no geometries
      mWkbType = QgsWkbTypes::NoGeometry;
      mSRId = 0;
    }
  }

  //fill type names into sets
  setNativeTypes( QList<NativeType>()
                  // integer types
                  << QgsVectorDataProvider::NativeType( tr( "8 Bytes integer" ), QStringLiteral( "bigint" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "4 Bytes integer" ), QStringLiteral( "int" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "2 Bytes integer" ), QStringLiteral( "smallint" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "1 Bytes integer" ), QStringLiteral( "tinyint" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

                  // floating point
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "real" ), QVariant::Double )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "float" ), QVariant::Double )

                  // date/time types
                  << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "datetime" ), QVariant::DateTime, -1, -1, -1, -1 )

                  // string types
                  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QVariant::String, 1, 255 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ), QVariant::String, 1, 255 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length unicode (nchar)" ), QStringLiteral( "nchar" ), QVariant::String, 1, 255 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length unicode (nvarchar)" ), QStringLiteral( "nvarchar" ), QVariant::String, 1, 255 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String )
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length unicode (ntext)" ), QStringLiteral( "text" ), QVariant::String )
                );
}

QgsMssqlProvider::~QgsMssqlProvider()
{
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

QgsAbstractFeatureSource *QgsMssqlProvider::featureSource() const
{
  return new QgsMssqlFeatureSource( this );
}

QgsFeatureIterator QgsMssqlProvider::getFeatures( const QgsFeatureRequest &request ) const
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

QSqlDatabase QgsMssqlProvider::GetDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password )
{
  QSqlDatabase db;
  QString connectionName;

  // create a separate database connection for each feature source
  QgsDebugMsg( "Creating a separate database connection" );

  if ( service.isEmpty() )
  {
    if ( !host.isEmpty() )
      connectionName = host + '.';

    if ( database.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider database name not specified" );
      return db;
    }

    connectionName += QStringLiteral( "%1.%2" ).arg( database ).arg( sConnectionId++ );
  }
  else
    connectionName = service;

  if ( !QSqlDatabase::contains( connectionName ) )
  {
    db = QSqlDatabase::addDatabase( QStringLiteral( "QODBC" ), connectionName );
    db.setConnectOptions( QStringLiteral( "SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV" ) );
  }
  else
    db = QSqlDatabase::database( connectionName );

  db.setHostName( host );
  QString connectionString = QLatin1String( "" );
  if ( !service.isEmpty() )
  {
    // driver was specified explicitly
    connectionString = service;
  }
  else
  {
#ifdef Q_OS_WIN
    connectionString = "driver={SQL Server}";
#else
    connectionString = QStringLiteral( "driver={FreeTDS};port=1433" );
#endif
  }

  if ( !host.isEmpty() )
    connectionString += ";server=" + host;

  if ( !database.isEmpty() )
    connectionString += ";database=" + database;

  if ( password.isEmpty() )
    connectionString += QLatin1String( ";trusted_connection=yes" );
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

QVariant::Type QgsMssqlProvider::DecodeSqlType( const QString &sqlTypeName )
{
  QVariant::Type type = QVariant::Invalid;
  if ( sqlTypeName.startsWith( QLatin1String( "decimal" ), Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( QLatin1String( "numeric" ), Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( QLatin1String( "real" ), Qt::CaseInsensitive ) ||
       sqlTypeName.startsWith( QLatin1String( "float" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::Double;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "char" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "nchar" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "varchar" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "nvarchar" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "text" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "ntext" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "uniqueidentifier" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "smallint" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "int" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "bit" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "tinyint" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::Int;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "bigint" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::LongLong;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "binary" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "varbinary" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "image" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::ByteArray;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "datetime" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "smalldatetime" ), Qt::CaseInsensitive ) ||
            sqlTypeName.startsWith( QLatin1String( "datetime2" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::DateTime;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "date" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::Date;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::String;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "time" ), Qt::CaseInsensitive ) )
  {
    type = QVariant::Time;
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
  mWkbType = QgsWkbTypes::Unknown;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( QStringLiteral( "select f_geometry_column, srid, geometry_type from geometry_columns where f_table_schema = '%1' and f_table_name = '%2'" ).arg( mSchemaName, mTableName ) ) )
  {
    QgsDebugMsg( query.lastError().text() );
  }
  if ( query.isActive() && query.next() )
  {
    mGeometryColName = query.value( 0 ).toString();
    mSRId = query.value( 1 ).toInt();
    mWkbType = getWkbType( query.value( 2 ).toString() );
  }
}

void QgsMssqlProvider::loadFields()
{
  mAttributeFields.clear();
  mDefaultValues.clear();
  mComputedColumns.clear();

  // get field spec
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  // Get computed columns which need to be ignored on insert or update.
  if ( !query.exec( QStringLiteral( "SELECT name FROM sys.columns WHERE is_computed = 1 AND object_id = OBJECT_ID('[%1].[%2]')" ).arg( mSchemaName, mTableName ) ) )
  {
    pushError( query.lastError().text() );
    return;
  }

  if ( query.isActive() )
  {
    while ( query.next() )
    {
      mComputedColumns.append( query.value( 0 ).toString() );
    }
  }

  if ( !query.exec( QStringLiteral( "exec sp_columns @table_name = N'%1', @table_owner = '%2'" ).arg( mTableName, mSchemaName ) ) )
  {
    pushError( query.lastError().text() );
    return;
  }
  if ( query.isActive() )
  {
    int i = 0;
    QStringList pkCandidates;
    while ( query.next() )
    {
      QString sqlTypeName = query.value( 5 ).toString();
      if ( sqlTypeName == QLatin1String( "geometry" ) || sqlTypeName == QLatin1String( "geography" ) )
      {
        mGeometryColName = query.value( 3 ).toString();
        mGeometryColType = sqlTypeName;
        mParser.IsGeography = sqlTypeName == QLatin1String( "geography" );
      }
      else
      {
        QVariant::Type sqlType = DecodeSqlType( sqlTypeName );
        if ( sqlTypeName == QLatin1String( "int identity" ) || sqlTypeName == QLatin1String( "bigint identity" ) )
          mFidColName = query.value( 3 ).toString();
        else if ( sqlTypeName == QLatin1String( "int" ) || sqlTypeName == QLatin1String( "bigint" ) )
        {
          pkCandidates << query.value( 3 ).toString();
        }
        if ( sqlType == QVariant::String )
        {
          int length = query.value( 7 ).toInt();
          if ( sqlTypeName.startsWith( "n" ) )
          {
            length = length / 2;
          }
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName,
              length ) );
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
        else if ( sqlType == QVariant::Date || sqlType == QVariant::DateTime || sqlType == QVariant::Time )
        {
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName,
              -1,
              -1 ) );
        }
        else
        {
          mAttributeFields.append(
            QgsField(
              query.value( 3 ).toString(), sqlType,
              sqlTypeName ) );
        }

        //COLUMN_DEF
        if ( !query.value( 12 ).isNull() )
        {
          mDefaultValues.insert( i, query.value( 12 ).toString() );
        }
        ++i;
      }
    }
    // get primary key
    if ( mFidColName.isEmpty() )
    {
      query.clear();
      query.setForwardOnly( true );
      if ( !query.exec( QStringLiteral( "exec sp_pkeys @table_name = N'%1', @table_owner = '%2' " ).arg( mTableName, mSchemaName ) ) )
      {
        QgsDebugMsg( query.lastError().text() );
      }
      if ( query.isActive() && query.next() )
      {
        mFidColName = query.value( 3 ).toString();
        return;
      }
      Q_FOREACH ( const QString &pk, pkCandidates )
      {
        query.clear();
        query.setForwardOnly( true );
        if ( !query.exec( QStringLiteral( "select count(distinct [%1]), count([%1]) from [%2].[%3]" )
                          .arg( pk, mSchemaName, mTableName ) ) )
        {
          QgsDebugMsg( query.lastError().text() );
        }
        if ( query.isActive() && query.next() && query.value( 0 ).toInt() == query.value( 1 ).toInt() )
        {
          mFidColName = pk;
          return;
        }
      }
      QString error = QStringLiteral( "No primary key could be found on table %1" ).arg( mTableName );
      QgsDebugMsg( error );
      mValid = false;
      setLastError( error );
    }

    if ( !mFidColName.isEmpty() )
    {
      mFidColIdx = mAttributeFields.indexFromName( mFidColName );
      if ( mFidColIdx >= 0 )
      {
        // primary key has not null, unique constraints
        QgsFieldConstraints constraints = mAttributeFields.at( mFidColIdx ).constraints();
        constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
        constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
        mAttributeFields[ mFidColIdx ].setConstraints( constraints );
      }
    }
  }
}

QString QgsMssqlProvider::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? "1" : "0";

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( '\'', QLatin1String( "''" ) );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "N'" ).append( '\'' );
      else
        return v.prepend( '\'' ).append( '\'' );
  }
}

QString QgsMssqlProvider::defaultValueClause( int fieldId ) const
{
  return mDefaultValues.value( fieldId, QString() );
}

QString QgsMssqlProvider::storageType() const
{
  return QStringLiteral( "MSSQL spatial database" );
}

// Returns the minimum value of an attribute
QVariant QgsMssqlProvider::minimumValue( int index ) const
{
  // get the field name
  QgsField fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select min([%1]) from " )
                .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QgsDebugMsg( query.lastError().text() );
  }

  if ( query.isActive() && query.next() )
  {
    return query.value( 0 );
  }

  return QVariant( QString() );
}

// Returns the maximum value of an attribute
QVariant QgsMssqlProvider::maximumValue( int index ) const
{
  // get the field name
  QgsField fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select max([%1]) from " )
                .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QgsDebugMsg( query.lastError().text() );
  }

  if ( query.isActive() && query.next() )
  {
    return query.value( 0 );
  }

  return QVariant( QString() );
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsMssqlProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;

  // get the field name
  QgsField fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select distinct " );

  if ( limit > 0 )
  {
    sql += QStringLiteral( " top %1 " ).arg( limit );
  }

  sql += QStringLiteral( "[%1] from " )
         .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( sql ) )
  {
    QgsDebugMsg( query.lastError().text() );
  }

  if ( query.isActive() )
  {
    // read all features
    while ( query.next() )
    {
      uniqueValues.insert( query.value( 0 ) );
    }
  }
  return uniqueValues;
}


// update the extent, wkb type and srid for this layer
void QgsMssqlProvider::UpdateStatistics( bool estimate ) const
{
  if ( mGeometryColName.isEmpty() )
    return;

  // get features to calculate the statistics
  QString statement;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  // Get the extents from the spatial index table to speed up load times.
  // We have to use max() and min() because you can have more then one index but the biggest area is what we want to use.
  QString sql = "SELECT min(bounding_box_xmin), min(bounding_box_ymin), max(bounding_box_xmax), max(bounding_box_ymax)"
                " FROM sys.spatial_index_tessellations WHERE object_id =  OBJECT_ID('[%1].[%2]')";

  statement = QString( sql ).arg( mSchemaName, mTableName );

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
    if ( mGeometryColType == QLatin1String( "geometry" ) )
      statement = QStringLiteral( "select min([%1].MakeValid().STPointN(1).STX), min([%1].MakeValid().STPointN(1).STY), max([%1].MakeValid().STPointN(1).STX), max([%1].MakeValid().STPointN(1).STY)" ).arg( mGeometryColName );
    else
      statement = QStringLiteral( "select min([%1].MakeValid().STPointN(1).Long), min([%1].MakeValid().STPointN(1).Lat), max([%1].MakeValid().STPointN(1).Long), max([%1].MakeValid().STPointN(1).Lat)" ).arg( mGeometryColName );
  }
  else
  {
    if ( mGeometryColType == QLatin1String( "geometry" ) )
      statement = QStringLiteral( "select min([%1].MakeValid().STEnvelope().STPointN(1).STX), min([%1].MakeValid().STEnvelope().STPointN(1).STY), max([%1].MakeValid().STEnvelope().STPointN(3).STX), max([%1].MakeValid().STEnvelope().STPointN(3).STY)" ).arg( mGeometryColName );
    else
    {
      statement = QStringLiteral( "select [%1]" ).arg( mGeometryColName );
      readAllGeography = true;
    }
  }

  statement += QStringLiteral( " from [%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    statement += " where (" + mSqlWhereClause + ')';
  }

  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( query.lastError().text() );
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
    unsigned char *wkb = mParser.ParseSqlGeometry( ( unsigned char * )ar.data(), ar.size() );
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
QgsRectangle QgsMssqlProvider::extent() const
{
  if ( mExtent.isEmpty() )
    UpdateStatistics( mUseEstimatedMetadata );
  return mExtent;
}

/**
 * Return the feature type
 */
QgsWkbTypes::Type QgsMssqlProvider::wkbType() const
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

  QString statement = QString( sql ).arg( mSchemaName, mTableName );

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

QgsFields QgsMssqlProvider::fields() const
{
  return mAttributeFields;
}

bool QgsMssqlProvider::isValid() const
{
  return mValid;
}

bool QgsMssqlProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    QString statement;
    QString values;
    statement = QStringLiteral( "INSERT INTO [%1].[%2] (" ).arg( mSchemaName, mTableName );

    bool first = true;
    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    QgsAttributes attrs = it->attributes();

    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i >= mAttributeFields.count() )
        break;

      QgsField fld = mAttributeFields.at( i );

      if ( fld.typeName().toLower() == QLatin1String( "timestamp" ) )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains( i ) && mDefaultValues.value( i ) == attrs.at( i ).toString() )
        continue; // skip fields having default values

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

      if ( !first )
      {
        statement += ',';
        values += ',';
      }
      else
        first = false;

      statement += QStringLiteral( "[%1]" ).arg( fld.name() );
      values += QStringLiteral( "?" );
    }

    // append geometry column name
    if ( !mGeometryColName.isEmpty() )
    {
      if ( !first )
      {
        statement += ',';
        values += ',';
      }

      statement += QStringLiteral( "[%1]" ).arg( mGeometryColName );
      if ( mGeometryColType == QLatin1String( "geometry" ) )
      {
        if ( mUseWkb )
          values += QStringLiteral( "geometry::STGeomFromWKB(%1,%2)" ).arg(
                      QStringLiteral( "?" ), QString::number( mSRId ) );
        else
          values += QStringLiteral( "geometry::STGeomFromText(%1,%2)" ).arg(
                      QStringLiteral( "?" ), QString::number( mSRId ) );
      }
      else
      {
        if ( mUseWkb )
          values += QStringLiteral( "geography::STGeomFromWKB(%1,%2)" ).arg(
                      QStringLiteral( "?" ), QString::number( mSRId ) );
        else
          values += QStringLiteral( "geography::STGeomFromText(%1,%2)" ).arg(
                      QStringLiteral( "?" ), QString::number( mSRId ) );
      }
    }

    statement += ") VALUES (" + values + ')';

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QString msg = query.lastError().text();
      QgsDebugMsg( msg );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
      else
        continue;
    }

    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i >= mAttributeFields.count() )
        break;

      QgsField fld = mAttributeFields.at( i );

      if ( fld.typeName().toLower() == QLatin1String( "timestamp" ) )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains( i ) && mDefaultValues.value( i ) == attrs.at( i ).toString() )
        continue; // skip fields having default values

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

      QVariant::Type type = fld.type();
      if ( attrs.at( i ).isNull() || !attrs.at( i ).isValid() )
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
        query.addBindValue( attrs.at( i ).toInt() );
      }
      else if ( type == QVariant::Double )
      {
        // binding a DOUBLE value
        query.addBindValue( attrs.at( i ).toDouble() );
      }
      else if ( type == QVariant::String )
      {
        // binding a TEXT value
        query.addBindValue( attrs.at( i ).toString() );
      }
      else if ( type == QVariant::Time )
      {
        // binding a TIME value
        query.addBindValue( attrs.at( i ).toTime().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::Date )
      {
        // binding a DATE value
        query.addBindValue( attrs.at( i ).toDate().toString( Qt::ISODate ) );
      }
      else if ( type == QVariant::DateTime )
      {
        // binding a DATETIME value
        query.addBindValue( attrs.at( i ).toDateTime().toString( Qt::ISODate ) );
      }
      else
      {
        query.addBindValue( attrs.at( i ) );
      }
    }

    if ( !mGeometryColName.isEmpty() )
    {
      QgsGeometry geom = it->geometry();
      if ( mUseWkb )
      {
        QByteArray bytea = geom.exportToWkb();
        query.addBindValue( bytea, QSql::In | QSql::Binary );
      }
      else
      {
        QString wkt;
        if ( !geom.isNull() )
        {
          // Z and M on the end of a WKT string isn't valid for
          // SQL Server so we have to remove it first.
          wkt = geom.exportToWkt();
          wkt.replace( QRegExp( "[mzMZ]+\\s*\\(" ), "(" );
        }
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


    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      statement = QStringLiteral( "SELECT IDENT_CURRENT('%1.%2')" ).arg( mSchemaName, mTableName );

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
      it->setId( query.value( 0 ).toLongLong() );
    }
  }

  return true;
}

bool QgsMssqlProvider::addAttributes( const QList<QgsField> &attributes )
{
  QString statement;

  if ( attributes.isEmpty() )
    return true;

  for ( QList<QgsField>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QString type = it->typeName();
    if ( type == QLatin1String( "char" ) || type == QLatin1String( "varchar" ) )
    {
      if ( it->length() > 0 )
        type = QStringLiteral( "%1(%2)" ).arg( type ).arg( it->length() );
    }
    else if ( type == QLatin1String( "numeric" ) || type == QLatin1String( "decimal" ) )
    {
      if ( it->length() > 0 && it->precision() > 0 )
        type = QStringLiteral( "%1(%2,%3)" ).arg( type ).arg( it->length() ).arg( it->precision() );
    }

    if ( statement.isEmpty() )
    {
      statement = QStringLiteral( "ALTER TABLE [%1].[%2] ADD " ).arg(
                    mSchemaName, mTableName );
    }
    else
      statement += ',';

    statement += QStringLiteral( "[%1] %2" ).arg( it->name(), type );
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( query.lastError().text() );
    return false;
  }

  loadFields();
  return true;
}

bool QgsMssqlProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QString statement;

  for ( QgsAttributeIds::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    if ( statement.isEmpty() )
    {
      statement = QStringLiteral( "ALTER TABLE [%1].[%2] DROP COLUMN " ).arg( mSchemaName, mTableName );
    }
    else
      statement += ',';

    statement += QStringLiteral( "[%1]" ).arg( mAttributeFields.at( *it ).name() );
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( query.lastError().text() );
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

    const QgsAttributeMap &attrs = it.value();
    if ( attrs.isEmpty() )
      continue;

    QString statement = QStringLiteral( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    bool first = true;
    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.typeName().toLower() == QLatin1String( "timestamp" ) )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

      if ( !first )
        statement += ',';
      else
        first = false;

      statement += QStringLiteral( "[%1]=?" ).arg( fld.name() );
    }

    if ( first )
      return true; // no fields have been changed

    // set attribute filter
    statement += QStringLiteral( " WHERE [%1]=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QgsDebugMsg( query.lastError().text() );
      return false;
    }

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.typeName().toLower() == QLatin1String( "timestamp" ) )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

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

bool QgsMssqlProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
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
      continue;

    QString statement;
    statement = QStringLiteral( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    if ( !mDatabase.isOpen() )
    {
      mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
    }
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );

    if ( mGeometryColType == QLatin1String( "geometry" ) )
    {
      if ( mUseWkb )
        statement += QStringLiteral( "[%1]=geometry::STGeomFromWKB(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QStringLiteral( "?" ), QString::number( mSRId ) );
      else
        statement += QStringLiteral( "[%1]=geometry::STGeomFromText(%2,%3).MakeValid()" ).arg(
                       mGeometryColName, QStringLiteral( "?" ), QString::number( mSRId ) );
    }
    else
    {
      if ( mUseWkb )
        statement += QStringLiteral( "[%1]=geography::STGeomFromWKB(%2,%3)" ).arg(
                       mGeometryColName, QStringLiteral( "?" ), QString::number( mSRId ) );
      else
        statement += QStringLiteral( "[%1]=geography::STGeomFromText(%2,%3)" ).arg(
                       mGeometryColName, QStringLiteral( "?" ), QString::number( mSRId ) );
    }

    // set attribute filter
    statement += QStringLiteral( " WHERE [%1]=%2" ).arg( mFidColName, FID_TO_STRING( fid ) );

    if ( !query.prepare( statement ) )
    {
      pushError( query.lastError().text() );
      return false;
    }

    // add geometry param
    if ( mUseWkb )
    {
      QByteArray bytea = it->exportToWkb();
      query.addBindValue( bytea, QSql::In | QSql::Binary );
    }
    else
    {
      QString wkt = it->exportToWkt();
      // Z and M on the end of a WKT string isn't valid for
      // SQL Server so we have to remove it first.
      wkt.replace( QRegExp( "[mzMZ]+\\s*\\(" ), "(" );
      query.addBindValue( wkt );
    }

    if ( !query.exec() )
    {
      pushError( query.lastError().text() );
      return false;
    }
  }

  return true;
}

bool QgsMssqlProvider::deleteFeatures( const QgsFeatureIds &id )
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
    mDatabase = GetDatabase( mService, mHost, mDatabaseName, mUserName, mPassword );
  }
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  QString statement;
  statement = QStringLiteral( "DELETE FROM [%1].[%2] WHERE [%3] IN (%4)" ).arg( mSchemaName,
              mTableName, mFidColName, featureIds );

  if ( !query.exec( statement ) )
  {
    pushError( query.lastError().text() );
    return false;
  }

  return true;
}

QgsVectorDataProvider::Capabilities QgsMssqlProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities cap = CreateAttributeIndex | AddFeatures | AddAttributes;
  bool hasGeom = false;
  if ( !mGeometryColName.isEmpty() )
  {
    hasGeom = true;
    cap |= CreateSpatialIndex;
  }

  if ( mFidColName.isEmpty() )
    return cap;
  else
  {
    if ( hasGeom )
      cap |= ChangeGeometries;

    return cap | DeleteFeatures | ChangeAttributeValues | DeleteAttributes |
           QgsVectorDataProvider::SelectAtId;
  }
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
  statement = QStringLiteral( "CREATE SPATIAL INDEX [qgs_%1_sidx] ON [%2].[%3] ( [%4] )" ).arg(
                mGeometryColName, mSchemaName, mTableName, mGeometryColName );

  if ( mGeometryColType == QLatin1String( "geometry" ) )
  {
    statement += QStringLiteral( " USING GEOMETRY_GRID WITH (BOUNDING_BOX =(%1, %2, %3, %4))" ).arg(
                   QString::number( mExtent.xMinimum() ), QString::number( mExtent.yMinimum() ),
                   QString::number( mExtent.xMaximum() ), QString::number( mExtent.yMaximum() ) );
  }
  else
  {
    statement += QLatin1String( " USING GEOGRAPHY_GRID" );
  }

  if ( !query.exec( statement ) )
  {
    pushError( query.lastError().text() );
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
    pushError( "createAttributeIndex invalid index" );
    return false;
  }

  statement = QStringLiteral( "CREATE NONCLUSTERED INDEX [qgs_%1_idx] ON [%2].[%3] ( [%4] )" ).arg(
                mGeometryColName, mSchemaName, mTableName, mAttributeFields.at( field ).name() );

  if ( !query.exec( statement ) )
  {
    pushError( query.lastError().text() );
    return false;
  }

  return true;
}

QgsCoordinateReferenceSystem QgsMssqlProvider::crs() const
{
  if ( !mCrs.isValid() && mSRId > 0 )
  {
    mCrs.createFromSrid( mSRId );
    if ( mCrs.isValid() )
      return mCrs;

    // try to load crs from the database tables as a fallback
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );
    bool execOk = query.exec( QStringLiteral( "select srtext from spatial_ref_sys where srid = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() )
    {
      if ( query.next() )
      {
        mCrs = QgsCoordinateReferenceSystem::fromWkt( query.value( 0 ).toString() );
        if ( mCrs.isValid() )
          return mCrs;
      }

      query.finish();
    }
    query.clear();

    // Look in the system reference table for the data if we can't find it yet
    execOk = query.exec( QStringLiteral( "select well_known_text from sys.spatial_reference_systems where spatial_reference_id = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() && query.next() )
    {
      mCrs = QgsCoordinateReferenceSystem::fromWkt( query.value( 0 ).toString() );
      if ( mCrs.isValid() )
        return mCrs;
    }
  }
  return mCrs;
}

QString QgsMssqlProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString  QgsMssqlProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // ::name()

bool QgsMssqlProvider::setSubsetString( const QString &theSQL, bool )
{
  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QStringLiteral( "select count(*) from " );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where %1" ).arg( mSqlWhereClause );
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

  QgsDataSourceUri anUri = QgsDataSourceUri( dataSourceUri() );
  anUri.setSql( mSqlWhereClause );

  setDataSourceUri( anUri.uri() );

  mExtent.setMinimal();

  emit dataChanged();

  return true;
}

QString  QgsMssqlProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsAttributeList QgsMssqlProvider::pkAttributeIndexes() const
{
  QgsAttributeList list;
  if ( mFidColIdx >= 0 )
    list << mFidColIdx;
  return list;
}

QStringList QgsMssqlProvider::subLayers() const
{
  return mTables;
}

bool QgsMssqlProvider::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "nvarchar(max)" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "bigint" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = QStringLiteral( "datetime" );
      fieldPrec = -1;
      break;

    case QVariant::Date:
      fieldType = QStringLiteral( "date" );
      fieldPrec = -1;
      break;

    case QVariant::Time:
      fieldType = QStringLiteral( "time" );
      fieldPrec = -1;
      break;

    case QVariant::String:
      fieldType = QStringLiteral( "nvarchar(max)" );
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "int" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "float" );
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = QStringLiteral( "decimal" );
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

void QgsMssqlProvider::mssqlWkbTypeAndDimension( QgsWkbTypes::Type wkbType, QString &geometryType, int &dim )
{
  if ( QgsWkbTypes::hasZ( wkbType ) )
    dim = 3;

  QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( wkbType );

  if ( flatType == QgsWkbTypes::Point )
    geometryType = QStringLiteral( "POINT" );
  else if ( flatType == QgsWkbTypes::LineString )
    geometryType = QStringLiteral( "LINESTRING" );
  else if ( flatType == QgsWkbTypes::Polygon )
    geometryType = QStringLiteral( "POLYGON" );
  else if ( flatType == QgsWkbTypes::MultiPoint )
    geometryType = QStringLiteral( "MULTIPOINT" );
  else if ( flatType == QgsWkbTypes::MultiLineString )
    geometryType = QStringLiteral( "MULTILINESTRING" );
  else if ( flatType == QgsWkbTypes::MultiPolygon )
    geometryType = QStringLiteral( "MULTIPOLYGON" );
  else if ( flatType == QgsWkbTypes::Unknown )
    geometryType = QStringLiteral( "GEOMETRY" );
  else
    dim = 0;
}

QgsWkbTypes::Type QgsMssqlProvider::getWkbType( const QString &geometryType )
{
  return QgsWkbTypes::parseType( geometryType );
}


QgsVectorLayerExporter::ExportError QgsMssqlProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options );

  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );

  // connect to database
  QSqlDatabase db = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return QgsVectorLayerExporter::ErrConnectionFailed;
  }

  QString dbName = dsUri.database();

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  if ( schemaName.isEmpty() )
    schemaName = QStringLiteral( "dbo" );

  if ( wkbType != QgsWkbTypes::NoGeometry && geometryColumn.isEmpty() )
    geometryColumn = QStringLiteral( "geom" );

  if ( primaryKey.isEmpty() )
    primaryKey = QStringLiteral( "qgs_fid" );

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = QStringLiteral( "qgs_fid" );
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      if ( fields.at( i ).name() == primaryKey )
      {
        // it already exists, try again with a new name
        primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
        i = 0;
      }
    }
  }
  else
  {
    // search for the passed field
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      if ( fields.at( i ).name() == primaryKey )
      {
        // found, get the field type
        QgsField fld = fields.at( i );
        if ( convertField( fld ) )
        {
          primaryKeyType = fld.typeName();
        }
      }
    }
  }

  // if the field doesn't not exist yet, create it as a serial field
  if ( primaryKeyType.isEmpty() )
    primaryKeyType = QStringLiteral( "serial" );

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
    return QgsVectorLayerExporter::ErrCreateLayer;
  }

  // set up spatial reference id
  int srid = 0;
  if ( srs.isValid() )
  {
    srid = srs.srsid();
    QString auth_srid = QStringLiteral( "null" );
    QString auth_name = QStringLiteral( "null" );
    QStringList sl = srs.authid().split( ':' );
    if ( sl.length() == 2 )
    {
      auth_name = '\'' + sl[0] + '\'';
      auth_srid = sl[1];
    }
    sql = QStringLiteral( "IF NOT EXISTS (SELECT * FROM spatial_ref_sys WHERE srid=%1) INSERT INTO spatial_ref_sys (srid, auth_name, auth_srid, srtext, proj4text) VALUES (%1, %2, %3, '%4', '%5')" )
          .arg( srs.srsid() )
          .arg( auth_name,
                auth_srid,
                srs.toWkt(),
                srs.toProj4() );
    if ( !q.exec( sql ) )
    {
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return QgsVectorLayerExporter::ErrCreateLayer;
    }
  }

  // get wkb type and dimension
  QString geometryType;
  int dim = 2;
  mssqlWkbTypeAndDimension( wkbType, geometryType, dim );

  if ( overwrite )
  {
    // remove the old table with the same name
    sql = QStringLiteral( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) BEGIN DROP TABLE [%1].[%2] DELETE FROM geometry_columns where f_table_schema = '%1' and f_table_name = '%2' END;" )
          .arg( schemaName, tableName );
    if ( !q.exec( sql ) )
    {
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return QgsVectorLayerExporter::ErrCreateLayer;
    }
  }

  if ( !geometryColumn.isEmpty() )
  {
    sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                   "CREATE TABLE [%1].[%2]([%3] [int] IDENTITY(1,1) NOT NULL, [%4] [geometry] NULL CONSTRAINT [PK_%2] PRIMARY KEY CLUSTERED ( [%3] ASC ))\n"
                   "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'\n"
                   "INSERT INTO [geometry_columns] ([f_table_catalog], [f_table_schema],[f_table_name], "
                   "[f_geometry_column],[coord_dimension],[srid],[geometry_type]) VALUES ('%5', '%1', '%2', '%4', %6, %7, '%8')" )
          .arg( schemaName,
                tableName,
                primaryKey,
                geometryColumn,
                dbName,
                QString::number( dim ),
                QString::number( srid ),
                geometryType );
  }
  else
  {
    //geometryless table
    sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                   "CREATE TABLE [%1].[%2]([%3] [int] IDENTITY(1,1) NOT NULL CONSTRAINT [PK_%2] PRIMARY KEY CLUSTERED ( [%3] ASC ))\n"
                   "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'\n"
                 )
          .arg( schemaName,
                tableName,
                primaryKey );
  }

  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return QgsVectorLayerExporter::ErrCreateLayer;
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
    return QgsVectorLayerExporter::ErrInvalidLayer;
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
      QgsField fld = fields.at( i );
      if ( oldToNewAttrIdxMap && fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( fields.lookupField( fld.name() ), 0 );
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
        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fields.lookupField( fld.name() ), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      delete provider;
      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }
  }
  return QgsVectorLayerExporter::NoError;
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

#ifdef HAVE_GUI
QGISEXTERN void *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsMssqlSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  Q_UNUSED( path );
  return new QgsMssqlRootItem( parentItem, QStringLiteral( "MSSQL" ), QStringLiteral( "mssql:" ) );
}

QGISEXTERN QgsVectorLayerExporter::ExportError createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
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
QGISEXTERN bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                           const QString &styleName, const QString &styleDescription,
                           const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  // connect to database
  QSqlDatabase mDatabase = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( mDatabase ) )
  {
    QgsDebugMsg( "Error connecting to database" );
    QgsDebugMsg( mDatabase.lastError().text() );
    return false;
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( QStringLiteral( "SELECT COUNT(*) FROM information_schema.tables WHERE table_name= N'layer_styles'" ) ) )
  {
    QgsDebugMsg( query.lastError().text() );
    return false;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsg( "Need to create styles table" );
    bool execOk = query.exec( QString( "CREATE TABLE [dbo].[layer_styles]("
                                       "[id] int IDENTITY(1,1) PRIMARY KEY,"
                                       "[f_table_catalog] [varchar](1024) NULL,"
                                       "[f_table_schema] [varchar](1024) NULL,"
                                       "[f_table_name] [varchar](1024) NULL,"
                                       "[f_geometry_column] [varchar](1024) NULL,"
                                       "[styleName] [varchar](1024) NULL,"
                                       "[styleQML] [text] NULL,"
                                       "[styleSLD] [text] NULL,"
                                       "[useAsDefault] [int] NULL,"
                                       "[description] [text] NULL,"
                                       "[owner] [varchar](1024) NULL,"
                                       "[ui] [text] NULL,"
                                       "[update_time] [datetime] NULL"
                                       ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]" ) );
    if ( !execOk )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database. Maybe this is due to table permissions. Please contact your database admin" );
      return false;
    }
    query.finish();
    query.clear();
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QStringLiteral( ",XMLPARSE(DOCUMENT %1)" ).arg( uiFileContent );
  }
  QgsDebugMsg( "Ready to insert new style" );
  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and sldStyle values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  QString sql = QString( "INSERT INTO layer_styles"
                         "(f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10%12"
                         ")" )
                .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsMssqlProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( QgsMssqlProvider::quotedValue( qmlStyle ) )
                .arg( QgsMssqlProvider::quotedValue( sldStyle ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsMssqlProvider::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsMssqlProvider::quotedValue( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_catalog=%1"
                                " AND f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column=%4"
                                " AND styleName=%5" )
                       .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                       .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                       .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                       .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) )
                       .arg( QgsMssqlProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  if ( !query.exec( checkQuery ) )
  {
    QgsDebugMsg( query.lastError().text() );
    QgsDebugMsg( "Check Query failed" );
    return false;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toString() == styleName )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      errCause = QObject::tr( "Operation aborted. No changes were made in the database" );
      QgsDebugMsg( "User selected not to overwrite styles" );
      return false;
    }

    QgsDebugMsg( "Updating styles" );
    sql = QString( "UPDATE layer_styles "
                   " SET useAsDefault=%1"
                   ",styleQML=%2"
                   ",styleSLD=%3"
                   ",description=%4"
                   ",owner=%5"
                   " WHERE f_table_catalog=%6"
                   " AND f_table_schema=%7"
                   " AND f_table_name=%8"
                   " AND f_geometry_column=%9"
                   " AND styleName=%10" )
          .arg( useAsDefault ? "1" : "0" )
          .arg( QgsMssqlProvider::quotedValue( qmlStyle ) )
          .arg( QgsMssqlProvider::quotedValue( sldStyle ) )
          .arg( QgsMssqlProvider::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsMssqlProvider::quotedValue( dsUri.username() ) )
          .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
          .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
          .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
          .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsMssqlProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );
  }
  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles "
                                        " SET useAsDefault=0"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4" )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );
    sql = QStringLiteral( "%1; %2;" ).arg( removeDefaultSql, sql );
  }

  QgsDebugMsg( "Inserting styles" );
  QgsDebugMsg( sql );
  bool execOk = query.exec( sql );

  if ( !execOk )
  {
    errCause = QObject::tr( "Unable to save layer style. It's not possible to insert a new record into the style table. Maybe this is due to table permissions. Please contact your database administrator." );
  }
  return execOk;
}


QGISEXTERN QString loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  // connect to database
  QSqlDatabase mDatabase = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( mDatabase ) )
  {
    QgsDebugMsg( "Error connecting to database" );
    QgsDebugMsg( mDatabase.lastError().text() );
    return QString();
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  QString selectQmlQuery = QString( "SELECT top 1 styleQML"
                                    " FROM layer_styles"
                                    " WHERE f_table_catalog=%1"
                                    " AND f_table_schema=%2"
                                    " AND f_table_name=%3"
                                    " AND f_geometry_column=%4"
                                    " ORDER BY useAsDefault desc" )
                           .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                           .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                           .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                           .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );

  if ( !query.exec( selectQmlQuery ) )
  {
    QgsDebugMsg( "Load of style failed" );
    QString msg = query.lastError().text();
    errCause = msg;
    QgsDebugMsg( msg );
    return QString();
  }
  if ( query.isActive() && query.next() )
  {
    QString style = query.value( 0 ).toString();
    return style;
  }
  return QString();
}

QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  // connect to database
  QSqlDatabase mDatabase = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( mDatabase ) )
  {
    QgsDebugMsg( "Error connecting to database" );
    QgsDebugMsg( mDatabase.lastError().text() );
    return -1;
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  // check if layer_styles table already exist
  if ( !query.exec( QStringLiteral( "SELECT COUNT(*) FROM information_schema.tables WHERE table_name= N'layer_styles'" ) ) )
  {
    QString msg = query.lastError().text();
    errCause = msg;
    QgsDebugMsg( msg );
    return -1;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsg( QObject::tr( "No styles available on DB, or there is an error connecting to the database." ) );
    return -1;
  }

  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles "
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4" )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                               .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );
  bool queryOk = query.exec( selectRelatedQuery );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    return -1;
  }
  int numberOfRelatedStyles = 0;
  while ( query.isActive() && query.next() )
  {
    QgsDebugMsg( query.value( 1 ).toString() );
    ids.append( query.value( 0 ).toString() );
    names.append( query.value( 1 ).toString() );
    descriptions.append( query.value( 2 ).toString() );
    numberOfRelatedStyles = numberOfRelatedStyles + 1;
  }
  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM layer_styles "
                                       " WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4)"
                                       " ORDER BY update_time DESC" )
                              .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                              .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                              .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                              .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );
  QgsDebugMsg( selectOthersQuery );
  queryOk = query.exec( selectOthersQuery );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    return -1;
  }
  QgsDebugMsg( query.isActive() && query.size() );
  while ( query.next() )
  {
    ids.append( query.value( 0 ).toString() );
    names.append( query.value( 1 ).toString() );
    descriptions.append( query.value( 2 ).toString() );
  }
  return numberOfRelatedStyles;
}
QGISEXTERN QString getStyleById( const QString &uri, QString styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  // connect to database
  QSqlDatabase mDatabase = QgsMssqlProvider::GetDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlProvider::OpenDatabase( mDatabase ) )
  {
    QgsDebugMsg( "Error connecting to database" );
    QgsDebugMsg( mDatabase.lastError().text() );
    return QString();
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  QString style = QLatin1String( "" );
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsMssqlProvider::quotedValue( styleId ) );
  bool queryOk = query.exec( selectQmlQuery );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    errCause = query.lastError().text();
    return QString();
  }
  while ( query.next() )
  {
    style = query.value( 0 ).toString();
  }
  return style;
}


#ifdef HAVE_GUI

//! Provider for msssql raster source select
class QgsMssqlSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    virtual QString providerKey() const override { return QStringLiteral( "mssql" ); }
    virtual QString text() const override { return QObject::tr( "MSSQL" ); }
    virtual int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 30; }
    virtual QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMssqlLayer.svg" ) ); }
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsMssqlSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsMssqlSourceSelectProvider ;

  return providers;
}

#endif
