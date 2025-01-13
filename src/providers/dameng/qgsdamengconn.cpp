/***************************************************************************
  qgsdamengconn.cpp  -  connection class to Dameng/Dameng
                             -------------------
    begin                : 2025/01/28
    copyright            : ( C ) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   ( at your option ) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

#include "qgsdamengconn.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsmessagelog.h"
#include "qgscredentials.h"
#include "qgsvectordataprovider.h"
#include "qgswkbtypes.h"
#include "qgssettings.h"
#include "qgsjsonutils.h"
#include "qgspostgresstringutils.h"
#include "qgsvariantutils.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsapplication.h"

#include "qgsdamengtablemodel.h"
#include "qgsdamengdatabase.h"
#include "qgsdamengconnpool.h"

#include <QApplication>
#include <QStringList>
#include <QThread>
#include <QFile>

#include <climits>

#include <nlohmann/json.hpp>

  // for htonl
  // for htonl
#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

const int DM_DEFAULT_TIMEOUT = 30;

static QString quotedString( const QString &v )
{
  QString result = v;
  result.replace( '\'', QLatin1String( "''" ) );
  if ( result.contains( '\\' ) )
    return result.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "E'" ).append( '\'' );
  else
    return result.prepend( '\'' ).append( '\'' );
}

QgsDamengResult::~QgsDamengResult()
{
  mRes = nullptr;
}

QgsDamengResult &QgsDamengResult::operator=( QgsDMResult *result )
{
  mRes = result;
  return *this;
}

QgsDamengResult &QgsDamengResult::operator=( const QgsDamengResult &src )
{
  mRes = src.result();
  return *this;
}

ExecStatusType QgsDamengResult::DMresultStatus()
{
  return mRes ? mRes->getResStatus() : DmResFatalError;
}

QString QgsDamengResult::DMresultErrorMessage()
{
  return mRes ? mRes->getMsg() : QObject::tr( "no result buffer" );
}

int QgsDamengResult::DMntuples()
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->ntuples();
}

QString QgsDamengResult::DMgetvalue( int col )
{
  Q_ASSERT( mRes );
  return mRes->value( col ).toString();
}

bool QgsDamengResult::DMgetisnull( int col )
{
  Q_ASSERT( mRes );
  return mRes->isNull( col );
}

int QgsDamengResult::DMnfields()
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->nfields();
}

udint4 QgsDamengResult::DMftype( int col )
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->ftype( col );
}

QString QgsDamengResult::DMftypeName( int col, udint4 type )
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->getSqlTypeName( col, type );
}

QString QgsDamengResult::DMfname( int col )
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->fname( col );
}

sdint4 QgsDamengResult::DMftable( int col )
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->ftable( col );
}

int QgsDamengResult::DMgetlength( int col, int type )
{
  Q_ASSERT( mRes );
  if ( !mRes )
    return 0;

  return mRes->getLength( col, type );
}


QgsPoolDamengConn::QgsPoolDamengConn( const QString &connInfo )
  : mDmConn( QgsDamengConnPool::instance()->acquireConnection( connInfo ) )
{
}

QgsPoolDamengConn::~QgsPoolDamengConn()
{
  if ( mDmConn )
    QgsDamengConnPool::instance()->releaseConnection( mDmConn );
}


QMap<QString, QgsDamengConn*> QgsDamengConn::sConnectionsRO;
QMap<QString, QgsDamengConn*> QgsDamengConn::sConnectionsRW;

const int QgsDamengConn::GEOM_TYPE_SELECT_LIMIT = 100;

DmConn *QgsDamengConn::DMconnect( const QString &ipaddr, const QString &port, const QString &user, const QString &pwd )
{
  DmConn *conn;
  bool rt;
  QString server;
  conn = new DmConn;
  conn->dmDriver = new QgsDMDriver;
  conn->dmResult = new QgsDMResult( conn->dmDriver );

  if ( port.size() > 5 || ( !port.isEmpty() && port.toInt() == 0 ) )
  {
    conn->connStatus = false;
    conn->dmDriver->setConnMsg( tr( "invaild port %1 of int value for dameng" ).arg( port ) );
    return conn;
  }

  if ( ipaddr.contains( ":" ) )
  {
    conn->connStatus = false;
    conn->dmDriver->setConnMsg( tr( "invaild host %1 for dameng" ).arg( ipaddr ) );
    return conn;
  }

  rt = conn->dmDriver->open( UserTrans, user, pwd, ipaddr, port.toInt(), NULL );
  
  if ( !rt )
    conn->connStatus = false;
  else
    conn->connStatus = true;

  return conn;
}

QgsDamengConn *QgsDamengConn::connectDb( const QString &conninfo, bool readonly, bool shared, bool transaction, bool allowRequestCredentials )
{
  QMap<QString, QgsDamengConn *> &connections = readonly ? QgsDamengConn::sConnectionsRO : QgsDamengConn::sConnectionsRW;

  // This is called from may places where shared parameter cannot be forced to false ( QgsVectorLayerExporter )
  // and which is run in a different thread ( drag and drop in browser )
  if ( QApplication::instance()->thread() != QThread::currentThread() )
  {
    // sharing connection between threads is not safe
    // See https://github.com/qgis/QGIS/issues/21205
    QgsDebugMsgLevel( QStringLiteral( "refusing to use shared connection as we are not the main thread" ), 2 );
    shared = false;
  }

  QgsDamengConn* conn;

  if ( shared )
  {
    QMap<QString, QgsDamengConn*>::iterator it = connections.find( conninfo );
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

  conn = new QgsDamengConn( conninfo, readonly, shared, transaction, allowRequestCredentials );
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

QgsDamengConn::QgsDamengConn( const QString &conninfo, bool readOnly, bool shared, bool transaction, bool allowRequestCredentials )
  : mRef( 1 )
  , mConnInfo( conninfo )
  , mGeosAvailable( false )
  , mProjAvailable( false )
  , mTopologyAvailable( false )
  , mDamengVersion( 0 )
  , mGotDmSpatialVersion( false )
  , mReadOnly( readOnly )
  , mSwapEndian( false )
  , mNextCursorId( 0 )
  , mShared( shared )
  , mTransaction( transaction )
{

  QgsDebugMsgLevel( QStringLiteral( "New Dameng connection for " ) + conninfo, 2 );

  // expand connectionInfo
  QgsDataSourceUri uri( conninfo );
  QString expandedConnectionInfo = uri.connectionInfo( true );
  UserTrans = QgsApplication::settingsLocaleUserLocale->value();

  auto addDefaultTimeoutAndClientEncoding = []( QString &connectString )
  {
    if ( !connectString.contains( QStringLiteral( "connect_timeout=" ) ) )
    {
      // add default timeout
      QgsSettings settings;
      int timeout = settings.value( QStringLiteral( "Dameng/default_timeout" ), DM_DEFAULT_TIMEOUT, QgsSettings::Providers ).toInt();
      connectString += QStringLiteral( " connect_timeout=%1" ).arg( timeout );
    }

    connectString += QLatin1String( " client_encoding='UTF-8'" );
  };
  addDefaultTimeoutAndClientEncoding( expandedConnectionInfo );

  // connect to Dameng Database now
  uri = QgsDataSourceUri( expandedConnectionInfo );

  if ( uri.host().isEmpty() && uri.port().isEmpty() )
    uri.setConnection( "127.0.0.1", "5236", "dameng", uri.username(), uri.password() );
  else if ( uri.port().isEmpty() )
    uri.setConnection( uri.host(), "5236", "dameng", uri.username(), uri.password() );
  else if ( uri.host().isEmpty() )
    uri.setConnection( "127.0.0.1", uri.port(), "dameng", uri.username(), uri.password() );

  std::unique_ptr<QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( QStringLiteral( "DMconnect()" ), expandedConnectionInfo.toUtf8(), QStringLiteral( "dameng" ), QStringLiteral( "QgsDamengConn" ), QGS_QUERY_LOG_ORIGIN );
  mConn = DMconnect( uri.host(), uri.port(), uri.username(), uri.password() );
  // check the connection status
  if ( !DMconnStatus() )
  {
    QString username = uri.username();
    QString password = uri.password();

    QgsCredentials::instance()->lock();

    int i = 0;
    while ( !DMconnStatus() && i < 5 )
    {
      ++i;
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, DMconnErrorMessage() );
      if ( !ok )
      {
        break;
      }
      
      const QString errorMsg = DMconnErrorMessage();
      DMfinish();
      logWrapper->setError( errorMsg );

      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsgLevel( "Connecting to " + uri.connectionInfo( false ), 2 );
      QString connectString = uri.connectionInfo();
      addDefaultTimeoutAndClientEncoding( connectString );

      logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( QStringLiteral( "DMconnect()" ), expandedConnectionInfo.toUtf8(), QStringLiteral( "dameng" ), QStringLiteral( "QgsDamengConn" ), QGS_QUERY_LOG_ORIGIN );
      mConn = DMconnect( uri.host(), uri.port(), uri.username(), uri.password() );
    }

    if ( DMconnStatus() )
      QgsCredentials::instance()->put( conninfo, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( !DMconnStatus() )
  {
    QString errorMsg = DMconnErrorMessage();
    DMfinish();
    logWrapper->setError( errorMsg );
    QgsMessageLog::logMessage( tr( "Connection to database failed" ) + '\n' + errorMsg, tr( "Dameng" ) );
    mRef = 0;
    return;
  }

  logWrapper = nullptr;
  QgsDebugMsgLevel( QStringLiteral( "Connection to the database was successful" ), 2 );

  /* Check to see if we have working Dameng support */
  if ( !dmSpatialVersion().isNull() )
  {
    /* Check to see if we have GEOS support and if not, warn the user about
       the problems they will see :) */
    QgsDebugMsgLevel( QStringLiteral( "Checking for GEOS support" ), 3 );

    if ( !hasGEOS() )
    {
      QgsMessageLog::logMessage( tr( "Your Dameng installation has no GEOS support. Feature selection and identification will not work properly. Please exec sql ( select SF_CHECK_RASTER_SYS ) to check it" ), tr( "Dameng" ) );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "GEOS support available!" ), 3 );
    }
  }

}

