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
#include "qgsmssqlconnection.h"
#include "qgsmssqldatabase.h"
#include "qgsmssqlproviderconnection.h"
#include "qgsfeedback.h"
#include "qgsdbquerylog.h"

#include <QtGlobal>
#include <QFileInfo>
#include <QDataStream>
#include <QStringList>
#include <QSettings>
#include <QRegularExpression>
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
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgis.h"

#include "qgsmssqldataitems.h"
#include "qgsmssqlfeatureiterator.h"
#include "qgsmssqltransaction.h"


#include "qgsconfig.h"
constexpr int sMssqlConQueryLogFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define LoggedExec(query, sql ) execLogged( query, sql, QString(QString( __FILE__ ).mid( sMssqlConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") )
#define LoggedExecMetadata(query, sql, uri ) execLogged( query, sql, uri, QString(QString( __FILE__ ).mid( sMssqlConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") )


const QString QgsMssqlProvider::MSSQL_PROVIDER_KEY = QStringLiteral( "mssql" );
const QString QgsMssqlProvider::MSSQL_PROVIDER_DESCRIPTION = QStringLiteral( "MSSQL spatial data provider" );
int QgsMssqlProvider::sConnectionId = 0;

QgsMssqlProvider::QgsMssqlProvider( const QString &uri, const ProviderOptions &options,
                                    QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
  , mShared( new QgsMssqlSharedData )
{
  const QgsDataSourceUri anUri = QgsDataSourceUri( uri );

  if ( !anUri.srid().isEmpty() )
    mSRId = anUri.srid().toInt();
  else
    mSRId = -1;

  mWkbType = anUri.wkbType();

  mValid = true;

  mUserName = anUri.username();
  mPassword = anUri.password();
  mService = anUri.service();
  mDatabaseName = anUri.database();
  mHost = anUri.host();

  mUseEstimatedMetadata = anUri.useEstimatedMetadata();
  if ( mReadFlags & QgsDataProvider::FlagTrustDataSource )
  {
    mUseEstimatedMetadata = true;
  }

  mDisableInvalidGeometryHandling = anUri.hasParam( QStringLiteral( "disableInvalidGeometryHandling" ) )
                                    ? anUri.param( QStringLiteral( "disableInvalidGeometryHandling" ) ).toInt()
                                    : false;

  mUseGeometryColumnsTableForExtent = anUri.hasParam( QStringLiteral( "extentInGeometryColumns" ) )
                                      ? anUri.param( QStringLiteral( "extentInGeometryColumns" ) ).toInt()
                                      : false;

  mSqlWhereClause = anUri.sql();

  mConn = QgsMssqlDatabase::connectDb( mService, mHost, mDatabaseName, mUserName, mPassword, false );
  if ( !mConn )
  {
    mValid = false;
    return;
  }
  QSqlDatabase db = mConn->db();

  if ( !db.isOpen() )
  {
    setLastError( db.lastError().text() );
    QgsDebugMsg( mLastError );
    mValid = false;
    return;
  }

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
    mTables = db.tables( QSql::Tables );
    if ( !mTables.isEmpty() )
      mTableName = mTables[0];
    else
      mValid = false;
  }

  if ( mValid )
  {
    if ( !anUri.geometryColumn().isEmpty() )
      mGeometryColName = anUri.geometryColumn();

    if ( mSRId < 0 || mWkbType == QgsWkbTypes::Unknown || mGeometryColName.isEmpty() )
    {
      loadMetadata();
    }
    loadFields();

    UpdateStatistics( mUseEstimatedMetadata );

    //only for views, defined in layer data when loading layer for first time
    bool primaryKeyFromGeometryColumnsTable = anUri.hasParam( QStringLiteral( "primaryKeyInGeometryColumns" ) )
        ? anUri.param( QStringLiteral( "primaryKeyInGeometryColumns" ) ).toInt()
        : false;

    QStringList cols;
    if ( primaryKeyFromGeometryColumnsTable )
    {
      mPrimaryKeyType = PktUnknown;
      mPrimaryKeyAttrs.clear();
      primaryKeyFromGeometryColumnsTable = getPrimaryKeyFromGeometryColumns( cols );
      if ( !primaryKeyFromGeometryColumnsTable )
        QgsMessageLog::logMessage( tr( "Invalid primary key from geometry_columns table for layer '%1', get primary key from the layer." )
                                   .arg( anUri.table() ), tr( "MSSQL" ) );
    }

    if ( !primaryKeyFromGeometryColumnsTable )
    {
      const QString primaryKey = anUri.keyColumn();
      if ( !primaryKey.isEmpty() )
      {

        mPrimaryKeyAttrs.clear();
        cols = parseUriKey( primaryKey );
      }
    }

    if ( mValid )
    {
      for ( const QString &col : cols )
      {
        const int idx = mAttributeFields.indexFromName( col );
        if ( idx < 0 )
        {
          QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( col ), tr( "MSSQL" ) );
          mPrimaryKeyType = PktUnknown;
          mPrimaryKeyAttrs.clear();
          break;
        }

        const QgsField &fld = mAttributeFields.at( idx );

        if ( mPrimaryKeyAttrs.size() == 0 &&
             ( fld.type() == QVariant::Int ||
               fld.type() == QVariant::LongLong ||
               ( fld.type() == QVariant::Double && fld.precision() == 0 ) ) )
        {
          mPrimaryKeyType = PktInt;
        }
        else
        {
          mPrimaryKeyType = PktFidMap;
        }

        mPrimaryKeyAttrs << idx;
      }

      if ( mGeometryColName.isEmpty() )
      {
        // table contains no geometries
        mWkbType = QgsWkbTypes::NoGeometry;
        mSRId = 0;
      }
    }
  }

  //fill type names into sets
  setNativeTypes( QgsMssqlConnection::nativeTypes() );
}

QgsMssqlProvider::~QgsMssqlProvider()
{
}

QgsAbstractFeatureSource *QgsMssqlProvider::featureSource() const
{
  return new QgsMssqlFeatureSource( this );
}

QgsFeatureIterator QgsMssqlProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsDebugMsg( QStringLiteral( "Read attempt on an invalid mssql data source" ) );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsMssqlFeatureIterator( new QgsMssqlFeatureSource( this ), true, request ) );
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
    QgsDebugMsg( QStringLiteral( "Unknown field type: %1" ).arg( sqlTypeName ) );
    // Everything else just dumped as a string.
    type = QVariant::String;
  }

  return type;
}

