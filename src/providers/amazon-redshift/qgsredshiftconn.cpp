/***************************************************************************
   qgsredshiftconn.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftconn.h"

#include <QApplication>
#include <QStringList>
#include <QThread>
#include <climits>
#include <nlohmann/json.hpp>

#include "qgis.h"
#include "qgsauthmanager.h"
#include "qgscredentials.h"
#include "qgsdatasourceuri.h"
#include "qgsfields.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshifttablemodel.h"
#include "qgssettings.h"
#include "qgsvectordataprovider.h"
#include "qgswkbtypes.h"

// for htonl
#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif


QgsRedshiftResult::~QgsRedshiftResult()
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = nullptr;
}

QgsRedshiftResult &QgsRedshiftResult::operator=( PGresult *result )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = result;
  return *this;
}

QgsRedshiftResult &QgsRedshiftResult::operator=( const QgsRedshiftResult &src )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = src.result();
  return *this;
}

ExecStatusType QgsRedshiftResult::PQresultStatus()
{
  return mRes ? ::PQresultStatus( mRes ) : PGRES_FATAL_ERROR;
}

QString QgsRedshiftResult::PQresultErrorMessage()
{
  return mRes ? QString::fromUtf8( ::PQresultErrorMessage( mRes ) ) : QObject::tr( "no result buffer" );
}

int QgsRedshiftResult::PQntuples()
{
  Q_ASSERT( mRes );
  return ::PQntuples( mRes );
}

QString QgsRedshiftResult::PQgetvalue( int row, int col )
{
  Q_ASSERT( mRes );
  return PQgetisnull( row, col ) ? QString() : QString::fromUtf8( ::PQgetvalue( mRes, row, col ) );
}

bool QgsRedshiftResult::PQgetisnull( int row, int col )
{
  Q_ASSERT( mRes );
  return ::PQgetisnull( mRes, row, col );
}

int QgsRedshiftResult::PQnfields()
{
  Q_ASSERT( mRes );
  return ::PQnfields( mRes );
}

QString QgsRedshiftResult::PQfname( int col )
{
  Q_ASSERT( mRes );
  return QString::fromUtf8( ::PQfname( mRes, col ) );
}

Oid QgsRedshiftResult::PQftable( int col )
{
  Q_ASSERT( mRes );
  return ::PQftable( mRes, col );
}

int QgsRedshiftResult::PQftablecol( int col )
{
  Q_ASSERT( mRes );
  return ::PQftablecol( mRes, col );
}

Oid QgsRedshiftResult::PQftype( int col )
{
  Q_ASSERT( mRes );
  return ::PQftype( mRes, col );
}

int QgsRedshiftResult::PQfmod( int col )
{
  Q_ASSERT( mRes );
  return ::PQfmod( mRes, col );
}

Oid QgsRedshiftResult::PQoidValue()
{
  Q_ASSERT( mRes );
  return ::PQoidValue( mRes );
}

QgsPoolRedshiftConn::QgsPoolRedshiftConn( const QString &connInfo )
  : mConn( QgsRedshiftConnPool::instance()->acquireConnection( connInfo ) )
{
}

QgsPoolRedshiftConn::~QgsPoolRedshiftConn()
{
  if ( mConn )
    QgsRedshiftConnPool::instance()->releaseConnection( mConn );
}

QMap<QString, QgsRedshiftConn *> QgsRedshiftConn::sConnectionsRO;
QMap<QString, QgsRedshiftConn *> QgsRedshiftConn::sConnectionsRW;

const int QgsRedshiftConn::GEOM_TYPE_SELECT_LIMIT = 100;

QgsRedshiftConn *QgsRedshiftConn::connectDb( const QString &conninfo, bool readonly, bool shared )
{
  QMap<QString, QgsRedshiftConn *> &connections =
    readonly ? QgsRedshiftConn::sConnectionsRO : QgsRedshiftConn::sConnectionsRW;

  // This is called from many places where shared parameter cannot be forced to
  // false (QgsVectorLayerExporter) and which is run in a different thread (drag
  // and drop in browser)
  if ( QApplication::instance()->thread() != QThread::currentThread() )
  {
    shared = false;
  }

  if ( shared )
  {
    // sharing connection between threads is not safe
    // See https://github.com/qgis/QGIS/issues/21205
    Q_ASSERT( QApplication::instance()->thread() == QThread::currentThread() );

    if ( connections.contains( conninfo ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached connection for %1" ).arg( conninfo ), 2 );
      connections[conninfo]->mRef++;
      return connections[conninfo];
    }
  }

  QgsRedshiftConn *conn = new QgsRedshiftConn( conninfo, readonly, shared );

  if ( conn->mRef == 0 )
  {
    delete conn;
    return nullptr;
  }

  if ( shared )
  {
    connections.insert( conninfo, conn );
  }

  return conn;
}

static void noticeProcessor( void *arg, const char *message )
{
  Q_UNUSED( arg )
  QString msg( QString::fromUtf8( message ) );
  msg.chop( 1 );
  QgsMessageLog::logMessage( QObject::tr( "NOTICE: %1" ).arg( msg ), QObject::tr( "Redshift Spatial" ) );
}

QgsRedshiftConn::QgsRedshiftConn( const QString &conninfo, bool readOnly, bool shared )
  : mRef( 1 ), mOpenCursors( 0 ), mConnInfo( conninfo ), mProjAvailable( false ), mGotSpatialVersion( false ),
    mSpatialVersion( 0 ), mSupportsGeography( false ), mUseWkbHex( false ), mReadOnly( readOnly ), mNextCursorId( 0 ),
    mShared( shared ), mLock( QMutex::Recursive )
{
  QgsDebugMsgLevel( QStringLiteral( "New Redshift connection for " ) + conninfo, 2 );

  // expand connectionInfo
  QgsDataSourceUri uri( conninfo );
  QString expandedConnectionInfo = uri.connectionInfo( true );

  auto addDefaultTimeout = []( QString & connectString )
  {
    if ( !connectString.contains( QStringLiteral( "connect_timeout=" ) ) )
    {
      // add default timeout
      QgsSettings settings;
      int timeout =
        settings.value( QStringLiteral( "Redshift/default_timeout" ), QgsRedshiftConn::DEFAULT_TIMEOUT, QgsSettings::Providers )
        .toInt();
      connectString += QStringLiteral( " connect_timeout=%1" ).arg( timeout );
    }
  };
  addDefaultTimeout( expandedConnectionInfo );

  mConn = PQconnectdb( expandedConnectionInfo.toLocal8Bit() ); // use what is set based on locale;
  // after connecting, use Utf8

  // remove temporary cert/key/CA if they exist
  QgsDataSourceUri expandedUri( expandedConnectionInfo );
  QStringList parameters;
  parameters << QStringLiteral( "sslcert" ) << QStringLiteral( "sslkey" ) << QStringLiteral( "sslrootcert" );
  const auto constParameters = parameters;
  for ( const QString &param : constParameters )
  {
    if ( expandedUri.hasParam( param ) )
    {
      QString fileName = expandedUri.param( param );
      fileName.remove( QStringLiteral( "'" ) );
      QFile file( fileName );
      // set minimal permission to allow removing on Win.
      // On linux and Mac if file is set with QFile::ReadUser
      // does not create problem removing certs
      if ( !file.setPermissions( QFile::WriteOwner ) )
      {
        QString errorMsg = tr( "Cannot set WriteOwner permission to cert: %0 to "
                               "allow removing it" )
                           .arg( file.fileName() );
        PQfinish();
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "Redshift Spatial" ) );
        mRef = 0;
        return;
      }
      if ( !file.remove() )
      {
        QString errorMsg = tr( "Cannot remove cert: %0" ).arg( file.fileName() );
        PQfinish();
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "Redshift Spatial" ) );
        mRef = 0;
        return;
      }
    }
  }

  // check the connection status
  if ( PQstatus() != CONNECTION_OK )
  {
    QString username = uri.username();
    QString password = uri.password();

    QgsCredentials::instance()->lock();

    int i = 0;
    while ( PQstatus() != CONNECTION_OK && i < 5 )
    {
      ++i;
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, PQerrorMessage() );
      if ( !ok )
      {
        break;
      }

      PQfinish();

      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsgLevel( "Connecting to " + uri.connectionInfo( false ), 2 );
      QString connectString = uri.connectionInfo();
      addDefaultTimeout( connectString );
      mConn = PQconnectdb( connectString.toLocal8Bit() );
    }

    if ( PQstatus() == CONNECTION_OK )
      QgsCredentials::instance()->put( conninfo, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( PQstatus() != CONNECTION_OK )
  {
    QString errorMsg = PQerrorMessage();
    PQfinish();
    QgsMessageLog::logMessage( tr( "Connection to database failed" ) + '\n' + errorMsg, tr( "Redshift Spatial" ) );
    mRef = 0;
    return;
  }

  // set client encoding to Unicode because QString uses UTF-8 anyway
  QgsDebugMsgLevel( QStringLiteral( "setting client encoding to UNICODE" ), 2 );
  int errcode = PQsetClientEncoding( mConn, QStringLiteral( "UNICODE" ).toLocal8Bit() );
  if ( errcode == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "encoding successfully set" ), 2 );
  }
  else if ( errcode == -1 )
  {
    QgsMessageLog::logMessage( tr( "error in setting encoding" ), tr( "Redshift Spatial" ) );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "undefined return value from encoding setting" ), tr( "Redshift Spatial" ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "Connection to the database was successful" ), 2 );
  mDatabaseName = uri.database();
  setClusterType();
  spatialVersion();

  /* Check to see if we have working Spatial support */
  if ( spatialVersion().isNull() )
  {
    QgsMessageLog::logMessage( tr( "Your Redshift instance has no Spatial support. Feature selection "
                                   "and identification will not work properly." ),
                               tr( "Redshift Spatial" ) );
  }
  else
    QgsDebugMsgLevel( QStringLiteral( "Spatial support available!" ), 3 );

  PQsetNoticeProcessor( mConn, noticeProcessor, nullptr );

  PQexec( QString( "SET extra_float_digits=2" ), false );
}