QgsDamengConn::~QgsDamengConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mConn )
    DMfinish();

  delete mConn;
  mConn = nullptr;
}

int QgsDamengConn::DMserverVersion() const
{
  if ( !mConn || !DMconnStatus() )
    return 0;

  QString version = "select substr( substr( SVR_VERSION,instr( SVR_VERSION,'V' ) ) || "
    " substr( id_code,15,8 ),2 ) from v$instance";
  QgsDMResult *res = DMexec( version );
  if ( res && res->execstatus() && res->fetchNext() )
    return res->value( 0 ).toString().remove( '\n' ).toInt();

  return 0;
}

void QgsDamengConn::ref()
{
  QMutexLocker locker( &mLock );
  ++mRef;
}

void QgsDamengConn::unref()
{
  QMutexLocker locker( &mLock );
  if ( --mRef > 0 )
    return;

  if ( mShared )
  {
    QMap< QString, QgsDamengConn*> &connections = mReadOnly ? sConnectionsRO : sConnectionsRW;

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
QStringList QgsDamengConn::supportedSpatialTypes() const
{
  QStringList supportedSpatialTypes;

  supportedSpatialTypes << quotedValue( "geometry" )
                        << quotedValue( "geography" );

  if ( hasTopology() )
    supportedSpatialTypes << quotedValue( "topogeometry" );

  return supportedSpatialTypes;
}

bool QgsDamengConn::getTableInfo( bool searchSysdbaOnly, bool allowGeometrylessTables, const QString &schema, const QString &name )
{
  QMutexLocker locker( &mLock );
  int nColumns = 0;
  int foundInTables = 0;
  QgsDamengResult result;
  QString query;

  mLayersSupported.clear();

  for ( int i = SctGeometry; i < SctRaster; ++i )
  {
    QString sql, tableName, schemaName, columnName, typeName, sridName, gtableName, dimName;

    if ( i == SctGeometry )
    {
      tableName = QStringLiteral( "G.F_TABLE_NAME" );
      schemaName = QStringLiteral( "G.F_TABLE_SCHEMA" );
      columnName = QStringLiteral( "G.F_GEOMETRY_COLUMN" );
      typeName = QStringLiteral( "upper( G.TYPE )" );
      sridName = QStringLiteral( "G.SRID" );
      dimName = QStringLiteral( "G.COORD_DIMENSION" );
      gtableName = QStringLiteral( "SYSGEO2.GEOMETRY_COLUMNS" );
    }
    // Geography 
    else if ( i == SctGeography )
    {
      tableName = QStringLiteral( "G.F_TABLE_NAME" );
      schemaName = QStringLiteral( "G.F_TABLE_SCHEMA" );
      columnName = QStringLiteral( "G.F_GEOGRAPHY_COLUMN" );
      typeName = QStringLiteral( "upper( G.TYPE )" );
      sridName = QStringLiteral( "G.SRID" );
      dimName = QStringLiteral( "2" );
      gtableName = QStringLiteral( "SYSGEO2.GEOGRAPHY_COLUMNS" );
    }
    else if ( i == SctTopoGeometry )
    {
      if ( !hasTopology() )
        continue;

      schemaName = QStringLiteral( "G.SCHEMA_NAME" );
      tableName = QStringLiteral( "G.TABLE_NAME" );
      columnName = QStringLiteral( "G.FEATURE_COLUMN" );
      typeName = "CASE "
        "WHEN G.FEATURE_TYPE = 1 THEN \'MULTIPOINT\' "
        "WHEN G.FEATURE_TYPE = 2 THEN \'MULTILINESTRING\' "
        "WHEN G.FEATURE_TYPE = 3 THEN \'MULTIPOLYGON\' "
        "WHEN G.FEATURE_TYPE = 4 THEN \'GEOMETRYCOLLECTION\' "
        "END AS type";
      sridName = QStringLiteral( "( SELECT srid FROM SYSTOPOLOGY.SYSTOPOLOGY t WHERE G.TOPOLOGY_ID=t.id )" );
      dimName = QStringLiteral( "2" );
      gtableName = QStringLiteral( "SYSTOPOLOGY.SYSLAYER" );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Unsupported spatial column type %1" )
        .arg( displayStringForGeomType( ( QgsDamengGeometryColumnType )i ) ) );
      continue;
    }

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.
    sql = QString( "SELECT distinct(%1),%2,%3,%4,%5,%6,c.SUBTYPE$,c.INFO5,"
                    "( select COMMENT$ from SYS.SYSTABLECOMMENTS where SCHNAME = c.SCH_NAME and "
                    "   TVNAME = ( select NAME from SYS.SYSOBJECTS where ID = c.ID ) ), "
                    "( select JSONB_AGG( NAME ) from SYS.SYSCOLUMNS where ID = c.ID ), "
                    "G.SPCOLS, "
                    "%8 "
                    " FROM ( select * from %7 G join "
                    "        ( select %2 SCHEMA_2, %1 TABLE_2, count(*) as SPCOLS "
                    "       from %7 G group by (%2,%1) ) as GD "
                    "       on %1 = GD.TABLE_2 ) G "
                    " left outer join"
                    "   ( select n.name SCH_NAME, c1.name TAB_NAME, c1.ID, c1.SUBTYPE$, c1.INFO5 "
                    "     from SYS.SYSOBJECTS c1, SYS.SYSOBJECTS n "
                    "     where n.ID = c1.schid	) c"
                    " on c.TAB_NAME = %1 and c.SCH_NAME = %2"
                    " WHERE %2 in ( select unique(OWNER) from SYS.ALL_TABLES ) "
                  ).arg( tableName, schemaName, columnName, typeName, sridName, dimName, gtableName )
                    .arg( i );

    if ( searchSysdbaOnly )
      sql += QStringLiteral( " AND %1=%2" ).arg( schemaName, quotedValue( "SYSDBA" ) );
    
    if ( !schema.isEmpty() )
      sql += QStringLiteral( " AND %1=%2" ).arg( schemaName, quotedValue( schema ) );
    
    if ( !name.isEmpty() )
      sql += QStringLiteral( " AND %1=%2" ).arg( tableName, quotedString( name ) );

    foundInTables |= 1 << i;

    if ( !query.isEmpty() )
      query += " UNION ";

    query += sql;
  }

  query += QLatin1String( " ORDER BY 2,1,3" );


  QgsDebugMsgLevel( "getting table info from layer registries: " + query, 2 );
  QgsDMResult *res = DMexec( query );

  if( !res || !res->execstatus() )
  {
    return false;
  }

  while ( res->fetchNext() )
  {
    QString tableName = res->value( 0 ).toString();
    QString schemaName = res->value( 1 ).toString();
    if ( schemaName == "SYSTOPOLOGY" && tableName == "TOPO$TMPFACE_CHECK" )
      continue;
    QString column = res->value( 2 ).toString();
    QString type = res->value( 3 ).toString();
    QString ssrid = res->value( 4 ).toString();
    int dim = res->value( 5 ).toInt();
    QString relkind = res->value( 6 ).toString();
    QString isView_info = res->value( 7 ).toString();
    bool isView = relkind == QLatin1String( "VIEW" ) && isView_info == QLatin1String( "0x" );
    bool isMaterializedView = relkind == QLatin1String( "VIEW" ) && isView_info != QLatin1String( "0x" );
    QString comment = res->value( 8 ).toString();
    QString attributes = res->value( 9 ).toString();
    int nSpCols = res->value( 10 ).toInt();
    QgsDamengGeometryColumnType columnType = SctNone;

    int columnTypeInt = res->value( 11 ).toInt();
    if ( columnTypeInt == SctGeometry )
      columnType = SctGeometry;
    else if ( columnTypeInt == SctGeography )
      columnType = SctGeography;
    else if ( columnTypeInt == SctTopoGeometry )
      columnType = SctTopoGeometry;
    else
    {
      QgsDebugError( QStringLiteral( "Unhandled columnType index %1" )
        .arg( columnTypeInt ) );
    }

    int srid = ssrid.isEmpty() ? std::numeric_limits<int>::min() : ssrid.toInt();

    if ( srid == 0 )
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

    QgsDamengLayerProperty layerProperty;
    layerProperty.schemaName = schemaName;
    layerProperty.tableName = tableName;
    layerProperty.geometryColName = column;
    layerProperty.geometryColType = columnType;
    if ( dim == 3 && !type.endsWith( 'M' ) )
      type += QLatin1Char( 'Z' );
    else if ( dim == 4 )
      type += QLatin1String( "ZM" );
    layerProperty.types = QList<Qgis::WkbType>() << ( QgsDamengConn::wkbTypeFromDmSpatial( type ) );
    layerProperty.srids = QList<int>() << srid;
    layerProperty.sql.clear();
    layerProperty.relKind = relkind;
    layerProperty.isView = isView;
    layerProperty.isMaterializedView = isMaterializedView;
    layerProperty.tableComment = comment;
    layerProperty.nSpCols = nSpCols;
    if ( isView )
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

  if ( allowGeometrylessTables )
  {
    QString sql = QStringLiteral( "SELECT distinct( c.NAME ) ,n.NAME ,c.SUBTYPE$"
      ",( select COMMENT$ from SYS.SYSTABLECOMMENTS where "
      "     SCHNAME = n.name AND TVNAME = ( select NAME from SYSOBJECTS where ID = c.ID ) )"
      ",%1,c.INFO5"
      " FROM ( select NAME,SCHID,ID,SUBTYPE$,INFO5 from SYSOBJECTS where SUBTYPE$ IN (\'VIEW\', \'UTAB\') ) c,"
      " ( select ID,NAME,TYPE$ from SYSOBJECTS ) n,"
      " ( select ID,NAME from SYSCOLUMNS where COLID >= 0 ) a"
      " WHERE n.ID = c.SCHID"
      " AND c.ID = a.ID" )
      .arg( "( select JSONB_AGG( a.NAME ) from SYS.SYSCOLUMNS a where a.ID = c.ID )" );

    sql += searchSysdbaOnly ? QStringLiteral( " AND n.NAME = \'SYSDBA\' AND n.TYPE$ = \'SCH\'" )
                            : QStringLiteral( " AND n.NAME = \'%1\' AND n.TYPE$ = \'SCH\'" )
                                .arg( schema.isEmpty() ? QStringLiteral( "SYSDBA" ) :schema );

    if ( !name.isEmpty() )
      sql += QStringLiteral( " AND c.name=%1" ).arg( quotedString( name ) );
    else
      sql += QStringLiteral( " AND ( c.name not like \'TOPO$%\' "
                              "and c.name not like \'MTAB$%\' "
                              "and c.name not like \'MDRT$%\' )" );

    QgsDebugMsgLevel( "getting non-spatial table info: " + sql, 2 );

    res = DMexec( sql );
    if ( !res || !res->execstatus() )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined.\nThe error message from the database was:\n%1" )
        .arg( res->getMsg() ),
        tr( "Dameng" ) );
      return false;
    }

    while ( res->fetchNext() )
    {
      QString table = res->value( 0 ).toString(); // relname
      QString schema = res->value( 1 ).toString(); // nspname
      if ( schema == "SYSRASTER" && table == "RASTER_OVERVIEWS" )
        continue;
      QString relkind = res->value( 2 ).toString(); // relation kind
      QString comment = res->value( 3 ).toString(); // table comment
      QString attributes = res->value( 4 ).toString(); // attributes array
      QString isView_info = res->value( 5 ).toString();
      bool isView = relkind == QLatin1String( "VIEW" ) && isView_info == QLatin1String( "0x" );
      bool isMaterializedView = relkind == QLatin1String( "VIEW" ) && isView_info != QLatin1String( "0x" );
      
      QgsDamengLayerProperty layerProperty;
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

      //check if we've already added this layer in some form
      bool alreadyFound = false;
      const auto constMLayersSupported = mLayersSupported;
      for ( const QgsDamengLayerProperty &foundLayer : constMLayersSupported )
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

      if ( isView )
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
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "Dameng" ) );
  }

  return true;
}