void QgsMssqlProvider::loadMetadata()
{
  mSRId = 0;
  mWkbType = QgsWkbTypes::Unknown;

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );
  if ( !LoggedExec( query, QStringLiteral( "SELECT f_geometry_column, srid, geometry_type, coord_dimension FROM geometry_columns WHERE f_table_schema=%1 AND f_table_name=%2" ).arg( quotedValue( mSchemaName ), quotedValue( mTableName ) ) ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( query.isActive() && query.next() )
  {
    mGeometryColName = query.value( 0 ).toString();
    mSRId = query.value( 1 ).toInt();
    QString detectedType = query.value( 2 ).toString();
    const QString dim = query.value( 3 ).toString();
    if ( dim == QLatin1String( "3" ) && !detectedType.endsWith( 'M' ) )
      detectedType += QLatin1Char( 'Z' );
    else if ( dim == QLatin1String( "4" ) )
      detectedType += QLatin1String( "ZM" );
    mWkbType = getWkbType( detectedType );
  }
}

bool QgsMssqlProvider::execLogged( QSqlQuery &qry, const QString &sql, const QString &queryOrigin ) const
{
  QgsDatabaseQueryLogWrapper logWrapper{ sql, uri().uri(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ),  queryOrigin };
  const bool res { qry.exec( sql ) };
  if ( ! res )
  {
    logWrapper.setError( qry.lastError().text() );
  }
  else
  {
    if ( qry.isSelect() )
    {
      logWrapper.setFetchedRows( qry.size() );
    }
    else
    {
      logWrapper.setFetchedRows( qry.numRowsAffected() );
    }
  }
  logWrapper.setQuery( qry.lastQuery() );
  return res;
}

void QgsMssqlProvider::setLastError( const QString &error )
{
  appendError( error );
  mLastError = error;
}

QSqlQuery QgsMssqlProvider::createQuery() const
{
  std::shared_ptr<QgsMssqlDatabase> conn = connection();
  QSqlDatabase d = conn->db();
  if ( !d.isOpen() )
  {
    QgsDebugMsg( "Creating query, but the database is not open!" );
  }
  return QSqlQuery( d );
}

void QgsMssqlProvider::loadFields()
{
  bool isIdentity = false;
  mAttributeFields.clear();
  mDefaultValues.clear();
  mComputedColumns.clear();

  // get field spec
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  const QString sql { QStringLiteral( "SELECT name FROM sys.columns WHERE is_computed = 1 AND object_id = OBJECT_ID('[%1].[%2]')" ).arg( mSchemaName, mTableName ) };

  // Get computed columns which need to be ignored on insert or update.
  if ( !LoggedExec( query, sql ) )
  {
    pushError( query.lastError().text() );
    return;
  }

  while ( query.next() )
  {
    mComputedColumns.append( query.value( 0 ).toString() );
  }

  // Field has unique constraint
  QSet<QString> setColumnUnique;
  {
    const QString sql2 { QStringLiteral( "SELECT * FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS TC"
                                         " INNER JOIN INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE CC ON TC.CONSTRAINT_NAME = CC.CONSTRAINT_NAME"
                                         " WHERE TC.CONSTRAINT_SCHEMA = '%1' AND TC.TABLE_NAME = '%2' AND TC.CONSTRAINT_TYPE = 'unique'" )
                         .arg( mSchemaName, mTableName ) };
    if ( !LoggedExec( query, sql2 ) )
    {
      pushError( query.lastError().text() );
      return;
    }

    while ( query.next() )
    {
      setColumnUnique.insert( query.value( QStringLiteral( "COLUMN_NAME" ) ).toString() );
    }
  }

  const QString sql3 { QStringLiteral( "exec sp_columns @table_name = N%1, @table_owner = %2" ).arg( quotedValue( mTableName ), quotedValue( mSchemaName ) ) };
  if ( !LoggedExec( query, sql3 ) )
  {
    pushError( query.lastError().text() );
    return;
  }

  int i = 0;
  QStringList pkCandidates;
  while ( query.next() )
  {
    const QString colName = query.value( QStringLiteral( "COLUMN_NAME" ) ).toString();
    const QString sqlTypeName = query.value( QStringLiteral( "TYPE_NAME" ) ).toString();
    bool columnIsIdentity = false;

    // if we don't have an explicitly set geometry column name, and this is a geometry column, then use it
    // but if we DO have an explicitly set geometry column name, then load the other information if this is that column
    if ( ( mGeometryColName.isEmpty() && ( sqlTypeName == QLatin1String( "geometry" ) || sqlTypeName == QLatin1String( "geography" ) ) )
         || colName == mGeometryColName )
    {
      mGeometryColName = colName;
      mGeometryColType = sqlTypeName;
      mParser.mIsGeography = sqlTypeName == QLatin1String( "geography" );
    }
    else
    {
      const QVariant::Type sqlType = DecodeSqlType( sqlTypeName );
      if ( sqlTypeName == QLatin1String( "int identity" ) || sqlTypeName == QLatin1String( "bigint identity" ) )
      {
        mPrimaryKeyType = PktInt;
        mPrimaryKeyAttrs << mAttributeFields.size();
        columnIsIdentity = true;
        isIdentity = true;
      }
      else if ( sqlTypeName == QLatin1String( "int" ) || sqlTypeName == QLatin1String( "bigint" ) )
      {
        pkCandidates << colName;
      }

      QgsField field;
      if ( sqlType == QVariant::String )
      {
        // Field length in chars is column 7 ("Length") of the sp_columns output,
        // except for uniqueidentifiers which must use column 6 ("Precision").
        int length = query.value( sqlTypeName.startsWith( QStringLiteral( "uniqueidentifier" ), Qt::CaseInsensitive ) ? 6 : 7 ).toInt();
        if ( sqlTypeName.startsWith( QLatin1Char( 'n' ) ) )
        {
          length = length / 2;
        }
        field = QgsField( colName,
                          sqlType,
                          sqlTypeName,
                          length );
      }
      else if ( sqlType == QVariant::Double )
      {
        field = QgsField( colName,
                          sqlType,
                          sqlTypeName,
                          query.value( QStringLiteral( "PRECISION" ) ).toInt(),
                          sqlTypeName == QLatin1String( "decimal" ) ? query.value( QStringLiteral( "SCALE" ) ).toInt() : -1 );
      }
      else if ( sqlType == QVariant::Date || sqlType == QVariant::DateTime || sqlType == QVariant::Time )
      {
        field = QgsField( colName,
                          sqlType,
                          sqlTypeName,
                          -1,
                          -1 );
      }
      else
      {
        field = QgsField( colName,
                          sqlType,
                          sqlTypeName );
      }

      // Field nullable
      const bool nullable = query.value( QStringLiteral( "NULLABLE" ) ).toBool();

      // Set constraints
      QgsFieldConstraints constraints;
      if ( !nullable )
        constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      if ( setColumnUnique.contains( colName ) )
        constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      field.setConstraints( constraints );

      if ( columnIsIdentity )
      {
        field.setReadOnly( true );
      }

      mAttributeFields.append( field );

      // Default value
      if ( !query.value( QStringLiteral( "COLUMN_DEF" ) ).isNull() )
      {
        mDefaultValues.insert( i, query.value( QStringLiteral( "COLUMN_DEF" ) ).toString() );
      }
      else if ( columnIsIdentity )
      {
        // identity column types don't report a default value clause in the COLUMN_DEF attribute. So we need to fake
        // one, so that we can correctly indicate that the database is responsible for populating this column.
        mDefaultValues.insert( i, QStringLiteral( "Autogenerate" ) );
      }

      ++i;
    }
  }

  // get primary key
  if ( mPrimaryKeyAttrs.isEmpty() )
  {
    query.clear();
    query.setForwardOnly( true );
    const QString sql4 { QStringLiteral( "exec sp_pkeys @table_name = N%1, @table_owner = %2 " ).arg( quotedValue( mTableName ), quotedValue( mSchemaName ) ) };
    if ( !LoggedExec( query, sql4 ) )
    {
      QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
    }

    if ( query.isActive() )
    {
      mPrimaryKeyType = PktInt;

      while ( query.next() )
      {
        const QString fidColName = query.value( 3 ).toString();
        const int idx = mAttributeFields.indexFromName( fidColName );
        const QgsField &fld = mAttributeFields.at( idx );

        if ( !mPrimaryKeyAttrs.isEmpty() ||
             ( fld.type() != QVariant::Int &&
               fld.type() != QVariant::LongLong &&
               ( fld.type() != QVariant::Double || fld.precision() != 0 ) ) )
          mPrimaryKeyType = PktFidMap;

        mPrimaryKeyAttrs << idx;
      }

      if ( mPrimaryKeyAttrs.isEmpty() )
      {
        mPrimaryKeyType = PktUnknown;
      }
    }
  }

  if ( mPrimaryKeyAttrs.isEmpty() )
  {
    const auto constPkCandidates = pkCandidates;
    for ( const QString &pk : constPkCandidates )
    {
      query.clear();
      query.setForwardOnly( true );
      const QString sql5 { QStringLiteral( "select count(distinct [%1]), count([%1]) from [%2].[%3]" )
                           .arg( pk, mSchemaName, mTableName ) };
      if ( !LoggedExec( query, sql5 ) )
      {
        QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
      }

      if ( query.isActive() && query.next() && query.value( 0 ).toInt() == query.value( 1 ).toInt() )
      {
        mPrimaryKeyType = PktInt;
        mPrimaryKeyAttrs << mAttributeFields.indexFromName( pk );
        return;
      }
    }
  }

  if ( mPrimaryKeyAttrs.isEmpty() )
  {
    const QString error = QStringLiteral( "No primary key could be found on table %1" ).arg( mTableName );
    QgsDebugMsg( error );
    mValid = false;
    setLastError( error );
  }

  if ( mPrimaryKeyAttrs.size() == 1 && !isIdentity )
  {
    // primary key has unique constraints
    QgsFieldConstraints constraints = mAttributeFields.at( mPrimaryKeyAttrs[0] ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ mPrimaryKeyAttrs[0] ].setConstraints( constraints );
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
      return QString( value.toBool() ? '1' : '0' );

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

QString QgsMssqlProvider::quotedIdentifier( const QString &value )
{
  return QStringLiteral( "[%1]" ).arg( value );
}

QString QgsMssqlProvider::defaultValueClause( int fieldId ) const
{
  const QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( defVal.isEmpty() )
    return QString();

  // NOTE: If EvaluateDefaultValues is activated it is impossible to get the defaultValueClause.
  //       This also apply to QgsPostgresProvider::defaultValueClause.
  if ( !providerProperty( EvaluateDefaultValues, false ).toBool() )
    return defVal;

  return QString();
}

QVariant QgsMssqlProvider::defaultValue( int fieldId ) const
{
  const QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( defVal.isEmpty() )
    return QVariant();

  if ( !providerProperty( EvaluateDefaultValues, false ).toBool() )
    return QVariant();

  const QString sql = QStringLiteral( "select %1" )
                      .arg( defVal );

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, sql ) )
  {
    const QString errorMessage( tr( "Could not execute query: %1" ).arg( query.lastError().text() ) );
    QgsDebugMsg( errorMessage );
    pushError( errorMessage );
    return QVariant();
  }

  if ( !query.next() )
  {
    const QString errorMessage( tr( "Could not fetch next query value: %1" ).arg( query.lastError().text() ) );
    QgsDebugMsg( errorMessage );
    pushError( errorMessage );
    return QVariant();
  }

  return query.value( 0 );
}

QString QgsMssqlProvider::storageType() const
{
  return QStringLiteral( "MSSQL spatial database" );
}

QVariant QgsMssqlProvider::convertTimeValue( const QVariant &value )
{
  if ( value.isValid() && value.type() == QVariant::ByteArray )
  {
    // time fields can be returned as byte arrays... woot
    const QByteArray ba = value.toByteArray();
    if ( ba.length() >= 5 )
    {
      const int hours = ba.at( 0 );
      const int mins = ba.at( 2 );
      const int seconds = ba.at( 4 );
      QVariant t = QTime( hours, mins, seconds );
      if ( !t.isValid() ) // can't handle it
        t = QVariant( QVariant::Time );
      return t;
    }
    return QVariant( QVariant::Time );
  }
  return value;
}

// Returns the minimum value of an attribute
QVariant QgsMssqlProvider::minimumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }

  // get the field name
  const QgsField &fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select min([%1]) from " )
                .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, sql ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( query.isActive() && query.next() )
  {
    QVariant v = query.value( 0 );
    if ( fld.type() == QVariant::Time )
      v = convertTimeValue( v );
    if ( v.type() != fld.type() )
      v = convertValue( fld.type(), v.toString() );
    return v;
  }

  return QVariant( QString() );
}

