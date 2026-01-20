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

#include "qgsapplication.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgsoracleconn.h"
#include "qgsoracleprovider.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QSqlField>
#include <QSqlRecord>

// read from QSettings and used in the provider connection
const QStringList CONFIGURATION_PARAMETERS {
  u"userTablesOnly"_s,
  u"geometryColumnsOnly"_s,
  u"allowGeometrylessTables"_s,
  u"disableInvalidGeometryHandling"_s,
  u"onlyExistingTypes"_s,
  u"includeGeoAttributes"_s,
  u"projectsInDatabase"_s,
  u"saveUsername"_s,
  u"savePassword"_s,
  u"schema"_s
};

// read from uri and used in the provider connection
const QStringList EXTRA_CONNECTION_PARAMETERS {
  u"dboptions"_s,
  u"dbworkspace"_s
};

/**
 * A light wrapper around QSqlQuery that keep a shared reference on the connection
 */
class QgsOracleQuery : public QSqlQuery
{
  public:
    explicit QgsOracleQuery( std::shared_ptr<QgsPoolOracleConn> pconn )
      : QSqlQuery( *pconn->get() )
      , mPconn( std::move( pconn ) )
    {}

  private:
    std::shared_ptr<QgsPoolOracleConn> mPconn;
};

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = u"oracle"_s;
  setUri( QgsOracleConn::connUri( name ).uri() );
  setDefaultCapabilities();

  // load existing configuration
  QgsSettings settings;
  QVariantMap configuration;
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    const QVariant v = settings.value( u"/Oracle/connections/%1/%2"_s.arg( name, p ) );
    if ( v.isValid() )
    {
      configuration.insert( p, v );
    }
  }
  setConfiguration( configuration );
}

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = u"oracle"_s;
  setDefaultCapabilities();

  // Additional connection information
  const QgsDataSourceUri inputUri( uri );
  QgsDataSourceUri currentUri { QgsDataSourceUri( uri ).connectionInfo( false ) };

  if ( inputUri.hasParam( u"estimatedMetadata"_s ) )
  {
    currentUri.setUseEstimatedMetadata( inputUri.param( u"estimatedMetadata"_s ) == "true"_L1 || inputUri.param( u"estimatedMetadata"_s ) == '1' );
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
  mCapabilities = {
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
  mGeometryColumnCapabilities = {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::SinglePart,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities = {
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
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : u"SELECT * FROM %1"_s.arg( tUri.quotedTablename() );
  return options;
}

QgsVectorLayer *QgsOracleProviderConnection::createSqlVectorLayer( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options ) const
{
  // Precondition
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  const QString optionsSql { sanitizeSqlForQueryLayer( options.sql ) };

  QgsDataSourceUri tUri( uri() );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );
    tUri.setTable( u"(%1)"_s.arg( optionsSql ) );
  }
  else
  {
    // Disable when there is no pk
    tUri.setUseEstimatedMetadata( false );
    int pkId { 0 };
    while ( optionsSql.contains( u"qgis_generated_uid_%1_"_s.arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId++;
    }
    tUri.setKeyColumn( u"qgis_generated_uid_%1_"_s.arg( pkId ) );

    int sqlId { 0 };
    while ( optionsSql.contains( u"qgis_generated_subq_%1_"_s.arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId++;
    }
    tUri.setTable( u"(SELECT row_number() over (ORDER BY NULL) AS qgis_generated_uid_%1_, qgis_generated_subq_%3_.* FROM (%2\n) qgis_generated_subq_%3_\n)"_s.arg( QString::number( pkId ), optionsSql, QString::number( sqlId ) ) );
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  auto vl = std::make_unique<QgsVectorLayer>( tUri.uri( false ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey() );

  // Try to guess the geometry and srid
  if ( !vl->isValid() )
  {
    const QString limit { QgsDataSourceUri( uri() ).useEstimatedMetadata() ? u"AND ROWNUM < 100"_s : QString() };
    const QString sql { QStringLiteral( R"(
      SELECT DISTINCT a.%1.SDO_GTYPE As gtype,
                            a.%1.SDO_SRID
            FROM (%2) a
            WHERE a.%1 IS NOT NULL %3
            ORDER BY a.%1.SDO_GTYPE
    )" )
                          .arg( options.geometryColumn, optionsSql, limit ) };
    const QList<QList<QVariant>> candidates { executeSql( sql ) };
    for ( const QList<QVariant> &row : std::as_const( candidates ) )
    {
      bool ok;
      const int type { row[0].toInt( &ok ) };
      if ( ok )
      {
        const int srid { row[1].toInt( &ok ) };
        if ( ok )
        {
          Qgis::WkbType geomType { Qgis::WkbType::Unknown };

          switch ( type )
          {
            case 2001:
              geomType = Qgis::WkbType::Point;
              break;
            case 2002:
              geomType = Qgis::WkbType::LineString;
              break;
            case 2003:
              geomType = Qgis::WkbType::Polygon;
              break;
            // Note: 2004 is missing
            case 2005:
              geomType = Qgis::WkbType::MultiPoint;
              break;
            case 2006:
              geomType = Qgis::WkbType::MultiLineString;
              break;
            case 2007:
              geomType = Qgis::WkbType::MultiPolygon;
              break;
            // 3K...
            case 3001:
              geomType = Qgis::WkbType::PointZ;
              break;
            case 3002:
              geomType = Qgis::WkbType::LineStringZ;
              break;
            case 3003:
              geomType = Qgis::WkbType::PolygonZ;
              break;
            // Note: 3004 is missing
            case 3005:
              geomType = Qgis::WkbType::MultiPointZ;
              break;
            case 3006:
              geomType = Qgis::WkbType::MultiLineStringZ;
              break;
            case 3007:
              geomType = Qgis::WkbType::MultiPolygonZ;
              break;
            default:
              geomType = Qgis::WkbType::Unknown;
          }
          if ( geomType != Qgis::WkbType::Unknown )
          {
            tUri.setSrid( QString::number( srid ) );
            tUri.setWkbType( geomType );
            vl = std::make_unique<QgsVectorLayer>( tUri.uri(), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey() );
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

Qgis::DatabaseProviderTableImportCapabilities QgsOracleProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName | Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName;
}

QString QgsOracleProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"id"_s;
}

void QgsOracleProviderConnection::store( const QString &name ) const
{
  QString baseKey = u"/Oracle/connections/"_s;
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
    else
    {
      settings.remove( param );
    }
  }

  // From configuration
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
    else
    {
      settings.remove( p );
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
  QgsPoolOracleConn conn( QgsDataSourceUri { uri() }.connectionInfo( false ) );
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

QString QgsOracleProviderConnection::defaultGeometryColumnName() const
{
  return u"GEOM"_s;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsOracleProviderConnection::sqlDictionary()
{
  return {
    { Qgis::SqlKeywordCategory::Keyword,
      { // From: http://docs.oracle.com/cd/B19306_01/server.102/b14200/ap_keywd.htm
        u"ACCESS"_s,
        u"ADD"_s,
        u"ALL"_s,
        u"ALTER"_s,
        u"AND"_s,
        u"ANY"_s,
        u"AS"_s,
        u"ASC"_s,

        u"AUDIT"_s,
        u"BETWEEN"_s,
        u"BY"_s,
        u"CHAR"_s,
        u"CHECK"_s,
        u"CLUSTER"_s,
        u"COLUMN"_s,

        u"COMMENT"_s,
        u"COMPRESS"_s,
        u"CONNECT"_s,
        u"CREATE"_s,
        u"CURRENT"_s,
        u"DATE"_s,

        u"DECIMAL"_s,
        u"DEFAULT"_s,
        u"DELETE"_s,
        u"DESC"_s,
        u"DISTINCT"_s,
        u"DROP"_s,

        u"ELSE"_s,
        u"EXCLUSIVE"_s,
        u"EXISTS"_s,
        u"FILE"_s,
        u"FLOAT"_s,
        u"FOR"_s,
        u"FROM"_s,

        u"GRANT"_s,
        u"GROUP"_s,
        u"HAVING"_s,
        u"IDENTIFIED"_s,
        u"IMMEDIATE"_s,
        u"IN"_s,

        u"INCREMENT"_s,
        u"INDEX"_s,
        u"INITIAL"_s,
        u"INSERT"_s,
        u"INTEGER"_s,
        u"INTERSECT"_s,

        u"INTO"_s,
        u"IS"_s,
        u"LEVEL"_s,
        u"LIKE"_s,
        u"LOCK"_s,
        u"LONG"_s,
        u"MAXEXTENTS"_s,

        u"MINUS"_s,
        u"MLSLABEL"_s,
        u"MODE"_s,
        u"MODIFY"_s,
        u"NOAUDIT"_s,
        u"NOCOMPRESS"_s,

        u"NOT"_s,
        u"NOWAIT"_s,
        u"NULL"_s,
        u"NUMBER"_s,
        u"OF"_s,
        u"OFFLINE"_s,
        u"ON"_s,

        u"ONLINE"_s,
        u"OPTION"_s,
        u"OR"_s,
        u"ORDER"_s,
        u"PCTFREE"_s,
        u"PRIOR"_s,

        u"PRIVILEGES"_s,
        u"PUBLIC"_s,
        u"RAW"_s,
        u"RENAME"_s,
        u"RESOURCE"_s,
        u"REVOKE"_s,

        u"ROW"_s,
        u"ROWID"_s,
        u"ROWNUM"_s,
        u"ROWS"_s,
        u"SELECT"_s,
        u"SESSION"_s,
        u"SET"_s,

        u"SHARE"_s,
        u"SIZE"_s,
        u"SMALLINT"_s,
        u"START"_s,
        u"SUCCESSFUL"_s,
        u"SYNONYM"_s,

        u"SYSDATE"_s,
        u"TABLE"_s,
        u"THEN"_s,
        u"TO"_s,
        u"TRIGGER"_s,
        u"UID"_s,
        u"UNION"_s,

        u"UNIQUE"_s,
        u"UPDATE"_s,
        u"USER"_s,
        u"VALIDATE"_s,
        u"VALUES"_s,
        u"VARCHAR"_s,

        u"VARCHAR2"_s,
        u"VIEW"_s,
        u"WHENEVER"_s,
        u"WHERE"_s,
        u"WITH"_s,

        // From http://docs.oracle.com/cd/B13789_01/appdev.101/a42525/apb.htm
        u"ADMIN"_s,
        u"CURSOR"_s,
        u"FOUND"_s,
        u"MOUNT"_s,
        u"AFTER"_s,
        u"CYCLE"_s,
        u"FUNCTION"_s,

        u"NEXT"_s,
        u"ALLOCATE"_s,
        u"DATABASE"_s,
        u"GO"_s,
        u"NEW"_s,
        u"ANALYZE"_s,

        u"DATAFILE"_s,
        u"GOTO"_s,
        u"NOARCHIVELOG"_s,
        u"ARCHIVE"_s,
        u"DBA"_s,
        u"GROUPS"_s,

        u"NOCACHE"_s,
        u"ARCHIVELOG"_s,
        u"DEC"_s,
        u"INCLUDING"_s,
        u"NOCYCLE"_s,

        u"AUTHORIZATION"_s,
        u"DECLARE"_s,
        u"INDICATOR"_s,
        u"NOMAXVALUE"_s,
        u"AVG"_s,

        u"DISABLE"_s,
        u"INITRANS"_s,
        u"NOMINVALUE"_s,
        u"BACKUP"_s,
        u"DISMOUNT"_s,

        u"INSTANCE"_s,
        u"NONE"_s,
        u"BEGIN"_s,
        u"DOUBLE"_s,
        u"INT"_s,
        u"NOORDER"_s,
        u"BECOME"_s,

        u"DUMP"_s,
        u"KEY"_s,
        u"NORESETLOGS"_s,
        u"BEFORE"_s,
        u"EACH"_s,
        u"LANGUAGE"_s,

        u"NORMAL"_s,
        u"BLOCK"_s,
        u"ENABLE"_s,
        u"LAYER"_s,
        u"NOSORT"_s,
        u"BODY"_s,
        u"END"_s,

        u"LINK"_s,
        u"NUMERIC"_s,
        u"CACHE"_s,
        u"ESCAPE"_s,
        u"LISTS"_s,
        u"OFF"_s,
        u"CANCEL"_s,

        u"EVENTS"_s,
        u"LOGFILE"_s,
        u"OLD"_s,
        u"CASCADE"_s,
        u"EXCEPT"_s,
        u"MANAGE"_s,
        u"ONLY"_s,

        u"CHANGE"_s,
        u"EXCEPTIONS"_s,
        u"MANUAL"_s,
        u"OPEN"_s,
        u"CHARACTER"_s,
        u"EXEC"_s,

        u"MAX"_s,
        u"OPTIMAL"_s,
        u"CHECKPOINT"_s,
        u"EXPLAIN"_s,
        u"MAXDATAFILES"_s,
        u"OWN"_s,

        u"CLOSE"_s,
        u"EXECUTE"_s,
        u"MAXINSTANCES"_s,
        u"PACKAGE"_s,
        u"COBOL"_s,
        u"EXTENT"_s,

        u"MAXLOGFILES"_s,
        u"PARALLEL"_s,
        u"COMMIT"_s,
        u"EXTERNALLY"_s,

        u"MAXLOGHISTORY"_s,
        u"PCTINCREASE"_s,
        u"COMPILE"_s,
        u"FETCH"_s,

        u"MAXLOGMEMBERS"_s,
        u"PCTUSED"_s,
        u"CONSTRAINT"_s,
        u"FLUSH"_s,
        u"MAXTRANS"_s,

        u"PLAN"_s,
        u"CONSTRAINTS"_s,
        u"FREELIST"_s,
        u"MAXVALUE"_s,
        u"PLI"_s,
        u"CONTENTS"_s,

        u"FREELISTS"_s,
        u"MIN"_s,
        u"PRECISION"_s,
        u"CONTINUE"_s,
        u"FORCE"_s,

        u"MINEXTENTS"_s,
        u"PRIMARY"_s,
        u"CONTROLFILE"_s,
        u"FOREIGN"_s,
        u"MINVALUE"_s,

        u"PRIVATE"_s,
        u"COUNT"_s,
        u"FORTRAN"_s,
        u"MODULE"_s,
        u"PROCEDURE"_s,
        u"PROFILE"_s,

        u"SAVEPOINT"_s,
        u"SQLSTATE"_s,
        u"TRACING"_s,
        u"QUOTA"_s,
        u"SCHEMA"_s,

        u"STATEMENT_ID"_s,
        u"TRANSACTION"_s,
        u"READ"_s,
        u"SCN"_s,
        u"STATISTICS"_s,

        u"TRIGGERS"_s,
        u"REAL"_s,
        u"SECTION"_s,
        u"STOP"_s,
        u"TRUNCATE"_s,
        u"RECOVER"_s,

        u"SEGMENT"_s,
        u"STORAGE"_s,
        u"UNDER"_s,
        u"REFERENCES"_s,
        u"SEQUENCE"_s,
        u"SUM"_s,

        u"UNLIMITED"_s,
        u"REFERENCING"_s,
        u"SHARED"_s,
        u"SWITCH"_s,
        u"UNTIL"_s,

        u"RESETLOGS"_s,
        u"SNAPSHOT"_s,
        u"SYSTEM"_s,
        u"USE"_s,
        u"RESTRICTED"_s,
        u"SOME"_s,

        u"TABLES"_s,
        u"USING"_s,
        u"REUSE"_s,
        u"SORT"_s,
        u"TABLESPACE"_s,
        u"WHEN"_s,
        u"ROLE"_s,

        u"SQL"_s,
        u"TEMPORARY"_s,
        u"WRITE"_s,
        u"ROLES"_s,
        u"SQLCODE"_s,
        u"THREAD"_s,
        u"WORK"_s,

        u"ROLLBACK"_s,
        u"SQLERROR"_s,
        u"TIME"_s,
        u"ABORT"_s,
        u"BETWEEN"_s,
        u"CRASH"_s,

        u"DIGITS"_s,
        u"ACCEPT"_s,
        u"BINARY_INTEGER"_s,
        u"CREATE"_s,
        u"DISPOSE"_s,

        u"ACCESS"_s,
        u"BODY"_s,
        u"CURRENT"_s,
        u"DISTINCT"_s,
        u"ADD"_s,
        u"BOOLEAN"_s,

        u"CURRVAL"_s,
        u"DO"_s,
        u"ALL"_s,
        u"BY"_s,
        u"CURSOR"_s,
        u"DROP"_s,
        u"ALTER"_s,
        u"CASE"_s,

        u"DATABASE"_s,
        u"ELSE"_s,
        u"AND"_s,
        u"CHAR"_s,
        u"DATA_BASE"_s,
        u"ELSIF"_s,
        u"ANY"_s,

        u"CHAR_BASE"_s,
        u"DATE"_s,
        u"END"_s,
        u"ARRAY"_s,
        u"CHECK"_s,
        u"DBA"_s,
        u"ENTRY"_s,

        u"ARRAYLEN"_s,
        u"CLOSE"_s,
        u"DEBUGOFF"_s,
        u"EXCEPTION"_s,
        u"AS"_s,
        u"CLUSTER"_s,

        u"DEBUGON"_s,
        u"EXCEPTION_INIT"_s,
        u"ASC"_s,
        u"CLUSTERS"_s,
        u"DECLARE"_s,

        u"EXISTS"_s,
        u"ASSERT"_s,
        u"COLAUTH"_s,
        u"DECIMAL"_s,
        u"EXIT"_s,
        u"ASSIGN"_s,

        u"COLUMNS"_s,
        u"DEFAULT"_s,
        u"FALSE"_s,
        u"AT"_s,
        u"COMMIT"_s,
        u"DEFINITION"_s,

        u"FETCH"_s,
        u"AUTHORIZATION"_s,
        u"COMPRESS"_s,
        u"DELAY"_s,
        u"FLOAT"_s,
        u"AVG"_s,

        u"CONNECT"_s,
        u"DELETE"_s,
        u"FOR"_s,
        u"BASE_TABLE"_s,
        u"CONSTANT"_s,
        u"DELTA"_s,

        u"FORM"_s,
        u"BEGIN"_s,
        u"COUNT"_s,
        u"DESC"_s,
        u"FROM"_s,
        u"FUNCTION"_s,
        u"NEW"_s,

        u"RELEASE"_s,
        u"SUM"_s,
        u"GENERIC"_s,
        u"NEXTVAL"_s,
        u"REMR"_s,
        u"TABAUTH"_s,
        u"GOTO"_s,

        u"NOCOMPRESS"_s,
        u"RENAME"_s,
        u"TABLE"_s,
        u"GRANT"_s,
        u"NOT"_s,
        u"RESOURCE"_s,

        u"TABLES"_s,
        u"GROUP"_s,
        u"NULL"_s,
        u"RETURN"_s,
        u"TASK"_s,
        u"HAVING"_s,
        u"NUMBER"_s,

        u"REVERSE"_s,
        u"TERMINATE"_s,
        u"IDENTIFIED"_s,
        u"NUMBER_BASE"_s,
        u"REVOKE"_s,

        u"THEN"_s,
        u"IF"_s,
        u"OF"_s,
        u"ROLLBACK"_s,
        u"TO"_s,
        u"IN"_s,
        u"ON"_s,
        u"ROWID"_s,
        u"TRUE"_s,

        u"INDEX"_s,
        u"OPEN"_s,
        u"ROWLABEL"_s,
        u"TYPE"_s,
        u"INDEXES"_s,
        u"OPTION"_s,

        u"ROWNUM"_s,
        u"UNION"_s,
        u"INDICATOR"_s,
        u"OR"_s,
        u"ROWTYPE"_s,
        u"UNIQUE"_s,

        u"INSERT"_s,
        u"ORDER"_s,
        u"RUN"_s,
        u"UPDATE"_s,
        u"INTEGER"_s,
        u"OTHERS"_s,

        u"SAVEPOINT"_s,
        u"USE"_s,
        u"INTERSECT"_s,
        u"OUT"_s,
        u"SCHEMA"_s,
        u"VALUES"_s,

        u"INTO"_s,
        u"PACKAGE"_s,
        u"SELECT"_s,
        u"VARCHAR"_s,
        u"IS"_s,
        u"PARTITION"_s,

        u"SEPARATE"_s,
        u"VARCHAR2"_s,
        u"LEVEL"_s,
        u"PCTFREE"_s,
        u"SET"_s,
        u"VARIANCE"_s,

        u"LIKE"_s,
        u"POSITIVE"_s,
        u"SIZE"_s,
        u"VIEW"_s,
        u"LIMITED"_s,
        u"PRAGMA"_s,

        u"SMALLINT"_s,
        u"VIEWS"_s,
        u"LOOP"_s,
        u"PRIOR"_s,
        u"SPACE"_s,
        u"WHEN"_s,
        u"MAX"_s,

        u"PRIVATE"_s,
        u"SQL"_s,
        u"WHERE"_s,
        u"MIN"_s,
        u"PROCEDURE"_s,
        u"SQLCODE"_s,
        u"WHILE"_s,

        u"MINUS"_s,
        u"PUBLIC"_s,
        u"SQLERRM"_s,
        u"WITH"_s,
        u"MLSLABEL"_s,
        u"RAISE"_s,

        u"START"_s,
        u"WORK"_s,
        u"MOD"_s,
        u"RANGE"_s,
        u"STATEMENT"_s,
        u"XOR"_s,
        u"MODE"_s,

        u"REAL"_s,
        u"STDDEV"_s,
        u"NATURAL"_s,
        u"RECORD"_s,
        u"SUBTYPE"_s
      }
    },
    { Qgis::SqlKeywordCategory::Function,
      { // From: https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions001.htm
        u"CAST"_s,
        u"COALESCE"_s,
        u"DECODE"_s,
        u"GREATEST"_s,
        u"LEAST"_s,
        u"LNNVL"_s,

        u"NULLIF"_s,
        u"NVL"_s,
        u"NVL2"_s,
        u"SET"_s,
        u"UID"_s,
        u"USER"_s,
        u"USERENV"_s
      }
    },
    { Qgis::SqlKeywordCategory::Math,
      { u"ABS"_s,
        u"ACOS"_s,
        u"ASIN"_s,
        u"ATAN"_s,
        u"ATAN2"_s,
        u"BITAND"_s,
        u"CEIL"_s,
        u"COS"_s,

        u"COSH"_s,
        u"EXP"_s,
        u"FLOOR"_s,
        u"LN"_s,
        u"LOG"_s,
        u"MOD"_s,
        u"NANVL"_s,
        u"POWER"_s,

        u"REMAINDER"_s,
        u"ROUND"_s,
        u"SIGN"_s,
        u"SIN"_s,
        u"SINH"_s,
        u"SQRT"_s,
        u"TAN"_s,

        u"TANH"_s,
        u"TRUNC"_s,
        u"WIDTH_BUCKET"_s
      }
    },
    { Qgis::SqlKeywordCategory::String,
      { u"CHR"_s,
        u"CONCAT"_s,
        u"INITCAP"_s,
        u"LOWER"_s,
        u"LPAD"_s,
        u"LTRIM"_s,
        u"NLS_INITCAP"_s,

        u"NLS_LOWER"_s,
        u"NLSSORT"_s,
        u"NLS_UPPER"_s,
        u"REGEXP_REPLACE"_s,
        u"REGEXP_SUBSTR"_s,

        u"REPLACE"_s,
        u"RPAD"_s,
        u"RTRIM"_s,
        u"SOUNDEX"_s,
        u"SUBSTR"_s,
        u"TRANSLATE"_s,
        u"TREAT"_s,

        u"TRIM"_s,
        u"UPPER"_s,
        u"ASCII"_s,
        u"INSTR"_s,
        u"LENGTH"_s,
        u"REGEXP_INSTR"_s
      }
    },
    { Qgis::SqlKeywordCategory::Aggregate,
      { u"AVG"_s,
        u"COLLECT"_s,
        u"CORR"_s,
        u"COUNT"_s,
        u"COVAR_POP"_s,
        u"COVAR_SAMP"_s,
        u"CUME_DIST"_s,

        u"DENSE_RANK"_s,
        u"FIRST"_s,
        u"GROUP_ID"_s,
        u"GROUPING"_s,
        u"GROUPING_ID"_s,

        u"LAST"_s,
        u"MAX"_s,
        u"MEDIAN"_s,
        u"MIN"_s,
        u"PERCENTILE_CONT"_s,

        u"PERCENTILE_DISC"_s,
        u"PERCENT_RANK"_s,
        u"RANK"_s,

        u"STATS_BINOMIAL_TEST"_s,
        u"STATS_CROSSTAB"_s,
        u"STATS_F_TEST"_s,

        u"STATS_KS_TEST"_s,
        u"STATS_MODE"_s,
        u"STATS_MW_TEST"_s,

        u"STATS_ONE_WAY_ANOVA"_s,
        u"STATS_WSR_TEST"_s,
        u"STDDEV"_s,

        u"STDDEV_POP"_s,
        u"STDDEV_SAMP"_s,
        u"SUM"_s,
        u"SYS_XMLAGG"_s,
        u"VAR_POP"_s,

        u"VAR_SAMP"_s,
        u"VARIANCE"_s,
        u"XMLAGG"_s
      }
    },
    { Qgis::SqlKeywordCategory::Geospatial,
      { // From http://docs.oracle.com/cd/B19306_01/appdev.102/b14255/toc.htm
        // Spatial operators
        u"SDO_ANYINTERACT"_s,
        u"SDO_CONTAINS"_s,
        u"SDO_COVEREDBY"_s,
        u"SDO_COVERS"_s,

        u"SDO_EQUAL"_s,
        u"SDO_FILTER"_s,
        u"SDO_INSIDE"_s,
        u"SDO_JOIN"_s,
        u"SDO_NN"_s,

        u"SDO_NN_DISTANCE"_s,
        u"SDO_ON"_s,
        u"SDO_OVERLAPBDYDISJOINT"_s,

        u"SDO_OVERLAPBDYINTERSECT"_s,
        u"SDO_OVERLAPS"_s,
        u"SDO_RELATE"_s,

        u"SDO_TOUCH"_s,
        u"SDO_WITHIN_DISTANCE"_s,

        // SPATIAL AGGREGATE FUNCTIONS
        u"SDO_AGGR_CENTROID"_s,
        u"SDO_AGGR_CONCAT_LINES"_s,

        u"SDO_AGGR_CONVEXHULL"_s,
        u"SDO_AGGR_LRS_CONCAT"_s,
        u"SDO_AGGR_MBR"_s,

        u"SDO_AGGR_UNION"_s,

        // COORDINATE SYSTEM TRANSFORMATION (SDO_CS)
        u"SDO_CS.ADD_PREFERENCE_FOR_OP"_s,
        u"SDO_CS.CONVERT_NADCON_TO_XML"_s,

        u"SDO_CS.CONVERT_NTV2_TO_XML"_s,
        u"SDO_CS.CONVERT_XML_TO_NADCON"_s,

        u"SDO_CS.CONVERT_XML_TO_NTV2"_s,
        u"SDO_CS.CREATE_CONCATENATED_OP"_s,

        u"SDO_CS.CREATE_OBVIOUS_EPSG_RULES"_s,

        u"SDO_CS.CREATE_PREF_CONCATENATED_OP"_s,

        u"SDO_CS.DELETE_ALL_EPSG_RULES"_s,
        u"SDO_CS.DELETE_OP"_s,

        u"SDO_CS.DETERMINE_CHAIN"_s,
        u"SDO_CS.DETERMINE_DEFAULT_CHAIN"_s,

        u"SDO_CS.FIND_GEOG_CRS"_s,
        u"SDO_CS.FIND_PROJ_CRS"_s,

        u"SDO_CS.FROM_OGC_SIMPLEFEATURE_SRS"_s,
        u"SDO_CS.FROM_USNG"_s,

        u"SDO_CS.MAP_EPSG_SRID_TO_ORACLE"_s,

        u"SDO_CS.MAP_ORACLE_SRID_TO_EPSG"_s,

        u"SDO_CS.REVOKE_PREFERENCE_FOR_OP"_s,

        u"SDO_CS.TO_OGC_SIMPLEFEATURE_SRS"_s,
        u"SDO_CS.TO_USNG"_s,

        u"SDO_CS.TRANSFORM"_s,
        u"SDO_CS.TRANSFORM_LAYER"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_ALL_EPSG_CRS"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_CRS"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_DATUM"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_ELLIPS"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_OP"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_PARAM"_s,

        u"SDO_CS.UPDATE_WKTS_FOR_EPSG_PM"_s,
        u"SDO_CS.VALIDATE_WKT"_s,

        u"SDO_CS.VIEWPORT_TRANSFORM"_s,

        // GEOCODING (SDO_GCDR)
        u"SDO_GCDR.GEOCODE"_s,
        u"SDO_GCDR.GEOCODE_ADDR"_s,

        u"SDO_GCDR.GEOCODE_ADDR_ALL"_s,
        u"SDO_GCDR.GEOCODE_ALL"_s,

        u"SDO_GCDR.GEOCODE_AS_GEOMETRY"_s,
        u"SDO_GCDR.REVERSE_GEOCODE"_s,

        // GEOMETRY (SDO_GEOM)
        u"SDO_GEOM.RELATE"_s,
        u"SDO_GEOM.SDO_ARC_DENSIFY"_s,

        u"SDO_GEOM.SDO_AREA"_s,
        u"SDO_GEOM.SDO_BUFFER"_s,

        u"SDO_GEOM.SDO_CENTROID"_s,
        u"SDO_GEOM.SDO_CONVEXHULL"_s,

        u"SDO_GEOM.SDO_DIFFERENCE"_s,
        u"SDO_GEOM.SDO_DISTANCE"_s,

        u"SDO_GEOM.SDO_INTERSECTION"_s,
        u"SDO_GEOM.SDO_LENGTH"_s,

        u"SDO_GEOM.SDO_MAX_MBR_ORDINATE"_s,
        u"SDO_GEOM.SDO_MBR"_s,

        u"SDO_GEOM.SDO_MIN_MBR_ORDINATE"_s,
        u"SDO_GEOM.SDO_POINTONSURFACE"_s,

        u"SDO_GEOM.SDO_UNION"_s,
        u"SDO_GEOM.SDO_XOR"_s,

        u"SDO_GEOM.VALIDATE_GEOMETRY_WITH_CONTEXT"_s,

        u"SDO_GEOM.VALIDATE_LAYER_WITH_CONTEXT"_s,

        u"SDO_GEOM.WITHIN_DISTANCE"_s,

        // LINEAR REFERENCING SYSTEM (SDO_LRS)
        u"SDO_LRS.CLIP_GEOM_SEGMENT"_s,
        u"SDO_LRS.CONCATENATE_GEOM_SEGMENTS"_s,

        u"SDO_LRS.CONNECTED_GEOM_SEGMENTS"_s,

        u"SDO_LRS.CONVERT_TO_LRS_DIM_ARRAY"_s,
        u"SDO_LRS.CONVERT_TO_LRS_GEOM"_s,

        u"SDO_LRS.CONVERT_TO_LRS_LAYER"_s,

        u"SDO_LRS.CONVERT_TO_STD_DIM_ARRAY"_s,
        u"SDO_LRS.CONVERT_TO_STD_GEOM"_s,

        u"SDO_LRS.CONVERT_TO_STD_LAYER"_s,
        u"SDO_LRS.DEFINE_GEOM_SEGMENT"_s,

        u"SDO_LRS.DYNAMIC_SEGMENT"_s,
        u"SDO_LRS.FIND_LRS_DIM_POS"_s,

        u"SDO_LRS.FIND_MEASURE"_s,
        u"SDO_LRS.FIND_OFFSET"_s,

        u"SDO_LRS.GEOM_SEGMENT_END_MEASURE"_s,
        u"SDO_LRS.GEOM_SEGMENT_END_PT"_s,

        u"SDO_LRS.GEOM_SEGMENT_LENGTH"_s,

        u"SDO_LRS.GEOM_SEGMENT_START_MEASURE"_s,

        u"SDO_LRS.GEOM_SEGMENT_START_PT"_s,
        u"SDO_LRS.GET_MEASURE"_s,

        u"SDO_LRS.GET_NEXT_SHAPE_PT"_s,
        u"SDO_LRS.GET_NEXT_SHAPE_PT_MEASURE"_s,

        u"SDO_LRS.GET_PREV_SHAPE_PT"_s,
        u"SDO_LRS.GET_PREV_SHAPE_PT_MEASURE"_s,

        u"SDO_LRS.IS_GEOM_SEGMENT_DEFINED"_s,

        u"SDO_LRS.IS_MEASURE_DECREASING"_s,
        u"SDO_LRS.IS_MEASURE_INCREASING"_s,

        u"SDO_LRS.IS_SHAPE_PT_MEASURE"_s,
        u"SDO_LRS.LOCATE_PT"_s,

        u"SDO_LRS.LRS_INTERSECTION"_s,
        u"SDO_LRS.MEASURE_RANGE"_s,

        u"SDO_LRS.MEASURE_TO_PERCENTAGE"_s,
        u"SDO_LRS.OFFSET_GEOM_SEGMENT"_s,

        u"SDO_LRS.PERCENTAGE_TO_MEASURE"_s,
        u"SDO_LRS.PROJECT_PT"_s,

        u"SDO_LRS.REDEFINE_GEOM_SEGMENT"_s,
        u"SDO_LRS.RESET_MEASURE"_s,

        u"SDO_LRS.REVERSE_GEOMETRY"_s,
        u"SDO_LRS.REVERSE_MEASURE"_s,

        u"SDO_LRS.SET_PT_MEASURE"_s,
        u"SDO_LRS.SPLIT_GEOM_SEGMENT"_s,

        u"SDO_LRS.TRANSLATE_MEASURE"_s,
        u"SDO_LRS.VALID_GEOM_SEGMENT"_s,

        u"SDO_LRS.VALID_LRS_PT"_s,
        u"SDO_LRS.VALID_MEASURE"_s,

        u"SDO_LRS.VALIDATE_LRS_GEOMETRY"_s,

        // SDO_MIGRATE
        u"SDO_MIGRATE.TO_CURRENT"_s,

        // SPATIAL ANALYSIS AND MINING (SDO_SAM)
        u"SDO_SAM.AGGREGATES_FOR_GEOMETRY"_s,
        u"SDO_SAM.AGGREGATES_FOR_LAYER"_s,

        u"SDO_SAM.BIN_GEOMETRY"_s,
        u"SDO_SAM.BIN_LAYER"_s,

        u"SDO_SAM.COLOCATED_REFERENCE_FEATURES"_s,

        u"SDO_SAM.SIMPLIFY_GEOMETRY"_s,
        u"SDO_SAM.SIMPLIFY_LAYER"_s,

        u"SDO_SAM.SPATIAL_CLUSTERS"_s,
        u"SDO_SAM.TILED_AGGREGATES"_s,

        u"SDO_SAM.TILED_BINS"_s,

        // TUNING (SDO_TUNE)
        u"SDO_TUNE.AVERAGE_MBR"_s,
        u"SDO_TUNE.ESTIMATE_RTREE_INDEX_SIZE"_s,

        u"SDO_TUNE.EXTENT_OF"_s,
        u"SDO_TUNE.MIX_INFO"_s,

        u"SDO_TUNE.QUALITY_DEGRADATION"_s,

        // UTILITY (SDO_UTIL)
        u"SDO_UTIL.APPEND"_s,
        u"SDO_UTIL.CIRCLE_POLYGON"_s,

        u"SDO_UTIL.CONCAT_LINES"_s,
        u"SDO_UTIL.CONVERT_UNIT"_s,

        u"SDO_UTIL.ELLIPSE_POLYGON"_s,
        u"SDO_UTIL.EXTRACT"_s,

        u"SDO_UTIL.FROM_WKBGEOMETRY"_s,
        u"SDO_UTIL.FROM_WKTGEOMETRY"_s,

        u"SDO_UTIL.GETNUMELEM"_s,
        u"SDO_UTIL.GETNUMVERTICES"_s,

        u"SDO_UTIL.GETVERTICES"_s,
        u"SDO_UTIL.INITIALIZE_INDEXES_FOR_TTS"_s,

        u"SDO_UTIL.POINT_AT_BEARING"_s,
        u"SDO_UTIL.POLYGONTOLINE"_s,

        u"SDO_UTIL.PREPARE_FOR_TTS"_s,
        u"SDO_UTIL.RECTIFY_GEOMETRY"_s,

        u"SDO_UTIL.REMOVE_DUPLICATE_VERTICES"_s,

        u"SDO_UTIL.REVERSE_LINESTRING"_s,
        u"SDO_UTIL.SIMPLIFY"_s,

        u"SDO_UTIL.TO_GMLGEOMETRY"_s,
        u"SDO_UTIL.TO_WKBGEOMETRY"_s,

        u"SDO_UTIL.TO_WKTGEOMETRY"_s,
        u"SDO_UTIL.VALIDATE_WKBGEOMETRY"_s,

        u"SDO_UTIL.VALIDATE_WKTGEOMETRY"_s
      }
    },
    { Qgis::SqlKeywordCategory::Operator,
      { u"AND"_s,
        u"OR"_s,
        u"||"_s,
        u"<"_s,
        u"<="_s,
        u">"_s,
        u">="_s,
        u"="_s,

        u"<>"_s,
        u"!="_s,
        u"^="_s,
        u"IS"_s,
        u"IS NOT"_s,
        u"IN"_s,
        u"ANY"_s,
        u"SOME"_s,

        u"NOT IN"_s,
        u"LIKE"_s,
        u"GLOB"_s,
        u"MATCH"_s,
        u"REGEXP"_s,

        u"BETWEEN x AND y"_s,
        u"NOT BETWEEN x AND y"_s,
        u"EXISTS"_s,

        u"IS NULL"_s,
        u"IS NOT NULL"_s,
        u"ALL"_s,
        u"NOT"_s,

        u"CASE {column} WHEN {value} THEN {value}"_s
      }
    },
    { Qgis::SqlKeywordCategory::Constant,
      { u"NULL"_s,
        u"FALSE"_s,
        u"TRUE"_s
      }
    }
  };
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOracleProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  auto pconn = std::make_shared<QgsPoolOracleConn>( QgsDataSourceUri { uri() }.connectionInfo( false ) );
  if ( !pconn->get() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }

  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  auto qry = std::make_unique<QgsOracleQuery>( pconn );
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  QgsDatabaseQueryLogWrapper logWrapper { sql, uri(), providerKey(), u"QgsAbstractDatabaseProviderConnection"_s, QGS_QUERY_LOG_ORIGIN };

  if ( !qry->exec( sql ) )
  {
    logWrapper.setError( qry->lastError().text() );
    throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 returned %2" )
                                            .arg( qry->lastQuery(), qry->lastError().text() ) );
  }

  if ( feedback && feedback->isCanceled() )
  {
    logWrapper.setCanceled();
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  if ( qry->isActive() )
  {
    const QSqlRecord rec { qry->record() };
    const int numCols { rec.count() };
    auto iterator = std::make_shared<QgsOracleProviderResultIterator>( numCols, std::move( qry ) );
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

QgsOracleProviderResultIterator::QgsOracleProviderResultIterator( int columnCount, std::unique_ptr<QgsOracleQuery> query )
  : mColumnCount( columnCount )
  , mQuery( std::move( query ) )
{
}

QVariantList QgsOracleProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow( mNextRow );
  mNextRow = nextRowInternal();
  return currentRow;
}

bool QgsOracleProviderResultIterator::hasNextRowPrivate() const
{
  return !mNextRow.isEmpty();
}

QVariantList QgsOracleProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mQuery->next() )
  {
    for ( int col = 0; col < mColumnCount; ++col )
    {
      row.push_back( mQuery->value( col ) );
    }
  }
  else
  {
    mQuery->finish();
  }
  return row;
}

long long QgsOracleProviderResultIterator::rowCountPrivate() const
{
  return mQuery->size();
}

void QgsOracleProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column and if it's not aspatial
  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( u"geometryColumn"_s, u"GEOM"_s ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  QString createdLayerUri;
  const Qgis::VectorExportResult res = QgsOracleProvider::createEmptyLayer(
    newUri.uri(),
    fields,
    wkbType,
    srs,
    overwrite,
    map,
    createdLayerUri,
    errCause,
    options
  );
  if ( res != Qgis::VectorExportResult::Success )
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
}

QString QgsOracleProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  QgsDataSourceUri destUri( uri() );

  destUri.setTable( options.layerName );
  destUri.setSchema( options.schema );
  destUri.setGeometryColumn( options.wkbType != Qgis::WkbType::NoGeometry ? ( options.geometryColumn.isEmpty() ? u"GEOM"_s : options.geometryColumn ) : QString() );
  if ( !options.primaryKeyColumns.isEmpty() )
  {
    if ( options.primaryKeyColumns.length() > 1 )
    {
      QgsMessageLog::logMessage( u"Multiple primary keys are not supported by Oracle, ignoring"_s, QString(), Qgis::MessageLevel::Info );
    }
    destUri.setKeyColumn( options.primaryKeyColumns.at( 0 ) );
  }

  providerOptions.clear();
  return destUri.uri( false );
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


QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsOracleProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables;

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  const bool geometryColumnsOnly { configuration().value( "geometryColumnsOnly", false ).toBool() };
  const bool userTablesOnly { configuration().value( "userTablesOnly", false ).toBool() && schema.isEmpty() };
  const bool onlyExistingTypes { configuration().value( "onlyExistingTypes", false ).toBool() };
  const bool aspatial { !flags || flags.testFlag( TableFlag::Aspatial ) };

  QVector<QgsOracleLayerProperty> properties;
  const bool ok = conn->supportedLayers( properties, schema, geometryColumnsOnly, userTablesOnly, aspatial );
  if ( !ok )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve tables: %1" ).arg( uri() ) );
  }

  for ( auto &pr : properties )
  {
    if ( feedback && feedback->isCanceled() )
      break;

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
    for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ); i++ )
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
  executeSqlPrivate( u"DROP TABLE %1.%2"_s
                       .arg( QgsOracleConn::quotedIdentifier( schema ) )
                       .arg( QgsOracleConn::quotedIdentifier( name ) ) );

  executeSqlPrivate( u"DELETE FROM user_sdo_geom_metadata WHERE TABLE_NAME = '%1'"_s
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
  executeSqlPrivate( u"ALTER TABLE %1.%2 RENAME TO %3"_s
                       .arg( QgsOracleConn::quotedIdentifier( schema ), QgsOracleConn::quotedIdentifier( name ), QgsOracleConn::quotedIdentifier( newName ) ) );

  executeSqlPrivate( u"UPDATE user_sdo_geom_metadata SET TABLE_NAME = '%1' where TABLE_NAME = '%2'"_s
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

  executeSqlPrivate( u"DROP INDEX %1"_s.arg( indexName ) );
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
  return QgsApplication::getThemeIcon( u"mIconOracle.svg"_s );
}

QStringList QgsOracleProviderConnection::schemas() const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;

  // get only non system schemas/users
  QList<QVariantList> users = executeSqlPrivate( u"SELECT USERNAME FROM ALL_USERS where ORACLE_MAINTAINED = 'N' AND USERNAME NOT IN ( 'PDBADMIN', 'HR' )"_s ).rows();
  for ( QVariantList userInfos : users )
    schemas << userInfos.at( 0 ).toString();

  return schemas;
}
