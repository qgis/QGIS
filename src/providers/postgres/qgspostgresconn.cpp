/***************************************************************************
  qgspostgresconn.cpp  -  connection class to PostgreSQL/PostGIS
                             -------------------
    begin                : 2011/01/28
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresconn.h"

#include <climits>
#include <memory>
#include <nlohmann/json.hpp>

#include "qgsapplication.h"
#include "qgscredentials.h"
#include "qgsdatasourceuri.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresstringutils.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"
#include "qgsvectordataprovider.h"
#include "qgswkbtypes.h"

#include <QApplication>
#include <QFile>
#include <QStringList>
#include <QThread>

#include "moc_qgspostgresconn.cpp"

// for htonl
#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

const int PG_DEFAULT_TIMEOUT = 30;

static QString quotedString( const QString &v )
{
  QString result = v;
  result.replace( '\'', "''"_L1 );
  if ( result.contains( '\\' ) )
    return result.replace( '\\', "\\\\"_L1 ).prepend( "E'" ).append( '\'' );
  else
    return result.prepend( '\'' ).append( '\'' );
}

QgsPostgresResult::~QgsPostgresResult()
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = nullptr;
}

QgsPostgresResult &QgsPostgresResult::operator=( PGresult *result )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = result;
  return *this;
}

QgsPostgresResult &QgsPostgresResult::operator=( const QgsPostgresResult &src )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = src.result();
  return *this;
}

ExecStatusType QgsPostgresResult::PQresultStatus()
{
  return mRes ? ::PQresultStatus( mRes ) : PGRES_FATAL_ERROR;
}

QString QgsPostgresResult::PQresultErrorMessage()
{
  return mRes ? QString::fromUtf8( ::PQresultErrorMessage( mRes ) ) : QObject::tr( "no result buffer" );
}

int QgsPostgresResult::PQntuples()
{
  Q_ASSERT( mRes );
  return ::PQntuples( mRes );
}

QString QgsPostgresResult::PQgetvalue( int row, int col )
{
  Q_ASSERT( mRes );
  return PQgetisnull( row, col )
           ? QString()
           : QString::fromUtf8( ::PQgetvalue( mRes, row, col ) );
}

bool QgsPostgresResult::PQgetisnull( int row, int col )
{
  Q_ASSERT( mRes );
  return ::PQgetisnull( mRes, row, col );
}

int QgsPostgresResult::PQnfields()
{
  Q_ASSERT( mRes );
  return ::PQnfields( mRes );
}

QString QgsPostgresResult::PQfname( int col )
{
  Q_ASSERT( mRes );
  return QString::fromUtf8( ::PQfname( mRes, col ) );
}

Oid QgsPostgresResult::PQftable( int col )
{
  Q_ASSERT( mRes );
  return ::PQftable( mRes, col );
}

int QgsPostgresResult::PQftablecol( int col )
{
  Q_ASSERT( mRes );
  return ::PQftablecol( mRes, col );
}

Oid QgsPostgresResult::PQftype( int col )
{
  Q_ASSERT( mRes );
  return ::PQftype( mRes, col );
}

int QgsPostgresResult::PQfmod( int col )
{
  Q_ASSERT( mRes );
  return ::PQfmod( mRes, col );
}

Oid QgsPostgresResult::PQoidValue()
{
  Q_ASSERT( mRes );
  return ::PQoidValue( mRes );
}

QgsPoolPostgresConn::QgsPoolPostgresConn( const QString &connInfo )
  : mPgConn( QgsPostgresConnPool::instance()->acquireConnection( connInfo ) )
{
}

QgsPoolPostgresConn::~QgsPoolPostgresConn()
{
  if ( mPgConn )
    QgsPostgresConnPool::instance()->releaseConnection( mPgConn );
}


QMap<QString, QgsPostgresConn *> QgsPostgresConn::sConnectionsRO;
QMap<QString, QgsPostgresConn *> QgsPostgresConn::sConnectionsRW;

const int QgsPostgresConn::GEOM_TYPE_SELECT_LIMIT = 100;

QgsPostgresConn *QgsPostgresConn::connectDb( const QString &conninfo, bool readonly, bool shared, bool transaction, bool allowRequestCredentials )
{
  QMap<QString, QgsPostgresConn *> &connections = readonly ? QgsPostgresConn::sConnectionsRO : QgsPostgresConn::sConnectionsRW;

  // This is called from may places where shared parameter cannot be forced to false (QgsVectorLayerExporter)
  // and which is run in a different thread (drag and drop in browser)
  if ( QApplication::instance()->thread() != QThread::currentThread() )
  {
    // sharing connection between threads is not safe
    // See https://github.com/qgis/QGIS/issues/21205
    QgsDebugMsgLevel( u"refusing to use shared connection as we are not the main thread"_s, 2 );
    shared = false;
  }

  QgsPostgresConn *conn;

  if ( shared )
  {
    QMap<QString, QgsPostgresConn *>::iterator it = connections.find( conninfo );
    if ( it != connections.end() )
    {
      conn = *it;
      QgsDebugMsgLevel(
        QStringLiteral(
          "Using cached (%3) connection for %1 (%2)"
        )
          .arg( conninfo )
          .arg( reinterpret_cast<std::uintptr_t>( conn ) )
          .arg( readonly ? "readonly" : "read-write" ),
        2
      );
      conn->mRef++;
      return conn;
    }
    QgsDebugMsgLevel(
      QStringLiteral(
        "Cached (%2) connection for %1 not found"
      )
        .arg( conninfo )
        .arg( readonly ? "readonly" : "read-write" ),
      2
    );
  }

  conn = new QgsPostgresConn( conninfo, readonly, shared, transaction, allowRequestCredentials );
  QgsDebugMsgLevel(
    QStringLiteral(
      "Created new (%4) connection %2 for %1%3"
    )
      .arg( conninfo )
      .arg( reinterpret_cast<std::uintptr_t>( conn ) )
      .arg( shared ? " (shared)" : "" )
      .arg( readonly ? "readonly" : "read-write" ),
    2
  );

  // mRef will be set to 0 when the connection fails
  if ( conn->mRef == 0 )
  {
    QgsDebugMsgLevel(
      QStringLiteral(
        "New (%3) connection %2 failed for conninfo %1"
      )
        .arg( conninfo )
        .arg( reinterpret_cast<std::uintptr_t>( conn ) )
        .arg( readonly ? "readonly" : "read-write" ),
      2
    );
    delete conn;
    return nullptr;
  }

  if ( shared )
  {
    connections.insert( conninfo, conn );
    QgsDebugMsgLevel(
      QStringLiteral(
        "Added connection %2 (for %1) in (%3) cache"
      )
        .arg( conninfo )
        .arg( reinterpret_cast<std::uintptr_t>( conn ) )
        .arg( readonly ? "readonly" : "read-write" ),
      2
    );
  }

  return conn;
}

QgsPostgresConn *QgsPostgresConn::connectDb( const QgsDataSourceUri &uri, bool readonly, bool shared, bool transaction, bool allowRequestCredentials )
{
  return QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), readonly, shared, transaction, allowRequestCredentials );
}

static void noticeProcessor( void *arg, const char *message )
{
  Q_UNUSED( arg )
  QString msg( QString::fromUtf8( message ) );
  msg.chop( 1 );
  QgsMessageLog::logMessage( QObject::tr( "NOTICE: %1" ).arg( msg ), QObject::tr( "PostGIS" ) );
}

QAtomicInt QgsPostgresConn::sNextCursorId = 0;

QgsPostgresConn::QgsPostgresConn( const QString &conninfo, bool readOnly, bool shared, bool transaction, bool allowRequestCredentials )
  : mConnInfo( conninfo )
  , mUri( conninfo )
  , mReadOnly( readOnly )
  , mShared( shared )
  , mTransaction( transaction )
{
  QgsDebugMsgLevel( u"New PostgreSQL connection for "_s + conninfo, 2 );

  // expand connectionInfo
  QString expandedConnectionInfo = mUri.connectionInfo( true );

  auto addDefaultTimeoutAndClientEncoding = []( QString &connectString ) {
    if ( !connectString.contains( u"connect_timeout="_s ) )
    {
      // add default timeout
      QgsSettings settings;
      int timeout = settings.value( u"PostgreSQL/default_timeout"_s, PG_DEFAULT_TIMEOUT, QgsSettings::Providers ).toInt();
      connectString += u" connect_timeout=%1"_s.arg( timeout );
    }

    connectString += " client_encoding='UTF-8'"_L1;
  };
  addDefaultTimeoutAndClientEncoding( expandedConnectionInfo );

  auto logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( u"libpq::PQconnectdb()"_s, expandedConnectionInfo.toUtf8(), u"postgres"_s, u"QgsPostgresConn"_s, QGS_QUERY_LOG_ORIGIN_PG_CON );
  mConn = PQconnectdb( expandedConnectionInfo.toUtf8() );

  // remove temporary cert/key/CA if they exist
  QgsDataSourceUri expandedUri( expandedConnectionInfo );
  QStringList parameters;
  parameters << u"sslcert"_s << u"sslkey"_s << u"sslrootcert"_s;
  const auto constParameters = parameters;
  for ( const QString &param : constParameters )
  {
    if ( expandedUri.hasParam( param ) )
    {
      QString fileName = expandedUri.param( param );
      fileName.remove( u"'"_s );
      QFile file( fileName );
      // set minimal permission to allow removing on Win.
      // On linux and Mac if file is set with QFile::ReadUser
      // does not create problem removing certs
      if ( !file.setPermissions( QFile::WriteOwner ) )
      {
        QString errorMsg = tr( "Cannot set WriteOwner permission to cert: %0 to allow removing it" ).arg( file.fileName() );
        PQfinish();
        logWrapper->setError( errorMsg );
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "PostGIS" ) );
        mRef = 0;
        return;
      }
      if ( !file.remove() )
      {
        QString errorMsg = tr( "Cannot remove cert: %0" ).arg( file.fileName() );
        PQfinish();
        logWrapper->setError( errorMsg );
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "PostGIS" ) );
        mRef = 0;
        return;
      }
    }
  }

  // check the connection status
  if ( PQstatus() != CONNECTION_OK )
  {
    QString username = mUri.username();
    QString password = mUri.password();

    QgsCredentials::instance()->lock();

    int i = 0;
    while ( PQstatus() != CONNECTION_OK && i < 5 )
    {
      ++i;
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, PQerrorMessage(), allowRequestCredentials );
      if ( !ok )
      {
        break;
      }

      const QString errorMsg = PQerrorMessage();
      PQfinish();
      logWrapper->setError( errorMsg );

      if ( !username.isEmpty() )
        mUri.setUsername( username );

      if ( !password.isEmpty() )
        mUri.setPassword( password );

      QgsDebugMsgLevel( "Connecting to " + QgsPostgresConn::connectionInfo( mUri, false ), 2 );
      QString connectString = QgsPostgresConn::connectionInfo( mUri );
      addDefaultTimeoutAndClientEncoding( connectString );
      // use conninfo for log, connectString - can contain clear text username & password
      logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( u"libpq::PQconnectdb()"_s, conninfo, u"postgres"_s, u"QgsPostgresConn"_s, QGS_QUERY_LOG_ORIGIN_PG_CON );
      mConn = PQconnectdb( connectString.toUtf8() );
    }

    if ( PQstatus() == CONNECTION_OK )
      QgsCredentials::instance()->put( conninfo, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( PQstatus() != CONNECTION_OK )
  {
    QString errorMsg = PQerrorMessage();
    PQfinish();
    logWrapper->setError( errorMsg );
    QgsMessageLog::logMessage( tr( "Connection to database failed" ) + '\n' + errorMsg, tr( "PostGIS" ) );
    mRef = 0;
    return;
  }

  const QString sessionRoleKey = u"session_role"_s;
  if ( mUri.hasParam( sessionRoleKey ) )
  {
    const QString sessionRole = mUri.param( sessionRoleKey );
    if ( !sessionRole.isEmpty() )
    {
      if ( !setSessionRole( sessionRole ) )
      {
        QgsDebugMsgLevel(
          QStringLiteral(
            "Set session role failed for ROLE %1"
          )
            .arg( quotedValue( sessionRole ) ),
          2
        );

        mRef = 0;
        return;
      }
    }
  }

  logWrapper = nullptr;
  QgsDebugMsgLevel( u"Connection to the database was successful"_s, 2 );

  mPostgresqlVersion = PQserverVersion( mConn );
  if ( mPostgresqlVersion < 70400 )
  {
    deduceEndian();
  }
  else
  {
    // todo - use std::endian when we require C++20
    int testInt = 1;
    //               mem addr: -->
    // testInt(little-endian): 01 0...0 00
    //    testInt(big-endian): 00 0...0 01
    //                  *char: ^^
    mSwapEndian = *( char * ) &testInt == 1;
  }

  /* Check to see if we have working PostGIS support */
  if ( !postgisVersion().isNull() )
  {
    /* Check to see if we have GEOS support and if not, warn the user about
       the problems they will see :) */
    QgsDebugMsgLevel( u"Checking for GEOS support"_s, 3 );

    if ( !hasGEOS() )
    {
      QgsMessageLog::logMessage( tr( "Your PostGIS installation has no GEOS support. Feature selection and identification will not work properly. Please install PostGIS with GEOS support (http://geos.refractions.net)" ), tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugMsgLevel( u"GEOS support available!"_s, 3 );
    }
  }

  if ( mPostgresqlVersion >= 90000 )
  {
    // Quoting floating point values, application name for PostgreSQL connection, etc in 1 request
    QString sql;
    sql += "SET extra_float_digits=3;"_L1;
    sql += u"SET application_name=%1;"_s.arg( quotedValue( QgsApplication::applicationFullName() ) );
    sql += "SET datestyle='ISO';"_L1;

    // Set the PostgreSQL message level so that we don't get the
    // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
    sql += "SET client_min_messages to error;"_L1;
#endif

    LoggedPQexecNR( "QgsPostgresConn", sql );
  }

  PQsetNoticeProcessor( mConn, noticeProcessor, nullptr );
}