bool QgsDamengConn::supportedLayers( QVector<QgsDamengLayerProperty> &layers, bool searchSysdbaOnly, bool allowGeometrylessTables, const QString &schema )
{
  return supportedLayersPrivate( layers, searchSysdbaOnly, allowGeometrylessTables, schema );
}

bool QgsDamengConn::supportedLayer( QgsDamengLayerProperty &layerProperty, const QString &schema, const QString &table )
{
  QVector<QgsDamengLayerProperty> layers;
  if ( !supportedLayersPrivate( layers, false, true /* allowGeometrylessTables */, schema, table ) || layers.empty() )
  {
    return false;
  }
  else
  {
    layerProperty = layers.first();
  }
  return true;
}

bool QgsDamengConn::supportedLayersPrivate( QVector<QgsDamengLayerProperty> &layers, bool searchSysdbaOnly, bool allowGeometrylessTables, const QString &schema, const QString &table )
{
  QMutexLocker locker( &mLock );

  // Get the list of supported tables
  if ( !getTableInfo( searchSysdbaOnly, allowGeometrylessTables, schema, table ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "Dameng" ) );
    return false;
  }

  layers = mLayersSupported;

  return true;
}

bool QgsDamengConn::getSchemas( QList<QgsDamengSchemaProperty> &schemas )
{
  schemas.clear();

  QString sql = QStringLiteral( "select SCH_OBJ.NAME sch_name, USER_OBJ.NAME user_name "
    "from ( select NAME, ID, PID, CRTDATE from SYS.SYSOBJECTS where TYPE$ = \'SCH\') SCH_OBJ, "
    "( select NAME, ID from SYS.SYSOBJECTS where TYPE$ = \'UR\' and SUBTYPE$ = \'USER\') USER_OBJ "
    "where SCH_OBJ.PID = USER_OBJ.ID "
    " and USER_OBJ.NAME != \'SYS\' and USER_OBJ.NAME != \'SYSAUDITOR\' and USER_OBJ.NAME != \'SYSSSO\';" );

  QgsDMResult *res = DMexec( sql );
  if( !res || !res->execstatus() )
  {
    return false;
  }

  while ( res->fetchNext() )
  {
    QgsDamengSchemaProperty schema;
    schema.name = res->value( 0 ).toString();
    schema.owner = res->value( 1 ).toString();
    schema.description = nullptr;
    schemas << schema;
  }

  return true;
}