// Returns the maximum value of an attribute
QVariant QgsMssqlProvider::maximumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }

  // get the field name
  const QgsField &fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select max([%1]) from " )
                .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " where (%1)" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, sql ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( query.isActive() && query.next() )
  {
    QVariant v = query.value( 0 );
    if ( fld.type() == QVariant::Time )
      v = convertTimeValue( v );
    if ( v.type() != fld.type() )
      v = convertValue( fld.type(), v.toString() );
    return v;
  }

  return QVariant( QString() );
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsMssqlProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return uniqueValues;
  }

  // get the field name
  const QgsField &fld = mAttributeFields.at( index );
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

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, sql ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( query.isActive() )
  {
    // read all features
    while ( query.next() )
    {
      QVariant v = query.value( 0 );
      if ( fld.type() == QVariant::Time )
        v = convertTimeValue( v );
      if ( v.type() != fld.type() )
        v = convertValue( fld.type(), v.toString() );
      uniqueValues.insert( v );
    }
  }
  return uniqueValues;
}

QStringList QgsMssqlProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return results;
  }

  // get the field name
  const QgsField &fld = mAttributeFields.at( index );
  QString sql = QStringLiteral( "select distinct " );

  if ( limit > 0 )
  {
    sql += QStringLiteral( " top %1 " ).arg( limit );
  }

  sql += QStringLiteral( "[%1] from " )
         .arg( fld.name() );

  sql += QStringLiteral( "[%1].[%2] WHERE" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " (%1) AND" ).arg( mSqlWhereClause );
  }

  sql += QStringLiteral( " [%1] LIKE '%%2%'" ).arg( fld.name(), substring );

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, sql ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( query.isActive() )
  {
    // read all features
    while ( query.next() )
    {
      results << query.value( 0 ).toString();
      if ( feedback && feedback->isCanceled() )
        break;
    }
  }
  return results;
}

// update the extent, wkb type and srid for this layer, returns false if fails
void QgsMssqlProvider::UpdateStatistics( bool estimate ) const
{
  if ( mGeometryColName.isEmpty() )
  {
    return;
  }

  // get features to calculate the statistics
  QString statement;

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );


  if ( mUseGeometryColumnsTableForExtent )
  {
    if ( !getExtentFromGeometryColumns( mExtent ) )
      QgsMessageLog::logMessage( tr( "Invalid extent from geometry_columns table for layer '%1', get extent from the layer." ).arg( mTableName ), tr( "MSSQL" ) );
    else
      return;
  }

  // Get the extents from the spatial index table to speed up load times.
  // We have to use max() and min() because you can have more then one index but the biggest area is what we want to use.
  const QString sql = "SELECT min(bounding_box_xmin), min(bounding_box_ymin), max(bounding_box_xmax), max(bounding_box_ymax)"
                      " FROM sys.spatial_index_tessellations WHERE object_id = OBJECT_ID('[%1].[%2]')";

  statement = QString( sql ).arg( mSchemaName, mTableName );

  if ( LoggedExec( query, statement ) )
  {
    if ( query.next() && ( !query.value( 0 ).isNull() ||
                           !query.value( 1 ).isNull() ||
                           !query.value( 2 ).isNull() ||
                           !query.value( 3 ).isNull() ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Found extents in spatial index" ), 2 );
      mExtent.setXMinimum( query.value( 0 ).toDouble() );
      mExtent.setYMinimum( query.value( 1 ).toDouble() );
      mExtent.setXMaximum( query.value( 2 ).toDouble() );
      mExtent.setYMaximum( query.value( 3 ).toDouble() );
      return;
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  // If we can't find the extents in the spatial index table just do what we normally do.
  bool readAllGeography = false;
  if ( estimate )
  {
    if ( mGeometryColType == QLatin1String( "geometry" ) )
    {
      if ( mDisableInvalidGeometryHandling )
        statement = QStringLiteral( "select min([%1].STPointN(1).STX), min([%1].STPointN(1).STY), max([%1].STPointN(1).STX), max([%1].STPointN(1).STY)" ).arg( mGeometryColName );
      else
        statement = QStringLiteral( "select min(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).STX else NULL end), min(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).STY else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).STX else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).STY else NULL end)" ).arg( mGeometryColName );
    }
    else
    {
      if ( mDisableInvalidGeometryHandling )
        statement = QStringLiteral( "select min([%1].STPointN(1).Long), min([%1].STPointN(1).Lat), max([%1].STPointN(1).Long), max([%1].STPointN(1).Lat)" ).arg( mGeometryColName );
      else
        statement = QStringLiteral( "select min(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).Long  else NULL end), min(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).Lat else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).Long else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STPointN(1).Lat else NULL end)" ).arg( mGeometryColName );
    }

    // we will first try to sample a small portion of the table/view, so the count of rows involved
    // will be useful to evaluate if we have enough data to use the sample
    statement += ", count(*)";
  }
  else
  {
    if ( mGeometryColType == QLatin1String( "geometry" ) )
    {
      if ( mDisableInvalidGeometryHandling )
        statement = QStringLiteral( "select min([%1].STEnvelope().STPointN(1).STX), min([%1].STEnvelope().STPointN(1).STY), max([%1].STEnvelope().STPointN(3).STX), max([%1].STEnvelope().STPointN(3).STY)" ).arg( mGeometryColName );
      else
        statement = QStringLiteral( "select min(case when ([%1].STIsValid() = 1) THEN [%1].STEnvelope().STPointN(1).STX  else NULL end), min(case when ([%1].STIsValid() = 1) THEN [%1].STEnvelope().STPointN(1).STY else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STEnvelope().STPointN(3).STX else NULL end), max(case when ([%1].STIsValid() = 1) THEN [%1].STEnvelope().STPointN(3).STY else NULL end)" ).arg( mGeometryColName );
    }
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

  if ( estimate )
  {
    // Try to use just 1% sample of the whole table/view to limit the amount of rows accessed.
    // This heuristic may fail (e.g. when the table is small or when primary key values do not
    // get sampled enough) so in case we do not have at least 10 features, we fall back to full
    // traversal of the table/view

    const int minSampleCount = 10;

    QString cols, delim;
    for ( const auto idx : mPrimaryKeyAttrs )
    {
      const QgsField &fld = mAttributeFields.at( idx );
      cols += delim + QStringLiteral( "[%1]" ).arg( fld.name() );
      delim = QStringLiteral( "," );
    }

    // See https://docs.microsoft.com/en-us/previous-versions/software-testing/cc441928(v=msdn.10)
    const QString sampleFilter = QString( "(ABS(CAST((BINARY_CHECKSUM([%1])) as int)) % 100) = 42" ).arg( cols );

    const QString statementSample = statement + ( mSqlWhereClause.isEmpty() ? " WHERE " : " AND " ) + sampleFilter;

    if ( LoggedExec( query, statementSample ) && query.next() &&
         !query.value( 0 ).isNull() && query.value( 4 ).toInt() >= minSampleCount )
    {
      mExtent.setXMinimum( query.value( 0 ).toDouble() );
      mExtent.setYMinimum( query.value( 1 ).toDouble() );
      mExtent.setXMaximum( query.value( 2 ).toDouble() );
      mExtent.setYMaximum( query.value( 3 ).toDouble() );
      return;
    }
  }

  if ( !LoggedExec( query, statement ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
  }

  if ( !query.isActive() )
  {
    return;
  }

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
    std::unique_ptr<QgsAbstractGeometry> geom = mParser.parseSqlGeometry( reinterpret_cast< unsigned char * >( ar.data() ), ar.size() );
    if ( geom )
    {
      const QgsRectangle rect = geom->boundingBox();

      if ( rect.xMinimum() < mExtent.xMinimum() )
        mExtent.setXMinimum( rect.xMinimum() );
      if ( rect.yMinimum() < mExtent.yMinimum() )
        mExtent.setYMinimum( rect.yMinimum() );
      if ( rect.xMaximum() > mExtent.xMaximum() )
        mExtent.setXMaximum( rect.xMaximum() );
      if ( rect.yMaximum() > mExtent.yMaximum() )
        mExtent.setYMaximum( rect.yMaximum() );

      mWkbType = geom->wkbType();
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
 * Returns the feature type
 */
QgsWkbTypes::Type QgsMssqlProvider::wkbType() const
{
  return mWkbType;
}

/**
 * Returns the feature type
 */
long long QgsMssqlProvider::featureCount() const
{
  // Return the count that we get from the subset.
  if ( !mSqlWhereClause.isEmpty() )
    return mNumberFeatures;

  // If there is no subset set we can get the count from the system tables.
  // Which is faster then doing select count(*)
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  const QString statement = QStringLiteral(
                              "SELECT rows"
                              " FROM sys.tables t"
                              " JOIN sys.partitions p ON t.object_id = p.object_id AND p.index_id IN (0,1)"
                              " WHERE SCHEMA_NAME(t.schema_id) = %1 AND OBJECT_NAME(t.OBJECT_ID) = %2" ).arg( quotedValue( mSchemaName ), quotedValue( mTableName ) );

  if ( LoggedExec( query, statement ) && query.next() )
  {
    return query.value( 0 ).toLongLong();
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
    if ( it->hasGeometry() && mWkbType == QgsWkbTypes::NoGeometry )
    {
      it->clearGeometry();
    }
    else if ( it->hasGeometry() && QgsWkbTypes::geometryType( it->geometry().wkbType() ) !=
              QgsWkbTypes::geometryType( mWkbType ) )
    {
      pushError( tr( "Could not add feature with geometry type %1 to layer of type %2" ).arg( QgsWkbTypes::displayString( it->geometry().wkbType() ),
                 QgsWkbTypes::displayString( mWkbType ) ) );
      if ( !mSkipFailures )
        return false;

      continue;
    }

    QString statement;
    QString values;
    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      statement += QLatin1String( "DECLARE @px TABLE (" );

      QString delim;
      for ( const auto idx : mPrimaryKeyAttrs )
      {
        const QgsField &fld = mAttributeFields.at( idx );

        QString type = fld.typeName();
        if ( type.endsWith( QLatin1String( " identity" ) ) )
          type = type.left( type.length() - 9 );
        if ( type == QLatin1String( "char" ) || type == QLatin1String( "varchar" ) )
        {
          if ( fld.length() > 0 )
            type = QStringLiteral( "%1(%2)" ).arg( type ).arg( fld.length() );
        }
        else if ( type == QLatin1String( "numeric" ) || type == QLatin1String( "decimal" ) )
        {
          if ( fld.length() > 0 && fld.precision() > 0 )
            type = QStringLiteral( "%1(%2,%3)" ).arg( type ).arg( fld.length() ).arg( fld.precision() );
        }

        statement += delim + QStringLiteral( "[%1] %2" ).arg( fld.name(), type );
        delim = ",";
      }

      statement += "); ";
    }

    statement += QStringLiteral( "INSERT INTO [%1].[%2] (" ).arg( mSchemaName, mTableName );

    bool first = true;
    QSqlQuery query = createQuery();
    query.setForwardOnly( true );

    const QgsAttributes attrs = it->attributes();

    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i >= mAttributeFields.count() )
        break;

      const QgsField &fld = mAttributeFields.at( i );

      if ( fld.typeName().compare( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) == 0 )
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
      values += QLatin1Char( '?' );
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
          values += QStringLiteral( "geometry::STGeomFromWKB(?,%1).MakeValid()" ).arg( mSRId );
        else
          values += QStringLiteral( "geometry::STGeomFromText(?,%1).MakeValid()" ).arg( mSRId );
      }
      else
      {
        if ( mUseWkb )
          values += QStringLiteral( "geography::STGeomFromWKB(?,%1)" ).arg( mSRId );
        else
          values += QStringLiteral( "geography::STGeomFromText(?,%1)" ).arg( mSRId );
      }
    }

    statement += QLatin1String( ") " );
    if ( !( flags & QgsFeatureSink::FastInsert ) && !mPrimaryKeyAttrs.isEmpty() )
    {
      statement += QLatin1String( " OUTPUT " );

      QString delim;

      for ( const auto idx : mPrimaryKeyAttrs )
      {
        const QgsField &fld = mAttributeFields.at( idx );
        statement += delim + QStringLiteral( "inserted.[%1]" ).arg( fld.name() );
        delim = QStringLiteral( "," );
      }

      statement += QLatin1String( " INTO @px " );
    }

    statement += QStringLiteral( " VALUES (" ) + values + ')';

    if ( !( flags & QgsFeatureSink::FastInsert && !mPrimaryKeyAttrs.isEmpty() ) )
    {
      statement += QLatin1String( "; SELECT * FROM @px;" );
    }

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      const QString msg = query.lastError().text();
      QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
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

      const QgsField &fld = mAttributeFields.at( i );

      if ( fld.typeName().compare( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) == 0 )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mDefaultValues.contains( i ) && mDefaultValues.value( i ) == attrs.at( i ).toString() )
        continue; // skip fields having default values

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

      const QVariant::Type type = fld.type();
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
      if ( QgsWkbTypes::isMultiType( mWkbType ) && !geom.isMultipart() )
      {
        geom.convertToMultiType();
      }
      if ( mUseWkb )
      {
        const QByteArray bytea = geom.asWkb();
        query.addBindValue( bytea, QSql::In | QSql::Binary );
      }
      else
      {
        QString wkt;
        if ( !geom.isNull() )
        {
          // Z and M on the end of a WKT string isn't valid for
          // SQL Server so we have to remove it first.
          wkt = geom.asWkt();
          wkt.replace( QRegularExpression( QStringLiteral( "[mzMZ]+\\s*\\(" ) ), QStringLiteral( "(" ) );
          // if we have M value only, we need to insert null-s for the Z value
          if ( QgsWkbTypes::hasM( geom.wkbType() ) && !QgsWkbTypes::hasZ( geom.wkbType() ) )
          {
            wkt.replace( QRegularExpression( QStringLiteral( "(?=\\s[0-9+-.]+[,)])" ) ), QStringLiteral( " NULL" ) );
          }
        }
        query.addBindValue( wkt );
      }
    }

    if ( !query.exec() )
    {
      const QString msg = query.lastError().text();
      QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
      if ( !mSkipFailures )
      {
        pushError( msg );
        return false;
      }
    }

    if ( !( flags & QgsFeatureSink::FastInsert ) && !mPrimaryKeyAttrs.isEmpty() )
    {
      if ( !query.next() )
      {
        const QString msg = query.lastError().text();
        QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
        if ( !mSkipFailures )
        {
          pushError( msg );
          return false;
        }
      }

      if ( mPrimaryKeyType == PktInt )
      {
        it->setId( query.value( 0 ).toLongLong() );
      }
      else
      {
        QVariantList keyvals;
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); ++i )
        {
          keyvals << query.value( i );
        }

        it->setId( mShared->lookupFid( keyvals ) );
      }
    }
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

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

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );
  if ( !LoggedExec( query, statement ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
    return false;
  }

  loadFields();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

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

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  if ( !LoggedExec( query, statement ) )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
    return false;
  }

  query.finish();

  loadFields();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}


