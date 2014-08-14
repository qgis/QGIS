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
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsmessagelog.h"
#include "qgscredentials.h"
#include "qgsfield.h"
#include "qgspgtablemodel.h"

#include <QApplication>
#include <QSettings>
#include <QThread>

#include <climits>

// for htonl
#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif


QgsPostgresResult::~QgsPostgresResult()
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = 0;
}

QgsPostgresResult &QgsPostgresResult::operator=( PGresult * theRes )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = theRes;
  return *this;
}

QgsPostgresResult &QgsPostgresResult::operator=( const QgsPostgresResult & src )
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
         ? QString::null
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

int QgsPostgresResult::PQftable( int col )
{
  Q_ASSERT( mRes );
  return ::PQftable( mRes, col );
}

int QgsPostgresResult::PQftablecol( int col )
{
  Q_ASSERT( mRes );
  return ::PQftablecol( mRes, col );
}

int QgsPostgresResult::PQftype( int col )
{
  Q_ASSERT( mRes );
  return ::PQftype( mRes, col );
}

Oid QgsPostgresResult::PQoidValue()
{
  Q_ASSERT( mRes );
  return ::PQoidValue( mRes );
}


QMap<QString, QgsPostgresConn *> QgsPostgresConn::sConnectionsRO;
QMap<QString, QgsPostgresConn *> QgsPostgresConn::sConnectionsRW;
const int QgsPostgresConn::sGeomTypeSelectLimit = 100;

QgsPostgresConn *QgsPostgresConn::connectDb( QString conninfo, bool readonly, bool shared )
{
  QMap<QString, QgsPostgresConn *> &connections =
    readonly ? QgsPostgresConn::sConnectionsRO : QgsPostgresConn::sConnectionsRW;

  if ( shared )
  {
    if ( connections.contains( conninfo ) )
    {
      QgsDebugMsg( QString( "Using cached connection for %1" ).arg( conninfo ) );
      connections[conninfo]->mRef++;
      return connections[conninfo];
    }
  }

  QgsPostgresConn *conn = new QgsPostgresConn( conninfo, readonly, shared );

  if ( conn->mRef == 0 )
  {
    delete conn;
    return 0;
  }

  if ( shared )
  {
    connections.insert( conninfo, conn );
  }

  return conn;
}

QgsPostgresConn::QgsPostgresConn( QString conninfo, bool readOnly, bool shared )
    : mRef( 1 )
    , mOpenCursors( 0 )
    , mConnInfo( conninfo )
    , mGotPostgisVersion( false )
    , mReadOnly( readOnly )
    , mNextCursorId( 0 )
    , mShared( shared )
{
  QgsDebugMsg( QString( "New PostgreSQL connection for " ) + conninfo );

  mConn = PQconnectdb( conninfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus() != CONNECTION_OK )
  {
    QgsDataSourceURI uri( conninfo );
    QString username = uri.username();
    QString password = uri.password();

    QgsCredentials::instance()->lock();

    while ( PQstatus() != CONNECTION_OK )
    {
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, PQerrorMessage() );
      if ( !ok )
        break;

      PQfinish();

      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsg( "Connecting to " + uri.connectionInfo() );
      mConn = PQconnectdb( uri.connectionInfo().toLocal8Bit() );
    }

    if ( PQstatus() == CONNECTION_OK )
      QgsCredentials::instance()->put( conninfo, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( PQstatus() != CONNECTION_OK )
  {
    QString errorMsg = PQerrorMessage();
    PQfinish();
    QgsMessageLog::logMessage( tr( "Connection to database failed" ) + "\n" + errorMsg, tr( "PostGIS" ) );
    mRef = 0;
    return;
  }

  //set client encoding to unicode because QString uses UTF-8 anyway
  QgsDebugMsg( "setting client encoding to UNICODE" );
  int errcode = PQsetClientEncoding( mConn, QString( "UNICODE" ).toLocal8Bit() );
  if ( errcode == 0 )
  {
    QgsDebugMsg( "encoding successfully set" );
  }
  else if ( errcode == -1 )
  {
    QgsMessageLog::logMessage( tr( "error in setting encoding" ), tr( "PostGIS" ) );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "undefined return value from encoding setting" ), tr( "PostGIS" ) );
  }

  QgsDebugMsg( "Connection to the database was successful" );

  deduceEndian();

  /* Check to see if we have working PostGIS support */
  if ( postgisVersion().isNull() )
  {
    QgsMessageLog::logMessage( tr( "Your database has no working PostGIS support." ), tr( "PostGIS" ) );
    PQfinish();
    mRef = 0;
    return;
  }

  if ( mPostgresqlVersion >= 90000 )
  {
    PQexecNR( "SET application_name='QGIS'" );
  }

  /* Check to see if we have GEOS support and if not, warn the user about
     the problems they will see :) */
  QgsDebugMsg( "Checking for GEOS support" );

  if ( !hasGEOS() )
  {
    QgsMessageLog::logMessage( tr( "Your PostGIS installation has no GEOS support. Feature selection and identification will not work properly. Please install PostGIS with GEOS support (http://geos.refractions.net)" ), tr( "PostGIS" ) );
  }

  if ( hasTopology() )
  {
    QgsDebugMsg( "Topology support available!" );
  }
}