/**
 * Check to see if GEOS is available
 */
bool QgsDamengConn::hasGEOS() const
{
  dmSpatialVersion();
  return mGeosAvailable;
}

/**
 * Check to see if topology is available
 */
bool QgsDamengConn::hasTopology() const
{
  dmSpatialVersion();
  return mTopologyAvailable;
}

/* Functions for determining available features in dmSpatial */
QString QgsDamengConn::dmSpatialVersion() const
{
  QMutexLocker locker( &mLock );
  if ( mGotDmSpatialVersion )
    return mDmSpatialVersionInfo;

  mDamengVersion = DMserverVersion();

  QgsDMResult *res = DMexec( QStringLiteral( "select spatial_full_version();" ) );
  if ( !res->fetchNext() )
  {
    QgsMessageLog::logMessage( tr( "No Dameng Spatial support in the database." ), tr( "Dameng" ) );
    mGotDmSpatialVersion = true;
    return QString();
  }

  mDmSpatialVersionInfo = res->value( 0 ).toString();

  QgsDebugMsgLevel( "Dameng Spatial version info: " + mDmSpatialVersionInfo, 2 );

  // checking for geos and proj support
  QString Geos_version;
  int idx1 = mDmSpatialVersionInfo.indexOf( "GEOS = " );
  mGeosAvailable = idx1 == -1 ? false : true;

  if ( mGeosAvailable )
  {
    idx1 += 8;
    int idx2 = mDmSpatialVersionInfo.indexOf( ',', idx1 );
    Geos_version = mDmSpatialVersionInfo.mid( idx1, idx2 - idx1 - 1 );
  }

  QString Proj_version;
  idx1 = mDmSpatialVersionInfo.indexOf( "PROJ = " );
  mGeosAvailable = idx1 == -1 ? false : true;

  if ( mGeosAvailable )
  {
    idx1 += 8;
    int idx2 = mDmSpatialVersionInfo.indexOf( ',', idx1 );
    Proj_version = mDmSpatialVersionInfo.mid( idx1, idx2 - idx1 - 1 );
  }

  QgsDebugMsgLevel( QStringLiteral( "geos:%1 proj:%2" ).arg( mGeosAvailable ? Geos_version : "none" ).arg( mProjAvailable ? Proj_version : "none" ), 2 );

  // checking for topology support
  QgsDebugMsgLevel( QStringLiteral( "Checking for topology support" ), 2 );
  mTopologyAvailable = false;

  QString sql = QStringLiteral( "SELECT SF_CHECK_USER_TABLE_PRIV(\'SYSTOPOLOGY\', \'SYSTOPOLOGY\', user, 0 ) "
            " & SF_CHECK_USER_TABLE_PRIV(\'SYSTOPOLOGY\', \'SYSLAYER\', user, 0 );" );
  
  res = DMexec( sql, false );
  if ( res->fetchNext() && res->value( 0 ).toInt() == 1 )
  {
    mTopologyAvailable = true;
  }

  if ( mTopologyAvailable )
  {
    QgsDebugMsgLevel( QStringLiteral( "Topology support available :)" ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Topology support not available :(" ), 2 );
  }

  mGotDmSpatialVersion = true;

  return mDmSpatialVersionInfo;
}

QString QgsDamengConn::quotedIdentifier( const QString &ident )
{
  QString result = ident;
  result.replace( '"', QLatin1String( "\"\"" ) );
  return result.prepend( '\"' ).append( '\"' );
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
    ret.append( doubleQuotedMapValue( i.key() ) + ":" + doubleQuotedMapValue( i.value().toString() ) );
  }
  return "'{" + ret + "}'";
}

QString QgsDamengConn::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

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

  case QMetaType::Type::Double:
  case QMetaType::Type::QString:
  default:
    return quotedString( value.toString() );
  }
}

QgsDMResult *QgsDamengConn::DMexec( const QString &query, bool logError, bool acquireRows, const QString &originatorClass, const QString &queryOrigin ) const
{
  QMutexLocker locker( &mLock );

  QgsDebugMsgLevel( QStringLiteral( "Executing SQL: %1" ).arg( query ), 3 );

  std::unique_ptr<QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( query, mConnInfo, QStringLiteral( "dameng" ), originatorClass, queryOrigin );

  if ( mConn && DMconnStatus() )
  {
    if ( !mConn->dmResult )
      mConn->dmResult = new QgsDMResult( mConn->dmDriver );

    mConn->dmResult->setForwardOnly( false );
    if ( !mConn->dmResult->getMsg().isEmpty() )
      mConn->dmResult->getMsg().clear();

    if ( mConn->dmResult->prepare( query ) )
      mConn->dmResult->exec();
    if ( acquireRows )
      mConn->dmResult->setNtuples();

    QgsDMResult *res = mConn->dmResult;
    int resStatus = res->getResStatus();
    if ( resStatus != DmResCommandOk && resStatus != DmResSuccessInfo )
    {
      const QString error { tr( "Erroneous query: %1 returned %2 [%3]" )
                              .arg( query ).arg( resStatus ).arg( res->getMsg() ) };
      logWrapper->setError( error );
      if ( logError )
      {
        QgsMessageLog::logMessage( error, tr( "Dameng" ) );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Not logged erroneous query: %1 returned %2 [%3]" )
          .arg( query ).arg( resStatus ).arg( res->getMsg() ) );
      }
    }
    logWrapper->setFetchedRows( res->ntuples() );
    return res;
  }

  if ( !DMconnStatus() )
  {
    const QString error { tr( "Connection error: %1 returned %2 [%3]" )
        .arg( query ).arg( DMconnStatus() ).arg( DMconnErrorMessage() ) };
    logWrapper->setError( error );
    if ( logError )
    {
      QgsMessageLog::logMessage( error, tr( "Dameng" ) );
    }
    else
    {
      QgsDebugError( QStringLiteral( "Connection error: %1 returned %2 [%3]" )
        .arg( query ).arg( DMconnStatus() ).arg( DMconnErrorMessage() ) );
    }
  }
  else
  {
    const QString error { tr( "Query failed: %1\nError: no result buffer" ).arg( query ) };
    logWrapper->setError( error );
    if ( logError )
    {
      QgsMessageLog::logMessage( error, tr( "Dameng" ) );
    }
    else
    {
      QgsDebugError( QStringLiteral( "Not logged query failed: %1\nError: no result buffer" ).arg( query ) );
    }
  }

  return nullptr;
}

bool QgsDamengConn::DMexecNR( const QString &query, const QString &originatorClass, const QString &queryOrigin  )
{
  QMutexLocker locker( &mLock );

  QgsDamengResult res( DMexec( query, false, false, originatorClass, queryOrigin ) );

  ExecStatusType errorStatus = res.DMresultStatus();
  if ( errorStatus == DmResCommandOk )
    return true;

  QgsMessageLog::logMessage( tr( "Query: %1 returned %2 [%3]" ).arg( query ).arg( errorStatus ).arg( res.DMresultErrorMessage() ), tr( "Dameng" ) );

  if ( DMconnStatus() )
  {
    DMexecNR( QStringLiteral( "ROLLBACK" ) );
  }

  return false;
}