bool QgsMssqlProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( attr_map.isEmpty() )
    return true;

  if ( mPrimaryKeyAttrs.isEmpty() )
    return false;

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    const QgsFeatureId fid = it.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    const QgsAttributeMap &attrs = it.value();
    if ( attrs.isEmpty() )
      continue;

    QString statement = QStringLiteral( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    bool first = true;
    bool pkChanged = false;
    QSqlQuery query = createQuery();
    query.setForwardOnly( true );

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      const QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.typeName().compare( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) == 0 )
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

      pkChanged = pkChanged || mPrimaryKeyAttrs.contains( it2.key() );

      statement += QStringLiteral( "[%1]=?" ).arg( fld.name() );
    }

    if ( first )
      return true; // no fields have been changed

    // set attribute filter
    statement += QStringLiteral( " WHERE " ) + whereClauseFid( fid );

    // use prepared statement to prevent from sql injection
    if ( !query.prepare( statement ) )
    {
      QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
      return false;
    }

    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
    {
      const QgsField fld = mAttributeFields.at( it2.key() );

      if ( fld.typeName().compare( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) == 0 )
        continue; // You can't update timestamp columns they are server only.

      if ( fld.typeName().endsWith( QLatin1String( " identity" ), Qt::CaseInsensitive ) )
        continue; // skip identity field

      if ( fld.name().isEmpty() )
        continue; // invalid

      if ( mComputedColumns.contains( fld.name() ) )
        continue; // skip computed columns because they are done server side.

      const QVariant::Type type = fld.type();
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
      QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
      return false;
    }

    if ( pkChanged && mPrimaryKeyType == PktFidMap )
    {
      const QVariant v = mShared->removeFid( fid );
      QVariantList k = v.toList();

      for ( int i = 0; i < mPrimaryKeyAttrs.size(); ++i )
      {
        const int idx = mPrimaryKeyAttrs[i];
        if ( !attrs.contains( idx ) )
          continue;

        k[i] = attrs[ idx ];
      }

      mShared->insertFid( fid, k );
    }
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsMssqlProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( geometry_map.isEmpty() )
    return true;

  if ( mPrimaryKeyAttrs.isEmpty() )
    return false;

  for ( QgsGeometryMap::const_iterator it = geometry_map.constBegin(); it != geometry_map.constEnd(); ++it )
  {
    const QgsFeatureId fid = it.key();
    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    QString statement;
    statement = QStringLiteral( "UPDATE [%1].[%2] SET " ).arg( mSchemaName, mTableName );

    QSqlQuery query = createQuery();
    query.setForwardOnly( true );

    if ( mGeometryColType == QLatin1String( "geometry" ) )
    {
      if ( mUseWkb )
        statement += QStringLiteral( "[%1]=geometry::STGeomFromWKB(?,%2).MakeValid()" ).arg( mGeometryColName ).arg( mSRId );
      else
        statement += QStringLiteral( "[%1]=geometry::STGeomFromText(?,%2).MakeValid()" ).arg( mGeometryColName ).arg( mSRId );
    }
    else
    {
      if ( mUseWkb )
        statement += QStringLiteral( "[%1]=geography::STGeomFromWKB(?,%2)" ).arg( mGeometryColName ).arg( mSRId );
      else
        statement += QStringLiteral( "[%1]=geography::STGeomFromText(?,%2)" ).arg( mGeometryColName ).arg( mSRId );
    }

    // set attribute filter
    statement += QStringLiteral( " WHERE " ) + whereClauseFid( fid );

    if ( !query.prepare( statement ) )
    {
      pushError( query.lastError().text() );
      return false;
    }

    // add geometry param
    if ( mUseWkb )
    {
      const QByteArray bytea = it->asWkb();
      query.addBindValue( bytea, QSql::In | QSql::Binary );
    }
    else
    {
      QString wkt = it->asWkt();
      // Z and M on the end of a WKT string isn't valid for
      // SQL Server so we have to remove it first.
      wkt.replace( QRegularExpression( QStringLiteral( "[mzMZ]+\\s*\\(" ) ), QStringLiteral( "(" ) );
      query.addBindValue( wkt );
    }

    if ( !query.exec() )
    {
      pushError( query.lastError().text() );
      return false;
    }
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsMssqlProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( mPrimaryKeyAttrs.isEmpty() )
    return false;

  if ( ids.empty() )
    return true; // for consistency providers return true to an empty list

  if ( mPrimaryKeyType == PktInt )
  {
    QString featureIds, delim;
    for ( QgsFeatureIds::const_iterator it = ids.begin(); it != ids.end(); ++it )
    {
      featureIds += delim + FID_TO_STRING( *it );
      delim = QStringLiteral( "," );
    }

    QSqlQuery query = createQuery();
    query.setForwardOnly( true );

    const QString statement = QStringLiteral( "DELETE FROM [%1].[%2] WHERE [%3] IN (%4)" ).arg( mSchemaName, mTableName, mAttributeFields.at( mPrimaryKeyAttrs[0] ).name(), featureIds );

    if ( LoggedExec( query, statement ) )
    {
      if ( query.numRowsAffected() == ids.size() )
      {
        if ( mTransaction )
          mTransaction->dirtyLastSavePoint();
        return true;
      }

      pushError( tr( "Only %1 of %2 features deleted" ).arg( query.numRowsAffected() ).arg( ids.size() ) );
    }
    else
    {
      pushError( query.lastError().text() );
    }
  }
  else if ( mPrimaryKeyType == PktFidMap )
  {
    int i = 0;

    QSqlQuery query = createQuery();
    for ( QgsFeatureIds::const_iterator it = ids.begin(); it != ids.end(); ++it )
    {
      const QString statement = QStringLiteral( "DELETE FROM [%1].[%2] WHERE %3" ).arg( mSchemaName, mTableName, whereClauseFid( *it ) );

      if ( LoggedExec( query, statement ) )
      {
        if ( query.numRowsAffected() == 1 )
        {
          mShared->removeFid( *it );
          i++;
        }
      }
      else
      {
        pushError( query.lastError().text() );
        break;
      }
    }

    if ( i == ids.size() )
    {
      if ( mTransaction )
        mTransaction->dirtyLastSavePoint();
      return true;
    }

    if ( i > 0 )
      pushError( tr( "Only %1 of %2 features deleted" ).arg( i ).arg( ids.size() ) );
  }

  return false;
}