QgsPostgresConn::~QgsPostgresConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mConn )
    ::PQfinish( mConn );
  mConn = nullptr;
}

void QgsPostgresConn::ref()
{
  QMutexLocker locker( &mLock );
  ++mRef;
}

void QgsPostgresConn::unref()
{
  QMutexLocker locker( &mLock );
  if ( --mRef > 0 )
    return;

  if ( mShared )
  {
    QMap<QString, QgsPostgresConn *> &connections = mReadOnly ? sConnectionsRO : sConnectionsRW;

    int removed = connections.remove( mConnInfo );
    Q_ASSERT( removed == 1 );

    QgsDebugMsgLevel(
      QStringLiteral(
        "Cached (%1) connection for %2 (%3) removed"
      )
        .arg( mReadOnly ? "readonly" : "read-write" )
        .arg( mConnInfo )
        .arg( reinterpret_cast<std::uintptr_t>( this ) ),
      2
    );
  }

  // to avoid destroying locked mutex
  locker.unlock();
  delete this;
}

/* private */
QStringList QgsPostgresConn::supportedSpatialTypes() const
{
  QStringList supportedSpatialTypes;

  supportedSpatialTypes << quotedValue( "geometry" )
                        << quotedValue( "geography" );

  if ( hasPointcloud() )
  {
    supportedSpatialTypes << quotedValue( "pcpatch" );
    supportedSpatialTypes << quotedValue( "pcpoint" );
  }

  if ( hasRaster() )
    supportedSpatialTypes << quotedValue( "raster" );

  if ( hasTopology() )
    supportedSpatialTypes << quotedValue( "topogeometry" );

  return supportedSpatialTypes;
}

/* private */
// TODO: deprecate this function
void QgsPostgresConn::addColumnInfo( QgsPostgresLayerProperty &layerProperty, const QString &schemaName, const QString &viewName, bool fetchPkCandidates )
{
  // TODO: optimize this query when pk candidates aren't needed
  //       could use array_agg() and count()
  //       array output would look like this: "{One,tWo}"
  QString sql = u"SELECT attname, CASE WHEN typname in (%1) THEN 1 ELSE null END AS isSpatial FROM pg_attribute JOIN pg_type ON atttypid=pg_type.oid WHERE attrelid=regclass('%2.%3') AND NOT attisdropped AND attnum>0 ORDER BY attnum"_s
                  .arg( supportedSpatialTypes().join( ',' ) )
                  .arg( quotedIdentifier( schemaName ), quotedIdentifier( viewName ) );
  QgsDebugMsgLevel( "getting column info: " + sql, 2 );
  QgsPostgresResult colRes( LoggedPQexec( "QgsPostgresConn", sql ) );

  layerProperty.pkCols.clear();
  layerProperty.nSpCols = 0;

  if ( colRes.PQresultStatus() == PGRES_TUPLES_OK )
  {
    for ( int i = 0; i < colRes.PQntuples(); i++ )
    {
      if ( fetchPkCandidates )
      {
        layerProperty.pkCols << colRes.PQgetvalue( i, 0 );
      }

      if ( colRes.PQgetisnull( i, 1 ) == 0 )
      {
        ++layerProperty.nSpCols;
      }
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "SQL: %1\nresult: %2\nerror: %3\n" ).arg( sql ).arg( colRes.PQresultStatus() ).arg( colRes.PQresultErrorMessage() ), tr( "PostGIS" ) );
  }
}

bool QgsPostgresConn::getTableInfo( bool searchGeometryColumnsOnly, bool allowGeometrylessTables, bool allowRasterOverviewTables, const QString &schema, const QString &name )
{
  QMutexLocker locker( &mLock );
  int nColumns = 0;
  int foundInTables = 0;
  QgsPostgresResult result;
  QString query;

  for ( int i = SctGeometry; i <= SctRaster; ++i )
  {
    QString sql, tableName, schemaName, columnName, typeName, sridName, gtableName, dimName;

    if ( i == SctGeometry )
    {
      tableName = u"l.f_table_name"_s;
      schemaName = u"l.f_table_schema"_s;
      columnName = u"l.f_geometry_column"_s;
      typeName = u"upper(l.type)"_s;
      sridName = u"l.srid"_s;
      dimName = u"l.coord_dimension"_s;
      gtableName = u"geometry_columns"_s;
    }
    // Geography since postgis 1.5
    else if ( i == SctGeography
              && ( mPostgisVersionMajor >= 2 || ( mPostgisVersionMajor == 1 && mPostgisVersionMinor >= 5 ) ) )
    {
      tableName = u"l.f_table_name"_s;
      schemaName = u"l.f_table_schema"_s;
      columnName = u"l.f_geography_column"_s;
      typeName = u"upper(l.type)"_s;
      sridName = u"l.srid"_s;
      dimName = u"2"_s;
      gtableName = u"geography_columns"_s;
    }
    else if ( i == SctTopoGeometry )
    {
      if ( !hasTopology() )
        continue;

      schemaName = u"l.schema_name"_s;
      tableName = u"l.table_name"_s;
      columnName = u"l.feature_column"_s;
      typeName = "CASE "
                 "WHEN l.feature_type = 1 THEN 'MULTIPOINT' "
                 "WHEN l.feature_type = 2 THEN 'MULTILINESTRING' "
                 "WHEN l.feature_type = 3 THEN 'MULTIPOLYGON' "
                 "WHEN l.feature_type = 4 THEN 'GEOMETRYCOLLECTION' "
                 "END AS type";
      sridName = u"(SELECT srid FROM topology.topology t WHERE l.topology_id=t.id)"_s;
      dimName = u"2"_s;
      gtableName = u"topology.layer"_s;
    }
    else if ( i == SctPcPatch )
    {
      if ( !hasPointcloud() )
        continue;

      tableName = u"l.\"table\""_s;
      schemaName = u"l.\"schema\""_s;
      columnName = u"l.\"column\""_s;
      typeName = u"'POLYGON'"_s;
      sridName = u"l.srid"_s;
      dimName = u"2"_s;
      gtableName = u"pointcloud_columns"_s;
    }
    else if ( i == SctRaster )
    {
      if ( !hasRaster() )
        continue;

      tableName = u"l.\"r_table_name\""_s;
      schemaName = u"l.\"r_table_schema\""_s;
      columnName = u"l.\"r_raster_column\""_s;
      typeName = u"'RASTER'"_s;
      sridName = u"l.srid"_s;
      dimName = u"2"_s;
      gtableName = u"raster_columns"_s;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Unsupported spatial column type %1" )
                                   .arg( displayStringForGeomType( ( QgsPostgresGeometryColumnType ) i ) ) );
      continue;
    }

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.
    sql = QString( "SELECT %1,%2,%3,%4,%5,%6,c.relkind,obj_description(c.oid),"
                   "%10, "
                   "count(CASE WHEN t.typname IN (%9) THEN 1 ELSE NULL END) "
                   ", %8 "
                   " FROM %7 l,pg_class c,pg_namespace n,pg_attribute a,pg_type t"
                   " WHERE c.relname=%1"
                   " AND %2=n.nspname"
                   " AND NOT a.attisdropped"
                   " AND a.attrelid=c.oid"
                   " AND a.atttypid=t.oid"
                   " AND a.attnum>0"
                   " AND n.oid=c.relnamespace"
                   " AND has_schema_privilege(n.nspname,'usage')"
                   " AND has_table_privilege(c.oid,'select')" // user has select privilege
    )
            .arg( tableName, schemaName, columnName, typeName, sridName, dimName, gtableName )
            .arg( i )
            .arg( supportedSpatialTypes().join( ',' ) )
            .arg( mPostgresqlVersion >= 90000 ? "array_agg(a.attname ORDER BY a.attnum)" : "(SELECT array_agg(attname) FROM (SELECT unnest(array_agg(a.attname)) AS attname ORDER BY unnest(array_agg(a.attnum))) AS attname)" );

    if ( !schema.isEmpty() )
      sql += u" AND %1=%2"_s.arg( schemaName, quotedString( schema ) );

    if ( !name.isEmpty() )
      sql += u" AND %1=%2"_s.arg( tableName, quotedString( name ) );

    sql += QString( " GROUP BY 1,2,3,4,5,6,7,c.oid,11" );

    foundInTables |= 1 << i;

    if ( !query.isEmpty() )
      query += " UNION ";

    query += sql;
  }

  query += " ORDER BY 2,1,3"_L1;


  QgsDebugMsgLevel( "getting table info from layer registries: " + query, 2 );
  result = LoggedPQexec( "QgsPostgresConn", query );
  // NOTE: we intentionally continue if the query fails
  //       (for example because PostGIS is not installed)

  if ( !result.result() )
  {
    return false;
  }

  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    QString tableName = result.PQgetvalue( idx, 0 );
    QString schemaName = result.PQgetvalue( idx, 1 );
    QString column = result.PQgetvalue( idx, 2 );
    QString type = result.PQgetvalue( idx, 3 );
    QString ssrid = result.PQgetvalue( idx, 4 );
    int dim = result.PQgetvalue( idx, 5 ).toInt();
    QString relkind = result.PQgetvalue( idx, 6 );
    bool isRaster = type == "RASTER"_L1;
    QString comment = result.PQgetvalue( idx, 7 );
    QString attributes = result.PQgetvalue( idx, 8 );
    int nSpCols = result.PQgetvalue( idx, 9 ).toInt();
    QgsPostgresGeometryColumnType columnType = SctNone;

    int columnTypeInt = result.PQgetvalue( idx, 10 ).toInt();
    if ( columnTypeInt == SctGeometry )
      columnType = SctGeometry;
    else if ( columnTypeInt == SctGeography )
      columnType = SctGeography;
    else if ( columnTypeInt == SctTopoGeometry )
      columnType = SctTopoGeometry;
    else if ( columnTypeInt == SctPcPatch )
      columnType = SctPcPatch;
    else if ( columnTypeInt == SctRaster )
      columnType = SctRaster;
    else
    {
      QgsDebugError( u"Unhandled columnType index %1"_s
                       .arg( columnTypeInt ) );
    }

    int srid = ssrid.isEmpty() ? std::numeric_limits<int>::min() : ssrid.toInt();

    if ( !isRaster && majorVersion() >= 2 && srid == 0 )
    {
      // 0 doesn't constraint => detect
      srid = std::numeric_limits<int>::min();
    }

#if 0
    QgsDebugMsgLevel( u"%1 : %2.%3.%4: %5 %6 %7 %8"_s
                      .arg( gtableName )
                      .arg( schemaName ).arg( tableName ).arg( column )
                      .arg( type )
                      .arg( srid )
                      .arg( relkind )
                      .arg( dim ), 2 );
