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

#include <QSettings>

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

QgsPostgresConn *QgsPostgresConn::connectDb( QString conninfo, bool readonly )
{
  QMap<QString, QgsPostgresConn *> &connections =
    readonly ? QgsPostgresConn::sConnectionsRO : QgsPostgresConn::sConnectionsRW;

  if ( connections.contains( conninfo ) )
  {
    QgsDebugMsg( QString( "Using cached connection for %1" ).arg( conninfo ) );
    connections[conninfo]->mRef++;
    return connections[conninfo];
  }

  QgsPostgresConn *conn = new QgsPostgresConn( conninfo, readonly );

  if ( conn->mRef == 0 )
  {
    delete conn;
    return 0;
  }

  connections.insert( conninfo, conn );

  return conn;
}

QgsPostgresConn::QgsPostgresConn( QString conninfo, bool readOnly )
    : mRef( 1 )
    , mOpenCursors( 0 )
    , mConnInfo( conninfo )
    , mGotPostgisVersion( false )
    , mReadOnly( readOnly )
{
  QgsDebugMsg( QString( "New PostgreSQL connection for " ) + conninfo );

  mConn = PQconnectdb( conninfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus() != CONNECTION_OK )
  {
    QgsDataSourceURI uri( conninfo );
    QString username = uri.username();
    QString password = uri.password();

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
  }

  if ( PQstatus() != CONNECTION_OK )
  {
    PQfinish();
    QgsMessageLog::logMessage( tr( "Connection to database failed" ), tr( "PostGIS" ) );
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
    PQexecNR( "SET application_name='Quantum GIS'" );
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

  QMap<QString, QgsPostgresConn *>& connections = mReadOnly ? sConnectionsRO : sConnectionsRW;

  QString key = connections.key( this, QString::null );

  Q_ASSERT( !key.isNull() );
  connections.remove( key );

  deleteLater();
}

QStringList QgsPostgresConn::pkCandidates( QString schemaName, QString viewName )
{
  QStringList cols;

  QString sql = QString( "SELECT attname FROM pg_attribute JOIN pg_type ON atttypid=pg_type.oid WHERE pg_type.typname IN ('int2','int4','int8','oid','serial','serial8') AND attrelid=regclass('%1.%2')" )
                .arg( quotedIdentifier( schemaName ) )
                .arg( quotedIdentifier( viewName ) );
  QgsDebugMsg( sql );
  QgsPostgresResult colRes = PQexec( sql );

  if ( colRes.PQresultStatus() == PGRES_TUPLES_OK )
  {
    for ( int i = 0; i < colRes.PQntuples(); i++ )
    {
      QgsDebugMsg( colRes.PQgetvalue( i, 0 ) );
      cols << colRes.PQgetvalue( i, 0 );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "SQL:%1\nresult:%2\nerror:%3\n" ).arg( sql ).arg( colRes.PQresultStatus() ).arg( colRes.PQresultErrorMessage() ), tr( "PostGIS" ) );
  }

  return cols;
}