void QgsMssqlProvider::updateExtents()
{
  mExtent.setMinimal();
}

QgsVectorDataProvider::Capabilities QgsMssqlProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities cap = CreateAttributeIndex | AddFeatures | AddAttributes | TransactionSupport;
  bool hasGeom = false;
  if ( !mGeometryColName.isEmpty() )
  {
    hasGeom = true;
    cap |= CreateSpatialIndex;
  }

  if ( mPrimaryKeyAttrs.isEmpty() )
    return cap;

  if ( hasGeom )
    cap |= ChangeGeometries;

  return cap | DeleteFeatures | ChangeAttributeValues | DeleteAttributes |
         QgsVectorDataProvider::SelectAtId;
}

bool QgsMssqlProvider::createSpatialIndex()
{
  if ( mUseEstimatedMetadata )
    UpdateStatistics( false );

  QSqlQuery query = createQuery();
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

  if ( !LoggedExec( query, statement ) )
  {
    pushError( query.lastError().text() );
    return false;
  }

  return true;
}

bool QgsMssqlProvider::createAttributeIndex( int field )
{
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );
  QString statement;

  if ( field < 0 || field >= mAttributeFields.size() )
  {
    pushError( QStringLiteral( "createAttributeIndex invalid index" ) );
    return false;
  }

  statement = QStringLiteral( "CREATE NONCLUSTERED INDEX [qgs_%1_idx] ON [%2].[%3] ( [%4] )" ).arg(
                mGeometryColName, mSchemaName, mTableName, mAttributeFields.at( field ).name() );

  if ( !LoggedExec( query, statement ) )
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
    // try to load crs from the database tables as a fallback
    QSqlQuery query = createQuery();
    query.setForwardOnly( true );
    const QString statement { QStringLiteral( "SELECT srtext FROM spatial_ref_sys WHERE srid=%1" ).arg( mSRId ) };

    bool execOk = LoggedExec( query, statement );
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
    query.setForwardOnly( true );

    // Look in the system reference table for the data if we can't find it yet
    execOk = LoggedExec( query, QStringLiteral( "SELECT well_known_text FROM sys.spatial_reference_systems WHERE spatial_reference_id=%1" ).arg( mSRId ) );
    if ( execOk && query.isActive() && query.next() )
    {
      mCrs = QgsCoordinateReferenceSystem::fromWkt( query.value( 0 ).toString() );
      if ( mCrs.isValid() )
        return mCrs;
    }
  }
  return mCrs;
}


void QgsMssqlProvider::setTransaction( QgsTransaction *transaction )
{
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsMssqlTransaction *>( transaction );
}

QgsTransaction *QgsMssqlProvider::transaction() const
{
  return mTransaction;
}

std::shared_ptr<QgsMssqlDatabase> QgsMssqlProvider::connection() const
{
  return mTransaction ? mTransaction->conn() : QgsMssqlDatabase::connectDb( uri().connectionInfo(), false );
}

void QgsMssqlProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mShared = qobject_cast<QgsMssqlProvider *>( source )->mShared;
}

QString QgsMssqlProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString  QgsMssqlProvider::name() const
{
  return MSSQL_PROVIDER_KEY;
}

bool QgsMssqlProvider::setSubsetString( const QString &theSQL, bool )
{
  if ( theSQL.trimmed() == mSqlWhereClause )
    return true;

  const QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QStringLiteral( "SELECT count(*) FROM " );

  sql += QStringLiteral( "[%1].[%2]" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
  }

  QSqlQuery query = createQuery();
  query.setForwardOnly( true );
  if ( !LoggedExec( query, sql ) )
  {
    pushError( query.lastError().text() );
    mSqlWhereClause = prevWhere;
    return false;
  }

  if ( query.isActive() && query.next() )
    mNumberFeatures = query.value( 0 ).toLongLong();

  QgsDataSourceUri anUri = QgsDataSourceUri( dataSourceUri() );
  anUri.setSql( mSqlWhereClause );

  setDataSourceUri( anUri.uri() );

  mExtent.setMinimal();

  emit dataChanged();

  return true;
}

QString  QgsMssqlProvider::description() const
{
  return MSSQL_PROVIDER_DESCRIPTION;
}

QgsAttributeList QgsMssqlProvider::pkAttributeIndexes() const
{
  return mPrimaryKeyAttrs;
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
      fieldPrec = 0;
      break;

    case QVariant::Date:
      fieldType = QStringLiteral( "date" );
      fieldPrec = 0;
      break;

    case QVariant::Time:
      fieldType = QStringLiteral( "time" );
      fieldPrec = 0;
      break;

    case QVariant::String:
      fieldType = QStringLiteral( "nvarchar(max)" );
      fieldPrec = 0;
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
        fieldPrec = 0;
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
  const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( wkbType );

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
  else if ( flatType == QgsWkbTypes::GeometryCollection )
    geometryType = QStringLiteral( "GEOMETRYCOLLECTION" );
  else if ( flatType == QgsWkbTypes::CircularString )
    geometryType = QStringLiteral( "CIRCULARSTRING" );
  else if ( flatType == QgsWkbTypes::CompoundCurve )
    geometryType = QStringLiteral( "COMPOUNDCURVE" );
  else if ( flatType == QgsWkbTypes::CurvePolygon )
    geometryType = QStringLiteral( "CURVEPOLYGON" );
  else if ( flatType == QgsWkbTypes::Unknown )
    geometryType = QStringLiteral( "GEOMETRY" );
  else
  {
    dim = 0;
    return;
  }

  if ( QgsWkbTypes::hasZ( wkbType ) && QgsWkbTypes::hasM( wkbType ) )
  {
    dim = 4;
  }
  else if ( QgsWkbTypes::hasZ( wkbType ) )
  {
    dim = 3;
  }
  else if ( QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1Char( 'M' );
    dim = 3;
  }
  else if ( wkbType >= QgsWkbTypes::Point25D && wkbType <= QgsWkbTypes::MultiPolygon25D )
  {
    dim = 3;
  }
}