QgsRedshiftConn::~QgsRedshiftConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mConn )
    ::PQfinish( mConn );
  mConn = nullptr;
}

void QgsRedshiftConn::ref()
{
  QMutexLocker locker( &mLock );
  ++mRef;
}

void QgsRedshiftConn::unref()
{
  QMutexLocker locker( &mLock );
  if ( --mRef > 0 )
    return;

  if ( mShared )
  {
    QMap<QString, QgsRedshiftConn *> &connections = mReadOnly ? sConnectionsRO : sConnectionsRW;

    QString key = connections.key( this, QString() );

    Q_ASSERT( !key.isNull() );
    connections.remove( key );
  }

  // to avoid destroying locked mutex
  locker.unlock();
  delete this;
}

/* private */
QStringList QgsRedshiftConn::supportedSpatialTypes() const
{
  QStringList supportedSpatialTypes;

  supportedSpatialTypes << quotedValue( "geometry" )
                        << quotedValue( "geography" );

  return supportedSpatialTypes;
}

bool QgsRedshiftConn::setPkColumns( const QString &fromTable,
                                    QStringList &pkCols )
{
  QString query = QString( "SELECT * from %1 LIMIT 0" ).arg( fromTable );
  QgsRedshiftResult result( PQexec( query, true ) );

  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage(
      tr( "Failed to get column info for datashare %1" ).arg( fromTable ),
      tr( "Redshift Spatial" ) );
    return false;
  }
  QStringList temp;
  for ( int i = 0; i < result.PQnfields(); i++ )
  {
    QString currentColumn = result.PQfname( i );
    // push accepted column name(s) to the front
    if ( mStandardKeyColumns.contains( currentColumn ) )
    {
      pkCols << currentColumn;
    }
    else
    {
      temp << currentColumn;
    }
  }
  // push the rest to the back of the suggestions;
  pkCols << temp;
  QgsDebugMsg( QStringLiteral( "datashare %1 columns: %2" )
               .arg( fromTable, temp.join( ", " ) ) );
  return true;
}

void QgsRedshiftConn::getDatashareTablesInfo( const QString &metaTableName,
    const int tableType,
    const QString &currentDatabaseName )
{
  QString query = QString( "SELECT * from %1 where f_table_catalog <> %2"
                           " AND f_table_catalog <> ''" )
                  .arg( metaTableName, quotedValue( currentDatabaseName ) );
  QgsDebugMsgLevel( "getting data share table info: " + query, 2 );
  QgsRedshiftResult result( PQexec( query, true ) );

  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( tr( "Failed to get datashare tables info" ),
                               tr( "Redshift Spatial" ) );
    return;
  }

  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    QString databaseName = result.PQgetvalue( idx, 0 );
    QString schemaName = result.PQgetvalue( idx, 1 );
    QString tableName = result.PQgetvalue( idx, 2 );
    QString column = result.PQgetvalue( idx, 3 );
    QString coordDimension = result.PQgetvalue( idx, 4 );
    QString ssrid = result.PQgetvalue( idx, 5 );

    QgsRedshiftGeometryColumnType columnType = SctNone;

    if ( tableType == SctGeometry )
      columnType = SctGeometry;
    else if ( tableType == SctGeography )
      columnType = SctGeography;
    else
      QgsDebugMsgLevel( QStringLiteral( "unknown columnType: %1" )
                        .arg( tableType ), 2 );

    int srid = ssrid.isEmpty()
               ? std::numeric_limits<int>::min() : ssrid.toInt();

    // srid = 0 doesn't constraint the geometry srids
    if ( srid == 0 )
    {
      srid = std::numeric_limits<int>::min();
    }

    QgsRedshiftLayerProperty layerProperty;
    const QString completeTableName = QStringLiteral( "%1.%2.%3" )
                                      .arg( quotedIdentifier( databaseName ) )
                                      .arg( quotedIdentifier( schemaName ) )
                                      .arg( quotedIdentifier( tableName ) );
    // skip datashare if SELECT fails
    if ( !setPkColumns( completeTableName, layerProperty.pkCols ) )
    {
      continue;
    }

    layerProperty.databaseName = databaseName;
    layerProperty.schemaName = schemaName;
    layerProperty.tableName = tableName;
    layerProperty.geometryColName = column;
    layerProperty.geometryColType = columnType;

    layerProperty.types = QList<Qgis::WkbType>()
                          << ( QgsWkbTypes::parseType( coordDimension ) );
    layerProperty.srids = QList<int>() << srid;
    layerProperty.sql.clear();
    layerProperty.relKind = 'r'; // r = ordinary table
    layerProperty.isView = false;

    layerProperty.isMaterializedView = false;
    layerProperty.tableComment = QString( "external table" );

    layerProperty.nSpCols = 1;  // TODO(reflectored): check if we need to query how
    // many there are

    mLayersSupported << layerProperty;
  }
}