QgsPostgresConn::~QgsPostgresConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mConn )
    ::PQfinish( mConn );
  mConn = 0;
}

void QgsPostgresConn::disconnect()
{
  if ( --mRef > 0 )
    return;

  if ( mShared )
  {
    QMap<QString, QgsPostgresConn *>& connections = mReadOnly ? sConnectionsRO : sConnectionsRW;

    QString key = connections.key( this, QString::null );

    Q_ASSERT( !key.isNull() );
    connections.remove( key );
  }

  delete this;
}

/* private */
void QgsPostgresConn::addColumnInfo( QgsPostgresLayerProperty& layerProperty, const QString& schemaName, const QString& viewName, bool fetchPkCandidates )
{
  // TODO: optimize this query when pk candidates aren't needed
  //       could use array_agg() and count()
  //       array output would look like this: "{One,tWo}"
  QString sql = QString( "SELECT attname, CASE WHEN typname = ANY(ARRAY['geometry','geography','topogeometry']) THEN 1 ELSE null END AS isSpatial FROM pg_attribute JOIN pg_type ON atttypid=pg_type.oid WHERE attrelid=regclass('%1.%2')" )
                .arg( quotedIdentifier( schemaName ) )
                .arg( quotedIdentifier( viewName ) );
  QgsDebugMsg( sql );
  QgsPostgresResult colRes = PQexec( sql );

  layerProperty.pkCols.clear();
  layerProperty.nSpCols = 0;

  if ( colRes.PQresultStatus() == PGRES_TUPLES_OK )
  {
    for ( int i = 0; i < colRes.PQntuples(); i++ )
    {
      if ( fetchPkCandidates )
      {
        QgsDebugMsg( colRes.PQgetvalue( i, 0 ) );
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
    QgsMessageLog::logMessage( tr( "SQL:%1\nresult:%2\nerror:%3\n" ).arg( sql ).arg( colRes.PQresultStatus() ).arg( colRes.PQresultErrorMessage() ), tr( "PostGIS" ) );
  }

}

bool QgsPostgresConn::getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables )
{
  int nColumns = 0;
  int foundInTables = 0;
  QgsPostgresResult result;
  QgsPostgresLayerProperty layerProperty;

  QgsDebugMsg( "Entering." );

  mLayersSupported.clear();

  for ( int i = 0; i < ( hasTopology() ? 3 : 2 ); i++ )
  {
    QString sql, tableName, schemaName, columnName, typeName, sridName, gtableName, dimName;
    QgsPostgresGeometryColumnType columnType = sctGeometry;

    if ( i == 0 )
    {
      tableName  = "l.f_table_name";
      schemaName = "l.f_table_schema";
      columnName = "l.f_geometry_column";
      typeName   = "upper(l.type)";
      sridName   = "l.srid";
      dimName    = "l.coord_dimension";
      gtableName = "geometry_columns";
      columnType = sctGeometry;
    }
    else if ( i == 1 )
    {
      tableName  = "l.f_table_name";
      schemaName = "l.f_table_schema";
      columnName = "l.f_geography_column";
      typeName   = "upper(l.type)";
      sridName   = "l.srid";
      dimName    = "2";
      gtableName = "geography_columns";
      columnType = sctGeography;
    }
    else if ( i == 2 )
    {
      schemaName = "l.schema_name";
      tableName  = "l.table_name";
      columnName = "l.feature_column";
      typeName   = "CASE "
                   "WHEN l.feature_type = 1 THEN 'MULTIPOINT' "
                   "WHEN l.feature_type = 2 THEN 'MULTILINESTRING' "
                   "WHEN l.feature_type = 3 THEN 'MULTIPOLYGON' "
                   "WHEN l.feature_type = 4 THEN 'GEOMETRYCOLLECTION' "
                   "END AS type";
      sridName   = "(SELECT srid FROM topology.topology t WHERE l.topology_id=t.id)";
      dimName    = "2";
      gtableName = "topology.layer";
      columnType = sctTopoGeometry;
    }

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.
    sql = QString( "SELECT %1,%2,%3,%4,%5,%6,c.relkind"
                   " FROM %7 l,pg_class c,pg_namespace n"
                   " WHERE c.relname=%1"
                   " AND %2=n.nspname"
                   " AND n.oid=c.relnamespace"
                   " AND has_schema_privilege(n.nspname,'usage')"
                   " AND has_table_privilege('\"'||n.nspname||'\".\"'||c.relname||'\"','select')" // user has select privilege
                 )
          .arg( tableName ).arg( schemaName ).arg( columnName ).arg( typeName ).arg( sridName ).arg( dimName ).arg( gtableName );

    if ( searchPublicOnly )
      sql += " AND n.nspname='public'";

    sql += QString( " ORDER BY n.nspname,c.relname,%1" ).arg( columnName );

    QgsDebugMsg( "getting table info: " + sql );
    result = PQexec( sql, i == 0 );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      PQexecNR( "COMMIT" );
      continue;
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
      bool isView = relkind == "v" || relkind == "m";

      int srid = ssrid.isEmpty() ? INT_MIN : ssrid.toInt();
      if ( majorVersion() >= 2 && srid == 0 )
      {
        // 0 doesn't constraint => detect
        srid = INT_MIN;
      }

      QgsDebugMsg( QString( "%1 : %2.%3.%4: %5 %6 %7 %8" )
                   .arg( gtableName )
                   .arg( schemaName ).arg( tableName ).arg( column )
                   .arg( type )
                   .arg( srid )
                   .arg( relkind )
                   .arg( dim ) );

      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = column;
      layerProperty.geometryColType = columnType;
      layerProperty.types = QList<QGis::WkbType>() << ( QgsPostgresConn::wkbTypeFromPostgis( type ) );
      layerProperty.srids = QList<int>() << srid;
      layerProperty.sql = "";
      layerProperty.force2d = dim == 4;
      addColumnInfo( layerProperty, schemaName, tableName, isView );

      if ( isView && layerProperty.pkCols.empty() )
      {
        QgsDebugMsg( "no key columns found." );
        continue;
      }

      mLayersSupported << layerProperty;
      nColumns++;
    }

    foundInTables |= 1 << i;
  }

  if ( nColumns == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "PostGIS" ) );
  }

  //search for geometry columns in tables that are not in the geometry_columns metatable
  if ( !searchGeometryColumnsOnly )
  {
    // Now have a look for geometry columns that aren't in the geometry_columns table.
    QString sql = "SELECT"
                  " c.relname"
                  ",n.nspname"
                  ",a.attname"
                  ",c.relkind"
                  ",CASE WHEN t.typname IN ('geometry','geography','topogeometry') THEN t.typname ELSE b.typname END AS coltype"
                  " FROM pg_attribute a"
                  " JOIN pg_class c ON c.oid=a.attrelid"
                  " JOIN pg_namespace n ON n.oid=c.relnamespace"
                  " JOIN pg_type t ON t.oid=a.atttypid"
                  " LEFT JOIN pg_type b ON b.oid=t.typbasetype"
                  " WHERE c.relkind IN ('v','r','m')"
                  " AND has_schema_privilege( n.nspname, 'usage' )"
                  " AND has_table_privilege( '\"' || n.nspname || '\".\"' || c.relname || '\"', 'select' )"
                  " AND (t.typname IN ('geometry','geography','topogeometry') OR b.typname IN ('geometry','geography','topogeometry'))";

    // user has select privilege
    if ( searchPublicOnly )
      sql += " AND n.nspname='public'";

    // skip columns of which we already derived information from the metadata tables
    if ( nColumns > 0 )
    {
      if ( foundInTables & 1 )
      {
        sql += " AND (n.nspname,c.relname,a.attname) NOT IN (SELECT f_table_schema,f_table_name,f_geometry_column FROM geometry_columns)";
      }

      if ( foundInTables & 2 )
      {
        sql += " AND (n.nspname,c.relname,a.attname) NOT IN (SELECT f_table_schema,f_table_name,f_geography_column FROM geography_columns)";
      }

      if ( foundInTables & 4 )
      {
        sql += " AND (n.nspname,c.relname,a.attname) NOT IN (SELECT schema_name,table_name,feature_column FROM topology.layer)";
      }
    }

    QgsDebugMsg( "sql: " + sql );

    result = PQexec( sql );

    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined. The error message from the database was:\n%1\n" )
                                 .arg( result.PQresultErrorMessage() ),
                                 tr( "PostGIS" ) );
      PQexecNR( "COMMIT" );
      return false;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      // Have the column name, schema name and the table name. The concept of a
      // catalog doesn't exist in postgresql so we ignore that, but we
      // do need to get the geometry type.

      // Make the assumption that the geometry type for the first
      // row is the same as for all other rows.

      QString tableName  = result.PQgetvalue( i, 0 ); // relname
      QString schemaName = result.PQgetvalue( i, 1 ); // nspname
      QString column     = result.PQgetvalue( i, 2 ); // attname
      QString relkind    = result.PQgetvalue( i, 3 ); // relation kind
      QString coltype    = result.PQgetvalue( i, 4 ); // column type
      bool isView = relkind == "v" || relkind == "m";

      QgsDebugMsg( QString( "%1.%2.%3: %4" ).arg( schemaName ).arg( tableName ).arg( column ).arg( relkind ) );

      layerProperty.types.clear();
      layerProperty.srids.clear();
      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = column;
      if ( coltype == "geometry" )
      {
        layerProperty.geometryColType = sctGeometry;
      }
      else if ( coltype == "geography" )
      {
        layerProperty.geometryColType = sctGeography;
      }
      else if ( coltype == "topogeometry" )
      {
        layerProperty.geometryColType = sctTopoGeometry;
      }
      else
      {
        Q_ASSERT( !"Unknown geometry type" );
      }

      addColumnInfo( layerProperty, schemaName, tableName, isView );
      if ( isView && layerProperty.pkCols.empty() )
      {
        QgsDebugMsg( "no key columns found." );
        continue;
      }

      layerProperty.sql = "";

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( allowGeometrylessTables )
  {
    QString sql = "SELECT "
                  "pg_class.relname"
                  ",pg_namespace.nspname"
                  ",pg_class.relkind"
                  " FROM "
                  " pg_class"
                  ",pg_namespace"
                  " WHERE pg_namespace.oid=pg_class.relnamespace"
                  " AND has_schema_privilege(pg_namespace.nspname,'usage')"
                  " AND has_table_privilege('\"' || pg_namespace.nspname || '\".\"' || pg_class.relname || '\"','select')"
                  " AND pg_class.relkind IN ('v','r','m')";

    // user has select privilege
    if ( searchPublicOnly )
      sql += " AND pg_namespace.nspname='public'";

    QgsDebugMsg( "sql: " + sql );

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
      bool isView = relkind == "v" || relkind == "m";

      QgsDebugMsg( QString( "%1.%2: %3" ).arg( schema ).arg( table ).arg( relkind ) );

      layerProperty.types = QList<QGis::WkbType>() << QGis::WKBNoGeometry;
      layerProperty.srids = QList<int>() << INT_MIN;
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = QString::null;
      layerProperty.geometryColType = sctNone;
      addColumnInfo( layerProperty, schema, table, isView );
      layerProperty.sql = "";

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( nColumns == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but no accessible tables were found. Please verify that you have SELECT privilege on a table carrying PostGIS geometry." ), tr( "PostGIS" ) );
  }

  return true;
}

