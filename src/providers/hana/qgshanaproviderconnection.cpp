/***************************************************************************
   qgshanaproviderconnection.cpp  -  QgsHanaProviderConnection
   --------------------------------------
   Date      : 07-04-2020
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
#include "qgshanaproviderconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanaprimarykeys.h"
#include "qgshanaprovider.h"
#include "qgshanaresultset.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include "odbc/PreparedStatement.h"

#include <chrono>

using namespace NS_ODBC;

QgsHanaProviderResultIterator::QgsHanaProviderResultIterator( QgsHanaConnectionRef &&conn, QgsHanaResultSetRef &&resultSet )
  : mConnection( std::move( conn ) )
  , mResultSet( std::move( resultSet ) )
  , mNumColumns( mResultSet->getMetadata().getColumnCount() )
  , mNextRow( mResultSet->next() )
{}

QVariantList QgsHanaProviderResultIterator::nextRowPrivate()
{
  QVariantList ret;
  if ( !mNextRow )
    return ret;

  ret.reserve( mNumColumns );
  for ( unsigned short i = 1; i <= mNumColumns; ++i )
    ret.push_back( mResultSet->getValue( i ) );
  mNextRow = mResultSet->next();
  return ret;
}

bool QgsHanaProviderResultIterator::hasNextRowPrivate() const
{
  return mNextRow;
}

long long QgsHanaProviderResultIterator::rowCountPrivate() const
{
  // The HANA ODBC driver doesn't support it.
  return static_cast<long long>( Qgis::FeatureCountState::UnknownCount );
}

QgsHanaProviderConnection::QgsHanaProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "hana" );
  QgsHanaSettings settings( name, true );
  setUri( settings.toDataSourceUri().uri() );
  setCapabilities();
}

QgsHanaProviderConnection::QgsHanaProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = QStringLiteral( "hana" );
  setCapabilities();
}

void QgsHanaProviderConnection::setCapabilities()
{
  mGeometryColumnCapabilities =
  {
    //GeometryColumnCapability::Curves, not fully supported yet
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds,
  };

  /*
   * Capability::DropSchema         | CREATE SCHEMA from SYSTEMPRIVILEGE
   * Capability::CreateSchema       | CREATE SCHEMA from SYSTEMPRIVILEGE
   * Capability::CreateVectorTable  | Note
   * Capability::DropVectorTable    | Note
   * Capability::RenameVectorTable  | Note
   * Capability::ExecuteSql         | Note
   * Capability::SqlLayers          | Note
   * Capability::Tables             | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::Schemas            | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::TableExists        | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::Spatial            | Always TRUE
   *
   * Note: Everyone has this privilege, but the execution might fail if the user does
   *       not have the necessary privileges for one of the objects in the query.
   */

  mCapabilities =
  {
    Capability::CreateVectorTable,
    Capability::DropVectorTable,
    Capability::RenameVectorTable,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    Capability::Spatial,
    Capability::AddField,
    Capability::DeleteField,
    Capability::DeleteFieldCascade
  };

  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( !conn.isNull() )
  {
    const QString sql = QStringLiteral( "SELECT OBJECT_TYPE, PRIVILEGE, SCHEMA_NAME, OBJECT_NAME FROM PUBLIC.EFFECTIVE_PRIVILEGES "
                                        "WHERE USER_NAME = CURRENT_USER AND IS_VALID = 'TRUE'" );
    try
    {
      QgsHanaResultSetRef rsPrivileges = conn->executeQuery( sql );
      while ( rsPrivileges->next() )
      {
        QString objType = rsPrivileges->getString( 1 );
        QString privType = rsPrivileges->getString( 2 );
        if ( objType == QLatin1String( "SYSTEMPRIVILEGE" ) )
        {
          if ( privType == QLatin1String( "CREATE SCHEMA" ) )
            mCapabilities |= Capability::CreateSchema | Capability::DropSchema | Capability::RenameSchema;
          else if ( privType == QLatin1String( "CATALOG READ" ) || privType == QLatin1String( "DATA ADMIN" ) )
            mCapabilities |= Capability::Schemas | Capability::Tables | Capability::TableExists;
        }
        else if ( objType == QLatin1String( "TABLE" ) || objType == QLatin1String( "VIEW" ) )
        {
          if ( privType == QLatin1String( "SELECT" ) )
          {
            QString schemaName = rsPrivileges->getString( 3 );
            QString objName = rsPrivileges->getString( 4 );

            if ( schemaName == QLatin1String( "SYS" ) && objName == QLatin1String( "SCHEMAS" ) )
              mCapabilities |= Capability::Schemas;
            else if ( objName == QLatin1String( "TABLE_COLUMNS" ) )
              mCapabilities |= Capability::Tables | Capability::TableExists;
          }
        }
      }
      rsPrivileges->close();

      return;
    }
    catch ( const QgsHanaException &ex )
    {
      QgsMessageLog::logMessage( QObject::tr( "Unable to retrieve user privileges: %1" )
                                 .arg( ex.what() ), QObject::tr( "SAP HANA" ) );
    }
  }

  // We enable all capabilities, if we were not able to retrieve them from the database.
  mCapabilities |= Capability::CreateSchema | Capability::DropSchema | Capability::RenameSchema |
                   Capability::Schemas | Capability::Tables | Capability::TableExists;
}

void QgsHanaProviderConnection::createVectorTable( const QString &schema,
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
  // Set geometry column if it's not aspatial
  if ( wkbType != QgsWkbTypes::Type::Unknown &&  wkbType != QgsWkbTypes::Type::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "geom" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult res = QgsHanaProvider::createEmptyLayer(
                                   newUri.uri(),
                                   fields,
                                   wkbType,
                                   srs,
                                   overwrite,
                                   &map,
                                   &errCause,
                                   options
                                 );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsHanaProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const TableProperty tableInfo { table( schema, name ) };

  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  dsUri.setGeometryColumn( tableInfo .geometryColumn() );
  return dsUri.uri( false );
}

void QgsHanaProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  const TableProperty tableInfo = table( schema, name );
  if ( tableInfo.flags().testFlag( TableFlag::View ) )
    executeSqlStatement( QStringLiteral( "DROP VIEW %1.%2" )
                         .arg( QgsHanaUtils::quotedIdentifier( schema ),
                               QgsHanaUtils::quotedIdentifier( name ) ) );
  else
    executeSqlStatement( QStringLiteral( "DROP TABLE %1.%2" )
                         .arg( QgsHanaUtils::quotedIdentifier( schema ),
                               QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  executeSqlStatement( QStringLiteral( "RENAME TABLE %1.%2 TO %1.%3" )
                       .arg( QgsHanaUtils::quotedIdentifier( schema ),
                             QgsHanaUtils::quotedIdentifier( name ),
                             QgsHanaUtils::quotedIdentifier( newName ) ) );
}

void QgsHanaProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlStatement( QStringLiteral( "CREATE SCHEMA %1" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::dropSchema( const QString &name,  bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlStatement( QStringLiteral( "DROP SCHEMA %1 %2" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ),
                             force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsHanaProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlStatement( QStringLiteral( "RENAME SCHEMA %1 TO %2" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ), QgsHanaUtils::quotedIdentifier( newName ) ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsHanaProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
    return QueryResult( std::make_shared<QgsHanaEmptyProviderResultIterator>() );

  QgsHanaConnectionRef conn = createConnection();

  if ( feedback && feedback->isCanceled() )
    return QueryResult( std::make_shared<QgsHanaEmptyProviderResultIterator>() );

  try
  {
    PreparedStatementRef stmt = conn->prepareStatement( sql );
    bool isQuery = stmt->getMetaDataUnicode()->getColumnCount() > 0;
    if ( isQuery )
    {
      QgsHanaResultSetRef rs = conn->executeQuery( sql );
      ResultSetMetaDataUnicode &md = rs->getMetadata();
      unsigned short numColumns = md.getColumnCount();
      QStringList columns;
      columns.reserve( numColumns );
      for ( unsigned short i = 1; i <= numColumns; ++i )
        columns << QgsHanaUtils::toQString( md.getColumnName( i ) );
      QueryResult ret( std::make_shared<QgsHanaProviderResultIterator>( std::move( conn ), std::move( rs ) ) );
      for ( unsigned short i = 0; i < numColumns; ++i )
        ret.appendColumn( columns[i] );
      return ret;
    }
    else
    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      conn->execute( sql );
      conn->commit();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      QueryResult results( std::make_shared<QgsHanaEmptyProviderResultIterator>() );
      results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );
      return results;
    }
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( ex.what() );
  }
}

QgsHanaConnectionRef QgsHanaProviderConnection::createConnection() const
{
  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  return conn;
}

void QgsHanaProviderConnection::executeSqlStatement( const QString &sql ) const
{
  QgsHanaConnectionRef conn = createConnection();

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( ex.what() );
  }
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsHanaProviderConnection::tablesWithFilter(
  const QString &schema,
  const TableFlags &flags, const std::function<bool( const QgsHanaLayerProperty &layer )> &layerFilter ) const
{
  checkCapability( Capability::Tables );

  QgsHanaConnectionRef conn = createConnection();
  QList<QgsHanaProviderConnection::TableProperty> tables;

  try
  {
    const bool aspatial { ! flags || flags.testFlag( TableFlag::Aspatial ) };
    const QVector<QgsHanaLayerProperty> layers = conn->getLayersFull( schema, aspatial, false, layerFilter );
    tables.reserve( layers.size() );
    for ( const QgsHanaLayerProperty &layerInfo :  layers )
    {
      // Classify
      TableFlags prFlags;
      if ( layerInfo.isView )
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::View );
      if ( !layerInfo.geometryColName.isEmpty() )
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::Vector );
      else
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::Aspatial );

      // Filter
      if ( ! flags || ( prFlags & flags ) )
      {
        QgsHanaProviderConnection::TableProperty property;
        property.setFlags( prFlags );

        QgsCoordinateReferenceSystem crs = conn->getCrs( layerInfo.srid );
        property.addGeometryColumnType( layerInfo.type, crs );

        property.setTableName( layerInfo.tableName );
        property.setSchema( layerInfo.schemaName );
        property.setGeometryColumn( layerInfo.geometryColName );
        property.setGeometryColumnCount( layerInfo.geometryColName.isEmpty() ? 0 : 1 );
        property.setComment( layerInfo.tableComment );

        if ( layerInfo.isView )
        {
          // Set the candidates
          property.setPrimaryKeyColumns( layerInfo.pkCols );
        }
        else  // Fetch and set the real pks
        {
          QStringList pks = conn->getLayerPrimaryKey( layerInfo.schemaName, layerInfo.tableName );
          property.setPrimaryKeyColumns( pks );
        }
        tables.push_back( property );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve tables: %1, %2" ).arg( uri(), ex.what() ) );
  }

  return tables;
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsHanaProviderConnection::table( const QString &schema, const QString &table ) const
{
  const QString geometryColumn = QgsDataSourceUri( uri() ).geometryColumn();
  auto layerFilter = [&table, &geometryColumn]( const QgsHanaLayerProperty & layer )
  {
    return layer.tableName == table && ( geometryColumn.isEmpty() || layer.geometryColName == geometryColumn );
  };
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tablesWithFilter( schema, TableFlags(), layerFilter ) };
  if ( constTables.empty() )
    throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                          .arg( table, schema ) );
  return constTables[0];
}

QList<QgsHanaProviderConnection::TableProperty> QgsHanaProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  return tablesWithFilter( schema, flags );
}

QStringList QgsHanaProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );

  QgsHanaConnectionRef conn = createConnection();

  try
  {
    QStringList schemas;
    const QVector<QgsHanaSchemaProperty> schemaProperties = conn->getSchemas( QString() );
    schemas.reserve( schemaProperties.size() );
    for ( const QgsHanaSchemaProperty &s : schemaProperties )
      schemas.push_back( s.name );
    return schemas;
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve schemas: %1, %2" ).arg( uri(), ex.what() ) );
  }
}