#endif

    QgsPostgresLayerProperty layerProperty;
    layerProperty.schemaName = schemaName;
    layerProperty.tableName = tableName;
    layerProperty.geometryColName = column;
    layerProperty.geometryColType = columnType;
    if ( dim == 3 && !type.endsWith( 'M' ) )
      type += QLatin1Char( 'Z' );
    else if ( dim == 4 )
      type += "ZM"_L1;
    layerProperty.types = QList<Qgis::WkbType>() << ( QgsPostgresConn::wkbTypeFromPostgis( type ) );
    layerProperty.srids = QList<int>() << srid;
    layerProperty.sql.clear();
    layerProperty.relKind = relKindFromValue( relkind );
    layerProperty.isRaster = isRaster;
    layerProperty.tableComment = comment;
    layerProperty.nSpCols = nSpCols;
    if ( ( layerProperty.relKind == Qgis::PostgresRelKind::View )
         || ( layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView )
         || ( layerProperty.relKind == Qgis::PostgresRelKind::ForeignTable ) )
    {
      // TODO: use std::transform
      for ( const auto &a : QgsPostgresStringUtils::parseArray( attributes ) )
      {
        layerProperty.pkCols << a.toString();
      }
    }

    if ( ( layerProperty.relKind == Qgis::PostgresRelKind::View
           || layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView )
         && layerProperty.pkCols.empty() )
    {
      //QgsDebugMsgLevel( u"no key columns found."_s, 2 );
      continue;
    }

    mLayersSupported << layerProperty;
    nColumns++;
  }

  //search for geometry columns in tables that are not in the geometry_columns metatable
  if ( !searchGeometryColumnsOnly )
  {
    // Now have a look for spatial columns that aren't in the geometry_columns table.
    QString sql = QStringLiteral( "SELECT"
                                  " c.relname"
                                  ",n.nspname"
                                  ",a.attname"
                                  ",c.relkind"
                                  ",CASE WHEN t.typname IN (%1) THEN t.typname ELSE b.typname END AS coltype"
                                  ",obj_description(c.oid)"
                                  " FROM pg_attribute a"
                                  " JOIN pg_class c ON c.oid=a.attrelid"
                                  " JOIN pg_namespace n ON n.oid=c.relnamespace"
                                  " JOIN pg_type t ON t.oid=a.atttypid"
                                  " LEFT JOIN pg_type b ON b.oid=t.typbasetype"
                                  " WHERE c.relkind IN ('v','r','m','p','f')"
                                  " AND has_schema_privilege( n.nspname, 'usage' )"
                                  " AND has_table_privilege( c.oid, 'select' )"
                                  " AND (t.typname IN (%1) OR b.typname IN (%1))" )
                    .arg( supportedSpatialTypes().join( ',' ) );

    // user has select privilege
    if ( !schema.isEmpty() )
      sql += u" AND n.nspname=%1"_s.arg( quotedString( schema ) );

    if ( !name.isEmpty() )
      sql += u" AND c.relname=%1"_s.arg( quotedString( name ) );

    // skip columns of which we already derived information from the metadata tables
    if ( nColumns > 0 )
    {
      if ( foundInTables & ( 1 << SctGeometry ) )
      {
        sql += " AND NOT EXISTS (SELECT 1 FROM geometry_columns WHERE n.nspname=f_table_schema AND c.relname=f_table_name AND a.attname=f_geometry_column)"_L1;
      }

      if ( foundInTables & ( 1 << SctGeography ) )
      {
        sql += " AND NOT EXISTS (SELECT 1 FROM geography_columns WHERE n.nspname=f_table_schema AND c.relname=f_table_name AND a.attname=f_geography_column)"_L1;
      }

      if ( foundInTables & ( 1 << SctPcPatch ) )
      {
        sql += " AND NOT EXISTS (SELECT 1 FROM pointcloud_columns WHERE n.nspname=\"schema\" AND c.relname=\"table\" AND a.attname=\"column\")"_L1;
      }

      if ( foundInTables & ( 1 << SctRaster ) )
      {
        sql += " AND NOT EXISTS (SELECT 1 FROM raster_columns WHERE n.nspname=\"r_table_schema\" AND c.relname=\"r_table_name\" AND a.attname=\"r_raster_column\")"_L1;
      }

      if ( foundInTables & ( 1 << SctTopoGeometry ) )
      {
        sql += " AND NOT EXISTS (SELECT 1 FROM topology.layer WHERE n.nspname=\"schema_name\" AND c.relname=\"table_name\" AND a.attname=\"feature_column\")"_L1;
      }
    }

    QgsDebugMsgLevel( "getting spatial table info from pg_catalog: " + sql, 2 );


    result = LoggedPQexec( u"QgsPostgresConn"_s, sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined. The error message from the database was:\n%1\n" ).arg( result.PQresultErrorMessage() ), tr( "PostGIS" ) );
      LoggedPQexecNR( "QgsPostgresConn", u"COMMIT"_s );
      return false;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      // Have the column name, schema name and the table name. The concept of a
      // catalog doesn't exist in PostgreSQL so we ignore that, but we
      // do need to get the geometry type.

      QString tableName = result.PQgetvalue( i, 0 );  // relname
      QString schemaName = result.PQgetvalue( i, 1 ); // nspname
      QString column = result.PQgetvalue( i, 2 );     // attname
      QString relkind = result.PQgetvalue( i, 3 );    // relation kind
      QString coltype = result.PQgetvalue( i, 4 );    // column type
      QString comment = result.PQgetvalue( i, 5 );    // table comment

      QgsPostgresLayerProperty layerProperty;
      layerProperty.types = QList<Qgis::WkbType>() << Qgis::WkbType::Unknown;
      layerProperty.srids = QList<int>() << std::numeric_limits<int>::min();
      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = column;
      layerProperty.relKind = relKindFromValue( relkind );
      layerProperty.isRaster = coltype == "raster"_L1;
      layerProperty.tableComment = comment;
      if ( coltype == "geometry"_L1 )
      {
        layerProperty.geometryColType = SctGeometry;
      }
      else if ( coltype == "geography"_L1 )
      {
        layerProperty.geometryColType = SctGeography;
      }
      else if ( coltype == "topogeometry"_L1 )
      {
        layerProperty.geometryColType = SctTopoGeometry;
      }
      else if ( coltype == "pcpatch"_L1 || coltype == "pcpoint"_L1 )
      {
        layerProperty.geometryColType = SctPcPatch;
      }
      else if ( coltype == "raster"_L1 )
      {
        layerProperty.geometryColType = SctRaster;
      }
      else
      {
        Q_ASSERT( !"Unknown geometry type" );
      }

      // TODO: use knowledge from already executed query to count
      //       spatial fields and list attribute names...
      addColumnInfo( layerProperty, schemaName, tableName, layerProperty.relKind == Qgis::PostgresRelKind::View || layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView || layerProperty.relKind == Qgis::PostgresRelKind::ForeignTable );

      if ( ( layerProperty.relKind == Qgis::PostgresRelKind::View
             || layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView )
           && layerProperty.pkCols.empty() )
      {
        //QgsDebugMsgLevel( u"no key columns found."_s, 2 );
        continue;
      }

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( allowGeometrylessTables )
  {
    QString sql = QStringLiteral( "SELECT "
                                  "pg_class.relname"
                                  ",pg_namespace.nspname"
                                  ",pg_class.relkind"
                                  ",obj_description(pg_class.oid)"
                                  ",%1"
                                  " FROM "
                                  " pg_class"
                                  ",pg_namespace"
                                  ",pg_attribute a"
                                  " WHERE pg_namespace.oid=pg_class.relnamespace"
                                  " AND has_schema_privilege(pg_namespace.nspname,'usage')"
                                  " AND has_table_privilege(pg_class.oid,'select')"
                                  " AND pg_class.relkind IN ('v','r','m','p','f')"
                                  " AND pg_class.oid = a.attrelid"
                                  " AND NOT a.attisdropped"
                                  " AND a.attnum > 0" )
                    .arg( mPostgresqlVersion >= 90000 ? "array_agg(a.attname ORDER BY a.attnum)" : "(SELECT array_agg(attname) FROM (SELECT unnest(array_agg(a.attname)) AS attname ORDER BY unnest(array_agg(a.attnum))) AS attname)" );

    // user has select privilege
    if ( !schema.isEmpty() )
      sql += u" AND pg_namespace.nspname=%1"_s.arg( quotedString( schema ) );

    if ( !name.isEmpty() )
      sql += u" AND pg_class.relname=%1"_s.arg( quotedString( name ) );


    sql += " GROUP BY 1,2,3,4"_L1;

    QgsDebugMsgLevel( "getting non-spatial table info: " + sql, 2 );

    result = LoggedPQexec( u"QgsPostgresConn"_s, sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined.\nThe error message from the database was:\n%1" ).arg( result.PQresultErrorMessage() ), tr( "PostGIS" ) );
      return false;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      QString table = result.PQgetvalue( i, 0 );      // relname
      QString schema = result.PQgetvalue( i, 1 );     // nspname
      QString relkind = result.PQgetvalue( i, 2 );    // relation kind
      QString comment = result.PQgetvalue( i, 3 );    // table comment
      QString attributes = result.PQgetvalue( i, 4 ); // attributes array

      QgsPostgresLayerProperty layerProperty;
      layerProperty.types = QList<Qgis::WkbType>() << Qgis::WkbType::NoGeometry;
      layerProperty.srids = QList<int>() << std::numeric_limits<int>::min();
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = QString();
      layerProperty.geometryColType = SctNone;
      layerProperty.nSpCols = 0;
      layerProperty.relKind = relKindFromValue( relkind );
      layerProperty.isRaster = false;
      layerProperty.tableComment = comment;

      //check if we've already added this layer in some form
      bool alreadyFound = false;
      const auto constMLayersSupported = mLayersSupported;
      for ( const QgsPostgresLayerProperty &foundLayer : constMLayersSupported )
      {
        if ( foundLayer.schemaName == schema && foundLayer.tableName == table )
        {
          //already found this table
          alreadyFound = true;
          break;
        }
      }
      if ( alreadyFound )
        continue;

      if ( layerProperty.relKind == Qgis::PostgresRelKind::View
           || layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView
           || layerProperty.relKind == Qgis::PostgresRelKind::ForeignTable )
      {
        // TODO: use std::transform
        for ( const auto &a : QgsPostgresStringUtils::parseArray( attributes ) )
        {
          layerProperty.pkCols << a.toString();
        }
      }

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  // remove raster overivews if allowRasterOverviewTables is FALSE
  if ( !allowRasterOverviewTables )
  {
    QString sqlRasterOverviewExist = QStringLiteral( "SELECT table_schema, table_name"
                                                     " FROM information_schema.views"
                                                     " WHERE table_schema = 'public' AND table_name = 'raster_overviews'" );

    QgsPostgresResult resultRasterOverviewsExist;
    resultRasterOverviewsExist = LoggedPQexec( "QgsPostgresConn", sqlRasterOverviewExist );

    if ( resultRasterOverviewsExist.result() && resultRasterOverviewsExist.PQntuples() > 0 )
    {
      const QString sqlRasterOverviews = u"SELECT o_table_schema, o_table_name FROM public.raster_overviews"_s;

      QgsPostgresResult resultRasterOverviews;
      resultRasterOverviews = LoggedPQexec( "QgsPostgresConn", sqlRasterOverviews );

      if ( resultRasterOverviews.result() && resultRasterOverviews.PQntuples() > 0 )
      {
        QVector<QgsPostgresRasterOverviewLayerProperty> overviews;
        for ( int idx = 0; idx < resultRasterOverviews.PQntuples(); idx++ )
        {
          QgsPostgresRasterOverviewLayerProperty rasterOverviewProperty;
          rasterOverviewProperty.schemaName = resultRasterOverviews.PQgetvalue( idx, 0 );
          rasterOverviewProperty.tableName = resultRasterOverviews.PQgetvalue( idx, 1 );
          overviews.append( rasterOverviewProperty );
        }

        QVector<QgsPostgresLayerProperty> layersToKeep;
        for ( int i = 0; i < mLayersSupported.count(); i++ )
        {
          QgsPostgresLayerProperty property = mLayersSupported.at( i );

          if ( !property.isRaster )
          {
            layersToKeep.append( property );
          }
          else
          {
            bool keepRasterTable = true;
            for ( const QgsPostgresRasterOverviewLayerProperty &overview : std::as_const( overviews ) )
            {
              if ( property.schemaName == overview.schemaName && property.tableName == overview.tableName )
              {
                keepRasterTable = false;
              }
            }

            if ( keepRasterTable )
              layersToKeep.append( property );
          }
        }
        mLayersSupported = layersToKeep;
      }
    }
  }

  if ( nColumns == 0 && schema.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "PostGIS" ) );
  }

  return true;
}

bool QgsPostgresConn::supportedLayersPrivate( QVector<QgsPostgresLayerProperty> &layers, bool searchGeometryColumnsOnly, bool allowGeometrylessTables, bool allowRasterOverviewTables, const QString &schema, const QString &table )
{
  QMutexLocker locker( &mLock );

  mLayersSupported.clear();

  // Get the list of supported tables
  if ( !getTableInfo( searchGeometryColumnsOnly, allowGeometrylessTables, allowRasterOverviewTables, schema, table ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "PostGIS" ) );
    return false;
  }

  layers = mLayersSupported;

  return true;
}