bool QgsPostgresConn::supportedLayers( QVector<QgsPostgresLayerProperty> &layers, bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables )
{
  // Get the list of supported tables
  if ( !getTableInfo( searchGeometryColumnsOnly, searchPublicOnly, allowGeometrylessTables ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "PostGIS" ) );
    return false;
  }

  layers = mLayersSupported;

  QgsDebugMsg( "Exiting." );

  return true;
}

/**
 * Check to see if GEOS is available
 */
bool QgsPostgresConn::hasGEOS()
{
  // make sure info is up to date for the current connection
  postgisVersion();
  // get geos capability
  return mGeosAvailable;
}

/**
 * Check to see if topology is available
 */
bool QgsPostgresConn::hasTopology()
{
  // make sure info is up to date for the current connection
  postgisVersion();
  // get topology capability
  return mTopologyAvailable;
}

/* Functions for determining available features in postGIS */
QString QgsPostgresConn::postgisVersion()
{
  if ( mGotPostgisVersion )
    return mPostgisVersionInfo;

  mPostgresqlVersion = PQserverVersion( mConn );

  QgsPostgresResult result = PQexec( "SELECT postgis_version()" );
  if ( result.PQntuples() != 1 )
  {
    QgsMessageLog::logMessage( tr( "Retrieval of postgis version failed" ), tr( "PostGIS" ) );
    return QString::null;
  }

  mPostgisVersionInfo = result.PQgetvalue( 0, 0 );

  QgsDebugMsg( "PostGIS version info: " + mPostgisVersionInfo );

  QStringList postgisParts = mPostgisVersionInfo.split( " ", QString::SkipEmptyParts );

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split( ".", QString::SkipEmptyParts );
  if ( postgisVersionParts.size() < 2 )
  {
    QgsMessageLog::logMessage( tr( "Could not parse postgis version string '%1'" ).arg( mPostgisVersionInfo ), tr( "PostGIS" ) );
    return QString::null;
  }

  mPostgisVersionMajor = postgisVersionParts[0].toInt();
  mPostgisVersionMinor = postgisVersionParts[1].toInt();

  mUseWkbHex = mPostgisVersionMajor < 1;

  // apparently postgis 1.5.2 doesn't report capabilities in postgis_version() anymore
  if ( mPostgisVersionMajor > 1 || ( mPostgisVersionMajor == 1 && mPostgisVersionMinor >= 5 ) )
  {
    result = PQexec( "SELECT postgis_geos_version(),postgis_proj_version()" );
    mGeosAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 );
    mProjAvailable = result.PQntuples() == 1 && !result.PQgetisnull( 0, 1 );
    QgsDebugMsg( QString( "geos:%1 proj:%2" )
                 .arg( mGeosAvailable ? result.PQgetvalue( 0, 0 ) : "none" )
                 .arg( mProjAvailable ? result.PQgetvalue( 0, 1 ) : "none" ) );
    mGistAvailable = true;
  }
  else
  {
    // assume no capabilities
    mGeosAvailable = false;
    mGistAvailable = false;
    mProjAvailable = false;

    // parse out the capabilities and store them
    QStringList geos = postgisParts.filter( "GEOS" );
    if ( geos.size() == 1 )
    {
      mGeosAvailable = ( geos[0].indexOf( "=1" ) > -1 );
    }
    QStringList gist = postgisParts.filter( "STATS" );
    if ( gist.size() == 1 )
    {
      mGistAvailable = ( geos[0].indexOf( "=1" ) > -1 );
    }
    QStringList proj = postgisParts.filter( "PROJ" );
    if ( proj.size() == 1 )
    {
      mProjAvailable = ( proj[0].indexOf( "=1" ) > -1 );
    }
  }

  // checking for topology support
  QgsDebugMsg( "Checking for topology support" );
  mTopologyAvailable = false;
  if ( mPostgisVersionMajor > 1 )
  {
    QgsPostgresResult result = PQexec( "SELECT EXISTS ( SELECT c.oid FROM pg_class AS c JOIN pg_namespace AS n ON c.relnamespace=n.oid WHERE n.nspname='topology' AND c.relname='topology' )" );
    if ( result.PQntuples() >= 1 && result.PQgetvalue( 0, 0 ) == "t" )
    {
      mTopologyAvailable = true;
    }
  }

  mGotPostgisVersion = true;

  return mPostgisVersionInfo;
}