bool QgsPostgresConn::getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables )
{
  int nColumns = 0;
  int nGTables = 0;
  QgsPostgresResult result;
  QgsPostgresLayerProperty layerProperty;

  QgsDebugMsg( "Entering." );

  mLayersSupported.clear();

  for ( int i = 0; i < 2; i++ )
  {
    QString gtableName, columnName;

    if ( i == 0 )
    {
      gtableName = "geometry_columns";
      columnName = "f_geometry_column";
    }
    else if ( i == 1 )
    {
      gtableName = "geography_columns";
      columnName = "f_geography_column";
    }

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.
    QString sql = QString( "SELECT "
                           "f_table_name,"
                           "f_table_schema,"
                           "%2,"
                           "upper(type),"
                           "srid,"
                           "pg_class.relkind"
                           " FROM "
                           "%1,pg_class,pg_namespace"
                           " WHERE relname=f_table_name"
                           " AND f_table_schema=nspname"
                           " AND pg_namespace.oid=pg_class.relnamespace"
                           " AND has_schema_privilege(pg_namespace.nspname,'usage')"
                           " AND has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select')" // user has select privilege
                         ).arg( gtableName ).arg( columnName );

    if ( searchPublicOnly )
      sql += " AND f_table_schema='public'";

    sql += QString( " ORDER BY f_table_schema,f_table_name,%1" ).arg( columnName );

    QgsDebugMsg( "getting table info: " + sql );
    result = PQexec( sql, i == 0 );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      PQexecNR( "COMMIT" );

      if ( i == 0 )
        return false;

      continue;
    }
    else
    {
      nGTables++;

      for ( int idx = 0; idx < result.PQntuples(); idx++ )
      {
        QString tableName = result.PQgetvalue( idx, 0 );
        QString schemaName = result.PQgetvalue( idx, 1 );
        QString column = result.PQgetvalue( idx, 2 );
        QString type = result.PQgetvalue( idx, 3 );
        QString srid = result.PQgetvalue( idx, 4 );
        QString relkind = result.PQgetvalue( idx, 5 );

        QgsDebugMsg( QString( "%1 : %2.%3.%4: %5 %6 %7" )
                     .arg( gtableName )
                     .arg( schemaName ).arg( tableName ).arg( column )
                     .arg( type )
                     .arg( srid )
                     .arg( relkind ) );

        layerProperty.type = type;
        layerProperty.schemaName = schemaName;
        layerProperty.tableName = tableName;
        layerProperty.geometryColName = column;
        layerProperty.pkCols = relkind == "v" ? pkCandidates( schemaName, tableName ) : QStringList();
        layerProperty.srid = srid;
        layerProperty.sql = "";
        layerProperty.isGeography = i == 1;

        mLayersSupported << layerProperty;
        nColumns++;
      }
    }
  }

  if ( nColumns == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "PostGIS" ) );
  }

  //search for geometry columns in tables that are not in the geometry_columns metatable
  if ( !searchGeometryColumnsOnly )
  {
    // Now have a look for geometry columns that aren't in the geometry_columns table.
    QString sql = "SELECT "
                  "pg_class.relname"
                  ",pg_namespace.nspname"
                  ",pg_attribute.attname"
                  ",pg_class.relkind"
                  " FROM "
                  "pg_attribute,pg_class,pg_namespace"
                  " WHERE pg_namespace.oid=pg_class.relnamespace"
                  " AND pg_attribute.attrelid = pg_class.oid"
                  " AND ("
                  " EXISTS (SELECT * FROM pg_type WHERE pg_type.oid=pg_attribute.atttypid AND pg_type.typname IN ('geometry','geography','topogeometry'))"
                  " OR pg_attribute.atttypid IN (SELECT oid FROM pg_type a WHERE EXISTS (SELECT * FROM pg_type b WHERE a.typbasetype=b.oid AND b.typname IN ('geometry','geography','topogeometry')))"
                  ")"
                  " AND has_schema_privilege( pg_namespace.nspname, 'usage' )"
                  " AND has_table_privilege( '\"' || pg_namespace.nspname || '\".\"' || pg_class.relname || '\"', 'select' )";

    // user has select privilege
    if ( searchPublicOnly )
      sql += " AND pg_namespace.nspname='public'";

    if ( nColumns > 0 )
    {
      // TODO: handle this for the topogeometry case
      sql += " AND (pg_namespace.nspname,pg_class.relname) NOT IN (SELECT f_table_schema,f_table_name FROM geometry_columns)";

      if ( nGTables > 1 )
      {
        // TODO: handle this for the topogeometry case
        // TODO: handle this for the geometry case ?
        sql += " AND (pg_namespace.nspname,pg_class.relname) NOT IN (SELECT f_table_schema,f_table_name FROM geography_columns)";
      }
    }

    sql += " AND pg_class.relkind IN ('v','r')"; // only from views and relations (tables)

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

      QString table   = result.PQgetvalue( i, 0 ); // relname
      QString schema  = result.PQgetvalue( i, 1 ); // nspname
      QString column  = result.PQgetvalue( i, 2 ); // attname
      QString relkind = result.PQgetvalue( i, 3 ); // relation kind

      QgsDebugMsg( QString( "%1.%2.%3: %4" ).arg( schema ).arg( table ).arg( column ).arg( relkind ) );

      layerProperty.type = QString::null;
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = column;
      layerProperty.pkCols = relkind == "v" ? pkCandidates( schema, table ) : QStringList();
      layerProperty.sql = "";
      layerProperty.isGeography = false; // TODO might be geography after all

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
                  " AND pg_class.relkind IN ('v','r')";

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
      return -1;
    }

    for ( int i = 0; i < result.PQntuples(); i++ )
    {
      QString table   = result.PQgetvalue( i, 0 ); // relname
      QString schema  = result.PQgetvalue( i, 1 ); // nspname
      QString relkind = result.PQgetvalue( i, 2 ); // relation kind

      QgsDebugMsg( QString( "%1.%2: %3" ).arg( schema ).arg( table ).arg( relkind ) );

      layerProperty.type = QString::null;
      layerProperty.schemaName = schema;
      layerProperty.tableName = table;
      layerProperty.geometryColName = QString::null;
      layerProperty.pkCols = relkind == "v" ? pkCandidates( schema, table ) : QStringList();
      layerProperty.srid = "";
      layerProperty.sql = "";
      layerProperty.isGeography = false;

      mLayersSupported << layerProperty;
      nColumns++;
    }
  }

  if ( nColumns == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but no accessible tables were found.  Please verify that you have SELECT privilege on a table carrying PostGIS geometry." ), tr( "PostGIS" ) );
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
    QgsPostgresResult result = PQexec( "SELECT count(c.oid) FROM pg_class AS c JOIN pg_namespace AS n ON c.relnamespace=n.oid WHERE n.nspname='topology' AND c.relname='topology'" );
    if ( result.PQntuples() >= 1 )
    {
      mTopologyAvailable = true;
    }
  }

  mGotPostgisVersion = true;

  return mPostgisVersionInfo;
}

