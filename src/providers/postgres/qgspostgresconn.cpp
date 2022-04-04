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
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsmessagelog.h"
#include "qgscredentials.h"
#include "qgsfields.h"
#include "qgspgtablemodel.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgswkbtypes.h"
#include "qgssettings.h"
#include "qgsjsonutils.h"
#include "qgspostgresstringutils.h"
#include "qgspostgresconnpool.h"
#include "qgsvariantutils.h"

#include <QApplication>
#include <QStringList>
#include <QThread>

#include <climits>

#include <nlohmann/json.hpp>

// for htonl
#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

const int PG_DEFAULT_TIMEOUT = 30;

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

QgsPostgresConn *QgsPostgresConn::connectDb( const QString &conninfo, bool readonly, bool shared, bool transaction )
{
  QMap<QString, QgsPostgresConn *> &connections =
    readonly ? QgsPostgresConn::sConnectionsRO : QgsPostgresConn::sConnectionsRW;

  // This is called from may places where shared parameter cannot be forced to false (QgsVectorLayerExporter)
  // and which is run in a different thread (drag and drop in browser)
  if ( QApplication::instance()->thread() != QThread::currentThread() )
  {
    shared = false;
  }

  QgsPostgresConn *conn;

  if ( shared )
  {
    // sharing connection between threads is not safe
    // See https://github.com/qgis/QGIS/issues/21205
    Q_ASSERT( QApplication::instance()->thread() == QThread::currentThread() );

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
        .arg( readonly ? "readonly" : "read-write" )
        ,
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
      .arg( readonly ? "readonly" : "read-write" )
      ,
      2
    );
  }

  conn = new QgsPostgresConn( conninfo, readonly, shared, transaction );
  QgsDebugMsgLevel(
    QStringLiteral(
      "Created new (%4) connection %2 for %1%3"
    )
    .arg( conninfo )
    .arg( reinterpret_cast<std::uintptr_t>( conn ) )
    .arg( shared ? " (shared)" : "" )
    .arg( readonly ? "readonly" : "read-write" )
    ,
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
      .arg( readonly ? "readonly" : "read-write" )
      ,
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
      .arg( readonly ? "readonly" : "read-write" )
      ,
      2
    );
  }

  return conn;
}

static void noticeProcessor( void *arg, const char *message )
{
  Q_UNUSED( arg )
  QString msg( QString::fromUtf8( message ) );
  msg.chop( 1 );
  QgsMessageLog::logMessage( QObject::tr( "NOTICE: %1" ).arg( msg ), QObject::tr( "PostGIS" ) );
}

QgsPostgresConn::QgsPostgresConn( const QString &conninfo, bool readOnly, bool shared, bool transaction )
  : mRef( 1 )
  , mOpenCursors( 0 )
  , mConnInfo( conninfo )
  , mGeosAvailable( false )
  , mProjAvailable( false )
  , mTopologyAvailable( false )
  , mGotPostgisVersion( false )
  , mPostgresqlVersion( 0 )
  , mPostgisVersionMajor( 0 )
  , mPostgisVersionMinor( 0 )
  , mPointcloudAvailable( false )
  , mRasterAvailable( false )
  , mUseWkbHex( false )
  , mReadOnly( readOnly )
  , mSwapEndian( false )
  , mNextCursorId( 0 )
  , mShared( shared )
  , mTransaction( transaction )
{

  QgsDebugMsgLevel( QStringLiteral( "New PostgreSQL connection for " ) + conninfo, 2 );

  // expand connectionInfo
  QgsDataSourceUri uri( conninfo );
  QString expandedConnectionInfo = uri.connectionInfo( true );

  auto addDefaultTimeoutAndClientEncoding = []( QString & connectString )
  {
    if ( !connectString.contains( QStringLiteral( "connect_timeout=" ) ) )
    {
      // add default timeout
      QgsSettings settings;
      int timeout = settings.value( QStringLiteral( "PostgreSQL/default_timeout" ), PG_DEFAULT_TIMEOUT, QgsSettings::Providers ).toInt();
      connectString += QStringLiteral( " connect_timeout=%1" ).arg( timeout );
    }

    connectString += QLatin1String( " client_encoding='UTF-8'" );
  };
  addDefaultTimeoutAndClientEncoding( expandedConnectionInfo );

  mConn = PQconnectdb( expandedConnectionInfo.toUtf8() );

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
        QString errorMsg = tr( "Cannot set WriteOwner permission to cert: %0 to allow removing it" ).arg( file.fileName() );
        PQfinish();
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "PostGIS" ) );
        mRef = 0;
        return;
      }
      if ( !file.remove() )
      {
        QString errorMsg = tr( "Cannot remove cert: %0" ).arg( file.fileName() );
        PQfinish();
        QgsMessageLog::logMessage( tr( "Client security failure" ) + '\n' + errorMsg, tr( "PostGIS" ) );
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
      addDefaultTimeoutAndClientEncoding( connectString );
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
    QgsMessageLog::logMessage( tr( "Connection to database failed" ) + '\n' + errorMsg, tr( "PostGIS" ) );
    mRef = 0;
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "Connection to the database was successful" ), 2 );

  deduceEndian();

  /* Check to see if we have working PostGIS support */
  if ( !postgisVersion().isNull() )
  {
    /* Check to see if we have GEOS support and if not, warn the user about
       the problems they will see :) */
    QgsDebugMsgLevel( QStringLiteral( "Checking for GEOS support" ), 3 );

    if ( !hasGEOS() )
    {
      QgsMessageLog::logMessage( tr( "Your PostGIS installation has no GEOS support. Feature selection and identification will not work properly. Please install PostGIS with GEOS support (http://geos.refractions.net)" ), tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "GEOS support available!" ), 3 );
    }
  }

  if ( mPostgresqlVersion >= 90000 )
  {
    PQexecNR( QStringLiteral( "SET application_name='QGIS'" ) );
    PQexecNR( QStringLiteral( "SET extra_float_digits=3" ) );
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
      .arg( reinterpret_cast<std::uintptr_t>( this ) )
      ,
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
  QString sql = QStringLiteral( "SELECT attname, CASE WHEN typname in (%1) THEN 1 ELSE null END AS isSpatial FROM pg_attribute JOIN pg_type ON atttypid=pg_type.oid WHERE attrelid=regclass('%2.%3') AND NOT attisdropped AND attnum>0 ORDER BY attnum" )
                .arg( supportedSpatialTypes().join( ',' ) )
                .arg( quotedIdentifier( schemaName ),
                      quotedIdentifier( viewName ) );
  QgsDebugMsgLevel( "getting column info: " + sql, 2 );
  QgsPostgresResult colRes( PQexec( sql ) );

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

bool QgsPostgresConn::getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables, const QString &schema )
{
  QMutexLocker locker( &mLock );
  int nColumns = 0;
  int foundInTables = 0;
  QgsPostgresResult result;
  QString query;

  mLayersSupported.clear();

  for ( int i = SctGeometry; i <= SctRaster; ++i )
  {
    QString sql, tableName, schemaName, columnName, typeName, sridName, gtableName, dimName;

    if ( i == SctGeometry )
    {
      tableName  = QStringLiteral( "l.f_table_name" );
      schemaName = QStringLiteral( "l.f_table_schema" );
      columnName = QStringLiteral( "l.f_geometry_column" );
      typeName   = QStringLiteral( "upper(l.type)" );
      sridName   = QStringLiteral( "l.srid" );
      dimName    = QStringLiteral( "l.coord_dimension" );
      gtableName = QStringLiteral( "geometry_columns" );
    }
    // Geography since postgis 1.5
    else if ( i == SctGeography
              && ( mPostgisVersionMajor >= 2
                   || ( mPostgisVersionMajor == 1 && mPostgisVersionMinor >= 5 ) ) )
    {
      tableName  = QStringLiteral( "l.f_table_name" );
      schemaName = QStringLiteral( "l.f_table_schema" );
      columnName = QStringLiteral( "l.f_geography_column" );
      typeName   = QStringLiteral( "upper(l.type)" );
      sridName   = QStringLiteral( "l.srid" );
      dimName    = QStringLiteral( "2" );
      gtableName = QStringLiteral( "geography_columns" );
    }
    else if ( i == SctTopoGeometry )
    {
      if ( !hasTopology() )
        continue;

      schemaName = QStringLiteral( "l.schema_name" );
      tableName  = QStringLiteral( "l.table_name" );
      columnName = QStringLiteral( "l.feature_column" );
      typeName   = "CASE "
                   "WHEN l.feature_type = 1 THEN 'MULTIPOINT' "
                   "WHEN l.feature_type = 2 THEN 'MULTILINESTRING' "
                   "WHEN l.feature_type = 3 THEN 'MULTIPOLYGON' "
                   "WHEN l.feature_type = 4 THEN 'GEOMETRYCOLLECTION' "
                   "END AS type";
      sridName   = QStringLiteral( "(SELECT srid FROM topology.topology t WHERE l.topology_id=t.id)" );
      dimName    = QStringLiteral( "2" );
      gtableName = QStringLiteral( "topology.layer" );
    }
    else if ( i == SctPcPatch )
    {
      if ( !hasPointcloud() )
        continue;

      tableName  = QStringLiteral( "l.\"table\"" );
      schemaName = QStringLiteral( "l.\"schema\"" );
      columnName = QStringLiteral( "l.\"column\"" );
      typeName   = QStringLiteral( "'POLYGON'" );
      sridName   = QStringLiteral( "l.srid" );
      dimName    = QStringLiteral( "2" );
      gtableName = QStringLiteral( "pointcloud_columns" );
    }
    else if ( i == SctRaster )
    {
      if ( !hasRaster() )
        continue;

      tableName  = QStringLiteral( "l.\"r_table_name\"" );
      schemaName = QStringLiteral( "l.\"r_table_schema\"" );
      columnName = QStringLiteral( "l.\"r_raster_column\"" );
      typeName   = QStringLiteral( "'RASTER'" );
      sridName   = QStringLiteral( "l.srid" );
      dimName    = QStringLiteral( "2" );
      gtableName = QStringLiteral( "raster_columns" );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Unsupported spatial column type %1" )
                                 .arg( displayStringForGeomType( ( QgsPostgresGeometryColumnType )i ) ) );
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
          .arg( mPostgresqlVersion >= 90000 ? "array_agg(a.attname ORDER BY a.attnum)" : "(SELECT array_agg(attname) FROM (SELECT unnest(array_agg(a.attname)) AS attname ORDER BY unnest(array_agg(a.attnum))) AS attname)" )
          ;

    if ( searchPublicOnly )
      sql += QLatin1String( " AND n.nspname='public'" );

    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND %1='%2'" ).arg( schemaName, schema );

    sql += QString( " GROUP BY 1,2,3,4,5,6,7,c.oid,11" );

    foundInTables |= 1 << i;

    if ( ! query.isEmpty() )
      query += " UNION ";

    query += sql;
  }

  query += QLatin1String( " ORDER BY 2,1,3" );


  QgsDebugMsgLevel( "getting table info from layer registries: " + query, 2 );
  result = PQexec( query, true );
  // NOTE: we intentionally continue if the query fails
  //       (for example because PostGIS is not installed)
  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    QString tableName = result.PQgetvalue( idx, 0 );
    QString schemaName = result.PQgetvalue( idx, 1 );
    QString column = result.PQgetvalue( idx, 2 );
    QString type = result.PQgetvalue( idx, 3 );
    QString ssrid = result.PQgetvalue( idx, 4 );
    int dim = result.PQgetvalue( idx, 5 ).toInt();
    QString relkind = result.PQgetvalue( idx, 6 );
    bool isView = relkind == QLatin1String( "v" ) || relkind == QLatin1String( "m" );
    bool isMaterializedView = relkind == QLatin1String( "m" );
    bool isForeignTable = relkind == QLatin1String( "f" );
    bool isRaster = type == QLatin1String( "RASTER" );
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
      QgsDebugMsg( QStringLiteral( "Unhandled columnType index %1" )
                   .  arg( columnTypeInt ) );
    }

    int srid = ssrid.isEmpty() ? std::numeric_limits<int>::min() : ssrid.toInt();

    if ( ! isRaster && majorVersion() >= 2 && srid == 0 )
    {
      // 0 doesn't constraint => detect
      srid = std::numeric_limits<int>::min();
    }