QString QgsPostgresConn::quotedIdentifier( QString ident )
{
  ident.replace( '"', "\"\"" );
  ident = ident.prepend( "\"" ).append( "\"" );
  return ident;
}

QString QgsPostgresConn::quotedValue( QVariant value )
{
  if ( value.isNull() )
    return "NULL";

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( "'", "''" );
      if ( v.contains( "\\" ) )
        return v.replace( "\\", "\\\\" ).prepend( "E'" ).append( "'" );
      else
        return v.prepend( "'" ).append( "'" );
  }
}

PGresult *QgsPostgresConn::PQexec( QString query, bool logError )
{
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
      QgsDebugMsg( QString( "Connection error: %1 returned %2 [%3]" )
                   .arg( query ).arg( PQstatus() ).arg( PQerrorMessage() ) );
    }

    return 0;
  }

  QgsDebugMsgLevel( QString( "Executing SQL: %1" ).arg( query ), 3 );
  PGresult *res = ::PQexec( mConn, query.toUtf8() );

  if ( res )
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
        QgsDebugMsg( QString( "Not logged erroneous query: %1 returned %2 [%3]" )
                     .arg( query ).arg( errorStatus ).arg( PQresultErrorMessage( res ) ) );
      }
    }
  }
  else if ( logError )
  {
    QgsMessageLog::logMessage( tr( "Query failed: %1\nError: no result buffer" ).arg( query ), tr( "PostGIS" ) );
  }
  else
  {
    QgsDebugMsg( QString( "Not logged query failed: %1\nError: no result buffer" ).arg( query ) );
  }

  return res;
}

