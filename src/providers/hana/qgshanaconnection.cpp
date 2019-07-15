/***************************************************************************
   qgshanaconnection.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

namespace
{

  static const uint8_t CREDENTIALS_INPUT_MAX_ATTEMPTS = 5;

  void addNewLayer( QVector<QgsHanaLayerProperty> &list,
                    const QgsHanaLayerProperty &layerProperty, bool checkDuplicates = false )
  {
    if ( checkDuplicates )
    {
      auto res = std::find_if( list.begin(), list.end(),
                               [&]( const QgsHanaLayerProperty & lp )
      { return ( lp.schemaName == layerProperty.schemaName && lp.tableName == layerProperty.tableName ); } );

      if ( res != list.end() )
        return;
    }

    list << layerProperty;
  }

}

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
}

QgsHanaConnection::~QgsHanaConnection()
{
  disconnect();
}

QgsHanaConnection *QgsHanaConnection::createConnection( const QgsDataSourceUri &uri )
{
  QgsHanaConnection *conn = nullptr;

  try
  {
    conn = new QgsHanaConnection( uri );

    if ( conn->getNativeRef().isNull() )
    {
      delete conn;
      return nullptr;
    }
  }
  catch ( const exception &ex )
  {
    QgsMessageLog::logMessage(
      QObject::tr( "Connection to database failed" ) + '\n' + QgsHanaUtils::formatErrorMessage( ex.what() ), tr( "HANA" ) );
  }

  return conn;
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
  try
  {
    if ( mConnection.isNull() )
      return false;

    QString sql = QStringLiteral( "DROP TABLE %1.%2" )
                  .arg( QgsHanaUtils::quotedIdentifier( schemaName ), QgsHanaUtils::quotedIdentifier( tableName ) );

    StatementRef stmt = mConnection->createStatement();
    stmt->execute( sql.toStdString().c_str() );
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
        mUserName = rs->getString( 1 )->c_str();
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

QVector<QgsHanaLayerProperty> QgsHanaConnection::getLayers(
  const QString &schemaName,
  bool allowGeometrylessTables,
  bool userTablesOnly )
{
  QVector<QgsHanaLayerProperty> list;
  try
  {
    // Read table names with geometry columns
    QString sqlTables = "SELECT SCHEMA_NAME, TABLE_NAME, COLUMN_NAME, DATA_TYPE_NAME, TABLE_OID, TABLE_COMMENTS FROM "
                        "(SELECT * FROM SYS.TABLE_COLUMNS WHERE TABLE_OID IN "
                        "(SELECT OBJECT_OID FROM OWNERSHIP WHERE OBJECT_TYPE = 'TABLE' AND OWNER_NAME LIKE '%1') AND "
                        "SCHEMA_NAME IN (SELECT SCHEMA_NAME FROM SYS.SCHEMAS WHERE HAS_PRIVILEGES = 'TRUE')) "
                        "INNER JOIN "
                        "(SELECT TABLE_OID AS TABLE_OID_2, COMMENTS AS TABLE_COMMENTS FROM SYS.TABLES WHERE "
                        "IS_USER_DEFINED_TYPE = 'FALSE' AND SCHEMA_NAME NOT LIKE_REGEXPR 'SYS|_SYS.*|UIS|SAP_XS|SAP_REST|HANA_XS') "
                        "ON TABLE_OID = TABLE_OID_2 AND SCHEMA_NAME LIKE '%2' AND DATA_TYPE_NAME LIKE_REGEXPR '%3'";
    QString schema = mUri.schema().isEmpty() ? schemaName : mUri.schema();
    QString sql = sqlTables.arg(
                    userTablesOnly ? "CURRENT_USER" : "%",
                    schema.isEmpty() ? "%" : schema,
                    "ST_GEOMETRY|ST_POINT" );
    StatementRef stmt = mConnection->createStatement();
    ResultSetRef rsTables = stmt->executeQuery( sql.toStdString().c_str() );

    while ( rsTables->next() )
    {
      QgsHanaLayerProperty layerProperty;
      layerProperty.schemaName = QgsHanaUtils::toQString( *rsTables->getString( 1 ) );
      layerProperty.tableName = QgsHanaUtils::toQString( *rsTables->getString( 2 ) );
      layerProperty.geometryColName = QgsHanaUtils::toQString( *rsTables->getString( 3 ) );
      layerProperty.tableComment = QgsHanaUtils::toQString( *rsTables->getString( 6 ) );

      addNewLayer( list, layerProperty );
    }
    rsTables->close();

    // Read geometryless tables
    if ( allowGeometrylessTables )
    {
      sql = QStringLiteral( "SELECT DISTINCT SCHEMA_NAME, TABLE_NAME, TABLE_COMMENTS FROM (%1)" ).arg(
              sqlTables.arg( userTablesOnly ? "CURRENT_USER" : "%", schema.isEmpty() ? "%" : schema, "" ) );
      ResultSetRef rsTables = stmt->executeQuery( sql.toStdString().c_str() );
      while ( rsTables->next() )
      {
        QgsHanaLayerProperty layerProperty;
        layerProperty.schemaName = QgsHanaUtils::toQString( *rsTables->getString( 1 ) );
        layerProperty.tableName = QgsHanaUtils::toQString( *rsTables->getString( 2 ) );
        layerProperty.geometryColName = "";
        layerProperty.tableComment = QgsHanaUtils::toQString( *rsTables->getString( 3 ) );

        addNewLayer( list, layerProperty, true );
      }
      rsTables->close();
    }

    // Read views
    sql = QStringLiteral( "SELECT SCHEMA_NAME, VIEW_NAME, COLUMN_NAME, DATA_TYPE_NAME, VIEW_OID, VIEW_COMMENTS FROM "
                          "(SELECT * FROM SYS.VIEW_COLUMNS WHERE VIEW_OID IN (SELECT OBJECT_OID FROM OWNERSHIP WHERE "
                          "OBJECT_TYPE = 'VIEW' AND OWNER_NAME LIKE '%1')) "
                          "INNER JOIN "
                          "(SELECT VIEW_OID AS VIEW_OID_2, COMMENTS AS VIEW_COMMENTS FROM SYS.VIEWS WHERE "
                          "IS_VALID = 'TRUE' AND SCHEMA_NAME IN (SELECT SCHEMA_NAME FROM SYS.SCHEMAS WHERE "
                          "HAS_PRIVILEGES = 'TRUE') AND SCHEMA_NAME NOT LIKE_REGEXPR 'SYS|_SYS.*|UIS|SAP_XS|SAP_REST|HANA_XS') "
                          "ON VIEW_OID = VIEW_OID_2 AND SCHEMA_NAME LIKE '%2' AND DATA_TYPE_NAME LIKE_REGEXPR '%3'" )
          .arg(
            userTablesOnly ? "CURRENT_USER" : "%",
            schema.isEmpty() ? "%" : schema,
            allowGeometrylessTables ? "" : "ST_GEOMETRY|ST_POINT" );
    ResultSetRef rsViews = stmt->executeQuery( sql.toStdString().c_str() );
    while ( rsViews->next() )
    {
      QgsHanaLayerProperty layerProperty;
      layerProperty.schemaName = QgsHanaUtils::toQString( *rsViews->getString( 1 ) );
      layerProperty.tableName = QgsHanaUtils::toQString( *rsViews->getString( 2 ) );
      layerProperty.geometryColName = ( QgsHanaUtils::toQString( *rsViews->getString( 4 ) ) != "ST_GEOMETRY" ) ? ""
                                      : QgsHanaUtils::toQString( *rsViews->getString( 3 ) );
      layerProperty.tableComment = QgsHanaUtils::toQString( *rsViews->getString( 6 ) );
      layerProperty.isView = true;

      addNewLayer( list, layerProperty, layerProperty.geometryColName.isEmpty() );
    }
    rsViews->close();

    int size = list.size();
    for ( int i = 0; i < size; ++i )
    {
      QgsHanaLayerProperty &lp1 = list[i];
      bool found = false;
      for ( int j = 0; j < size; ++j )
      {
        if ( i != j )
        {
          const QgsHanaLayerProperty &lp2 = list.at( j );
          if ( lp1.schemaName == lp2.schemaName && lp1.tableName == lp2.tableName )
          {
            found = true;
            break;
          }
        }
      }

      if ( !found )
        lp1.isUnique = true;
    }
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
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
  QVector<QgsHanaSchemaProperty> list;
  try
  {
    QString sql = QStringLiteral( "SELECT SCHEMA_NAME, SCHEMA_OWNER FROM SYS.SCHEMAS WHERE "
                                  "HAS_PRIVILEGES = 'TRUE' AND SCHEMA_OWNER LIKE '%1' AND "
                                  "SCHEMA_NAME NOT LIKE_REGEXPR 'SYS|_SYS.*|UIS|SAP_XS|SAP_REST|HANA_XS|XSSQLCC_' AND "
                                  "SCHEMA_NAME LIKE '%2'" )
                  .arg( ownerName.isEmpty() ? QStringLiteral( "%" ) : ownerName, mUri.schema().isEmpty() ? QStringLiteral( "%" ) : mUri.schema() );
    StatementRef stmt = mConnection->createStatement();
    ResultSetRef rsSchemas = stmt->executeQuery( sql.toStdString().c_str() );
    while ( rsSchemas->next() )
    {
      QgsHanaSchemaProperty schema;
      schema.name = QgsHanaUtils::toQString( rsSchemas->getString( 1 ) );
      schema.owner = QgsHanaUtils::toQString( rsSchemas->getString( 2 ) );
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

  int ret = -1;

  try
  {
    StatementRef stmt = mConnection->createStatement();

    if ( !layerProperty.isView )
    {
      QString sql = QStringLiteral( "SELECT SRS_ID FROM SYS.ST_GEOMETRY_COLUMNS "
                                    "WHERE SCHEMA_NAME = '%1' AND TABLE_NAME = '%2' AND COLUMN_NAME = '%3'" )
                    .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName );
      ResultSetRef rsSrid = stmt->executeQuery( sql.toStdString().c_str() );
      ret = rsSrid->next() ? *rsSrid->getInt( 1 ) : -1;
      rsSrid->close();
    }
    else
    {
      QString sql = QStringLiteral( "SELECT %1.ST_SRID() FROM %2.%3 WHERE %1 IS NOT NULL LIMIT 10" )
                    .arg( QgsHanaUtils::quotedIdentifier( layerProperty.geometryColName ),
                          QgsHanaUtils::quotedIdentifier( layerProperty.schemaName ),
                          QgsHanaUtils::quotedIdentifier( layerProperty.tableName ) );
      ResultSetRef rsSrid = stmt->executeQuery( sql.toStdString().c_str() );
      int srid = -1, prevSrid = -1;
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

      ret = srid;
      rsSrid->close();
    }
  }
  catch ( const Exception &ex )
  {
    throw QgsHanaException( ex.what() );
  }

  return ret;
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
      String clmName = *rsPrimaryKeys->getString( 4 );
      if ( !clmName.isNull() )
      {
        ResultSetRef rsColumns = dbmd->getColumns( nullptr,
                                 layerProperty.schemaName.toStdString().c_str(),
                                 layerProperty.tableName.toStdString().c_str(),
                                 clmName->c_str() );

        if ( rsColumns->next() )
        {
          Short dataType = rsColumns->getShort( 5 );
          if ( !dataType.isNull() )
          {
            short dt = *dataType;
            if ( dt == SQLDataTypes::TinyInt || dt == SQLDataTypes::SmallInt ||
                 dt == SQLDataTypes::Integer || dt == SQLDataTypes::BigInt )
              ret << QString( clmName->c_str() );
          }
        }
        rsColumns->close();
      }
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

  try
  {
    QString sql = QStringLiteral( "SELECT upper(%1.ST_GeometryType()) AS geom_type FROM %2.%3 "
                                  "WHERE %1 IS NOT NULL LIMIT 10" ).arg(
                    QgsHanaUtils::quotedIdentifier( layerProperty.geometryColName ),
                    QgsHanaUtils::quotedIdentifier( layerProperty.schemaName ),
                    QgsHanaUtils::quotedIdentifier( layerProperty.tableName ) );
    StatementRef stmt = mConnection->createStatement();
    ResultSetRef rsGeomType = stmt->executeQuery( sql.toStdString().c_str() );
    QgsWkbTypes::Type geomType = QgsWkbTypes::Unknown, prevGeomType = QgsWkbTypes::Unknown;
    while ( rsGeomType->next() )
    {
      geomType = QgsWkbTypes::singleType( QgsHanaUtils::toWkbType( rsGeomType->getString( 1 )->c_str() ) );
      if ( prevGeomType != QgsWkbTypes::Unknown && geomType != prevGeomType )
      {
        geomType = QgsWkbTypes::Unknown;
        break;
      }
      prevGeomType = geomType;
    }
    ret = geomType;
    rsGeomType->close();
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