bool QgsPostgresConn::getSchemas( QList<QgsPostgresSchemaProperty> &schemas, const QStringList &restrictToSchemas )
{
  schemas.clear();
  QgsPostgresResult result;

  QString additionalFilter;
  if ( !restrictToSchemas.empty() )
  {
    QStringList schemaIn;
    schemaIn.reserve( restrictToSchemas.size() );
    for ( const QString &schema : restrictToSchemas )
    {
      schemaIn.append( quotedString( schema ) );
    }
    additionalFilter = u"nspname IN (%1)"_s.arg( schemaIn.join( ',' ) );
  }

  const QString sql = u"SELECT nspname, pg_get_userbyid(nspowner), pg_catalog.obj_description(oid) FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema' %1ORDER BY nspname"_s.arg( !additionalFilter.isEmpty() ? u"AND (%1) "_s.arg( additionalFilter ) : QString() );

  result = LoggedPQexec( u"QgsPostgresConn"_s, sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    LoggedPQexecNR( "QgsPostgresConn", u"COMMIT"_s );
    return false;
  }

  const int resultSize = result.PQntuples();
  schemas.reserve( resultSize );
  for ( int idx = 0; idx < resultSize; idx++ )
  {
    QgsPostgresSchemaProperty schema;
    schema.name = result.PQgetvalue( idx, 0 );
    schema.owner = result.PQgetvalue( idx, 1 );
    schema.description = result.PQgetvalue( idx, 2 );
    schemas << schema;
  }
  return true;
}

/**
 * Check to see if GEOS is available
 */
bool QgsPostgresConn::hasGEOS() const
{
  // make sure info is up to date for the current connection
  postgisVersion();
  return mGeosAvailable;
}

/**
 * Check to see if topology is available
 */
bool QgsPostgresConn::hasTopology() const
{
  // make sure info is up to date for the current connection
  postgisVersion();
  return mTopologyAvailable;
}

/**
 * Check to see if pointcloud is available
 */
bool QgsPostgresConn::hasPointcloud() const
{
  // make sure info is up to date for the current connection
  postgisVersion();
  return mPointcloudAvailable;
}

/**
 * Check to see if raster is available
 */
bool QgsPostgresConn::hasRaster() const
{
  // make sure info is up to date for the current connection
  postgisVersion();
  return mRasterAvailable;
}
/* Functions for determining available features in postGIS */
QString QgsPostgresConn::postgisVersion() const
{
  QMutexLocker locker( &mLock );
  if ( mGotPostgisVersion )
    return mPostgisVersionInfo;

  mPostgresqlVersion = PQserverVersion( mConn );

  QgsPostgresResult result( LoggedPQexecNoLogError( u"QgsPostgresConn"_s, u"SELECT postgis_version()"_s ) );
  if ( result.PQntuples() != 1 )
  {
    QgsMessageLog::logMessage( tr( "No PostGIS support in the database." ), tr( "PostGIS" ) );
    mGotPostgisVersion = true;
    return QString();
  }

  mPostgisVersionInfo = result.PQgetvalue( 0, 0 );

  QgsDebugMsgLevel( "PostGIS version info: " + mPostgisVersionInfo, 2 );

  QStringList postgisParts = mPostgisVersionInfo.split( ' ', Qt::SkipEmptyParts );

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split( '.', Qt::SkipEmptyParts );
  if ( postgisVersionParts.size() < 2 )
  {
    QgsMessageLog::logMessage( tr( "Could not parse postgis version string '%1'" ).arg( mPostgisVersionInfo ), tr( "PostGIS" ) );
    return QString();
  }

  mPostgisVersionMajor = postgisVersionParts[0].toInt();
  mPostgisVersionMinor = postgisVersionParts[1].toInt();

  mUseWkbHex = mPostgisVersionMajor < 1;

  // apparently PostGIS 1.5.2 doesn't report capabilities in postgis_version() anymore
  if ( mPostgisVersionMajor > 1 || ( mPostgisVersionMajor == 1 && mPostgisVersionMinor >= 5 ) )
  {
    result = LoggedPQexec( u"QgsPostgresConn"_s, u"SELECT postgis_geos_version(), postgis_proj_version()"_s );
    mGeosAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 );
    mProjAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 1 );
    QgsDebugMsgLevel( u"geos:%1 proj:%2"_s.arg( mGeosAvailable ? result.PQgetvalue( 0, 0 ) : "none" ).arg( mProjAvailable ? result.PQgetvalue( 0, 1 ) : "none" ), 2 );
  }
  else
  {
    // assume no capabilities
    mGeosAvailable = false;

    // parse out the capabilities and store them
    QStringList geos = postgisParts.filter( u"GEOS"_s );
    if ( geos.size() == 1 )
    {
      mGeosAvailable = ( geos[0].indexOf( "=1"_L1 ) > -1 );
    }
  }

  // checking for topology support
  QgsDebugMsgLevel( u"Checking for topology support"_s, 2 );
  mTopologyAvailable = false;
  if ( mPostgisVersionMajor > 1 )
  {
    const QString query = QStringLiteral(
      "SELECT has_schema_privilege(n.oid, 'usage')"
      " AND has_table_privilege(t.oid, 'select')"
      " AND has_table_privilege(l.oid, 'select')"
      " FROM pg_namespace n, pg_class t, pg_class l"
      " WHERE n.nspname = 'topology'"
      " AND t.relnamespace = n.oid"
      " AND l.relnamespace = n.oid"
      " AND t.relname = 'topology'"
      " AND l.relname = 'layer'"
    );
    QgsPostgresResult result( LoggedPQexec( u"QgsPostgresConn"_s, query ) );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == "t"_L1 )
    {
      mTopologyAvailable = true;
    }
  }

  if ( mTopologyAvailable )
  {
    QgsDebugMsgLevel( u"Topology support available :)"_s, 2 );
  }
  else
  {
    QgsDebugMsgLevel( u"Topology support not available :("_s, 2 );
  }

  mGotPostgisVersion = true;

  if ( mPostgresqlVersion >= 90000 )
  {
    QgsDebugMsgLevel( u"Checking for pointcloud support"_s, 2 );
    result = LoggedPQexecNoLogError( u"QgsPostgresConn"_s, QStringLiteral( "SELECT has_table_privilege(c.oid, 'select')"
                                                                           " AND has_table_privilege(f.oid, 'select')"
                                                                           " FROM pg_class c, pg_class f, pg_namespace n, pg_extension e"
                                                                           " WHERE c.relnamespace = n.oid"
                                                                           " AND c.relname = 'pointcloud_columns'"
                                                                           " AND f.relnamespace = n.oid"
                                                                           " AND f.relname = 'pointcloud_formats'"
                                                                           " AND n.oid = e.extnamespace"
                                                                           " AND e.extname = 'pointcloud'" ) );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == "t"_L1 )
    {
      mPointcloudAvailable = true;
      QgsDebugMsgLevel( u"Pointcloud support available!"_s, 2 );
    }
  }

  QgsDebugMsgLevel( u"Checking for raster support"_s, 2 );
  if ( mPostgisVersionMajor >= 2 )
  {
    result = LoggedPQexecNoLogError( u"QgsPostgresConn"_s, QStringLiteral( "SELECT has_table_privilege(c.oid, 'select')"
                                                                           " FROM pg_class c, pg_namespace n, pg_type t"
                                                                           " WHERE c.relnamespace = n.oid"
                                                                           " AND n.oid = t.typnamespace"
                                                                           " AND c.relname = 'raster_columns'"
                                                                           " AND t.typname = 'raster'" ) );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == "t"_L1 )
    {
      mRasterAvailable = true;
      QgsDebugMsgLevel( u"Raster support available!"_s, 2 );
    }
  }

  return mPostgisVersionInfo;
}

bool QgsPostgresConn::setSessionRole( const QString &sessionRole )
{
  return LoggedPQexecNR( "QgsPostgresConn", u"SET ROLE %1"_s.arg( quotedValue( sessionRole ) ) );
}

QString QgsPostgresConn::quotedIdentifier( const QString &ident )
{
  QString result = ident;
  result.replace( '"', "\"\""_L1 );
  return result.prepend( '\"' ).append( '\"' );
}

static QString doubleQuotedMapValue( const QString &v )
{
  QString result = v;
  return "\"" + result.replace( '\\', "\\\\\\\\"_L1 ).replace( '\"', "\\\\\""_L1 ).replace( '\'', "\\'"_L1 ) + "\"";
}

static QString quotedMap( const QVariantMap &map )
{
  QString ret;
  for ( QVariantMap::const_iterator i = map.constBegin(); i != map.constEnd(); ++i )
  {
    if ( !ret.isEmpty() )
    {
      ret += QLatin1Char( ',' );
    }
    ret.append( doubleQuotedMapValue( i.key() ) + "=>" + doubleQuotedMapValue( i.value().toString() ) );
  }
  return "E'" + ret + "'::hstore";
}

static QString quotedList( const QVariantList &list )
{
  QString ret;
  for ( QVariantList::const_iterator i = list.constBegin(); i != list.constEnd(); ++i )
  {
    if ( !ret.isEmpty() )
    {
      ret += QLatin1Char( ',' );
    }

    QString inner = i->toString();
    if ( inner.startsWith( '{' ) || i->userType() == QMetaType::Type::Int || i->userType() == QMetaType::Type::LongLong )
    {
      ret.append( inner );
    }
    else
    {
      ret.append( doubleQuotedMapValue( i->toString() ) );
    }
  }
  return "E'{" + ret + "}'";
}

QString QgsPostgresConn::quotedValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"NULL"_s;

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
      return value.toString();

    case QMetaType::Type::QDateTime:
      return quotedString( value.toDateTime().toString( Qt::ISODateWithMs ) );

    case QMetaType::Type::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    case QMetaType::Type::QVariantMap:
      return quotedMap( value.toMap() );

    case QMetaType::Type::QStringList:
    case QMetaType::Type::QVariantList:
      return quotedList( value.toList() );

    case QMetaType::Type::Double:
    case QMetaType::Type::QString:
    default:
      return quotedString( value.toString() );
  }
}

QString QgsPostgresConn::quotedJsonValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"null"_s;
  // where json is a string literal just construct it from that rather than dump
  if ( value.userType() == QMetaType::Type::QString )
  {
    QString valueStr = value.toString();
    if ( valueStr.at( 0 ) == '\"' && valueStr.at( valueStr.size() - 1 ) == '\"' )
    {
      return quotedString( value.toString() );
    }
  }
  const auto j = QgsJsonUtils::jsonFromVariant( value );
  return quotedString( QString::fromStdString( j.dump() ) );
}

Qgis::PostgresRelKind QgsPostgresConn::relKindFromValue( const QString &value )
{
  if ( value == 'r' )
  {
    return Qgis::PostgresRelKind::OrdinaryTable;
  }
  else if ( value == 'i' )
  {
    return Qgis::PostgresRelKind::Index;
  }
  else if ( value == 's' )
  {
    return Qgis::PostgresRelKind::Sequence;
  }
  else if ( value == 'v' )
  {
    return Qgis::PostgresRelKind::View;
  }
  else if ( value == 'm' )
  {
    return Qgis::PostgresRelKind::MaterializedView;
  }
  else if ( value == 'c' )
  {
    return Qgis::PostgresRelKind::CompositeType;
  }
  else if ( value == 't' )
  {
    return Qgis::PostgresRelKind::ToastTable;
  }
  else if ( value == 'f' )
  {
    return Qgis::PostgresRelKind::ForeignTable;
  }
  else if ( value == 'p' )
  {
    return Qgis::PostgresRelKind::PartitionedTable;
  }

  return Qgis::PostgresRelKind::Unknown;
}

bool QgsPostgresConn::supportedLayers( QVector<QgsPostgresLayerProperty> &layers, bool searchGeometryColumnsOnly, bool allowGeometrylessTables, bool allowRasterOverviewTables, const QString &schema )
{
  return supportedLayersPrivate( layers, searchGeometryColumnsOnly, allowGeometrylessTables, allowRasterOverviewTables, schema );
}

bool QgsPostgresConn::supportedLayer( QgsPostgresLayerProperty &layerProperty, const QString &schema, const QString &table )
{
  QVector<QgsPostgresLayerProperty> layers;
  if ( !supportedLayersPrivate( layers, false, true /* allowGeometrylessTables */, false, schema, table ) || layers.empty() )
  {
    return false;
  }
  else
  {
    layerProperty = layers.first();
  }
  return true;
}