QgsWkbTypes::Type QgsMssqlProvider::getWkbType( const QString &geometryType )
{
  return QgsWkbTypes::parseType( geometryType );
}


Qgis::VectorExportResult QgsMssqlProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options )

  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return Qgis::VectorExportResult::ErrorConnectionFailed;
  }

  const QString dbName = dsUri.database();

  QString schemaName = dsUri.schema();
  const QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  if ( schemaName.isEmpty() )
    schemaName = QStringLiteral( "dbo" );

  if ( wkbType != QgsWkbTypes::NoGeometry && geometryColumn.isEmpty() )
    geometryColumn = QStringLiteral( "geom" );

  // get the pk's name and type
  bool createdNewPk = false;

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    const QString pk = primaryKey = QStringLiteral( "qgs_fid" );
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      if ( fields.at( i ).name() == primaryKey )
      {
        // it already exists, try again with a new name
        primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
        i = 0;
      }
    }
    createdNewPk = true;
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
        if ( ( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) || convertField( fld ) )
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
  QSqlQuery q = QSqlQuery( db->db() );
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

  std::unique_ptr <QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), QGS_QUERY_LOG_ORIGIN );

  if ( !q.exec( sql ) )
  {
    logWrapper->setError( q.lastError().text() );
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  // set up spatial reference id
  long srid = 0;
  if ( srs.isValid() )
  {
    srid = srs.postgisSrid();
    QString auth_srid = QStringLiteral( "null" );
    QString auth_name = QStringLiteral( "null" );
    QStringList sl = srs.authid().split( ':' );
    if ( sl.length() == 2 )
    {
      auth_name = sl[0];
      auth_srid = sl[1];
    }
    sql = QStringLiteral( "IF NOT EXISTS (SELECT * FROM spatial_ref_sys WHERE srid=%1) INSERT INTO spatial_ref_sys (srid, auth_name, auth_srid, srtext, proj4text) VALUES (%1, %2, %3, %4, %5)" )
          .arg( srid )
          .arg( quotedValue( auth_name ),
                auth_srid,
                quotedValue( srs.toWkt() ),
                quotedValue( srs.toProj() ) );

    logWrapper.reset( new QgsDatabaseQueryLogWrapper( sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), QGS_QUERY_LOG_ORIGIN ) );

    if ( !q.exec( sql ) )
    {
      logWrapper->setError( q.lastError().text() );
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }
  }

  // get wkb type and dimension
  QString geometryType;
  int dim = 2;
  mssqlWkbTypeAndDimension( wkbType, geometryType, dim );

  if ( overwrite )
  {
    // remove the old table with the same name
    sql = QStringLiteral( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) BEGIN DROP TABLE [%1].[%2] DELETE FROM geometry_columns where f_table_schema='%1' and f_table_name='%2' END;" )
          .arg( schemaName, tableName );

    logWrapper.reset( new QgsDatabaseQueryLogWrapper( sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), QGS_QUERY_LOG_ORIGIN ) );

    if ( !q.exec( sql ) )
    {
      logWrapper->setError( q.lastError().text() );
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }
  }
  else
  {
    // test for existing
    sql = QStringLiteral( "SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')" )
          .arg( schemaName, tableName );

    logWrapper.reset( new QgsDatabaseQueryLogWrapper( sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), QGS_QUERY_LOG_ORIGIN ) );

    if ( !q.exec( sql ) )
    {
      logWrapper->setError( q.lastError().text() );
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }

    // if we got a hit, abort!!
    if ( q.next() )
    {
      if ( errorMessage )
        *errorMessage = tr( "Table [%1].[%2] already exists" ).arg( schemaName, tableName );
      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }
  }

  if ( !geometryColumn.isEmpty() )
  {
    sql = QStringLiteral( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
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
    sql = QStringLiteral( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                          "CREATE TABLE [%1].[%2]([%3] [int] IDENTITY(1,1) NOT NULL CONSTRAINT [PK_%2] PRIMARY KEY CLUSTERED ( [%3] ASC ))\n"
                          "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'\n"
                        )
          .arg( schemaName,
                tableName,
                primaryKey );
  }

  logWrapper.reset( new QgsDatabaseQueryLogWrapper( sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), QGS_QUERY_LOG_ORIGIN ) );

  if ( !q.exec( sql ) )
  {
    logWrapper->setError( q.lastError().text() );
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  // clear any resources hold by the query
  q.clear();
  q.setForwardOnly( true );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );

  const QgsDataProvider::ProviderOptions providerOptions;
  const QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  QgsMssqlProvider *provider = new QgsMssqlProvider( dsUri.uri(), providerOptions, flags );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the MSSQL provider failed" );

    delete provider;
    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    // if we had to create a primary key column, we start the old columns from 1
    int offset = createdNewPk ? 1 : 0;

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

      if ( !( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) && !convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        delete provider;
        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
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
      return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
    }
  }
  return Qgis::VectorExportResult::Success;
}



/**
 * Class factory to return a pointer to a newly created
 * QgsMssqlProvider object
 */
QgsMssqlProvider *QgsMssqlProviderMetadata::createProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
{
  return new QgsMssqlProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsMssqlProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsMssqlDataItemProvider;
  return providers;
}

QgsTransaction *QgsMssqlProviderMetadata::createTransaction( const QString &connString )
{
  return new QgsMssqlTransaction( connString );
}

QMap<QString, QgsAbstractProviderConnection *> QgsMssqlProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsMssqlProviderConnection, QgsMssqlConnection>( cached );
}

QgsAbstractProviderConnection *QgsMssqlProviderMetadata::createConnection( const QString &name )
{
  return new QgsMssqlProviderConnection( name );
}

QgsAbstractProviderConnection *QgsMssqlProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  return new QgsMssqlProviderConnection( uri, configuration );
}

void QgsMssqlProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsMssqlProviderConnection>( name );
}

void QgsMssqlProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn, const QString &name )
{
  saveConnectionProtected( conn, name );
}

Qgis::VectorExportResult QgsMssqlProviderMetadata::createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> &oldToNewAttrIdxMap,
  QString &errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsMssqlProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, &errorMessage, options
         );
}

