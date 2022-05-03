/***************************************************************************
  qgsOracleproviderconnection.cpp - QgsOracleProviderConnection

 ---------------------
 begin                : 28.12.2020
 copyright            : (C) 2020 by Julien Cabieces
 email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsoracleproviderconnection.h"
#include "qgsoracleconn.h"
#include "qgsoracleconnpool.h"
#include "qgssettings.h"
#include "qgsoracleprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"

#include <QSqlRecord>
#include <QSqlField>

// read from QSettings and used in the provider connection
const QStringList CONFIGURATION_PARAMETERS
{
  QStringLiteral( "geometryColumnsOnly" ),
  QStringLiteral( "allowGeometrylessTables" ),
  QStringLiteral( "disableInvalidGeometryHandling" ),
  QStringLiteral( "onlyExistingTypes" ),
  QStringLiteral( "saveUsername" ),
  QStringLiteral( "savePassword" ),
};

// read from uri and used in the provider connection
const QStringList EXTRA_CONNECTION_PARAMETERS
{
  QStringLiteral( "dboptions" ),
  QStringLiteral( "dbworkspace" )
};

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "oracle" );
  setUri( QgsOracleConn::connUri( name ).uri() );
  setDefaultCapabilities();

  // load existing configuration
  QgsSettings settings;
  QVariantMap configuration;
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    const QVariant v = settings.value( QStringLiteral( "/Oracle/connections/%1/%2" ).arg( name, p ) );
    if ( v.isValid() )
    {
      configuration.insert( p, v );
    }
  }
  setConfiguration( configuration );
}

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "oracle" );
  setDefaultCapabilities();

  // Additional connection information
  const QgsDataSourceUri inputUri( uri );
  QgsDataSourceUri currentUri { QgsDataSourceUri( uri ).connectionInfo( false ) };

  if ( inputUri.hasParam( QStringLiteral( "estimatedMetadata" ) ) )
  {
    currentUri.setUseEstimatedMetadata( inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == QStringLiteral( "true" )
                                        || inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == '1' );
  }

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( inputUri.hasParam( param ) )
    {
      currentUri.setParam( param, inputUri.param( param ) );
    }
  }

  setUri( currentUri.uri() );
}

void QgsOracleProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropVectorTable,
    Capability::DropRasterTable,
    Capability::CreateVectorTable,
    Capability::RenameVectorTable,
    Capability::RenameRasterTable,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists,
    Capability::CreateSpatialIndex,
    Capability::SpatialIndexExists,
    Capability::DeleteSpatialIndex,
    Capability::DeleteField,
    Capability::DeleteFieldCascade,
    Capability::AddField,
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::SinglePart,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
  };
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsOracleProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  const QgsDataSourceUri tUri( layerSource );
  options.primaryKeyColumns = tUri.keyColumn().split( ',' );
  options.disableSelectAtId = tUri.selectAtIdDisabled();
  options.geometryColumn = tUri.geometryColumn();
  options.filter = tUri.sql();
  const QString trimmedTable { tUri.table().trimmed() };
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : QStringLiteral( "SELECT * FROM %1" ).arg( tUri.quotedTablename() );
  return options;
}

QgsVectorLayer *QgsOracleProviderConnection::createSqlVectorLayer( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options ) const
{

  // Precondition
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri( ) );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( ! options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );
    tUri.setTable( QStringLiteral( "(%1)" ).arg( options.sql ) );
  }
  else
  {
    // Disable when there is no pk
    tUri.setUseEstimatedMetadata( false );
    int pkId { 0 };
    while ( options.sql.contains( QStringLiteral( "qgis_generated_uid_%1_" ).arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId ++;
    }
    tUri.setKeyColumn( QStringLiteral( "qgis_generated_uid_%1_" ).arg( pkId ) );

    int sqlId { 0 };
    while ( options.sql.contains( QStringLiteral( "qgis_generated_subq_%1_" ).arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId ++;
    }
    tUri.setTable( QStringLiteral( "(SELECT row_number() over (ORDER BY NULL) AS qgis_generated_uid_%1_, qgis_generated_subq_%3_.* FROM (%2\n) qgis_generated_subq_%3_\n)" ).arg( QString::number( pkId ), options.sql, QString::number( sqlId ) ) );
  }

  if ( ! options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  std::unique_ptr<QgsVectorLayer> vl = std::make_unique<QgsVectorLayer>( tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey() );

  // Try to guess the geometry and srid
  if ( ! vl->isValid() )
  {
    const QString limit { QgsDataSourceUri( uri() ).useEstimatedMetadata() ? QStringLiteral( "AND ROWNUM < 100" ) : QString() };
    const QString sql { QStringLiteral( R"(
      SELECT DISTINCT a.%1.SDO_GTYPE As gtype,
                            a.%1.SDO_SRID
            FROM (%2) a
            WHERE a.%1 IS NOT NULL %3
            ORDER BY a.%1.SDO_GTYPE
    )" ).arg( options.geometryColumn, options.sql, limit ) };
    const QList<QList<QVariant>> candidates { executeSql( sql ) };
    for ( const QList<QVariant> &row : std::as_const( candidates ) )
    {
      bool ok;
      const int type { row[ 0 ].toInt( &ok ) };
      if ( ok )
      {
        const int srid { row[ 0 ].toInt( &ok ) };

        if ( ok )
        {

          QgsWkbTypes::Type geomType { QgsWkbTypes::Type::Unknown };

          switch ( type )
          {
            case 2001:
              geomType = QgsWkbTypes::Point;
              break;
            case 2002:
              geomType = QgsWkbTypes::LineString;
              break;
            case 2003:
              geomType = QgsWkbTypes::Polygon;
              break;
            // Note: 2004 is missing
            case 2005:
              geomType = QgsWkbTypes::MultiPoint;
              break;
            case 2006:
              geomType = QgsWkbTypes::MultiLineString;
              break;
            case 2007:
              geomType = QgsWkbTypes::MultiPolygon;
              break;
            // 3K...
            case 3001:
              geomType = QgsWkbTypes::Point25D;
              break;
            case 3002:
              geomType = QgsWkbTypes::LineString25D;
              break;
            case 3003:
              geomType = QgsWkbTypes::Polygon25D;
              break;
            // Note: 3004 is missing
            case 3005:
              geomType = QgsWkbTypes::MultiPoint25D;
              break;
            case 3006:
              geomType = QgsWkbTypes::MultiLineString25D;
              break;
            case 3007:
              geomType = QgsWkbTypes::MultiPolygon25D;
              break;
            default:
              geomType = QgsWkbTypes::Type::Unknown;
          }
          if ( geomType != QgsWkbTypes::Type::Unknown )
          {
            tUri.setSrid( QString::number( srid ) );
            tUri.setWkbType( geomType );
            vl = std::make_unique<QgsVectorLayer>( tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey() );
            if ( vl->isValid() )
            {
              break;
            }
          }
        }
      }
    }
  }

  return vl.release();
}

void QgsOracleProviderConnection::store( const QString &name ) const
{
  QString baseKey = QStringLiteral( "/Oracle/connections/" );
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name );

  // From URI
  const QgsDataSourceUri dsUri { uri() };
  settings.setValue( "authcfg", dsUri.authConfigId() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "estimatedMetadata", dsUri.useEstimatedMetadata() );

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( dsUri.hasParam( param ) )
    {
      settings.setValue( param, dsUri.param( param ) );
    }
  }

  // From configuration
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
}

void QgsOracleProviderConnection::remove( const QString &name ) const
{
  QgsOracleConn::deleteConnection( name );
}

QList<QgsVectorDataProvider::NativeType> QgsOracleProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsPoolOracleConn conn( QgsDataSourceUri{ uri() }.connectionInfo( false ) );
  if ( conn.get() )
  {
    types = conn.get()->nativeTypes();
  }
  if ( types.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  }
  return types;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsOracleProviderConnection::sqlDictionary()
{
  return
  {
    {
      Qgis::SqlKeywordCategory::Keyword,
      {
        // From: http://docs.oracle.com/cd/B19306_01/server.102/b14200/ap_keywd.htm
        QStringLiteral( "ACCESS" ),
        QStringLiteral( "ADD" ),
        QStringLiteral( "ALL" ),
        QStringLiteral( "ALTER" ),
        QStringLiteral( "AND" ),
        QStringLiteral( "ANY" ),
        QStringLiteral( "AS" ),
        QStringLiteral( "ASC" ),

        QStringLiteral( "AUDIT" ),
        QStringLiteral( "BETWEEN" ),
        QStringLiteral( "BY" ),
        QStringLiteral( "CHAR" ),
        QStringLiteral( "CHECK" ),
        QStringLiteral( "CLUSTER" ),
        QStringLiteral( "COLUMN" ),

        QStringLiteral( "COMMENT" ),
        QStringLiteral( "COMPRESS" ),
        QStringLiteral( "CONNECT" ),
        QStringLiteral( "CREATE" ),
        QStringLiteral( "CURRENT" ),
        QStringLiteral( "DATE" ),

        QStringLiteral( "DECIMAL" ),
        QStringLiteral( "DEFAULT" ),
        QStringLiteral( "DELETE" ),
        QStringLiteral( "DESC" ),
        QStringLiteral( "DISTINCT" ),
        QStringLiteral( "DROP" ),

        QStringLiteral( "ELSE" ),
        QStringLiteral( "EXCLUSIVE" ),
        QStringLiteral( "EXISTS" ),
        QStringLiteral( "FILE" ),
        QStringLiteral( "FLOAT" ),
        QStringLiteral( "FOR" ),
        QStringLiteral( "FROM" ),

        QStringLiteral( "GRANT" ),
        QStringLiteral( "GROUP" ),
        QStringLiteral( "HAVING" ),
        QStringLiteral( "IDENTIFIED" ),
        QStringLiteral( "IMMEDIATE" ),
        QStringLiteral( "IN" ),

        QStringLiteral( "INCREMENT" ),
        QStringLiteral( "INDEX" ),
        QStringLiteral( "INITIAL" ),
        QStringLiteral( "INSERT" ),
        QStringLiteral( "INTEGER" ),
        QStringLiteral( "INTERSECT" ),

        QStringLiteral( "INTO" ),
        QStringLiteral( "IS" ),
        QStringLiteral( "LEVEL" ),
        QStringLiteral( "LIKE" ),
        QStringLiteral( "LOCK" ),
        QStringLiteral( "LONG" ),
        QStringLiteral( "MAXEXTENTS" ),

        QStringLiteral( "MINUS" ),
        QStringLiteral( "MLSLABEL" ),
        QStringLiteral( "MODE" ),
        QStringLiteral( "MODIFY" ),
        QStringLiteral( "NOAUDIT" ),
        QStringLiteral( "NOCOMPRESS" ),

        QStringLiteral( "NOT" ),
        QStringLiteral( "NOWAIT" ),
        QStringLiteral( "NULL" ),
        QStringLiteral( "NUMBER" ),
        QStringLiteral( "OF" ),
        QStringLiteral( "OFFLINE" ),
        QStringLiteral( "ON" ),

        QStringLiteral( "ONLINE" ),
        QStringLiteral( "OPTION" ),
        QStringLiteral( "OR" ),
        QStringLiteral( "ORDER" ),
        QStringLiteral( "PCTFREE" ),
        QStringLiteral( "PRIOR" ),

        QStringLiteral( "PRIVILEGES" ),
        QStringLiteral( "PUBLIC" ),
        QStringLiteral( "RAW" ),
        QStringLiteral( "RENAME" ),
        QStringLiteral( "RESOURCE" ),
        QStringLiteral( "REVOKE" ),

        QStringLiteral( "ROW" ),
        QStringLiteral( "ROWID" ),
        QStringLiteral( "ROWNUM" ),
        QStringLiteral( "ROWS" ),
        QStringLiteral( "SELECT" ),
        QStringLiteral( "SESSION" ),
        QStringLiteral( "SET" ),

        QStringLiteral( "SHARE" ),
        QStringLiteral( "SIZE" ),
        QStringLiteral( "SMALLINT" ),
        QStringLiteral( "START" ),
        QStringLiteral( "SUCCESSFUL" ),
        QStringLiteral( "SYNONYM" ),

        QStringLiteral( "SYSDATE" ),
        QStringLiteral( "TABLE" ),
        QStringLiteral( "THEN" ),
        QStringLiteral( "TO" ),
        QStringLiteral( "TRIGGER" ),
        QStringLiteral( "UID" ),
        QStringLiteral( "UNION" ),

        QStringLiteral( "UNIQUE" ),
        QStringLiteral( "UPDATE" ),
        QStringLiteral( "USER" ),
        QStringLiteral( "VALIDATE" ),
        QStringLiteral( "VALUES" ),
        QStringLiteral( "VARCHAR" ),

        QStringLiteral( "VARCHAR2" ),
        QStringLiteral( "VIEW" ),
        QStringLiteral( "WHENEVER" ),
        QStringLiteral( "WHERE" ),
        QStringLiteral( "WITH" ),

        // From http://docs.oracle.com/cd/B13789_01/appdev.101/a42525/apb.htm
        QStringLiteral( "ADMIN" ),
        QStringLiteral( "CURSOR" ),
        QStringLiteral( "FOUND" ),
        QStringLiteral( "MOUNT" ),
        QStringLiteral( "AFTER" ),
        QStringLiteral( "CYCLE" ),
        QStringLiteral( "FUNCTION" ),

        QStringLiteral( "NEXT" ),
        QStringLiteral( "ALLOCATE" ),
        QStringLiteral( "DATABASE" ),
        QStringLiteral( "GO" ),
        QStringLiteral( "NEW" ),
        QStringLiteral( "ANALYZE" ),

        QStringLiteral( "DATAFILE" ),
        QStringLiteral( "GOTO" ),
        QStringLiteral( "NOARCHIVELOG" ),
        QStringLiteral( "ARCHIVE" ),
        QStringLiteral( "DBA" ),
        QStringLiteral( "GROUPS" ),

        QStringLiteral( "NOCACHE" ),
        QStringLiteral( "ARCHIVELOG" ),
        QStringLiteral( "DEC" ),
        QStringLiteral( "INCLUDING" ),
        QStringLiteral( "NOCYCLE" ),

        QStringLiteral( "AUTHORIZATION" ),
        QStringLiteral( "DECLARE" ),
        QStringLiteral( "INDICATOR" ),
        QStringLiteral( "NOMAXVALUE" ),
        QStringLiteral( "AVG" ),

        QStringLiteral( "DISABLE" ),
        QStringLiteral( "INITRANS" ),
        QStringLiteral( "NOMINVALUE" ),
        QStringLiteral( "BACKUP" ),
        QStringLiteral( "DISMOUNT" ),

        QStringLiteral( "INSTANCE" ),
        QStringLiteral( "NONE" ),
        QStringLiteral( "BEGIN" ),
        QStringLiteral( "DOUBLE" ),
        QStringLiteral( "INT" ),
        QStringLiteral( "NOORDER" ),
        QStringLiteral( "BECOME" ),

        QStringLiteral( "DUMP" ),
        QStringLiteral( "KEY" ),
        QStringLiteral( "NORESETLOGS" ),
        QStringLiteral( "BEFORE" ),
        QStringLiteral( "EACH" ),
        QStringLiteral( "LANGUAGE" ),

        QStringLiteral( "NORMAL" ),
        QStringLiteral( "BLOCK" ),
        QStringLiteral( "ENABLE" ),
        QStringLiteral( "LAYER" ),
        QStringLiteral( "NOSORT" ),
        QStringLiteral( "BODY" ),
        QStringLiteral( "END" ),

        QStringLiteral( "LINK" ),
        QStringLiteral( "NUMERIC" ),
        QStringLiteral( "CACHE" ),
        QStringLiteral( "ESCAPE" ),
        QStringLiteral( "LISTS" ),
        QStringLiteral( "OFF" ),
        QStringLiteral( "CANCEL" ),

        QStringLiteral( "EVENTS" ),
        QStringLiteral( "LOGFILE" ),
        QStringLiteral( "OLD" ),
        QStringLiteral( "CASCADE" ),
        QStringLiteral( "EXCEPT" ),
        QStringLiteral( "MANAGE" ),
        QStringLiteral( "ONLY" ),

        QStringLiteral( "CHANGE" ),
        QStringLiteral( "EXCEPTIONS" ),
        QStringLiteral( "MANUAL" ),
        QStringLiteral( "OPEN" ),
        QStringLiteral( "CHARACTER" ),
        QStringLiteral( "EXEC" ),

        QStringLiteral( "MAX" ),
        QStringLiteral( "OPTIMAL" ),
        QStringLiteral( "CHECKPOINT" ),
        QStringLiteral( "EXPLAIN" ),
        QStringLiteral( "MAXDATAFILES" ),
        QStringLiteral( "OWN" ),

        QStringLiteral( "CLOSE" ),
        QStringLiteral( "EXECUTE" ),
        QStringLiteral( "MAXINSTANCES" ),
        QStringLiteral( "PACKAGE" ),
        QStringLiteral( "COBOL" ),
        QStringLiteral( "EXTENT" ),

        QStringLiteral( "MAXLOGFILES" ),
        QStringLiteral( "PARALLEL" ),
        QStringLiteral( "COMMIT" ),
        QStringLiteral( "EXTERNALLY" ),

        QStringLiteral( "MAXLOGHISTORY" ),
        QStringLiteral( "PCTINCREASE" ),
        QStringLiteral( "COMPILE" ),
        QStringLiteral( "FETCH" ),

        QStringLiteral( "MAXLOGMEMBERS" ),
        QStringLiteral( "PCTUSED" ),
        QStringLiteral( "CONSTRAINT" ),
        QStringLiteral( "FLUSH" ),
        QStringLiteral( "MAXTRANS" ),

        QStringLiteral( "PLAN" ),
        QStringLiteral( "CONSTRAINTS" ),
        QStringLiteral( "FREELIST" ),
        QStringLiteral( "MAXVALUE" ),
        QStringLiteral( "PLI" ),
        QStringLiteral( "CONTENTS" ),

        QStringLiteral( "FREELISTS" ),
        QStringLiteral( "MIN" ),
        QStringLiteral( "PRECISION" ),
        QStringLiteral( "CONTINUE" ),
        QStringLiteral( "FORCE" ),

        QStringLiteral( "MINEXTENTS" ),
        QStringLiteral( "PRIMARY" ),
        QStringLiteral( "CONTROLFILE" ),
        QStringLiteral( "FOREIGN" ),
        QStringLiteral( "MINVALUE" ),

        QStringLiteral( "PRIVATE" ),
        QStringLiteral( "COUNT" ),
        QStringLiteral( "FORTRAN" ),
        QStringLiteral( "MODULE" ),
        QStringLiteral( "PROCEDURE" ),
        QStringLiteral( "PROFILE" ),

        QStringLiteral( "SAVEPOINT" ),
        QStringLiteral( "SQLSTATE" ),
        QStringLiteral( "TRACING" ),
        QStringLiteral( "QUOTA" ),
        QStringLiteral( "SCHEMA" ),

        QStringLiteral( "STATEMENT_ID" ),
        QStringLiteral( "TRANSACTION" ),
        QStringLiteral( "READ" ),
        QStringLiteral( "SCN" ),
        QStringLiteral( "STATISTICS" ),

        QStringLiteral( "TRIGGERS" ),
        QStringLiteral( "REAL" ),
        QStringLiteral( "SECTION" ),
        QStringLiteral( "STOP" ),
        QStringLiteral( "TRUNCATE" ),
        QStringLiteral( "RECOVER" ),

        QStringLiteral( "SEGMENT" ),
        QStringLiteral( "STORAGE" ),
        QStringLiteral( "UNDER" ),
        QStringLiteral( "REFERENCES" ),
        QStringLiteral( "SEQUENCE" ),
        QStringLiteral( "SUM" ),

        QStringLiteral( "UNLIMITED" ),
        QStringLiteral( "REFERENCING" ),
        QStringLiteral( "SHARED" ),
        QStringLiteral( "SWITCH" ),
        QStringLiteral( "UNTIL" ),

        QStringLiteral( "RESETLOGS" ),
        QStringLiteral( "SNAPSHOT" ),
        QStringLiteral( "SYSTEM" ),
        QStringLiteral( "USE" ),
        QStringLiteral( "RESTRICTED" ),
        QStringLiteral( "SOME" ),

        QStringLiteral( "TABLES" ),
        QStringLiteral( "USING" ),
        QStringLiteral( "REUSE" ),
        QStringLiteral( "SORT" ),
        QStringLiteral( "TABLESPACE" ),
        QStringLiteral( "WHEN" ),
        QStringLiteral( "ROLE" ),

        QStringLiteral( "SQL" ),
        QStringLiteral( "TEMPORARY" ),
        QStringLiteral( "WRITE" ),
        QStringLiteral( "ROLES" ),
        QStringLiteral( "SQLCODE" ),
        QStringLiteral( "THREAD" ),
        QStringLiteral( "WORK" ),

        QStringLiteral( "ROLLBACK" ),
        QStringLiteral( "SQLERROR" ),
        QStringLiteral( "TIME" ),
        QStringLiteral( "ABORT" ),
        QStringLiteral( "BETWEEN" ),
        QStringLiteral( "CRASH" ),

        QStringLiteral( "DIGITS" ),
        QStringLiteral( "ACCEPT" ),
        QStringLiteral( "BINARY_INTEGER" ),
        QStringLiteral( "CREATE" ),
        QStringLiteral( "DISPOSE" ),

        QStringLiteral( "ACCESS" ),
        QStringLiteral( "BODY" ),
        QStringLiteral( "CURRENT" ),
        QStringLiteral( "DISTINCT" ),
        QStringLiteral( "ADD" ),
        QStringLiteral( "BOOLEAN" ),

        QStringLiteral( "CURRVAL" ),
        QStringLiteral( "DO" ),
        QStringLiteral( "ALL" ),
        QStringLiteral( "BY" ),
        QStringLiteral( "CURSOR" ),
        QStringLiteral( "DROP" ),
        QStringLiteral( "ALTER" ),
        QStringLiteral( "CASE" ),

        QStringLiteral( "DATABASE" ),
        QStringLiteral( "ELSE" ),
        QStringLiteral( "AND" ),
        QStringLiteral( "CHAR" ),
        QStringLiteral( "DATA_BASE" ),
        QStringLiteral( "ELSIF" ),
        QStringLiteral( "ANY" ),

        QStringLiteral( "CHAR_BASE" ),
        QStringLiteral( "DATE" ),
        QStringLiteral( "END" ),
        QStringLiteral( "ARRAY" ),
        QStringLiteral( "CHECK" ),
        QStringLiteral( "DBA" ),
        QStringLiteral( "ENTRY" ),

        QStringLiteral( "ARRAYLEN" ),
        QStringLiteral( "CLOSE" ),
        QStringLiteral( "DEBUGOFF" ),
        QStringLiteral( "EXCEPTION" ),
        QStringLiteral( "AS" ),
        QStringLiteral( "CLUSTER" ),

        QStringLiteral( "DEBUGON" ),
        QStringLiteral( "EXCEPTION_INIT" ),
        QStringLiteral( "ASC" ),
        QStringLiteral( "CLUSTERS" ),
        QStringLiteral( "DECLARE" ),

        QStringLiteral( "EXISTS" ),
        QStringLiteral( "ASSERT" ),
        QStringLiteral( "COLAUTH" ),
        QStringLiteral( "DECIMAL" ),
        QStringLiteral( "EXIT" ),
        QStringLiteral( "ASSIGN" ),

        QStringLiteral( "COLUMNS" ),
        QStringLiteral( "DEFAULT" ),
        QStringLiteral( "FALSE" ),
        QStringLiteral( "AT" ),
        QStringLiteral( "COMMIT" ),
        QStringLiteral( "DEFINITION" ),

        QStringLiteral( "FETCH" ),
        QStringLiteral( "AUTHORIZATION" ),
        QStringLiteral( "COMPRESS" ),
        QStringLiteral( "DELAY" ),
        QStringLiteral( "FLOAT" ),
        QStringLiteral( "AVG" ),

        QStringLiteral( "CONNECT" ),
        QStringLiteral( "DELETE" ),
        QStringLiteral( "FOR" ),
        QStringLiteral( "BASE_TABLE" ),
        QStringLiteral( "CONSTANT" ),
        QStringLiteral( "DELTA" ),

        QStringLiteral( "FORM" ),
        QStringLiteral( "BEGIN" ),
        QStringLiteral( "COUNT" ),
        QStringLiteral( "DESC" ),
        QStringLiteral( "FROM" ),
        QStringLiteral( "FUNCTION" ),
        QStringLiteral( "NEW" ),

        QStringLiteral( "RELEASE" ),
        QStringLiteral( "SUM" ),
        QStringLiteral( "GENERIC" ),
        QStringLiteral( "NEXTVAL" ),
        QStringLiteral( "REMR" ),
        QStringLiteral( "TABAUTH" ),
        QStringLiteral( "GOTO" ),

        QStringLiteral( "NOCOMPRESS" ),
        QStringLiteral( "RENAME" ),
        QStringLiteral( "TABLE" ),
        QStringLiteral( "GRANT" ),
        QStringLiteral( "NOT" ),
        QStringLiteral( "RESOURCE" ),

        QStringLiteral( "TABLES" ),
        QStringLiteral( "GROUP" ),
        QStringLiteral( "NULL" ),
        QStringLiteral( "RETURN" ),
        QStringLiteral( "TASK" ),
        QStringLiteral( "HAVING" ),
        QStringLiteral( "NUMBER" ),

        QStringLiteral( "REVERSE" ),
        QStringLiteral( "TERMINATE" ),
        QStringLiteral( "IDENTIFIED" ),
        QStringLiteral( "NUMBER_BASE" ),
        QStringLiteral( "REVOKE" ),

        QStringLiteral( "THEN" ),
        QStringLiteral( "IF" ),
        QStringLiteral( "OF" ),
        QStringLiteral( "ROLLBACK" ),
        QStringLiteral( "TO" ),
        QStringLiteral( "IN" ),
        QStringLiteral( "ON" ),
        QStringLiteral( "ROWID" ),
        QStringLiteral( "TRUE" ),

        QStringLiteral( "INDEX" ),
        QStringLiteral( "OPEN" ),
        QStringLiteral( "ROWLABEL" ),
        QStringLiteral( "TYPE" ),
        QStringLiteral( "INDEXES" ),
        QStringLiteral( "OPTION" ),

        QStringLiteral( "ROWNUM" ),
        QStringLiteral( "UNION" ),
        QStringLiteral( "INDICATOR" ),
        QStringLiteral( "OR" ),
        QStringLiteral( "ROWTYPE" ),
        QStringLiteral( "UNIQUE" ),

        QStringLiteral( "INSERT" ),
        QStringLiteral( "ORDER" ),
        QStringLiteral( "RUN" ),
        QStringLiteral( "UPDATE" ),
        QStringLiteral( "INTEGER" ),
        QStringLiteral( "OTHERS" ),

        QStringLiteral( "SAVEPOINT" ),
        QStringLiteral( "USE" ),
        QStringLiteral( "INTERSECT" ),
        QStringLiteral( "OUT" ),
        QStringLiteral( "SCHEMA" ),
        QStringLiteral( "VALUES" ),

        QStringLiteral( "INTO" ),
        QStringLiteral( "PACKAGE" ),
        QStringLiteral( "SELECT" ),
        QStringLiteral( "VARCHAR" ),
        QStringLiteral( "IS" ),
        QStringLiteral( "PARTITION" ),

        QStringLiteral( "SEPARATE" ),
        QStringLiteral( "VARCHAR2" ),
        QStringLiteral( "LEVEL" ),
        QStringLiteral( "PCTFREE" ),
        QStringLiteral( "SET" ),
        QStringLiteral( "VARIANCE" ),

        QStringLiteral( "LIKE" ),
        QStringLiteral( "POSITIVE" ),
        QStringLiteral( "SIZE" ),
        QStringLiteral( "VIEW" ),
        QStringLiteral( "LIMITED" ),
        QStringLiteral( "PRAGMA" ),

        QStringLiteral( "SMALLINT" ),
        QStringLiteral( "VIEWS" ),
        QStringLiteral( "LOOP" ),
        QStringLiteral( "PRIOR" ),
        QStringLiteral( "SPACE" ),
        QStringLiteral( "WHEN" ),
        QStringLiteral( "MAX" ),

        QStringLiteral( "PRIVATE" ),
        QStringLiteral( "SQL" ),
        QStringLiteral( "WHERE" ),
        QStringLiteral( "MIN" ),
        QStringLiteral( "PROCEDURE" ),
        QStringLiteral( "SQLCODE" ),
        QStringLiteral( "WHILE" ),

        QStringLiteral( "MINUS" ),
        QStringLiteral( "PUBLIC" ),
        QStringLiteral( "SQLERRM" ),
        QStringLiteral( "WITH" ),
        QStringLiteral( "MLSLABEL" ),
        QStringLiteral( "RAISE" ),

        QStringLiteral( "START" ),
        QStringLiteral( "WORK" ),
        QStringLiteral( "MOD" ),
        QStringLiteral( "RANGE" ),
        QStringLiteral( "STATEMENT" ),
        QStringLiteral( "XOR" ),
        QStringLiteral( "MODE" ),

        QStringLiteral( "REAL" ),
        QStringLiteral( "STDDEV" ),
        QStringLiteral( "NATURAL" ),
        QStringLiteral( "RECORD" ),
        QStringLiteral( "SUBTYPE" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Function,
      {
        // From: https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions001.htm
        QStringLiteral( "CAST" ),
        QStringLiteral( "COALESCE" ),
        QStringLiteral( "DECODE" ),
        QStringLiteral( "GREATEST" ),
        QStringLiteral( "LEAST" ),
        QStringLiteral( "LNNVL" ),

        QStringLiteral( "NULLIF" ),
        QStringLiteral( "NVL" ),
        QStringLiteral( "NVL2" ),
        QStringLiteral( "SET" ),
        QStringLiteral( "UID" ),
        QStringLiteral( "USER" ),
        QStringLiteral( "USERENV" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Math,
      {
        QStringLiteral( "ABS" ),
        QStringLiteral( "ACOS" ),
        QStringLiteral( "ASIN" ),
        QStringLiteral( "ATAN" ),
        QStringLiteral( "ATAN2" ),
        QStringLiteral( "BITAND" ),
        QStringLiteral( "CEIL" ),
        QStringLiteral( "COS" ),

        QStringLiteral( "COSH" ),
        QStringLiteral( "EXP" ),
        QStringLiteral( "FLOOR" ),
        QStringLiteral( "LN" ),
        QStringLiteral( "LOG" ),
        QStringLiteral( "MOD" ),
        QStringLiteral( "NANVL" ),
        QStringLiteral( "POWER" ),

        QStringLiteral( "REMAINDER" ),
        QStringLiteral( "ROUND" ),
        QStringLiteral( "SIGN" ),
        QStringLiteral( "SIN" ),
        QStringLiteral( "SINH" ),
        QStringLiteral( "SQRT" ),
        QStringLiteral( "TAN" ),

        QStringLiteral( "TANH" ),
        QStringLiteral( "TRUNC" ),
        QStringLiteral( "WIDTH_BUCKET" )
      }
    },
    {
      Qgis::SqlKeywordCategory::String,
      {
        QStringLiteral( "CHR" ),
        QStringLiteral( "CONCAT" ),
        QStringLiteral( "INITCAP" ),
        QStringLiteral( "LOWER" ),
        QStringLiteral( "LPAD" ),
        QStringLiteral( "LTRIM" ),
        QStringLiteral( "NLS_INITCAP" ),

        QStringLiteral( "NLS_LOWER" ),
        QStringLiteral( "NLSSORT" ),
        QStringLiteral( "NLS_UPPER" ),
        QStringLiteral( "REGEXP_REPLACE" ),
        QStringLiteral( "REGEXP_SUBSTR" ),

        QStringLiteral( "REPLACE" ),
        QStringLiteral( "RPAD" ),
        QStringLiteral( "RTRIM" ),
        QStringLiteral( "SOUNDEX" ),
        QStringLiteral( "SUBSTR" ),
        QStringLiteral( "TRANSLATE" ),
        QStringLiteral( "TREAT" ),

        QStringLiteral( "TRIM" ),
        QStringLiteral( "UPPER" ),
        QStringLiteral( "ASCII" ),
        QStringLiteral( "INSTR" ),
        QStringLiteral( "LENGTH" ),
        QStringLiteral( "REGEXP_INSTR" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Aggregate,
      {
        QStringLiteral( "AVG" ),
        QStringLiteral( "COLLECT" ),
        QStringLiteral( "CORR" ),
        QStringLiteral( "COUNT" ),
        QStringLiteral( "COVAR_POP" ),
        QStringLiteral( "COVAR_SAMP" ),
        QStringLiteral( "CUME_DIST" ),

        QStringLiteral( "DENSE_RANK" ),
        QStringLiteral( "FIRST" ),
        QStringLiteral( "GROUP_ID" ),
        QStringLiteral( "GROUPING" ),
        QStringLiteral( "GROUPING_ID" ),

        QStringLiteral( "LAST" ),
        QStringLiteral( "MAX" ),
        QStringLiteral( "MEDIAN" ),
        QStringLiteral( "MIN" ),
        QStringLiteral( "PERCENTILE_CONT" ),

        QStringLiteral( "PERCENTILE_DISC" ),
        QStringLiteral( "PERCENT_RANK" ),
        QStringLiteral( "RANK" ),

        QStringLiteral( "STATS_BINOMIAL_TEST" ),
        QStringLiteral( "STATS_CROSSTAB" ),
        QStringLiteral( "STATS_F_TEST" ),

        QStringLiteral( "STATS_KS_TEST" ),
        QStringLiteral( "STATS_MODE" ),
        QStringLiteral( "STATS_MW_TEST" ),

        QStringLiteral( "STATS_ONE_WAY_ANOVA" ),
        QStringLiteral( "STATS_WSR_TEST" ),
        QStringLiteral( "STDDEV" ),

        QStringLiteral( "STDDEV_POP" ),
        QStringLiteral( "STDDEV_SAMP" ),
        QStringLiteral( "SUM" ),
        QStringLiteral( "SYS_XMLAGG" ),
        QStringLiteral( "VAR_POP" ),

        QStringLiteral( "VAR_SAMP" ),
        QStringLiteral( "VARIANCE" ),
        QStringLiteral( "XMLAGG" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Geospatial,
      {
        // From http://docs.oracle.com/cd/B19306_01/appdev.102/b14255/toc.htm
        // Spatial operators
        QStringLiteral( "SDO_ANYINTERACT" ),
        QStringLiteral( "SDO_CONTAINS" ),
        QStringLiteral( "SDO_COVEREDBY" ),
        QStringLiteral( "SDO_COVERS" ),

        QStringLiteral( "SDO_EQUAL" ),
        QStringLiteral( "SDO_FILTER" ),
        QStringLiteral( "SDO_INSIDE" ),
        QStringLiteral( "SDO_JOIN" ),
        QStringLiteral( "SDO_NN" ),

        QStringLiteral( "SDO_NN_DISTANCE" ),
        QStringLiteral( "SDO_ON" ),
        QStringLiteral( "SDO_OVERLAPBDYDISJOINT" ),

        QStringLiteral( "SDO_OVERLAPBDYINTERSECT" ),
        QStringLiteral( "SDO_OVERLAPS" ),
        QStringLiteral( "SDO_RELATE" ),

        QStringLiteral( "SDO_TOUCH" ),
        QStringLiteral( "SDO_WITHIN_DISTANCE" ),

        // SPATIAL AGGREGATE FUNCTIONS
        QStringLiteral( "SDO_AGGR_CENTROID" ),
        QStringLiteral( "SDO_AGGR_CONCAT_LINES" ),

        QStringLiteral( "SDO_AGGR_CONVEXHULL" ),
        QStringLiteral( "SDO_AGGR_LRS_CONCAT" ),
        QStringLiteral( "SDO_AGGR_MBR" ),

        QStringLiteral( "SDO_AGGR_UNION" ),

        // COORDINATE SYSTEM TRANSFORMATION (SDO_CS)
        QStringLiteral( "SDO_CS.ADD_PREFERENCE_FOR_OP" ),
        QStringLiteral( "SDO_CS.CONVERT_NADCON_TO_XML" ),

        QStringLiteral( "SDO_CS.CONVERT_NTV2_TO_XML" ),
        QStringLiteral( "SDO_CS.CONVERT_XML_TO_NADCON" ),

        QStringLiteral( "SDO_CS.CONVERT_XML_TO_NTV2" ),
        QStringLiteral( "SDO_CS.CREATE_CONCATENATED_OP" ),

        QStringLiteral( "SDO_CS.CREATE_OBVIOUS_EPSG_RULES" ),

        QStringLiteral( "SDO_CS.CREATE_PREF_CONCATENATED_OP" ),

        QStringLiteral( "SDO_CS.DELETE_ALL_EPSG_RULES" ),
        QStringLiteral( "SDO_CS.DELETE_OP" ),

        QStringLiteral( "SDO_CS.DETERMINE_CHAIN" ),
        QStringLiteral( "SDO_CS.DETERMINE_DEFAULT_CHAIN" ),

        QStringLiteral( "SDO_CS.FIND_GEOG_CRS" ),
        QStringLiteral( "SDO_CS.FIND_PROJ_CRS" ),

        QStringLiteral( "SDO_CS.FROM_OGC_SIMPLEFEATURE_SRS" ),
        QStringLiteral( "SDO_CS.FROM_USNG" ),

        QStringLiteral( "SDO_CS.MAP_EPSG_SRID_TO_ORACLE" ),

        QStringLiteral( "SDO_CS.MAP_ORACLE_SRID_TO_EPSG" ),

        QStringLiteral( "SDO_CS.REVOKE_PREFERENCE_FOR_OP" ),

        QStringLiteral( "SDO_CS.TO_OGC_SIMPLEFEATURE_SRS" ),
        QStringLiteral( "SDO_CS.TO_USNG" ),

        QStringLiteral( "SDO_CS.TRANSFORM" ),
        QStringLiteral( "SDO_CS.TRANSFORM_LAYER" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_ALL_EPSG_CRS" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_CRS" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_DATUM" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_ELLIPS" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_OP" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_PARAM" ),

        QStringLiteral( "SDO_CS.UPDATE_WKTS_FOR_EPSG_PM" ),
        QStringLiteral( "SDO_CS.VALIDATE_WKT" ),

        QStringLiteral( "SDO_CS.VIEWPORT_TRANSFORM" ),

        // GEOCODING (SDO_GCDR)
        QStringLiteral( "SDO_GCDR.GEOCODE" ),
        QStringLiteral( "SDO_GCDR.GEOCODE_ADDR" ),

        QStringLiteral( "SDO_GCDR.GEOCODE_ADDR_ALL" ),
        QStringLiteral( "SDO_GCDR.GEOCODE_ALL" ),

        QStringLiteral( "SDO_GCDR.GEOCODE_AS_GEOMETRY" ),
        QStringLiteral( "SDO_GCDR.REVERSE_GEOCODE" ),

        // GEOMETRY (SDO_GEOM)
        QStringLiteral( "SDO_GEOM.RELATE" ),
        QStringLiteral( "SDO_GEOM.SDO_ARC_DENSIFY" ),

        QStringLiteral( "SDO_GEOM.SDO_AREA" ),
        QStringLiteral( "SDO_GEOM.SDO_BUFFER" ),

        QStringLiteral( "SDO_GEOM.SDO_CENTROID" ),
        QStringLiteral( "SDO_GEOM.SDO_CONVEXHULL" ),

        QStringLiteral( "SDO_GEOM.SDO_DIFFERENCE" ),
        QStringLiteral( "SDO_GEOM.SDO_DISTANCE" ),

        QStringLiteral( "SDO_GEOM.SDO_INTERSECTION" ),
        QStringLiteral( "SDO_GEOM.SDO_LENGTH" ),

        QStringLiteral( "SDO_GEOM.SDO_MAX_MBR_ORDINATE" ),
        QStringLiteral( "SDO_GEOM.SDO_MBR" ),

        QStringLiteral( "SDO_GEOM.SDO_MIN_MBR_ORDINATE" ),
        QStringLiteral( "SDO_GEOM.SDO_POINTONSURFACE" ),

        QStringLiteral( "SDO_GEOM.SDO_UNION" ),
        QStringLiteral( "SDO_GEOM.SDO_XOR" ),

        QStringLiteral( "SDO_GEOM.VALIDATE_GEOMETRY_WITH_CONTEXT" ),

        QStringLiteral( "SDO_GEOM.VALIDATE_LAYER_WITH_CONTEXT" ),

        QStringLiteral( "SDO_GEOM.WITHIN_DISTANCE" ),

        // LINEAR REFERENCING SYSTEM (SDO_LRS)
        QStringLiteral( "SDO_LRS.CLIP_GEOM_SEGMENT" ),
        QStringLiteral( "SDO_LRS.CONCATENATE_GEOM_SEGMENTS" ),

        QStringLiteral( "SDO_LRS.CONNECTED_GEOM_SEGMENTS" ),

        QStringLiteral( "SDO_LRS.CONVERT_TO_LRS_DIM_ARRAY" ),
        QStringLiteral( "SDO_LRS.CONVERT_TO_LRS_GEOM" ),

        QStringLiteral( "SDO_LRS.CONVERT_TO_LRS_LAYER" ),

        QStringLiteral( "SDO_LRS.CONVERT_TO_STD_DIM_ARRAY" ),
        QStringLiteral( "SDO_LRS.CONVERT_TO_STD_GEOM" ),

        QStringLiteral( "SDO_LRS.CONVERT_TO_STD_LAYER" ),
        QStringLiteral( "SDO_LRS.DEFINE_GEOM_SEGMENT" ),

        QStringLiteral( "SDO_LRS.DYNAMIC_SEGMENT" ),
        QStringLiteral( "SDO_LRS.FIND_LRS_DIM_POS" ),

        QStringLiteral( "SDO_LRS.FIND_MEASURE" ),
        QStringLiteral( "SDO_LRS.FIND_OFFSET" ),

        QStringLiteral( "SDO_LRS.GEOM_SEGMENT_END_MEASURE" ),
        QStringLiteral( "SDO_LRS.GEOM_SEGMENT_END_PT" ),

        QStringLiteral( "SDO_LRS.GEOM_SEGMENT_LENGTH" ),

        QStringLiteral( "SDO_LRS.GEOM_SEGMENT_START_MEASURE" ),

        QStringLiteral( "SDO_LRS.GEOM_SEGMENT_START_PT" ),
        QStringLiteral( "SDO_LRS.GET_MEASURE" ),

        QStringLiteral( "SDO_LRS.GET_NEXT_SHAPE_PT" ),
        QStringLiteral( "SDO_LRS.GET_NEXT_SHAPE_PT_MEASURE" ),

        QStringLiteral( "SDO_LRS.GET_PREV_SHAPE_PT" ),
        QStringLiteral( "SDO_LRS.GET_PREV_SHAPE_PT_MEASURE" ),

        QStringLiteral( "SDO_LRS.IS_GEOM_SEGMENT_DEFINED" ),

        QStringLiteral( "SDO_LRS.IS_MEASURE_DECREASING" ),
        QStringLiteral( "SDO_LRS.IS_MEASURE_INCREASING" ),

        QStringLiteral( "SDO_LRS.IS_SHAPE_PT_MEASURE" ),
        QStringLiteral( "SDO_LRS.LOCATE_PT" ),

        QStringLiteral( "SDO_LRS.LRS_INTERSECTION" ),
        QStringLiteral( "SDO_LRS.MEASURE_RANGE" ),

        QStringLiteral( "SDO_LRS.MEASURE_TO_PERCENTAGE" ),
        QStringLiteral( "SDO_LRS.OFFSET_GEOM_SEGMENT" ),

        QStringLiteral( "SDO_LRS.PERCENTAGE_TO_MEASURE" ),
        QStringLiteral( "SDO_LRS.PROJECT_PT" ),

        QStringLiteral( "SDO_LRS.REDEFINE_GEOM_SEGMENT" ),
        QStringLiteral( "SDO_LRS.RESET_MEASURE" ),

        QStringLiteral( "SDO_LRS.REVERSE_GEOMETRY" ),
        QStringLiteral( "SDO_LRS.REVERSE_MEASURE" ),

        QStringLiteral( "SDO_LRS.SET_PT_MEASURE" ),
        QStringLiteral( "SDO_LRS.SPLIT_GEOM_SEGMENT" ),

        QStringLiteral( "SDO_LRS.TRANSLATE_MEASURE" ),
        QStringLiteral( "SDO_LRS.VALID_GEOM_SEGMENT" ),

        QStringLiteral( "SDO_LRS.VALID_LRS_PT" ),
        QStringLiteral( "SDO_LRS.VALID_MEASURE" ),

        QStringLiteral( "SDO_LRS.VALIDATE_LRS_GEOMETRY" ),

        // SDO_MIGRATE
        QStringLiteral( "SDO_MIGRATE.TO_CURRENT" ),

        // SPATIAL ANALYSIS AND MINING (SDO_SAM)
        QStringLiteral( "SDO_SAM.AGGREGATES_FOR_GEOMETRY" ),
        QStringLiteral( "SDO_SAM.AGGREGATES_FOR_LAYER" ),

        QStringLiteral( "SDO_SAM.BIN_GEOMETRY" ),
        QStringLiteral( "SDO_SAM.BIN_LAYER" ),

        QStringLiteral( "SDO_SAM.COLOCATED_REFERENCE_FEATURES" ),

        QStringLiteral( "SDO_SAM.SIMPLIFY_GEOMETRY" ),
        QStringLiteral( "SDO_SAM.SIMPLIFY_LAYER" ),

        QStringLiteral( "SDO_SAM.SPATIAL_CLUSTERS" ),
        QStringLiteral( "SDO_SAM.TILED_AGGREGATES" ),

        QStringLiteral( "SDO_SAM.TILED_BINS" ),

        // TUNING (SDO_TUNE)
        QStringLiteral( "SDO_TUNE.AVERAGE_MBR" ),
        QStringLiteral( "SDO_TUNE.ESTIMATE_RTREE_INDEX_SIZE" ),

        QStringLiteral( "SDO_TUNE.EXTENT_OF" ),
        QStringLiteral( "SDO_TUNE.MIX_INFO" ),

        QStringLiteral( "SDO_TUNE.QUALITY_DEGRADATION" ),

        // UTILITY (SDO_UTIL)
        QStringLiteral( "SDO_UTIL.APPEND" ),
        QStringLiteral( "SDO_UTIL.CIRCLE_POLYGON" ),

        QStringLiteral( "SDO_UTIL.CONCAT_LINES" ),
        QStringLiteral( "SDO_UTIL.CONVERT_UNIT" ),

        QStringLiteral( "SDO_UTIL.ELLIPSE_POLYGON" ),
        QStringLiteral( "SDO_UTIL.EXTRACT" ),

        QStringLiteral( "SDO_UTIL.FROM_WKBGEOMETRY" ),
        QStringLiteral( "SDO_UTIL.FROM_WKTGEOMETRY" ),

        QStringLiteral( "SDO_UTIL.GETNUMELEM" ),
        QStringLiteral( "SDO_UTIL.GETNUMVERTICES" ),

        QStringLiteral( "SDO_UTIL.GETVERTICES" ),
        QStringLiteral( "SDO_UTIL.INITIALIZE_INDEXES_FOR_TTS" ),

        QStringLiteral( "SDO_UTIL.POINT_AT_BEARING" ),
        QStringLiteral( "SDO_UTIL.POLYGONTOLINE" ),

        QStringLiteral( "SDO_UTIL.PREPARE_FOR_TTS" ),
        QStringLiteral( "SDO_UTIL.RECTIFY_GEOMETRY" ),

        QStringLiteral( "SDO_UTIL.REMOVE_DUPLICATE_VERTICES" ),

        QStringLiteral( "SDO_UTIL.REVERSE_LINESTRING" ),
        QStringLiteral( "SDO_UTIL.SIMPLIFY" ),

        QStringLiteral( "SDO_UTIL.TO_GMLGEOMETRY" ),
        QStringLiteral( "SDO_UTIL.TO_WKBGEOMETRY" ),

        QStringLiteral( "SDO_UTIL.TO_WKTGEOMETRY" ),
        QStringLiteral( "SDO_UTIL.VALIDATE_WKBGEOMETRY" ),

        QStringLiteral( "SDO_UTIL.VALIDATE_WKTGEOMETRY" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Operator,
      {
        QStringLiteral( "AND" ),
        QStringLiteral( "OR" ),
        QStringLiteral( "||" ),
        QStringLiteral( "<" ),
        QStringLiteral( "<=" ),
        QStringLiteral( ">" ),
        QStringLiteral( ">=" ),
        QStringLiteral( "=" ),

        QStringLiteral( "<>" ),
        QStringLiteral( "!=" ),
        QStringLiteral( "^=" ),
        QStringLiteral( "IS" ),
        QStringLiteral( "IS NOT" ),
        QStringLiteral( "IN" ),
        QStringLiteral( "ANY" ),
        QStringLiteral( "SOME" ),

        QStringLiteral( "NOT IN" ),
        QStringLiteral( "LIKE" ),
        QStringLiteral( "GLOB" ),
        QStringLiteral( "MATCH" ),
        QStringLiteral( "REGEXP" ),

        QStringLiteral( "BETWEEN x AND y" ),
        QStringLiteral( "NOT BETWEEN x AND y" ),
        QStringLiteral( "EXISTS" ),

        QStringLiteral( "IS NULL" ),
        QStringLiteral( "IS NOT NULL" ),
        QStringLiteral( "ALL" ),
        QStringLiteral( "NOT" ),

        QStringLiteral( "CASE {column} WHEN {value} THEN {value}" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Constant,
      {
        QStringLiteral( "NULL" ),
        QStringLiteral( "FALSE" ),
        QStringLiteral( "TRUE" )
      }
    }
  };
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOracleProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  QgsPoolOracleConn pconn( QgsDataSourceUri{ uri() }.connectionInfo( false ) );
  if ( !pconn.get() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }

  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  QSqlQuery qry( *pconn.get() );
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  QgsDatabaseQueryLogWrapper logWrapper { sql, uri(), providerKey(), QStringLiteral( "QgsAbstractDatabaseProviderConnection" ), QGS_QUERY_LOG_ORIGIN };

  if ( !qry.exec( sql ) )
  {
    logWrapper.setError( qry.lastError().text() );
    throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 returned %2" )
                                          .arg( qry.lastQuery(),
                                              qry.lastError().text() ) );
  }

  if ( feedback && feedback->isCanceled() )
  {
    logWrapper.setCanceled();
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  if ( qry.isActive() )
  {
    const QSqlRecord rec { qry.record() };
    const int numCols { rec.count() };
    auto iterator = std::make_shared<QgsOracleProviderResultIterator>( numCols, qry );
    QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );
    for ( int idx = 0; idx < numCols; ++idx )
    {
      results.appendColumn( rec.field( idx ).name() );
    }
    iterator->nextRow();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );
    return results;
  }

  return QgsAbstractDatabaseProviderConnection::QueryResult();
}

QVariantList QgsOracleProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow( mNextRow );
  mNextRow = nextRowInternal();
  return currentRow;
}

bool QgsOracleProviderResultIterator::hasNextRowPrivate() const
{
  return ! mNextRow.isEmpty();
}

QVariantList QgsOracleProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mQuery.next() )
  {
    for ( int col = 0; col < mColumnCount; ++col )
    {
      row.push_back( mQuery.value( col ) );
    }
  }
  else
  {
    mQuery.finish();
  }
  return row;
}

long long QgsOracleProviderResultIterator::rowCountPrivate() const
{
  return mQuery.size();
}

void QgsOracleProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString,
    QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column and if it's not aspatial
  if ( wkbType != QgsWkbTypes::Type::Unknown &&  wkbType != QgsWkbTypes::Type::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "GEOM" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  const Qgis::VectorExportResult res = QgsOracleProvider::createEmptyLayer(
                                         newUri.uri(),
                                         fields,
                                         wkbType,
                                         srs,
                                         overwrite,
                                         map,
                                         errCause,
                                         options
                                       );
  if ( res != Qgis::VectorExportResult::Success )
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
}

QString QgsOracleProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  dsUri.setGeometryColumn( tableInfo.geometryColumn() );
  return dsUri.uri( false );
}


QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsOracleProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables;

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  const bool geometryColumnsOnly { configuration().value( "geometryColumnsOnly", false ).toBool() };
  const bool userTablesOnly { configuration().value( "userTablesOnly", false ).toBool() &&schema.isEmpty() };
  const bool onlyExistingTypes { configuration().value( "onlyExistingTypes", false ).toBool() };
  const bool aspatial { ! flags || flags.testFlag( TableFlag::Aspatial ) };

  QVector<QgsOracleLayerProperty> properties;
  const bool ok = conn->supportedLayers( properties, schema, geometryColumnsOnly, userTablesOnly, aspatial );
  if ( ! ok )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve tables: %1" ).arg( uri() ) );
  }

  for ( auto &pr : properties )
  {
    // Classify
    TableFlags prFlags;
    if ( pr.isView )
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::View );
    }
    if ( !pr.geometryColName.isEmpty() )
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Vector );
    }
    else
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Aspatial );
    }

    // Filter
    if ( flags && !( prFlags & flags ) )
      continue;

    // retrieve layer types if needed
    conn->retrieveLayerTypes( pr, dsUri.useEstimatedMetadata(), onlyExistingTypes );

    QgsAbstractDatabaseProviderConnection::TableProperty property;
    property.setFlags( prFlags );
    for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ) ; i++ )
    {
      property.addGeometryColumnType( pr.types.at( i ), QgsCoordinateReferenceSystem::fromEpsgId( pr.srids.at( i ) ) );
    }
    property.setTableName( pr.tableName );
    property.setSchema( pr.ownerName );
    property.setGeometryColumn( pr.geometryColName );
    property.setGeometryColumnCount( ( prFlags & QgsAbstractDatabaseProviderConnection::TableFlag::Aspatial ) ? 0 : 1 );
    property.setPrimaryKeyColumns( pr.isView ? pr.pkCols : conn->getPrimaryKeys( pr.ownerName, pr.tableName ) );

    tables.push_back( property );
  }

  return tables;
}

void QgsOracleProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsOracleConn::quotedIdentifier( schema ) )
                     .arg( QgsOracleConn::quotedIdentifier( name ) ) );

  executeSqlPrivate( QStringLiteral( "DELETE FROM user_sdo_geom_metadata WHERE TABLE_NAME = '%1'" )
                     .arg( name ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOracleProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql, feedback );
}

void QgsOracleProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsOracleConn::quotedIdentifier( schema ),
                           QgsOracleConn::quotedIdentifier( name ),
                           QgsOracleConn::quotedIdentifier( newName ) ) );

  executeSqlPrivate( QStringLiteral( "UPDATE user_sdo_geom_metadata SET TABLE_NAME = '%1' where TABLE_NAME = '%2'" )
                     .arg( newName, name ) );
}

void QgsOracleProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsOracleProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  const QString indexName = conn->createSpatialIndex( schema, name, options.geometryColumnName );
  if ( indexName.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Failed to create spatial index for %1.%2(%3)" ).arg( schema, name, options.geometryColumnName ) );
}

void QgsOracleProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  bool isValid;
  const QString indexName = conn->getSpatialIndexName( schema, name, geometryColumn, isValid );

  if ( indexName.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "No spatial index exists for %1.%2(%3)" ).arg( schema, name, geometryColumn ) );

  executeSqlPrivate( QStringLiteral( "DROP INDEX %1" ).arg( indexName ) );
}

bool QgsOracleProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::SpatialIndexExists );

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  bool isValid;
  conn->getSpatialIndexName( schema, name, geometryColumn, isValid );
  return isValid;
}

QIcon QgsOracleProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconOracle.svg" ) );
}

QStringList QgsOracleProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;

  QList<QVariantList> users = executeSqlPrivate( QStringLiteral( "SELECT USERNAME FROM ALL_USERS" ) ).rows();
  for ( QVariantList userInfos : users )
    schemas << userInfos.at( 0 ).toString();

  return schemas;
}