ExecStatusType QgsDamengConn::DMprepare( const QString &query, int nParams, const udint4 *paramTypes, const QString &originatorClass, const QString &queryOrigin )
{
  Q_UNUSED( nParams )
  Q_UNUSED( paramTypes )
  QMutexLocker locker( &mLock );

  std::unique_ptr<QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( QStringLiteral( "DMprepare(): %1 " ).arg( query ), mConnInfo, QStringLiteral( "dameng" ), originatorClass, queryOrigin );

  if ( !mConn || !mConn->connStatus )
    return DmResFatalError;

  if ( !mConn->dmResult )
    mConn->dmResult = new QgsDMResult( mConn->dmDriver );

  ExecStatusType status = mConn->dmResult->prepare( query ) ? DmResCommandOk : DmResFatalError;

  if ( status == DmResFatalError )
  {
    logWrapper->setError( mConn->dmResult->getMsg() );
  }

  return status;
}

QgsDMResult *QgsDamengConn::DMexecPrepared( const QByteArray &wkb, const QStringList &params, const QString &originatorClass, const QString &queryOrigin )
{
  QMutexLocker locker( &mLock );
  sdint2		sql_type;
  ulength		prec;
  sdint2		scale;
  slength   size_null = DSQL_NULL_DATA;
  sdint2    nullable = 1;

  slength   size = wkb.size();
  if ( size == 0 )
  {
      dpi_desc_param( *mConn->dmResult->getStmt(), 1, &sql_type, &prec, &scale, &nullable );
      dpi_bind_param( *mConn->dmResult->getStmt(), 1, DSQL_PARAM_INPUT, DSQL_C_BINARY, DSQL_CLASS,
          prec, scale, ( dpointer )NULL, 0, &size_null );
  }
  else
      dpi_bind_param( *mConn->dmResult->getStmt(), 1, DSQL_PARAM_INPUT, DSQL_C_BINARY, DSQL_CLASS,
        ( ulength )wkb.size(), 0, ( dpointer )wkb.data(), wkb.size(), &size );

  char** str = new char*[ params.size() + 1 ];
  for ( int i = 0; i < params.size(); i++)
  {
    sdint2 nullable = 1;
    str[i] = new char[ params[i].size() * 8 + 33 ];
    strcpy( str[i], params[i].toUtf8().data() );
    dpi_desc_param( *mConn->dmResult->getStmt(), i+2, &sql_type, &prec, &scale, &nullable );
    if ( !params[i].isNull() )
      dpi_bind_param( *mConn->dmResult->getStmt(), i+2, DSQL_PARAM_INPUT, DSQL_C_NCHAR, sql_type,
                    prec, scale, ( dpointer )str[i], strlen( str[i]), NULL );
    else
      dpi_bind_param( *mConn->dmResult->getStmt(), i + 2, DSQL_PARAM_INPUT, DSQL_C_NCHAR, sql_type,
        prec, scale, ( dpointer )NULL, 0, &size_null );
  }
  std::unique_ptr<QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( QStringLiteral( "DMexecPrepared()" ), mConnInfo, QStringLiteral( "dameng" ), originatorClass, queryOrigin );

  QgsDMResult *res = mConn->dmResult;
  res->exec();
  
  const int errorStatus = res->getResStatus();

  if ( errorStatus != DmResCommandOk && errorStatus != DmResSuccessInfo )
  {
    logWrapper->setError( res->getMsg() );
  }

  for ( int i = 0; i < params.size(); i++)
      delete[] str[i];
  delete[] str;

  return res;
}


void QgsDamengConn::DMfinish()
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );

  if ( mConn->dmResult )
  {
    delete mConn->dmResult;
    mConn->dmResult = nullptr;
  }

  if ( DMconnStatus() || mConn->dmDriver->isOpen() )
  {
    mConn->dmDriver->close();
    mConn->connStatus = false;
  }
}

int QgsDamengConn::DMconnStatus() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );
  if ( !mConn )
    return false;
  return mConn->connStatus;
}

QString QgsDamengConn::DMconnErrorMessage() const
{
  QMutexLocker locker( &mLock );

  Q_ASSERT( mConn );

  if ( mConn->connStatus )
    mConn->dmDriver->getConnMsg().clear();

  return mConn->dmDriver->getConnMsg();
}

bool QgsDamengConn::begin()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return DMexecNR( QStringLiteral( "SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return mConn->dmDriver->beginTransaction();
  }
}

bool QgsDamengConn::commit()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return DMexecNR( QStringLiteral( "RELEASE SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return mConn->dmDriver->commitTransaction();
  }
}

bool QgsDamengConn::rollback()
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return DMexecNR( QStringLiteral( "ROLLBACK TO SAVEPOINT transaction_savepoint" ) )
           && DMexecNR( QStringLiteral( "RELEASE SAVEPOINT transaction_savepoint" ) );
  }
  else
  {
    return mConn->dmDriver->rollbackTransaction();
  }
}

QgsDMResult *QgsDamengConn::DMgetResult()
{
  return mConn->dmResult;
}

QString QgsDamengConn::fieldExpressionForWhereClause( const QgsField &fld, QMetaType::Type valueType, QString expr )
{
  Q_UNUSED( valueType )
  QString out;
  const QString &type = fld.typeName();

  if ( type == QLatin1String( "timestamp" ) || type == QLatin1String( "time" ) || type == QLatin1String( "date" )
    || type == QLatin1String( "bigint" ) || type == QLatin1String( "smallint" ) || type == QLatin1String( "int" ) 
    || type == QLatin1String( "real" ) || type == QLatin1String( "double precision" ) || type == QLatin1String( "float" ) || type == QLatin1String( "double" ) //
    || type == QLatin1String( "numeric" ) || type == QLatin1String( "dec" ) || type == QLatin1String( "decimal" ) )
  {
    out = expr.arg( quotedIdentifier( fld.name() ) );
    // if field and value havev incompatible types, rollback to text cast
    if ( valueType != QMetaType::Type::UnknownType && valueType != QMetaType::Type::Int && valueType != QMetaType::Type::LongLong && valueType != QMetaType::Type::Double )
    {
      out = "cast( " + out + " as text )";
    }
  }
  else
  {
    out = fieldExpression( fld, expr ); // same as fieldExpression by default
  }

  return out;
}

QString QgsDamengConn::fieldExpression( const QgsField &fld, QString expr )
{
  const QString &type = fld.typeName();
  expr = expr.arg( quotedIdentifier( fld.name() ) );

  if ( type == QLatin1String( "bit" ) )
  {
    return QStringLiteral( "%1" ).arg( expr );
  }
  else if ( type == QLatin1String( "geometry" ) )
  {
    return QStringLiteral( "%1(%2)" )
      .arg( "DMGEO2.st_asewkt",
        expr );
  }
  else if ( type == QLatin1String( "geography" ) )
  {
    return QStringLiteral( "DMGEO2.st_astext(%1)" ).arg( expr );
  }
  else if ( type == QLatin1String( "bigint" ) )
  {
    return QStringLiteral( "%1" ).arg( expr );
  }
  else
  {
    return expr;
  }
}

