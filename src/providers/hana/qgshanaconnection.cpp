/***************************************************************************
   qgshanaconnection.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaconnectionstringbuilder.h"
#include "qgshanadriver.h"
#include "qgshanaexception.h"
#include "qgshanaresultset.h"
#include "qgshanasettings.h"
#include "qgshanatablemodel.h"
#include "qgshanautils.h"
#include "qgsmessagelog.h"
#include "qgscredentials.h"
#include "qgssettings.h"
#include "qexception.h"

#include "odbc/Connection.h"
#include "odbc/DatabaseMetaData.h"
#include "odbc/Environment.h"
#include "odbc/Exception.h"
#include "odbc/PreparedStatement.h"
#include "odbc/ResultSet.h"
#include "odbc/Statement.h"

using namespace odbc;
using namespace std;

static const uint8_t CREDENTIALS_INPUT_MAX_ATTEMPTS = 5;
static const int GEOM_TYPE_SELECT_LIMIT = 10;

bool QgsHanaConnection::sConnectionAttemptCanceled = false;

QgsHanaConnection::QgsHanaConnection( const QgsDataSourceUri &uri )
  : mUri( uri )
  , mDatabaseVersion( "" )
  , mUserName( "" )
{
  setConnectionAttemptCanceled( false );
  ConnectionRef conn = QgsHanaDriver::instance()->createConnection();
  conn->setAutoCommit( false );
  QString errorMessage = "";

  if ( !connect( conn, uri, errorMessage ) )
  {
    QString conninfo = uri.uri( false );
    QString username = uri.username();
    QString password = uri.password();
    QgsDataSourceUri tmpUri( uri );

    QgsCredentials::instance()->lock();

    int i = 0;
    while ( i < CREDENTIALS_INPUT_MAX_ATTEMPTS )
    {
      ++i;
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, errorMessage );
      if ( !ok )
      {
        setConnectionAttemptCanceled( true );
        break;
      }

      if ( !username.isEmpty() )
        tmpUri.setUsername( username );
      if ( !password.isEmpty() )
        tmpUri.setPassword( password );

      if ( connect( conn, tmpUri, errorMessage ) )
        break;
    }

    QgsCredentials::instance()->put( conninfo, username, password );
    QgsCredentials::instance()->unlock();
  }

  if ( conn->connected() )
    mConnection = conn;
  else
    throw QgsHanaException( errorMessage.toStdString().c_str() );
}

QgsHanaConnection::~QgsHanaConnection()
{
  disconnect();
}

QgsHanaConnection *QgsHanaConnection::createConnection( const QgsDataSourceUri &uri )
{
  QString errorMessage;
  QgsHanaConnection *conn = createConnection( uri, &errorMessage );

  if ( !errorMessage.isEmpty() )
  {
    QString logMessage = QObject::tr( "Connection to database failed" ) + '\n' + errorMessage;
    QgsDebugMsg( logMessage );
    QgsMessageLog::logMessage( logMessage, tr( "HANA" ) );
  }

  return conn;
}

QgsHanaConnection *QgsHanaConnection::createConnection( const QgsDataSourceUri &uri, QString *errorMessage )
{
  try
  {
    QgsHanaConnection *conn = new QgsHanaConnection( uri );

    if ( conn->getNativeRef().isNull() )
    {
      delete conn;
      return nullptr;
    }

    return conn;
  }
  catch ( const QgsHanaException &ex )
  {
    if ( errorMessage != nullptr )
      *errorMessage = ex.what();
  }

  return nullptr;
}

QStringList QgsHanaConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "HANA/connections" ) );
  return settings.childGroups();
}

bool QgsHanaConnection::connect(
  ConnectionRef &conn,
  const QgsDataSourceUri &uri,
  QString &errorMessage )
{
  try
  {
    QgsHanaConnectionStringBuilder sb( uri );
    conn->connect( sb.toString().toStdString().c_str() );
    errorMessage = "";
  }
  catch ( const Exception &ex )
  {
    errorMessage = QgsHanaUtils::formatErrorMessage( ex.what() );
    QgsDebugMsg( errorMessage );
  }

  return conn->connected();
}

QString QgsHanaConnection::connInfo()
{
  return QgsHanaUtils::connectionInfo( mUri );
}

void QgsHanaConnection::disconnect()
{
  if ( mConnection.isNull() || !mConnection->connected() )
    return;

  try
  {
    // The rollback needs to be called here because the driver throws an exception
    // if AutoCommit is set to false.
    mConnection->rollback();

    mConnection->disconnect();
  }
  catch ( const Exception &ex )
  {
    QgsMessageLog::logMessage( QgsHanaUtils::formatErrorMessage( ex.what() ), tr( "HANA" ) );
  }
}

bool QgsHanaConnection::dropTable( const QString &schemaName, const QString &tableName, QString *errMessage )
{
  if ( mConnection.isNull() )
    return false;

  QString sql = QStringLiteral( "DROP TABLE %1.%2" )
                .arg( QgsHanaUtils::quotedIdentifier( schemaName ), QgsHanaUtils::quotedIdentifier( tableName ) );

  try
  {
    StatementRef stmt = mConnection->createStatement();
    stmt->execute( QgsHanaUtils::toQueryString( sql ) );
    mConnection->commit();
    return true;
  }
  catch ( const Exception &ex )
  {
    if ( errMessage )
      *errMessage = QgsHanaUtils::formatErrorMessage( ex.what() );
  }

  return false;
}

const QString &QgsHanaConnection::getDatabaseVersion()
{
  if ( mDatabaseVersion.isEmpty() && !mConnection.isNull() )
  {
    try
    {
      DatabaseMetaDataRef dbmd = mConnection->getDatabaseMetaData();
      mDatabaseVersion = dbmd->getDBMSVersion().c_str();
    }
    catch ( const Exception &ex )
    {
      throw QgsHanaException( ex.what() );
    }
  }

  return mDatabaseVersion;
}

const QString &QgsHanaConnection::getUserName()
{
  if ( mUserName.isEmpty() && !mConnection.isNull() )
  {
    try
    {
      StatementRef stmt = mConnection->createStatement();
      ResultSetRef rs = stmt->executeQuery( "SELECT CURRENT_USER FROM DUMMY" );
      while ( rs->next() )
      {
        mUserName = QgsHanaUtils::toQString( rs->getNString( 1 ) );
      }
      rs->close();
    }
    catch ( const Exception &ex )
    {
      throw QgsHanaException( ex.what() );
    }
  }

  return mUserName;
}

QgsCoordinateReferenceSystem QgsHanaConnection::getCrs( int srid )
{
  QgsCoordinateReferenceSystem crs;

  const char *sql = "SELECT ORGANIZATION, ORGANIZATION_COORDSYS_ID, DEFINITION, TRANSFORM_DEFINITION FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = ?";
  PreparedStatementRef stmt = mConnection->prepareStatement( sql );
  stmt->setInt( 1, Int( srid ) );
  ResultSetRef rsSrs = stmt->executeQuery();

  if ( rsSrs->next() )
  {
    auto organization = rsSrs->getNString( 1 );
    if ( !organization.isNull() )
    {
      QString srid = QStringLiteral( "%1:%2" ).arg( QgsHanaUtils::toQString( organization ).toLower(), QString::number( *rsSrs->getInt( 2 ) ) );
      crs.createFromString( srid );
    }

    if ( !crs.isValid() )
    {
      auto wkt = rsSrs->getNString( 3 );
      if ( !wkt.isNull() )
        crs = QgsCoordinateReferenceSystem::fromWkt( QgsHanaUtils::toQString( wkt ) );

      if ( !crs.isValid() )
      {
        auto proj = rsSrs->getNString( 4 );
        if ( !proj.isNull() )
          crs = QgsCoordinateReferenceSystem::fromProj( QgsHanaUtils::toQString( proj ) );
      }
    }
  }
  rsSrs->close();

  return crs;
}

QVector<QgsHanaLayerProperty> QgsHanaConnection::getLayers(
  const QString &schemaName,
  bool allowGeometrylessTables,
  bool userTablesOnly )
{
  const QString schema = mUri.schema().isEmpty() ? schemaName : mUri.schema();
  const QString sqlSchemaFilter = QStringLiteral(
                                    "SELECT DISTINCT(SCHEMA_NAME) FROM SYS.EFFECTIVE_PRIVILEGES WHERE "
                                    "SCHEMA_NAME LIKE ? AND "
                                    "SCHEMA_NAME NOT LIKE_REGEXPR 'SYS|_SYS.*|UIS|SAP_XS|SAP_REST|HANA_XS' AND "
                                    "PRIVILEGE IN ('SELECT', 'CREATE ANY') AND "
                                    "USER_NAME = CURRENT_USER AND IS_VALID = 'TRUE'" );

  const QString sqlOwnerFilter = userTablesOnly ? QStringLiteral( "OWNER_NAME = CURRENT_USER" ) : QStringLiteral( "OWNER_NAME IS NOT NULL" );

  const QString sqlDataTypeFilter = !allowGeometrylessTables ? QStringLiteral( "DATA_TYPE_NAME IN ('ST_GEOMETRY','ST_POINT')" ) : "DATA_TYPE_NAME IS NOT NULL";

  const QString sqlTables = QStringLiteral(
                              "SELECT SCHEMA_NAME, TABLE_NAME, COLUMN_NAME, DATA_TYPE_NAME, TABLE_COMMENTS FROM "
                              "(SELECT * FROM SYS.TABLE_COLUMNS WHERE "
                              "TABLE_OID IN (SELECT OBJECT_OID FROM OWNERSHIP WHERE OBJECT_TYPE = 'TABLE' AND %1) AND "
                              "SCHEMA_NAME IN (%2) AND %3) "
                              "INNER JOIN "
                              "(SELECT TABLE_OID AS TABLE_OID_2, COMMENTS AS TABLE_COMMENTS FROM SYS.TABLES WHERE IS_USER_DEFINED_TYPE = 'FALSE') "
                              "ON TABLE_OID = TABLE_OID_2" );

  const QString sqlViews = QStringLiteral(
                             "SELECT SCHEMA_NAME, VIEW_NAME, COLUMN_NAME, DATA_TYPE_NAME, VIEW_COMMENTS FROM "
                             "(SELECT * FROM SYS.VIEW_COLUMNS WHERE "
                             "VIEW_OID IN (SELECT OBJECT_OID FROM OWNERSHIP WHERE OBJECT_TYPE = 'VIEW' AND %1) AND "
                             "SCHEMA_NAME IN (%2) AND %3) "
                             "INNER JOIN "
                             "(SELECT VIEW_OID AS VIEW_OID_2, COMMENTS AS VIEW_COMMENTS FROM SYS.VIEWS) "
                             "ON VIEW_OID = VIEW_OID_2" );


  QMultiHash<QPair<QString, QString>, QgsHanaLayerProperty> layers;

  auto addLayers = [&]( const QString & sql, bool isView )
  {
    PreparedStatementRef stmt = mConnection->prepareStatement( QgsHanaUtils::toQueryString( sql ) );
    stmt->setNString( 1, NString( schema.isEmpty() ? u"%" : schema.toStdU16String() ) );
    QgsHanaResultSetRef rsLayers = QgsHanaResultSet::create( stmt );

    while ( rsLayers->next() )
    {
      QgsHanaLayerProperty layer;
      layer.schemaName = rsLayers->getString( 1 );
      layer.tableName = rsLayers->getString( 2 );
      QString geomColumnType = rsLayers->getString( 4 );
      bool isGeometryColumn = ( geomColumnType == "ST_GEOMETRY" || geomColumnType == "ST_POINT" );
      layer.geometryColName = isGeometryColumn ? rsLayers->getString( 3 ) : "";
      layer.tableComment = rsLayers->getString( 5 );
      layer.isView = isView;

      QPair<QString, QString> layerKey( layer.schemaName, layer.tableName );
      if ( allowGeometrylessTables )
      {
        int layersCount = layers.count( layerKey );
        if ( !isGeometryColumn && layersCount >= 1 )
          continue;
        if ( layersCount == 1 )
        {
          QgsHanaLayerProperty firstLayer = layers.values( layerKey )[0];
          if ( firstLayer.geometryColName.isEmpty() )
          {
            if ( isGeometryColumn )
              layers.remove( layerKey );
            else
              continue;
          }
        }
      }

      layers.insert( layerKey, layer );
    }
    rsLayers->close();
  };

  try
  {
    QString sql = sqlTables.arg( sqlOwnerFilter, sqlSchemaFilter, sqlDataTypeFilter );
    addLayers( sql, false );

    sql = sqlViews.arg( sqlOwnerFilter, sqlSchemaFilter, sqlDataTypeFilter );
    addLayers( sql, true );
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  QVector<QgsHanaLayerProperty> list;
  for ( QPair<QString, QString> key : layers.uniqueKeys() )
  {
    QList<QgsHanaLayerProperty> values = layers.values( key );
    if ( values.size() == 1 )
      values[0].isUnique = values.size() == 1;

    for ( auto lp : values )
      list << lp;
  }

  return list;
}

void QgsHanaConnection::readLayerInfo( QgsHanaLayerProperty &layerProperty )
{
  layerProperty.srid = getLayerSRID( layerProperty );
  layerProperty.type = getLayerGeometryType( layerProperty );
  layerProperty.pkCols = getLayerPrimaryeKeys( layerProperty );
}

QVector<QgsHanaSchemaProperty> QgsHanaConnection::getSchemas( const QString &ownerName )
{
  QString sql = QStringLiteral( "SELECT SCHEMA_NAME, SCHEMA_OWNER FROM SYS.SCHEMAS WHERE "
                                "HAS_PRIVILEGES = 'TRUE' AND %1 AND "
                                "SCHEMA_NAME NOT LIKE_REGEXPR 'SYS|_SYS.*|UIS|SAP_XS|SAP_REST|HANA_XS|XSSQLCC_'" )
                .arg( !ownerName.isEmpty() ? QStringLiteral( "SCHEMA_OWNER = ?" ) : QStringLiteral( "SCHEMA_OWNER IS NOT NULL" ) );

  QVector<QgsHanaSchemaProperty> list;

  try
  {
    PreparedStatementRef stmt = mConnection->prepareStatement( QgsHanaUtils::toQueryString( sql ) );
    if ( !ownerName.isEmpty() )
      stmt->setNString( 1, NString( ownerName.toStdU16String() ) );
    QgsHanaResultSetRef rsSchemas = QgsHanaResultSet::create( stmt );
    while ( rsSchemas->next() )
    {
      QgsHanaSchemaProperty schema;
      schema.name = rsSchemas->getString( 1 );
      schema.owner = rsSchemas->getString( 2 );
      list << schema;
    }
    rsSchemas->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  return list;
}

int QgsHanaConnection::getLayerSRID( const QgsHanaLayerProperty &layerProperty )
{
  if ( layerProperty.geometryColName.isEmpty() )
    return -1;

  int srid = -1;

  try
  {
    PreparedStatementRef stmt;
    if ( !layerProperty.isView )
    {
      const char *sql = "SELECT SRS_ID FROM SYS.ST_GEOMETRY_COLUMNS "
                        "WHERE SCHEMA_NAME = ? AND TABLE_NAME = ? AND COLUMN_NAME = ?";
      stmt = mConnection->prepareStatement( sql );
      stmt->setNString( 1, NString( layerProperty.schemaName.toStdU16String() ) );
      stmt->setNString( 2, NString( layerProperty.tableName.toStdU16String() ) );
      stmt->setNString( 3, NString( layerProperty.geometryColName.toStdU16String() ) );
    }
    else
    {
      QString sql = QStringLiteral( "SELECT %1.ST_SRID() FROM %2.%3 WHERE %1 IS NOT NULL LIMIT %4" )
                    .arg( QgsHanaUtils::quotedIdentifier( layerProperty.geometryColName ),
                          QgsHanaUtils::quotedIdentifier( layerProperty.schemaName ),
                          QgsHanaUtils::quotedIdentifier( layerProperty.tableName ),
                          QString::number( GEOM_TYPE_SELECT_LIMIT ) );
      stmt = mConnection->prepareStatement( QgsHanaUtils::toQueryString( sql ) );
    }

    int prevSrid = -1;
    ResultSetRef rsSrid = stmt->executeQuery( );
    while ( rsSrid->next() )
    {
      srid = *rsSrid->getInt( 1 );
      if ( prevSrid != -1 && srid != prevSrid )
      {
        srid = -1;
        break;
      }
      prevSrid = srid;
    }

    rsSrid->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  return srid;
}

QStringList QgsHanaConnection::getLayerPrimaryeKeys( const QgsHanaLayerProperty &layerProperty )
{
  QStringList ret;

  try
  {
    DatabaseMetaDataRef dbmd = mConnection->getDatabaseMetaData();
    ResultSetRef rsPrimaryKeys = dbmd->getPrimaryKeys( nullptr,
                                 layerProperty.schemaName.toStdString().c_str(),
                                 layerProperty.tableName.toStdString().c_str() );

    while ( rsPrimaryKeys->next() )
    {
      auto keyName = rsPrimaryKeys->getNString( 4 );
      QString clmName = QgsHanaUtils::toQString( keyName );
      ResultSetRef rsColumns = dbmd->getColumns( nullptr,
                               layerProperty.schemaName.toStdString().c_str(),
                               layerProperty.tableName.toStdString().c_str(),
                               clmName.toStdString().c_str() );

      if ( rsColumns->next() )
      {
        Short dataType = rsColumns->getShort( 5 );
        short dt = *dataType;
        if ( dt == SQLDataTypes::TinyInt || dt == SQLDataTypes::SmallInt ||
             dt == SQLDataTypes::Integer || dt == SQLDataTypes::BigInt )
          ret << clmName;
      }
      rsColumns->close();
    }
    rsPrimaryKeys->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  return ret;
}

QgsWkbTypes::Type QgsHanaConnection::getLayerGeometryType( const QgsHanaLayerProperty &layerProperty )
{
  if ( layerProperty.geometryColName.isEmpty() )
    return QgsWkbTypes::NoGeometry;

  QgsWkbTypes::Type ret;

  QString sql = QStringLiteral( "SELECT upper(%1.ST_GeometryType()), %1.ST_Is3D(), %1.ST_IsMeasured() FROM %2.%3 "
                                "WHERE %1 IS NOT NULL LIMIT %4" ).arg(
                  QgsHanaUtils::quotedIdentifier( layerProperty.geometryColName ),
                  QgsHanaUtils::quotedIdentifier( layerProperty.schemaName ),
                  QgsHanaUtils::quotedIdentifier( layerProperty.tableName ),
                  QString::number( GEOM_TYPE_SELECT_LIMIT ) );

  try
  {
    StatementRef stmt = mConnection->createStatement();
    ResultSetRef rsGeomInfo = stmt->executeQuery( QgsHanaUtils::toQueryString( sql ) );
    QgsWkbTypes::Type geomType = QgsWkbTypes::Unknown, prevGeomType = QgsWkbTypes::Unknown;
    while ( rsGeomInfo->next() )
    {
      geomType = QgsWkbTypes::singleType( QgsHanaUtils::toWkbType(
                                            rsGeomInfo->getString( 1 ), rsGeomInfo->getInt( 2 ), rsGeomInfo->getInt( 3 ) ) );
      if ( prevGeomType != QgsWkbTypes::Unknown && geomType != prevGeomType )
      {
        geomType = QgsWkbTypes::Unknown;
        break;
      }
      prevGeomType = geomType;
    }
    ret = geomType;
    rsGeomInfo->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }
  return ret;
}

QString QgsHanaConnection::getColumnDataType( const QString &schemaName, const QString &tableName, const QString &columnName )
{
  const char *sql = "SELECT DATA_TYPE_NAME FROM SYS.TABLE_COLUMNS WHERE SCHEMA_NAME = ? AND "
                    "TABLE_NAME = ? AND COLUMN_NAME = ?";

  QString ret;
  try
  {
    PreparedStatementRef stmt = mConnection->prepareStatement( sql );
    stmt->setNString( 1, NString( schemaName.toStdU16String() ) );
    stmt->setNString( 2, NString( tableName.toStdU16String() ) );
    stmt->setNString( 3, NString( columnName.toStdU16String() ) );
    ResultSetRef rsDataType = stmt->executeQuery();
    while ( rsDataType->next() )
    {
      ret = QgsHanaUtils::toQString( rsDataType->getString( 1 ) );
    }
    rsDataType->close();
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  return ret;
}

QgsHanaConnectionRef::QgsHanaConnectionRef( const QgsDataSourceUri &uri )
{
  mConnection = std::unique_ptr<QgsHanaConnection>(
                  QgsHanaConnectionPool::instance()->acquireConnection( QgsHanaUtils::connectionInfo( uri ) ) );
}

QgsHanaConnectionRef::QgsHanaConnectionRef( const QString &name )
{
  QgsHanaSettings settings( name, true );
  mConnection = std::unique_ptr<QgsHanaConnection>(
                  QgsHanaConnectionPool::instance()->acquireConnection( QgsHanaUtils::connectionInfo( settings.toDataSourceUri() ) ) );
}

QgsHanaConnectionRef::~QgsHanaConnectionRef()
{
  if ( mConnection && QgsHanaConnectionPool::hasInstance() )
    QgsHanaConnectionPool::instance()->releaseConnection( mConnection.release() );
}