bool QgsRedshiftConn::getTableInfo( bool searchPublicOnly, bool allowGeometrylessTables, QString schema )
{
  QMutexLocker locker( &mLock );
  int nColumns = 0;
  QgsRedshiftResult result;
  QString query;

  schema = schema.toLower(); // lowercase the schema name

  mLayersSupported.clear();

  for ( int i = SctGeometry; i <= SctGeography; ++i )
  {
    QString sql, tableName, schemaName, columnName, typeName,
            sridName, gtableName;

    if ( i == SctGeometry )
    {
      tableName = QStringLiteral( "l.f_table_name" );
      schemaName = QStringLiteral( "l.f_table_schema" );
      columnName = QStringLiteral( "l.f_geometry_column" );
      typeName = QStringLiteral( "upper(l.type)" );
      sridName = QStringLiteral( "l.srid" );
      gtableName = QStringLiteral( "svv_geometry_columns" );
    }
    else if ( i == SctGeography )
    {
      // TODO(marcel): enable when geography support is enabled
      if ( !supportsGeography() )
        continue;
      tableName = QStringLiteral( "l.f_table_name" );
      schemaName = QStringLiteral( "l.f_table_schema" );
      columnName = QStringLiteral( "l.f_geography_column" );
      typeName = QStringLiteral( "upper(l.type)" );
      sridName = QStringLiteral( "l.srid" );
      gtableName = QStringLiteral( "geography_columns" );
    }

    // Check if the catalog table exists
    sql = QString( "SELECT * FROM %1 LIMIT 0" ).arg( gtableName );
    result = PQexec( sql, false );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Couldn't access catalog view: %1. Check that the view was "
                                     "properly created and you have access to it. "
                                     "The error message from the database was:\n%2\n" )
                                 .arg( gtableName, result.PQresultErrorMessage() ),
                                 tr( "Redshift Spatial" ) );
      continue;
    }
    // Add all available datashare columns to layerProperties
    getDatashareTablesInfo( gtableName, i, mDatabaseName );

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.

    sql = QString( "SELECT %1,%2,%3,%4,%5,c.relkind,pg_descr.description,"
                   "listagg(a.attname::text , ',') "
                   "WITHIN GROUP (ORDER BY a.attnum), "
                   "count(CASE WHEN t.typname IN (%7) THEN 1 ELSE NULL END), "
                   "mv_info.state "
                   " FROM %6 l, "
                   " pg_class c LEFT JOIN pg_namespace n ON n.oid=c.relnamespace "
                   " LEFT JOIN pg_description pg_descr ON c.oid=pg_descr.objoid "
                   " LEFT JOIN stv_mv_info mv_info ON"
                   " db_name='%8'"
                   " AND schema = n.nspname AND name=c.relname,"
                   " pg_attribute a,pg_type t"
                   " WHERE c.relname::text=%1"
                   " AND l.f_table_catalog='%8'"
                   " AND NOT a.attisdropped"
                   " AND a.attrelid=c.oid"
                   " AND a.atttypid=t.oid"
                   " AND a.attnum>0"
                   " AND n.oid=c.relnamespace" )
          .arg( tableName, schemaName, columnName, typeName, sridName,
                gtableName, supportedSpatialTypes().join( ',' ),
                mDatabaseName );

    if ( searchPublicOnly )
      sql += QLatin1String( " AND n.nspname='public'" );

    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND %1='%2'" ).arg( schemaName, schema );

    sql += QString( " GROUP BY 1,2,3,4,5,6,7,10" );

    // execute query
    query = sql;
    query += QLatin1String( " ORDER BY 2,1,3" );
    QgsDebugMsgLevel( "getting table info from layer registries: " + query, 2 );
    result = PQexec( query, true );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Failed to get table info from: " ).arg( gtableName ), tr( "Redshift Spatial" ) );
      continue;
    }

    for ( int idx = 0; idx < result.PQntuples(); idx++ )
    {
      QString tableName = result.PQgetvalue( idx, 0 );
      QString schemaName = result.PQgetvalue( idx, 1 );
      QString column = result.PQgetvalue( idx, 2 );
      QString type = result.PQgetvalue( idx, 3 );
      QString ssrid = result.PQgetvalue( idx, 4 );
      QString relkind = result.PQgetvalue( idx, 5 );
      QString comment = result.PQgetvalue( idx, 6 );
      QString attributes = result.PQgetvalue( idx, 7 );
      int nSpCols = result.PQgetvalue( idx, 8 ).toInt();
      bool isView = relkind == QLatin1String( "v" );
      bool isMaterializedView = !result.PQgetvalue( idx, 9 ).isNull();

      QgsRedshiftGeometryColumnType columnType = SctNone;

      if ( i == SctGeometry )
        columnType = SctGeometry;
      else if ( i == SctGeography )
        columnType = SctGeography;

      int srid = ssrid.isEmpty() ? std::numeric_limits<int>::min() : ssrid.toInt();

      // srid = 0 doesn't constraint the geometry srids
      if ( srid == 0 )
      {
        srid = std::numeric_limits<int>::min();
      }

      QgsRedshiftLayerProperty layerProperty;
      layerProperty.databaseName = mDatabaseName;
      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = column;
      layerProperty.geometryColType = columnType;

      layerProperty.types = QList<Qgis::WkbType>() << ( QgsWkbTypes::parseType( type ) );
      layerProperty.srids = QList<int>() << srid;
      layerProperty.sql.clear();
      layerProperty.relKind = relkind;
      layerProperty.isView = isView;

      layerProperty.isMaterializedView = isMaterializedView;
      layerProperty.tableComment = comment;

      layerProperty.nSpCols = nSpCols;
      // if ( isView )
      // {
      layerProperty.pkCols = attributes.split( QLatin1String( "," ), Qt::SkipEmptyParts );
      // }

      if ( /*isView &&*/ layerProperty.pkCols.empty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "no key columns found." ), 2 );
        continue;
      }

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( allowGeometrylessTables )
  {
    QString sql = QStringLiteral( "SELECT "
                                  "c.relname::text"
                                  ",n.nspname::text"
                                  ",c.relkind"
                                  ",pg_descr.description"
                                  ",listagg(a.attname::text, ',') WITHIN GROUP (ORDER BY a.attnum)"
                                  ",stv_mv_info.state"
                                  " FROM "
                                  " pg_class c LEFT JOIN pg_namespace n ON n.oid = c.relnamespace "
                                  " LEFT JOIN stv_mv_info ON db_name='%1' AND schema=n.nspname AND name=c.relname "
                                  " LEFT JOIN pg_description pg_descr ON c.oid=pg_descr.objoid "
                                  ",pg_attribute a"
                                  " WHERE c.relkind IN ('v','r')"
                                  " AND c.oid = a.attrelid"
                                  " AND NOT a.attisdropped"
                                  " AND a.attnum > 0" ).arg( mDatabaseName );

    // user has select privilege
    if ( searchPublicOnly )
      sql += QLatin1String( " AND n.nspname='public'" );

    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND n.nspname='%2'" ).arg( schema );

    sql += QLatin1String( " GROUP BY 1,2,3,4,6" );

    QgsDebugMsgLevel( "getting non-spatial table info: " + sql, 2 );

    result = PQexec( sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables "
                                     "could not be determined.\nThe error message from the database "
                                     "was:\n%1" )
                                 .arg( result.PQresultErrorMessage() ),
                                 tr( "Redshift Spatial" ) );
      return false;
    }


    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      QString table = result.PQgetvalue( i, 0 ); // relname
      QString schema = result.PQgetvalue( i, 1 ); // nspname
      QString relkind = result.PQgetvalue( i, 2 ); // relation kind
      QString comment = result.PQgetvalue( i, 3 ); // table comment
      QString attributes = result.PQgetvalue( i, 4 ); // table columns

      bool isView = relkind == QLatin1String( "v" );
      bool isMaterializedView = !result.PQgetvalue( i, 5 ).isNull();

      QgsRedshiftLayerProperty layerProperty;
      layerProperty.databaseName = mDatabaseName;
      layerProperty.types = QList<Qgis::WkbType>() << Qgis::WkbType::NoGeometry;
      layerProperty.srids = QList<int>() << std::numeric_limits<int>::min();
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = QString();
      layerProperty.geometryColType = SctNone;
      layerProperty.nSpCols = 0;
      layerProperty.relKind = relkind;
      layerProperty.isView = isView;
      layerProperty.isMaterializedView = isMaterializedView;
      layerProperty.tableComment = comment;

      // check if we've already added this layer in some form
      bool alreadyFound = false;
      const auto constMLayersSupported = mLayersSupported;
      for ( const QgsRedshiftLayerProperty &foundLayer : constMLayersSupported )
      {
        if ( foundLayer.schemaName == schema && foundLayer.tableName == table )
        {
          // already found this table
          alreadyFound = true;
          break;
        }
      }
      if ( alreadyFound )
        continue;

      if ( isView )
      {
        layerProperty.pkCols = attributes.split( QLatin1String( "," ), Qt::SkipEmptyParts );
      }

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( nColumns == 0 && schema.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the "
                                   "accessible tables could not be determined." ),
                               tr( "Redshift Spatial" ) );
  }

  return true;
}