PGresult *QgsPostgresConn::PQexec( const QString &query, bool logError, bool retry, const QString &originatorClass, const QString &queryOrigin ) const
{
  QMutexLocker locker( &mLock );

  QgsDebugMsgLevel( u"Executing SQL: %1"_s.arg( query ), 3 );

  auto logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( query, mConnInfo, u"postgres"_s, originatorClass, queryOrigin );

  PGresult *res = ::PQexec( mConn, query.toUtf8() );

  // libpq may return a non null ptr with conn status not OK so we need to check for it to allow a retry below
  if ( res && PQstatus() == CONNECTION_OK )
  {
    int errorStatus = PQresultStatus( res );
    if ( errorStatus != PGRES_COMMAND_OK && errorStatus != PGRES_TUPLES_OK )
    {
      const QString error { tr( "Erroneous query: %1 returned %2 [%3]" )
                              .arg( query )
                              .arg( errorStatus )
                              .arg( PQresultErrorMessage( res ) ) };
      logWrapper->setError( error );

      if ( logError )
      {
        QgsMessageLog::logMessage( error, tr( "PostGIS" ) );
      }
      else
      {
        QgsDebugError( u"Not logged erroneous query: %1 returned %2 [%3]"_s
                         .arg( query )
                         .arg( errorStatus )
                         .arg( PQresultErrorMessage( res ) ) );
      }
    }
    logWrapper->setFetchedRows( PQntuples( res ) );
    return res;
  }
  if ( PQstatus() != CONNECTION_OK )
  {
    const QString error { tr( "Connection error: %1 returned %2 [%3]" )
                            .arg( query )
                            .arg( PQstatus() )
                            .arg( PQerrorMessage() ) };
    logWrapper->setError( error );
    if ( logError )
    {
      QgsMessageLog::logMessage( error, tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugError( u"Connection error: %1 returned %2 [%3]"_s
                       .arg( query )
                       .arg( PQstatus() )
                       .arg( PQerrorMessage() ) );
    }
  }
  else
  {
    const QString error { tr( "Query failed: %1\nError: no result buffer" ).arg( query ) };
    logWrapper->setError( error );
    if ( logError )
    {
      QgsMessageLog::logMessage( error, tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugError( u"Not logged query failed: %1\nError: no result buffer"_s.arg( query ) );
    }
  }

  if ( retry )
  {
    QgsMessageLog::logMessage( tr( "resetting bad connection." ), tr( "PostGIS" ) );
    ::PQreset( mConn );
    logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( query, mConnInfo, u"postgres"_s, originatorClass, queryOrigin );
    res = PQexec( query, logError, false );
    if ( PQstatus() == CONNECTION_OK )
    {
      if ( res )
      {
        QgsMessageLog::logMessage( tr( "retry after reset succeeded." ), tr( "PostGIS" ) );
        return res;
      }
      else
      {
        const QString error { tr( "retry after reset failed again." ) };
        logWrapper->setError( error );
        QgsMessageLog::logMessage( error, tr( "PostGIS" ) );
        return nullptr;
      }
    }
    else
    {
      const QString error { tr( "connection still bad after reset." ) };
      logWrapper->setError( error );
      QgsMessageLog::logMessage( error, tr( "PostGIS" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "bad connection, not retrying." ), tr( "PostGIS" ) );
  }
  return nullptr;
}

int QgsPostgresConn::PQCancel()
{
  // No locker: this is supposed to be thread safe
  int result = 0;
  auto cancel = ::PQgetCancel( mConn );
  if ( cancel )
  {
    char errbuf[255];
    result = ::PQcancel( cancel, errbuf, 255 );
#ifdef QGISDEBUG
    if ( !result )
    {
      QgsDebugMsgLevel( u"Error canceling the query:"_s.arg( errbuf ), 3 );
    }
#endif
  }
  ::PQfreeCancel( cancel );
  return result;
}

bool QgsPostgresConn::openCursor( const QString &cursorName, const QString &sql )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors
  QString preStr;

  if ( mOpenCursors++ == 0 && !mTransaction )
  {
    QgsDebugMsgLevel( u"Starting read-only transaction: %1"_s.arg( mPostgresqlVersion ), 4 );
    if ( mPostgresqlVersion >= 80000 )
      preStr = u"BEGIN READ ONLY;"_s;
    else
      preStr = u"BEGIN;"_s;
  }
  QgsDebugMsgLevel( u"Binary cursor %1 for %2"_s.arg( cursorName, sql ), 3 );
  return LoggedPQexecNR( "QgsPostgresConn", u"%1DECLARE %2 BINARY CURSOR%3 FOR %4"_s.arg( preStr, cursorName, !mTransaction ? QString() : u" WITH HOLD"_s, sql ) );
}

bool QgsPostgresConn::closeCursor( const QString &cursorName )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors
  QString postStr;

  if ( --mOpenCursors == 0 && !mTransaction )
  {
    QgsDebugMsgLevel( u"Committing read-only transaction"_s, 4 );
    postStr = u";COMMIT"_s;
  }

  if ( !LoggedPQexecNR( u"QgsPostgresConn"_s, u"CLOSE %1%2"_s.arg( cursorName, postStr ) ) )
    return false;

  return true;
}

QString QgsPostgresConn::uniqueCursorName()
{
  return u"qgis_%1"_s.arg( ++sNextCursorId );
}

bool QgsPostgresConn::PQexecNR( const QString &query, const QString &originatorClass, const QString &queryOrigin )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors

  QgsPostgresResult res( PQexec( query, false, true, originatorClass, queryOrigin ) );

  ExecStatusType errorStatus = res.PQresultStatus();
  if ( errorStatus == PGRES_COMMAND_OK )
    return true;

  QgsMessageLog::logMessage( tr( "Query: %1 returned %2 [%3]" ).arg( query ).arg( errorStatus ).arg( res.PQresultErrorMessage() ), tr( "PostGIS" ) );

  if ( mOpenCursors )
  {
    QgsMessageLog::logMessage( tr( "%1 cursor states lost.\nSQL: %2\nResult: %3 (%4)" ).arg( mOpenCursors ).arg( query ).arg( errorStatus ).arg( res.PQresultErrorMessage() ), tr( "PostGIS" ) );
    mOpenCursors = 0;
  }

  if ( PQstatus() == CONNECTION_OK )
  {
    LoggedPQexecNR( u"QgsPostgresConn"_s, u"ROLLBACK"_s );
  }

  return false;
}

PGresult *QgsPostgresConn::PQgetResult()
{
  return ::PQgetResult( mConn );
}

PGresult *QgsPostgresConn::PQprepare( const QString &stmtName, const QString &query, int nParams, const Oid *paramTypes, const QString &originatorClass, const QString &queryOrigin )
{
  QMutexLocker locker( &mLock );

  auto logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( u"PQprepare(%1): %2 "_s.arg( stmtName, query ), mConnInfo, u"postgres"_s, originatorClass, queryOrigin );

  PGresult *res { ::PQprepare( mConn, stmtName.toUtf8(), query.toUtf8(), nParams, paramTypes ) };

  const int errorStatus = PQresultStatus( res );

  if ( errorStatus != PGRES_TUPLES_OK )
  {
    logWrapper->setError( PQresultErrorMessage( res ) );
  }

  return res;
}

PGresult *QgsPostgresConn::PQexecPrepared( const QString &stmtName, const QStringList &params, const QString &originatorClass, const QString &queryOrigin )
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

  auto logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( u"PQexecPrepared(%1)"_s.arg( stmtName ), mConnInfo, u"postgres"_s, originatorClass, queryOrigin );

  PGresult *res { ::PQexecPrepared( mConn, stmtName.toUtf8(), params.size(), param, nullptr, nullptr, 0 ) };

  const int errorStatus = PQresultStatus( res );

  if ( errorStatus != PGRES_TUPLES_OK && errorStatus != PGRES_COMMAND_OK )
  {
    logWrapper->setError( PQresultErrorMessage( res ) );
  }

  delete[] param;

  return res;
}

void QgsPostgresConn::PQfinish()
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  ::PQfinish( mConn );
  mConn = nullptr;
}

int QgsPostgresConn::PQstatus() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  return ::PQstatus( mConn );
}

QString QgsPostgresConn::PQerrorMessage() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  return QString::fromUtf8( ::PQerrorMessage( mConn ) );
}

int QgsPostgresConn::PQsendQuery( const QString &query )
{
  QMutexLocker locker( &mLock );
  Q_ASSERT( mConn );
  return ::PQsendQuery( mConn, query.toUtf8() );
}

bool QgsPostgresConn::begin()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"SAVEPOINT transaction_savepoint"_s );
  }
  else
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"BEGIN"_s );
  }
}

bool QgsPostgresConn::commit()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"RELEASE SAVEPOINT transaction_savepoint"_s );
  }
  else
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"COMMIT"_s );
  }
}

bool QgsPostgresConn::rollback()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"ROLLBACK TO SAVEPOINT transaction_savepoint"_s )
           && LoggedPQexecNR( u"QgsPostgresConn"_s, u"RELEASE SAVEPOINT transaction_savepoint"_s );
  }
  else
  {
    return LoggedPQexecNR( u"QgsPostgresConn"_s, u"ROLLBACK"_s );
  }
}

qint64 QgsPostgresConn::getBinaryInt( QgsPostgresResult &queryResult, int row, int col )
{
  QMutexLocker locker( &mLock );
  quint64 oid;
  char *p = PQgetvalue( queryResult.result(), row, col );
  size_t s = PQgetlength( queryResult.result(), row, col );

#ifdef QGISDEBUG
  if ( QgsLogger::debugLevel() >= 4 )
  {
    QString buf;
    for ( size_t i = 0; i < s; i++ )
    {
      buf += u"%1 "_s.arg( *( unsigned char * ) ( p + i ), 0, 16, QLatin1Char( ' ' ) );
    }

    QgsDebugMsgLevel( u"int in hex:%1"_s.arg( buf ), 2 );
  }
#endif

  switch ( s )
  {
    case 2:
      oid = *( quint16 * ) p;
      if ( mSwapEndian )
        oid = ntohs( oid );
      /* cast to signed 16bit
       * See https://github.com/qgis/QGIS/issues/22258 */
      oid = ( qint16 ) oid;
      break;

    case 6:
    {
      quint64 block = *( quint32 * ) p;
      quint64 offset = *( quint16 * ) ( p + sizeof( quint32 ) );

      if ( mSwapEndian )
      {
        block = ntohl( block );
        offset = ntohs( offset );
      }

      oid = ( block << 16 ) + offset;
    }
    break;

    case 8:
    {
      quint32 oid0 = *( quint32 * ) p;
      quint32 oid1 = *( quint32 * ) ( p + sizeof( quint32 ) );

      if ( mSwapEndian )
      {
        QgsDebugMsgLevel( u"swap oid0:%1 oid1:%2"_s.arg( oid0 ).arg( oid1 ), 4 );
        oid0 = ntohl( oid0 );
        oid1 = ntohl( oid1 );
      }

      QgsDebugMsgLevel( u"oid0:%1 oid1:%2"_s.arg( oid0 ).arg( oid1 ), 4 );
      oid = oid0;
      QgsDebugMsgLevel( u"oid:%1"_s.arg( oid ), 4 );
      oid <<= 32;
      QgsDebugMsgLevel( u"oid:%1"_s.arg( oid ), 4 );
      oid |= oid1;
      QgsDebugMsgLevel( u"oid:%1"_s.arg( oid ), 4 );
    }
    break;

    default:
      QgsDebugError( u"unexpected size %1"_s.arg( s ) );
      //intentional fall-through
      [[fallthrough]];
    case 4:
      oid = *( quint32 * ) p;
      if ( mSwapEndian )
        oid = ntohl( oid );
      /* cast to signed 32bit
       * See https://github.com/qgis/QGIS/issues/22258 */
      oid = ( qint32 ) oid;
      break;
  }

  return oid;
}

QString QgsPostgresConn::fieldExpressionForWhereClause( const QgsField &fld, QMetaType::Type valueType, QString expr )
{
  QString out;
  const QString &type = fld.typeName();

  if ( type == "timestamp"_L1 || type == "time"_L1 || type == "date"_L1 )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // if field and value havev incompatible types, rollback to text cast
    if ( valueType != QMetaType::Type::UnknownType && valueType != QMetaType::Type::QDateTime && valueType != QMetaType::Type::QDate && valueType != QMetaType::Type::QTime )
    {
      out = out + "::text";
    }
  }

  else if ( type == "int8"_L1 || type == "serial8"_L1                                                           //
            || type == "int2"_L1 || type == "int4"_L1 || type == "oid"_L1 || type == "serial"_L1                //
            || type == "real"_L1 || type == "double precision"_L1 || type == "float4"_L1 || type == "float8"_L1 //
            || type == "numeric"_L1 )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // if field and value havev incompatible types, rollback to text cast
    if ( valueType != QMetaType::Type::UnknownType && valueType != QMetaType::Type::Int && valueType != QMetaType::Type::LongLong && valueType != QMetaType::Type::Double )
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