bool QgsPostgresConn::openCursor( QString cursorName, QString sql )
{
  if ( mOpenCursors++ == 0 )
  {
    QgsDebugMsg( "Starting read-only transaction" );
    PQexecNR( "BEGIN READ ONLY" );
  }
  QgsDebugMsgLevel( QString( "Binary cursor %1 for %2" ).arg( cursorName ).arg( sql ), 3 );
  return PQexecNR( QString( "DECLARE %1 BINARY CURSOR FOR %2" ).arg( cursorName ).arg( sql ) );
}

bool QgsPostgresConn::closeCursor( QString cursorName )
{
  if ( !PQexecNR( QString( "CLOSE %1" ).arg( cursorName ) ) )
    return false;

  if ( --mOpenCursors == 0 )
  {
    QgsDebugMsg( "Committing read-only transaction" );
    PQexecNR( "COMMIT" );
  }

  return true;
}

QString QgsPostgresConn::uniqueCursorName()
{
  return QString( "qgis_%1" ).arg( ++mNextCursorId );
}

bool QgsPostgresConn::PQexecNR( QString query, bool retry )
{
  QgsPostgresResult res = PQexec( query, false );

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
    PQexecNR( "ROLLBACK" );
  }
  else if ( retry )
  {
    QgsMessageLog::logMessage( tr( "resetting bad connection." ), tr( "PostGIS" ) );
    ::PQreset( mConn );
    if ( PQstatus() == CONNECTION_OK )
    {
      if ( PQexecNR( query, false ) )
      {
        QgsMessageLog::logMessage( tr( "retry after reset succeeded." ), tr( "PostGIS" ) );
        return true;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "retry after reset failed again." ), tr( "PostGIS" ) );
        return false;
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

  return false;
}

PGresult *QgsPostgresConn::PQgetResult()
{
  return ::PQgetResult( mConn );
}

PGresult *QgsPostgresConn::PQprepare( QString stmtName, QString query, int nParams, const Oid *paramTypes )
{
  return ::PQprepare( mConn, stmtName.toUtf8(), query.toUtf8(), nParams, paramTypes );
}

PGresult *QgsPostgresConn::PQexecPrepared( QString stmtName, const QStringList &params )
{
  const char **param = new const char *[ params.size()];
  QList<QByteArray> qparam;

  for ( int i = 0; i < params.size(); i++ )
  {
    qparam << params[i].toUtf8();

    if ( params[i].isNull() )
      param[i] = 0;
    else
      param[i] = qparam[i];
  }

  PGresult *res = ::PQexecPrepared( mConn, stmtName.toUtf8(), params.size(), param, NULL, NULL, 0 );

  delete [] param;

  return res;
}

void QgsPostgresConn::PQfinish()
{
  Q_ASSERT( mConn );
  ::PQfinish( mConn );
  mConn = 0;
}

int QgsPostgresConn::PQstatus()
{
  Q_ASSERT( mConn );
  return ::PQstatus( mConn );
}