#if 0
    QgsDebugMsgLevel( QStringLiteral( "%1 : %2.%3.%4: %5 %6 %7 %8" )
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
      type += QLatin1String( "ZM" );
    layerProperty.types = QList<QgsWkbTypes::Type>() << ( QgsPostgresConn::wkbTypeFromPostgis( type ) );
    layerProperty.srids = QList<int>() << srid;
    layerProperty.sql.clear();
    layerProperty.relKind = relkind;
    layerProperty.isView = isView;
    layerProperty.isForeignTable = isForeignTable;
    layerProperty.isRaster = isRaster;
    layerProperty.isMaterializedView = isMaterializedView;
    layerProperty.tableComment = comment;
    layerProperty.nSpCols = nSpCols;
    if ( isView || isForeignTable )
    {
      // TODO: use std::transform
      for ( const auto &a : QgsPostgresStringUtils::parseArray( attributes ) )
      {
        layerProperty.pkCols << a.toString();
      }
    }

    if ( isView && layerProperty.pkCols.empty() )
    {
      //QgsDebugMsgLevel( QStringLiteral( "no key columns found." ), 2 );
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
    if ( searchPublicOnly )
      sql += QLatin1String( " AND n.nspname='public'" );

    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND n.nspname='%2'" ).arg( schema );

    // skip columns of which we already derived information from the metadata tables
    if ( nColumns > 0 )
    {
      if ( foundInTables & ( 1 << SctGeometry ) )
      {
        sql += QLatin1String( " AND NOT EXISTS (SELECT 1 FROM geometry_columns WHERE n.nspname=f_table_schema AND c.relname=f_table_name AND a.attname=f_geometry_column)" );
      }

      if ( foundInTables & ( 1 << SctGeography ) )
      {
        sql += QLatin1String( " AND NOT EXISTS (SELECT 1 FROM geography_columns WHERE n.nspname=f_table_schema AND c.relname=f_table_name AND a.attname=f_geography_column)" );
      }

      if ( foundInTables & ( 1 << SctPcPatch ) )
      {
        sql += QLatin1String( " AND NOT EXISTS (SELECT 1 FROM pointcloud_columns WHERE n.nspname=\"schema\" AND c.relname=\"table\" AND a.attname=\"column\")" );
      }

      if ( foundInTables & ( 1 << SctRaster ) )
      {
        sql += QLatin1String( " AND NOT EXISTS (SELECT 1 FROM raster_columns WHERE n.nspname=\"r_table_schema\" AND c.relname=\"r_table_name\" AND a.attname=\"r_raster_column\")" );
      }

      if ( foundInTables & ( 1 << SctTopoGeometry ) )
      {
        sql += QLatin1String( " AND NOT EXISTS (SELECT 1 FROM topology.layer WHERE n.nspname=\"schema_name\" AND c.relname=\"table_name\" AND a.attname=\"feature_column\")" );
      }
    }

    QgsDebugMsgLevel( "getting spatial table info from pg_catalog: " + sql, 2 );

    result = PQexec( sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined. The error message from the database was:\n%1\n" )
                                 .arg( result.PQresultErrorMessage() ),
                                 tr( "PostGIS" ) );
      PQexecNR( QStringLiteral( "COMMIT" ) );
      return false;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      // Have the column name, schema name and the table name. The concept of a
      // catalog doesn't exist in PostgreSQL so we ignore that, but we
      // do need to get the geometry type.

      QString tableName  = result.PQgetvalue( i, 0 ); // relname
      QString schemaName = result.PQgetvalue( i, 1 ); // nspname
      QString column     = result.PQgetvalue( i, 2 ); // attname
      QString relkind    = result.PQgetvalue( i, 3 ); // relation kind
      QString coltype    = result.PQgetvalue( i, 4 ); // column type
      bool isView = relkind == QLatin1String( "v" ) || relkind == QLatin1String( "m" );
      bool isMaterializedView = relkind == QLatin1String( "m" );
      bool isForeignTable = relkind == QLatin1String( "f" );
      QString comment    = result.PQgetvalue( i, 5 ); // table comment

      QgsPostgresLayerProperty layerProperty;
      layerProperty.types = QList<QgsWkbTypes::Type>() << QgsWkbTypes::Unknown;
      layerProperty.srids = QList<int>() << std::numeric_limits<int>::min();
      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = column;
      layerProperty.relKind = relkind;
      layerProperty.isView = isView;
      layerProperty.isForeignTable = isForeignTable;
      layerProperty.isRaster = coltype == QLatin1String( "raster" );
      layerProperty.isMaterializedView = isMaterializedView;
      layerProperty.tableComment = comment;
      if ( coltype == QLatin1String( "geometry" ) )
      {
        layerProperty.geometryColType = SctGeometry;
      }
      else if ( coltype == QLatin1String( "geography" ) )
      {
        layerProperty.geometryColType = SctGeography;
      }
      else if ( coltype == QLatin1String( "topogeometry" ) )
      {
        layerProperty.geometryColType = SctTopoGeometry;
      }
      else if ( coltype == QLatin1String( "pcpatch" ) ||
                coltype == QLatin1String( "pcpoint" ) )
      {
        layerProperty.geometryColType = SctPcPatch;
      }
      else if ( coltype == QLatin1String( "raster" ) )
      {
        layerProperty.geometryColType = SctRaster;
      }
      else
      {
        Q_ASSERT( !"Unknown geometry type" );
      }

      // TODO: use knowledge from already executed query to count
      //       spatial fields and list attribute names...
      addColumnInfo( layerProperty, schemaName, tableName, isView || isForeignTable );

      if ( isView && layerProperty.pkCols.empty() )
      {
        //QgsDebugMsgLevel( QStringLiteral( "no key columns found." ), 2 );
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
    if ( searchPublicOnly )
      sql += QLatin1String( " AND pg_namespace.nspname='public'" );

    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND pg_namespace.nspname='%2'" ).arg( schema );

    sql += QLatin1String( " GROUP BY 1,2,3,4" );

    QgsDebugMsgLevel( "getting non-spatial table info: " + sql, 2 );

    result = PQexec( sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined.\nThe error message from the database was:\n%1" )
                                 .arg( result.PQresultErrorMessage() ),
                                 tr( "PostGIS" ) );
      return false;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      QString table   = result.PQgetvalue( i, 0 ); // relname
      QString schema  = result.PQgetvalue( i, 1 ); // nspname
      QString relkind = result.PQgetvalue( i, 2 ); // relation kind
      QString comment = result.PQgetvalue( i, 3 ); // table comment
      QString attributes = result.PQgetvalue( i, 4 ); // attributes array
      bool isView = relkind == QLatin1String( "v" ) || relkind == QLatin1String( "m" );
      bool isMaterializedView = relkind == QLatin1String( "m" );
      bool isForeignTable = relkind == QLatin1String( "f" );

      QgsPostgresLayerProperty layerProperty;
      layerProperty.types = QList<QgsWkbTypes::Type>() << QgsWkbTypes::NoGeometry;
      layerProperty.srids = QList<int>() << std::numeric_limits<int>::min();
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = QString();
      layerProperty.geometryColType = SctNone;
      layerProperty.nSpCols = 0;
      layerProperty.relKind = relkind;
      layerProperty.isView = isView;
      layerProperty.isForeignTable = isForeignTable;
      layerProperty.isRaster = false;
      layerProperty.isMaterializedView = isMaterializedView;
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

      if ( isView || isForeignTable )
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

  if ( nColumns == 0 && schema.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "PostGIS" ) );
  }

  return true;
}