QString QgsPostgresConn::fieldExpression( const QgsField &fld, QString expr )
{
  const QString &type = fld.typeName();
  expr = expr.arg( quotedIdentifier( fld.name() ) );
  if ( type == "money"_L1 )
  {
    return u"%1::numeric::text"_s.arg( expr );
  }
  else if ( type.startsWith( '_' ) )
  {
    //TODO: add native support for arrays
    return u"array_out(%1)::text"_s.arg( expr );
  }
  else if ( type == "bool"_L1 )
  {
    return u"boolout(%1)::text"_s.arg( expr );
  }
  else if ( type == "geometry"_L1 )
  {
    return u"%1(%2)"_s
      .arg( majorVersion() < 2 ? "asewkt" : "st_asewkt", expr );
  }
  else if ( type == "geography"_L1 )
  {
    return u"st_astext(%1)"_s.arg( expr );
  }
  else if ( type == "int8"_L1 )
  {
    return expr;
  }
  //TODO: add support for hstore
  //TODO: add support for json/jsonb
  else
  {
    return expr + "::text";
  }
}

QList<QgsVectorDataProvider::NativeType> QgsPostgresConn::nativeTypes()
{
  QList<QgsVectorDataProvider::NativeType> types;

  types // integer types
    << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 64bit)" ), u"int8"_s, QMetaType::Type::LongLong, -1, -1, 0, 0 )
    << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 32bit)" ), u"int4"_s, QMetaType::Type::Int, -1, -1, 0, 0 )
    << QgsVectorDataProvider::NativeType( tr( "Whole Number (smallint - 16bit)" ), u"int2"_s, QMetaType::Type::Int, -1, -1, 0, 0 )
    << QgsVectorDataProvider::NativeType( tr( "Decimal Number (numeric)" ), u"numeric"_s, QMetaType::Type::Double, 1, 20, 0, 20 )
    << QgsVectorDataProvider::NativeType( tr( "Decimal Number (decimal)" ), u"decimal"_s, QMetaType::Type::Double, 1, 20, 0, 20 )

    // floating point
    << QgsVectorDataProvider::NativeType( tr( "Decimal Number (double)" ), u"double precision"_s, QMetaType::Type::Double, -1, -1, -1, -1 )
    << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), u"real"_s, QMetaType::Type::Double, -1, -1, -1, -1 )

    // string types
    << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), u"varchar"_s, QMetaType::Type::QString, 1, 255, -1, -1 )
    << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), u"char"_s, QMetaType::Type::QString, 1, 255, -1, -1 )
    << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), u"text"_s, QMetaType::Type::QString, -1, -1, -1, -1 )
    << QgsVectorDataProvider::NativeType( tr( "Text, case-insensitive unlimited length (citext)" ), u"citext"_s, QMetaType::Type::QString, -1, -1, -1, -1 )

    // date type
    << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), u"date"_s, QMetaType::Type::QDate, -1, -1, -1, -1 )
    << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), u"time"_s, QMetaType::Type::QTime, -1, -1, -1, -1 )
    << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), u"timestamp without time zone"_s, QMetaType::Type::QDateTime, -1, -1, -1, -1 )

    // complex types
    << QgsVectorDataProvider::NativeType( tr( "Map (hstore)" ), u"hstore"_s, QMetaType::Type::QVariantMap, -1, -1, -1, -1, QMetaType::Type::QString )
    << QgsVectorDataProvider::NativeType( tr( "Array of Number (integer - 32bit)" ), u"int4[]"_s, QMetaType::Type::QVariantList, -1, -1, -1, -1, QMetaType::Type::Int )
    << QgsVectorDataProvider::NativeType( tr( "Array of Number (integer - 64bit)" ), u"int8[]"_s, QMetaType::Type::QVariantList, -1, -1, -1, -1, QMetaType::Type::LongLong )
    << QgsVectorDataProvider::NativeType( tr( "Array of Number (double)" ), u"double precision[]"_s, QMetaType::Type::QVariantList, -1, -1, -1, -1, QMetaType::Type::Double )
    << QgsVectorDataProvider::NativeType( tr( "Array of Text" ), u"text[]"_s, QMetaType::Type::QStringList, -1, -1, -1, -1, QMetaType::Type::QString )

    // boolean
    << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), u"bool"_s, QMetaType::Type::Bool, -1, -1, -1, -1 )

    // binary (bytea)
    << QgsVectorDataProvider::NativeType( tr( "Binary Object (bytea)" ), u"bytea"_s, QMetaType::Type::QByteArray, -1, -1, -1, -1 );

  if ( pgVersion() >= 90200 )
  {
    types << QgsVectorDataProvider::NativeType( tr( "JSON (json)" ), u"json"_s, QMetaType::Type::QVariantMap, -1, -1, -1, -1, QMetaType::Type::QString );

    if ( pgVersion() >= 90400 )
    {
      types << QgsVectorDataProvider::NativeType( tr( "JSON (jsonb)" ), u"jsonb"_s, QMetaType::Type::QVariantMap, -1, -1, -1, -1, QMetaType::Type::QString );
    }
  }
  return types;
}

void QgsPostgresConn::deduceEndian()
{
  QMutexLocker locker( &mLock );
  // need to store the PostgreSQL endian format used in binary cursors
  // since it appears that starting with
  // version 7.4, binary cursors return data in XDR whereas previous versions
  // return data in the endian of the server

  QgsPostgresResult resOID;
#ifdef QGISDEBUG
  int queryCounter = 0;
#endif
  int errorCounter = 0;
  int oidStatus = 0;
  int oidSelectSet = 1 << 0;
  int oidBinaryCursorSet = 1 << 1;
  qint64 oidSelect = 0;
  qint64 oidBinaryCursor = 0;

  if ( 0 == PQsendQuery( QStringLiteral( "SELECT regclass('pg_class')::oid AS oidselect;"
                                         "BEGIN;"
                                         "DECLARE oidcursor BINARY CURSOR FOR SELECT regclass('pg_class')::oid AS oidbinarycursor;"
                                         "FETCH FORWARD 1 FROM oidcursor;"
                                         "CLOSE oidcursor;"
                                         "COMMIT;" ) ) )
  {
    QgsDebugMsgLevel( u"PQsendQuery(...) error %1"_s.arg( PQerrorMessage() ), 2 );
  }

  for ( ;; )
  {
    // PQgetResult() must be called repeatedly until it returns a null pointer
    resOID = PQgetResult();

    if ( !resOID.result() )
      break;

#ifdef QGISDEBUG
    queryCounter++;
#endif
    if ( resOID.PQresultStatus() == PGRES_FATAL_ERROR )
    {
      errorCounter++;
      QgsDebugMsgLevel( u"QUERY #%1 PGRES_FATAL_ERROR %2"_s.arg( queryCounter ).arg( PQerrorMessage().trimmed() ), 2 );
      continue;
    }

    if ( resOID.PQresultStatus() == PGRES_TUPLES_OK && resOID.PQnfields() && resOID.PQntuples() )
    {
      if ( resOID.PQfname( 0 ) == "oidselect"_L1 )
      {
        oidSelect = resOID.PQgetvalue( 0, 0 ).toLongLong();
        oidStatus |= oidSelectSet;
      }
      if ( resOID.PQfname( 0 ) == "oidbinarycursor"_L1 )
      {
        oidBinaryCursor = getBinaryInt( resOID, 0, 0 );
        oidStatus |= oidBinaryCursorSet;
      }
    }
  }

  if ( errorCounter == 0 && oidStatus == ( oidSelectSet | oidBinaryCursorSet ) )
  {
    mSwapEndian = mSwapEndian == ( oidSelect == oidBinaryCursor );
    return;
  }

  QgsDebugMsgLevel( u"Back to old deduceEndian(): PQstatus() - %1, queryCounter = %2, errorCounter = %3"_s.arg( PQstatus() ).arg( queryCounter ).arg( errorCounter ), 2 );

  QgsPostgresResult res( LoggedPQexec( u"QgsPostgresConn"_s, u"select regclass('pg_class')::oid"_s ) );
  QString oidValue = res.PQgetvalue( 0, 0 );

  QgsDebugMsgLevel( u"Creating binary cursor"_s, 2 );

  // get the same value using a binary cursor
  openCursor( u"oidcursor"_s, u"select regclass('pg_class')::oid"_s );

  QgsDebugMsgLevel( u"Fetching a record and attempting to get check endian-ness"_s, 2 );

  res = LoggedPQexec( u"QgsPostgresConn"_s, u"fetch forward 1 from oidcursor"_s );

  mSwapEndian = true;
  if ( res.PQntuples() > 0 )
  {
    // get the oid value from the binary cursor
    qint64 oid = getBinaryInt( res, 0, 0 );

    QgsDebugMsgLevel( u"Got oid of %1 from the binary cursor"_s.arg( oid ), 2 );
    QgsDebugMsgLevel( u"First oid is %1"_s.arg( oidValue ), 2 );

    // compare the two oid values to determine if we need to do an endian swap
    if ( oid != oidValue.toLongLong() )
      mSwapEndian = false;
  }

  closeCursor( u"oidcursor"_s );
}

void QgsPostgresConn::retrieveLayerTypes( QgsPostgresLayerProperty &layerProperty, bool useEstimatedMetadata, QgsFeedback *feedback )
{
  QVector<QgsPostgresLayerProperty *> vect;
  vect << &layerProperty;
  retrieveLayerTypes( vect, useEstimatedMetadata, feedback );
}