bool QgsRedshiftConn::supportedLayers( QVector<QgsRedshiftLayerProperty> &layers, bool searchPublicOnly,
                                       bool allowGeometrylessTables, QString schema )
{
  QMutexLocker locker( &mLock );
  schema = schema.toLower(); // lower case the schema name

  // Get the list of supported tables
  if ( !getTableInfo( searchPublicOnly, allowGeometrylessTables, schema ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ),
                               tr( "Redshift Spatial" ) );
    return false;
  }

  layers = mLayersSupported;

  return true;
}

bool QgsRedshiftConn::getSchemas( QList<QgsRedshiftSchemaProperty> &schemas )
{
  schemas.clear();
  QgsRedshiftResult result;

  QString sql = QStringLiteral( "SELECT nspname, pg_get_userbyid(nspowner), "
                                "pg_catalog.obj_description(oid) FROM pg_namespace WHERE nspname !~ "
                                "'^pg_' AND nspname != 'information_schema' ORDER BY nspname" );

  result = PQexec( sql, true );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    PQexecNR( QStringLiteral( "COMMIT" ) );
    return false;
  }

  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    QgsRedshiftSchemaProperty schema;
    schema.name = result.PQgetvalue( idx, 0 );
    schema.owner = result.PQgetvalue( idx, 1 );
    schema.description = result.PQgetvalue( idx, 2 );
    schemas << schema;
  }
  return true;
}

/* Function that determines the Redshift spatial version */
QString QgsRedshiftConn::spatialVersion() const
{
  QMutexLocker locker( &mLock );
  if ( mGotSpatialVersion )
    return mSpatialVersionInfo;

  QgsRedshiftResult result( PQexec( QStringLiteral( "SELECT spatial_version()" ), false ) );
  if ( result.PQntuples() != 1 )
  {
    QgsMessageLog::logMessage( tr( "No Spatial support in the database." ), tr( "Redshift Spatial" ) );
    mGotSpatialVersion = true;
    return QString();
  }

  mSpatialVersionInfo = result.PQgetvalue( 0, 0 );
  mSpatialVersion = mSpatialVersionInfo.toInt();

  QgsDebugMsgLevel( tr( "Redshift Spatial version: %1" ).arg( mSpatialVersionInfo ), 2 );

  mGotSpatialVersion = true;
  mSupportsGeography = false;
  mUseWkbHex = true;

  return mSpatialVersionInfo;
}

/* Function that determines the Redshift cluster type */
void QgsRedshiftConn::setClusterType()
{
  QMutexLocker locker( &mLock );
  QgsRedshiftResult result(
    PQexec( QStringLiteral( "SELECT MAX(node)+1 AS cluster_size "
                            "FROM stv_slices" ) ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    mIsSingleNode = result.PQgetvalue( 0, 0 ).toInt() < 2;
  }
  else
    QgsMessageLog::logMessage( tr( "Could not determine cluster type" ),
                               tr( "Redshift Spatial" ) );
}

int QgsRedshiftConn::getFetchLimit() const
{
  return mIsSingleNode ? SINGLE_NODE_MAX_FETCH_SIZE : MULTI_NODE_MAX_FETCH_SIZE;
}

bool QgsRedshiftConn::supportsGeography() const
{
  spatialVersion();
  return mSupportsGeography;
}

QString QgsRedshiftConn::quotedIdentifier( const QString &ident )
{
  QString result = ident;
  result.replace( '"', QLatin1String( "\"\"" ) );
  return result.prepend( '\"' ).append( '\"' );
}

static QString quotedString( const QString &v )
{
  QString result = v;
  result.replace( '\'', QLatin1String( "''" ) );
  if ( result.contains( '\\' ) )
    return result.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "E'" ).append( '\'' );
  else
    return result.prepend( '\'' ).append( '\'' );
}