QString QgsPostgresConn::PQerrorMessage()
{
  Q_ASSERT( mConn );
  return QString::fromUtf8( ::PQerrorMessage( mConn ) );
}

int QgsPostgresConn::PQsendQuery( QString query )
{
  Q_ASSERT( mConn );
  return ::PQsendQuery( mConn, query.toUtf8() );
}

qint64 QgsPostgresConn::getBinaryInt( QgsPostgresResult &queryResult, int row, int col )
{
  quint64 oid;
  char *p = PQgetvalue( queryResult.result(), row, col );
  size_t s = PQgetlength( queryResult.result(), row, col );

#ifdef QGISDEBUG
  if ( QgsLogger::debugLevel() >= 4 )
  {
    QString buf;
    for ( size_t i = 0; i < s; i++ )
    {
      buf += QString( "%1 " ).arg( *( unsigned char * )( p + i ), 0, 16, QLatin1Char( ' ' ) );
    }

    QgsDebugMsg( QString( "int in hex:%1" ).arg( buf ) );
  }
#endif

  switch ( s )
  {
    case 2:
      oid = *( quint16 * )p;
      if ( mSwapEndian )
        oid = ntohs( oid );
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
        QgsDebugMsgLevel( QString( "swap oid0:%1 oid1:%2" ).arg( oid0 ).arg( oid1 ), 4 );
        oid0 = ntohl( oid0 );
        oid1 = ntohl( oid1 );
      }

      QgsDebugMsgLevel( QString( "oid0:%1 oid1:%2" ).arg( oid0 ).arg( oid1 ), 4 );
      oid   = oid0;
      QgsDebugMsgLevel( QString( "oid:%1" ).arg( oid ), 4 );
      oid <<= 32;
      QgsDebugMsgLevel( QString( "oid:%1" ).arg( oid ), 4 );
      oid  |= oid1;
      QgsDebugMsgLevel( QString( "oid:%1" ).arg( oid ), 4 );
    }
    break;

    default:
      QgsDebugMsg( QString( "unexpected size %1" ).arg( s ) );

    case 4:
      oid = *( quint32 * )p;
      if ( mSwapEndian )
        oid = ntohl( oid );
      break;
  }

  return oid;
}