QgsFields QgsHanaProviderConnection::fields( const QString &schema, const QString &table ) const
{
  QgsHanaConnectionRef conn = createConnection();
  const QString geometryColumn = QgsDataSourceUri( uri() ).geometryColumn();
  try
  {
    QgsFields fields;
    auto processField = [&geometryColumn, &fields]( const AttributeField & field )
    {
      if ( field.name != geometryColumn )
        fields.append( field.toQgsField() );
    };
    conn->readTableFields( schema, table, processField );
    return fields;
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve fields: %1" ).arg( ex.what() ) );
  }
}

void QgsHanaProviderConnection::store( const QString &name ) const
{
  // delete the original entry first
  remove( name );

  QgsHanaSettings settings( name );
  settings.setFromDataSourceUri( uri() );
  settings.setSaveUserName( true );
  settings.setSavePassword( true );
  settings.save();
}

void QgsHanaProviderConnection::remove( const QString &name ) const
{
  QgsHanaSettings::removeConnection( name );
}

QIcon QgsHanaProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconHana.svg" ) );
}

QList<QgsVectorDataProvider::NativeType> QgsHanaProviderConnection::nativeTypes() const
{
  QgsHanaConnectionRef conn = createConnection();

  QList<QgsVectorDataProvider::NativeType> types = conn->getNativeTypes();
  if ( types.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  return types;
}

QgsVectorLayer *QgsHanaProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions &options ) const
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
    tUri.setKeyColumn( QgsHanaPrimaryKeyUtils::buildUriKey( options.primaryKeyColumns ) );
    tUri.setTable( QStringLiteral( "(%1)" ).arg( options.sql ) );
  }
  else
  {
    int pkId { 0 };
    while ( options.sql.contains( QStringLiteral( "_uid%1_" ).arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId ++;
    }
    tUri.setKeyColumn( QStringLiteral( "_uid%1_" ).arg( pkId ) );

    int sqlId { 0 };
    while ( options.sql.contains( QStringLiteral( "_subq_%1_" ).arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId ++;
    }
    tUri.setTable( QStringLiteral( "(SELECT ROW_NUMBER() over () AS \"_uid%1_\", * FROM (%2\n) AS \"_subq_%3_\"\n)" ).arg( QString::number( pkId ), options.sql, QString::number( sqlId ) ) );
  }

  if ( ! options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer{ tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey(), vectorLayerOptions };
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsHanaProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  const QgsDataSourceUri tUri( layerSource );
  options.primaryKeyColumns = QgsHanaPrimaryKeyUtils::parseUriKey( tUri.keyColumn() );
  options.disableSelectAtId = tUri.selectAtIdDisabled();
  options.geometryColumn = tUri.geometryColumn();
  options.filter = tUri.sql();
  const QString trimmedTable { tUri.table().trimmed() };
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : QStringLiteral( "SELECT * FROM %1" ).arg( tUri.quotedTablename() );
  return options;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsHanaProviderConnection::sqlDictionary()
{
  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
  {
    {
      Qgis::SqlKeywordCategory::Keyword,
      {
        QStringLiteral( "ABAP_CHAR" ),
        QStringLiteral( "ABAP_DATE" ),
        QStringLiteral( "ABAP_DECFLOAT16" ),
        QStringLiteral( "ABAP_DECFLOAT34" ),
        QStringLiteral( "ABAP_FLOAT" ),
        QStringLiteral( "ABAP_HEX" ),
        QStringLiteral( "ABAP_INT" ),
        QStringLiteral( "ABAP_INT1" ),
        QStringLiteral( "ABAP_INT2" ),
        QStringLiteral( "ABAP_NUM" ),
        QStringLiteral( "ABAP_PACKED" ),
        QStringLiteral( "ABAP_STRING" ),
        QStringLiteral( "ABAP_TIME" ),
        QStringLiteral( "ABAP_XSTRING" ),
        QStringLiteral( "ABAPITAB" ),
        QStringLiteral( "ABAPSTRUCT" ),
        QStringLiteral( "ABSOLUTE" ),
        QStringLiteral( "ABSTRACT" ),
        QStringLiteral( "ACCESS" ),
        QStringLiteral( "ACCURACY" ),
        QStringLiteral( "ACKNOWLEDGED" ),
        QStringLiteral( "ACTION" ),
        QStringLiteral( "ACTIONS" ),
        QStringLiteral( "ACTIVATE" ),
        QStringLiteral( "ADAPTER" ),
        QStringLiteral( "ADD" ),
        QStringLiteral( "ADD_DAYS" ),
        QStringLiteral( "ADD_MONTHS" ),
        QStringLiteral( "ADD_SECONDS" ),
        QStringLiteral( "ADD_YEARS" ),
        QStringLiteral( "ADDRESS" ),
        QStringLiteral( "ADMIN" ),
        QStringLiteral( "ADOPT" ),
        QStringLiteral( "AFL" ),
        QStringLiteral( "AFTER" ),
        QStringLiteral( "AGENT" ),
        QStringLiteral( "ALERT" ),
        QStringLiteral( "ALGORITHM" ),
        QStringLiteral( "ALIAS" ),
        QStringLiteral( "ALIGNMENT" ),
        QStringLiteral( "ALL" ),
        QStringLiteral( "ALLATTRSFREESTYLERELEVANT" ),
        QStringLiteral( "ALLOCATOR" ),
        QStringLiteral( "ALLOCATORS" ),
        QStringLiteral( "ALLOWED" ),
        QStringLiteral( "ALPHANUM" ),
        QStringLiteral( "ALPHANUM_IDENTIFIER" ),
        QStringLiteral( "ALTER" ),
        QStringLiteral( "ALTERNATE" ),
        QStringLiteral( "ALWAYS" ),
        QStringLiteral( "ANALYSIS" ),
        QStringLiteral( "ANALYTIC" ),
        QStringLiteral( "ANALYZE" ),
        QStringLiteral( "ANCHOR" ),
        QStringLiteral( "AND" ),
        QStringLiteral( "ANGULAR" ),
        QStringLiteral( "ANNOTATE" ),
        QStringLiteral( "ANNOTATIONS" ),
        QStringLiteral( "ANONYMIZATION" ),
        QStringLiteral( "ANY" ),
        QStringLiteral( "APPEND" ),
        QStringLiteral( "APPLICATION" ),
        QStringLiteral( "APPLICATION_TIME" ),
        QStringLiteral( "APPLICATIONUSER" ),
        QStringLiteral( "APPLY" ),
        QStringLiteral( "APPLY_FILTER" ),
        QStringLiteral( "ARCHIVE" ),
        QStringLiteral( "AREA" ),
        QStringLiteral( "ARRAY" ),
        QStringLiteral( "ARRAY_AGG" ),
        QStringLiteral( "AS" ),
        QStringLiteral( "ASC" ),
        QStringLiteral( "ASSERTION" ),
        QStringLiteral( "ASSOCIATION" ),
        QStringLiteral( "ASSOCIATIONS" ),
        QStringLiteral( "ASYNC" ),
        QStringLiteral( "ASYNCHRONOUS" ),
        QStringLiteral( "AT" ),
        QStringLiteral( "ATOMIC" ),
        QStringLiteral( "ATTACH" ),
        QStringLiteral( "ATTEMPTS" ),
        QStringLiteral( "ATTRIBUTE" ),
        QStringLiteral( "ATTRIBUTEMAPPING" ),
        QStringLiteral( "AUDIT" ),
        QStringLiteral( "AUDITING" ),
        QStringLiteral( "AUTHENTICATION" ),
        QStringLiteral( "AUTHORIZATION" ),
        QStringLiteral( "AUTO" ),
        QStringLiteral( "AUTOCOMMIT" ),
        QStringLiteral( "AUTOMATIC" ),
        QStringLiteral( "AUTOMERGE" ),
        QStringLiteral( "AXIS" ),
        QStringLiteral( "BACKINT" ),
        QStringLiteral( "BACKUP" ),
        QStringLiteral( "BACKUP_ID" ),
        QStringLiteral( "BACKUPS" ),
        QStringLiteral( "BALANCE" ),
        QStringLiteral( "BASIC" ),
        QStringLiteral( "BATCH" ),
        QStringLiteral( "BEFORE" ),
        QStringLiteral( "BEGIN" ),
        QStringLiteral( "BERNOULLI" ),
        QStringLiteral( "BEST" ),
        QStringLiteral( "BETWEEN" ),
        QStringLiteral( "BIGINT" ),
        QStringLiteral( "BINARY" ),
        QStringLiteral( "BINARYCONSTRAINT" ),
        QStringLiteral( "BIND_AS_PARAMETER" ),
        QStringLiteral( "BIND_AS_VALUE" ),
        QStringLiteral( "BIND_BIGINT" ),
        QStringLiteral( "BIND_CHAR" ),
        QStringLiteral( "BIND_DECIMAL" ),
        QStringLiteral( "BIND_DOUBLE" ),
        QStringLiteral( "BIND_NCHAR" ),
        QStringLiteral( "BIND_REAL" ),
        QStringLiteral( "BITS" ),
        QStringLiteral( "BLOB" ),
        QStringLiteral( "BOOLEAN" ),
        QStringLiteral( "BOTH" ),
        QStringLiteral( "BOUNDARY" ),
        QStringLiteral( "BREAK" ),
        QStringLiteral( "BREAKUP" ),
        QStringLiteral( "BTREE" ),
        QStringLiteral( "BUCKETS" ),
        QStringLiteral( "BUFFER" ),
        QStringLiteral( "BUILTIN" ),
        QStringLiteral( "BULK" ),
        QStringLiteral( "BUSINESS" ),
        QStringLiteral( "BY" ),
        QStringLiteral( "CA" ),
        QStringLiteral( "CACHE" ),
        QStringLiteral( "CALCULATEDKEYFIGURE" ),
        QStringLiteral( "CALCULATEDVIEWATTRIBUTE" ),
        QStringLiteral( "CALCULATION" ),
        QStringLiteral( "CALENDAR" ),
        QStringLiteral( "CALL" ),
        QStringLiteral( "CALLS" ),
        QStringLiteral( "CALLSTACK" ),
        QStringLiteral( "CANCEL" ),
        QStringLiteral( "CAPTURE" ),
        QStringLiteral( "CASCADE" ),
        QStringLiteral( "CASE" ),
        QStringLiteral( "CAST" ),
        QStringLiteral( "CATALOG" ),
        QStringLiteral( "CDS" ),
        QStringLiteral( "CE_CALC" ),
        QStringLiteral( "CE_JOIN" ),
        QStringLiteral( "CE_PROJECTION" ),
        QStringLiteral( "CELL" ),
        QStringLiteral( "CELLS" ),
        QStringLiteral( "CENTER" ),
        QStringLiteral( "CERTIFICATE" ),
        QStringLiteral( "CERTIFICATES" ),
        QStringLiteral( "CHANGE" ),
        QStringLiteral( "CHANGES" ),
        QStringLiteral( "CHAR" ),
        QStringLiteral( "CHARACTER" ),
        QStringLiteral( "CHARACTERISTIC" ),
        QStringLiteral( "CHECK" ),
        QStringLiteral( "CHECKPOINT" ),
        QStringLiteral( "CHILDRENATTRIBUTE" ),
        QStringLiteral( "CLAIM" ),
        QStringLiteral( "CLASS" ),
        QStringLiteral( "CLEAR" ),
        QStringLiteral( "CLIENT" ),
        QStringLiteral( "CLIENTPKI" ),
        QStringLiteral( "CLIENTSIDE" ),
        QStringLiteral( "CLIP" ),
        QStringLiteral( "CLOB" ),
        QStringLiteral( "CLOSE" ),
        QStringLiteral( "CLUSTER" ),
        QStringLiteral( "CLUSTERING" ),
        QStringLiteral( "CLUSTERS" ),
        QStringLiteral( "COALESCE" ),
        QStringLiteral( "CODE" ),
        QStringLiteral( "CODEPAGE" ),
        QStringLiteral( "COLLATE" ),
        QStringLiteral( "COLLATION" ),
        QStringLiteral( "COLLECTION" ),
        QStringLiteral( "COLUMN" ),
        QStringLiteral( "COLUMN_VIEW_ESTIMATION" ),
        QStringLiteral( "COLUMNS" ),
        QStringLiteral( "COMMENT" ),
        QStringLiteral( "COMMIT" ),
        QStringLiteral( "COMMITTED" ),
        QStringLiteral( "COMPACT" ),
        QStringLiteral( "COMPATIBILITY" ),
        QStringLiteral( "COMPLETE" ),
        QStringLiteral( "COMPONENT" ),
        QStringLiteral( "COMPONENTS" ),
        QStringLiteral( "COMPRESSED" ),
        QStringLiteral( "COMPRESSION" ),
        QStringLiteral( "COMPUTE" ),
        QStringLiteral( "COMPUTE_REPLICA_COUNT" ),
        QStringLiteral( "COMPUTE_REPLICA_TYPE" ),
        QStringLiteral( "CONCAT" ),
        QStringLiteral( "CONDITION" ),
        QStringLiteral( "CONDITIONAL" ),
        QStringLiteral( "CONFIG" ),
        QStringLiteral( "CONFIGURATION" ),
        QStringLiteral( "CONFIGURE" ),
        QStringLiteral( "CONNECT" ),
        QStringLiteral( "CONNECTION" ),
        QStringLiteral( "CONSISTENCY" ),
        QStringLiteral( "CONSTANT" ),
        QStringLiteral( "CONSTRAINT" ),
        QStringLiteral( "CONSTRAINTS" ),
        QStringLiteral( "CONTAINS" ),
        QStringLiteral( "CONTENT" ),
        QStringLiteral( "CONTEXT" ),
        QStringLiteral( "CONTINUE" ),
        QStringLiteral( "CONTROL" ),
        QStringLiteral( "CONTROLLED" ),
        QStringLiteral( "CONV" ),
        QStringLiteral( "CONVERT" ),
        QStringLiteral( "COORDINATE" ),
        QStringLiteral( "COPY" ),
        QStringLiteral( "COREFILE" ),
        QStringLiteral( "CORRELATION" ),
        QStringLiteral( "COVERAGE" ),
        QStringLiteral( "CPBTREE" ),
        QStringLiteral( "CPU" ),
        QStringLiteral( "CREATE" ),
        QStringLiteral( "CREATION" ),
        QStringLiteral( "CREATOR" ),
        QStringLiteral( "CREDENTIAL" ),
        QStringLiteral( "CRITICAL" ),
        QStringLiteral( "CRON" ),
        QStringLiteral( "CROSS" ),
        QStringLiteral( "CS_DATE" ),
        QStringLiteral( "CS_DAYDATE" ),
        QStringLiteral( "CS_DECIMAL_FLOAT" ),
        QStringLiteral( "CS_DOUBLE" ),
        QStringLiteral( "CS_FIXED" ),
        QStringLiteral( "CS_FIXEDSTRING" ),
        QStringLiteral( "CS_FLOAT" ),
        QStringLiteral( "CS_GEOMETRY" ),
        QStringLiteral( "CS_INT" ),
        QStringLiteral( "CS_LONGDATE" ),
        QStringLiteral( "CS_POINT" ),
        QStringLiteral( "CS_POINTZ" ),
        QStringLiteral( "CS_RAW" ),
        QStringLiteral( "CS_SDFLOAT" ),
        QStringLiteral( "CS_SECONDDATE" ),
        QStringLiteral( "CS_SECONDTIME" ),
        QStringLiteral( "CS_SHORTTEXT" ),
        QStringLiteral( "CS_STRING" ),
        QStringLiteral( "CS_TEXT" ),
        QStringLiteral( "CS_TEXT_AE" ),
        QStringLiteral( "CS_TIME" ),
        QStringLiteral( "CSV" ),
        QStringLiteral( "CUBE" ),
        QStringLiteral( "CUME_DIST" ),
        QStringLiteral( "CURDATE" ),
        QStringLiteral( "CURRENT" ),
        QStringLiteral( "CURRENT_CONNECTION" ),
        QStringLiteral( "CURRENT_DATABASE" ),
        QStringLiteral( "CURRENT_DATE" ),
        QStringLiteral( "CURRENT_SCHEMA" ),
        QStringLiteral( "CURRENT_TIME" ),
        QStringLiteral( "CURRENT_TIMESTAMP" ),
        QStringLiteral( "CURRENT_TRANSACTION_ISOLATION_LEVEL" ),
        QStringLiteral( "CURRENT_USER" ),
        QStringLiteral( "CURRENT_UTCDATE" ),
        QStringLiteral( "CURRENT_UTCTIME" ),
        QStringLiteral( "CURRENT_UTCTIMESTAMP" ),
        QStringLiteral( "CURRVAL" ),
        QStringLiteral( "CURSOR" ),
        QStringLiteral( "CURTIME" ),
        QStringLiteral( "CURVE" ),
        QStringLiteral( "CYCLE" ),
        QStringLiteral( "D" ),
        QStringLiteral( "DATA" ),
        QStringLiteral( "DATABASE" ),
        QStringLiteral( "DATABASES" ),
        QStringLiteral( "DATAPROVISIONING" ),
        QStringLiteral( "DATASET" ),
        QStringLiteral( "DATASOURCE" ),
        QStringLiteral( "DATAVOLUME" ),
        QStringLiteral( "DATE" ),
        QStringLiteral( "DATEFORMAT" ),
        QStringLiteral( "DATETIME" ),
        QStringLiteral( "DATS_EXTRACT" ),
        QStringLiteral( "DAY" ),
        QStringLiteral( "DAYDATE" ),
        QStringLiteral( "DAYOFMONTH" ),
        QStringLiteral( "DAYOFWEEK" ),
        QStringLiteral( "DAYS_BETWEEN" ),
        QStringLiteral( "DBSCAN" ),
        QStringLiteral( "DBSPACE" ),
        QStringLiteral( "DDIC_ACCP" ),
        QStringLiteral( "DDIC_CDAY" ),
        QStringLiteral( "DDIC_CHAR" ),
        QStringLiteral( "DDIC_CLNT" ),
        QStringLiteral( "DDIC_CUKY" ),
        QStringLiteral( "DDIC_CURR" ),
        QStringLiteral( "DDIC_D16D" ),
        QStringLiteral( "DDIC_D16R" ),
        QStringLiteral( "DDIC_D16S" ),
        QStringLiteral( "DDIC_D34D" ),
        QStringLiteral( "DDIC_D34R" ),
        QStringLiteral( "DDIC_D34S" ),
        QStringLiteral( "DDIC_DATS" ),
        QStringLiteral( "DDIC_DAY" ),
        QStringLiteral( "DDIC_DEC" ),
        QStringLiteral( "DDIC_FLTP" ),
        QStringLiteral( "DDIC_GUID" ),
        QStringLiteral( "DDIC_INT1" ),
        QStringLiteral( "DDIC_INT2" ),
        QStringLiteral( "DDIC_INT4" ),
        QStringLiteral( "DDIC_INT8" ),
        QStringLiteral( "DDIC_LANG" ),
        QStringLiteral( "DDIC_LCHR" ),
        QStringLiteral( "DDIC_LRAW" ),
        QStringLiteral( "DDIC_MIN" ),
        QStringLiteral( "DDIC_MON" ),
        QStringLiteral( "DDIC_NUMC" ),
        QStringLiteral( "DDIC_PREC" ),
        QStringLiteral( "DDIC_QUAN" ),
        QStringLiteral( "DDIC_RAW" ),
        QStringLiteral( "DDIC_RSTR" ),
        QStringLiteral( "DDIC_SEC" ),
        QStringLiteral( "DDIC_SRST" ),
        QStringLiteral( "DDIC_SSTR" ),
        QStringLiteral( "DDIC_STRG" ),
        QStringLiteral( "DDIC_STXT" ),
        QStringLiteral( "DDIC_TEXT" ),
        QStringLiteral( "DDIC_TIMS" ),
        QStringLiteral( "DDIC_UNIT" ),
        QStringLiteral( "DDIC_UTCL" ),
        QStringLiteral( "DDIC_UTCM" ),
        QStringLiteral( "DDIC_UTCS" ),
        QStringLiteral( "DDIC_VARC" ),
        QStringLiteral( "DDIC_WEEK" ),
        QStringLiteral( "DDL" ),
        QStringLiteral( "DEACTIVATE" ),
        QStringLiteral( "DEBUG" ),
        QStringLiteral( "DEBUGGER" ),
        QStringLiteral( "DEC" ),
        QStringLiteral( "DECIMAL" ),
        QStringLiteral( "DECLARE" ),
        QStringLiteral( "DEFAULT" ),
        QStringLiteral( "DEFAULTVIEW" ),
        QStringLiteral( "DEFERRED" ),
        QStringLiteral( "DEFINER" ),
        QStringLiteral( "DEFINITION" ),
        QStringLiteral( "DEFRAGMENT" ),
        QStringLiteral( "DELAY" ),
        QStringLiteral( "DELETE" ),
        QStringLiteral( "DELIMITED" ),
        QStringLiteral( "DELTA" ),
        QStringLiteral( "DENSE_RANK" ),
        QStringLiteral( "DEPENDENCIES" ),
        QStringLiteral( "DEPENDENCY" ),
        QStringLiteral( "DEPENDENT" ),
        QStringLiteral( "DEPTH" ),
        QStringLiteral( "DESC" ),
        QStringLiteral( "DESCRIPTION" ),
        QStringLiteral( "DETACH" ),
        QStringLiteral( "DETECTION" ),
        QStringLiteral( "DETERMINISTIC" ),
        QStringLiteral( "DEV_CS_ONLY" ),
        QStringLiteral( "DEV_NO_SEMI_JOIN_REDUCTION_TARGET" ),
        QStringLiteral( "DEV_PROC_CE" ),
        QStringLiteral( "DEV_PROC_INLINE" ),
        QStringLiteral( "DEV_PROC_L" ),
        QStringLiteral( "DEV_PROC_NO_INLINE" ),
        QStringLiteral( "DEV_PROC_SE_FT" ),
        QStringLiteral( "DEV_RS_ONLY" ),
        QStringLiteral( "DEV_SEMI_JOIN_REDUCTION_TARGET" ),
        QStringLiteral( "DEVELOPMENT" ),
        QStringLiteral( "DIFFERENTIAL" ),
        QStringLiteral( "DISABLE" ),
        QStringLiteral( "DISABLED" ),
        QStringLiteral( "DISCONNECT" ),
        QStringLiteral( "DISK" ),
        QStringLiteral( "DISTANCE" ),
        QStringLiteral( "DISTINCT" ),
        QStringLiteral( "DISTRIBUTE" ),
        QStringLiteral( "DISTRIBUTION" ),
        QStringLiteral( "DO" ),
        QStringLiteral( "DOCUMENT" ),
        QStringLiteral( "DOCUMENTS" ),
        QStringLiteral( "DOUBLE" ),
        QStringLiteral( "DPSERVER" ),
        QStringLiteral( "DROP" ),
        QStringLiteral( "DTAB" ),
        QStringLiteral( "DUPLICATES" ),
        QStringLiteral( "DURATION" ),
        QStringLiteral( "DW_OPTIMIZED" ),
        QStringLiteral( "DYNAMIC" ),
        QStringLiteral( "DYNAMIC_RANGE_THRESHOLD" ),
        QStringLiteral( "EACH" ),
        QStringLiteral( "EARTH" ),
        QStringLiteral( "EDGE" ),
        QStringLiteral( "ELEMENTS" ),
        QStringLiteral( "ELLIPSOID" ),
        QStringLiteral( "ELSE" ),
        QStringLiteral( "ELSEIF" ),
        QStringLiteral( "EMAIL" ),
        QStringLiteral( "EMERGENCY" ),
        QStringLiteral( "EMPTY" ),
        QStringLiteral( "ENABLE" ),
        QStringLiteral( "ENABLED" ),
        QStringLiteral( "ENCLOSED" ),
        QStringLiteral( "ENCODED" ),
        QStringLiteral( "ENCODING" ),
        QStringLiteral( "ENCRYPTED" ),
        QStringLiteral( "ENCRYPTION" ),
        QStringLiteral( "END" ),
        QStringLiteral( "ENFORCED" ),
        QStringLiteral( "ENTITY" ),
        QStringLiteral( "ENTRY" ),
        QStringLiteral( "EPM" ),
        QStringLiteral( "EPS" ),
        QStringLiteral( "EQ" ),
        QStringLiteral( "EQUIDISTANT" ),
        QStringLiteral( "ERROR" ),
        QStringLiteral( "ES" ),
        QStringLiteral( "ESCAPE" ),
        QStringLiteral( "ESTIMATE" ),
        QStringLiteral( "ESTIMATED" ),
        QStringLiteral( "ESTIMATION" ),
        QStringLiteral( "EVENT" ),
        QStringLiteral( "EVENTS" ),
        QStringLiteral( "EVERY" ),
        QStringLiteral( "EVICTION" ),
        QStringLiteral( "EXACT" ),
        QStringLiteral( "EXCEPT" ),
        QStringLiteral( "EXCEPTION" ),
        QStringLiteral( "EXCLUDE" ),
        QStringLiteral( "EXCLUDED" ),
        QStringLiteral( "EXCLUSIVE" ),
        QStringLiteral( "EXEC" ),
        QStringLiteral( "EXECUTE" ),
        QStringLiteral( "EXECUTION" ),
        QStringLiteral( "EXISTING" ),
        QStringLiteral( "EXISTS" ),
        QStringLiteral( "EXIT" ),
        QStringLiteral( "EXPLAIN" ),
        QStringLiteral( "EXPLICIT" ),
        QStringLiteral( "EXPORT" ),
        QStringLiteral( "EXPRESSION" ),
        QStringLiteral( "EXPRESSIONFLAGS" ),
        QStringLiteral( "EXTENDED" ),
        QStringLiteral( "EXTERNAL" ),
        QStringLiteral( "EXTERNAL_BACKUP_ID" ),
        QStringLiteral( "EXTERNALATTRIBUTE" ),
        QStringLiteral( "EXTERNALLY" ),
        QStringLiteral( "EXTRACT" ),
        QStringLiteral( "FACT" ),
        QStringLiteral( "FACTOR" ),
        QStringLiteral( "FAIL" ),
        QStringLiteral( "FALLBACK" ),
        QStringLiteral( "FALSE" ),
        QStringLiteral( "FAST" ),
        QStringLiteral( "FBO" ),
        QStringLiteral( "FETCH" ),
        QStringLiteral( "FIELD" ),
        QStringLiteral( "FILE" ),
        QStringLiteral( "FILL" ),
        QStringLiteral( "FILLFACTOR" ),
        QStringLiteral( "FILTER" ),
        QStringLiteral( "FINALIZE" ),
        QStringLiteral( "FINISH" ),
        QStringLiteral( "FIRST" ),
        QStringLiteral( "FLAG" ),
        QStringLiteral( "FLAGS" ),
        QStringLiteral( "FLATTEN" ),
        QStringLiteral( "FLATTENING" ),
        QStringLiteral( "FLOAT" ),
        QStringLiteral( "FLUSH" ),
        QStringLiteral( "FN" ),
        QStringLiteral( "FOLLOWING" ),
        QStringLiteral( "FOLLOWS" ),
        QStringLiteral( "FOR" ),
        QStringLiteral( "FORCE" ),
        QStringLiteral( "FORCE_FIRST_PASSWORD_CHANGE" ),
        QStringLiteral( "FORCE_RESULT_CACHE_REFRESH" ),
        QStringLiteral( "FOREIGN" ),
        QStringLiteral( "FOREVER" ),
        QStringLiteral( "FORGY" ),
        QStringLiteral( "FORMAT" ),
        QStringLiteral( "FORMULA" ),
        QStringLiteral( "FREESTYLESEARCHATTRIBUTE" ),
        QStringLiteral( "FREESTYLETRANSLATION" ),
        QStringLiteral( "FROM" ),
        QStringLiteral( "FULL" ),
        QStringLiteral( "FULLTEXT" ),
        QStringLiteral( "FUNCTION" ),
        QStringLiteral( "FUNCTION_PROFILER" ),
        QStringLiteral( "FUZZINESSTHRESHOLD" ),
        QStringLiteral( "FUZZY" ),
        QStringLiteral( "GB" ),
        QStringLiteral( "GENERATED" ),
        QStringLiteral( "GET_NUM_SERVERS" ),
        QStringLiteral( "GLOBAL" ),
        QStringLiteral( "GLOBALDICT" ),
        QStringLiteral( "GLOBALLY" ),
        QStringLiteral( "GOTO" ),
        QStringLiteral( "GRANT" ),
        QStringLiteral( "GRANTED" ),
        QStringLiteral( "GRAPH" ),
        QStringLiteral( "GREATEST" ),
        QStringLiteral( "GRID" ),
        QStringLiteral( "GROUP" ),
        QStringLiteral( "GROUP_NAME" ),
        QStringLiteral( "GROUP_TYPE" ),
        QStringLiteral( "GROUPING" ),
        QStringLiteral( "GROUPING_FILTER" ),
        QStringLiteral( "GROUPING_ID" ),
        QStringLiteral( "GROUPS" ),
        QStringLiteral( "GT" ),
        QStringLiteral( "GUID" ),
        QStringLiteral( "HANDLED" ),
        QStringLiteral( "HANDOVER" ),
        QStringLiteral( "HAS" ),
        QStringLiteral( "HASANYPRIVILEGES" ),
        QStringLiteral( "HASH" ),
        QStringLiteral( "HASH_JOIN" ),
        QStringLiteral( "HASSYSTEMPRIVILEGE" ),
        QStringLiteral( "HAVING" ),
        QStringLiteral( "HEXAGON" ),
        QStringLiteral( "HEXTOBIN" ),
        QStringLiteral( "HIDDEN" ),
        QStringLiteral( "HIERARCHY" ),
        QStringLiteral( "HIERARCHY_ANCESTORS" ),
        QStringLiteral( "HIERARCHY_ANCESTORS_AGGREGATE" ),
        QStringLiteral( "HIERARCHY_DESCENDANTS" ),
        QStringLiteral( "HIERARCHY_DESCENDANTS_AGGREGATE" ),
        QStringLiteral( "HIERARCHY_LEVELED" ),
        QStringLiteral( "HIERARCHY_SIBLINGS" ),
        QStringLiteral( "HIERARCHY_SPANTREE" ),
        QStringLiteral( "HIERARCHY_TEMPORAL" ),
        QStringLiteral( "HIERARCHYCHARACTERISTIC" ),
        QStringLiteral( "HIERARCHYINDEX" ),
        QStringLiteral( "HIERARCHYNAME" ),
        QStringLiteral( "HIGH" ),
        QStringLiteral( "HIGHLIGHTED" ),
        QStringLiteral( "HILBERT" ),
        QStringLiteral( "HINT" ),
        QStringLiteral( "HISTOGRAM" ),
        QStringLiteral( "HISTORY" ),
        QStringLiteral( "HOLD" ),
        QStringLiteral( "HORIZONTAL" ),
        QStringLiteral( "HOST" ),
        QStringLiteral( "HOSTS" ),
        QStringLiteral( "HOUR" ),
        QStringLiteral( "HOUSENUMBER" ),
        QStringLiteral( "ID" ),
        QStringLiteral( "IDENTICAL" ),
        QStringLiteral( "IDENTIFIED" ),
        QStringLiteral( "IDENTIFIER" ),
        QStringLiteral( "IDENTITY" ),
        QStringLiteral( "IF" ),
        QStringLiteral( "IFNULL" ),
        QStringLiteral( "IGNORE" ),
        QStringLiteral( "IMMEDIATE" ),
        QStringLiteral( "IMPORT" ),
        QStringLiteral( "IN" ),
        QStringLiteral( "INCLUDE" ),
        QStringLiteral( "INCREMENT" ),
        QStringLiteral( "INCREMENTAL" ),
        QStringLiteral( "INDEPENDENT" ),
        QStringLiteral( "INDEX" ),
        QStringLiteral( "INDEX_JOIN" ),
        QStringLiteral( "INDEX_UNION" ),
        QStringLiteral( "INDEXALIAS" ),
        QStringLiteral( "INDEXED" ),
        QStringLiteral( "INDEXES" ),
        QStringLiteral( "INDEXID" ),
        QStringLiteral( "INDEXTYPE" ),
        QStringLiteral( "INDIVIDUAL" ),
        QStringLiteral( "INFO" ),
        QStringLiteral( "INHERITS" ),
        QStringLiteral( "INIFILE" ),
        QStringLiteral( "INIT" ),
        QStringLiteral( "INITIAL" ),
        QStringLiteral( "INITIAL_PARTITIONS" ),
        QStringLiteral( "INITIALLY" ),
        QStringLiteral( "INLINE" ),
        QStringLiteral( "INNER" ),
        QStringLiteral( "INOUT" ),
        QStringLiteral( "INSENSITIVE" ),
        QStringLiteral( "INSERT" ),
        QStringLiteral( "INSTEAD" ),
        QStringLiteral( "INSTR" ),
        QStringLiteral( "INT" ),
        QStringLiteral( "INTEGER" ),
        QStringLiteral( "INTERNAL" ),
        QStringLiteral( "INTERSECT" ),
        QStringLiteral( "INTERVAL" ),
        QStringLiteral( "INTO" ),
        QStringLiteral( "INVALID" ),
        QStringLiteral( "INVERSE" ),
        QStringLiteral( "INVERTED" ),
        QStringLiteral( "INVOKER" ),
        QStringLiteral( "IS" ),
        QStringLiteral( "IS_EMPTY" ),
        QStringLiteral( "ISAUTHORIZED" ),
        QStringLiteral( "ISMEMORYINDEX" ),
        QStringLiteral( "ISOLATION" ),
        QStringLiteral( "ISSUER" ),
        QStringLiteral( "ISTOTAL" ),
        QStringLiteral( "ISTRANSACTIONAL" ),
        QStringLiteral( "JOB" ),
        QStringLiteral( "JOIN" ),
        QStringLiteral( "JOINCONDITION" ),
        QStringLiteral( "JOININDEX" ),
        QStringLiteral( "JOININDEXESTIMATION" ),
        QStringLiteral( "JOININDEXTYPE" ),
        QStringLiteral( "JOINPATH" ),
        QStringLiteral( "JSON" ),
        QStringLiteral( "JSON_QUERY" ),
        QStringLiteral( "JSON_TABLE" ),
        QStringLiteral( "JSON_VALUE" ),
        QStringLiteral( "JWT" ),
        QStringLiteral( "KB" ),
        QStringLiteral( "KEEP" ),
        QStringLiteral( "KERBEROS" ),
        QStringLiteral( "KERNEL" ),
        QStringLiteral( "KERNELTRACE" ),
        QStringLiteral( "KEY" ),
        QStringLiteral( "KEYATTRIBUTE" ),
        QStringLiteral( "KEYCOPY" ),
        QStringLiteral( "KEYFIGURE" ),
        QStringLiteral( "KEYPAIR" ),
        QStringLiteral( "KEYS" ),
        QStringLiteral( "KEYVALUE" ),
        QStringLiteral( "KMEANS" ),
        QStringLiteral( "L" ),
        QStringLiteral( "LABEL" ),
        QStringLiteral( "LAG" ),
        QStringLiteral( "LANGUAGE" ),
        QStringLiteral( "LAST" ),
        QStringLiteral( "LAST_DAY" ),
        QStringLiteral( "LATENCY" ),
        QStringLiteral( "LATERAL" ),
        QStringLiteral( "LAYOUT" ),
        QStringLiteral( "LDAP" ),
        QStringLiteral( "LEAD" ),
        QStringLiteral( "LEADING" ),
        QStringLiteral( "LEAF" ),
        QStringLiteral( "LEAST" ),
        QStringLiteral( "LEAVES" ),
        QStringLiteral( "LEFT" ),
        QStringLiteral( "LENGTH" ),
        QStringLiteral( "LENGTHB" ),
        QStringLiteral( "LEVEL" ),
        QStringLiteral( "LEVELNUMBERATTRIBUTE" ),
        QStringLiteral( "LEVELS" ),
        QStringLiteral( "LEVELTEXTATTRIBUTE" ),
        QStringLiteral( "LIBRARY" ),
        QStringLiteral( "LICENSE" ),
        QStringLiteral( "LIFETIME" ),
        QStringLiteral( "LIKE" ),
        QStringLiteral( "LIKE_REGEXPR" ),
        QStringLiteral( "LIMIT" ),
        QStringLiteral( "LINE" ),
        QStringLiteral( "LINEAR" ),
        QStringLiteral( "LINKED" ),
        QStringLiteral( "LIST" ),
        QStringLiteral( "LOAD" ),
        QStringLiteral( "LOAD_HISTORY" ),
        QStringLiteral( "LOADABLE" ),
        QStringLiteral( "LOB" ),
        QStringLiteral( "LOCAL" ),
        QStringLiteral( "LOCALE" ),
        QStringLiteral( "LOCATE" ),
        QStringLiteral( "LOCATE_REGEXPR" ),
        QStringLiteral( "LOCATION" ),
        QStringLiteral( "LOCATIONS" ),
        QStringLiteral( "LOCK" ),
        QStringLiteral( "LOCKED" ),
        QStringLiteral( "LOG" ),
        QStringLiteral( "LOGFLUSH" ),
        QStringLiteral( "LOGGING" ),
        QStringLiteral( "LONGDATE" ),
        QStringLiteral( "LOOKUP" ),
        QStringLiteral( "LOOP" ),
        QStringLiteral( "LOOPBACK" ),
        QStringLiteral( "LPAD" ),
        QStringLiteral( "LTRIM" ),
        QStringLiteral( "MACROS" ),
        QStringLiteral( "MAIN" ),
        QStringLiteral( "MAJOR" ),
        QStringLiteral( "MANAGEMENT" ),
        QStringLiteral( "MANUAL" ),
        QStringLiteral( "MANY" ),
        QStringLiteral( "MAP" ),
        QStringLiteral( "MAP_MERGE" ),
        QStringLiteral( "MAP_REDUCE" ),
        QStringLiteral( "MAPPING" ),
        QStringLiteral( "MASK" ),
        QStringLiteral( "MATCHED" ),
        QStringLiteral( "MATCHES" ),
        QStringLiteral( "MATCHING" ),
        QStringLiteral( "MAXITERATIONS" ),
        QStringLiteral( "MAXVALUE" ),
        QStringLiteral( "MB" ),
        QStringLiteral( "MDRS_TEST" ),
        QStringLiteral( "MDX" ),
        QStringLiteral( "MEASURE" ),
        QStringLiteral( "MEASURES" ),
        QStringLiteral( "MEDIUM" ),
        QStringLiteral( "MEMBER" ),
        QStringLiteral( "MEMORY" ),
        QStringLiteral( "MERGE" ),
        QStringLiteral( "MESSAGING" ),
        QStringLiteral( "META" ),
        QStringLiteral( "METADATA" ),
        QStringLiteral( "MIGRATE" ),
        QStringLiteral( "MIME" ),
        QStringLiteral( "MIN_ROWS_FOR_PARTITIONING" ),
        QStringLiteral( "MINING" ),
        QStringLiteral( "MINOR" ),
        QStringLiteral( "MINPTS" ),
        QStringLiteral( "MINUS" ),
        QStringLiteral( "MINUTE" ),
        QStringLiteral( "MINUTES" ),
        QStringLiteral( "MINVALUE" ),
        QStringLiteral( "MISSING" ),
        QStringLiteral( "MODE" ),
        QStringLiteral( "MODEL" ),
        QStringLiteral( "MODIFIED" ),
        QStringLiteral( "MODIFY" ),
        QStringLiteral( "MODULE" ),
        QStringLiteral( "MONITOR" ),
        QStringLiteral( "MONITORING" ),
        QStringLiteral( "MONTH" ),
        QStringLiteral( "MOVABLE" ),
        QStringLiteral( "MOVE" ),
        QStringLiteral( "MULTIPARENT" ),
        QStringLiteral( "MULTIPLE" ),
        QStringLiteral( "MULTIPROVIDERCONFIG" ),
        QStringLiteral( "MVCC_SNAPSHOT_TIMESTAMP" ),
        QStringLiteral( "NAME" ),
        QStringLiteral( "NATIONAL" ),
        QStringLiteral( "NATURAL" ),
        QStringLiteral( "NCHAR" ),
        QStringLiteral( "NCLOB" ),
        QStringLiteral( "NEAREST" ),
        QStringLiteral( "NEIGHBORS" ),
        QStringLiteral( "NESTED" ),
        QStringLiteral( "NESTED_LOOP_JOIN" ),
        QStringLiteral( "NETAPP" ),
        QStringLiteral( "NEW" ),
        QStringLiteral( "NEWFACTTABLE" ),
        QStringLiteral( "NEXT" ),
        QStringLiteral( "NEXT_DAY" ),
        QStringLiteral( "NEXTVAL" ),
        QStringLiteral( "NO" ),
        QStringLiteral( "NO_CALC_DIMENSION" ),
        QStringLiteral( "NO_DISTINCT_FILTER" ),
        QStringLiteral( "NO_INDEX" ),
        QStringLiteral( "NO_INLINE" ),
        QStringLiteral( "NO_ROUTE_TO" ),
        QStringLiteral( "NO_USE_C2C_CONV" ),
        QStringLiteral( "NO_USE_OLAP_PLAN" ),
        QStringLiteral( "NO_USE_TRANSFORMATION" ),
        QStringLiteral( "NO_VIRTUAL_TABLE_REPLICA" ),
        QStringLiteral( "NOCOMPRESS" ),
        QStringLiteral( "NODE" ),
        QStringLiteral( "NON" ),
        QStringLiteral( "NONE" ),
        QStringLiteral( "NONLEAF" ),
        QStringLiteral( "NOT" ),
        QStringLiteral( "NOW" ),
        QStringLiteral( "NOWAIT" ),
        QStringLiteral( "NTEXT" ),
        QStringLiteral( "NTILE" ),
        QStringLiteral( "NULL" ),
        QStringLiteral( "NULLABLE" ),
        QStringLiteral( "NULLIF" ),
        QStringLiteral( "NULLS" ),
        QStringLiteral( "NUMA" ),
        QStringLiteral( "NUMA_NODE_INDEXES" ),
        QStringLiteral( "NUMBER" ),
        QStringLiteral( "NUMERIC" ),
        QStringLiteral( "NVARCHAR" ),
        QStringLiteral( "OBJECT" ),
        QStringLiteral( "OBJECTS" ),
        QStringLiteral( "OCCURRENCE" ),
        QStringLiteral( "OCCURRENCES_REGEXPR" ),
        QStringLiteral( "ODATA" ),
        QStringLiteral( "OF" ),
        QStringLiteral( "OFF" ),
        QStringLiteral( "OFFSET" ),
        QStringLiteral( "OJ" ),
        QStringLiteral( "OLAP" ),
        QStringLiteral( "OLAP_PARALLEL_AGGREGATION" ),
        QStringLiteral( "OLAP_SERIAL_AGGREGATION" ),
        QStringLiteral( "OLD" ),
        QStringLiteral( "OLYMP" ),
        QStringLiteral( "ON" ),
        QStringLiteral( "ONE" ),
        QStringLiteral( "ONLINE" ),
        QStringLiteral( "ONLY" ),
        QStringLiteral( "OPEN" ),
        QStringLiteral( "OPENCYPHER_TABLE" ),
        QStringLiteral( "OPERATION" ),
        QStringLiteral( "OPERATOR" ),
        QStringLiteral( "OPTIMIZATION" ),
        QStringLiteral( "OPTIMIZEMETAMODEL" ),
        QStringLiteral( "OPTIMIZER" ),
        QStringLiteral( "OPTION" ),
        QStringLiteral( "OPTIONALLY" ),
        QStringLiteral( "OR" ),
        QStringLiteral( "ORDER" ),
        QStringLiteral( "ORDINALITY" ),
        QStringLiteral( "ORGANIZATION" ),
        QStringLiteral( "ORPHAN" ),
        QStringLiteral( "OS" ),
        QStringLiteral( "OTHERS" ),
        QStringLiteral( "OUT" ),
        QStringLiteral( "OUTER" ),
        QStringLiteral( "OVER" ),
        QStringLiteral( "OVERLAY" ),
        QStringLiteral( "OVERRIDE" ),
        QStringLiteral( "OVERRIDING" ),
        QStringLiteral( "OVERVIEW" ),
        QStringLiteral( "OWN" ),
        QStringLiteral( "OWNED" ),
        QStringLiteral( "OWNER" ),
        QStringLiteral( "PACKAGE" ),
        QStringLiteral( "PAGE" ),
        QStringLiteral( "PAGE_LOADABLE" ),
        QStringLiteral( "PAGES" ),
        QStringLiteral( "PARALLEL" ),
        QStringLiteral( "PARALLELIZED" ),
        QStringLiteral( "PARAMETER" ),
        QStringLiteral( "PARAMETERS" ),
        QStringLiteral( "PARENT" ),
        QStringLiteral( "PARENTSATTRIBUTE" ),
        QStringLiteral( "PARQUET" ),
        QStringLiteral( "PART" ),
        QStringLiteral( "PARTIAL" ),
        QStringLiteral( "PARTITION" ),
        QStringLiteral( "PARTITIONING" ),
        QStringLiteral( "PARTITIONS" ),
        QStringLiteral( "PARTS" ),
        QStringLiteral( "PASS" ),
        QStringLiteral( "PASSING" ),
        QStringLiteral( "PASSPORT_TRACELEVEL" ),
        QStringLiteral( "PASSWORD" ),
        QStringLiteral( "PATH" ),
        QStringLiteral( "PERCENT" ),
        QStringLiteral( "PERCENT_RANK" ),
        QStringLiteral( "PERCENTILE_CONT" ),
        QStringLiteral( "PERCENTILE_DISC" ),
        QStringLiteral( "PERFTRACE" ),
        QStringLiteral( "PERIOD" ),
        QStringLiteral( "PERSISTENCE" ),
        QStringLiteral( "PERSISTENT" ),
        QStringLiteral( "PERSISTENT_MEMORY" ),
        QStringLiteral( "PHRASE" ),
        QStringLiteral( "PHYSICAL" ),
        QStringLiteral( "PIN" ),
        QStringLiteral( "PLACEMENT" ),
        QStringLiteral( "PLAIN" ),
        QStringLiteral( "PLAN" ),
        QStringLiteral( "PLAN_EXECUTION" ),
        QStringLiteral( "PLANAR" ),
        QStringLiteral( "PLANNING" ),
        QStringLiteral( "PLANVIZ" ),
        QStringLiteral( "POBJECTKEY" ),
        QStringLiteral( "POLICY" ),
        QStringLiteral( "POLYGON" ),
        QStringLiteral( "PORT" ),
        QStringLiteral( "PORTION" ),
        QStringLiteral( "POSITION" ),
        QStringLiteral( "POSTCODE" ),
        QStringLiteral( "PPROPERTYNAME" ),
        QStringLiteral( "PRECEDES" ),
        QStringLiteral( "PRECEDING" ),
        QStringLiteral( "PRECISION" ),
        QStringLiteral( "PREDEFINED" ),
        QStringLiteral( "PREFERENCE" ),
        QStringLiteral( "PREFERRED" ),
        QStringLiteral( "PREFIX" ),
        QStringLiteral( "PRELOAD" ),
        QStringLiteral( "PREPROCESS" ),
        QStringLiteral( "PRESERVE" ),
        QStringLiteral( "PREVIOUS" ),
        QStringLiteral( "PRIMARY" ),
        QStringLiteral( "PRINCIPAL" ),
        QStringLiteral( "PRIOR" ),
        QStringLiteral( "PRIORITY" ),
        QStringLiteral( "PRIVATE" ),
        QStringLiteral( "PRIVILEGE" ),
        QStringLiteral( "PRIVILEGES" ),
        QStringLiteral( "PROCEDURE" ),
        QStringLiteral( "PROCESS" ),
        QStringLiteral( "PRODUCT" ),
        QStringLiteral( "PROFILE" ),
        QStringLiteral( "PROFILER" ),
        QStringLiteral( "PROJECTION" ),
        QStringLiteral( "PROPERTIES" ),
        QStringLiteral( "PROPERTY" ),
        QStringLiteral( "PROTOCOL" ),
        QStringLiteral( "PROVIDER" ),
        QStringLiteral( "PRUNING" ),
        QStringLiteral( "PSE" ),
        QStringLiteral( "PTIME" ),
        QStringLiteral( "PUBLIC" ),
        QStringLiteral( "PURPOSE" ),
        QStringLiteral( "PVALUENAME" ),
        QStringLiteral( "QERROR" ),
        QStringLiteral( "QTHETA" ),
        QStringLiteral( "QUERY" ),
        QStringLiteral( "QUEUE" ),
        QStringLiteral( "RAISE" ),
        QStringLiteral( "RANDOM" ),
        QStringLiteral( "RANGE" ),
        QStringLiteral( "RANK" ),
        QStringLiteral( "RATIO" ),
        QStringLiteral( "RAW" ),
        QStringLiteral( "RDICT" ),
        QStringLiteral( "READ" ),
        QStringLiteral( "READS" ),
        QStringLiteral( "REAL" ),
        QStringLiteral( "REALTIME" ),
        QStringLiteral( "REBUILD" ),
        QStringLiteral( "RECLAIM" ),
        QStringLiteral( "RECOMPILE" ),
        QStringLiteral( "RECOMPILED" ),
        QStringLiteral( "RECONFIGURE" ),
        QStringLiteral( "RECORD" ),
        QStringLiteral( "RECORD_COMMIT_TIMESTAMP" ),
        QStringLiteral( "RECORD_COUNT" ),
        QStringLiteral( "RECORD_ID" ),
        QStringLiteral( "RECORDS" ),
        QStringLiteral( "RECOVER" ),
        QStringLiteral( "RECOVERY" ),
        QStringLiteral( "RECURSIVE" ),
        QStringLiteral( "REFERENCE" ),
        QStringLiteral( "REFERENCES" ),
        QStringLiteral( "REFERENCING" ),
        QStringLiteral( "REFRESH" ),
        QStringLiteral( "REGISTER" ),
        QStringLiteral( "RELEASE" ),
        QStringLiteral( "RELOAD" ),
        QStringLiteral( "REMOTE" ),
        QStringLiteral( "REMOTE_EXECUTE_QUERY" ),
        QStringLiteral( "REMOTE_SCAN" ),
        QStringLiteral( "REMOVE" ),
        QStringLiteral( "RENAME" ),
        QStringLiteral( "REORGANIZE" ),
        QStringLiteral( "REPARTITIONING_THRESHOLD" ),
        QStringLiteral( "REPEATABLE" ),
        QStringLiteral( "REPLACE" ),
        QStringLiteral( "REPLACE_REGEXPR" ),
        QStringLiteral( "REPLAY" ),
        QStringLiteral( "REPLICA" ),
        QStringLiteral( "REPLICA_COUNT" ),
        QStringLiteral( "REPLICA_TYPE" ),
        QStringLiteral( "REPLICAS" ),
        QStringLiteral( "REPLICATION" ),
        QStringLiteral( "REPOSITORY" ),
        QStringLiteral( "RESERVE" ),
        QStringLiteral( "RESET" ),
        QStringLiteral( "RESIGNAL" ),
        QStringLiteral( "RESOURCE" ),
        QStringLiteral( "RESTART" ),
        QStringLiteral( "RESTORE" ),
        QStringLiteral( "RESTRICT" ),
        QStringLiteral( "RESTRICTED" ),
        QStringLiteral( "RESTRICTION" ),
        QStringLiteral( "RESULT" ),
        QStringLiteral( "RESULT_LAG" ),
        QStringLiteral( "RESULTSETS" ),
        QStringLiteral( "RESUME" ),
        QStringLiteral( "RETAIN" ),
        QStringLiteral( "RETENTION" ),
        QStringLiteral( "RETRY" ),
        QStringLiteral( "RETURN" ),
        QStringLiteral( "RETURNING" ),
        QStringLiteral( "RETURNS" ),
        QStringLiteral( "REVERSE" ),
        QStringLiteral( "REVOKE" ),
        QStringLiteral( "RIGHT" ),
        QStringLiteral( "ROLE" ),
        QStringLiteral( "ROLEGROUP" ),
        QStringLiteral( "ROLLBACK" ),
        QStringLiteral( "ROLLUP" ),
        QStringLiteral( "ROOT" ),
        QStringLiteral( "ROOT_STATEMENT_HASH" ),
        QStringLiteral( "ROUND" ),
        QStringLiteral( "ROUND_CEILING" ),
        QStringLiteral( "ROUND_DOWN" ),
        QStringLiteral( "ROUND_FLOOR" ),
        QStringLiteral( "ROUND_HALF_DOWN" ),
        QStringLiteral( "ROUND_HALF_EVEN" ),
        QStringLiteral( "ROUND_HALF_UP" ),
        QStringLiteral( "ROUND_UP" ),
        QStringLiteral( "ROUNDROBIN" ),
        QStringLiteral( "ROUTE" ),
        QStringLiteral( "ROUTE_BY" ),
        QStringLiteral( "ROUTE_BY_CARDINALITY" ),
        QStringLiteral( "ROUTE_TO" ),
        QStringLiteral( "ROW" ),
        QStringLiteral( "ROW_NUMBER" ),
        QStringLiteral( "ROWCOUNT" ),
        QStringLiteral( "ROWID" ),
        QStringLiteral( "ROWS" ),
        QStringLiteral( "RPAD" ),
        QStringLiteral( "RTREE" ),
        QStringLiteral( "RTRIM" ),
        QStringLiteral( "RULE" ),
        QStringLiteral( "RULES" ),
        QStringLiteral( "RUNTIME" ),
        QStringLiteral( "RUNTIMEDUMP" ),
        QStringLiteral( "SAME_PARTITION_COUNT" ),
        QStringLiteral( "SAML" ),
        QStringLiteral( "SAMPLE" ),
        QStringLiteral( "SAMPLING" ),
        QStringLiteral( "SAP_TIMEZONE_DATASET" ),
        QStringLiteral( "SATISFIES" ),
        QStringLiteral( "SAVE" ),
        QStringLiteral( "SAVEPOINT" ),
        QStringLiteral( "SCAN" ),
        QStringLiteral( "SCENARIO" ),
        QStringLiteral( "SCHEDULER" ),
        QStringLiteral( "SCHEMA" ),
        QStringLiteral( "SCHEMA_NAME" ),
        QStringLiteral( "SCORE" ),
        QStringLiteral( "SCRAMBLE" ),
        QStringLiteral( "SCROLL" ),
        QStringLiteral( "SEARCH" ),
        QStringLiteral( "SECOND" ),
        QStringLiteral( "SECONDDATE" ),
        QStringLiteral( "SECONDS_BETWEEN" ),
        QStringLiteral( "SECONDTIME" ),
        QStringLiteral( "SECTIONS" ),
        QStringLiteral( "SECURE" ),
        QStringLiteral( "SECURITY" ),
        QStringLiteral( "SEED" ),
        QStringLiteral( "SELECT" ),
        QStringLiteral( "SEMANTICRELATION" ),
        QStringLiteral( "SEMI" ),
        QStringLiteral( "SENSITIVE" ),
        QStringLiteral( "SEPARATORS" ),
        QStringLiteral( "SEQUENCE" ),
        QStringLiteral( "SEQUENTIAL" ),
        QStringLiteral( "SERIALIZABLE" ),
        QStringLiteral( "SERIES" ),
        QStringLiteral( "SERIES_ELEMENT_TO_PERIOD" ),
        QStringLiteral( "SERIES_PERIOD_TO_ELEMENT" ),
        QStringLiteral( "SERIES_ROUND" ),
        QStringLiteral( "SERVICE" ),
        QStringLiteral( "SERVICES" ),
        QStringLiteral( "SESSION" ),
        QStringLiteral( "SESSION_CONTEXT" ),
        QStringLiteral( "SESSION_USER" ),
        QStringLiteral( "SET" ),
        QStringLiteral( "SETOLAPMODEL" ),
        QStringLiteral( "SETS" ),
        QStringLiteral( "SHAPEFILE" ),
        QStringLiteral( "SHARE" ),
        QStringLiteral( "SHARED" ),
        QStringLiteral( "SHOW" ),
        QStringLiteral( "SIBLING" ),
        QStringLiteral( "SIDATTRIBUTE" ),
        QStringLiteral( "SIGNAL" ),
        QStringLiteral( "SIMPLE" ),
        QStringLiteral( "SITE" ),
        QStringLiteral( "SIZE" ),
        QStringLiteral( "SKETCH" ),
        QStringLiteral( "SKIP" ),
        QStringLiteral( "SMALLDECIMAL" ),
        QStringLiteral( "SMALLINT" ),
        QStringLiteral( "SNAP" ),
        QStringLiteral( "SNAPINT" ),
        QStringLiteral( "SNAPSHOT" ),
        QStringLiteral( "SOME" ),
        QStringLiteral( "SORT" ),
        QStringLiteral( "SOURCE" ),
        QStringLiteral( "SPACE" ),
        QStringLiteral( "SPARSIFY" ),
        QStringLiteral( "SPATIAL" ),
        QStringLiteral( "SPLITFACTOR" ),
        QStringLiteral( "SQL" ),
        QStringLiteral( "SQL_ERROR_CODE" ),
        QStringLiteral( "SQLSCRIPT" ),
        QStringLiteral( "SRID" ),
        QStringLiteral( "SSL" ),
        QStringLiteral( "STAB" ),
        QStringLiteral( "STANDARD" ),
        QStringLiteral( "START" ),
        QStringLiteral( "STATEMENT" ),
        QStringLiteral( "STATEMENT_NAME" ),
        QStringLiteral( "STATIC" ),
        QStringLiteral( "STATISTICS" ),
        QStringLiteral( "STOP" ),
        QStringLiteral( "STORAGE" ),
        QStringLiteral( "STORE" ),
        QStringLiteral( "STRING" ),
        QStringLiteral( "STRIP" ),
        QStringLiteral( "STRUCTURED" ),
        QStringLiteral( "STRUCTUREDPRIVILEGE" ),
        QStringLiteral( "SUB_TYPE" ),
        QStringLiteral( "SUBJECT" ),
        QStringLiteral( "SUBPARTITION" ),
        QStringLiteral( "SUBPARTITIONS" ),
        QStringLiteral( "SUBSCRIPTION" ),
        QStringLiteral( "SUBSTR" ),
        QStringLiteral( "SUBSTR_AFTER" ),
        QStringLiteral( "SUBSTR_BEFORE" ),
        QStringLiteral( "SUBSTR_REGEXPR" ),
        QStringLiteral( "SUBSTRING" ),
        QStringLiteral( "SUBSTRING_REGEXPR" ),
        QStringLiteral( "SUBTOTAL" ),
        QStringLiteral( "SUBTYPE" ),
        QStringLiteral( "SUCCESSFUL" ),
        QStringLiteral( "SUPPORT" ),
        QStringLiteral( "SUSPEND" ),
        QStringLiteral( "SYNC" ),
        QStringLiteral( "SYNCHRONOUS" ),
        QStringLiteral( "SYNONYM" ),
        QStringLiteral( "SYSLOG" ),
        QStringLiteral( "SYSTEM" ),
        QStringLiteral( "SYSTEM_TIME" ),
        QStringLiteral( "SYSTEMS" ),
        QStringLiteral( "SYSUUID" ),
        QStringLiteral( "T" ),
        QStringLiteral( "TABLE" ),
        QStringLiteral( "TABLE_NAME" ),
        QStringLiteral( "TABLES" ),
        QStringLiteral( "TABLESAMPLE" ),
        QStringLiteral( "TAKEOVER" ),
        QStringLiteral( "TARGET" ),
        QStringLiteral( "TASK" ),
        QStringLiteral( "TB" ),
        QStringLiteral( "TEMPLATEINDEX" ),
        QStringLiteral( "TEMPORARY" ),
        QStringLiteral( "TENANT" ),
        QStringLiteral( "TERM" ),
        QStringLiteral( "TEXT" ),
        QStringLiteral( "TEXTATTRIBUTE" ),
        QStringLiteral( "THEN" ),
        QStringLiteral( "THREAD" ),
        QStringLiteral( "THREADS" ),
        QStringLiteral( "THRESHOLD" ),
        QStringLiteral( "THROW_ERROR" ),
        QStringLiteral( "TIME" ),
        QStringLiteral( "TIMELINE" ),
        QStringLiteral( "TIMEOUT" ),
        QStringLiteral( "TIMESTAMP" ),
        QStringLiteral( "TIMEZONE" ),
        QStringLiteral( "TIMS_EXTRACT" ),
        QStringLiteral( "TINYINT" ),
        QStringLiteral( "TM_CATEGORIZE_KNN" ),
        QStringLiteral( "TM_GET_RELATED_DOCUMENTS" ),
        QStringLiteral( "TM_GET_RELATED_TERMS" ),
        QStringLiteral( "TM_GET_RELEVANT_DOCUMENTS" ),
        QStringLiteral( "TM_GET_RELEVANT_TERMS" ),
        QStringLiteral( "TM_GET_SUGGESTED_TERMS" ),
        QStringLiteral( "TO" ),
        QStringLiteral( "TO_BIGINT" ),
        QStringLiteral( "TO_BINARY" ),
        QStringLiteral( "TO_BLOB" ),
        QStringLiteral( "TO_CHAR" ),
        QStringLiteral( "TO_CLOB" ),
        QStringLiteral( "TO_DATE" ),
        QStringLiteral( "TO_DECIMAL" ),
        QStringLiteral( "TO_DOUBLE" ),
        QStringLiteral( "TO_INT" ),
        QStringLiteral( "TO_INTEGER" ),
        QStringLiteral( "TO_JSON_BOOLEAN" ),
        QStringLiteral( "TO_JSON_NUMBER" ),
        QStringLiteral( "TO_NCHAR" ),
        QStringLiteral( "TO_NCLOB" ),
        QStringLiteral( "TO_NUMBER" ),
        QStringLiteral( "TO_NVARCHAR" ),
        QStringLiteral( "TO_REAL" ),
        QStringLiteral( "TO_SECONDDATE" ),
        QStringLiteral( "TO_SMALLDECIMAL" ),
        QStringLiteral( "TO_SMALLINT" ),
        QStringLiteral( "TO_TIME" ),
        QStringLiteral( "TO_TIMESTAMP" ),
        QStringLiteral( "TO_TINYINT" ),
        QStringLiteral( "TO_VARBINARY" ),
        QStringLiteral( "TO_VARCHAR" ),
        QStringLiteral( "TOKEN" ),
        QStringLiteral( "TOLERANCE" ),
        QStringLiteral( "TOOLOPTION" ),
        QStringLiteral( "TOP" ),
        QStringLiteral( "TOPK" ),
        QStringLiteral( "TOTAL" ),
        QStringLiteral( "TRACE" ),
        QStringLiteral( "TRACEPROFILE" ),
        QStringLiteral( "TRACES" ),
        QStringLiteral( "TRAIL" ),
        QStringLiteral( "TRAILING" ),
        QStringLiteral( "TRANSACTION" ),
        QStringLiteral( "TRANSFORM" ),
        QStringLiteral( "TREE" ),
        QStringLiteral( "TREX" ),
        QStringLiteral( "TRIGGER" ),
        QStringLiteral( "TRIGGER_UPDATE_COLUMN" ),
        QStringLiteral( "TRIM" ),
        QStringLiteral( "TRUE" ),
        QStringLiteral( "TRUNCATE" ),
        QStringLiteral( "TRUST" ),
        QStringLiteral( "TS" ),
        QStringLiteral( "TYPE" ),
        QStringLiteral( "TYPENUMBERATTRIBUTE" ),
        QStringLiteral( "TYPETEXTATTRIBUTE" ),
        QStringLiteral( "UNAUTHORIZED" ),
        QStringLiteral( "UNBOUNDED" ),
        QStringLiteral( "UNCOMMITTED" ),
        QStringLiteral( "UNCONDITIONAL" ),
        QStringLiteral( "UNION" ),
        QStringLiteral( "UNIQUE" ),
        QStringLiteral( "UNIT" ),
        QStringLiteral( "UNITCONVERSION" ),
        QStringLiteral( "UNITCONVERSIONNAME" ),
        QStringLiteral( "UNKNOWN" ),
        QStringLiteral( "UNLOAD" ),
        QStringLiteral( "UNLOCK" ),
        QStringLiteral( "UNMASKED" ),
        QStringLiteral( "UNNEST" ),
        QStringLiteral( "UNPIN" ),
        QStringLiteral( "UNREGISTER" ),
        QStringLiteral( "UNSET" ),
        QStringLiteral( "UNSUCCESSFUL" ),
        QStringLiteral( "UNTIL" ),
        QStringLiteral( "UNUSED" ),
        QStringLiteral( "UP" ),
        QStringLiteral( "UPDATE" ),
        QStringLiteral( "UPSERT" ),
        QStringLiteral( "URL" ),
        QStringLiteral( "USAGE" ),
        QStringLiteral( "USE_C2R_CONV" ),
        QStringLiteral( "USE_COLUMN_JOIN_IMPLICIT_CAST" ),
        QStringLiteral( "USE_OLAP_PLAN" ),
        QStringLiteral( "USE_PREAGGR" ),
        QStringLiteral( "USE_QUERY_MATCH" ),
        QStringLiteral( "USE_R2C_CONV" ),
        QStringLiteral( "USE_TRANSFORMATION" ),
        QStringLiteral( "USE_UNION_OPT" ),
        QStringLiteral( "USEINITIALREORG" ),
        QStringLiteral( "USER" ),
        QStringLiteral( "USERGROUP" ),
        QStringLiteral( "USERS" ),
        QStringLiteral( "USING" ),
        QStringLiteral( "UTCTIMESTAMP" ),
        QStringLiteral( "UTF16" ),
        QStringLiteral( "UTF32" ),
        QStringLiteral( "UTF8" ),
        QStringLiteral( "VALID" ),
        QStringLiteral( "VALIDATE" ),
        QStringLiteral( "VALIDATED" ),
        QStringLiteral( "VALIDATION" ),
        QStringLiteral( "VALUE" ),
        QStringLiteral( "VALUES" ),
        QStringLiteral( "VARBINARY" ),
        QStringLiteral( "VARCHAR" ),
        QStringLiteral( "VARCHAR1" ),
        QStringLiteral( "VARCHAR2" ),
        QStringLiteral( "VARCHAR3" ),
        QStringLiteral( "VARIABLE" ),
        QStringLiteral( "VARYING" ),
        QStringLiteral( "VERIFY" ),
        QStringLiteral( "VERSION" ),
        QStringLiteral( "VERSIONING" ),
        QStringLiteral( "VERSIONS" ),
        QStringLiteral( "VERTEX" ),
        QStringLiteral( "VERTICAL" ),
        QStringLiteral( "VIEW" ),
        QStringLiteral( "VIEWATTRIBUTE" ),
        QStringLiteral( "VIRTUAL" ),
        QStringLiteral( "VOLUME" ),
        QStringLiteral( "VOLUMES" ),
        QStringLiteral( "WAIT" ),
        QStringLiteral( "WAITGRAPH" ),
        QStringLiteral( "WARNING" ),
        QStringLiteral( "WEEKDAY" ),
        QStringLiteral( "WEIGHT" ),
        QStringLiteral( "WHEN" ),
        QStringLiteral( "WHERE" ),
        QStringLiteral( "WHILE" ),
        QStringLiteral( "WHY_FOUND" ),
        QStringLiteral( "WILDCARD" ),
        QStringLiteral( "WINDOW" ),
        QStringLiteral( "WITH" ),
        QStringLiteral( "WITHIN" ),
        QStringLiteral( "WITHOUT" ),
        QStringLiteral( "WORK" ),
        QStringLiteral( "WORKAROUND" ),
        QStringLiteral( "WORKERGROUPS" ),
        QStringLiteral( "WORKLOAD" ),
        QStringLiteral( "WORKSPACE" ),
        QStringLiteral( "WRAPPER" ),
        QStringLiteral( "WRITE" ),
        QStringLiteral( "X" ),
        QStringLiteral( "X509" ),
        QStringLiteral( "XML" ),
        QStringLiteral( "XMLNAMESPACE" ),
        QStringLiteral( "XMLTABLE" ),
        QStringLiteral( "XTAB" ),
        QStringLiteral( "Y" ),
        QStringLiteral( "YEAR" ),
        QStringLiteral( "YTAB" ),
        QStringLiteral( "ZONE" ),
      }
    },
    {
      Qgis::SqlKeywordCategory::Aggregate,
      {
        QStringLiteral( "AUTO_CORR" ),
        QStringLiteral( "AVG" ),
        QStringLiteral( "CORR" ),
        QStringLiteral( "CORR_SPEARMAN" ),
        QStringLiteral( "COUNT" ),
        QStringLiteral( "CROSS_CORR" ),
        QStringLiteral( "DFT" ),
        QStringLiteral( "FIRST_VALUE" ),
        QStringLiteral( "LAST_VALUE" ),
        QStringLiteral( "MAX" ),
        QStringLiteral( "MEDIAN" ),
        QStringLiteral( "MIN" ),
        QStringLiteral( "NTH_VALUE" ),
        QStringLiteral( "STDDEV" ),
        QStringLiteral( "STDDEV_POP" ),
        QStringLiteral( "STDDEV_SAMP" ),
        QStringLiteral( "STRING_AGG" ),
        QStringLiteral( "SUM" ),
        QStringLiteral( "VAR" ),
        QStringLiteral( "VAR_POP" ),
        QStringLiteral( "VAR_SAMP" ),
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
        QStringLiteral( "BITCOUNT" ),
        QStringLiteral( "BITNOT" ),
        QStringLiteral( "BITOR" ),
        QStringLiteral( "BITSET" ),
        QStringLiteral( "BITUNSET" ),
        QStringLiteral( "BITXOR" ),
        QStringLiteral( "CEIL" ),
        QStringLiteral( "COS" ),
        QStringLiteral( "COSH" ),
        QStringLiteral( "COT" ),
        QStringLiteral( "EXP" ),
        QStringLiteral( "FLOOR" ),
        QStringLiteral( "LN" ),
        QStringLiteral( "LOG" ),
        QStringLiteral( "MOD" ),
        QStringLiteral( "NDIV0" ),
        QStringLiteral( "POWER" ),
        QStringLiteral( "RAND" ),
        QStringLiteral( "RAND_SECURE" ),
        QStringLiteral( "ROUND" ),
        QStringLiteral( "SIGN" ),
        QStringLiteral( "SIN" ),
        QStringLiteral( "SINH" ),
        QStringLiteral( "SQRT" ),
        QStringLiteral( "TAN" ),
        QStringLiteral( "TANH" ),
      }
    },
    {
      Qgis::SqlKeywordCategory::Geospatial,
      {
        QStringLiteral( "ST_AlphaShape" ),
        QStringLiteral( "ST_AlphaShapeAggr" ),
        QStringLiteral( "ST_AlphaShapeArea" ),
        QStringLiteral( "ST_AlphaShapeAreaAggr" ),
        QStringLiteral( "ST_AlphaShapeEdge" ),
        QStringLiteral( "ST_AlphaShapeEdgeAggr" ),
        QStringLiteral( "ST_AsBinary" ),
        QStringLiteral( "ST_AsEsriJSON" ),
        QStringLiteral( "ST_AsEWKB" ),
        QStringLiteral( "ST_AsEWKT" ),
        QStringLiteral( "ST_AsGeoJSON" ),
        QStringLiteral( "ST_AsSVG" ),
        QStringLiteral( "ST_AsSVGAggr" ),
        QStringLiteral( "ST_AsText" ),
        QStringLiteral( "ST_AsWKB" ),
        QStringLiteral( "ST_AsWKT" ),
        QStringLiteral( "ST_Boundary" ),
        QStringLiteral( "ST_Buffer" ),
        QStringLiteral( "ST_CircularString" ),
        QStringLiteral( "ST_Collect" ),
        QStringLiteral( "ST_CollectAggr" ),
        QStringLiteral( "ST_Contains" ),
        QStringLiteral( "ST_ConvexHull" ),
        QStringLiteral( "ST_ConvexHullAggr" ),
        QStringLiteral( "ST_CoordDim" ),
        QStringLiteral( "ST_CoveredBy" ),
        QStringLiteral( "ST_Covers" ),
        QStringLiteral( "ST_Crosses" ),
        QStringLiteral( "ST_Difference" ),
        QStringLiteral( "ST_Dimension" ),
        QStringLiteral( "ST_Disjoint" ),
        QStringLiteral( "ST_Distance" ),
        QStringLiteral( "ST_Envelope" ),
        QStringLiteral( "ST_EnvelopeAggr" ),
        QStringLiteral( "ST_EnvelopeAggr" ),
        QStringLiteral( "ST_Equals" ),
        QStringLiteral( "ST_Force2D" ),
        QStringLiteral( "ST_Force3DM" ),
        QStringLiteral( "ST_Force3DZ" ),
        QStringLiteral( "ST_Force4D" ),
        QStringLiteral( "ST_GeoHash" ),
        QStringLiteral( "ST_GeomFromEsriJSON" ),
        QStringLiteral( "ST_GeomFromEWKB" ),
        QStringLiteral( "ST_GeomFromEWKT" ),
        QStringLiteral( "ST_GeomFromGeoHash" ),
        QStringLiteral( "ST_GeomFromText" ),
        QStringLiteral( "ST_GeomFromWKB" ),
        QStringLiteral( "ST_GeomFromWKT" ),
        QStringLiteral( "ST_GeometryCollection" ),
        QStringLiteral( "ST_GeometryN" ),
        QStringLiteral( "ST_GeometryType" ),
        QStringLiteral( "ST_Intersection" ),
        QStringLiteral( "ST_IntersectionAggr" ),
        QStringLiteral( "ST_Intersects" ),
        QStringLiteral( "ST_IntersectsFilter" ),
        QStringLiteral( "ST_IntersectsRect" ),
        QStringLiteral( "ST_Is3D" ),
        QStringLiteral( "ST_IsEmpty" ),
        QStringLiteral( "ST_IsMeasured" ),
        QStringLiteral( "ST_IsSimple" ),
        QStringLiteral( "ST_IsValid" ),
        QStringLiteral( "ST_LineString" ),
        QStringLiteral( "ST_MultiLineString" ),
        QStringLiteral( "ST_MultiPoint" ),
        QStringLiteral( "ST_MultiPolygon" ),
        QStringLiteral( "ST_MMax" ),
        QStringLiteral( "ST_MMin" ),
        QStringLiteral( "ST_NumInteriorRing" ),
        QStringLiteral( "ST_NumInteriorRings" ),
        QStringLiteral( "ST_OrderingEquals" ),
        QStringLiteral( "ST_Overlaps" ),
        QStringLiteral( "ST_Perimeter" ),
        QStringLiteral( "ST_Point" ),
        QStringLiteral( "ST_PointOnSurface" ),
        QStringLiteral( "ST_Polygon" ),
        QStringLiteral( "ST_SquareGrid" ),
        QStringLiteral( "ST_RectangleGrid" ),
        QStringLiteral( "ST_RectangleGridBoundingBox" ),
        QStringLiteral( "ST_Relate" ),
        QStringLiteral( "ST_Rotate" ),
        QStringLiteral( "ST_Scale" ),
        QStringLiteral( "ST_Simplify" ),
        QStringLiteral( "ST_SnapToGrid" ),
        QStringLiteral( "ST_SRID" ),
        QStringLiteral( "ST_SymDifference" ),
        QStringLiteral( "ST_Touches" ),
        QStringLiteral( "ST_Transform" ),
        QStringLiteral( "ST_Translate" ),
        QStringLiteral( "ST_Translate3D" ),
        QStringLiteral( "ST_Union" ),
        QStringLiteral( "ST_UnionAggr" ),
        QStringLiteral( "ST_VoronoiCell" ),
        QStringLiteral( "ST_Within" ),
        QStringLiteral( "ST_WithinDistance" ),
        QStringLiteral( "ST_XMax" ),
        QStringLiteral( "ST_XMin" ),
        QStringLiteral( "ST_YMax" ),
        QStringLiteral( "ST_YMin" ),
        QStringLiteral( "ST_ZMax" ),
        QStringLiteral( "ST_ZMin" ),
      }
    }
  } );
}

QVariantList QgsHanaEmptyProviderResultIterator::nextRowPrivate()
{
  return QVariantList();
}

bool QgsHanaEmptyProviderResultIterator::hasNextRowPrivate() const
{
  return false;
}