QString QgsRedshiftConn::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
      return value.toString();

    case QVariant::DateTime:
      return quotedString( value.toDateTime().toString( Qt::ISODateWithMs ) ) + "::timestamp";

    case QVariant::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    default:
      return quotedString( value.toString() );
  }
}

bool QgsRedshiftConn::isMaterializedView( const QString &schemaName, const QString &viewName )
{
  QString sql = QStringLiteral( "SELECT count(*) FROM stv_mv_info "
                                "WHERE schema=%1 AND name=%2 AND db_name='%3';" )
                .arg( quotedValue( schemaName ), quotedValue( viewName ),
                      mDatabaseName );
  QgsRedshiftResult res( PQexec( sql ) );

  return res.PQgetvalue( 0, 0 ).toInt() != 0;
}

PGresult *QgsRedshiftConn::PQexec( const QString &query, bool logError, bool retry ) const
{
  QMutexLocker locker( &mLock );

  QgsDebugMsgLevel( QStringLiteral( "Executing SQL: %1" ).arg( query ), 3 );

  PGresult *res = ::PQexec( mConn, query.toUtf8() );

  // libpq may return a non null ptr with conn status not OK so we need to check
  // for it to allow a retry below
  if ( res && PQstatus() == CONNECTION_OK )
  {
    int errorStatus = PQresultStatus( res );
    if ( errorStatus != PGRES_COMMAND_OK && errorStatus != PGRES_TUPLES_OK )
    {
      if ( logError )
      {
        QgsMessageLog::logMessage( tr( "Erroneous query: %1 returned %2 [%3]" )
                                   .arg( query )
                                   .arg( errorStatus )
                                   .arg( PQresultErrorMessage( res ) ),
                                   tr( "Redshift Spatial" ) );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Not logged erroneous query: %1 returned %2 [%3]" )
                     .arg( query )
                     .arg( errorStatus )
                     .arg( PQresultErrorMessage( res ) ) );
      }
    }
    return res;
  }
  if ( PQstatus() != CONNECTION_OK )
  {
    if ( logError )
    {
      QgsMessageLog::logMessage(
        tr( "Connection error: %1 returned %2 [%3]" ).arg( query ).arg( PQstatus() ).arg( PQerrorMessage() ),
        tr( "Redshift Spatial" ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Connection error: %1 returned %2 [%3]" )
                   .arg( query )
                   .arg( PQstatus() )
                   .arg( PQerrorMessage() ) );
    }
  }
  else
  {
    if ( logError )
    {
      QgsMessageLog::logMessage( tr( "Query failed: %1\nError: no result buffer" ).arg( query ),
                                 tr( "Redshift Spatial" ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Not logged query failed: %1\nError: no result buffer" ).arg( query ) );
    }
  }

  if ( retry )
  {
    QgsMessageLog::logMessage( tr( "resetting bad connection." ), tr( "Redshift Spatial" ) );
    ::PQreset( mConn );
    res = PQexec( query, logError, false );
    if ( PQstatus() == CONNECTION_OK )
    {
      if ( res )
      {
        QgsMessageLog::logMessage( tr( "retry after reset succeeded." ), tr( "Redshift Spatial" ) );
        return res;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "retry after reset failed again." ), tr( "Redshift Spatial" ) );
        return nullptr;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "connection still bad after reset." ), tr( "Redshift Spatial" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "bad connection, not retrying." ), tr( "Redshift Spatial" ) );
  }
  return nullptr;
}

bool QgsRedshiftConn::openCursor( const QString &cursorName, const QString &sql,
                                  bool isExternalDatabase )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors

  if ( mOpenCursors++ == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Starting read-only transaction" ), 4 );

    PQexecNR( QStringLiteral( "BEGIN%1" )
              .arg( ( !isExternalDatabase ? " READ ONLY" : "" ) ) );
  }
  QgsDebugMsgLevel( QStringLiteral( "Binary cursor %1 for %2" ).arg( cursorName, sql ), 3 );
  return PQexecNR( QStringLiteral(
                     "DECLARE %1 BINARY CURSOR FOR %3 " )
                   .arg( cursorName, sql ) );
}

bool QgsRedshiftConn::closeCursor( const QString &cursorName )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors

  if ( cursorName.isEmpty() || !PQexecNR( QStringLiteral( "CLOSE %1" ).arg( cursorName ) ) )
    return false;

  if ( --mOpenCursors == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Committing read-only transaction" ), 4 );
    PQexecNR( QStringLiteral( "COMMIT" ) );
  }

  return true;
}

QString QgsRedshiftConn::uniqueCursorName()
{
  QMutexLocker locker( &mLock ); // to protect access to mNextCursorId
  return QStringLiteral( "qgis_%1" ).arg( ++mNextCursorId );
}

int QgsRedshiftConn::PQCancel()
{
  // No locker: this is supposed to be thread safe
  int result = 0;
  auto cancel = ::PQgetCancel( mConn );
  if ( cancel )
  {
    char errbuf[255];
    result = ::PQcancel( cancel, errbuf, 255 );
    if ( !result )
      QgsDebugMsgLevel( QStringLiteral( "Error canceling the query:" ).arg( errbuf ), 3 );
  }
  ::PQfreeCancel( cancel );
  return result;
}

bool QgsRedshiftConn::PQexecNR( const QString &query )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors

  QgsRedshiftResult res( PQexec( query, false ) );

  ExecStatusType errorStatus = res.PQresultStatus();
  if ( errorStatus == PGRES_COMMAND_OK )
    return true;

  QgsMessageLog::logMessage(
    tr( "Query: %1 returned %2 [%3]" ).arg( query ).arg( errorStatus ).arg( res.PQresultErrorMessage() ),
    tr( "Redshift Spatial" ) );

  if ( mOpenCursors )
  {
    QgsMessageLog::logMessage( tr( "%1 cursor states lost.\nSQL: %2\nResult: %3 (%4)" )
                               .arg( mOpenCursors )
                               .arg( query )
                               .arg( errorStatus )
                               .arg( res.PQresultErrorMessage() ),
                               tr( "Redshift Spatial" ) );
    mOpenCursors = 0;
  }

  if ( PQstatus() == CONNECTION_OK )
  {
    PQexecNR( QStringLiteral( "ROLLBACK" ) );
  }

  return false;
}

PGresult *QgsRedshiftConn::PQgetResult()
{
  return ::PQgetResult( mConn );
}

PGresult *QgsRedshiftConn::PQprepare( const QString &stmtName, const QString &query, int nParams, const Oid *paramTypes )
{
  QMutexLocker locker( &mLock );

  return ::PQprepare( mConn, stmtName.toUtf8(), query.toUtf8(), nParams, paramTypes );
}

PGresult *QgsRedshiftConn::PQexecPrepared( const QString &stmtName, const QStringList &params )
{
  QMutexLocker locker( &mLock );

  const char **param = new const char *[params.size()];
  QList<QByteArray> qparam;

  qparam.reserve( params.size() );
  for ( int i = 0; i < params.size(); i++ )
  {
    qparam << params[i].toUtf8();

    if ( params[i].isNull() )
      param[i] = nullptr;
    else
      param[i] = qparam[i];
  }

  PGresult *res = ::PQexecPrepared( mConn, stmtName.toUtf8(), params.size(), param, nullptr, nullptr, 0 );

  delete[] param;

  return res;
}

