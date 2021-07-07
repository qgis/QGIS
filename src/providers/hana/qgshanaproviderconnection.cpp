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
#include "qgshanaprovider.h"
#include "qgshanaresultset.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgsexception.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsfeedback.h"

#include "odbc/PreparedStatement.h"

#include <chrono>

QgsHanaProviderResultIterator::QgsHanaProviderResultIterator( QgsHanaResultSetRef &&resultSet )
  : mResultSet( std::move( resultSet ) )
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
  // TODO: hana team, this is for you.
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

    odbc::PreparedStatementRef stmt = conn->prepareStatement( sql );
    bool isQuery = stmt->getMetaDataUnicode()->getColumnCount() > 0;
    if ( isQuery )
    {
      QgsHanaResultSetRef rs = conn->executeQuery( sql );
      odbc::ResultSetMetaDataUnicode &md = rs->getMetadata();
      QueryResult ret( std::make_shared<QgsHanaProviderResultIterator>( std::move( rs ) ) );
      unsigned short numColumns = md.getColumnCount();
      for ( unsigned short i = 1; i <= numColumns; ++i )
        ret.appendColumn( QgsHanaUtils::toQString( md.getColumnName( i ) ) );
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
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve fields: %1, %2" ).arg( uri(), ex.what() ) );
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

QVariantList QgsHanaEmptyProviderResultIterator::nextRowPrivate()
{
  return QVariantList();
}

bool QgsHanaEmptyProviderResultIterator::hasNextRowPrivate() const
{
  return false;
}