bool QgsMssqlProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();
  const QgsDataSourceUri dsUri( uri );
  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    errorCause = QObject::tr( "Error connecting to database: %1" ).arg( db->errorText() );
    return false;
  }

  QSqlQuery query = QSqlQuery( db->db() );
  query.setForwardOnly( true );
  const QString sql { QStringLiteral( "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = N'layer_styles'" ) };

  if ( !LoggedExecMetadata( query, sql, uri ) )
  {
    errorCause = QObject::tr( "Could not check if layer_styles table exists: %1" ).arg( query.lastError().text() );
    return false;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    // no layer_styles table
    query.finish();
    return false;
  }

  query.clear();
  query.setForwardOnly( true );
  const QString checkQuery = QString( "SELECT styleName"
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
                             .arg( QgsMssqlProvider::quotedValue( styleId.isEmpty() ? dsUri.table() : styleId ) );

  if ( !LoggedExecMetadata( query, checkQuery, uri ) )
  {
    errorCause = QObject::tr( "Checking for style failed: %1" ).arg( query.lastError().text() );
    return false;
  }

  if ( query.isActive() && query.next() && query.value( 0 ).toString() == styleId )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsMssqlProviderMetadata::saveStyle( const QString &uri,
    const QString &qmlStyle,
    const QString &sldStyle,
    const QString &styleName,
    const QString &styleDescription,
    const QString &uiFileContent,
    bool useAsDefault,
    QString &errCause )
{
  const QgsDataSourceUri dsUri( uri );
  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Error connecting to database" ) );
    QgsDebugMsg( db->errorText() );
    return false;
  }

  QSqlQuery query = QSqlQuery( db->db() );
  query.setForwardOnly( true );

  QString sql { QStringLiteral( "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME= N'layer_styles'" ) };

  if ( !LoggedExecMetadata( query, sql, uri ) )
  {
    QgsDebugMsg( query.lastError().text() );
    return false;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Need to create styles table" ), 2 );

    sql = QStringLiteral( "CREATE TABLE [dbo].[layer_styles]("
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
                          ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]" );

    const bool execOk = LoggedExecMetadata( query, sql, uri );
    if ( !execOk )
    {
      const QString error { QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database. Maybe this is due to table permissions. Please contact your database admin" ) };
      errCause = error;
      return false;
    }
    query.finish();
    query.clear();
    query.setForwardOnly( true );
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QStringLiteral( ",XMLPARSE(DOCUMENT %1)" ).arg( uiFileContent );
  }
  QgsDebugMsgLevel( QStringLiteral( "Ready to insert new style" ), 2 );
  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and sldStyle values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  sql = QStringLiteral( "INSERT INTO layer_styles"
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
        .arg( useAsDefault ? QStringLiteral( "1" ) : QStringLiteral( "0" ) )
        .arg( QgsMssqlProvider::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
        .arg( QgsMssqlProvider::quotedValue( dsUri.username() ) )
        .arg( uiFileColumn )
        .arg( uiFileValue );

  const QString checkQuery = QStringLiteral( "SELECT styleName"
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

  if ( !LoggedExecMetadata( query, checkQuery, uri ) )
  {
    QgsDebugMsg( query.lastError().text() );
    QgsDebugMsg( QStringLiteral( "Check Query failed" ) );
    return false;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toString() == styleName )
  {
    QgsDebugMsgLevel( QStringLiteral( "Updating styles" ), 2 );
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
          .arg( useAsDefault ? QStringLiteral( "1" ) : QStringLiteral( "0" ) )
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
    const QString removeDefaultSql = QString( "UPDATE layer_styles "
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

  QgsDebugMsgLevel( QStringLiteral( "Inserting styles" ), 2 );
  QgsDebugMsgLevel( sql, 2 );

  const bool execOk = LoggedExecMetadata( query, sql, uri );

  if ( !execOk )
  {
    errCause = QObject::tr( "Unable to save layer style. It's not possible to insert a new record into the style table. Maybe this is due to table permissions. Please contact your database administrator." );
  }
  return execOk;
}

QString QgsMssqlProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  errCause.clear();
  const QgsDataSourceUri dsUri( uri );
  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Error connecting to database" ) );
    QgsDebugMsg( db->errorText() );
    errCause = tr( "Cannot connect to database: %1" ).arg( db->errorText() );
    return QString();
  }

  QSqlQuery query = QSqlQuery( db->db() );

  query.setForwardOnly( true );

  const QString sql { QStringLiteral( "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME= N'layer_styles'" ) };

  if ( !LoggedExecMetadata( query, sql, uri ) )
  {
    errCause = tr( "Could not check if layer_styles table exists: %1" ).arg( query.lastError().text() );
    return QString();
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    // no layer_styles table
    errCause = tr( "Style does not exist" );
    query.finish();
    return QString();
  }

  query.clear();
  query.setForwardOnly( true );

  const QString selectQmlQuery = QString( "SELECT top 1 styleQML"
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

  if ( !LoggedExecMetadata( query, selectQmlQuery, uri ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Load of style failed" ), 2 );
    const QString msg = query.lastError().text();
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

int QgsMssqlProviderMetadata::listStyles( const QString &uri,
    QStringList &ids,
    QStringList &names,
    QStringList &descriptions,
    QString &errCause )
{
  const QgsDataSourceUri dsUri( uri );
  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Error connecting to database" ) );
    QgsDebugMsg( db->errorText() );
    return -1;
  }

  QSqlQuery query = QSqlQuery( db->db() );
  query.setForwardOnly( true );

  QString sql  { QStringLiteral( "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME= N'layer_styles'" ) };

  // check if layer_styles table already exist
  if ( !LoggedExecMetadata( query, sql, uri ) )
  {
    const QString msg = query.lastError().text();
    errCause = msg;
    QgsDebugMsg( msg );
    return -1;
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsgLevel( QObject::tr( "No styles available on DB" ), 2 );
    return -1;
  }

  const QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                     " FROM layer_styles "
                                     " WHERE f_table_catalog=%1"
                                     " AND f_table_schema=%2"
                                     " AND f_table_name=%3"
                                     " AND f_geometry_column=%4"
                                     " ORDER BY useasdefault DESC, update_time DESC" )
                                     .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                                     .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                                     .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                                     .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );


  bool queryOk = LoggedExecMetadata( query, selectRelatedQuery, uri );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    return -1;
  }
  int numberOfRelatedStyles = 0;
  while ( query.isActive() && query.next() )
  {
    QgsDebugMsgLevel( query.value( 1 ).toString(), 2 );
    ids.append( query.value( 0 ).toString() );
    names.append( query.value( 1 ).toString() );
    descriptions.append( query.value( 2 ).toString() );
    numberOfRelatedStyles = numberOfRelatedStyles + 1;
  }
  const QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                    " FROM layer_styles "
                                    " WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4)"
                                    " ORDER BY update_time DESC" )
                                    .arg( QgsMssqlProvider::quotedValue( dsUri.database() ) )
                                    .arg( QgsMssqlProvider::quotedValue( dsUri.schema() ) )
                                    .arg( QgsMssqlProvider::quotedValue( dsUri.table() ) )
                                    .arg( QgsMssqlProvider::quotedValue( dsUri.geometryColumn() ) );
  QgsDebugMsgLevel( selectOthersQuery, 2 );

  queryOk = LoggedExecMetadata( query, selectOthersQuery, uri );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    return -1;
  }
  while ( query.next() )
  {
    ids.append( query.value( 0 ).toString() );
    names.append( query.value( 1 ).toString() );
    descriptions.append( query.value( 2 ).toString() );
  }
  return numberOfRelatedStyles;
}

QgsMssqlProviderMetadata::QgsMssqlProviderMetadata():
  QgsProviderMetadata( QgsMssqlProvider::MSSQL_PROVIDER_KEY, QgsMssqlProvider::MSSQL_PROVIDER_DESCRIPTION )
{
}

QString QgsMssqlProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  const QgsDataSourceUri dsUri( uri );
  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Error connecting to database" ) );
    QgsDebugMsg( db->errorText() );
    errCause = tr( "Cannot connect to database: %1" ).arg( db->errorText() );
    return QString();
  }

  QSqlQuery query = QSqlQuery( db->db() );
  query.setForwardOnly( true );

  const QString sql {  QStringLiteral( "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME= N'layer_styles'" ) };

  if ( !LoggedExecMetadata( query, sql, uri ) )
  {
    errCause = tr( "Could not check if layer_styles table exists: %1" ).arg( query.lastError().text() );
    return QString();
  }
  if ( query.isActive() && query.next() && query.value( 0 ).toInt() == 0 )
  {
    // no layer_styles table
    errCause = tr( "Style does not exist" );
    query.finish();
    return QString();
  }

  query.clear();
  query.setForwardOnly( true );

  QString style;
  const QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsMssqlProvider::quotedValue( styleId ) );

  const bool queryOk = LoggedExecMetadata( query, selectQmlQuery, uri );
  if ( !queryOk )
  {
    QgsDebugMsg( query.lastError().text() );
    errCause = query.lastError().text();
    return QString();
  }
  if ( !query.next() )
  {
    errCause = tr( "Style does not exist" );
  }
  else
  {
    style = query.value( 0 ).toString();
  }
  return style;
}

QVariantMap QgsMssqlProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap uriParts;

  if ( ! dsUri.database().isEmpty() )
    uriParts[ QStringLiteral( "dbname" ) ] = dsUri.database();
  if ( ! dsUri.host().isEmpty() )
    uriParts[ QStringLiteral( "host" ) ] = dsUri.host();
  if ( ! dsUri.port().isEmpty() )
    uriParts[ QStringLiteral( "port" ) ] = dsUri.port();
  if ( ! dsUri.service().isEmpty() )
    uriParts[ QStringLiteral( "service" ) ] = dsUri.service();
  if ( ! dsUri.username().isEmpty() )
    uriParts[ QStringLiteral( "username" ) ] = dsUri.username();
  if ( ! dsUri.password().isEmpty() )
    uriParts[ QStringLiteral( "password" ) ] = dsUri.password();

  // Supported?
  //if ( ! dsUri.authConfigId().isEmpty() )
  //  uriParts[ QStringLiteral( "authcfg" ) ] = dsUri.authConfigId();

  if ( dsUri.wkbType() != QgsWkbTypes::Type::Unknown )
    uriParts[ QStringLiteral( "type" ) ] = dsUri.wkbType();

  // Supported?
  // uriParts[ QStringLiteral( "selectatid" ) ] = dsUri.selectAtIdDisabled();

  if ( ! dsUri.table().isEmpty() )
    uriParts[ QStringLiteral( "table" ) ] = dsUri.table();
  if ( ! dsUri.schema().isEmpty() )
    uriParts[ QStringLiteral( "schema" ) ] = dsUri.schema();
  if ( ! dsUri.keyColumn().isEmpty() )
    uriParts[ QStringLiteral( "key" ) ] = dsUri.keyColumn();
  if ( ! dsUri.srid().isEmpty() )
    uriParts[ QStringLiteral( "srid" ) ] = dsUri.srid();

  uriParts[ QStringLiteral( "estimatedmetadata" ) ] = dsUri.useEstimatedMetadata();

  // is this supported?
  // uriParts[ QStringLiteral( "sslmode" ) ] = dsUri.sslMode();

  if ( ! dsUri.sql().isEmpty() )
    uriParts[ QStringLiteral( "sql" ) ] = dsUri.sql();
  if ( ! dsUri.geometryColumn().isEmpty() )
    uriParts[ QStringLiteral( "geometrycolumn" ) ] = dsUri.geometryColumn();

  // From configuration
  static const QStringList configurationParameters
  {
    QStringLiteral( "geometryColumnsOnly" ),
    QStringLiteral( "allowGeometrylessTables" ),
    QStringLiteral( "saveUsername" ),
    QStringLiteral( "savePassword" ),
    QStringLiteral( "estimatedMetadata" ),
    QStringLiteral( "disableInvalidGeometryHandling" ),
    QStringLiteral( "extentInGeometryColumns" ),
    QStringLiteral( "primaryKeyInGeometryColumns" )
  };

  for ( const auto &configParam : configurationParameters )
  {
    if ( dsUri.hasParam( configParam ) )
    {
      uriParts[ configParam ] = dsUri.param( configParam );
    }
  }

  return uriParts;
}