void QgsRedshiftConn::PQfinish()
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  ::PQfinish( mConn );
  mConn = nullptr;
}

int QgsRedshiftConn::PQstatus() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  return ::PQstatus( mConn );
}

QString QgsRedshiftConn::PQerrorMessage() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  return QString::fromUtf8( ::PQerrorMessage( mConn ) );
}

int QgsRedshiftConn::PQsendQuery( const QString &query )
{
  QMutexLocker locker( &mLock );
  Q_ASSERT( mConn );
  return ::PQsendQuery( mConn, query.toUtf8() );
}

bool QgsRedshiftConn::begin()
{
  QMutexLocker locker( &mLock );

  return PQexecNR( QStringLiteral( "BEGIN" ) );
}

bool QgsRedshiftConn::commit()
{
  QMutexLocker locker( &mLock );

  return PQexecNR( QStringLiteral( "COMMIT" ) );
}

bool QgsRedshiftConn::rollback()
{
  QMutexLocker locker( &mLock );

  return PQexecNR( QStringLiteral( "ROLLBACK" ) );
}

QString QgsRedshiftConn::fieldExpressionForWhereClause( const QgsField &fld, QVariant::Type valueType, QString expr )
{
  QString out;
  const QString &type = fld.typeName();

  if ( type == QLatin1String( "timestamp" ) || type == QLatin1String( "time" ) || type == QLatin1String( "date" ) ||
       type == QLatin1String( "timestamptz" ) || type == QLatin1String( "timetz" ) )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // If field and value have incompatible types, rollback to text cast.
    if ( valueType != QVariant::LastType && valueType != QVariant::DateTime && valueType != QVariant::Date &&
         valueType != QVariant::Time )
    {
      out = out + "::text";
    }
  }

  else if ( type == QLatin1String( "int8" ) || type == QLatin1String( "int2" ) || type == QLatin1String( "int4" ) ||
            type == QLatin1String( "float4" ) || type == QLatin1String( "float8" )
            || type == QLatin1String( "numeric" ) )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // If field and value have incompatible types, rollback to text cast.
    if ( valueType != QVariant::LastType && valueType != QVariant::Int && valueType != QVariant::LongLong &&
         valueType != QVariant::Double )
    {
      out = out + "::text";
    }
  }

  else
  {
    out = fieldExpression( fld, expr ); // same as fieldExpression by default
  }

  return out;
}

QString QgsRedshiftConn::fieldExpression( const QgsField &fld, QString expr )
{
  const QString &type = fld.typeName();
  expr = expr.arg( quotedIdentifier( fld.name() ) );
  if ( type == QLatin1String( "bool" ) )
  {
    return QStringLiteral( "decode(%1,true,'t',false,'f')::text" ).arg( expr );
  }
  else if ( type == QLatin1String( "geometry" ) )
  {
    return expr;
    // return QStringLiteral( "%1(%2)" ).arg( "st_asewkt", expr );
  }
  else if ( type == QLatin1String( "geography" ) )
  {
    return QStringLiteral( "st_astext(%1)" ).arg( expr );
  }
  else
  {
    return expr + "::text";
  }
}

QList<QgsVectorDataProvider::NativeType> QgsRedshiftConn::nativeTypes()
{
  QList<QgsVectorDataProvider::NativeType> types;

  types // integer types
      << QgsVectorDataProvider::NativeType( tr( "Whole number (smallint - 16bit)" ), QStringLiteral( "int2" ),
          QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 32bit)" ), QStringLiteral( "int4" ),
          QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole number (bigint - 64bit)" ), QStringLiteral( "int8" ),
          QVariant::LongLong, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), QStringLiteral( "numeric" ),
          QVariant::Double, 1, 20, 0, 20 )
      // floating point
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "float4" ), QVariant::Double,
          -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (double precision)" ), QStringLiteral( "float8" ),
          QVariant::Double, -1, -1, -1, -1 )

      // string types
      << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QVariant::String,
          1, 255, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ),
          QVariant::String, 1, 255, -1, -1 )

      // date type
      << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Time (without time zone)" ), QStringLiteral( "time" ), QVariant::Time, -1,
          -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Time (with time zone)" ), QStringLiteral( "timetz" ), QVariant::String,
          -1, -1, -1, -1 )

      << QgsVectorDataProvider::NativeType( tr( "Timestamp (without time zone)" ), QStringLiteral( "timestamp" ),
          QVariant::DateTime, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Timestamp (with time zone)" ), QStringLiteral( "timestamptz" ),
          QVariant::String, -1, -1, -1, -1 )

      // boolean
      << QgsVectorDataProvider::NativeType( tr( "Boolean" ), QStringLiteral( "bool" ), QVariant::Bool, -1, -1, -1, -1 )

      ;

  return types;
}

void QgsRedshiftConn::retrieveLayerTypes( QgsRedshiftLayerProperty &layerProperty, bool useEstimatedMetadata )
{
  QVector<QgsRedshiftLayerProperty *> vect;
  vect << &layerProperty;
  retrieveLayerTypes( vect, useEstimatedMetadata );
}