QString QgsPostgresConn::quotedIdentifier( QString ident, bool isGeography )
{
  ident.replace( '"', "\"\"" );
  ident = ident.prepend( "\"" ).append( "\"" );
  if ( isGeography )
    ident += "::geometry";
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
      v.replace( "\\\"", "\\\\\"" );
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
    QgsMessageLog::logMessage( tr( "Query failed: %1\nError: no result buffer" ).arg( query ) );
  }
  else
  {
    QgsDebugMsg( tr( "Not logged query failed: %1\nError: no result buffer" ).arg( query ) );
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
  qint64 oid;
  char *p = PQgetvalue( queryResult.result(), row, col );
  size_t s = PQgetlength( queryResult.result(), row, col );

#ifdef QGISDEBUG
  QString buf = "";
  for ( size_t i = 0; i < s; i++ )
  {
    buf += QString( "%1 " ).arg( *( unsigned char * )( p + i ), 0, 16, QLatin1Char( ' ' ) );
  }

  QgsDebugMsgLevel( QString( "int in hex:%1" ).arg( buf ), 4 );
#endif

  switch ( s )
  {
    case 2:
      oid = *( qint16 * )p;
      if ( mSwapEndian )
        oid = ntohs( oid );
      break;

    case 6:
    {
      qint64 block  = *( qint32 * ) p;
      qint64 offset = *( qint16 * )( p + sizeof( qint32 ) );

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
      qint32 oid0 = *( qint32 * ) p;
      qint32 oid1 = *( qint32 * )( p + sizeof( qint32 ) );

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
      oid = *( qint32 * )p;
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

  // it is possible that the where clause restricts the feature type or srid
  if ( useEstimatedMetadata )
  {
    table = QString( "(SELECT %1 FROM %2.%3 WHERE %1 IS NOT NULL%4 LIMIT %5) AS t" )
            .arg( quotedIdentifier( layerProperty.geometryColName ) )
            .arg( quotedIdentifier( layerProperty.schemaName ) )
            .arg( quotedIdentifier( layerProperty.tableName ) )
            .arg( layerProperty.sql.isEmpty() ? "" : QString( " AND (%1)" ).arg( layerProperty.sql ) )
            .arg( sGeomTypeSelectLimit );
  }
  else if ( !layerProperty.schemaName.isEmpty() )
  {
    table = QString( "%1.%2%3" )
            .arg( quotedIdentifier( layerProperty.schemaName ) )
            .arg( quotedIdentifier( layerProperty.tableName ) )
            .arg( layerProperty.sql.isEmpty() ? "" : QString( " WHERE %1" ).arg( layerProperty.sql ) );
  }
  else
  {
    table = QString( "%1%2" )
            .arg( layerProperty.tableName )
            .arg( layerProperty.sql.isEmpty() ? "" : QString( " WHERE %1" ).arg( layerProperty.sql ) );
  }

  QString query = QString( "SELECT DISTINCT"
                           " CASE"
                           " WHEN %1 THEN 'POINT'"
                           " WHEN %2 THEN 'LINESTRING'"
                           " WHEN %3 THEN 'POLYGON'"
                           " END,"
                           " %4(%5)"
                           " FROM %6" )
                  .arg( postgisTypeFilter( layerProperty.geometryColName, QGis::Point, layerProperty.isGeography ) )
                  .arg( postgisTypeFilter( layerProperty.geometryColName, QGis::Line, layerProperty.isGeography ) )
                  .arg( postgisTypeFilter( layerProperty.geometryColName, QGis::Polygon, layerProperty.isGeography ) )
                  .arg( majorVersion() < 2 ? "srid" : "st_srid" )
                  .arg( quotedIdentifier( layerProperty.geometryColName, layerProperty.isGeography ) )
                  .arg( table );

  QgsDebugMsg( "Retrieving geometry types: " + query );

  QgsPostgresResult gresult = PQexec( query );

  QString type;
  QString srid;
  if ( gresult.PQresultStatus() == PGRES_TUPLES_OK )
  {
    QStringList types;
    QStringList srids;

    for ( int i = 0; i < gresult.PQntuples(); i++ )
    {
      QString type = gresult.PQgetvalue( i, 0 );
      QString srid = gresult.PQgetvalue( i, 1 );
      if ( type.isEmpty() )
        continue;

      types << type;
      srids << srid;
    }

    type = types.join( "," );
    srid = srids.join( "," );
  }

  QgsDebugMsg( QString( "type:%1 srid:%2" ).arg( type ).arg( srid ) );
  layerProperty.type = type;
  layerProperty.srid = srid;
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

QString QgsPostgresConn::postgisTypeFilter( QString geomCol, QGis::GeometryType geomType, bool isGeography )
{
  geomCol = quotedIdentifier( geomCol, isGeography );

  switch ( geomType )
  {
    case QGis::Point:
      return QString( "upper(geometrytype(%1)) IN ('POINT','MULTIPOINT','POINTM','MULTIPOINTM')" ).arg( geomCol );
    case QGis::Line:
      return QString( "upper(geometrytype(%1)) IN ('LINESTRING','MULTILINESTRING','LINESTRINGM','MULTILINESTRINGM')" ).arg( geomCol );
    case QGis::Polygon:
      return QString( "upper(geometrytype(%1)) IN ('POLYGON','MULTIPOLYGON','POLYGONM','MULTIPOLYGONM')" ).arg( geomCol );
    case QGis::NoGeometry:
      return QString( "geometrytype(%1) IS NULL" ).arg( geomCol );
    case QGis::UnknownGeometry:
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
  if ( type == "POINT" || type == "POINTM" )
  {
    return QGis::WKBPoint;
  }
  else if ( type == "MULTIPOINT" || type == "MULTIPOINTM" )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( type == "LINESTRING" || type == "LINESTRINGM" )
  {
    return QGis::WKBLineString;
  }
  else if ( type == "MULTILINESTRING" || type == "MULTILINESTRINGM" )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( type == "POLYGON" || type == "POLYGONM" )
  {
    return QGis::WKBPolygon;
  }
  else if ( type == "MULTIPOLYGON" || type == "MULTIPOLYGONM" )
  {
    return QGis::WKBMultiPolygon;
  }
  else
  {
    return QGis::WKBUnknown;
  }
}

QString QgsPostgresConn::displayStringForGeomType( QGis::GeometryType type )
{
  switch ( type )
  {
    case QGis::Point:
      return tr( "Point" );
    case QGis::Line:
      return tr( "Line" );
    case QGis::Polygon:
      return tr( "Polygon" );
    case QGis::NoGeometry:
      return tr( "No Geometry" );
    case QGis::UnknownGeometry:
      return tr( "Unknown" );
  }

  Q_ASSERT( !"unexpected geometryType" );
  return QString::null;
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

QGis::GeometryType QgsPostgresConn::geomTypeFromPostgis( QString dbType )
{
  dbType = dbType.toUpper();

  if ( dbType == "POINT" || dbType == "MULTIPOINT" || dbType == "POINTM" || dbType == "MULTIPOINTM" )
  {
    return QGis::Point;
  }
  else if ( dbType == "LINESTRING" || dbType == "MULTILINESTRING" || dbType == "LINESTRINGM" || dbType == "MULTILINESTRINGM" )
  {
    return QGis::Line;
  }
  else if ( dbType == "POLYGON" || dbType == "MULTIPOLYGON" || dbType == "POLYGONM" || dbType == "MULTIPOLYGONM" )
  {
    return QGis::Polygon;
  }
  else
  {
    return QGis::UnknownGeometry;
  }
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

  return settings.value( "/PostgreSQL/connections/" + theConnName + "/geometrycolumnsOnly", false ).toBool();
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