QString QgsMssqlProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( QStringLiteral( "dbname" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "dbname" ) ).toString() );
  // Also accepts "database"
  if ( parts.contains( QStringLiteral( "database" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "database" ) ).toString() );
  // Supported?
  //if ( parts.contains( QStringLiteral( "port" ) ) )
  //  dsUri.setParam( QStringLiteral( "port" ), parts.value( QStringLiteral( "port" ) ).toString() );
  if ( parts.contains( QStringLiteral( "host" ) ) )
    dsUri.setParam( QStringLiteral( "host" ), parts.value( QStringLiteral( "host" ) ).toString() );
  if ( parts.contains( QStringLiteral( "service" ) ) )
    dsUri.setParam( QStringLiteral( "service" ), parts.value( QStringLiteral( "service" ) ).toString() );
  if ( parts.contains( QStringLiteral( "username" ) ) )
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  if ( parts.contains( QStringLiteral( "password" ) ) )
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  // Supported?
  //if ( parts.contains( QStringLiteral( "authcfg" ) ) )
  //  dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  if ( parts.contains( QStringLiteral( "type" ) ) )
    dsUri.setParam( QStringLiteral( "type" ), QgsWkbTypes::displayString( static_cast<QgsWkbTypes::Type>( parts.value( QStringLiteral( "type" ) ).toInt() ) ) );
  // Supported?
  //if ( parts.contains( QStringLiteral( "selectatid" ) ) )
  //  dsUri.setParam( QStringLiteral( "selectatid" ), parts.value( QStringLiteral( "selectatid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "table" ) ) )
    dsUri.setTable( parts.value( QStringLiteral( "table" ) ).toString() );
  if ( parts.contains( QStringLiteral( "schema" ) ) )
    dsUri.setSchema( parts.value( QStringLiteral( "schema" ) ).toString() );
  if ( parts.contains( QStringLiteral( "key" ) ) )
    dsUri.setParam( QStringLiteral( "key" ), parts.value( QStringLiteral( "key" ) ).toString() );
  if ( parts.contains( QStringLiteral( "srid" ) ) )
    dsUri.setSrid( parts.value( QStringLiteral( "srid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "estimatedmetadata" ) ) )
    dsUri.setParam( QStringLiteral( "estimatedmetadata" ), parts.value( QStringLiteral( "estimatedmetadata" ) ).toString() );
  // Supported?
  //if ( parts.contains( QStringLiteral( "sslmode" ) ) )
  //  dsUri.setParam( QStringLiteral( "sslmode" ), QgsDataSourceUri::encodeSslMode( static_cast<QgsDataSourceUri::SslMode>( parts.value( QStringLiteral( "sslmode" ) ).toInt( ) ) ) );
  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  // Supported?
  //if ( parts.contains( QStringLiteral( "checkPrimaryKeyUnicity" ) ) )
  //  dsUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), parts.value( QStringLiteral( "checkPrimaryKeyUnicity" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  if ( parts.contains( QStringLiteral( "disableInvalidGeometryHandling" ) ) )
    dsUri.setParam( QStringLiteral( "disableInvalidGeometryHandling" ), parts.value( QStringLiteral( "disableInvalidGeometryHandling" ) ).toString() );
  if ( parts.contains( QStringLiteral( "allowGeometrylessTables" ) ) )
    dsUri.setParam( QStringLiteral( "allowGeometrylessTables" ), parts.value( QStringLiteral( "allowGeometrylessTables" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometryColumnsOnly" ) ) )
    dsUri.setParam( QStringLiteral( "geometryColumnsOnly" ), parts.value( QStringLiteral( "geometryColumnsOnly" ) ).toString() );
  if ( parts.contains( QStringLiteral( "extentInGeometryColumns" ) ) )
    dsUri.setParam( QStringLiteral( "extentInGeometryColumns" ), parts.value( QStringLiteral( "extentInGeometryColumns" ) ).toString() );
  if ( parts.contains( QStringLiteral( "primaryKeyInGeometryColumns" ) ) )
    dsUri.setParam( QStringLiteral( "primaryKeyInGeometryColumns" ), parts.value( QStringLiteral( "primaryKeyInGeometryColumns" ) ).toString() );
  return dsUri.uri();
}

bool QgsMssqlProviderMetadata::execLogged( QSqlQuery &qry, const QString &sql, const QString &uri, const QString &queryOrigin ) const
{
  QgsDatabaseQueryLogWrapper logWrapper{ sql, uri, QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProviderMetadata" ),  queryOrigin };
  const bool res { qry.exec( sql ) };
  if ( ! res )
  {
    logWrapper.setError( qry.lastError().text() );
  }
  else
  {
    if ( qry.isSelect() )
    {
      logWrapper.setFetchedRows( qry.size() );
    }
    else
    {
      logWrapper.setFetchedRows( qry.numRowsAffected() );
    }
  }
  logWrapper.setQuery( qry.lastQuery() );
  return res;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsMssqlProviderMetadata();
}

// ----------

QgsFeatureId QgsMssqlSharedData::lookupFid( const QVariantList &v )
{
  const QMutexLocker locker( &mMutex );

  const QMap<QVariantList, QgsFeatureId>::const_iterator it = mKeyToFid.constFind( v );

  if ( it != mKeyToFid.constEnd() )
  {
    return it.value();
  }

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}

QVariant QgsMssqlSharedData::removeFid( QgsFeatureId fid )
{
  const QMutexLocker locker( &mMutex );

  const QVariantList v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsMssqlSharedData::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  const QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsMssqlSharedData::lookupKey( QgsFeatureId featureId )
{
  const QMutexLocker locker( &mMutex );

  const QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.find( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

QString QgsMssqlProvider::whereClauseFid( QgsFeatureId featureId )
{
  QString whereClause;

  switch ( mPrimaryKeyType )
  {
    case PktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      whereClause = QStringLiteral( "[%1]=%2" ).arg( mAttributeFields.at( mPrimaryKeyAttrs[0] ).name(), FID_TO_STRING( featureId ) );
      break;

    case PktFidMap:
    {
      const QVariantList &pkVals = mShared->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        Q_ASSERT( pkVals.size() == mPrimaryKeyAttrs.size() );

        whereClause = QStringLiteral( "(" );

        QString delim;
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); ++i )
        {
          const QgsField &fld = mAttributeFields.at( mPrimaryKeyAttrs[i] );
          whereClause += QStringLiteral( "%1[%2]=%3" ).arg( delim, fld.name(), quotedValue( pkVals[i] ) );
          delim = QStringLiteral( " AND " );
        }

        whereClause += QLatin1Char( ')' );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = QStringLiteral( "NULL IS NOT NULL" );
      }
    }
    break;

    default:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = QStringLiteral( "NULL IS NOT NULL" );
      break;
  }

  return whereClause;
}

/* static */
QStringList QgsMssqlProvider::parseUriKey( const QString &key )
{
  if ( key.isEmpty() ) return QStringList();

  QStringList cols;

  // remove quotes from key list
  if ( key.startsWith( '"' ) && key.endsWith( '"' ) )
  {
    int i = 1;
    QString col;
    while ( i < key.size() )
    {
      if ( key[i] == '"' )
      {
        if ( i + 1 < key.size() && key[i + 1] == '"' )
        {
          i++;
        }
        else
        {
          cols << col;
          col.clear();

          if ( ++i == key.size() )
            break;

          Q_ASSERT( key[i] == ',' );
          i++;
          Q_ASSERT( key[i] == '"' );
          i++;
          col.clear();
          continue;
        }
      }

      col += key[i++];
    }
  }
  else if ( key.contains( ',' ) )
  {
    cols = key.split( ',' );
  }
  else
  {
    cols << key;
  }

  return cols;
}

bool QgsMssqlProvider::getExtentFromGeometryColumns( QgsRectangle &extent ) const
{
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  const QString sql = QStringLiteral( "SELECT qgis_xmin,qgis_xmax,qgis_ymin,qgis_ymax "
                                      "FROM geometry_columns WHERE f_table_name = %1 AND f_table_schema = %2 "
                                      "AND NOT (qgis_xmin IS NULL OR qgis_xmax IS NULL OR qgis_ymin IS NULL OR qgis_ymax IS NULL)" );

  const QString statement = sql.arg( quotedValue( mTableName ), quotedValue( mSchemaName ) );

  if ( LoggedExec( query, statement ) && query.isActive() )
  {
    query.next();
    if ( query.isValid() )
    {
      extent.setXMinimum( query.value( 0 ).toDouble() );
      extent.setXMaximum( query.value( 1 ).toDouble() );
      extent.setYMinimum( query.value( 2 ).toDouble() );
      extent.setYMaximum( query.value( 3 ).toDouble() );

      return true;
    }
  }

  return false;

}

bool QgsMssqlProvider::getPrimaryKeyFromGeometryColumns( QStringList &primaryKeys )
{
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );
  primaryKeys.clear();

  const QString sql = QStringLiteral( "SELECT qgis_pkey FROM geometry_columns WHERE f_table_name = '%1' AND NOT qgis_pkey IS NULL" );
  const QString statement = sql.arg( mTableName );

  if ( LoggedExec( query, statement ) && query.isActive() )
  {
    query.next();
    if ( query.isValid() )
    {
      primaryKeys = query.value( 0 ).toString().split( ',' );
      if ( !primaryKeys.isEmpty() )
        return true;
    }
  }

  return false;
}