void QgsPostgresConn::retrieveLayerTypes( QVector<QgsPostgresLayerProperty *> &layerProperties, bool useEstimatedMetadata, QgsFeedback *feedback )
{
  QString table;
  QString query;

  // Limit table row scan if useEstimatedMetadata
  const QString tableScanLimit { useEstimatedMetadata ? u" LIMIT %1"_s.arg( GEOM_TYPE_SELECT_LIMIT ) : QString() };

  int i = 0;
  for ( auto *layerPropertyPtr : layerProperties )
  {
    QgsPostgresLayerProperty &layerProperty = *layerPropertyPtr;

    if ( i++ )
      query += " UNION ";

    if ( !layerProperty.schemaName.isEmpty() )
    {
      table = u"%1.%2"_s
                .arg( quotedIdentifier( layerProperty.schemaName ), quotedIdentifier( layerProperty.tableName ) );
    }
    else
    {
      // Query
      table = layerProperty.tableName;
    }

    if ( layerProperty.geometryColName.isEmpty() )
      continue;

    if ( layerProperty.isRaster )
    {
      QString sql;

      int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
      // SRID is already known
      if ( srid != std::numeric_limits<int>::min() )
      {
        sql += u"SELECT %1, array_agg( '%2:RASTER:-1'::text )"_s
                 .arg( i - 1 )
                 .arg( srid );
      }
      else
      {
        if ( useEstimatedMetadata )
        {
          sql = QStringLiteral( "SELECT %1, "
                                "array_agg(srid || ':RASTER:-1') "
                                "FROM raster_columns "
                                "WHERE r_raster_column = %2 AND r_table_schema = %3 AND r_table_name = %4" )
                  .arg( i - 1 )
                  .arg( quotedValue( layerProperty.geometryColName ) )
                  .arg( quotedValue( layerProperty.schemaName ) )
                  .arg( quotedValue( layerProperty.tableName ) );
        }
        else
        {
          sql = QStringLiteral( "SELECT %1, "
                                "array_agg(DISTINCT st_srid(%2) || ':RASTER:-1') "
                                "FROM %3 "
                                "WHERE %2 IS NOT NULL "
                                "%4" // SQL clause
                                "%5" )
                  .arg( i - 1 )
                  .arg( quotedIdentifier( layerProperty.geometryColName ) )
                  .arg( table )
                  .arg( layerProperty.sql.isEmpty() ? QString() : u" AND %1"_s.arg( layerProperty.sql ) )
                  .arg( tableScanLimit );
        }
      }

      QgsDebugMsgLevel( "Raster srids query: " + sql, 2 );
      query += sql;
    }
    else // vectors
    {
      // our estimation ignores that a where clause might restrict the feature type or srid
      if ( useEstimatedMetadata )
      {
        table = u"(SELECT %1 FROM %2 WHERE %3%1 IS NOT NULL%4) AS t"_s
                  .arg( quotedIdentifier( layerProperty.geometryColName ), table, layerProperty.sql.isEmpty() ? QString() : u" (%1) AND "_s.arg( layerProperty.sql ) )
                  .arg( tableScanLimit );
      }
      else if ( !layerProperty.sql.isEmpty() )
      {
        table += u" WHERE %1"_s.arg( layerProperty.sql );
      }

      QString sql = u"SELECT %1, "_s.arg( i - 1 );

      if ( layerProperty.geometryColType == SctTopoGeometry )
      {
        sql += QLatin1String( R"SQL(
          array_agg( DISTINCT
            t.srid::text
            || ':' ||
            CASE
              WHEN l.feature_type = 1 THEN 'MULTIPOINT'
              WHEN l.feature_type = 2 THEN 'MULTILINESTRING'
              WHEN l.feature_type = 3 THEN 'MULTIPOLYGON'
              ELSE 'GEOMETRYCOLLECTION'
            END
            || ':' ||
            CASE
              WHEN t.hasz THEN '2'
              ELSE '0'
            END
          )
          FROM
            (
              SELECT DISTINCT topology_id(%1), layer_id(%1)
              FROM (
                  SELECT %1 FROM %2%3
              ) qry
            ) lyr
            JOIN topology.topology t ON ( t.id = lyr.topology_id )
            JOIN topology.layer l ON ( lyr.layer_id = l.layer_id AND l.topology_id = lyr.topology_id )
        )SQL" )
                 .arg( quotedIdentifier( layerProperty.geometryColName ) )
                 .arg( table )
                 .arg( tableScanLimit );
      }
      else
      {
        bool castToGeometry = layerProperty.geometryColType == SctGeography || layerProperty.geometryColType == SctPcPatch;

        sql += "array_agg(DISTINCT "_L1;

        int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
        if ( srid == std::numeric_limits<int>::min() )
        {
          sql += u"%1(%2%3)::text"_s
                   .arg( majorVersion() < 2 ? "srid" : "st_srid", quotedIdentifier( layerProperty.geometryColName ), castToGeometry ? "::geometry" : "" );
        }
        else
        {
          sql += u"%1::text"_s
                   .arg( QString::number( srid ) );
        }

        sql += " || ':' || ";

        Qgis::WkbType type = layerProperty.types.value( 0, Qgis::WkbType::Unknown );
        if ( type == Qgis::WkbType::Unknown )
        {
          // Note that we would like to apply a "LIMIT GEOM_TYPE_SELECT_LIMIT"
          // here, so that the previous "array_agg(DISTINCT" does not scan the
          // full table. However SQL does not allow that.
          // So we have to do a subselect on the table to add the LIMIT,
          // see comment in the following code.
          sql += u"UPPER(geometrytype(%1%2))  || ':' || ST_Zmflag(%1%2)"_s
                   .arg( quotedIdentifier( layerProperty.geometryColName ), castToGeometry ? "::geometry" : "" );
        }
        else
        {
          sql += u"%1::text  || ':-1'"_s
                   .arg( quotedValue( QgsPostgresConn::postgisWkbTypeName( type ) ) );
        }


        sql += ") "_L1;

        if ( type == Qgis::WkbType::Unknown )
        {
          // Subselect to limit the "array_agg(DISTINCT", see previous comment.
          sql += u" FROM (SELECT %1 FROM %2%3) AS _unused"_s
                   .arg( quotedIdentifier( layerProperty.geometryColName ) )
                   .arg( table )
                   .arg( tableScanLimit );
        }
        else
        {
          sql += " FROM " + table;
        }
      }

      QgsDebugMsgLevel( "Geometry types,srids and dims query: " + sql, 2 );

      query += sql;
    }
  }

  QgsDebugMsgLevel( "Layer types,srids and dims query: " + query, 3 );

  QgsPostgresResult res( LoggedPQexec( u"QgsPostgresConn"_s, query ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    // TODO: print some error here ?
    return;
  }

  for ( int i = 0; i < res.PQntuples(); i++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    int idx = res.PQgetvalue( i, 0 ).toInt();
    auto srids_and_types = QgsPostgresStringUtils::parseArray( res.PQgetvalue( i, 1 ) );
    QgsPostgresLayerProperty &layerProperty = *layerProperties[idx];

    QgsDebugMsgLevel( u"Layer %1.%2.%3 has %4 srid/type combinations"_s.arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ).arg( srids_and_types.length() ), 3 );

    /* Gather found types */
    QList<std::pair<Qgis::WkbType, int>> foundCombinations;
    for ( const auto &sridAndTypeVariant : srids_and_types )
    {
      QString sridAndTypeString = sridAndTypeVariant.toString();

      QgsDebugMsgLevel( QStringLiteral( "Analyzing layer's %1.%2.%3 sridAndType %4"
                                        " against %6 found combinations" )
                          .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                          .arg( sridAndTypeString )
                          .arg( foundCombinations.length() ),
                        3 );

      if ( sridAndTypeString == "NULL" )
        continue;

      const QStringList sridAndType = sridAndTypeString.split( ':' );
      Q_ASSERT( sridAndType.size() == 3 );
      const int srid = sridAndType[0].toInt();
      QString typeString = sridAndType[1];
      const int zmFlags = sridAndType[2].toInt();

      switch ( zmFlags )
      {
        case 1:
          // Unlike Geometry(Z), GeometryM contains already M suffix.
          // It's useless to add M suffix here,
          // since it will add a new name and the geometry type name will be something like GeometryMM.
          // see: https://github.com/qgis/QGIS/issues/55223
          //typeString.append( 'M' );
          break;
        case 2:
          typeString.append( 'Z' );
          break;
        case 3:
          typeString.append( u"ZM"_s );
          break;
        default:
        case 0:
        case -1:
          break;
      }

      auto type = QgsPostgresConn::wkbTypeFromPostgis( typeString );
      auto flatType = QgsWkbTypes::flatType( type );
      auto multiType = QgsWkbTypes::multiType( flatType );
      auto curveType = QgsWkbTypes::curveType( flatType );
      auto multiCurveType = QgsWkbTypes::multiType( curveType );

      // if both multi and single types exists, go for the multi type,
      // so that st_multi can be applied if necessary.

      // if both flat and curve types exists, go for the curve type,
      // so that st_multi can be applied if necessary.

      int j;
      for ( j = 0; j < foundCombinations.length(); j++ )
      {
        auto foundPair = foundCombinations.at( j );
        if ( foundPair.second != srid )
          continue; // srid must match

        auto knownType = foundPair.first;
        if ( type == knownType )
          break; // found

        auto knownMultiType = QgsWkbTypes::multiType( knownType );
        auto knownCurveType = QgsWkbTypes::curveType( knownType );
        auto knownMultiCurveType = QgsWkbTypes::multiType( knownCurveType );

        if ( multiCurveType == knownMultiCurveType )
        {
          QgsDebugMsgLevel( QStringLiteral( "Upgrading type[%1] of layer %2.%3.%4 "
                                            "to multiCurved type %5" )
                              .arg( j )
                              .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                              .arg( qgsEnumValueToKey( multiCurveType ) ),
                            3 );
          foundCombinations[j].first = multiCurveType;
          break;
        }
        else if ( multiType == knownMultiType )
        {
          QgsDebugMsgLevel( QStringLiteral( "Upgrading type[%1] of layer %2.%3.%4 "
                                            "to multi type %5" )
                              .arg( j )
                              .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                              .arg( qgsEnumValueToKey( multiType ) ),
                            3 );
          foundCombinations[j].first = multiType;
          break;
        }
        else if ( curveType == knownCurveType )
        {
          QgsDebugMsgLevel( QStringLiteral( "Upgrading type[%1] of layer %2.%3.%4 "
                                            "to curved type %5" )
                              .arg( j )
                              .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                              .arg( qgsEnumValueToKey( multiType ) ),
                            3 );
          foundCombinations[j].first = curveType;
          break;
        }
      }

      if ( j < foundCombinations.length() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Pre-existing compatible combination %1/%2 "
                                          "found for layer %3.%4.%5 " )
                            .arg( j )
                            .arg( foundCombinations.length() )
                            .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ),
                          3 );
        continue; // already found
      }

      QgsDebugMsgLevel( QStringLiteral( "Setting typeSridCombination[%1] of layer %2.%3.%4 "
                                        "to srid %5 and type %6" )
                          .arg( j )
                          .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                          .arg( srid )
                          .arg( qgsEnumValueToKey( type ) ),
                        3 );

      foundCombinations << std::make_pair( type, srid );
    }

    QgsDebugMsgLevel( QStringLiteral( "Completed scan of %1 srid/type combinations "
                                      "for layer of layer %2.%3.%4 " )
                        .arg( srids_and_types.length() )
                        .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ),
                      2 );

    /* Rewrite srids and types to match found combinations
     * of srids and types */
    layerProperty.srids.clear();
    layerProperty.types.clear();
    for ( const auto &comb : foundCombinations )
    {
      layerProperty.types << comb.first;
      layerProperty.srids << comb.second;
    }
    QgsDebugMsgLevel( u"Final layer %1.%2.%3 types: %4"_s.arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ).arg( layerProperty.types.length() ), 2 );
    QgsDebugMsgLevel( u"Final layer %1.%2.%3 srids: %4"_s.arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ).arg( layerProperty.srids.length() ), 2 );
  }
}

void QgsPostgresConn::postgisWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim )
{
  dim = 2;
  Qgis::WkbType flatType = QgsWkbTypes::flatType( wkbType );
  switch ( flatType )
  {
    case Qgis::WkbType::Point:
      geometryType = u"POINT"_s;
      break;

    case Qgis::WkbType::LineString:
      geometryType = u"LINESTRING"_s;
      break;

    case Qgis::WkbType::Polygon:
      geometryType = u"POLYGON"_s;
      break;

    case Qgis::WkbType::MultiPoint:
      geometryType = u"MULTIPOINT"_s;
      break;

    case Qgis::WkbType::MultiLineString:
      geometryType = u"MULTILINESTRING"_s;
      break;

    case Qgis::WkbType::MultiPolygon:
      geometryType = u"MULTIPOLYGON"_s;
      break;

    case Qgis::WkbType::CircularString:
      geometryType = u"CIRCULARSTRING"_s;
      break;

    case Qgis::WkbType::CompoundCurve:
      geometryType = u"COMPOUNDCURVE"_s;
      break;

    case Qgis::WkbType::CurvePolygon:
      geometryType = u"CURVEPOLYGON"_s;
      break;

    case Qgis::WkbType::MultiCurve:
      geometryType = u"MULTICURVE"_s;
      break;

    case Qgis::WkbType::MultiSurface:
      geometryType = u"MULTISURFACE"_s;
      break;

    case Qgis::WkbType::Triangle:
      geometryType = u"TRIANGLE"_s;
      break;

    case Qgis::WkbType::PolyhedralSurface:
      geometryType = u"POLYHEDRALSURFACE"_s;
      break;

    case Qgis::WkbType::TIN:
      geometryType = u"TIN"_s;
      break;

    case Qgis::WkbType::Unknown:
      geometryType = u"GEOMETRY"_s;
      break;

    case Qgis::WkbType::NoGeometry:
    default:
      dim = 0;
      break;
  }

  if ( QgsWkbTypes::hasZ( wkbType ) && QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += "ZM"_L1;
    dim = 4;
  }
  else if ( QgsWkbTypes::hasZ( wkbType ) )
  {
    geometryType += QLatin1Char( 'Z' );
    dim = 3;
  }
  else if ( QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1Char( 'M' );
    dim = 3;
  }
  else if ( wkbType >= Qgis::WkbType::Point25D && wkbType <= Qgis::WkbType::MultiPolygon25D )
  {
    dim = 3;
  }
}

QString QgsPostgresConn::postgisWkbTypeName( Qgis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return geometryType;
}

QString QgsPostgresConn::postgisTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry )
{
  geomCol = quotedIdentifier( geomCol );
  if ( castToGeometry )
    geomCol += "::geometry"_L1;

  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      return u"upper(geometrytype(%1)) IN ('POINT','POINTZ','POINTM','POINTZM','MULTIPOINT','MULTIPOINTZ','MULTIPOINTM','MULTIPOINTZM')"_s.arg( geomCol );
    case Qgis::GeometryType::Line:
      return u"upper(geometrytype(%1)) IN ('LINESTRING','LINESTRINGZ','LINESTRINGM','LINESTRINGZM','CIRCULARSTRING','CIRCULARSTRINGZ','CIRCULARSTRINGM','CIRCULARSTRINGZM','COMPOUNDCURVE','COMPOUNDCURVEZ','COMPOUNDCURVEM','COMPOUNDCURVEZM','MULTILINESTRING','MULTILINESTRINGZ','MULTILINESTRINGM','MULTILINESTRINGZM','MULTICURVE','MULTICURVEZ','MULTICURVEM','MULTICURVEZM')"_s.arg( geomCol );
    case Qgis::GeometryType::Polygon:
      return u"upper(geometrytype(%1)) IN ('POLYGON','POLYGONZ','POLYGONM','POLYGONZM','CURVEPOLYGON','CURVEPOLYGONZ','CURVEPOLYGONM','CURVEPOLYGONZM','MULTIPOLYGON','MULTIPOLYGONZ','MULTIPOLYGONM','MULTIPOLYGONZM','MULTIPOLYGONM','MULTISURFACE','MULTISURFACEZ','MULTISURFACEM','MULTISURFACEZM','TRIANGLE','TRIANGLEZ','TRIANGLEM','TRIANGLEZM','POLYHEDRALSURFACE','POLYHEDRALSURFACEZ','POLYHEDRALSURFACEM','POLYHEDRALSURFACEZM','TIN','TINZ','TINM','TINZM')"_s.arg( geomCol );
    case Qgis::GeometryType::Null:
      return u"geometrytype(%1) IS NULL"_s.arg( geomCol );
    default: //unknown geometry
      return QString();
  }
}