QList<QgsVectorDataProvider::NativeType> QgsDamengConn::nativeTypes()
{
  QList<QgsVectorDataProvider::NativeType> types;

  types     // integer types
    <<QgsVectorDataProvider::NativeType( tr( "8 Bytes integer( bigint )" ), QStringLiteral( "bigint" ), QMetaType::Type::LongLong, -1, -1, 0, 0 )
    <<QgsVectorDataProvider::NativeType( tr( "4 Bytes integer( int )" ), QStringLiteral( "int" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    <<QgsVectorDataProvider::NativeType( tr( "2 Bytes integer( smallint )" ), QStringLiteral( "smallint" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    <<QgsVectorDataProvider::NativeType( tr( "1 Bytes integer( tinyint )" ), QStringLiteral( "tinyint" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    <<QgsVectorDataProvider::NativeType( tr( "1 Bytes integer( byte )" ), QStringLiteral( "byte" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( number )" ), QStringLiteral( "number " ), QMetaType::Type::Double, 1, 38, 0, 38 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( numeric )" ), QStringLiteral( "numeric" ), QMetaType::Type::Double, 1, 38, 0, 38 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( decimal )" ), QStringLiteral( "decimal" ), QMetaType::Type::Double, 1, 38, 0, 38 )

    // floating point
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( real )" ), QStringLiteral( "real" ), QMetaType::Type::Double, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( float )" ), QStringLiteral( "float" ), QMetaType::Type::Double, 1, 126, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( double )" ), QStringLiteral( "double" ), QMetaType::Type::Double, 1, 126, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Decimal number ( double precision )" ), QStringLiteral( "double precision" ), QMetaType::Type::Double, 1, 126, -1, -1 )

    // string types
    <<QgsVectorDataProvider::NativeType( tr( "Text, fixed length ( char )" ), QStringLiteral( "char" ), QMetaType::Type::QString, 1, 255, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Text, fixed length ( character )" ), QStringLiteral( "character" ), QMetaType::Type::QString, 1, 255, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Text, limited variable length ( varchar )" ), QStringLiteral( "varchar" ), QMetaType::Type::QString, 1, 8188, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Text, unlimited length ( text )" ), QStringLiteral( "text" ), QMetaType::Type::QString, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Text, unlimited length ( clob )" ), QStringLiteral( "clob" ), QMetaType::Type::QString, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Text, unlimited length ( longvarchar )" ), QStringLiteral( "longvarchar" ), QMetaType::Type::QString, -1, -1, -1, -1 )

    // date type
    <<QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), QStringLiteral( "date" ), QMetaType::Type::QDate, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), QStringLiteral( "time" ), QMetaType::Type::QTime, 0, 6, -1, -1 )
    <<QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), QStringLiteral( "timestamp" ), QMetaType::Type::QDateTime, 0, 9, -1, -1 )
    <<QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), QStringLiteral( "datetime" ), QMetaType::Type::QDateTime, 0, 9, -1, -1 )

    // boolean
    <<QgsVectorDataProvider::NativeType( tr( "Boolean ( bit )" ), QStringLiteral( "bit" ), QMetaType::Type::Bool, -1, -1, -1, -1 )

    // binary ( binary )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( binary )" ), QStringLiteral( "binary" ), QMetaType::Type::QByteArray, 1, 8188, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( varbinary )" ), QStringLiteral( "varbinary" ), QMetaType::Type::QByteArray, 1, 8188, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( raw )" ), QStringLiteral( "raw" ), QMetaType::Type::QByteArray, 1, 8188, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( blob )" ), QStringLiteral( "blob" ), QMetaType::Type::QByteArray, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( image )" ), QStringLiteral( "image" ), QMetaType::Type::QByteArray, -1, -1, -1, -1 )
    <<QgsVectorDataProvider::NativeType( tr( "Binary object ( longvarbinary )" ), QStringLiteral( "longvarbinary" ), QMetaType::Type::QByteArray, -1, -1, -1, -1 )
    ;

    types <<QgsVectorDataProvider::NativeType( tr( "JSON ( json )" ), QStringLiteral( "json" ), QMetaType::Type::QVariantMap, -1, -1, -1, -1, QMetaType::Type::QString );

  return types;
}

void QgsDamengConn::retrieveLayerTypes( QgsDamengLayerProperty &layerProperty, bool useEstimatedMetadata )
{
  QVector<QgsDamengLayerProperty *> vect;
  vect << &layerProperty;
  retrieveLayerTypes( vect, useEstimatedMetadata );
}

