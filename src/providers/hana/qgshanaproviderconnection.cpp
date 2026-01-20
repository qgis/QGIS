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

#include <chrono>
#include <odbc/PreparedStatement.h>

#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanaprimarykeys.h"
#include "qgshanaprovider.h"
#include "qgshanaresultset.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"

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
  // cppcheck-suppress unsignedLessThanZero
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
  mProviderKey = u"hana"_s;
  QgsHanaSettings settings( name, true );
  setUri( settings.toDataSourceUri().uri( false ) );
  setCapabilities();
}

QgsHanaProviderConnection::QgsHanaProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = u"hana"_s;
  setCapabilities();
}

void QgsHanaProviderConnection::setCapabilities()
{
  mGeometryColumnCapabilities = {
    //GeometryColumnCapability::Curves, not fully supported yet
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart
  };
  mSqlLayerDefinitionCapabilities = {
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

  mCapabilities = {
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
        if ( objType == "SYSTEMPRIVILEGE"_L1 )
        {
          if ( privType == "CREATE SCHEMA"_L1 )
            mCapabilities |= Capability::CreateSchema | Capability::DropSchema | Capability::RenameSchema;
          else if ( privType == "CATALOG READ"_L1 || privType == "DATA ADMIN"_L1 )
            mCapabilities |= Capability::Schemas | Capability::Tables | Capability::TableExists;
        }
        else if ( objType == "TABLE"_L1 || objType == "VIEW"_L1 )
        {
          if ( privType == "SELECT"_L1 )
          {
            QString schemaName = rsPrivileges->getString( 3 );
            QString objName = rsPrivileges->getString( 4 );

            if ( schemaName == "SYS"_L1 && objName == "SCHEMAS"_L1 )
              mCapabilities |= Capability::Schemas;
            else if ( objName == "TABLE_COLUMNS"_L1 )
              mCapabilities |= Capability::Tables | Capability::TableExists;
          }
        }
      }
      rsPrivileges->close();

      return;
    }
    catch ( const QgsHanaException &ex )
    {
      QgsMessageLog::logMessage( QObject::tr( "Unable to retrieve user privileges: %1" ).arg( ex.what() ), QObject::tr( "SAP HANA" ) );
    }
  }

  // We enable all capabilities, if we were not able to retrieve them from the database.
  mCapabilities |= Capability::CreateSchema | Capability::DropSchema | Capability::RenameSchema | Capability::Schemas | Capability::Tables | Capability::TableExists;
}

void QgsHanaProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column if it's not aspatial
  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( u"geometryColumn"_s, u"geom"_s ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  QString createdLayerUri;
  Qgis::VectorExportResult res = QgsHanaProvider::createEmptyLayer(
    newUri.uri(),
    fields,
    wkbType,
    srs,
    overwrite,
    &map,
    createdLayerUri,
    &errCause,
    options
  );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsHanaProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  QgsDataSourceUri destUri( uri() );

  destUri.setTable( options.layerName );
  destUri.setSchema( options.schema );
  destUri.setGeometryColumn( options.wkbType != Qgis::WkbType::NoGeometry ? ( options.geometryColumn.isEmpty() ? u"geom"_s : options.geometryColumn ) : QString() );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    if ( options.primaryKeyColumns.length() > 1 )
    {
      QgsMessageLog::logMessage( u"Multiple primary keys are not supported by HANA, ignoring"_s, QString(), Qgis::MessageLevel::Info );
    }
    destUri.setKeyColumn( options.primaryKeyColumns.at( 0 ) );
  }

  providerOptions.clear();

  return destUri.uri( false );
}

QString QgsHanaProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const TableProperty tableInfo { table( schema, name ) };

  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  dsUri.setGeometryColumn( tableInfo.geometryColumn() );
  return dsUri.uri( false );
}

void QgsHanaProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  const TableProperty tableInfo = table( schema, name );
  if ( tableInfo.flags().testFlag( TableFlag::View ) )
    executeSqlStatement( u"DROP VIEW %1.%2"_s
                           .arg( QgsHanaUtils::quotedIdentifier( schema ), QgsHanaUtils::quotedIdentifier( name ) ) );
  else
    executeSqlStatement( u"DROP TABLE %1.%2"_s
                           .arg( QgsHanaUtils::quotedIdentifier( schema ), QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  executeSqlStatement( u"RENAME TABLE %1.%2 TO %1.%3"_s
                         .arg( QgsHanaUtils::quotedIdentifier( schema ), QgsHanaUtils::quotedIdentifier( name ), QgsHanaUtils::quotedIdentifier( newName ) ) );
}

void QgsHanaProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlStatement( u"CREATE SCHEMA %1"_s
                         .arg( QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::dropSchema( const QString &name, bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlStatement( u"DROP SCHEMA %1 %2"_s
                         .arg( QgsHanaUtils::quotedIdentifier( name ), force ? u"CASCADE"_s : QString() ) );
}

void QgsHanaProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlStatement( u"RENAME SCHEMA %1 TO %2"_s
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
  const TableFlags &flags, const std::function<bool( const QgsHanaLayerProperty &layer )> &layerFilter
) const
{
  checkCapability( Capability::Tables );

  QgsHanaConnectionRef conn = createConnection();
  QList<QgsHanaProviderConnection::TableProperty> tables;

  try
  {
    const bool aspatial { !flags || flags.testFlag( TableFlag::Aspatial ) };
    const QVector<QgsHanaLayerProperty> layers = conn->getLayersFull( schema, aspatial, false, layerFilter );
    tables.reserve( layers.size() );
    for ( const QgsHanaLayerProperty &layerInfo : layers )
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
      if ( !flags || ( prFlags & flags ) )
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
        else // Fetch and set the real pks
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

QgsAbstractDatabaseProviderConnection::TableProperty QgsHanaProviderConnection::table( const QString &schema, const QString &table, QgsFeedback * ) const
{
  const QString geometryColumn = QgsDataSourceUri( uri() ).geometryColumn();
  auto layerFilter = [&table, &geometryColumn]( const QgsHanaLayerProperty &layer ) {
    return layer.tableName == table && ( geometryColumn.isEmpty() || layer.geometryColName == geometryColumn );
  };
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tablesWithFilter( schema, TableFlags(), layerFilter ) };
  if ( constTables.empty() )
    throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                            .arg( table, schema ) );
  return constTables[0];
}

QList<QgsHanaProviderConnection::TableProperty> QgsHanaProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback * ) const
{
  return tablesWithFilter( schema, flags );
}

QStringList QgsHanaProviderConnection::schemas() const
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

QgsFields QgsHanaProviderConnection::fields( const QString &schema, const QString &table, QgsFeedback * ) const
{
  QgsHanaConnectionRef conn = createConnection();
  const QString geometryColumn = QgsDataSourceUri( uri() ).geometryColumn();
  try
  {
    QgsFields fields;
    auto processField = [&geometryColumn, &fields]( const AttributeField &field ) {
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
  return QgsApplication::getThemeIcon( u"mIconHana.svg"_s );
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

  QgsDataSourceUri tUri( uri() );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( QgsHanaPrimaryKeyUtils::buildUriKey( options.primaryKeyColumns ) );
    tUri.setTable( u"(%1)"_s.arg( sanitizeSqlForQueryLayer( options.sql ) ) );
  }
  else
  {
    int pkId { 0 };
    while ( options.sql.contains( u"_uid%1_"_s.arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId++;
    }
    tUri.setKeyColumn( u"_uid%1_"_s.arg( pkId ) );

    int sqlId { 0 };
    while ( options.sql.contains( u"_subq_%1_"_s.arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId++;
    }
    tUri.setTable( u"(SELECT ROW_NUMBER() over () AS \"_uid%1_\", * FROM (%2\n) AS \"_subq_%3_\"\n)"_s.arg( QString::number( pkId ), options.sql, QString::number( sqlId ) ) );
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer { tUri.uri( false ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey(), vectorLayerOptions };
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
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : u"SELECT * FROM %1"_s.arg( tUri.quotedTablename() );
  return options;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsHanaProviderConnection::sqlDictionary()
{
  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
    { { Qgis::SqlKeywordCategory::Keyword,
        {
          u"ABAP_CHAR"_s,
          u"ABAP_DATE"_s,
          u"ABAP_DECFLOAT16"_s,
          u"ABAP_DECFLOAT34"_s,
          u"ABAP_FLOAT"_s,
          u"ABAP_HEX"_s,
          u"ABAP_INT"_s,
          u"ABAP_INT1"_s,
          u"ABAP_INT2"_s,
          u"ABAP_NUM"_s,
          u"ABAP_PACKED"_s,
          u"ABAP_STRING"_s,
          u"ABAP_TIME"_s,
          u"ABAP_XSTRING"_s,
          u"ABAPITAB"_s,
          u"ABAPSTRUCT"_s,
          u"ABSOLUTE"_s,
          u"ABSTRACT"_s,
          u"ACCESS"_s,
          u"ACCURACY"_s,
          u"ACKNOWLEDGED"_s,
          u"ACTION"_s,
          u"ACTIONS"_s,
          u"ACTIVATE"_s,
          u"ADAPTER"_s,
          u"ADD"_s,
          u"ADD_DAYS"_s,
          u"ADD_MONTHS"_s,
          u"ADD_SECONDS"_s,
          u"ADD_YEARS"_s,
          u"ADDRESS"_s,
          u"ADMIN"_s,
          u"ADOPT"_s,
          u"AFL"_s,
          u"AFTER"_s,
          u"AGENT"_s,
          u"ALERT"_s,
          u"ALGORITHM"_s,
          u"ALIAS"_s,
          u"ALIGNMENT"_s,
          u"ALL"_s,
          u"ALLATTRSFREESTYLERELEVANT"_s,
          u"ALLOCATOR"_s,
          u"ALLOCATORS"_s,
          u"ALLOWED"_s,
          u"ALPHANUM"_s,
          u"ALPHANUM_IDENTIFIER"_s,
          u"ALTER"_s,
          u"ALTERNATE"_s,
          u"ALWAYS"_s,
          u"ANALYSIS"_s,
          u"ANALYTIC"_s,
          u"ANALYZE"_s,
          u"ANCHOR"_s,
          u"AND"_s,
          u"ANGULAR"_s,
          u"ANNOTATE"_s,
          u"ANNOTATIONS"_s,
          u"ANONYMIZATION"_s,
          u"ANY"_s,
          u"APPEND"_s,
          u"APPLICATION"_s,
          u"APPLICATION_TIME"_s,
          u"APPLICATIONUSER"_s,
          u"APPLY"_s,
          u"APPLY_FILTER"_s,
          u"ARCHIVE"_s,
          u"AREA"_s,
          u"ARRAY"_s,
          u"ARRAY_AGG"_s,
          u"AS"_s,
          u"ASC"_s,
          u"ASSERTION"_s,
          u"ASSOCIATION"_s,
          u"ASSOCIATIONS"_s,
          u"ASYNC"_s,
          u"ASYNCHRONOUS"_s,
          u"AT"_s,
          u"ATOMIC"_s,
          u"ATTACH"_s,
          u"ATTEMPTS"_s,
          u"ATTRIBUTE"_s,
          u"ATTRIBUTEMAPPING"_s,
          u"AUDIT"_s,
          u"AUDITING"_s,
          u"AUTHENTICATION"_s,
          u"AUTHORIZATION"_s,
          u"AUTO"_s,
          u"AUTOCOMMIT"_s,
          u"AUTOMATIC"_s,
          u"AUTOMERGE"_s,
          u"AXIS"_s,
          u"BACKINT"_s,
          u"BACKUP"_s,
          u"BACKUP_ID"_s,
          u"BACKUPS"_s,
          u"BALANCE"_s,
          u"BASIC"_s,
          u"BATCH"_s,
          u"BEFORE"_s,
          u"BEGIN"_s,
          u"BERNOULLI"_s,
          u"BEST"_s,
          u"BETWEEN"_s,
          u"BIGINT"_s,
          u"BINARY"_s,
          u"BINARYCONSTRAINT"_s,
          u"BIND_AS_PARAMETER"_s,
          u"BIND_AS_VALUE"_s,
          u"BIND_BIGINT"_s,
          u"BIND_CHAR"_s,
          u"BIND_DECIMAL"_s,
          u"BIND_DOUBLE"_s,
          u"BIND_NCHAR"_s,
          u"BIND_REAL"_s,
          u"BITS"_s,
          u"BLOB"_s,
          u"BOOLEAN"_s,
          u"BOTH"_s,
          u"BOUNDARY"_s,
          u"BREAK"_s,
          u"BREAKUP"_s,
          u"BTREE"_s,
          u"BUCKETS"_s,
          u"BUFFER"_s,
          u"BUILTIN"_s,
          u"BULK"_s,
          u"BUSINESS"_s,
          u"BY"_s,
          u"CA"_s,
          u"CACHE"_s,
          u"CALCULATEDKEYFIGURE"_s,
          u"CALCULATEDVIEWATTRIBUTE"_s,
          u"CALCULATION"_s,
          u"CALENDAR"_s,
          u"CALL"_s,
          u"CALLS"_s,
          u"CALLSTACK"_s,
          u"CANCEL"_s,
          u"CAPTURE"_s,
          u"CASCADE"_s,
          u"CASE"_s,
          u"CAST"_s,
          u"CATALOG"_s,
          u"CDS"_s,
          u"CE_CALC"_s,
          u"CE_JOIN"_s,
          u"CE_PROJECTION"_s,
          u"CELL"_s,
          u"CELLS"_s,
          u"CENTER"_s,
          u"CERTIFICATE"_s,
          u"CERTIFICATES"_s,
          u"CHANGE"_s,
          u"CHANGES"_s,
          u"CHAR"_s,
          u"CHARACTER"_s,
          u"CHARACTERISTIC"_s,
          u"CHECK"_s,
          u"CHECKPOINT"_s,
          u"CHILDRENATTRIBUTE"_s,
          u"CLAIM"_s,
          u"CLASS"_s,
          u"CLEAR"_s,
          u"CLIENT"_s,
          u"CLIENTPKI"_s,
          u"CLIENTSIDE"_s,
          u"CLIP"_s,
          u"CLOB"_s,
          u"CLOSE"_s,
          u"CLUSTER"_s,
          u"CLUSTERING"_s,
          u"CLUSTERS"_s,
          u"COALESCE"_s,
          u"CODE"_s,
          u"CODEPAGE"_s,
          u"COLLATE"_s,
          u"COLLATION"_s,
          u"COLLECTION"_s,
          u"COLUMN"_s,
          u"COLUMN_VIEW_ESTIMATION"_s,
          u"COLUMNS"_s,
          u"COMMENT"_s,
          u"COMMIT"_s,
          u"COMMITTED"_s,
          u"COMPACT"_s,
          u"COMPATIBILITY"_s,
          u"COMPLETE"_s,
          u"COMPONENT"_s,
          u"COMPONENTS"_s,
          u"COMPRESSED"_s,
          u"COMPRESSION"_s,
          u"COMPUTE"_s,
          u"COMPUTE_REPLICA_COUNT"_s,
          u"COMPUTE_REPLICA_TYPE"_s,
          u"CONCAT"_s,
          u"CONDITION"_s,
          u"CONDITIONAL"_s,
          u"CONFIG"_s,
          u"CONFIGURATION"_s,
          u"CONFIGURE"_s,
          u"CONNECT"_s,
          u"CONNECTION"_s,
          u"CONSISTENCY"_s,
          u"CONSTANT"_s,
          u"CONSTRAINT"_s,
          u"CONSTRAINTS"_s,
          u"CONTAINS"_s,
          u"CONTENT"_s,
          u"CONTEXT"_s,
          u"CONTINUE"_s,
          u"CONTROL"_s,
          u"CONTROLLED"_s,
          u"CONV"_s,
          u"CONVERT"_s,
          u"COORDINATE"_s,
          u"COPY"_s,
          u"COREFILE"_s,
          u"CORRELATION"_s,
          u"COVERAGE"_s,
          u"CPBTREE"_s,
          u"CPU"_s,
          u"CREATE"_s,
          u"CREATION"_s,
          u"CREATOR"_s,
          u"CREDENTIAL"_s,
          u"CRITICAL"_s,
          u"CRON"_s,
          u"CROSS"_s,
          u"CS_DATE"_s,
          u"CS_DAYDATE"_s,
          u"CS_DECIMAL_FLOAT"_s,
          u"CS_DOUBLE"_s,
          u"CS_FIXED"_s,
          u"CS_FIXEDSTRING"_s,
          u"CS_FLOAT"_s,
          u"CS_GEOMETRY"_s,
          u"CS_INT"_s,
          u"CS_LONGDATE"_s,
          u"CS_POINT"_s,
          u"CS_POINTZ"_s,
          u"CS_RAW"_s,
          u"CS_SDFLOAT"_s,
          u"CS_SECONDDATE"_s,
          u"CS_SECONDTIME"_s,
          u"CS_SHORTTEXT"_s,
          u"CS_STRING"_s,
          u"CS_TEXT"_s,
          u"CS_TEXT_AE"_s,
          u"CS_TIME"_s,
          u"CSV"_s,
          u"CUBE"_s,
          u"CUME_DIST"_s,
          u"CURDATE"_s,
          u"CURRENT"_s,
          u"CURRENT_CONNECTION"_s,
          u"CURRENT_DATABASE"_s,
          u"CURRENT_DATE"_s,
          u"CURRENT_SCHEMA"_s,
          u"CURRENT_TIME"_s,
          u"CURRENT_TIMESTAMP"_s,
          u"CURRENT_TRANSACTION_ISOLATION_LEVEL"_s,
          u"CURRENT_USER"_s,
          u"CURRENT_UTCDATE"_s,
          u"CURRENT_UTCTIME"_s,
          u"CURRENT_UTCTIMESTAMP"_s,
          u"CURRVAL"_s,
          u"CURSOR"_s,
          u"CURTIME"_s,
          u"CURVE"_s,
          u"CYCLE"_s,
          u"D"_s,
          u"DATA"_s,
          u"DATABASE"_s,
          u"DATABASES"_s,
          u"DATAPROVISIONING"_s,
          u"DATASET"_s,
          u"DATASOURCE"_s,
          u"DATAVOLUME"_s,
          u"DATE"_s,
          u"DATEFORMAT"_s,
          u"DATETIME"_s,
          u"DATS_EXTRACT"_s,
          u"DAY"_s,
          u"DAYDATE"_s,
          u"DAYOFMONTH"_s,
          u"DAYOFWEEK"_s,
          u"DAYS_BETWEEN"_s,
          u"DBSCAN"_s,
          u"DBSPACE"_s,
          u"DDIC_ACCP"_s,
          u"DDIC_CDAY"_s,
          u"DDIC_CHAR"_s,
          u"DDIC_CLNT"_s,
          u"DDIC_CUKY"_s,
          u"DDIC_CURR"_s,
          u"DDIC_D16D"_s,
          u"DDIC_D16R"_s,
          u"DDIC_D16S"_s,
          u"DDIC_D34D"_s,
          u"DDIC_D34R"_s,
          u"DDIC_D34S"_s,
          u"DDIC_DATS"_s,
          u"DDIC_DAY"_s,
          u"DDIC_DEC"_s,
          u"DDIC_FLTP"_s,
          u"DDIC_GUID"_s,
          u"DDIC_INT1"_s,
          u"DDIC_INT2"_s,
          u"DDIC_INT4"_s,
          u"DDIC_INT8"_s,
          u"DDIC_LANG"_s,
          u"DDIC_LCHR"_s,
          u"DDIC_LRAW"_s,
          u"DDIC_MIN"_s,
          u"DDIC_MON"_s,
          u"DDIC_NUMC"_s,
          u"DDIC_PREC"_s,
          u"DDIC_QUAN"_s,
          u"DDIC_RAW"_s,
          u"DDIC_RSTR"_s,
          u"DDIC_SEC"_s,
          u"DDIC_SRST"_s,
          u"DDIC_SSTR"_s,
          u"DDIC_STRG"_s,
          u"DDIC_STXT"_s,
          u"DDIC_TEXT"_s,
          u"DDIC_TIMS"_s,
          u"DDIC_UNIT"_s,
          u"DDIC_UTCL"_s,
          u"DDIC_UTCM"_s,
          u"DDIC_UTCS"_s,
          u"DDIC_VARC"_s,
          u"DDIC_WEEK"_s,
          u"DDL"_s,
          u"DEACTIVATE"_s,
          u"DEBUG"_s,
          u"DEBUGGER"_s,
          u"DEC"_s,
          u"DECIMAL"_s,
          u"DECLARE"_s,
          u"DEFAULT"_s,
          u"DEFAULTVIEW"_s,
          u"DEFERRED"_s,
          u"DEFINER"_s,
          u"DEFINITION"_s,
          u"DEFRAGMENT"_s,
          u"DELAY"_s,
          u"DELETE"_s,
          u"DELIMITED"_s,
          u"DELTA"_s,
          u"DENSE_RANK"_s,
          u"DEPENDENCIES"_s,
          u"DEPENDENCY"_s,
          u"DEPENDENT"_s,
          u"DEPTH"_s,
          u"DESC"_s,
          u"DESCRIPTION"_s,
          u"DETACH"_s,
          u"DETECTION"_s,
          u"DETERMINISTIC"_s,
          u"DEV_CS_ONLY"_s,
          u"DEV_NO_SEMI_JOIN_REDUCTION_TARGET"_s,
          u"DEV_PROC_CE"_s,
          u"DEV_PROC_INLINE"_s,
          u"DEV_PROC_L"_s,
          u"DEV_PROC_NO_INLINE"_s,
          u"DEV_PROC_SE_FT"_s,
          u"DEV_RS_ONLY"_s,
          u"DEV_SEMI_JOIN_REDUCTION_TARGET"_s,
          u"DEVELOPMENT"_s,
          u"DIFFERENTIAL"_s,
          u"DISABLE"_s,
          u"DISABLED"_s,
          u"DISCONNECT"_s,
          u"DISK"_s,
          u"DISTANCE"_s,
          u"DISTINCT"_s,
          u"DISTRIBUTE"_s,
          u"DISTRIBUTION"_s,
          u"DO"_s,
          u"DOCUMENT"_s,
          u"DOCUMENTS"_s,
          u"DOUBLE"_s,
          u"DPSERVER"_s,
          u"DROP"_s,
          u"DTAB"_s,
          u"DUPLICATES"_s,
          u"DURATION"_s,
          u"DW_OPTIMIZED"_s,
          u"DYNAMIC"_s,
          u"DYNAMIC_RANGE_THRESHOLD"_s,
          u"EACH"_s,
          u"EARTH"_s,
          u"EDGE"_s,
          u"ELEMENTS"_s,
          u"ELLIPSOID"_s,
          u"ELSE"_s,
          u"ELSEIF"_s,
          u"EMAIL"_s,
          u"EMERGENCY"_s,
          u"EMPTY"_s,
          u"ENABLE"_s,
          u"ENABLED"_s,
          u"ENCLOSED"_s,
          u"ENCODED"_s,
          u"ENCODING"_s,
          u"ENCRYPTED"_s,
          u"ENCRYPTION"_s,
          u"END"_s,
          u"ENFORCED"_s,
          u"ENTITY"_s,
          u"ENTRY"_s,
          u"EPM"_s,
          u"EPS"_s,
          u"EQ"_s,
          u"EQUIDISTANT"_s,
          u"ERROR"_s,
          u"ES"_s,
          u"ESCAPE"_s,
          u"ESTIMATE"_s,
          u"ESTIMATED"_s,
          u"ESTIMATION"_s,
          u"EVENT"_s,
          u"EVENTS"_s,
          u"EVERY"_s,
          u"EVICTION"_s,
          u"EXACT"_s,
          u"EXCEPT"_s,
          u"EXCEPTION"_s,
          u"EXCLUDE"_s,
          u"EXCLUDED"_s,
          u"EXCLUSIVE"_s,
          u"EXEC"_s,
          u"EXECUTE"_s,
          u"EXECUTION"_s,
          u"EXISTING"_s,
          u"EXISTS"_s,
          u"EXIT"_s,
          u"EXPLAIN"_s,
          u"EXPLICIT"_s,
          u"EXPORT"_s,
          u"EXPRESSION"_s,
          u"EXPRESSIONFLAGS"_s,
          u"EXTENDED"_s,
          u"EXTERNAL"_s,
          u"EXTERNAL_BACKUP_ID"_s,
          u"EXTERNALATTRIBUTE"_s,
          u"EXTERNALLY"_s,
          u"EXTRACT"_s,
          u"FACT"_s,
          u"FACTOR"_s,
          u"FAIL"_s,
          u"FALLBACK"_s,
          u"FALSE"_s,
          u"FAST"_s,
          u"FBO"_s,
          u"FETCH"_s,
          u"FIELD"_s,
          u"FILE"_s,
          u"FILL"_s,
          u"FILLFACTOR"_s,
          u"FILTER"_s,
          u"FINALIZE"_s,
          u"FINISH"_s,
          u"FIRST"_s,
          u"FLAG"_s,
          u"FLAGS"_s,
          u"FLATTEN"_s,
          u"FLATTENING"_s,
          u"FLOAT"_s,
          u"FLUSH"_s,
          u"FN"_s,
          u"FOLLOWING"_s,
          u"FOLLOWS"_s,
          u"FOR"_s,
          u"FORCE"_s,
          u"FORCE_FIRST_PASSWORD_CHANGE"_s,
          u"FORCE_RESULT_CACHE_REFRESH"_s,
          u"FOREIGN"_s,
          u"FOREVER"_s,
          u"FORGY"_s,
          u"FORMAT"_s,
          u"FORMULA"_s,
          u"FREESTYLESEARCHATTRIBUTE"_s,
          u"FREESTYLETRANSLATION"_s,
          u"FROM"_s,
          u"FULL"_s,
          u"FULLTEXT"_s,
          u"FUNCTION"_s,
          u"FUNCTION_PROFILER"_s,
          u"FUZZINESSTHRESHOLD"_s,
          u"FUZZY"_s,
          u"GB"_s,
          u"GENERATED"_s,
          u"GET_NUM_SERVERS"_s,
          u"GLOBAL"_s,
          u"GLOBALDICT"_s,
          u"GLOBALLY"_s,
          u"GOTO"_s,
          u"GRANT"_s,
          u"GRANTED"_s,
          u"GRAPH"_s,
          u"GREATEST"_s,
          u"GRID"_s,
          u"GROUP"_s,
          u"GROUP_NAME"_s,
          u"GROUP_TYPE"_s,
          u"GROUPING"_s,
          u"GROUPING_FILTER"_s,
          u"GROUPING_ID"_s,
          u"GROUPS"_s,
          u"GT"_s,
          u"GUID"_s,
          u"HANDLED"_s,
          u"HANDOVER"_s,
          u"HAS"_s,
          u"HASANYPRIVILEGES"_s,
          u"HASH"_s,
          u"HASH_JOIN"_s,
          u"HASSYSTEMPRIVILEGE"_s,
          u"HAVING"_s,
          u"HEXAGON"_s,
          u"HEXTOBIN"_s,
          u"HIDDEN"_s,
          u"HIERARCHY"_s,
          u"HIERARCHY_ANCESTORS"_s,
          u"HIERARCHY_ANCESTORS_AGGREGATE"_s,
          u"HIERARCHY_DESCENDANTS"_s,
          u"HIERARCHY_DESCENDANTS_AGGREGATE"_s,
          u"HIERARCHY_LEVELED"_s,
          u"HIERARCHY_SIBLINGS"_s,
          u"HIERARCHY_SPANTREE"_s,
          u"HIERARCHY_TEMPORAL"_s,
          u"HIERARCHYCHARACTERISTIC"_s,
          u"HIERARCHYINDEX"_s,
          u"HIERARCHYNAME"_s,
          u"HIGH"_s,
          u"HIGHLIGHTED"_s,
          u"HILBERT"_s,
          u"HINT"_s,
          u"HISTOGRAM"_s,
          u"HISTORY"_s,
          u"HOLD"_s,
          u"HORIZONTAL"_s,
          u"HOST"_s,
          u"HOSTS"_s,
          u"HOUR"_s,
          u"HOUSENUMBER"_s,
          u"ID"_s,
          u"IDENTICAL"_s,
          u"IDENTIFIED"_s,
          u"IDENTIFIER"_s,
          u"IDENTITY"_s,
          u"IF"_s,
          u"IFNULL"_s,
          u"IGNORE"_s,
          u"IMMEDIATE"_s,
          u"IMPORT"_s,
          u"IN"_s,
          u"INCLUDE"_s,
          u"INCREMENT"_s,
          u"INCREMENTAL"_s,
          u"INDEPENDENT"_s,
          u"INDEX"_s,
          u"INDEX_JOIN"_s,
          u"INDEX_UNION"_s,
          u"INDEXALIAS"_s,
          u"INDEXED"_s,
          u"INDEXES"_s,
          u"INDEXID"_s,
          u"INDEXTYPE"_s,
          u"INDIVIDUAL"_s,
          u"INFO"_s,
          u"INHERITS"_s,
          u"INIFILE"_s,
          u"INIT"_s,
          u"INITIAL"_s,
          u"INITIAL_PARTITIONS"_s,
          u"INITIALLY"_s,
          u"INLINE"_s,
          u"INNER"_s,
          u"INOUT"_s,
          u"INSENSITIVE"_s,
          u"INSERT"_s,
          u"INSTEAD"_s,
          u"INSTR"_s,
          u"INT"_s,
          u"INTEGER"_s,
          u"INTERNAL"_s,
          u"INTERSECT"_s,
          u"INTERVAL"_s,
          u"INTO"_s,
          u"INVALID"_s,
          u"INVERSE"_s,
          u"INVERTED"_s,
          u"INVOKER"_s,
          u"IS"_s,
          u"IS_EMPTY"_s,
          u"ISAUTHORIZED"_s,
          u"ISMEMORYINDEX"_s,
          u"ISOLATION"_s,
          u"ISSUER"_s,
          u"ISTOTAL"_s,
          u"ISTRANSACTIONAL"_s,
          u"JOB"_s,
          u"JOIN"_s,
          u"JOINCONDITION"_s,
          u"JOININDEX"_s,
          u"JOININDEXESTIMATION"_s,
          u"JOININDEXTYPE"_s,
          u"JOINPATH"_s,
          u"JSON"_s,
          u"JSON_QUERY"_s,
          u"JSON_TABLE"_s,
          u"JSON_VALUE"_s,
          u"JWT"_s,
          u"KB"_s,
          u"KEEP"_s,
          u"KERBEROS"_s,
          u"KERNEL"_s,
          u"KERNELTRACE"_s,
          u"KEY"_s,
          u"KEYATTRIBUTE"_s,
          u"KEYCOPY"_s,
          u"KEYFIGURE"_s,
          u"KEYPAIR"_s,
          u"KEYS"_s,
          u"KEYVALUE"_s,
          u"KMEANS"_s,
          u"L"_s,
          u"LABEL"_s,
          u"LAG"_s,
          u"LANGUAGE"_s,
          u"LAST"_s,
          u"LAST_DAY"_s,
          u"LATENCY"_s,
          u"LATERAL"_s,
          u"LAYOUT"_s,
          u"LDAP"_s,
          u"LEAD"_s,
          u"LEADING"_s,
          u"LEAF"_s,
          u"LEAST"_s,
          u"LEAVES"_s,
          u"LEFT"_s,
          u"LENGTH"_s,
          u"LENGTHB"_s,
          u"LEVEL"_s,
          u"LEVELNUMBERATTRIBUTE"_s,
          u"LEVELS"_s,
          u"LEVELTEXTATTRIBUTE"_s,
          u"LIBRARY"_s,
          u"LICENSE"_s,
          u"LIFETIME"_s,
          u"LIKE"_s,
          u"LIKE_REGEXPR"_s,
          u"LIMIT"_s,
          u"LINE"_s,
          u"LINEAR"_s,
          u"LINKED"_s,
          u"LIST"_s,
          u"LOAD"_s,
          u"LOAD_HISTORY"_s,
          u"LOADABLE"_s,
          u"LOB"_s,
          u"LOCAL"_s,
          u"LOCALE"_s,
          u"LOCATE"_s,
          u"LOCATE_REGEXPR"_s,
          u"LOCATION"_s,
          u"LOCATIONS"_s,
          u"LOCK"_s,
          u"LOCKED"_s,
          u"LOG"_s,
          u"LOGFLUSH"_s,
          u"LOGGING"_s,
          u"LONGDATE"_s,
          u"LOOKUP"_s,
          u"LOOP"_s,
          u"LOOPBACK"_s,
          u"LPAD"_s,
          u"LTRIM"_s,
          u"MACROS"_s,
          u"MAIN"_s,
          u"MAJOR"_s,
          u"MANAGEMENT"_s,
          u"MANUAL"_s,
          u"MANY"_s,
          u"MAP"_s,
          u"MAP_MERGE"_s,
          u"MAP_REDUCE"_s,
          u"MAPPING"_s,
          u"MASK"_s,
          u"MATCHED"_s,
          u"MATCHES"_s,
          u"MATCHING"_s,
          u"MAXITERATIONS"_s,
          u"MAXVALUE"_s,
          u"MB"_s,
          u"MDRS_TEST"_s,
          u"MDX"_s,
          u"MEASURE"_s,
          u"MEASURES"_s,
          u"MEDIUM"_s,
          u"MEMBER"_s,
          u"MEMORY"_s,
          u"MERGE"_s,
          u"MESSAGING"_s,
          u"META"_s,
          u"METADATA"_s,
          u"MIGRATE"_s,
          u"MIME"_s,
          u"MIN_ROWS_FOR_PARTITIONING"_s,
          u"MINING"_s,
          u"MINOR"_s,
          u"MINPTS"_s,
          u"MINUS"_s,
          u"MINUTE"_s,
          u"MINUTES"_s,
          u"MINVALUE"_s,
          u"MISSING"_s,
          u"MODE"_s,
          u"MODEL"_s,
          u"MODIFIED"_s,
          u"MODIFY"_s,
          u"MODULE"_s,
          u"MONITOR"_s,
          u"MONITORING"_s,
          u"MONTH"_s,
          u"MOVABLE"_s,
          u"MOVE"_s,
          u"MULTIPARENT"_s,
          u"MULTIPLE"_s,
          u"MULTIPROVIDERCONFIG"_s,
          u"MVCC_SNAPSHOT_TIMESTAMP"_s,
          u"NAME"_s,
          u"NATIONAL"_s,
          u"NATURAL"_s,
          u"NCHAR"_s,
          u"NCLOB"_s,
          u"NEAREST"_s,
          u"NEIGHBORS"_s,
          u"NESTED"_s,
          u"NESTED_LOOP_JOIN"_s,
          u"NETAPP"_s,
          u"NEW"_s,
          u"NEWFACTTABLE"_s,
          u"NEXT"_s,
          u"NEXT_DAY"_s,
          u"NEXTVAL"_s,
          u"NO"_s,
          u"NO_CALC_DIMENSION"_s,
          u"NO_DISTINCT_FILTER"_s,
          u"NO_INDEX"_s,
          u"NO_INLINE"_s,
          u"NO_ROUTE_TO"_s,
          u"NO_USE_C2C_CONV"_s,
          u"NO_USE_OLAP_PLAN"_s,
          u"NO_USE_TRANSFORMATION"_s,
          u"NO_VIRTUAL_TABLE_REPLICA"_s,
          u"NOCOMPRESS"_s,
          u"NODE"_s,
          u"NON"_s,
          u"NONE"_s,
          u"NONLEAF"_s,
          u"NOT"_s,
          u"NOW"_s,
          u"NOWAIT"_s,
          u"NTEXT"_s,
          u"NTILE"_s,
          u"NULL"_s,
          u"NULLABLE"_s,
          u"NULLIF"_s,
          u"NULLS"_s,
          u"NUMA"_s,
          u"NUMA_NODE_INDEXES"_s,
          u"NUMBER"_s,
          u"NUMERIC"_s,
          u"NVARCHAR"_s,
          u"OBJECT"_s,
          u"OBJECTS"_s,
          u"OCCURRENCE"_s,
          u"OCCURRENCES_REGEXPR"_s,
          u"ODATA"_s,
          u"OF"_s,
          u"OFF"_s,
          u"OFFSET"_s,
          u"OJ"_s,
          u"OLAP"_s,
          u"OLAP_PARALLEL_AGGREGATION"_s,
          u"OLAP_SERIAL_AGGREGATION"_s,
          u"OLD"_s,
          u"OLYMP"_s,
          u"ON"_s,
          u"ONE"_s,
          u"ONLINE"_s,
          u"ONLY"_s,
          u"OPEN"_s,
          u"OPENCYPHER_TABLE"_s,
          u"OPERATION"_s,
          u"OPERATOR"_s,
          u"OPTIMIZATION"_s,
          u"OPTIMIZEMETAMODEL"_s,
          u"OPTIMIZER"_s,
          u"OPTION"_s,
          u"OPTIONALLY"_s,
          u"OR"_s,
          u"ORDER"_s,
          u"ORDINALITY"_s,
          u"ORGANIZATION"_s,
          u"ORPHAN"_s,
          u"OS"_s,
          u"OTHERS"_s,
          u"OUT"_s,
          u"OUTER"_s,
          u"OVER"_s,
          u"OVERLAY"_s,
          u"OVERRIDE"_s,
          u"OVERRIDING"_s,
          u"OVERVIEW"_s,
          u"OWN"_s,
          u"OWNED"_s,
          u"OWNER"_s,
          u"PACKAGE"_s,
          u"PAGE"_s,
          u"PAGE_LOADABLE"_s,
          u"PAGES"_s,
          u"PARALLEL"_s,
          u"PARALLELIZED"_s,
          u"PARAMETER"_s,
          u"PARAMETERS"_s,
          u"PARENT"_s,
          u"PARENTSATTRIBUTE"_s,
          u"PARQUET"_s,
          u"PART"_s,
          u"PARTIAL"_s,
          u"PARTITION"_s,
          u"PARTITIONING"_s,
          u"PARTITIONS"_s,
          u"PARTS"_s,
          u"PASS"_s,
          u"PASSING"_s,
          u"PASSPORT_TRACELEVEL"_s,
          u"PASSWORD"_s,
          u"PATH"_s,
          u"PERCENT"_s,
          u"PERCENT_RANK"_s,
          u"PERCENTILE_CONT"_s,
          u"PERCENTILE_DISC"_s,
          u"PERFTRACE"_s,
          u"PERIOD"_s,
          u"PERSISTENCE"_s,
          u"PERSISTENT"_s,
          u"PERSISTENT_MEMORY"_s,
          u"PHRASE"_s,
          u"PHYSICAL"_s,
          u"PIN"_s,
          u"PLACEMENT"_s,
          u"PLAIN"_s,
          u"PLAN"_s,
          u"PLAN_EXECUTION"_s,
          u"PLANAR"_s,
          u"PLANNING"_s,
          u"PLANVIZ"_s,
          u"POBJECTKEY"_s,
          u"POLICY"_s,
          u"POLYGON"_s,
          u"PORT"_s,
          u"PORTION"_s,
          u"POSITION"_s,
          u"POSTCODE"_s,
          u"PPROPERTYNAME"_s,
          u"PRECEDES"_s,
          u"PRECEDING"_s,
          u"PRECISION"_s,
          u"PREDEFINED"_s,
          u"PREFERENCE"_s,
          u"PREFERRED"_s,
          u"PREFIX"_s,
          u"PRELOAD"_s,
          u"PREPROCESS"_s,
          u"PRESERVE"_s,
          u"PREVIOUS"_s,
          u"PRIMARY"_s,
          u"PRINCIPAL"_s,
          u"PRIOR"_s,
          u"PRIORITY"_s,
          u"PRIVATE"_s,
          u"PRIVILEGE"_s,
          u"PRIVILEGES"_s,
          u"PROCEDURE"_s,
          u"PROCESS"_s,
          u"PRODUCT"_s,
          u"PROFILE"_s,
          u"PROFILER"_s,
          u"PROJECTION"_s,
          u"PROPERTIES"_s,
          u"PROPERTY"_s,
          u"PROTOCOL"_s,
          u"PROVIDER"_s,
          u"PRUNING"_s,
          u"PSE"_s,
          u"PTIME"_s,
          u"PUBLIC"_s,
          u"PURPOSE"_s,
          u"PVALUENAME"_s,
          u"QERROR"_s,
          u"QTHETA"_s,
          u"QUERY"_s,
          u"QUEUE"_s,
          u"RAISE"_s,
          u"RANDOM"_s,
          u"RANGE"_s,
          u"RANK"_s,
          u"RATIO"_s,
          u"RAW"_s,
          u"RDICT"_s,
          u"READ"_s,
          u"READS"_s,
          u"REAL"_s,
          u"REALTIME"_s,
          u"REBUILD"_s,
          u"RECLAIM"_s,
          u"RECOMPILE"_s,
          u"RECOMPILED"_s,
          u"RECONFIGURE"_s,
          u"RECORD"_s,
          u"RECORD_COMMIT_TIMESTAMP"_s,
          u"RECORD_COUNT"_s,
          u"RECORD_ID"_s,
          u"RECORDS"_s,
          u"RECOVER"_s,
          u"RECOVERY"_s,
          u"RECURSIVE"_s,
          u"REFERENCE"_s,
          u"REFERENCES"_s,
          u"REFERENCING"_s,
          u"REFRESH"_s,
          u"REGISTER"_s,
          u"RELEASE"_s,
          u"RELOAD"_s,
          u"REMOTE"_s,
          u"REMOTE_EXECUTE_QUERY"_s,
          u"REMOTE_SCAN"_s,
          u"REMOVE"_s,
          u"RENAME"_s,
          u"REORGANIZE"_s,
          u"REPARTITIONING_THRESHOLD"_s,
          u"REPEATABLE"_s,
          u"REPLACE"_s,
          u"REPLACE_REGEXPR"_s,
          u"REPLAY"_s,
          u"REPLICA"_s,
          u"REPLICA_COUNT"_s,
          u"REPLICA_TYPE"_s,
          u"REPLICAS"_s,
          u"REPLICATION"_s,
          u"REPOSITORY"_s,
          u"RESERVE"_s,
          u"RESET"_s,
          u"RESIGNAL"_s,
          u"RESOURCE"_s,
          u"RESTART"_s,
          u"RESTORE"_s,
          u"RESTRICT"_s,
          u"RESTRICTED"_s,
          u"RESTRICTION"_s,
          u"RESULT"_s,
          u"RESULT_LAG"_s,
          u"RESULTSETS"_s,
          u"RESUME"_s,
          u"RETAIN"_s,
          u"RETENTION"_s,
          u"RETRY"_s,
          u"RETURN"_s,
          u"RETURNING"_s,
          u"RETURNS"_s,
          u"REVERSE"_s,
          u"REVOKE"_s,
          u"RIGHT"_s,
          u"ROLE"_s,
          u"ROLEGROUP"_s,
          u"ROLLBACK"_s,
          u"ROLLUP"_s,
          u"ROOT"_s,
          u"ROOT_STATEMENT_HASH"_s,
          u"ROUND"_s,
          u"ROUND_CEILING"_s,
          u"ROUND_DOWN"_s,
          u"ROUND_FLOOR"_s,
          u"ROUND_HALF_DOWN"_s,
          u"ROUND_HALF_EVEN"_s,
          u"ROUND_HALF_UP"_s,
          u"ROUND_UP"_s,
          u"ROUNDROBIN"_s,
          u"ROUTE"_s,
          u"ROUTE_BY"_s,
          u"ROUTE_BY_CARDINALITY"_s,
          u"ROUTE_TO"_s,
          u"ROW"_s,
          u"ROW_NUMBER"_s,
          u"ROWCOUNT"_s,
          u"ROWID"_s,
          u"ROWS"_s,
          u"RPAD"_s,
          u"RTREE"_s,
          u"RTRIM"_s,
          u"RULE"_s,
          u"RULES"_s,
          u"RUNTIME"_s,
          u"RUNTIMEDUMP"_s,
          u"SAME_PARTITION_COUNT"_s,
          u"SAML"_s,
          u"SAMPLE"_s,
          u"SAMPLING"_s,
          u"SAP_TIMEZONE_DATASET"_s,
          u"SATISFIES"_s,
          u"SAVE"_s,
          u"SAVEPOINT"_s,
          u"SCAN"_s,
          u"SCENARIO"_s,
          u"SCHEDULER"_s,
          u"SCHEMA"_s,
          u"SCHEMA_NAME"_s,
          u"SCORE"_s,
          u"SCRAMBLE"_s,
          u"SCROLL"_s,
          u"SEARCH"_s,
          u"SECOND"_s,
          u"SECONDDATE"_s,
          u"SECONDS_BETWEEN"_s,
          u"SECONDTIME"_s,
          u"SECTIONS"_s,
          u"SECURE"_s,
          u"SECURITY"_s,
          u"SEED"_s,
          u"SELECT"_s,
          u"SEMANTICRELATION"_s,
          u"SEMI"_s,
          u"SENSITIVE"_s,
          u"SEPARATORS"_s,
          u"SEQUENCE"_s,
          u"SEQUENTIAL"_s,
          u"SERIALIZABLE"_s,
          u"SERIES"_s,
          u"SERIES_ELEMENT_TO_PERIOD"_s,
          u"SERIES_PERIOD_TO_ELEMENT"_s,
          u"SERIES_ROUND"_s,
          u"SERVICE"_s,
          u"SERVICES"_s,
          u"SESSION"_s,
          u"SESSION_CONTEXT"_s,
          u"SESSION_USER"_s,
          u"SET"_s,
          u"SETOLAPMODEL"_s,
          u"SETS"_s,
          u"SHAPEFILE"_s,
          u"SHARE"_s,
          u"SHARED"_s,
          u"SHOW"_s,
          u"SIBLING"_s,
          u"SIDATTRIBUTE"_s,
          u"SIGNAL"_s,
          u"SIMPLE"_s,
          u"SITE"_s,
          u"SIZE"_s,
          u"SKETCH"_s,
          u"SKIP"_s,
          u"SMALLDECIMAL"_s,
          u"SMALLINT"_s,
          u"SNAP"_s,
          u"SNAPINT"_s,
          u"SNAPSHOT"_s,
          u"SOME"_s,
          u"SORT"_s,
          u"SOURCE"_s,
          u"SPACE"_s,
          u"SPARSIFY"_s,
          u"SPATIAL"_s,
          u"SPLITFACTOR"_s,
          u"SQL"_s,
          u"SQL_ERROR_CODE"_s,
          u"SQLSCRIPT"_s,
          u"SRID"_s,
          u"SSL"_s,
          u"STAB"_s,
          u"STANDARD"_s,
          u"START"_s,
          u"STATEMENT"_s,
          u"STATEMENT_NAME"_s,
          u"STATIC"_s,
          u"STATISTICS"_s,
          u"STOP"_s,
          u"STORAGE"_s,
          u"STORE"_s,
          u"STRING"_s,
          u"STRIP"_s,
          u"STRUCTURED"_s,
          u"STRUCTUREDPRIVILEGE"_s,
          u"SUB_TYPE"_s,
          u"SUBJECT"_s,
          u"SUBPARTITION"_s,
          u"SUBPARTITIONS"_s,
          u"SUBSCRIPTION"_s,
          u"SUBSTR"_s,
          u"SUBSTR_AFTER"_s,
          u"SUBSTR_BEFORE"_s,
          u"SUBSTR_REGEXPR"_s,
          u"SUBSTRING"_s,
          u"SUBSTRING_REGEXPR"_s,
          u"SUBTOTAL"_s,
          u"SUBTYPE"_s,
          u"SUCCESSFUL"_s,
          u"SUPPORT"_s,
          u"SUSPEND"_s,
          u"SYNC"_s,
          u"SYNCHRONOUS"_s,
          u"SYNONYM"_s,
          u"SYSLOG"_s,
          u"SYSTEM"_s,
          u"SYSTEM_TIME"_s,
          u"SYSTEMS"_s,
          u"SYSUUID"_s,
          u"T"_s,
          u"TABLE"_s,
          u"TABLE_NAME"_s,
          u"TABLES"_s,
          u"TABLESAMPLE"_s,
          u"TAKEOVER"_s,
          u"TARGET"_s,
          u"TASK"_s,
          u"TB"_s,
          u"TEMPLATEINDEX"_s,
          u"TEMPORARY"_s,
          u"TENANT"_s,
          u"TERM"_s,
          u"TEXT"_s,
          u"TEXTATTRIBUTE"_s,
          u"THEN"_s,
          u"THREAD"_s,
          u"THREADS"_s,
          u"THRESHOLD"_s,
          u"THROW_ERROR"_s,
          u"TIME"_s,
          u"TIMELINE"_s,
          u"TIMEOUT"_s,
          u"TIMESTAMP"_s,
          u"TIMEZONE"_s,
          u"TIMS_EXTRACT"_s,
          u"TINYINT"_s,
          u"TM_CATEGORIZE_KNN"_s,
          u"TM_GET_RELATED_DOCUMENTS"_s,
          u"TM_GET_RELATED_TERMS"_s,
          u"TM_GET_RELEVANT_DOCUMENTS"_s,
          u"TM_GET_RELEVANT_TERMS"_s,
          u"TM_GET_SUGGESTED_TERMS"_s,
          u"TO"_s,
          u"TO_BIGINT"_s,
          u"TO_BINARY"_s,
          u"TO_BLOB"_s,
          u"TO_CHAR"_s,
          u"TO_CLOB"_s,
          u"TO_DATE"_s,
          u"TO_DECIMAL"_s,
          u"TO_DOUBLE"_s,
          u"TO_INT"_s,
          u"TO_INTEGER"_s,
          u"TO_JSON_BOOLEAN"_s,
          u"TO_JSON_NUMBER"_s,
          u"TO_NCHAR"_s,
          u"TO_NCLOB"_s,
          u"TO_NUMBER"_s,
          u"TO_NVARCHAR"_s,
          u"TO_REAL"_s,
          u"TO_SECONDDATE"_s,
          u"TO_SMALLDECIMAL"_s,
          u"TO_SMALLINT"_s,
          u"TO_TIME"_s,
          u"TO_TIMESTAMP"_s,
          u"TO_TINYINT"_s,
          u"TO_VARBINARY"_s,
          u"TO_VARCHAR"_s,
          u"TOKEN"_s,
          u"TOLERANCE"_s,
          u"TOOLOPTION"_s,
          u"TOP"_s,
          u"TOPK"_s,
          u"TOTAL"_s,
          u"TRACE"_s,
          u"TRACEPROFILE"_s,
          u"TRACES"_s,
          u"TRAIL"_s,
          u"TRAILING"_s,
          u"TRANSACTION"_s,
          u"TRANSFORM"_s,
          u"TREE"_s,
          u"TREX"_s,
          u"TRIGGER"_s,
          u"TRIGGER_UPDATE_COLUMN"_s,
          u"TRIM"_s,
          u"TRUE"_s,
          u"TRUNCATE"_s,
          u"TRUST"_s,
          u"TS"_s,
          u"TYPE"_s,
          u"TYPENUMBERATTRIBUTE"_s,
          u"TYPETEXTATTRIBUTE"_s,
          u"UNAUTHORIZED"_s,
          u"UNBOUNDED"_s,
          u"UNCOMMITTED"_s,
          u"UNCONDITIONAL"_s,
          u"UNION"_s,
          u"UNIQUE"_s,
          u"UNIT"_s,
          u"UNITCONVERSION"_s,
          u"UNITCONVERSIONNAME"_s,
          u"UNKNOWN"_s,
          u"UNLOAD"_s,
          u"UNLOCK"_s,
          u"UNMASKED"_s,
          u"UNNEST"_s,
          u"UNPIN"_s,
          u"UNREGISTER"_s,
          u"UNSET"_s,
          u"UNSUCCESSFUL"_s,
          u"UNTIL"_s,
          u"UNUSED"_s,
          u"UP"_s,
          u"UPDATE"_s,
          u"UPSERT"_s,
          u"URL"_s,
          u"USAGE"_s,
          u"USE_C2R_CONV"_s,
          u"USE_COLUMN_JOIN_IMPLICIT_CAST"_s,
          u"USE_OLAP_PLAN"_s,
          u"USE_PREAGGR"_s,
          u"USE_QUERY_MATCH"_s,
          u"USE_R2C_CONV"_s,
          u"USE_TRANSFORMATION"_s,
          u"USE_UNION_OPT"_s,
          u"USEINITIALREORG"_s,
          u"USER"_s,
          u"USERGROUP"_s,
          u"USERS"_s,
          u"USING"_s,
          u"UTCTIMESTAMP"_s,
          u"UTF16"_s,
          u"UTF32"_s,
          u"UTF8"_s,
          u"VALID"_s,
          u"VALIDATE"_s,
          u"VALIDATED"_s,
          u"VALIDATION"_s,
          u"VALUE"_s,
          u"VALUES"_s,
          u"VARBINARY"_s,
          u"VARCHAR"_s,
          u"VARCHAR1"_s,
          u"VARCHAR2"_s,
          u"VARCHAR3"_s,
          u"VARIABLE"_s,
          u"VARYING"_s,
          u"VERIFY"_s,
          u"VERSION"_s,
          u"VERSIONING"_s,
          u"VERSIONS"_s,
          u"VERTEX"_s,
          u"VERTICAL"_s,
          u"VIEW"_s,
          u"VIEWATTRIBUTE"_s,
          u"VIRTUAL"_s,
          u"VOLUME"_s,
          u"VOLUMES"_s,
          u"WAIT"_s,
          u"WAITGRAPH"_s,
          u"WARNING"_s,
          u"WEEKDAY"_s,
          u"WEIGHT"_s,
          u"WHEN"_s,
          u"WHERE"_s,
          u"WHILE"_s,
          u"WHY_FOUND"_s,
          u"WILDCARD"_s,
          u"WINDOW"_s,
          u"WITH"_s,
          u"WITHIN"_s,
          u"WITHOUT"_s,
          u"WORK"_s,
          u"WORKAROUND"_s,
          u"WORKERGROUPS"_s,
          u"WORKLOAD"_s,
          u"WORKSPACE"_s,
          u"WRAPPER"_s,
          u"WRITE"_s,
          u"X"_s,
          u"X509"_s,
          u"XML"_s,
          u"XMLNAMESPACE"_s,
          u"XMLTABLE"_s,
          u"XTAB"_s,
          u"Y"_s,
          u"YEAR"_s,
          u"YTAB"_s,
          u"ZONE"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Aggregate,
        {
          u"AUTO_CORR"_s,
          u"AVG"_s,
          u"CORR"_s,
          u"CORR_SPEARMAN"_s,
          u"COUNT"_s,
          u"CROSS_CORR"_s,
          u"DFT"_s,
          u"FIRST_VALUE"_s,
          u"LAST_VALUE"_s,
          u"MAX"_s,
          u"MEDIAN"_s,
          u"MIN"_s,
          u"NTH_VALUE"_s,
          u"STDDEV"_s,
          u"STDDEV_POP"_s,
          u"STDDEV_SAMP"_s,
          u"STRING_AGG"_s,
          u"SUM"_s,
          u"VAR"_s,
          u"VAR_POP"_s,
          u"VAR_SAMP"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Math,
        {
          u"ABS"_s,
          u"ACOS"_s,
          u"ASIN"_s,
          u"ATAN"_s,
          u"ATAN2"_s,
          u"BITAND"_s,
          u"BITCOUNT"_s,
          u"BITNOT"_s,
          u"BITOR"_s,
          u"BITSET"_s,
          u"BITUNSET"_s,
          u"BITXOR"_s,
          u"CEIL"_s,
          u"COS"_s,
          u"COSH"_s,
          u"COT"_s,
          u"EXP"_s,
          u"FLOOR"_s,
          u"LN"_s,
          u"LOG"_s,
          u"MOD"_s,
          u"NDIV0"_s,
          u"POWER"_s,
          u"RAND"_s,
          u"RAND_SECURE"_s,
          u"ROUND"_s,
          u"SIGN"_s,
          u"SIN"_s,
          u"SINH"_s,
          u"SQRT"_s,
          u"TAN"_s,
          u"TANH"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Geospatial,
        {
          u"ST_AlphaShape"_s,
          u"ST_AlphaShapeAggr"_s,
          u"ST_AlphaShapeArea"_s,
          u"ST_AlphaShapeAreaAggr"_s,
          u"ST_AlphaShapeEdge"_s,
          u"ST_AlphaShapeEdgeAggr"_s,
          u"ST_AsBinary"_s,
          u"ST_AsEsriJSON"_s,
          u"ST_AsEWKB"_s,
          u"ST_AsEWKT"_s,
          u"ST_AsGeoJSON"_s,
          u"ST_AsSVG"_s,
          u"ST_AsSVGAggr"_s,
          u"ST_AsText"_s,
          u"ST_AsWKB"_s,
          u"ST_AsWKT"_s,
          u"ST_Boundary"_s,
          u"ST_Buffer"_s,
          u"ST_CircularString"_s,
          u"ST_Collect"_s,
          u"ST_CollectAggr"_s,
          u"ST_Contains"_s,
          u"ST_ConvexHull"_s,
          u"ST_ConvexHullAggr"_s,
          u"ST_CoordDim"_s,
          u"ST_CoveredBy"_s,
          u"ST_Covers"_s,
          u"ST_Crosses"_s,
          u"ST_Difference"_s,
          u"ST_Dimension"_s,
          u"ST_Disjoint"_s,
          u"ST_Distance"_s,
          u"ST_Envelope"_s,
          u"ST_EnvelopeAggr"_s,
          u"ST_EnvelopeAggr"_s,
          u"ST_Equals"_s,
          u"ST_Force2D"_s,
          u"ST_Force3DM"_s,
          u"ST_Force3DZ"_s,
          u"ST_Force4D"_s,
          u"ST_GeoHash"_s,
          u"ST_GeomFromEsriJSON"_s,
          u"ST_GeomFromEWKB"_s,
          u"ST_GeomFromEWKT"_s,
          u"ST_GeomFromGeoHash"_s,
          u"ST_GeomFromText"_s,
          u"ST_GeomFromWKB"_s,
          u"ST_GeomFromWKT"_s,
          u"ST_GeometryCollection"_s,
          u"ST_GeometryN"_s,
          u"ST_GeometryType"_s,
          u"ST_Intersection"_s,
          u"ST_IntersectionAggr"_s,
          u"ST_Intersects"_s,
          u"ST_IntersectsFilter"_s,
          u"ST_IntersectsRect"_s,
          u"ST_Is3D"_s,
          u"ST_IsEmpty"_s,
          u"ST_IsMeasured"_s,
          u"ST_IsSimple"_s,
          u"ST_IsValid"_s,
          u"ST_LineString"_s,
          u"ST_MultiLineString"_s,
          u"ST_MultiPoint"_s,
          u"ST_MultiPolygon"_s,
          u"ST_MMax"_s,
          u"ST_MMin"_s,
          u"ST_NumInteriorRing"_s,
          u"ST_NumInteriorRings"_s,
          u"ST_OrderingEquals"_s,
          u"ST_Overlaps"_s,
          u"ST_Perimeter"_s,
          u"ST_Point"_s,
          u"ST_PointOnSurface"_s,
          u"ST_Polygon"_s,
          u"ST_SquareGrid"_s,
          u"ST_RectangleGrid"_s,
          u"ST_RectangleGridBoundingBox"_s,
          u"ST_Relate"_s,
          u"ST_Rotate"_s,
          u"ST_Scale"_s,
          u"ST_Simplify"_s,
          u"ST_SnapToGrid"_s,
          u"ST_SRID"_s,
          u"ST_SymDifference"_s,
          u"ST_Touches"_s,
          u"ST_Transform"_s,
          u"ST_Translate"_s,
          u"ST_Translate3D"_s,
          u"ST_Union"_s,
          u"ST_UnionAggr"_s,
          u"ST_VoronoiCell"_s,
          u"ST_Within"_s,
          u"ST_WithinDistance"_s,
          u"ST_XMax"_s,
          u"ST_XMin"_s,
          u"ST_YMax"_s,
          u"ST_YMin"_s,
          u"ST_ZMax"_s,
          u"ST_ZMin"_s,
        }
      }
    }
  );
}

Qgis::DatabaseProviderTableImportCapabilities QgsHanaProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName | Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName;
}

QString QgsHanaProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"id"_s;
}

QVariantList QgsHanaEmptyProviderResultIterator::nextRowPrivate()
{
  return QVariantList();
}

bool QgsHanaEmptyProviderResultIterator::hasNextRowPrivate() const
{
  return false;
}