void QgsRedshiftConn::retrieveLayerTypes( QVector<QgsRedshiftLayerProperty *> &layerProperties,
    bool useEstimatedMetadata )
{
  if ( layerProperties.isEmpty() )
    return;

  QString query;

  static bool warnUserUnrestrictedColumns = true;
  if ( warnUserUnrestrictedColumns )
  {
    QgsMessageLog::logMessage( tr( "Some of the layers have unrestricted types or SRIDs. Consider "
                                   "updating the types of your geometry columns or propertly casting "
                                   "the columns of your views/materialized views." ),
                               tr( "Redshift Spatial" ) );
    warnUserUnrestrictedColumns = false;
  }

  int i = 0;
  for ( auto *layerPropertyPtr : layerProperties )
  {
    QString table;
    QgsRedshiftLayerProperty &layerProperty = *layerPropertyPtr;

    if ( i++ )
      query += " UNION ";
    QgsDebugMsg( QStringLiteral( "db.schema.table: %1.%2.%3" )
                 .arg( layerProperty.databaseName )
                 .arg( layerProperty.schemaName )
                 .arg( layerProperty.tableName ) );
    if ( !layerProperty.databaseName.isEmpty() )
    {
      table += QStringLiteral( "%1." ).arg(
                 quotedIdentifier( layerProperty.databaseName ) );
    }

    if ( !layerProperty.schemaName.isEmpty() )
    {
      table += QStringLiteral( "%1.%2" )
               .arg( quotedIdentifier( layerProperty.schemaName ),
                     quotedIdentifier( layerProperty.tableName ) );
    }
    else
    {
      // Query
      table = layerProperty.tableName;
    }

    if ( layerProperty.geometryColName.isEmpty() )
      continue;

    // our estimatation ignores that a where clause might restrict the feature
    // type or srid
    if ( useEstimatedMetadata )
    {
      table =
        QStringLiteral( "(SELECT %1 FROM %2 WHERE %3%1 IS NOT NULL LIMIT %4) AS t" )
        .arg( quotedIdentifier( layerProperty.geometryColName ), table,
              layerProperty.sql.isEmpty() ? QString() : QStringLiteral( " (%1) AND " ).arg( layerProperty.sql ) )
        .arg( GEOM_TYPE_SELECT_LIMIT );
    }
    else if ( !layerProperty.sql.isEmpty() )
    {
      table += QStringLiteral( " WHERE %1" ).arg( layerProperty.sql );
    }

    QString sql = QStringLiteral( "SELECT %1, " ).arg( i - 1 );

    bool castToGeometry = layerProperty.geometryColType == SctGeography;

    // sql += QStringLiteral( "listagg(DISTINCT " );

    int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
    if ( srid == std::numeric_limits<int>::min() )
    {
      sql += QStringLiteral( "%1(%2%3)::text" )
             .arg( "st_srid", quotedIdentifier( layerProperty.geometryColName ),
                   castToGeometry ? "::geometry" : "" );
    }
    else
    {
      sql += QStringLiteral( "%1::text" ).arg( QString::number( srid ) );
    }

    sql += " , ";

    Qgis::WkbType type = layerProperty.types.value( 0, Qgis::WkbType::Unknown );
    if ( type == Qgis::WkbType::Unknown )
    {
      sql += QStringLiteral( "UPPER(geometrytype(%1%2))" )
             .arg( quotedIdentifier( layerProperty.geometryColName ), castToGeometry ? "::geometry" : "" );
    }
    else
    {
      sql += QStringLiteral( "%1::text" ).arg( quotedValue( QgsRedshiftConn::redshiftWkbTypeName( type ) ) );
    }

    // sql += ", ',') ";

    sql += " FROM " + table;

    QgsDebugMsgLevel( "Geometry types,srids and dims query: " + sql, 2 );

    query += sql;
  }

  QgsDebugMsgLevel( "Layer types,srids and dims query: " + query, 3 );

  QgsRedshiftResult res( PQexec( query ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( tr( "Couldn't get layer types of unrestricted layers!" ), tr( "Redshift Spatial" ) );
    return;
  }

  QHash<int, QList<std::pair<Qgis::WkbType, int>>> foundCombinationsPerLayer;
  for ( int i = 0; i < res.PQntuples(); i++ )
  {
    int idx = res.PQgetvalue( i, 0 ).toInt();
    int srid = res.PQgetvalue( i, 1 ).toInt();
    QString typeString = res.PQgetvalue( i, 2 );
    #ifdef QGISDEBUG
    QgsRedshiftLayerProperty &layerProperty = *layerProperties[idx];
    #endif
    if ( srid == 0 || typeString == "NULL" )
      continue;

    /* Gather found types */
    auto foundCombinations = foundCombinationsPerLayer.find( idx );
    auto type = QgsWkbTypes::parseType( typeString );
    auto flatType = QgsWkbTypes::flatType( type );
    auto multiType = QgsWkbTypes::multiType( flatType );

    // if both multi and single types exists, go for the multi type,
    // so that st_multi can be applied if necessary.

    int j = 0;
    for ( ; foundCombinations != foundCombinationsPerLayer.end() && j < foundCombinations.value().length(); j++ )
    {
      auto foundPair = foundCombinations.value().at( j );
      if ( foundPair.second != srid )
        continue; // srid must match

      auto knownType = foundPair.first;
      if ( type == knownType )
        break; // found

      auto knownMultiType = QgsWkbTypes::multiType( knownType );

      if ( multiType == knownMultiType )
      {
        QgsDebugMsgLevel(
          QStringLiteral( "Upgrading type[%1] of layer %2.%3.%4 "
                          "to multi type %5" )
          .arg( j )
          .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
          .arg( qgsEnumValueToKey( multiType ) ),
          3 );
        foundCombinations.value()[j].first = multiType;
        break;
      }
    }
    if ( foundCombinations != foundCombinationsPerLayer.end() && j < foundCombinations.value().length() )
    {
      continue; // already found
    }

    QgsDebugMsgLevel( QStringLiteral( "Setting typeSridCombination[%1] of layer %2.%3.%4 "
                                      "to srid %5 and type %6" )
                      .arg( j )
                      .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                      .arg( srid )
                      .arg( qgsEnumValueToKey( type ) ),
                      3 );
    if ( foundCombinations != foundCombinationsPerLayer.end() )
    {
      foundCombinations.value() << std::make_pair( type, srid );
    }
    else
    {
      foundCombinationsPerLayer[idx] = QList<std::pair<Qgis::WkbType, int>>();
      foundCombinationsPerLayer[idx] << std::make_pair( type, srid );
    }
  }
  for ( auto iterator = foundCombinationsPerLayer.keyValueBegin(); iterator != foundCombinationsPerLayer.keyValueEnd(); ++iterator )
  {
    auto [idx, srids_and_types] = *iterator;
    QgsRedshiftLayerProperty &layerProperty = *layerProperties[idx];
    QgsDebugMsgLevel( QStringLiteral( "Completed scan of %1 srid/type combinations "
                                      "for layer of layer %2.%3.%4 " )
                      .arg( srids_and_types.length() )
                      .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ),
                      2 );

    /* Rewrite srids and types to match found combinations
     * of srids and types */
    if ( srids_and_types.length() > 0 )
    {
      layerProperty.srids.clear();
      layerProperty.types.clear();
      for ( const auto &comb : srids_and_types )
      {
        layerProperty.types << comb.first;
        layerProperty.srids << comb.second;
      }
      QgsDebugMsgLevel( QStringLiteral( "Final layer %1.%2.%3 types: %4" )
                        .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                        .arg( layerProperty.types.length() ),
                        2 );
      QgsDebugMsgLevel( QStringLiteral( "Final layer %1.%2.%3 srids: %4" )
                        .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                        .arg( layerProperty.srids.length() ),
                        2 );

    }
  }
}