void QgsDamengConn::retrieveLayerTypes( QVector<QgsDamengLayerProperty *> &layerProperties, bool useEstimatedMetadata )
{
  QString table;
  QString query;
  QgsDamengGeometryColumnType geomType;
  // Limit table row scan if useEstimatedMetadata
  const QString tableScanLimit { useEstimatedMetadata ? QStringLiteral( " LIMIT %1" ).arg( GEOM_TYPE_SELECT_LIMIT ) : QString() };
  setEstimatedMetadata( useEstimatedMetadata );

  int i = 0;
  for ( auto *layerPropertyPtr : layerProperties )
  {
    geomType = layerPropertyPtr->geometryColType;

    QgsDamengLayerProperty &layerProperty = *layerPropertyPtr;

    if ( i++)
      query += QLatin1String( " UNION " );

    if ( !layerProperty.schemaName.isEmpty() )
      table = QStringLiteral( "%1.%2" ).arg( quotedIdentifier( layerProperty.schemaName ),
                                          quotedIdentifier( layerProperty.tableName ) );
    else
      table = layerProperty.tableName;

    if ( layerProperty.geometryColName.isEmpty() )
      continue;

    // vectors & topology
    // our estimation ignores that a where clause might restrict the feature type or srid
    if ( useEstimatedMetadata )
    {
      table = QStringLiteral( "( SELECT %1 FROM %2 WHERE %3%1 IS NOT NULL %4 ) AS t" )
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

    QString geom_col;
    switch ( geomType )
    {
    case SctGeometry:
      geom_col = quotedIdentifier( layerProperty.geometryColName );
      break;
    case SctGeography:
      geom_col = quotedIdentifier( layerProperty.geometryColName ) + "::SYSGEO2.st_geometry";
      break;
    case SctTopoGeometry:
      geom_col = QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" )
                  .arg( quotedIdentifier( layerProperty.geometryColName ) );
      break;
    default:
      break;
    }

    sql += QLatin1String( "JSONB_AGG( DISTINCT " );

    int srid = layerProperty.srids.value( 0, std::numeric_limits<int>::min() );
    if ( srid == std::numeric_limits<int>::min() )
      sql += QStringLiteral( "( %1(%2) )" ).arg( "DMGEO2.st_srid", geom_col );
    else
      sql += QStringLiteral( "\'%1\'" ).arg( QString::number( srid ) );

    sql += QLatin1String( " || \':\' || " );

    Qgis::WkbType type = layerProperty.types.value( 0, Qgis::WkbType::Unknown );
    if ( type == Qgis::WkbType::Unknown )
    {
      sql += QStringLiteral( "( UPPER( DMGEO2.ST_GeometryType(%1) ) )  || \':\' "
        "|| ( DMGEO2.ST_Zmflag(%1) )" ).arg( geom_col );
      table = QStringLiteral( "(select * from %1 order by DMGEO2.ST_Zmflag(%2))" ).arg( table ).arg( geom_col );
    }
    else
    {
      QString WkbTypeName = QgsDamengConn::dmSpatialWkbTypeName( type );
      sql += WkbTypeName == "NULL" ? QStringLiteral( "\':-1\'" )
              : QStringLiteral( " \'ST_\' || %1  || \':-1\' " ).arg( quotedValue( WkbTypeName ) );
    }
    sql += QLatin1String( ") " );

    if ( type == Qgis::WkbType::Unknown )
      sql += QStringLiteral( " FROM %1 AS _unused %2" ).arg( table ).arg( tableScanLimit );
    else
      sql += " FROM " + table;

    QgsDebugMsgLevel( "Geometry types,srids and dims query: " + sql, 2 );

    query += QStringLiteral( "(%1)" ).arg( sql );
    
  }

  QgsDebugMsgLevel( "Layer types,srids and dims query: " + query, 3 );

  QgsDMResult *res = DMexec( query );
  if ( !res || !res->execstatus() )
  {
    return;
  }

  while ( res->fetchNext() )
  {
    int idx = res->value( 0 ).toInt();
    auto srids_and_types = QgsPostgresStringUtils::parseArray( res->value( 1 ).toString() );
    QgsDamengLayerProperty &layerProperty = *layerProperties[idx];

    QgsDebugMsgLevel( QStringLiteral( "Layer %1.%2.%3 has %4 srid/type combinations" )
                        .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName )
                        .arg( srids_and_types.length() ), 3);

    /* Gather found types */
    QList< std::pair<Qgis::WkbType, int> > foundCombinations;
    for ( const auto &sridAndTypeVariant : srids_and_types )
    {
      QString sridAndTypeString = sridAndTypeVariant.toString();

      QgsDebugMsgLevel( QStringLiteral( "Analyzing layer's %1.%2.%3 sridAndType %4"
                                        " against %6 found combinations"
                        ).arg( layerProperty.schemaName,layerProperty.tableName,layerProperty.geometryColName )
                        .arg( sridAndTypeString ).arg( foundCombinations.length() ), 3 );
      
      const QStringList sridAndType = sridAndTypeString.split( ':' );
      Q_ASSERT( sridAndType.size() == 3 );
      const int srid = sridAndType[0].toInt();
      QString typeString = sridAndType[1]; 
      const int zmFlags = sridAndType[2].toInt();

      if ( sridAndTypeString == "NULL" || sridAndTypeString.startsWith( ":" ) || typeString.isEmpty() )
        continue;

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

      auto type = QgsDamengConn::wkbTypeFromDmSpatial( typeString );
      auto flatType = QgsWkbTypes::flatType( type );
      auto multiType = QgsWkbTypes::multiType( flatType );
      auto curveType = QgsWkbTypes::curveType( flatType );
      auto multiCurveType = QgsWkbTypes::multiType( curveType );

      int j;
      for ( j = 0; j < foundCombinations.length(); j++)
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
          QgsDebugMsgLevel( QStringLiteral( "Udmrading type[%1] of layer %2.%3.%4 "
                                            "to multiCurved type %5" )
                            .arg( j )
                            .arg( layerProperty.schemaName,
                              layerProperty.tableName,
                              layerProperty.geometryColName )
                            .arg( multiCurveType ),
                          3);
          foundCombinations[j].first = multiCurveType;
          break;
        }
        else if ( multiType == knownMultiType )
        {
          QgsDebugMsgLevel( QStringLiteral( "Udmrading type[%1] of layer %2.%3.%4 "
                                            "to multi type %5" )
                            .arg( j )
                            .arg( layerProperty.schemaName,
                              layerProperty.tableName,
                              layerProperty.geometryColName )
                            .arg( multiType ),
                          3);
          foundCombinations[j].first = multiType;
          break;
        }
        else if ( curveType == knownCurveType )
        {
          QgsDebugMsgLevel( QStringLiteral( "Udmrading type[%1] of layer %2.%3.%4 "
                                            "to curved type %5" )
                              .arg( j )
                              .arg( layerProperty.schemaName,
                                layerProperty.tableName,
                                layerProperty.geometryColName )
                              .arg( multiType ),
                            3);
          foundCombinations[j].first = curveType;
          break;
        }
      }

      if ( j < foundCombinations.length() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Pre-existing compatible combination %1/%2 "
                                          "found for layer %3.%4.%5 " )
                            .arg( j ).arg( foundCombinations.length() )
                            .arg( layerProperty.schemaName,
                              layerProperty.tableName,
                              layerProperty.geometryColName ),
                          3);
        continue; // already found
      }

      QgsDebugMsgLevel( QStringLiteral( "Setting typeSridCombination[%1] of layer %2.%3.%4 "
                                        "to srid %5 and type %6" )
                        .arg( j )
                        .arg( layerProperty.schemaName,
                          layerProperty.tableName,
                          layerProperty.geometryColName )
                        .arg( srid )
                        .arg( type ),
                      3);

      foundCombinations << std::make_pair( type, srid );
    }

    QgsDebugMsgLevel( QStringLiteral( "Completed scan of %1 srid/type combinations "
                                      "for layer of layer %2.%3.%4 " )
                      .arg( srids_and_types.length() )
                      .arg( layerProperty.schemaName,
                        layerProperty.tableName,
                        layerProperty.geometryColName ),
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
    QgsDebugMsgLevel( QStringLiteral( "Final layer %1.%2.%3 types: %4" ).arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ).arg( layerProperty.types.length() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "Final layer %1.%2.%3 srids: %4" ).arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ).arg( layerProperty.srids.length() ), 2 );
  }
}

void QgsDamengConn::dmSpatialWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim )
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

  case Qgis::WkbType::CircularString:
    geometryType = QStringLiteral( "CIRCULARSTRING" );
    break;

  case Qgis::WkbType::CompoundCurve:
    geometryType = QStringLiteral( "COMPOUNDCURVE" );
    break;

  case Qgis::WkbType::CurvePolygon:
    geometryType = QStringLiteral( "CURVEPOLYGON" );
    break;

  case Qgis::WkbType::MultiCurve:
    geometryType = QStringLiteral( "MULTICURVE" );
    break;

  case Qgis::WkbType::MultiSurface:
    geometryType = QStringLiteral( "MULTISURFACE" );
    break;

  case Qgis::WkbType::Triangle:
    geometryType = QStringLiteral( "TRIANGLE" );
    break;

  case Qgis::WkbType::PolyhedralSurface:
    geometryType = QStringLiteral( "POLYHEDRALSURFACE" );
    break;

  case Qgis::WkbType::TIN:
    geometryType = QStringLiteral( "TIN" );
    break;

  case Qgis::WkbType::Unknown:
    geometryType = QStringLiteral( "GEOMETRY" );
    break;

  case Qgis::WkbType::NoGeometry:
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
  else if ( wkbType >= Qgis::WkbType::Point25D && wkbType <= Qgis::WkbType::MultiPolygon25D )
  {
    dim = 3;
  }
}

QString QgsDamengConn::dmSpatialWkbTypeName( Qgis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  dmSpatialWkbType( wkbType, geometryType, dim );

  return geometryType;
}

QString QgsDamengConn::dmSpatialTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry )
{
  geomCol = quotedIdentifier( geomCol );
  if ( castToGeometry )
    geomCol += QLatin1String( "::SYSGEO2.st_geometry" );

  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
  switch ( geomType )
  {
  case Qgis::GeometryType::Point:
    return QStringLiteral( "upper( DMGEO2.st_geometrytype(%1) ) IN ('ST_POINT','ST_POINTZ','ST_POINTM','ST_POINTZM','ST_MULTIPOINT','ST_MULTIPOINTZ','ST_MULTIPOINTM','ST_MULTIPOINTZM')" ).arg( geomCol );
  case Qgis::GeometryType::Line:
    return QStringLiteral( "upper( DMGEO2.st_geometrytype(%1) ) IN ('ST_LINESTRING','ST_LINESTRINGZ','ST_LINESTRINGM','ST_LINESTRINGZM','ST_CIRCULARSTRING','ST_CIRCULARSTRINGZ','ST_CIRCULARSTRINGM','ST_CIRCULARSTRINGZM','ST_COMPOUNDCURVE','ST_COMPOUNDCURVEZ','ST_COMPOUNDCURVEM','ST_COMPOUNDCURVEZM','ST_MULTILINESTRING','ST_MULTILINESTRINGZ','ST_MULTILINESTRINGM','ST_MULTILINESTRINGZM','ST_MULTICURVE','ST_MULTICURVEZ','ST_MULTICURVEM','ST_MULTICURVEZM')" ).arg( geomCol );
  case Qgis::GeometryType::Polygon:
    return QStringLiteral( "upper( DMGEO2.st_geometrytype(%1) ) IN ('ST_POLYGON','ST_POLYGONZ','ST_POLYGONM','ST_POLYGONZM','ST_CURVEPOLYGON','ST_CURVEPOLYGONZ','ST_CURVEPOLYGONM','ST_CURVEPOLYGONZM','ST_MULTIPOLYGON','ST_MULTIPOLYGONZ','ST_MULTIPOLYGONM','ST_MULTIPOLYGONZM','ST_MULTIPOLYGONM','ST_MULTISURFACE','ST_MULTISURFACEZ','ST_MULTISURFACEM','ST_MULTISURFACEZM','ST_POLYHEDRALSURFACE','ST_TIN')" ).arg( geomCol );
  case Qgis::GeometryType::Null:
    return QStringLiteral( "DMGEO2.st_geometrytype(%1) IS NULL" ).arg( geomCol );
  default: //unknown geometry
    return QString();
  }
}