int QgsPostgresConn::postgisWkbTypeDim( Qgis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return dim;
}

Qgis::WkbType QgsPostgresConn::wkbTypeFromPostgis( const QString &type )
{
  return QgsWkbTypes::parseType( type );
}

Qgis::WkbType QgsPostgresConn::wkbTypeFromOgcWkbType( unsigned int wkbType )
{
  return ( Qgis::WkbType ) wkbType;
}

QString QgsPostgresConn::displayStringForWkbType( Qgis::WkbType type )
{
  return QgsWkbTypes::displayString( type );
}

QString QgsPostgresConn::displayStringForGeomType( QgsPostgresGeometryColumnType type )
{
  switch ( type )
  {
    case SctNone:
      return tr( "None" );
    case SctGeometry:
      return tr( "Geometry" );
    case SctGeography:
      return tr( "Geography" );
    case SctTopoGeometry:
      return tr( "TopoGeometry" );
    case SctPcPatch:
      return tr( "PcPatch" );
    case SctRaster:
      return tr( "Raster" );
  }

  Q_ASSERT( !"unexpected geometry column type" );
  return QString();
}

Qgis::WkbType QgsPostgresConn::wkbTypeFromGeomType( Qgis::GeometryType geomType )
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

QStringList QgsPostgresConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( u"PostgreSQL/connections"_s );
  return settings.childGroups();
}

QString QgsPostgresConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( u"PostgreSQL/connections/selected"_s ).toString();
}

void QgsPostgresConn::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( u"PostgreSQL/connections/selected"_s, name );
}

QgsDataSourceUri QgsPostgresConn::connUri( const QString &connName )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 2 );

  QgsSettings settings;

  QString key = u"/PostgreSQL/connections/"_s + connName;

  QString service = settings.value( key + u"/service"_s ).toString();
  QString host = settings.value( key + u"/host"_s ).toString();
  QString port = settings.value( key + u"/port"_s ).toString();
  if ( port.length() == 0 )
  {
    port = u"5432"_s;
  }
  QString database = settings.value( key + u"/database"_s ).toString();

  bool estimatedMetadata = useEstimatedMetadata( connName );
  QgsDataSourceUri::SslMode sslmode = settings.enumValue( key + u"/sslmode"_s, QgsDataSourceUri::SslPrefer );

  QString username;
  QString password;
  if ( settings.value( key + u"/saveUsername"_s ).toString() == "true"_L1 )
  {
    username = settings.value( key + u"/username"_s ).toString();
  }

  if ( settings.value( key + u"/savePassword"_s ).toString() == "true"_L1 )
  {
    password = settings.value( key + u"/password"_s ).toString();
  }

  // Old save setting
  if ( settings.contains( key + u"/save"_s ) )
  {
    username = settings.value( key + u"/username"_s ).toString();

    if ( settings.value( key + u"/save"_s ).toString() == "true"_L1 )
    {
      password = settings.value( key + u"/password"_s ).toString();
    }
  }

  QString authcfg = settings.value( key + u"/authcfg"_s ).toString();

  QgsDataSourceUri uri;
  if ( !service.isEmpty() )
  {
    uri.setConnection( service, database, username, password, sslmode, authcfg );
  }
  else
  {
    uri.setConnection( host, port, database, username, password, sslmode, authcfg );
  }
  uri.setUseEstimatedMetadata( estimatedMetadata );

  const QString sessionRole = QgsPostgresConn::sessionRole( connName );
  if ( !sessionRole.isEmpty() )
  {
    uri.setParam( "session_role", sessionRole );
  }

  return uri;
}

bool QgsPostgresConn::publicSchemaOnly( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/publicOnly", false ).toBool();
}

bool QgsPostgresConn::geometryColumnsOnly( const QString &connName )
{
  QgsSettings settings;

  return settings.value( "/PostgreSQL/connections/" + connName + "/geometryColumnsOnly", false ).toBool();
}

bool QgsPostgresConn::dontResolveType( const QString &connName )
{
  QgsSettings settings;

  return settings.value( "/PostgreSQL/connections/" + connName + "/dontResolveType", false ).toBool();
}

bool QgsPostgresConn::useEstimatedMetadata( const QString &connName )
{
  QgsSettings settings;

  return settings.value( "/PostgreSQL/connections/" + connName + "/estimatedMetadata", false ).toBool();
}

bool QgsPostgresConn::allowGeometrylessTables( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/allowGeometrylessTables", false ).toBool();
}

bool QgsPostgresConn::allowProjectsInDatabase( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/projectsInDatabase", false ).toBool();
}

bool QgsPostgresConn::allowRasterOverviewTables( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/allowRasterOverviewTables", true ).toBool();
}

QString QgsPostgresConn::schemaToRestrict( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/PostgreSQL/connections/"_s + connName + u"/schema"_s ).toString();
}

QString QgsPostgresConn::sessionRole( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/session_role" ).toString();
}

void QgsPostgresConn::deleteConnection( const QString &connName )
{
  QgsSettings settings;

  QString key = "/PostgreSQL/connections/" + connName;
  settings.remove( key + "/service" );
  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/sslmode" );
  settings.remove( key + "/publicOnly" );
  settings.remove( key + "/geometryColumnsOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/estimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/save" );
  settings.remove( key + "/authcfg" );
  settings.remove( key + "/projectsInDatabase" );
  settings.remove( key + "/metadataInDatabase" );
  settings.remove( key + "/dontResolveType" );
  settings.remove( key + "/session_role" );
  settings.remove( key + "/allowRasterOverviewTables" );
  settings.remove( key + "/schema" );
  settings.remove( key );
}

void QgsPostgresConn::duplicateConnection( const QString &src, const QString &dst )
{
  const QString key( u"/PostgreSQL/connections/"_s + src );
  const QString newKey( u"/PostgreSQL/connections/"_s + dst );

  QgsSettings settings;
  settings.setValue( newKey + u"/service"_s, settings.value( key + u"/service"_s ).toString() );
  settings.setValue( newKey + u"/host"_s, settings.value( key + u"/host"_s ).toString() );
  settings.setValue( newKey + u"/port"_s, settings.value( key + u"/port"_s ).toString() );
  settings.setValue( newKey + u"/database"_s, settings.value( key + u"/database"_s ).toString() );
  settings.setValue( newKey + u"/username"_s, settings.value( key + u"/username"_s ).toString() );
  settings.setValue( newKey + u"/password"_s, settings.value( key + u"/password"_s ).toString() );
  settings.setValue( newKey + u"/sslmode"_s, settings.value( key + u"/sslmode"_s ).toString() );
  settings.setValue( newKey + u"/publicOnly"_s, settings.value( key + u"/publicOnly"_s ).toBool() );
  settings.setValue( newKey + u"/geometryColumnsOnly"_s, settings.value( key + u"/geometryColumnsOnly"_s ).toBool() );
  settings.setValue( newKey + u"/allowGeometrylessTables"_s, settings.value( key + u"/allowGeometrylessTables"_s ).toBool() );
  settings.setValue( newKey + u"/estimatedMetadata"_s, settings.value( key + u"/estimatedMetadata"_s ).toBool() );
  settings.setValue( newKey + u"/projectsInDatabase"_s, settings.value( key + u"/projectsInDatabase"_s ).toBool() );
  settings.setValue( newKey + u"/metadataInDatabase"_s, settings.value( key + u"/metadataInDatabase"_s ).toBool() );
  settings.setValue( newKey + u"/dontResolveType"_s, settings.value( key + u"/dontResolveType"_s ).toString() );
  settings.setValue( newKey + u"/session_role"_s, settings.value( key + u"/session_role"_s ).toString() );
  settings.setValue( newKey + u"/saveUsername"_s, settings.value( key + u"/saveUsername"_s ).toString() );
  settings.setValue( newKey + u"/savePassword"_s, settings.value( key + u"/savePassword"_s ).toString() );
  settings.setValue( newKey + u"/authcfg"_s, settings.value( key + u"/authcfg"_s ).toString() );
  settings.setValue( newKey + u"/allowRasterOverviewTables"_s, settings.value( key + u"/allowRasterOverviewTables"_s ).toString() );
  settings.setValue( newKey + u"/schema"_s, settings.value( key + u"/schema"_s ).toString() );
  settings.sync();
}

bool QgsPostgresConn::allowMetadataInDatabase( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/PostgreSQL/connections/" + connName + "/metadataInDatabase", false ).toBool();
}

bool QgsPostgresConn::cancel()
{
  QMutexLocker locker( &mLock );
  PGcancel *c = ::PQgetCancel( mConn );
  if ( !c )
  {
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( tr( "PQgetCancel failed" ) ), tr( "PostGIS" ) );
    return false;
  }

  char errbuf[256];
  int res = ::PQcancel( c, errbuf, sizeof errbuf );
  ::PQfreeCancel( c );

  if ( !res )
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( errbuf ), tr( "PostGIS" ) );

  return res == 0;
}

QString QgsPostgresConn::currentDatabase() const
{
  QMutexLocker locker( &mLock );
  QString database;
  QString sql = "SELECT current_database()";
  QgsPostgresResult res( LoggedPQexec( u"QgsPostgresConn"_s, sql ) );

  if ( res.PQresultStatus() == PGRES_TUPLES_OK )
  {
    database = res.PQgetvalue( 0, 0 );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "SQL: %1\nresult: %2\nerror: %3\n" ).arg( sql ).arg( res.PQresultStatus() ).arg( res.PQresultErrorMessage() ), tr( "PostGIS" ) );
  }

  return database;
}

QgsCoordinateReferenceSystem QgsPostgresConn::sridToCrs( int srid )
{
  QgsCoordinateReferenceSystem crs;

  QMutexLocker locker( &mCrsCacheMutex );
  if ( mCrsCache.contains( srid ) )
    crs = mCrsCache.value( srid );
  else
  {
    QgsPostgresResult result( LoggedPQexec( u"QgsPostgresProvider"_s, u"SELECT auth_name, auth_srid, srtext, proj4text FROM spatial_ref_sys WHERE srid=%1"_s.arg( srid ) ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      if ( result.PQntuples() > 0 )
      {
        const QString authName = result.PQgetvalue( 0, 0 );
        const QString authSRID = result.PQgetvalue( 0, 1 );
        const QString srText = result.PQgetvalue( 0, 2 );
        bool ok = false;
        if ( authName == "EPSG"_L1 || authName == "ESRI"_L1 )
        {
          ok = crs.createFromUserInput( authName + ':' + authSRID );
        }
        if ( !ok && !srText.isEmpty() )
        {
          ok = crs.createFromUserInput( srText );
        }
        if ( !ok )
          crs = QgsCoordinateReferenceSystem::fromProj( result.PQgetvalue( 0, 3 ) );
      }
      mCrsCache.insert( srid, crs );
    }
  }
  return crs;
}

int QgsPostgresConn::crsToSrid( const QgsCoordinateReferenceSystem &crs )
{
  QMutexLocker locker( &mCrsCacheMutex );
  int srid = mCrsCache.key( crs );

  if ( srid > -1 )
    return srid;
  else
  {
    QStringList authParts = crs.authid().split( ':' );
    if ( authParts.size() != 2 )
      return -1;
    const QString authName = authParts.first();
    const QString authId = authParts.last();
    QgsPostgresResult result( PQexec( u"SELECT srid FROM spatial_ref_sys WHERE auth_name=%1 AND auth_srid=%2"_s.arg( quotedString( authName ), authId ) ) );

    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      int srid = result.PQgetvalue( 0, 0 ).toInt();
      mCrsCache.insert( srid, crs );
      return srid;
    }
  }

  return -1;
}

QString QgsPostgresConn::connectionInfo( const QgsDataSourceUri &uri, const bool expandAuthCfg )
{
  QString strUri = uri.connectionInfo( expandAuthCfg );
  if ( uri.hasParam( "session_role" ) )
  {
    strUri += " session_role=" + uri.param( "session_role" );
  }

  return strUri;
}