void QgsRedshiftConn::redshiftWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim )
{
  dim = 2;
  Qgis::WkbType flatType = QgsWkbTypes::flatType( wkbType );
  switch ( flatType )
  {
    case Qgis::WkbType::Point:
      geometryType = QStringLiteral( "POINT" );
      break;

    case Qgis::WkbType::LineString:
      geometryType = QStringLiteral( "LINESTRING" );
      break;

    case Qgis::WkbType::Polygon:
      geometryType = QStringLiteral( "POLYGON" );
      break;

    case Qgis::WkbType::MultiPoint:
      geometryType = QStringLiteral( "MULTIPOINT" );
      break;

    case Qgis::WkbType::MultiLineString:
      geometryType = QStringLiteral( "MULTILINESTRING" );
      break;

    case Qgis::WkbType::MultiPolygon:
      geometryType = QStringLiteral( "MULTIPOLYGON" );
      break;

    case Qgis::WkbType::GeometryCollection:
      geometryType = QStringLiteral( "GEOMETRYCOLLECTION" );
      break;

    case Qgis::WkbType::Unknown:
      geometryType = QStringLiteral( "GEOMETRY" );
      break;

    case Qgis::WkbType::NoGeometry:
      dim = 0;
      break;

    default:
      dim = 0;
      QgsMessageLog::logMessage( tr( "Unsupported type: " ).arg( QgsWkbTypes::displayString( wkbType ) ),
                                 tr( "Redshift Spatial" ) );
      return;
  }

  if ( QgsWkbTypes::hasZ( wkbType ) && QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1String( "ZM" );
    dim = 4;
  }
  else if ( QgsWkbTypes::hasZ( wkbType ) )
  {
    geometryType += QLatin1String( "Z" );
    dim = 3;
  }
  else if ( QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1String( "M" );
    dim = 3;
  }
  else if ( wkbType >= Qgis::WkbType::Point25D && wkbType <= Qgis::WkbType::MultiPolygon25D )
  {
    geometryType = "";
    dim = 0;
    QgsMessageLog::logMessage( tr( "Unsupported type: " ).arg( QgsWkbTypes::displayString( wkbType ) ),
                               tr( "Redshift Spatial" ) );
  }
}

QString QgsRedshiftConn::redshiftWkbTypeName( Qgis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  redshiftWkbType( wkbType, geometryType, dim );

  return geometryType;
}

QString QgsRedshiftConn::spatialTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry )
{
  geomCol = quotedIdentifier( geomCol );
  if ( castToGeometry )
    geomCol += QLatin1String( "::geometry" );

  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      return QStringLiteral( "upper(geometrytype(%1)) IN "
                             "('POINT','POINTZ','POINTM','POINTZM','MULTIPOINT','"
                             "MULTIPOINTZ','MULTIPOINTM','MULTIPOINTZM')" )
             .arg( geomCol );
    case Qgis::GeometryType::Line:
      return QStringLiteral( "upper(geometrytype(%1)) IN "
                             "('LINESTRING','LINESTRINGZ','LINESTRINGM','"
                             "LINESTRINGZM','MULTILINESTRING','MULTILINESTRINGZ','"
                             "MULTILINESTRINGM','MULTILINESTRINGZM')" )
             .arg( geomCol );
    case Qgis::GeometryType::Polygon:
      return QStringLiteral( "upper(geometrytype(%1)) IN "
                             "('POLYGON','POLYGONZ','POLYGONM','POLYGONZM','"
                             "MULTIPOLYGON','MULTIPOLYGONZ','MULTIPOLYGONM','"
                             "MULTIPOLYGONZM','MULTIPOLYGONM')" )
             .arg( geomCol );
    case Qgis::GeometryType::Null:
      return QStringLiteral( "geometrytype(%1) IS NULL" ).arg( geomCol );
    default: // unknown geometry
      return QString();
  }
}

QString QgsRedshiftConn::displayStringForWkbType( Qgis::WkbType type )
{
  return QgsWkbTypes::displayString( Qgis::WkbType( type ) );
}

QString QgsRedshiftConn::displayStringForGeomType( QgsRedshiftGeometryColumnType type )
{
  switch ( type )
  {
    case SctNone:
      return tr( "None" );
    case SctGeometry:
      return tr( "Geometry" );
    case SctGeography:
      return tr( "Geography" );
  }

  Q_ASSERT( !"unexpected geometry column type" );
  return QString();
}

Qgis::WkbType QgsRedshiftConn::wkbTypeFromGeomType( Qgis::GeometryType geomType )
{
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      return Qgis::WkbType::Point;
    case Qgis::GeometryType::Line:
      return Qgis::WkbType::LineString;
    case Qgis::GeometryType::Polygon:
      return Qgis::WkbType::Polygon;
    case Qgis::GeometryType::Null:
      return Qgis::WkbType::NoGeometry;
    case Qgis::GeometryType::Unknown:
      return Qgis::WkbType::Unknown;
  }

  Q_ASSERT( !"unexpected geomType" );
  return Qgis::WkbType::Unknown;
}

QStringList QgsRedshiftConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "Redshift/connections" ) );
  return settings.childGroups();
}

QString QgsRedshiftConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "Redshift/connections/selected" ) ).toString();
}

void QgsRedshiftConn::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( QStringLiteral( "Redshift/connections/selected" ), name );
}

QgsDataSourceUri QgsRedshiftConn::connUri( const QString &connName )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 2 );

  QgsSettings settings;

  QString key = "/Redshift/connections/" + connName;

  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = QStringLiteral( "5439" );
  }
  QString database = settings.value( key + "/database" ).toString();

  bool estimatedMetadata = useEstimatedMetadata( connName );
  QgsDataSourceUri::SslMode sslmode = settings.enumValue( key + "/sslmode", QgsDataSourceUri::SslPrefer );

  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
  {
    password = settings.value( key + "/password" ).toString();
  }

  QString authcfg = settings.value( key + "/authcfg" ).toString();

  QgsDataSourceUri uri;

  uri.setConnection( host, port, database, username, password, sslmode, authcfg );

  uri.setUseEstimatedMetadata( estimatedMetadata );

  return uri;
}

bool QgsRedshiftConn::publicSchemaOnly( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Redshift/connections/" + connName + "/publicOnly", false ).toBool();
}

bool QgsRedshiftConn::dontResolveType( const QString &connName )
{
  QgsSettings settings;

  return settings.value( "/Redshift/connections/" + connName + "/dontResolveType", false ).toBool();
}

bool QgsRedshiftConn::useEstimatedMetadata( const QString &connName )
{
  QgsSettings settings;

  return settings.value( "/Redshift/connections/" + connName + "/estimatedMetadata", false ).toBool();
}

bool QgsRedshiftConn::allowGeometrylessTables( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Redshift/connections/" + connName + "/allowGeometrylessTables", false ).toBool();
}

bool QgsRedshiftConn::allowProjectsInDatabase( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Redshift/connections/" + connName + "/projectsInDatabase", false ).toBool();
}

void QgsRedshiftConn::deleteConnection( const QString &connName )
{
  QgsSettings settings;

  QString key = "/Redshift/connections/" + connName;
  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/sslmode" );
  settings.remove( key + "/publicOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/estimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/authcfg" );
  settings.remove( key + "/keys" );
  settings.remove( key );
}

bool QgsRedshiftConn::cancel()
{
  QMutexLocker locker( &mLock );
  PGcancel *c = ::PQgetCancel( mConn );
  if ( !c )
  {
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( tr( "PQgetCancel failed" ) ),
                               tr( "Redshift Spatial" ) );
    return false;
  }

  char errbuf[256];
  int res = ::PQcancel( c, errbuf, sizeof errbuf );
  ::PQfreeCancel( c );

  if ( !res )
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( errbuf ), tr( "Redshift Spatial" ) );

  return res == 0;
}

QString QgsRedshiftConn::currentDatabase() const
{
  return mDatabaseName;
}