QString QgsPostgresConn::fieldExpression( const QgsField &fld )
{
  const QString &type = fld.typeName();
  if ( type == "money" )
  {
    return QString( "cash_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type.startsWith( "_" ) )
  {
    return QString( "array_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "bool" )
  {
    return QString( "boolout(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geometry" )
  {
    return QString( "%1(%2)" )
           .arg( majorVersion() < 2 ? "asewkt" : "st_asewkt" )
           .arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geography" )
  {
    return QString( "st_astext(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else
  {
    return quotedIdentifier( fld.name() ) + "::text";
  }
}

void QgsPostgresConn::deduceEndian()
{
  // need to store the PostgreSQL endian format used in binary cursors
  // since it appears that starting with
  // version 7.4, binary cursors return data in XDR whereas previous versions
  // return data in the endian of the server

  QgsPostgresResult res = PQexec( "select regclass('pg_class')::oid" );
  QString oidValue = res.PQgetvalue( 0, 0 );

  QgsDebugMsg( "Creating binary cursor" );

  // get the same value using a binary cursor
  openCursor( "oidcursor", "select regclass('pg_class')::oid" );

  QgsDebugMsg( "Fetching a record and attempting to get check endian-ness" );

  res = PQexec( "fetch forward 1 from oidcursor" );

  mSwapEndian = true;
  if ( res.PQntuples() > 0 )
  {
    // get the oid value from the binary cursor
    qint64 oid = getBinaryInt( res, 0, 0 );

    QgsDebugMsg( QString( "Got oid of %1 from the binary cursor" ).arg( oid ) );
    QgsDebugMsg( QString( "First oid is %1" ).arg( oidValue ) );

    // compare the two oid values to determine if we need to do an endian swap
    if ( oid != oidValue.toLongLong() )
      mSwapEndian = false;
  }

  closeCursor( "oidcursor" );
}

void QgsPostgresConn::retrieveLayerTypes( QgsPostgresLayerProperty &layerProperty, bool useEstimatedMetadata )
{
  QString table;

  if ( !layerProperty.schemaName.isEmpty() )
  {
    table = QString( "%1.%2" )
            .arg( quotedIdentifier( layerProperty.schemaName ) )
            .arg( quotedIdentifier( layerProperty.tableName ) );
  }
  else
  {
    // Query
    table = layerProperty.tableName;
  }

  if ( !layerProperty.geometryColName.isEmpty() )
  {
    // our estimatation ignores that a where clause might restrict the feature type or srid
    if ( useEstimatedMetadata )
    {
      table = QString( "(SELECT %1 FROM %2%3 LIMIT %4) AS t" )
              .arg( quotedIdentifier( layerProperty.geometryColName ) )
              .arg( table )
              .arg( layerProperty.sql.isEmpty() ? "" : QString( " WHERE %1" ).arg( layerProperty.sql ) )
              .arg( sGeomTypeSelectLimit );
    }
    else if ( !layerProperty.sql.isEmpty() )
    {
      table += QString( " WHERE %1" ).arg( layerProperty.sql );
    }

    QString query = "SELECT DISTINCT ";

    QGis::WkbType type = layerProperty.types.value( 0, QGis::WKBUnknown );
    if ( type == QGis::WKBUnknown )
    {
      query += QString( "upper(geometrytype(%1%2))" )
               .arg( quotedIdentifier( layerProperty.geometryColName ) )
               .arg( layerProperty.geometryColType == sctGeography ? "::geometry" : "" );
    }
    else
    {
      query += quotedValue( QgsPostgresConn::postgisWkbTypeName( type ) );
    }

    query += ",";

    int srid = layerProperty.srids.value( 0, INT_MIN );
    if ( srid  == INT_MIN )
    {
      query += QString( "%1(%2%3)" )
               .arg( majorVersion() < 2 ? "srid" : "st_srid" )
               .arg( quotedIdentifier( layerProperty.geometryColName ) )
               .arg( layerProperty.geometryColType == sctGeography ? "::geometry" : "" );
    }
    else
    {
      query += QString::number( srid );
    }

    query += " FROM " + table;

    QgsDebugMsg( "Retrieving geometry types: " + query );

    QgsPostgresResult gresult = PQexec( query );

    if ( gresult.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < gresult.PQntuples(); i++ )
      {
        QString type = gresult.PQgetvalue( i, 0 );
        QString srid = gresult.PQgetvalue( i, 1 );
        if ( type.isEmpty() )
          continue;

        // if both multi and single types exists, go for the multi type,
        // so that st_multi can be applied if necessary.
        QGis::WkbType wkbType0 = QGis::flatType( QgsPostgresConn::wkbTypeFromPostgis( type ) );
        QGis::WkbType multiType0 = QGis::multiType( wkbType0 );

        int j;
        for ( j = 0; j < layerProperty.size(); j++ )
        {
          if ( layerProperty.srids[j] != srid.toInt() )
            continue;

          QGis::WkbType wkbType1 = layerProperty.types[j];
          QGis::WkbType multiType1 = QGis::multiType( wkbType1 );
          if ( multiType0 == multiType1 && wkbType0 != wkbType1 )
          {
            layerProperty.types[j] = multiType0;
            break;
          }
        }

        if ( j < layerProperty.size() )
          break;

        layerProperty.types << wkbType0;
        layerProperty.srids << srid.toInt();
      }
    }
  }
}

void QgsPostgresConn::postgisWkbType( QGis::WkbType wkbType, QString &geometryType, int &dim )
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

QString QgsPostgresConn::postgisWkbTypeName( QGis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return geometryType;
}

QString QgsPostgresConn::postgisTypeFilter( QString geomCol, QGis::WkbType geomType, bool isGeography )
{
  geomCol = quotedIdentifier( geomCol );
  if ( isGeography )
    geomCol += "::geometry";

  switch ( geomType )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return QString( "upper(geometrytype(%1)) IN ('POINT','MULTIPOINT','POINTM','MULTIPOINTM')" ).arg( geomCol );
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return QString( "upper(geometrytype(%1)) IN ('LINESTRING','MULTILINESTRING','LINESTRINGM','MULTILINESTRINGM')" ).arg( geomCol );
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return QString( "upper(geometrytype(%1)) IN ('POLYGON','MULTIPOLYGON','POLYGONM','MULTIPOLYGONM','POLYHEDRALSURFACE','TIN')" ).arg( geomCol );
    case QGis::WKBNoGeometry:
      return QString( "geometrytype(%1) IS NULL" ).arg( geomCol );
    case QGis::WKBUnknown:
      Q_ASSERT( !"unknown geometry unexpected" );
      return QString::null;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QString::null;
}

int QgsPostgresConn::postgisWkbTypeDim( QGis::WkbType wkbType )
{
  QString geometryType;
  int dim;

  postgisWkbType( wkbType, geometryType, dim );

  return dim;
}

QGis::WkbType QgsPostgresConn::wkbTypeFromPostgis( QString type )
{
  if ( type == "POINT" )
  {
    return QGis::WKBPoint;
  }
  else if ( type == "POINTM" )
  {
    return QGis::WKBPoint25D;
  }
  else if ( type == "MULTIPOINT" )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( type == "MULTIPOINTM" )
  {
    return QGis::WKBMultiPoint25D;
  }
  else if ( type == "LINESTRING" )
  {
    return QGis::WKBLineString;
  }
  else if ( type == "LINESTRINGM" )
  {
    return QGis::WKBLineString25D;
  }
  else if ( type == "MULTILINESTRING" )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( type == "MULTILINESTRINGM" )
  {
    return QGis::WKBMultiLineString25D;
  }
  else if ( type == "POLYGON" )
  {
    return QGis::WKBPolygon;
  }
  else if ( type == "POLYGONM" )
  {
    return QGis::WKBPolygon25D;
  }
  else if ( type == "MULTIPOLYGON" )
  {
    return QGis::WKBMultiPolygon;
  }
  else if ( type == "MULTIPOLYGONM" || type == "TIN" || type == "POLYHEDRALSURFACE" )
  {
    return QGis::WKBMultiPolygon25D;
  }
  else
  {
    return QGis::WKBUnknown;
  }
}

QGis::WkbType QgsPostgresConn::wkbTypeFromOgcWkbType( unsigned int wkbType )
{
  // polyhedralsurface / TIN / triangle => MultiPolygon
  if ( wkbType % 100 >= 15 )
    wkbType = wkbType / 1000 * 1000 + QGis::WKBMultiPolygon;

  switch ( wkbType / 1000 )
  {
    case 0:
      break;
    case 1: // Z
      wkbType = 0x80000000 + wkbType % 100;
      break;
    case 2: // M => Z
      wkbType = 0x80000000 + wkbType % 100;
      break;
    case 3: // ZM
      wkbType = 0xc0000000 + wkbType % 100;
      break;
  }

  return ( QGis::WkbType ) wkbType;
}

QString QgsPostgresConn::displayStringForWkbType( QGis::WkbType type )
{
  switch ( type )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
      return tr( "Point" );

    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return tr( "Multipoint" );

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      return tr( "Line" );

    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return tr( "Multiline" );

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
      return tr( "Polygon" );

    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return tr( "Multipolygon" );

    case QGis::WKBNoGeometry:
      return tr( "No Geometry" );

    case QGis::WKBUnknown:
      return tr( "Unknown Geometry" );
  }

  Q_ASSERT( !"unexpected wkbType" );
  return QString::null;
}

QString QgsPostgresConn::displayStringForGeomType( QgsPostgresGeometryColumnType type )
{
  switch ( type )
  {
    case sctNone:
      return tr( "None" );
    case sctGeometry:
      return tr( "Geometry" );
    case sctGeography:
      return tr( "Geography" );
    case sctTopoGeometry:
      return tr( "TopoGeometry" );
  }

  Q_ASSERT( !"unexpected geometry column type" );
  return QString::null;
}

QGis::WkbType QgsPostgresConn::wkbTypeFromGeomType( QGis::GeometryType geomType )
{
  switch ( geomType )
  {
    case QGis::Point:
      return QGis::WKBPoint;
    case QGis::Line:
      return QGis::WKBLineString;
    case QGis::Polygon:
      return QGis::WKBPolygon;
    case QGis::NoGeometry:
      return QGis::WKBNoGeometry;
    case QGis::UnknownGeometry:
      return QGis::WKBUnknown;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QGis::WKBUnknown;
}

QStringList QgsPostgresConn::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/PostgreSQL/connections" );
  return settings.childGroups();
}

QString QgsPostgresConn::selectedConnection()
{
  QSettings settings;
  return settings.value( "/PostgreSQL/connections/selected" ).toString();
}

void QgsPostgresConn::setSelectedConnection( QString name )
{
  QSettings settings;
  return settings.setValue( "/PostgreSQL/connections/selected", name );
}

QgsDataSourceURI QgsPostgresConn::connUri( QString theConnName )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  QString key = "/PostgreSQL/connections/" + theConnName;

  QString service = settings.value( key + "/service" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = "5432";
  }
  QString database = settings.value( key + "/database" ).toString();

  bool useEstimatedMetadata = settings.value( key + "/estimatedMetadata", false ).toBool();
  int sslmode = settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt();

  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == "true" )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true" )
  {
    password = settings.value( key + "/password" ).toString();
  }

  // Old save setting
  if ( settings.contains( key + "/save" ) )
  {
    username = settings.value( key + "/username" ).toString();

    if ( settings.value( key + "/save" ).toString() == "true" )
    {
      password = settings.value( key + "/password" ).toString();
    }
  }

  QgsDataSourceURI uri;
  if ( !service.isEmpty() )
  {
    uri.setConnection( service, database, username, password, ( QgsDataSourceURI::SSLmode ) sslmode );
  }
  else
  {
    uri.setConnection( host, port, database, username, password, ( QgsDataSourceURI::SSLmode ) sslmode );
  }
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  return uri;
}

bool QgsPostgresConn::publicSchemaOnly( QString theConnName )
{
  QSettings settings;
  return settings.value( "/PostgreSQL/connections/" + theConnName + "/publicOnly", false ).toBool();
}

bool QgsPostgresConn::geometryColumnsOnly( QString theConnName )
{
  QSettings settings;

  return settings.value( "/PostgreSQL/connections/" + theConnName + "/geometryColumnsOnly", false ).toBool();
}

bool QgsPostgresConn::dontResolveType( QString theConnName )
{
  QSettings settings;

  return settings.value( "/PostgreSQL/connections/" + theConnName + "/dontResolveType", false ).toBool();
}

bool QgsPostgresConn::allowGeometrylessTables( QString theConnName )
{
  QSettings settings;
  return settings.value( "/PostgreSQL/connections/" + theConnName + "/allowGeometrylessTables", false ).toBool();
}

void QgsPostgresConn::deleteConnection( QString theConnName )
{
  QSettings settings;

  QString key = "/PostgreSQL/connections/" + theConnName;
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
  settings.remove( key );
}

bool QgsPostgresConn::cancel()
{
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