int QgsDamengConn::dmSpatialWkbTypeDim( Qgis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  dmSpatialWkbType( wkbType, geometryType, dim );

  return dim;
}

Qgis::WkbType QgsDamengConn::wkbTypeFromDmSpatial( const QString &type1 )
{
  // Polyhedral surfaces and TIN are stored in Dameng as geometry collections
  // of Polygons and Triangles.
  // So, since QGIS does not natively support PS and TIN, but we would like to open them if possible,
  // we consider them as multipolygons. WKB will be converted by the feature iterator
  QString type = type1.mid( 3, -1 );  // Remove the prefix "ST_" from the Dameng spatial type name
  
  return QgsWkbTypes::parseType( type );
}

Qgis::WkbType QgsDamengConn::wkbTypeFromOgcWkbType( unsigned int wkbType )
{
  return ( Qgis::WkbType )wkbType;
}

QString QgsDamengConn::displayStringForWkbType( Qgis::WkbType type )
{
  return QgsWkbTypes::displayString( type );
}

QString QgsDamengConn::displayStringForGeomType( QgsDamengGeometryColumnType type )
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
  }

  Q_ASSERT( !"unexpected geometry column type" );
  return QString();
}

Qgis::WkbType QgsDamengConn::wkbTypeFromGeomType( Qgis::GeometryType geomType )
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

QStringList QgsDamengConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "Dameng/connections" ) );
  return settings.childGroups();
}

QString QgsDamengConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "Dameng/connections/selected" ) ).toString();
}

void QgsDamengConn::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( QStringLiteral( "Dameng/connections/selected" ), name );
}

QgsDataSourceUri QgsDamengConn::connUri( const QString &connName )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 2 );

  QgsSettings settings;

  QString key = "/Dameng/connections/" + connName;

  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = QStringLiteral( "5236" );
  }

  bool estimatedMetadata = useEstimatedMetadata( connName );

  QString username;
  QString password;
  if ( settings.value( key + QStringLiteral( "/saveUsername" ) ).toString() == QLatin1String( "true" ) )
  {
    username = settings.value( key + QStringLiteral( "/username" ) ).toString();
  }

  if ( settings.value( key + QStringLiteral( "/savePassword" ) ).toString() == QLatin1String( "true" ) )
  {
    password = settings.value( key + QStringLiteral( "/password" ) ).toString();
  }

  // Old save setting
  if ( settings.contains( key + QStringLiteral( "/save" ) ) )
  {
    username = settings.value( key + QStringLiteral( "/username" ) ).toString();

    if ( settings.value( key + QStringLiteral( "/save" ) ).toString() == QLatin1String( "true" ) )
    {
      password = settings.value( key + QStringLiteral( "/password" ) ).toString();
    }
  }

  QgsDataSourceUri uri;
  uri.setConnection( host, port, "dameng", username, password );

  QString authcfg = settings.value( key + QStringLiteral( "/authcfg" ) ).toString();
  if ( !authcfg.isEmpty() )
  {
    uri.setAuthConfigId( authcfg );
  }

  uri.setUseEstimatedMetadata( estimatedMetadata );

  return uri;
}

bool QgsDamengConn::sysdbaSchemaOnly( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Dameng/connections/" + connName + "/sysdbaOnly", false ).toBool();
}

bool QgsDamengConn::dontResolveType( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Dameng/connections/" + connName + "/dontResolveType", false ).toBool();
}

bool QgsDamengConn::useEstimatedMetadata( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Dameng/connections/" + connName + "/estimatedMetadata", false ).toBool();
}


bool QgsDamengConn::allowGeometrylessTables( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Dameng/connections/" + connName + "/allowGeometrylessTables", false ).toBool();
}

bool QgsDamengConn::allowProjectsInDatabase( const QString &connName )
{
  QgsSettings settings;
  return settings.value( "/Dameng/connections/" + connName + "/projectsInDatabase", false ).toBool();
}

void QgsDamengConn::deleteConnection( const QString &connName )
{
  QgsSettings settings;

  QString key = "/Dameng/connections/" + connName;
  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/sysdbaOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/estimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/save" );
  settings.remove( key + "/authcfg" );
  settings.remove( key + "/projectsInDatabase" );
  settings.remove( key + "/dontResolveType" );
  settings.remove( key );
}

bool QgsDamengConn::cancel()
{
  QMutexLocker locker( &mLock );

  char errbuf[256];
  DPIRETURN ret = 0;

  dhstmt stmt = *mConn->dmResult->getStmt();
  ret = dpi_cancel( stmt );
  if ( ret != DSQL_SUCCESS )
  {
    QgsMessageLog::logMessage( tr( "Query could not be canceled [%1]" ).arg( errbuf ), tr( "Dameng" ) );
    return false;
  }

  return true;
}

bool QgsDamengConn::DMCancel()
{
  DPIRETURN ret = 0;

  dhstmt stmt = *mConn->dmResult->getStmt();
  ret = dpi_cancel( stmt );
  if ( ret != DSQL_SUCCESS )
  {
    QgsDebugMsgLevel( QStringLiteral( "Error canceling the query" ), 3 );
    return false;
  }

  return true;
}

QString QgsDamengConn::currentDatabase() const
{
  QMutexLocker locker( &mLock );
  QString database;
  QString sql( QStringLiteral( "SELECT instance_name from v$instance;" ) );
  QgsDMResult *res = DMexec( sql );

  if ( res->ntuples() )
    database = res->value( 0 ).toString();
  else
    QgsMessageLog::logMessage( tr( "SQL: %1\nerror: %2\n" ).arg( sql ).arg( res->getMsg() ), tr( "Dameng" ) );

  return database;
}

QgsCoordinateReferenceSystem QgsDamengConn::sridToCrs( int srid )
{
  QgsCoordinateReferenceSystem crs;
  static QMutex sMutex;
  QMutexLocker locker( &sMutex );
  static QMap<int, QgsCoordinateReferenceSystem> sCrsCache;
  if ( sCrsCache.contains( srid ) )
    crs = sCrsCache.value( srid );
  else
  {
    QgsDMResult *result( DMexec( QStringLiteral( "SELECT auth_name, auth_srid, srtext, proj4text FROM SYSGEO2.spatial_ref_sys WHERE auth_srid=%1" ).arg( srid ) ) );
    result->fetchNext();
    if ( result && result->execstatus() )
    {
      const QString authName = result->value( 0 ).toString();
      const QString authSRID = result->value( 1 ).toString();
      const QString srText = result->value( 2 ).toString();
      bool ok = false;
      if ( authName == QLatin1String( "EPSG" ) || authName == QLatin1String( "ESRI" ) )
      {
        ok = crs.createFromUserInput( authName + ':' + authSRID );
      }
      if ( !ok && !srText.isEmpty() )
      {
        ok = crs.createFromUserInput( srText );
      }
      if ( !ok )
        crs = QgsCoordinateReferenceSystem::fromProj( result->value( 3 ).toString() );
      sCrsCache.insert( srid, crs);
    }
    
  }
  
  return crs;
}