bool QgsPostgresConn::supportedLayers( QVector<QgsPostgresLayerProperty> &layers, bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables, const QString &schema )
{
  QMutexLocker locker( &mLock );

  // Get the list of supported tables
  if ( !getTableInfo( searchGeometryColumnsOnly, searchPublicOnly, allowGeometrylessTables, schema ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "PostGIS" ) );
    return false;
  }

  layers = mLayersSupported;

  return true;
}

bool QgsPostgresConn::getSchemas( QList<QgsPostgresSchemaProperty> &schemas )
{
  schemas.clear();
  QgsPostgresResult result;

  QString sql = QStringLiteral( "SELECT nspname, pg_get_userbyid(nspowner), pg_catalog.obj_description(oid) FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema' ORDER BY nspname" );

  result = PQexec( sql, true );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    PQexecNR( QStringLiteral( "COMMIT" ) );
    return false;
  }

  for ( int idx = 0; idx < result.PQntuples(); idx++ )
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

  QgsPostgresResult result( PQexec( QStringLiteral( "SELECT postgis_version()" ), false ) );
  if ( result.PQntuples() != 1 )
  {
    QgsMessageLog::logMessage( tr( "No PostGIS support in the database." ), tr( "PostGIS" ) );
    mGotPostgisVersion = true;
    return QString();
  }

  mPostgisVersionInfo = result.PQgetvalue( 0, 0 );

  QgsDebugMsgLevel( "PostGIS version info: " + mPostgisVersionInfo, 2 );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QStringList postgisParts = mPostgisVersionInfo.split( ' ', QString::SkipEmptyParts );

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split( '.', QString::SkipEmptyParts );
#else
  QStringList postgisParts = mPostgisVersionInfo.split( ' ', Qt::SkipEmptyParts );

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split( '.', Qt::SkipEmptyParts );
#endif
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
    result = PQexec( QStringLiteral( "SELECT postgis_geos_version(), postgis_proj_version()" ) );
    mGeosAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 );
    mProjAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 1 );
    QgsDebugMsgLevel( QStringLiteral( "geos:%1 proj:%2" )
                      .arg( mGeosAvailable ? result.PQgetvalue( 0, 0 ) : "none" )
                      .arg( mProjAvailable ? result.PQgetvalue( 0, 1 ) : "none" ), 2 );
  }
  else
  {
    // assume no capabilities
    mGeosAvailable = false;

    // parse out the capabilities and store them
    QStringList geos = postgisParts.filter( QStringLiteral( "GEOS" ) );
    if ( geos.size() == 1 )
    {
      mGeosAvailable = ( geos[0].indexOf( QLatin1String( "=1" ) ) > -1 );
    }
  }

  // checking for topology support
  QgsDebugMsgLevel( QStringLiteral( "Checking for topology support" ), 2 );
  mTopologyAvailable = false;
  if ( mPostgisVersionMajor > 1 )
  {
    QgsPostgresResult result(
      PQexec(
        QStringLiteral(
          "SELECT has_schema_privilege(n.oid, 'usage')"
          " AND has_table_privilege(t.oid, 'select')"
          " AND has_table_privilege(l.oid, 'select')"
          " FROM pg_namespace n, pg_class t, pg_class l"
          " WHERE n.nspname = 'topology'"
          " AND t.relnamespace = n.oid"
          " AND l.relnamespace = n.oid"
          " AND t.relname = 'topology'"
          " AND l.relname = 'layer'"
        ) ) );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
    {
      mTopologyAvailable = true;
    }
  }

  if ( mTopologyAvailable )
  {
    QgsDebugMsgLevel( QStringLiteral( "Topology support available :)" ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Topology support not available :(" ), 2 );
  }

  mGotPostgisVersion = true;

  if ( mPostgresqlVersion >= 90000 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Checking for pointcloud support" ), 2 );
    result = PQexec( QStringLiteral(
                       "SELECT has_table_privilege(c.oid, 'select')"
                       " AND has_table_privilege(f.oid, 'select')"
                       " FROM pg_class c, pg_class f, pg_namespace n, pg_extension e"
                       " WHERE c.relnamespace = n.oid"
                       " AND c.relname = 'pointcloud_columns'"
                       " AND f.relnamespace = n.oid"
                       " AND f.relname = 'pointcloud_formats'"
                       " AND n.oid = e.extnamespace"
                       " AND e.extname = 'pointcloud'"
                     ), false );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
    {
      mPointcloudAvailable = true;
      QgsDebugMsgLevel( QStringLiteral( "Pointcloud support available!" ), 2 );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Checking for raster support" ), 2 );
  if ( mPostgisVersionMajor >= 2 )
  {
    result = PQexec( QStringLiteral(
                       "SELECT has_table_privilege(c.oid, 'select')"
                       " FROM pg_class c, pg_namespace n, pg_type t"
                       " WHERE c.relnamespace = n.oid"
                       " AND n.oid = t.typnamespace"
                       " AND c.relname = 'raster_columns'"
                       " AND t.typname = 'raster'"
                     ), false );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
    {
      mRasterAvailable = true;
      QgsDebugMsgLevel( QStringLiteral( "Raster support available!" ), 2 );
    }
  }

  return mPostgisVersionInfo;
}

QString QgsPostgresConn::quotedIdentifier( const QString &ident )
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

static QString doubleQuotedMapValue( const QString &v )
{
  QString result = v;
  return "\"" + result.replace( '\\', QLatin1String( "\\\\\\\\" ) ).replace( '\"', QLatin1String( "\\\\\"" ) ).replace( '\'', QLatin1String( "\\'" ) ) + "\"";
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
    ret.append( doubleQuotedMapValue( i.key() ) + "=>" +
                doubleQuotedMapValue( i.value().toString() ) );
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
    if ( inner.startsWith( '{' ) || i->type() == QVariant::Int || i->type() == QVariant::LongLong )
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
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
      return value.toString();

    case QVariant::DateTime:
      return quotedString( value.toDateTime().toString( Qt::ISODateWithMs ) );

    case QVariant::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    case QVariant::Map:
      return quotedMap( value.toMap() );

    case QVariant::StringList:
    case QVariant::List:
      return quotedList( value.toList() );

    case QVariant::Double:
    case QVariant::String:
    default:
      return quotedString( value.toString() );
  }
}

QString QgsPostgresConn::quotedJsonValue( const QVariant &value )
{
  if ( value.isNull() || !value.isValid() )
    return QStringLiteral( "null" );
  // where json is a string literal just construct it from that rather than dump
  if ( value.type() == QVariant::String )
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

PGresult *QgsPostgresConn::PQexec( const QString &query, bool logError, bool retry ) const
{
  QMutexLocker locker( &mLock );

  QgsDebugMsgLevel( QStringLiteral( "Executing SQL: %1" ).arg( query ), 3 );

  PGresult *res = ::PQexec( mConn, query.toUtf8() );

  // libpq may return a non null ptr with conn status not OK so we need to check for it to allow a retry below
  if ( res && PQstatus() == CONNECTION_OK )
  {
    int errorStatus = PQresultStatus( res );
    if ( errorStatus != PGRES_COMMAND_OK && errorStatus != PGRES_TUPLES_OK )
    {
      if ( logError )
      {
        QgsMessageLog::logMessage( tr( "Erroneous query: %1 returned %2 [%3]" )
                                   .arg( query ).arg( errorStatus ).arg( PQresultErrorMessage( res ) ),
                                   tr( "PostGIS" ) );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Not logged erroneous query: %1 returned %2 [%3]" )
                     .arg( query ).arg( errorStatus ).arg( PQresultErrorMessage( res ) ) );
      }
    }
    return res;
  }
  if ( PQstatus() != CONNECTION_OK )
  {
    if ( logError )
    {
      QgsMessageLog::logMessage( tr( "Connection error: %1 returned %2 [%3]" )
                                 .arg( query ).arg( PQstatus() ).arg( PQerrorMessage() ),
                                 tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Connection error: %1 returned %2 [%3]" )
                   .arg( query ).arg( PQstatus() ).arg( PQerrorMessage() ) );
    }
  }
  else
  {
    if ( logError )
    {
      QgsMessageLog::logMessage( tr( "Query failed: %1\nError: no result buffer" ).arg( query ), tr( "PostGIS" ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Not logged query failed: %1\nError: no result buffer" ).arg( query ) );
    }
  }

  if ( retry )
  {
    QgsMessageLog::logMessage( tr( "resetting bad connection." ), tr( "PostGIS" ) );
    ::PQreset( mConn );
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
        QgsMessageLog::logMessage( tr( "retry after reset failed again." ), tr( "PostGIS" ) );
        return nullptr;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "connection still bad after reset." ), tr( "PostGIS" ) );
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
  auto cancel = ::PQgetCancel( mConn ) ;
  if ( cancel )
  {
    char errbuf[255];
    result = ::PQcancel( cancel, errbuf, 255 );
    if ( ! result )
      QgsDebugMsgLevel( QStringLiteral( "Error canceling the query:" ).arg( errbuf ), 3 );
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
    QgsDebugMsgLevel( QStringLiteral( "Starting read-only transaction: %1" ).arg( mPostgresqlVersion ), 4 );
    if ( mPostgresqlVersion >= 80000 )
      preStr = QStringLiteral( "BEGIN READ ONLY;" );
    else
      preStr = QStringLiteral( "BEGIN;" );
  }
  QgsDebugMsgLevel( QStringLiteral( "Binary cursor %1 for %2" ).arg( cursorName, sql ), 3 );
  return PQexecNR( QStringLiteral( "%1DECLARE %2 BINARY CURSOR%3 FOR %4" ).
                   arg( preStr, cursorName, !mTransaction ? QString() : QStringLiteral( " WITH HOLD" ), sql ) );
}

bool QgsPostgresConn::closeCursor( const QString &cursorName )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors
  QString postStr;

  if ( --mOpenCursors == 0 && !mTransaction )
  {
    QgsDebugMsgLevel( QStringLiteral( "Committing read-only transaction" ), 4 );
    postStr = QStringLiteral( ";COMMIT" );
  }

  if ( !PQexecNR( QStringLiteral( "CLOSE %1%2" ).arg( cursorName, postStr ) ) )
    return false;

  return true;
}

QString QgsPostgresConn::uniqueCursorName()
{
  QMutexLocker locker( &mLock ); // to protect access to mNextCursorId
  return QStringLiteral( "qgis_%1" ).arg( ++mNextCursorId );
}

bool QgsPostgresConn::PQexecNR( const QString &query )
{
  QMutexLocker locker( &mLock ); // to protect access to mOpenCursors

  QgsPostgresResult res( PQexec( query, false ) );

  ExecStatusType errorStatus = res.PQresultStatus();
  if ( errorStatus == PGRES_COMMAND_OK )
    return true;

  QgsMessageLog::logMessage( tr( "Query: %1 returned %2 [%3]" )
                             .arg( query )
                             .arg( errorStatus )
                             .arg( res.PQresultErrorMessage() ),
                             tr( "PostGIS" ) );

  if ( mOpenCursors )
  {
    QgsMessageLog::logMessage( tr( "%1 cursor states lost.\nSQL: %2\nResult: %3 (%4)" )
                               .arg( mOpenCursors ).arg( query ).arg( errorStatus )
                               .arg( res.PQresultErrorMessage() ), tr( "PostGIS" ) );
    mOpenCursors = 0;
  }

  if ( PQstatus() == CONNECTION_OK )
  {
    PQexecNR( QStringLiteral( "ROLLBACK" ) );
  }

  return false;
}

PGresult *QgsPostgresConn::PQgetResult()
{
  return ::PQgetResult( mConn );
}

PGresult *QgsPostgresConn::PQprepare( const QString &stmtName, const QString &query, int nParams, const Oid *paramTypes )
{
  QMutexLocker locker( &mLock );

  return ::PQprepare( mConn, stmtName.toUtf8(), query.toUtf8(), nParams, paramTypes );
}

PGresult *QgsPostgresConn::PQexecPrepared( const QString &stmtName, const QStringList &params )
{
  QMutexLocker locker( &mLock );

  const char **param = new const char *[ params.size()];
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

  delete [] param;

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
    return PQexecNR( QStringLiteral( "SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return PQexecNR( QStringLiteral( "BEGIN" ) );
  }
}

bool QgsPostgresConn::commit()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return PQexecNR( QStringLiteral( "RELEASE SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return PQexecNR( QStringLiteral( "COMMIT" ) );
  }
}

bool QgsPostgresConn::rollback()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return PQexecNR( QStringLiteral( "ROLLBACK TO SAVEPOINT transaction_savepoint" ) )
           && PQexecNR( QStringLiteral( "RELEASE SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return PQexecNR( QStringLiteral( "ROLLBACK" ) );
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
      buf += QStringLiteral( "%1 " ).arg( *( unsigned char * )( p + i ), 0, 16, QLatin1Char( ' ' ) );
    }

    QgsDebugMsgLevel( QStringLiteral( "int in hex:%1" ).arg( buf ), 2 );
  }
#endif

  switch ( s )
  {
    case 2:
      oid = *( quint16 * )p;
      if ( mSwapEndian )
        oid = ntohs( oid );
      /* cast to signed 16bit
       * See https://github.com/qgis/QGIS/issues/22258 */
      oid = ( qint16 )oid;
      break;

    case 6:
    {
      quint64 block  = *( quint32 * ) p;
      quint64 offset = *( quint16 * )( p + sizeof( quint32 ) );

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
      quint32 oid1 = *( quint32 * )( p + sizeof( quint32 ) );

      if ( mSwapEndian )
      {
        QgsDebugMsgLevel( QStringLiteral( "swap oid0:%1 oid1:%2" ).arg( oid0 ).arg( oid1 ), 4 );
        oid0 = ntohl( oid0 );
        oid1 = ntohl( oid1 );
      }

      QgsDebugMsgLevel( QStringLiteral( "oid0:%1 oid1:%2" ).arg( oid0 ).arg( oid1 ), 4 );
      oid   = oid0;
      QgsDebugMsgLevel( QStringLiteral( "oid:%1" ).arg( oid ), 4 );
      oid <<= 32;
      QgsDebugMsgLevel( QStringLiteral( "oid:%1" ).arg( oid ), 4 );
      oid  |= oid1;
      QgsDebugMsgLevel( QStringLiteral( "oid:%1" ).arg( oid ), 4 );
    }
    break;

    default:
      QgsDebugMsg( QStringLiteral( "unexpected size %1" ).arg( s ) );
      //intentional fall-through
      FALLTHROUGH
    case 4:
      oid = *( quint32 * )p;
      if ( mSwapEndian )
        oid = ntohl( oid );
      /* cast to signed 32bit
       * See https://github.com/qgis/QGIS/issues/22258 */
      oid = ( qint32 )oid;
      break;
  }

  return oid;
}

QString QgsPostgresConn::fieldExpressionForWhereClause( const QgsField &fld, QVariant::Type valueType, QString expr )
{
  QString out;
  const QString &type = fld.typeName();

  if ( type == QLatin1String( "timestamp" ) || type == QLatin1String( "time" ) || type == QLatin1String( "date" ) )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // if field and value havev incompatible types, rollback to text cast
    if ( valueType !=  QVariant::LastType && valueType != QVariant::DateTime && valueType != QVariant::Date && valueType != QVariant::Time )
    {
      out = out + "::text";
    }
  }

  else if ( type == QLatin1String( "int8" ) || type == QLatin1String( "serial8" ) //
            || type == QLatin1String( "int2" ) || type == QLatin1String( "int4" ) || type == QLatin1String( "oid" ) || type == QLatin1String( "serial" ) //
            || type == QLatin1String( "real" ) || type == QLatin1String( "double precision" ) || type == QLatin1String( "float4" ) || type == QLatin1String( "float8" ) //
            || type == QLatin1String( "numeric" ) )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // if field and value havev incompatible types, rollback to text cast
    if ( valueType !=  QVariant::LastType && valueType != QVariant::Int && valueType != QVariant::LongLong && valueType != QVariant::Double )
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
  if ( type == QLatin1String( "money" ) )
  {
    return QStringLiteral( "cash_out(%1)::text" ).arg( expr );
  }
  else if ( type.startsWith( '_' ) )
  {
    //TODO: add native support for arrays
    return QStringLiteral( "array_out(%1)::text" ).arg( expr );
  }
  else if ( type == QLatin1String( "bool" ) )
  {
    return QStringLiteral( "boolout(%1)::text" ).arg( expr );
  }
  else if ( type == QLatin1String( "geometry" ) )
  {
    return QStringLiteral( "%1(%2)" )
           .arg( majorVersion() < 2 ? "asewkt" : "st_asewkt",
                 expr );
  }
  else if ( type == QLatin1String( "geography" ) )
  {
    return QStringLiteral( "st_astext(%1)" ).arg( expr );
  }
  else if ( type == QLatin1String( "int8" ) )
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

  types     // integer types
      << QgsVectorDataProvider::NativeType( tr( "Whole Number (smallint - 16bit)" ), QStringLiteral( "int2" ), QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 32bit)" ), QStringLiteral( "int4" ), QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 64bit)" ), QStringLiteral( "int8" ), QVariant::LongLong, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal Number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal Number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

      // floating point
      << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), QStringLiteral( "real" ), QVariant::Double, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal Number (double)" ), QStringLiteral( "double precision" ), QVariant::Double, -1, -1, -1, -1 )

      // string types
      << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QVariant::String, 1, 255, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ), QVariant::String, 1, 255, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, case-insensitive unlimited length (citext)" ), QStringLiteral( "citext" ), QVariant::String, -1, -1, -1, -1 )

      // date type
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Date ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Time ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), QStringLiteral( "timestamp without time zone" ), QVariant::DateTime, -1, -1, -1, -1 )

      // complex types
      << QgsVectorDataProvider::NativeType( tr( "Map (hstore)" ), QStringLiteral( "hstore" ), QVariant::Map, -1, -1, -1, -1, QVariant::String )
      << QgsVectorDataProvider::NativeType( tr( "Array of Number (integer - 32bit)" ), QStringLiteral( "int4[]" ), QVariant::List, -1, -1, -1, -1, QVariant::Int )
      << QgsVectorDataProvider::NativeType( tr( "Array of Number (integer - 64bit)" ), QStringLiteral( "int8[]" ), QVariant::List, -1, -1, -1, -1, QVariant::LongLong )
      << QgsVectorDataProvider::NativeType( tr( "Array of Number (double)" ), QStringLiteral( "double precision[]" ), QVariant::List, -1, -1, -1, -1, QVariant::Double )
      << QgsVectorDataProvider::NativeType( tr( "Array of Text" ), QStringLiteral( "text[]" ), QVariant::StringList, -1, -1, -1, -1, QVariant::String )

      // boolean
      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Bool ), QStringLiteral( "bool" ), QVariant::Bool, -1, -1, -1, -1 )

      // binary (bytea)
      << QgsVectorDataProvider::NativeType( tr( "Binary Object (bytea)" ), QStringLiteral( "bytea" ), QVariant::ByteArray, -1, -1, -1, -1 )
      ;

  if ( pgVersion() >= 90200 )
  {
    types << QgsVectorDataProvider::NativeType( tr( "JSON (json)" ), QStringLiteral( "json" ), QVariant::Map, -1, -1, -1, -1, QVariant::String );

    if ( pgVersion() >= 90400 )
    {
      types << QgsVectorDataProvider::NativeType( tr( "JSON (jsonb)" ), QStringLiteral( "jsonb" ), QVariant::Map, -1, -1, -1, -1, QVariant::String );
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
  int queryCounter = 0;
  int errorCounter = 0;
  int oidStatus = 0;
  int oidSelectSet = 1 << 0;
  int oidBinaryCursorSet = 1 << 1;
  qint64 oidSelect = 0;
  qint64 oidBinaryCursor = 0;

  if ( 0 == PQsendQuery( QStringLiteral(
                           "SELECT regclass('pg_class')::oid AS oidselect;"
                           "BEGIN;"
                           "DECLARE oidcursor BINARY CURSOR FOR SELECT regclass('pg_class')::oid AS oidbinarycursor;"
                           "FETCH FORWARD 1 FROM oidcursor;"
                           "CLOSE oidcursor;"
                           "COMMIT;" ) ) )
    QgsDebugMsgLevel( QStringLiteral( "PQsendQuery(...) error %1" ).arg( PQerrorMessage() ), 2 );

  for ( ;; )
  {
    // PQgetResult() must be called repeatedly until it returns a null pointer
    resOID = PQgetResult();

    if ( resOID.result() == nullptr )
      break;

    queryCounter++;
    if ( resOID.PQresultStatus() == PGRES_FATAL_ERROR )
    {
      errorCounter++;
      QgsDebugMsgLevel( QStringLiteral( "QUERY #%1 PGRES_FATAL_ERROR %2" )
                        .arg( queryCounter )
                        .arg( PQerrorMessage().trimmed() ), 2 );
      continue;
    }

    if ( resOID.PQresultStatus() == PGRES_TUPLES_OK && resOID.PQnfields() && resOID.PQntuples() )
    {
      if ( resOID.PQfname( 0 ) == QLatin1String( "oidselect" ) )
      {
        oidSelect = resOID.PQgetvalue( 0, 0 ).toLongLong();
        oidStatus |= oidSelectSet;
      }
      if ( resOID.PQfname( 0 ) == QLatin1String( "oidbinarycursor" ) )
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

  QgsDebugMsgLevel( QStringLiteral( "Back to old deduceEndian(): PQstatus() - %1, queryCounter = %2, errorCounter = %3" )
                    .arg( PQstatus() )
                    .arg( queryCounter )
                    .arg( errorCounter ), 2 );

  QgsPostgresResult res( PQexec( QStringLiteral( "select regclass('pg_class')::oid" ) ) );
  QString oidValue = res.PQgetvalue( 0, 0 );

  QgsDebugMsgLevel( QStringLiteral( "Creating binary cursor" ), 2 );

  // get the same value using a binary cursor
  openCursor( QStringLiteral( "oidcursor" ), QStringLiteral( "select regclass('pg_class')::oid" ) );

  QgsDebugMsgLevel( QStringLiteral( "Fetching a record and attempting to get check endian-ness" ), 2 );

  res = PQexec( QStringLiteral( "fetch forward 1 from oidcursor" ) );

  mSwapEndian = true;
  if ( res.PQntuples() > 0 )
  {
    // get the oid value from the binary cursor
    qint64 oid = getBinaryInt( res, 0, 0 );

    QgsDebugMsgLevel( QStringLiteral( "Got oid of %1 from the binary cursor" ).arg( oid ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "First oid is %1" ).arg( oidValue ), 2 );

    // compare the two oid values to determine if we need to do an endian swap
    if ( oid != oidValue.toLongLong() )
      mSwapEndian = false;
  }

  closeCursor( QStringLiteral( "oidcursor" ) );
}

void QgsPostgresConn::retrieveLayerTypes( QgsPostgresLayerProperty &layerProperty, bool useEstimatedMetadata )
{
  QVector<QgsPostgresLayerProperty *> vect;
  vect << &layerProperty;
  retrieveLayerTypes( vect, useEstimatedMetadata );
}

void QgsPostgresConn::retrieveLayerTypes( QVector<QgsPostgresLayerProperty *> &layerProperties, bool useEstimatedMetadata )
{
  QString table;
  QString query;

  // Limit table row scan if useEstimatedMetadata
  const QString tableScanLimit { useEstimatedMetadata ? QStringLiteral( " LIMIT %1" ).arg( GEOM_TYPE_SELECT_LIMIT ) : QString() };

  int i = 0;
  for ( auto *layerPropertyPtr : layerProperties )
  {
    QgsPostgresLayerProperty &layerProperty = *layerPropertyPtr;

    if ( i++ )
      query += " UNION ";

    if ( !layerProperty.schemaName.isEmpty() )
    {
      table = QStringLiteral( "%1.%2" )
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

    if ( layerProperty.isRaster )
    {
      QString sql;

      int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
      // SRID is already known
      if ( srid != std::numeric_limits<int>::min() )
      {
        sql += QStringLiteral( "SELECT %1, array_agg( '%2:RASTER:-1'::text )" )
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
                                "%2 IS NOT NULL "
                                "%4"   // SQL clause
                                "%5" )
                .arg( i - 1 )
                .arg( quotedIdentifier( layerProperty.geometryColName ) )
                .arg( table )
                .arg( layerProperty.sql.isEmpty() ? QString() : QStringLiteral( " AND %1" ).arg( layerProperty.sql ) )
                .arg( tableScanLimit );
        }
      }

      QgsDebugMsgLevel( "Raster srids query: " + sql, 2 );
      query += sql;
    }
    else  // vectors
    {
      // our estimation ignores that a where clause might restrict the feature type or srid
      if ( useEstimatedMetadata )
      {
        table = QStringLiteral( "(SELECT %1 FROM %2 WHERE %3%1 IS NOT NULL%4) AS t" )
                .arg( quotedIdentifier( layerProperty.geometryColName ),
                      table,
                      layerProperty.sql.isEmpty() ? QString() : QStringLiteral( " (%1) AND " ).arg( layerProperty.sql ) )
                .arg( tableScanLimit );
      }
      else if ( !layerProperty.sql.isEmpty() )
      {
        table += QStringLiteral( " WHERE %1" ).arg( layerProperty.sql );
      }

      QString sql = QStringLiteral( "SELECT %1, " ).arg( i - 1 );

      bool castToGeometry = layerProperty.geometryColType == SctGeography ||
                            layerProperty.geometryColType == SctPcPatch;

      sql += QLatin1String( "array_agg(DISTINCT " );

      int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
      if ( srid == std::numeric_limits<int>::min() )
      {
        sql += QStringLiteral( "%1(%2%3)::text" )
               .arg( majorVersion() < 2 ? "srid" : "st_srid",
                     quotedIdentifier( layerProperty.geometryColName ),
                     castToGeometry ?  "::geometry" : "" );
      }
      else
      {
        sql += QStringLiteral( "%1::text" )
               .arg( QString::number( srid ) );
      }

      sql += " || ':' || ";

      QgsWkbTypes::Type type = layerProperty.types.value( 0, QgsWkbTypes::Unknown );
      if ( type == QgsWkbTypes::Unknown )
      {
        // Note that we would like to apply a "LIMIT GEOM_TYPE_SELECT_LIMIT"
        // here, so that the previous "array_agg(DISTINCT" does not scan the
        // full table. However SQL does not allow that.
        // So we have to do a subselect on the table to add the LIMIT,
        // see comment in the following code.
        sql += QStringLiteral( "UPPER(geometrytype(%1%2))  || ':' || ST_Zmflag(%1%2)" )
               .arg( quotedIdentifier( layerProperty.geometryColName ),
                     castToGeometry ?  "::geometry" : "" );
      }
      else
      {
        sql += QStringLiteral( "%1::text  || ':-1'" )
               .arg( quotedValue( QgsPostgresConn::postgisWkbTypeName( type ) ) );
      }


      sql += QLatin1String( ") " );

      if ( type == QgsWkbTypes::Unknown )
      {
        // Subselect to limit the "array_agg(DISTINCT", see previous comment.
        sql += QStringLiteral( " FROM (SELECT %1 FROM %2%3) AS _unused" )
               .arg( quotedIdentifier( layerProperty.geometryColName ) )
               .arg( table )
               .arg( tableScanLimit );
      }
      else
      {
        sql += " FROM " + table;
      }

      QgsDebugMsgLevel( "Geometry types,srids and dims query: " + sql, 2 );

      query += sql;
    }
  }

  QgsDebugMsgLevel( "Layer types,srids and dims query: " + query, 3 );

  QgsPostgresResult res( PQexec( query ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    // TODO: print some error here ?
    return;
  }

  for ( int i = 0; i < res.PQntuples(); i++ )
  {
    int idx = res.PQgetvalue( i, 0 ).toInt();
    auto srids_and_types = QgsPostgresStringUtils::parseArray( res.PQgetvalue( i, 1 ) );
    QgsPostgresLayerProperty &layerProperty = *layerProperties[idx];

    QgsDebugMsgLevel( QStringLiteral(
                        "Layer %1.%2.%3 has %4 srid/type combinations"
                      )
                      .arg( layerProperty.schemaName,
                            layerProperty.tableName,
                            layerProperty.geometryColName )
                      .arg( srids_and_types.length() )
                      , 3
                    );

    /* Gather found types */
    QList< std::pair<QgsWkbTypes::Type, int> > foundCombinations;
    for ( const auto &sridAndTypeVariant : srids_and_types )
    {
      QString sridAndTypeString = sridAndTypeVariant.toString();

      QgsDebugMsgLevel( QStringLiteral(
                          "Analyzing layer's %1.%2.%3 sridAndType %4"
                          " against %6 found combinations"
                        )
                        .arg( layerProperty.schemaName,
                              layerProperty.tableName,
                              layerProperty.geometryColName )
                        .arg( sridAndTypeString )
                        .arg( foundCombinations.length() )
                        , 3
                      );

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
          typeString.append( 'M' );
          break;
        case 2:
          typeString.append( 'Z' );
          break;
        case 3:
          typeString.append( QStringLiteral( "ZM" ) );
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
          QgsDebugMsgLevel( QStringLiteral(
                              "Upgrading type[%1] of layer %2.%3.%4 "
                              "to multiCurved type %5" )
                            .arg( j )
                            .arg( layerProperty.schemaName,
                                  layerProperty.tableName,
                                  layerProperty.geometryColName )
                            .arg( multiCurveType ), 3
                          );
          foundCombinations[j].first = multiCurveType;
          break;
        }
        else if ( multiType == knownMultiType )
        {
          QgsDebugMsgLevel( QStringLiteral(
                              "Upgrading type[%1] of layer %2.%3.%4 "
                              "to multi type %5" )
                            .arg( j )
                            .arg( layerProperty.schemaName,
                                  layerProperty.tableName,
                                  layerProperty.geometryColName )
                            .arg( multiType ), 3
                          );
          foundCombinations[j].first = multiType;
          break;
        }
        else if ( curveType == knownCurveType )
        {
          QgsDebugMsgLevel( QStringLiteral(
                              "Upgrading type[%1] of layer %2.%3.%4 "
                              "to curved type %5" )
                            .arg( j )
                            .arg( layerProperty.schemaName,
                                  layerProperty.tableName,
                                  layerProperty.geometryColName )
                            .arg( multiType ), 3
                          );
          foundCombinations[j].first = curveType;
          break;
        }
      }

      if ( j < foundCombinations.length() )
      {
        QgsDebugMsgLevel( QStringLiteral(
                            "Pre-existing compatible combination %1/%2 "
                            "found for layer %3.%4.%5 "
                          )
                          .arg( j ) .arg( foundCombinations.length() )
                          .arg( layerProperty.schemaName,
                                layerProperty.tableName,
                                layerProperty.geometryColName ), 3
                        );
        continue; // already found
      }

      QgsDebugMsgLevel( QStringLiteral(
                          "Setting typeSridCombination[%1] of layer %2.%3.%4 "
                          "to srid %5 and type %6" )
                        .arg( j )
                        .arg( layerProperty.schemaName,
                              layerProperty.tableName,
                              layerProperty.geometryColName )
                        .arg( srid )
                        .arg( type ), 3
                      );

      foundCombinations << std::make_pair( type, srid );
    }

    QgsDebugMsgLevel( QStringLiteral(
                        "Completed scan of %1 srid/type combinations "
                        "for layer of layer %2.%3.%4 " )
                      .arg( srids_and_types.length() )
                      .arg( layerProperty.schemaName,
                            layerProperty.tableName,
                            layerProperty.geometryColName ), 2
                    );

    /* Rewrite srids and types to match found combinations
     * of srids and types */
    layerProperty.srids.clear();
    layerProperty.types.clear();
    for ( const auto &comb : foundCombinations )
    {
      layerProperty.types << comb.first;
      layerProperty.srids << comb.second;
    }
    QgsDebugMsgLevel( QStringLiteral(
                        "Final layer %1.%2.%3 types: %4" )
                      .arg( layerProperty.schemaName,
                            layerProperty.tableName,
                            layerProperty.geometryColName )
                      .arg( layerProperty.types.length() ), 2
                    );
    QgsDebugMsgLevel( QStringLiteral(
                        "Final layer %1.%2.%3 srids: %4" )
                      .arg( layerProperty.schemaName,
                            layerProperty.tableName,
                            layerProperty.geometryColName )
                      .arg( layerProperty.srids.length() ), 2
                    );
  }
}

void QgsPostgresConn::postgisWkbType( QgsWkbTypes::Type wkbType, QString &geometryType, int &dim )
{
  dim = 2;
  QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( wkbType );
  switch ( flatType )
  {
    case QgsWkbTypes::Point:
      geometryType = QStringLiteral( "POINT" );
      break;

    case QgsWkbTypes::LineString:
      geometryType = QStringLiteral( "LINESTRING" );
      break;

    case QgsWkbTypes::Polygon:
      geometryType = QStringLiteral( "POLYGON" );
      break;

    case QgsWkbTypes::MultiPoint:
      geometryType = QStringLiteral( "MULTIPOINT" );
      break;

    case QgsWkbTypes::MultiLineString:
      geometryType = QStringLiteral( "MULTILINESTRING" );
      break;

    case QgsWkbTypes::MultiPolygon:
      geometryType = QStringLiteral( "MULTIPOLYGON" );
      break;

    case QgsWkbTypes::CircularString:
      geometryType = QStringLiteral( "CIRCULARSTRING" );
      break;

    case QgsWkbTypes::CompoundCurve:
      geometryType = QStringLiteral( "COMPOUNDCURVE" );
      break;

    case QgsWkbTypes::CurvePolygon:
      geometryType = QStringLiteral( "CURVEPOLYGON" );
      break;

    case QgsWkbTypes::MultiCurve:
      geometryType = QStringLiteral( "MULTICURVE" );
      break;

    case QgsWkbTypes::MultiSurface:
      geometryType = QStringLiteral( "MULTISURFACE" );
      break;

    case QgsWkbTypes::Unknown:
      geometryType = QStringLiteral( "GEOMETRY" );
      break;

    case QgsWkbTypes::NoGeometry:
    default:
      dim = 0;
      break;
  }

  if ( QgsWkbTypes::hasZ( wkbType ) && QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1String( "ZM" );
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
  else if ( wkbType >= QgsWkbTypes::Point25D && wkbType <= QgsWkbTypes::MultiPolygon25D )
  {
    dim = 3;
  }
}

QString QgsPostgresConn::postgisWkbTypeName( QgsWkbTypes::Type wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return geometryType;
}

QString QgsPostgresConn::postgisTypeFilter( QString geomCol, QgsWkbTypes::Type wkbType, bool castToGeometry )
{
  geomCol = quotedIdentifier( geomCol );
  if ( castToGeometry )
    geomCol += QLatin1String( "::geometry" );

  QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      return QStringLiteral( "upper(geometrytype(%1)) IN ('POINT','POINTZ','POINTM','POINTZM','MULTIPOINT','MULTIPOINTZ','MULTIPOINTM','MULTIPOINTZM')" ).arg( geomCol );
    case QgsWkbTypes::LineGeometry:
      return QStringLiteral( "upper(geometrytype(%1)) IN ('LINESTRING','LINESTRINGZ','LINESTRINGM','LINESTRINGZM','CIRCULARSTRING','CIRCULARSTRINGZ','CIRCULARSTRINGM','CIRCULARSTRINGZM','COMPOUNDCURVE','COMPOUNDCURVEZ','COMPOUNDCURVEM','COMPOUNDCURVEZM','MULTILINESTRING','MULTILINESTRINGZ','MULTILINESTRINGM','MULTILINESTRINGZM','MULTICURVE','MULTICURVEZ','MULTICURVEM','MULTICURVEZM')" ).arg( geomCol );
    case QgsWkbTypes::PolygonGeometry:
      return QStringLiteral( "upper(geometrytype(%1)) IN ('POLYGON','POLYGONZ','POLYGONM','POLYGONZM','CURVEPOLYGON','CURVEPOLYGONZ','CURVEPOLYGONM','CURVEPOLYGONZM','MULTIPOLYGON','MULTIPOLYGONZ','MULTIPOLYGONM','MULTIPOLYGONZM','MULTIPOLYGONM','MULTISURFACE','MULTISURFACEZ','MULTISURFACEM','MULTISURFACEZM','POLYHEDRALSURFACE','TIN')" ).arg( geomCol );
    case QgsWkbTypes::NullGeometry:
      return QStringLiteral( "geometrytype(%1) IS NULL" ).arg( geomCol );
    default: //unknown geometry
      return QString();
  }
}

int QgsPostgresConn::postgisWkbTypeDim( QgsWkbTypes::Type wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return dim;
}

QgsWkbTypes::Type QgsPostgresConn::wkbTypeFromPostgis( const QString &type )
{
  // Polyhedral surfaces and TIN are stored in PostGIS as geometry collections
  // of Polygons and Triangles.
  // So, since QGIS does not natively support PS and TIN, but we would like to open them if possible,
  // we consider them as multipolygons. WKB will be converted by the feature iterator
  if ( ( type == QLatin1String( "POLYHEDRALSURFACE" ) ) || ( type == QLatin1String( "TIN" ) ) )
  {
    return QgsWkbTypes::MultiPolygon;
  }
  else if ( ( type == QLatin1String( "POLYHEDRALSURFACEZ" ) ) || ( type == QLatin1String( "TINZ" ) ) )
  {
    return QgsWkbTypes::MultiPolygonZ;
  }
  else if ( ( type == QLatin1String( "POLYHEDRALSURFACEM" ) ) || ( type == QLatin1String( "TINM" ) ) )
  {
    return QgsWkbTypes::MultiPolygonM;
  }
  else if ( ( type == QLatin1String( "POLYHEDRALSURFACEZM" ) ) || ( type == QLatin1String( "TINZM" ) ) )
  {
    return QgsWkbTypes::MultiPolygonZM;
  }
  else if ( type == QLatin1String( "TRIANGLE" ) )
  {
    return QgsWkbTypes::Polygon;
  }
  else if ( type == QLatin1String( "TRIANGLEZ" ) )
  {
    return QgsWkbTypes::PolygonZ;
  }
  else if ( type == QLatin1String( "TRIANGLEM" ) )
  {
    return QgsWkbTypes::PolygonM;
  }
  else if ( type == QLatin1String( "TRIANGLEZM" ) )
  {
    return QgsWkbTypes::PolygonZM;
  }
  return QgsWkbTypes::parseType( type );
}

QgsWkbTypes::Type QgsPostgresConn::wkbTypeFromOgcWkbType( unsigned int wkbType )
{
  // PolyhedralSurface => MultiPolygon
  if ( wkbType % 1000 == 15 )
    return ( QgsWkbTypes::Type )( wkbType / 1000 * 1000 + QgsWkbTypes::MultiPolygon );
  // TIN => MultiPolygon
  if ( wkbType % 1000 == 16 )
    return ( QgsWkbTypes::Type )( wkbType / 1000 * 1000 + QgsWkbTypes::MultiPolygon );
  // Triangle => Polygon
  if ( wkbType % 1000 == 17 )
    return ( QgsWkbTypes::Type )( wkbType / 1000 * 1000 + QgsWkbTypes::Polygon );
  return ( QgsWkbTypes::Type ) wkbType;
}

QString QgsPostgresConn::displayStringForWkbType( QgsWkbTypes::Type type )
{
  return QgsWkbTypes::displayString( QgsWkbTypes::Type( type ) );
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

QgsWkbTypes::Type QgsPostgresConn::wkbTypeFromGeomType( QgsWkbTypes::GeometryType geomType )
{
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      return QgsWkbTypes::Point;
    case QgsWkbTypes::LineGeometry:
      return QgsWkbTypes::LineString;
    case QgsWkbTypes::PolygonGeometry:
      return QgsWkbTypes::Polygon;
    case QgsWkbTypes::NullGeometry:
      return QgsWkbTypes::NoGeometry;
    case QgsWkbTypes::UnknownGeometry:
      return QgsWkbTypes::Unknown;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QgsWkbTypes::Unknown;
}

QStringList QgsPostgresConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "PostgreSQL/connections" ) );
  return settings.childGroups();
}

QString QgsPostgresConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "PostgreSQL/connections/selected" ) ).toString();
}

void QgsPostgresConn::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( QStringLiteral( "PostgreSQL/connections/selected" ), name );
}

QgsDataSourceUri QgsPostgresConn::connUri( const QString &connName )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 2 );

  QgsSettings settings;

  QString key = "/PostgreSQL/connections/" + connName;

  QString service = settings.value( key + "/service" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = QStringLiteral( "5432" );
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

  // Old save setting
  if ( settings.contains( key + "/save" ) )
  {
    username = settings.value( key + "/username" ).toString();

    if ( settings.value( key + "/save" ).toString() == QLatin1String( "true" ) )
    {
      password = settings.value( key + "/password" ).toString();
    }
  }

  QString authcfg = settings.value( key + "/authcfg" ).toString();

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
  settings.remove( key + "/keys" );
  settings.remove( key );
}

bool QgsPostgresConn::cancel()
{
  QMutexLocker locker( &mLock );
  PGcancel *c = ::PQgetCancel( mConn );
  if ( !c )
  {
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( tr( "PQgetCancel failed" ) ),
                               tr( "PostGIS" ) );
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
  QgsPostgresResult res( PQexec( sql ) );

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
